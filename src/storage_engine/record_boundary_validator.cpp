/**
 * @file record_boundary_validator.cpp
 * @brief 记录操作边界验证器实现 - 实现严格的记录大小、类型和完整性验证
 *
 * 该文件实现了记录边界验证器的核心功能，包括：
 * - 记录大小限制和验证
 * - 字段类型边界检查
 * - 数据完整性约束验证
 * - 并发访问控制验证
 */

#include "storage/record_boundary_validator.h"
#include "storage/table_storage.h"
#include "exception.h"
#include "utils/logger.h"
#include <algorithm>
#include <chrono>
#include <cstring>
#include <regex>
#include <sstream>
#include <cmath>

namespace sqlcc {

// 初始化静态正则表达式模式
const std::regex FieldTypeValidation::UUID_PATTERN(
    R"([0-9a-f]{8}-[0-9a-f]{4}-[1-5][0-9a-f]{3}-[89ab][0-9a-f]{3}-[0-9a-f]{12})",
    std::regex_constants::icase);

const std::regex FieldTypeValidation::EMAIL_PATTERN(
    R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");

const std::regex FieldTypeValidation::URL_PATTERN(
    R"(https?://(www\.)?[-a-zA-Z0-9@:%._\+~#=]{1,256}\.[a-zA-Z0-9()]{1,6}\b([-a-zA-Z0-9()@:%_\+.~#?&//=]*))");

const std::regex FieldTypeValidation::PHONE_PATTERN(
    R"(\+?[\d\s\-\(\)]{10,20})");

const std::regex FieldTypeValidation::DATE_PATTERN(
    R"(\d{4}-\d{2}-\d{2})");

const std::regex FieldTypeValidation::TIME_PATTERN(
    R"(\d{2}:\d{2}:\d{2}(\.\d{1,6})?)");

const std::regex FieldTypeValidation::DATETIME_PATTERN(
    R"(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}(\.\d{1,6})?)");

// 记录大小验证器实现
RecordSizeValidator::RecordSizeValidator()
    : max_record_size_(RecordSizeLimits::DEFAULT_MAX_RECORD_SIZE),
      strict_mode_(true) {
}

ValidationResult RecordSizeValidator::ValidateRecordSize(size_t record_size, size_t max_size) const {
    if (record_size == 0) {
        return INVALID_SIZE;
    }

    if (record_size > max_size) {
        return INVALID_SIZE;
    }

    if (record_size > RecordSizeLimits::MAX_RECORD_SIZE) {
        return BOUNDARY_VIOLATION;
    }

    return VALID;
}

ValidationResult RecordSizeValidator::ValidateFieldSize(const std::string& field_type, size_t field_size) const {
    if (field_type == "VARCHAR" && field_size > RecordSizeLimits::MAX_VARCHAR_LENGTH) {
        return INVALID_SIZE;
    }

    if (field_type == "TEXT" && field_size > RecordSizeLimits::MAX_TEXT_LENGTH) {
        return INVALID_SIZE;
    }

    if (field_type == "BLOB" && field_size > RecordSizeLimits::MAX_BLOB_LENGTH) {
        return INVALID_SIZE;
    }

    return VALID;
}

ValidationResult RecordSizeValidator::ValidateTotalRecordSize(
    const std::vector<std::pair<std::string, size_t>>& field_sizes,
    size_t header_size) const {

    size_t total_size = header_size;

    for (const auto& field_size : field_sizes) {
        const std::string& field_type = field_size.first;
        size_t size = field_size.second;

        // 检查字段大小
        ValidationResult field_result = ValidateFieldSize(field_type, size);
        if (field_result != VALID) {
            return field_result;
        }

        // 累加到总大小
        if (field_type == "VARCHAR" || field_type == "TEXT" || field_type == "BLOB") {
            total_size += sizeof(uint32_t) + size; // 长度前缀 + 实际数据
        } else if (field_type == "INT" || field_type == "INTEGER") {
            total_size += sizeof(int32_t);
        } else if (field_type == "BIGINT") {
            total_size += sizeof(int64_t);
        } else if (field_type == "FLOAT") {
            total_size += sizeof(float);
        } else if (field_type == "DOUBLE") {
            total_size += sizeof(double);
        } else {
            // 默认当作固定大小处理
            total_size += size;
        }
    }

    return ValidateRecordSize(total_size, max_record_size_);
}

size_t RecordSizeValidator::CalculateFieldSize(const std::string& field_type, const std::string& value) const {
    if (field_type == "VARCHAR" || field_type == "TEXT" || field_type == "BLOB") {
        return value.length();
    } else if (field_type == "INT" || field_type == "INTEGER") {
        return sizeof(int32_t);
    } else if (field_type == "BIGINT") {
        return sizeof(int64_t);
    } else if (field_type == "FLOAT") {
        return sizeof(float);
    } else if (field_type == "DOUBLE") {
        return sizeof(double);
    } else {
        // 默认当作固定大小处理
        return value.length();
    }
}

size_t RecordSizeValidator::CalculateRecordSize(const std::vector<std::pair<std::string, std::string>>& fields,
                                              const std::shared_ptr<TableMetadata>& metadata) const {
    size_t total_size = sizeof(RecordHeader); // 记录头部大小

    for (const auto& field : fields) {
        const std::string& field_name = field.first;
        const std::string& value = field.second;

        // 从元数据中获取字段类型
        auto it = metadata->column_index_map.find(field_name);
        if (it != metadata->column_index_map.end()) {
            size_t col_index = it->second;
            if (col_index < metadata->columns.size()) {
                const std::string& field_type = metadata->columns[col_index].type;
                total_size += CalculateFieldSize(field_type, value);
            }
        }
    }

    return total_size;
}

void RecordSizeValidator::SetMaxRecordSize(size_t max_size) {
    std::lock_guard<std::mutex> lock(validator_mutex_);
    max_record_size_ = max_size;
}

void RecordSizeValidator::SetStrictMode(bool strict) {
    std::lock_guard<std::mutex> lock(validator_mutex_);
    strict_mode_ = strict;
}

size_t RecordSizeValidator::GetMaxRecordSize() const {
    std::lock_guard<std::mutex> lock(validator_mutex_);
    return max_record_size_;
}

// 字段类型边界验证器实现
FieldTypeBoundaryValidator::FieldTypeBoundaryValidator() = default;

ValidationResult FieldTypeBoundaryValidator::ValidateInteger(const std::string& value, bool is_bigint) const {
    if (!IsValidIntegerFormat(value)) {
        return INVALID_FORMAT;
    }

    try {
        if (is_bigint) {
            long long int_value = std::stoll(value);
            if (int_value < FieldTypeValidation::INT64_MIN_VAL ||
                int_value > FieldTypeValidation::INT64_MAX_VAL) {
                return BOUNDARY_VIOLATION;
            }
        } else {
            long long int_value = std::stoll(value);
            if (int_value < FieldTypeValidation::INT32_MIN_VAL ||
                int_value > FieldTypeValidation::INT32_MAX_VAL) {
                return BOUNDARY_VIOLATION;
            }
        }
    } catch (const std::exception&) {
        return INVALID_VALUE;
    }

    return VALID;
}

ValidationResult FieldTypeBoundaryValidator::ValidateFloat(const std::string& value, bool is_double) const {
    if (!IsValidFloatFormat(value)) {
        return INVALID_FORMAT;
    }

    try {
        if (is_double) {
            double double_value = std::stod(value);
            if (std::isnan(double_value) || std::isinf(double_value)) {
                return INVALID_VALUE;
            }
        } else {
            float float_value = std::stof(value);
            if (std::isnan(float_value) || std::isinf(float_value)) {
                return INVALID_VALUE;
            }
        }
    } catch (const std::exception&) {
        return INVALID_VALUE;
    }

    return VALID;
}

ValidationResult FieldTypeBoundaryValidator::ValidateNumeric(const std::string& value, int precision, int scale) const {
    if (!IsValidFloatFormat(value)) {
        return INVALID_FORMAT;
    }

    try {
        double numeric_value = std::stod(value);

        // 检查精度
        if (precision > 0) {
            std::string int_part = value.substr(0, value.find('.'));
            if (int_part.length() > static_cast<size_t>(precision - scale)) {
                return BOUNDARY_VIOLATION;
            }
        }

        // 检查小数位数
        if (scale >= 0) {
            size_t dot_pos = value.find('.');
            if (dot_pos != std::string::npos) {
                size_t decimal_digits = value.length() - dot_pos - 1;
                if (decimal_digits > static_cast<size_t>(scale)) {
                    return BOUNDARY_VIOLATION;
                }
            }
        }

    } catch (const std::exception&) {
        return INVALID_VALUE;
    }

    return VALID;
}

ValidationResult FieldTypeBoundaryValidator::ValidateString(const std::string& value, size_t max_length) const {
    if (value.length() > max_length) {
        return INVALID_SIZE;
    }

    if (ContainsInvalidCharacters(value)) {
        return INVALID_VALUE;
    }

    return VALID;
}

ValidationResult FieldTypeBoundaryValidator::ValidateVarchar(const std::string& value, size_t max_length) const {
    return ValidateString(value, max_length);
}

ValidationResult FieldTypeBoundaryValidator::ValidateText(const std::string& value, size_t max_length) const {
    return ValidateString(value, max_length);
}

ValidationResult FieldTypeBoundaryValidator::ValidateDate(const std::string& value) const {
    if (!std::regex_match(value, FieldTypeValidation::DATE_PATTERN)) {
        return INVALID_FORMAT;
    }

    // 简单的日期有效性检查
    try {
        int year, month, day;
        if (sscanf(value.c_str(), "%d-%d-%d", &year, &month, &day) != 3) {
            return INVALID_FORMAT;
        }

        if (year < 1900 || year > 2100 || month < 1 || month > 12 || day < 1 || day > 31) {
            return INVALID_VALUE;
        }

        // 检查月份的日期有效性
        static int days_in_month[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        if (month == 2 && ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0)) {
            days_in_month[2] = 29; // 闰年2月
        }

        if (day > days_in_month[month]) {
            return INVALID_VALUE;
        }

    } catch (const std::exception&) {
        return INVALID_VALUE;
    }

    return VALID;
}

ValidationResult FieldTypeBoundaryValidator::ValidateTime(const std::string& value) const {
    if (!std::regex_match(value, FieldTypeValidation::TIME_PATTERN)) {
        return INVALID_FORMAT;
    }

    try {
        int hour, minute, second;
        if (sscanf(value.c_str(), "%d:%d:%d", &hour, &minute, &second) != 3) {
            return INVALID_FORMAT;
        }

        if (hour < 0 || hour > 23 || minute < 0 || minute > 59 || second < 0 || second > 59) {
            return INVALID_VALUE;
        }

    } catch (const std::exception&) {
        return INVALID_VALUE;
    }

    return VALID;
}

ValidationResult FieldTypeBoundaryValidator::ValidateDateTime(const std::string& value) const {
    if (!std::regex_match(value, FieldTypeValidation::DATETIME_PATTERN)) {
        return INVALID_FORMAT;
    }

    // 分离日期和时间部分
    size_t space_pos = value.find(' ');
    if (space_pos == std::string::npos) {
        return INVALID_FORMAT;
    }

    std::string date_part = value.substr(0, space_pos);
    std::string time_part = value.substr(space_pos + 1);

    ValidationResult date_result = ValidateDate(date_part);
    if (date_result != VALID) {
        return date_result;
    }

    ValidationResult time_result = ValidateTime(time_part);
    if (time_result != VALID) {
        return time_result;
    }

    return VALID;
}

ValidationResult FieldTypeBoundaryValidator::ValidateTimestamp(const std::string& value) const {
    return ValidateDateTime(value);
}

ValidationResult FieldTypeBoundaryValidator::ValidateBoolean(const std::string& value) const {
    std::string trimmed = TrimWhitespace(value);
    if (trimmed == "true" || trimmed == "false" || trimmed == "1" || trimmed == "0" ||
        trimmed == "TRUE" || trimmed == "FALSE" || trimmed == "yes" || trimmed == "no" ||
        trimmed == "YES" || trimmed == "NO") {
        return VALID;
    }

    return INVALID_VALUE;
}

ValidationResult FieldTypeBoundaryValidator::ValidateUUID(const std::string& value) const {
    if (!std::regex_match(value, FieldTypeValidation::UUID_PATTERN)) {
        return INVALID_FORMAT;
    }

    return VALID;
}

ValidationResult FieldTypeBoundaryValidator::ValidateEmail(const std::string& value) const {
    if (!std::regex_match(value, FieldTypeValidation::EMAIL_PATTERN)) {
        return INVALID_FORMAT;
    }

    return VALID;
}

ValidationResult FieldTypeBoundaryValidator::ValidateURL(const std::string& value) const {
    if (!std::regex_match(value, FieldTypeValidation::URL_PATTERN)) {
        return INVALID_FORMAT;
    }

    return VALID;
}

ValidationResult FieldTypeBoundaryValidator::ValidateJSON(const std::string& value) const {
    if (!IsValidJSONFormat(value)) {
        return INVALID_FORMAT;
    }

    return VALID;
}

ValidationResult FieldTypeBoundaryValidator::ValidateFieldValue(const std::string& field_name,
                                                               const std::string& field_type,
                                                               const std::string& value,
                                                               const FieldValidationRule* rule) const {

    // 检查空值约束
    if (rule && !rule->nullable && value.empty()) {
        return NULL_VIOLATION;
    }

    // 根据字段类型进行验证
    if (field_type == "INT" || field_type == "INTEGER") {
        return ValidateInteger(value, false);
    } else if (field_type == "BIGINT") {
        return ValidateInteger(value, true);
    } else if (field_type == "FLOAT") {
        return ValidateFloat(value, false);
    } else if (field_type == "DOUBLE") {
        return ValidateFloat(value, true);
    } else if (field_type == "VARCHAR") {
        size_t max_len = rule ? rule->max_length : RecordSizeLimits::MAX_VARCHAR_LENGTH;
        return ValidateVarchar(value, max_len);
    } else if (field_type == "TEXT") {
        size_t max_len = rule ? rule->max_length : RecordSizeLimits::MAX_TEXT_LENGTH;
        return ValidateText(value, max_len);
    } else if (field_type == "DATE") {
        return ValidateDate(value);
    } else if (field_type == "TIME") {
        return ValidateTime(value);
    } else if (field_type == "DATETIME" || field_type == "TIMESTAMP") {
        return ValidateDateTime(value);
    } else if (field_type == "BOOLEAN" || field_type == "BOOL") {
        return ValidateBoolean(value);
    } else if (field_type == "UUID") {
        return ValidateUUID(value);
    } else if (field_type == "EMAIL") {
        return ValidateEmail(value);
    } else if (field_type == "URL") {
        return ValidateURL(value);
    } else if (field_type == "JSON") {
        return ValidateJSON(value);
    } else {
        // 默认当作字符串处理
        return ValidateString(value);
    }
}

std::vector<ValidationResult> FieldTypeBoundaryValidator::ValidateFields(
    const std::vector<std::string>& field_names,
    const std::vector<std::string>& field_types,
    const std::vector<std::string>& values,
    const std::vector<FieldValidationRule>& rules) const {

    std::vector<ValidationResult> results;

    if (field_names.size() != field_types.size() || field_types.size() != values.size()) {
        results.push_back(INVALID_SIZE);
        return results;
    }

    for (size_t i = 0; i < values.size(); ++i) {
        const FieldValidationRule* rule = nullptr;
        if (i < rules.size()) {
            rule = &rules[i];
        }

        ValidationResult result = ValidateFieldValue(field_names[i], field_types[i], values[i], rule);
        results.push_back(result);
    }

    return results;
}

// 私有辅助方法实现
bool FieldTypeBoundaryValidator::IsValidIntegerFormat(const std::string& value) const {
    if (value.empty()) return false;

    size_t start = 0;
    if (value[0] == '+' || value[0] == '-') {
        start = 1;
    }

    for (size_t i = start; i < value.length(); ++i) {
        if (!std::isdigit(value[i])) {
            return false;
        }
    }

    return true;
}

bool FieldTypeBoundaryValidator::IsValidFloatFormat(const std::string& value) const {
    if (value.empty()) return false;

    std::regex float_pattern(R"(^[+-]?\d*\.?\d+([eE][+-]?\d+)?$)");
    return std::regex_match(value, float_pattern);
}

bool FieldTypeBoundaryValidator::IsValidDateFormat(const std::string& value) const {
    return std::regex_match(value, FieldTypeValidation::DATE_PATTERN);
}

bool FieldTypeBoundaryValidator::IsValidTimeFormat(const std::string& value) const {
    return std::regex_match(value, FieldTypeValidation::TIME_PATTERN);
}

bool FieldTypeBoundaryValidator::IsValidUUIDFormat(const std::string& value) const {
    return std::regex_match(value, FieldTypeValidation::UUID_PATTERN);
}

bool FieldTypeBoundaryValidator::IsValidEmailFormat(const std::string& value) const {
    return std::regex_match(value, FieldTypeValidation::EMAIL_PATTERN);
}

bool FieldTypeBoundaryValidator::IsValidURLFormat(const std::string& value) const {
    return std::regex_match(value, FieldTypeValidation::URL_PATTERN);
}

bool FieldTypeBoundaryValidator::IsValidJSONFormat(const std::string& value) const {
    if (value.empty()) return false;

    std::string trimmed = TrimWhitespace(value);

    // 简单的JSON格式检查
    if ((trimmed[0] == '{' && trimmed.back() == '}') ||
        (trimmed[0] == '[' && trimmed.back() == ']') ||
        trimmed == "null" || trimmed == "true" || trimmed == "false" ||
        (trimmed[0] == '"' && trimmed.back() == '"') ||
        IsValidIntegerFormat(trimmed) || IsValidFloatFormat(trimmed)) {
        return true;
    }

    return false;
}

bool FieldTypeBoundaryValidator::ContainsInvalidCharacters(const std::string& value) const {
    for (char c : value) {
        if (c == '\0') {
            return true; // 不允许空字符
        }
        // 可以添加更多检查，如控制字符等
    }
    return false;
}

std::string FieldTypeBoundaryValidator::TrimWhitespace(const std::string& value) const {
    size_t start = value.find_first_not_of(" \t\n\r\f\v");
    if (start == std::string::npos) return "";

    size_t end = value.find_last_not_of(" \t\n\r\f\v");
    return value.substr(start, end - start + 1);
}

// 数据完整性约束验证器实现
DataIntegrityConstraintValidator::DataIntegrityConstraintValidator(std::shared_ptr<StorageEngine> storage_engine)
    : storage_engine_(std::move(storage_engine)) {
}

ValidationResult DataIntegrityConstraintValidator::ValidateNotNull(const std::string& field_name,
                                                                  const std::string& value,
                                                                  bool nullable) const {
    if (!nullable && value.empty()) {
        return NULL_VIOLATION;
    }

    return VALID;
}

ValidationResult DataIntegrityConstraintValidator::ValidateUnique(const std::string& table_name,
                                                                 const std::string& field_name,
                                                                 const std::string& value,
                                                                 int32_t exclude_record_id) const {
    // 检查唯一性约束
    // 这里需要查询数据库中是否已存在相同值的记录
    // 简化实现：假设有一个唯一性索引可以快速检查

    std::string cache_key = table_name + "." + field_name;

    {
        std::lock_guard<std::mutex> lock(validator_mutex_);
        auto it = unique_value_cache_.find(cache_key);
        if (it != unique_value_cache_.end()) {
            if (it->second.count(value) > 0) {
                // 检查是否是当前记录本身
                if (exclude_record_id != -1) {
                    // 如果是排除的记录ID，则允许
                    return VALID;
                }
                return UNIQUE_VIOLATION;
            }
        }
    }

    // 如果缓存中没有，需要查询数据库
    // 这里是简化实现，实际需要执行数据库查询
    // 假设查询成功，没有重复值

    // 更新缓存
    {
        std::lock_guard<std::mutex> lock(validator_mutex_);
        unique_value_cache_[cache_key].insert(value);
    }

    return VALID;
}

ValidationResult DataIntegrityConstraintValidator::ValidateCheckConstraint(const std::string& field_name,
                                                                          const std::string& value,
                                                                          const std::string& constraint_expr) const {
    std::lock_guard<std::mutex> lock(validator_mutex_);

    // 查找或创建约束函数
    auto it = check_constraint_cache_.find(constraint_expr);
    if (it == check_constraint_cache_.end()) {
        // 解析约束表达式并创建验证函数
        // 这里是简化实现，实际需要解析SQL表达式
        auto constraint_func = [constraint_expr](const std::string& val) -> bool {
            // 简单的示例：检查长度约束
            if (constraint_expr.find("LEN(") != std::string::npos) {
                // 解析 LEN(field) > N 这样的表达式
                // 简化实现：假设约束总是通过
                return true;
            }
            return true;
        };

        check_constraint_cache_[constraint_expr] = constraint_func;
        it = check_constraint_cache_.find(constraint_expr);
    }

    if (it->second(value)) {
        return VALID;
    } else {
        return CHECK_CONSTRAINT_VIOLATION;
    }
}

ValidationResult DataIntegrityConstraintValidator::ValidateForeignKey(const std::string& field_name,
                                                                     const std::string& value,
                                                                     const std::string& foreign_table,
                                                                     const std::string& foreign_column) const {
    // 检查外键约束
    if (ForeignKeyExists(foreign_table, foreign_column, value)) {
        return VALID;
    } else {
        return FOREIGN_KEY_VIOLATION;
    }
}

std::string DataIntegrityConstraintValidator::ApplyDefaultValue(const std::string& field_name,
                                                               const std::string& value,
                                                               const std::string& default_value) const {
    if (value.empty() && !default_value.empty()) {
        return default_value;
    }

    return value;
}

void DataIntegrityConstraintValidator::AddValidationRule(const std::string& table_name,
                                                        const FieldValidationRule& rule) {
    std::lock_guard<std::mutex> lock(validator_mutex_);
    constraint_rules_[table_name].push_back(rule);
}

void DataIntegrityConstraintValidator::RemoveValidationRule(const std::string& table_name,
                                                           const std::string& field_name) {
    std::lock_guard<std::mutex> lock(validator_mutex_);

    auto table_it = constraint_rules_.find(table_name);
    if (table_it != constraint_rules_.end()) {
        auto& rules = table_it->second;
        rules.erase(std::remove_if(rules.begin(), rules.end(),
                                  [field_name](const FieldValidationRule& rule) {
                                      return rule.field_name == field_name;
                                  }), rules.end());
    }
}

const std::vector<FieldValidationRule>* DataIntegrityConstraintValidator::GetValidationRules(
    const std::string& table_name) const {
    std::lock_guard<std::mutex> lock(validator_mutex_);

    auto it = constraint_rules_.find(table_name);
    if (it != constraint_rules_.end()) {
        return &it->second;
    }

    return nullptr;
}

ValidationResult DataIntegrityConstraintValidator::ValidateRecordConstraints(
    const std::string& table_name,
    const std::vector<std::string>& field_names,
    const std::vector<std::string>& values,
    int32_t record_id) const {

    const std::vector<FieldValidationRule>* rules = GetValidationRules(table_name);
    if (!rules) {
        return VALID; // 没有约束规则
    }

    for (size_t i = 0; i < field_names.size() && i < values.size() && i < rules->size(); ++i) {
        const FieldValidationRule& rule = (*rules)[i];
        const std::string& field_name = field_names[i];
        const std::string& value = values[i];

        // 空值约束
        ValidationResult null_result = ValidateNotNull(field_name, value, rule.nullable);
        if (null_result != VALID) {
            return null_result;
        }

        // 唯一性约束
        if (rule.unique) {
            ValidationResult unique_result = ValidateUnique(table_name, field_name, value, record_id);
            if (unique_result != VALID) {
                return unique_result;
            }
        }

        // 检查约束
        if (!rule.check_constraint.empty()) {
            ValidationResult check_result = ValidateCheckConstraint(field_name, value, rule.check_constraint);
            if (check_result != VALID) {
                return check_result;
            }
        }

        // 外键约束
        if (!rule.foreign_key_table.empty() && !rule.foreign_key_column.empty()) {
            ValidationResult fk_result = ValidateForeignKey(field_name, value,
                                                          rule.foreign_key_table, rule.foreign_key_column);
            if (fk_result != VALID) {
                return fk_result;
            }
        }

        // 自定义验证
        if (rule.custom_validator) {
            if (!rule.custom_validator(value)) {
                return INVALID_VALUE;
            }
        }
    }

    return VALID;
}

// 私有辅助方法实现
bool DataIntegrityConstraintValidator::EvaluateCheckConstraint(const std::string& constraint_expr,
                                                             const std::string& value) const {
    // 简化实现：解析简单的约束表达式
    // 实际实现应该使用完整的SQL表达式解析器

    // 示例：LEN(field) > 5
    if (constraint_expr.find("LEN(") != std::string::npos) {
        // 解析 LEN(field) > N 这样的表达式
        // 简化实现：假设约束总是通过
        return true;
    }
    return true;
}

bool DataIntegrityConstraintValidator::ForeignKeyExists(const std::string& foreign_table,
                                                      const std::string& foreign_column,
                                                      const std::string& value) const {
    // 简化实现：假设外键总是存在
    // 实际实现应该查询外键表
    return true;
}

void DataIntegrityConstraintValidator::UpdateUniqueValueCache(const std::string& table_name,
                                                            const std::string& field_name,
                                                            const std::string& old_value,
                                                            const std::string& new_value) {
    std::string cache_key = table_name + "." + field_name;

    if (!old_value.empty()) {
        unique_value_cache_[cache_key].erase(old_value);
    }

    if (!new_value.empty()) {
        unique_value_cache_[cache_key].insert(new_value);
    }
}

// 并发访问控制验证器实现
ConcurrentAccessControlValidator::ConcurrentAccessControlValidator(
    std::shared_ptr<TransactionManager> transaction_manager)
    : transaction_manager_(std::move(transaction_manager)) {
}

ValidationResult ConcurrentAccessControlValidator::ValidateIsolationLevel(int32_t transaction_id) const {
    // 简化实现：假设隔离级别总是有效的
    return VALID;
}

ValidationResult ConcurrentAccessControlValidator::ValidateConcurrentAccess(
    const std::string& table_name, int32_t record_id, int32_t transaction_id, bool is_write) const {

    // 简化实现：假设并发访问总是允许的
    // 实际实现应该检查锁状态和事务冲突
    return VALID;
}

ValidationResult ConcurrentAccessControlValidator::ValidateLockWait(int32_t transaction_id,
                                                                   std::chrono::milliseconds max_wait_time) const {
    // 简化实现：假设锁等待总是有效的
    return VALID;
}

ValidationResult ConcurrentAccessControlValidator::ValidateDeadlockFreedom(int32_t transaction_id) const {
    // 简化实现：假设没有死锁
    return VALID;
}

ValidationResult ConcurrentAccessControlValidator::ValidateMVCCVersion(
    const std::string& table_name, int32_t record_id, int32_t transaction_id, uint64_t expected_version) const {

    // 简化实现：假设版本总是匹配的
    return VALID;
}

ValidationResult ConcurrentAccessControlValidator::ValidateOptimisticLock(
    const std::string& table_name, int32_t record_id, const std::string& version_field,
    const std::string& expected_version) const {

    // 简化实现：假设乐观锁总是成功的
    return VALID;
}

// 记录边界验证器主类实现
RecordBoundaryValidator::RecordBoundaryValidator(std::shared_ptr<StorageEngine> storage_engine,
                                               std::shared_ptr<TransactionManager> transaction_manager)
    : storage_engine_(std::move(storage_engine)),
      transaction_manager_(std::move(transaction_manager)),
      constraint_validator_(storage_engine_),  // 正确调用DataIntegrityConstraintValidator构造函数
      access_validator_(transaction_manager_) {

    // 初始化统计信息
    stats_.last_validation_time = std::chrono::steady_clock::now();
}

ValidationResult RecordBoundaryValidator::ValidateRecord(const std::string& table_name,
                                                       const std::vector<std::string>& field_names,
                                                       const std::vector<std::string>& field_types,
                                                       const std::vector<std::string>& values,
                                                       int32_t transaction_id,
                                                       int32_t record_id) const {

    auto start_time = std::chrono::steady_clock::now();

    ValidationResult result = VALID;

    // 1. 大小验证
    result = PerformSizeValidation(values, GetTableMetadata(table_name));
    if (result != VALID && config_.strict_mode) {
        UpdateValidationStats(result, std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now() - start_time));
        return result;
    }

    // 2. 类型验证
    result = PerformTypeValidation(field_names, field_types, values, {});
    if (result != VALID && config_.strict_mode) {
        UpdateValidationStats(result, std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now() - start_time));
        return result;
    }

    // 3. 约束验证
    if (config_.enable_constraint_checking) {
        result = PerformConstraintValidation(table_name, field_names, values, record_id);
        if (result != VALID && config_.strict_mode) {
            UpdateValidationStats(result, std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now() - start_time));
            return result;
        }
    }

    // 4. 并发验证
    result = PerformConcurrencyValidation(table_name, record_id, transaction_id, false);
    if (result != VALID && config_.strict_mode) {
        UpdateValidationStats(result, std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now() - start_time));
        return result;
    }

    UpdateValidationStats(VALID, std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now() - start_time));

    return VALID;
}

ValidationResult RecordBoundaryValidator::ValidateRecordUpdate(const std::string& table_name,
                                                             const std::vector<std::string>& field_names,
                                                             const std::vector<std::string>& old_values,
                                                             const std::vector<std::string>& new_values,
                                                             int32_t transaction_id,
                                                             int32_t record_id) const {

    // 对于更新操作，我们需要验证新值，但也可能需要检查某些业务规则
    return ValidateRecord(table_name, field_names, {}, new_values, transaction_id, record_id);
}

ValidationResult RecordBoundaryValidator::ValidateRecordDeletion(const std::string& table_name,
                                                               int32_t record_id,
                                                               int32_t transaction_id) const {

    // 删除操作主要关注并发控制
    return PerformConcurrencyValidation(table_name, record_id, transaction_id, true);
}

std::vector<ValidationResult> RecordBoundaryValidator::ValidateRecords(const std::string& table_name,
                                                                     const std::vector<std::vector<std::string>>& record_batch,
                                                                     int32_t transaction_id) const {

    std::vector<ValidationResult> results;

    for (const auto& record : record_batch) {
        // 简化实现：假设所有记录都有相同的字段结构
        std::vector<std::string> field_names;
        std::vector<std::string> field_types;

        auto metadata = GetTableMetadata(table_name);
        if (metadata) {
            for (const auto& column : metadata->columns) {
                field_names.push_back(column.name);
                field_types.push_back(column.type);
            }
        }

        ValidationResult result = ValidateRecord(table_name, field_names, field_types,
                                               record, transaction_id, -1);
        results.push_back(result);
    }

    return results;
}

void RecordBoundaryValidator::SetValidationConfig(const RecordValidationConfig& config) {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    config_ = config;
}

const RecordValidationConfig& RecordBoundaryValidator::GetValidationConfig() const {
    return config_;
}

RecordBoundaryValidator::ValidationStats RecordBoundaryValidator::GetValidationStats() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    return stats_;
}

void RecordBoundaryValidator::AddFieldValidationRule(const std::string& table_name,
                                                   const FieldValidationRule& rule) {
    constraint_validator_.AddValidationRule(table_name, rule);
}

void RecordBoundaryValidator::RemoveFieldValidationRule(const std::string& table_name,
                                                      const std::string& field_name) {
    constraint_validator_.RemoveValidationRule(table_name, field_name);
}

void RecordBoundaryValidator::ClearValidationRules(const std::string& table_name) {
    // 简化实现：重新创建空的规则列表
    // 实际实现应该在DataIntegrityConstraintValidator中添加Clear方法
}

// 私有辅助方法实现
ValidationResult RecordBoundaryValidator::PerformSizeValidation(const std::vector<std::string>& values,
                                                              const std::shared_ptr<TableMetadata>& metadata) const {
    if (!metadata) {
        return INVALID_VALUE;
    }

    // 计算记录大小
    size_t record_size = size_validator_.CalculateRecordSize({}, metadata);

    return size_validator_.ValidateRecordSize(record_size, config_.max_record_size);
}

ValidationResult RecordBoundaryValidator::PerformTypeValidation(
    const std::vector<std::string>& field_names,
    const std::vector<std::string>& field_types,
    const std::vector<std::string>& values,
    const std::vector<FieldValidationRule>& rules) const {

    auto results = type_validator_.ValidateFields(field_names, field_types, values, rules);

    // 返回第一个失败的结果
    for (ValidationResult result : results) {
        if (result != VALID) {
            return result;
        }
    }

    return VALID;
}

ValidationResult RecordBoundaryValidator::PerformConstraintValidation(
    const std::string& table_name,
    const std::vector<std::string>& field_names,
    const std::vector<std::string>& values,
    int32_t record_id) const {

    return constraint_validator_.ValidateRecordConstraints(table_name, field_names, values, record_id);
}

ValidationResult RecordBoundaryValidator::PerformConcurrencyValidation(
    const std::string& table_name,
    int32_t record_id,
    int32_t transaction_id,
    bool is_write) const {

    return access_validator_.ValidateConcurrentAccess(table_name, record_id, transaction_id, is_write);
}

void RecordBoundaryValidator::UpdateValidationStats(ValidationResult result,
                                                  std::chrono::microseconds duration) const {
    std::lock_guard<std::mutex> lock(stats_mutex_);

    stats_.total_validations++;
    validation_times_.push_back(duration.count());

    if (result == VALID) {
        stats_.successful_validations++;
    } else {
        stats_.failed_validations++;

        // 分类统计失败原因
        switch (result) {
            case INVALID_SIZE:
                stats_.size_validation_failures++;
                break;
            case INVALID_TYPE:
            case INVALID_VALUE:
            case INVALID_FORMAT:
            case ENCODING_ERROR:
                stats_.type_validation_failures++;
                break;
            case NULL_VIOLATION:
            case UNIQUE_VIOLATION:
            case FOREIGN_KEY_VIOLATION:
            case CHECK_CONSTRAINT_VIOLATION:
                stats_.constraint_violations++;
                break;
            default:
                stats_.concurrency_conflicts++;
                break;
        }
    }

    // 计算平均验证时间
    if (!validation_times_.empty()) {
        double sum = 0.0;
        for (double time : validation_times_) {
            sum += time;
        }
        stats_.average_validation_time_us = sum / validation_times_.size();
    }

    // 限制时间记录数量
    if (validation_times_.size() > 1000) {
        validation_times_.erase(validation_times_.begin());
    }

    stats_.last_validation_time = std::chrono::steady_clock::now();
}

std::shared_ptr<TableMetadata> RecordBoundaryValidator::GetTableMetadata(const std::string& table_name) const {
    // 简化实现：假设有一个全局的表元数据管理器
    // 实际实现应该通过StorageEngine获取
    return nullptr;
}

// 验证器工厂实现
std::shared_ptr<RecordBoundaryValidator> RecordBoundaryValidatorFactory::CreateBasicValidator(
    std::shared_ptr<StorageEngine> storage_engine) {

    return std::make_shared<RecordBoundaryValidator>(storage_engine);
}

std::shared_ptr<RecordBoundaryValidator> RecordBoundaryValidatorFactory::CreateStrictValidator(
    std::shared_ptr<StorageEngine> storage_engine,
    std::shared_ptr<TransactionManager> transaction_manager) {

    auto validator = std::make_shared<RecordBoundaryValidator>(storage_engine, transaction_manager);

    RecordValidationConfig config;
    config.strict_mode = true;
    config.enable_constraint_checking = true;
    config.enable_foreign_key_checking = true;
    config.enable_format_validation = true;
    config.enable_encoding_validation = true;

    validator->SetValidationConfig(config);

    return validator;
}

std::shared_ptr<RecordBoundaryValidator> RecordBoundaryValidatorFactory::CreateEnterpriseValidator(
    std::shared_ptr<StorageEngine> storage_engine,
    std::shared_ptr<TransactionManager> transaction_manager,
    const RecordValidationConfig& config) {

    auto validator = std::make_shared<RecordBoundaryValidator>(storage_engine, transaction_manager);
    validator->SetValidationConfig(config);

    return validator;
}

// 验证结果格式化器实现
std::string ValidationResultFormatter::FormatResult(ValidationResult result) {
    switch (result) {
        case VALID: return "VALID";
        case INVALID_SIZE: return "INVALID_SIZE";
        case INVALID_TYPE: return "INVALID_TYPE";
        case INVALID_VALUE: return "INVALID_VALUE";
        case NULL_VIOLATION: return "NULL_VIOLATION";
        case UNIQUE_VIOLATION: return "UNIQUE_VIOLATION";
        case FOREIGN_KEY_VIOLATION: return "FOREIGN_KEY_VIOLATION";
        case CHECK_CONSTRAINT_VIOLATION: return "CHECK_CONSTRAINT_VIOLATION";
        case INVALID_FORMAT: return "INVALID_FORMAT";
        case ENCODING_ERROR: return "ENCODING_ERROR";
        case BOUNDARY_VIOLATION: return "BOUNDARY_VIOLATION";
        default: return "UNKNOWN";
    }
}

std::string ValidationResultFormatter::FormatDetailedResult(ValidationResult result,
                                                          const std::string& field_name,
                                                          const std::string& details) {
    std::string message = FormatResult(result);

    if (!field_name.empty()) {
        message += " in field '" + field_name + "'";
    }

    if (!details.empty()) {
        message += ": " + details;
    }

    return message;
}

std::string ValidationResultFormatter::GetResultSeverity(ValidationResult result) {
    switch (result) {
        case VALID:
            return "INFO";
        case INVALID_SIZE:
        case INVALID_TYPE:
        case INVALID_VALUE:
        case INVALID_FORMAT:
        case ENCODING_ERROR:
        case BOUNDARY_VIOLATION:
            return "WARNING";
        case NULL_VIOLATION:
        case UNIQUE_VIOLATION:
        case FOREIGN_KEY_VIOLATION:
        case CHECK_CONSTRAINT_VIOLATION:
            return "ERROR";
        default:
            return "UNKNOWN";
    }
}

bool ValidationResultFormatter::IsCriticalError(ValidationResult result) {
    return result == NULL_VIOLATION || result == UNIQUE_VIOLATION ||
           result == FOREIGN_KEY_VIOLATION || result == CHECK_CONSTRAINT_VIOLATION;
}

} // namespace sqlcc
}

bool DataIntegrityConstraintValidator::ForeignKeyExists(const std::string& foreign_table,
                                                      const std::string& foreign_column,
                                                      const std::string& value) const {
    // 简化实现：假设外键总是存在
    // 实际实现应该查询外键表
    return true;
}

void DataIntegrityConstraintValidator::UpdateUniqueValueCache(const std::string& table_name,
                                                            const std::string& field_name,
                                                            const std::string& old_value,
                                                            const std::string& new_value) {
    std::string cache_key = table_name + "." + field_name;

    if (!old_value.empty()) {
        unique_value_cache_[cache_key].erase(old_value);
    }

    if (!new_value.empty()) {
        unique_value_cache_[cache_key].insert(new_value);
    }
}

// 并发访问控制验证器实现
ConcurrentAccessControlValidator::ConcurrentAccessControlValidator(
    std::shared_ptr<TransactionManager> transaction_manager)
    : transaction_manager_(std::move(transaction_manager)) {
}

ValidationResult ConcurrentAccessControlValidator::ValidateIsolationLevel(int32_t transaction_id) const {
    // 简化实现：假设隔离级别总是有效的
    return VALID;
}

ValidationResult ConcurrentAccessControlValidator::ValidateConcurrentAccess(
    const std::string& table_name, int32_t record_id, int32_t transaction_id, bool is_write) const {

    // 简化实现：假设并发访问总是允许的
    // 实际实现应该检查锁状态和事务冲突
    return VALID;
}

ValidationResult ConcurrentAccessControlValidator::ValidateLockWait(int32_t transaction_id,
                                                                   std::chrono::milliseconds max_wait_time) const {
    // 简化实现：假设锁等待总是有效的
    return VALID;
}

ValidationResult ConcurrentAccessControlValidator::ValidateDeadlockFreedom(int32_t transaction_id) const {
    // 简化实现：假设没有死锁
    return VALID;
}

ValidationResult ConcurrentAccessControlValidator::ValidateMVCCVersion(
    const std::string& table_name, int32_t record_id, int32_t transaction_id, uint64_t expected_version) const {

    // 简化实现：假设版本总是匹配的
    return VALID;
}

ValidationResult ConcurrentAccessControlValidator::ValidateOptimisticLock(
    const std::string& table_name, int32_t record_id, const std::string& version_field,
    const std::string& expected_version) const {

    // 简化实现：假设乐观锁总是成功的
    return VALID;
}

// 记录边界验证器主类实现
RecordBoundaryValidator::RecordBoundaryValidator(std::shared_ptr<StorageEngine> storage_engine,
                                               std::shared_ptr<TransactionManager> transaction_manager)
    : storage_engine_(std::move(storage_engine)),
      transaction_manager_(std::move(transaction_manager)) {

    // 初始化统计信息
    stats_.last_validation_time = std::chrono::steady_clock::now();
}

ValidationResult RecordBoundaryValidator::ValidateRecord(const std::string& table_name,
                                                       const std::vector<std::string>& field_names,
                                                       const std::vector<std::string>& field_types,
                                                       const std::vector<std::string>& values,
                                                       int32_t transaction_id,
                                                       int32_t record_id) const {

    auto start_time = std::chrono::steady_clock::now();

    ValidationResult result = VALID;

    // 1. 大小验证
    result = PerformSizeValidation(values, GetTableMetadata(table_name));
    if (result != VALID && config_.strict_mode) {
        UpdateValidationStats(result, std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now() - start_time));
        return result;
    }

    // 2. 类型验证
    result = PerformTypeValidation(field_names, field_types, values, {});
    if (result != VALID && config_.strict_mode) {
        UpdateValidationStats(result, std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now() - start_time));
        return result;
    }

    // 3. 约束验证
    if (config_.enable_constraint_checking) {
        result = PerformConstraintValidation(table_name, field_names, values, record_id);
        if (result != VALID && config_.strict_mode) {
            UpdateValidationStats(result, std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now() - start_time));
            return result;
        }
    }

    // 4. 并发验证
    result = PerformConcurrencyValidation(table_name, record_id, transaction_id, false);
    if (result != VALID && config_.strict_mode) {
        UpdateValidationStats(result, std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now() - start_time));
        return result;
    }

    UpdateValidationStats(VALID, std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now() - start_time));

    return VALID;
}

ValidationResult RecordBoundaryValidator::ValidateRecordUpdate(const std::string& table_name,
                                                             const std::vector<std::string>& field_names,
                                                             const std::vector<std::string>& old_values,
                                                             const std::vector<std::string>& new_values,
                                                             int32_t transaction_id,
                                                             int32_t record_id) const {

    // 对于更新操作，我们需要验证新值，但也可能需要检查某些业务规则
    return ValidateRecord(table_name, field_names, {}, new_values, transaction_id, record_id);
}

ValidationResult RecordBoundaryValidator::ValidateRecordDeletion(const std::string& table_name,
                                                               int32_t record_id,
                                                               int32_t transaction_id) const {

    // 删除操作主要关注并发控制
    return PerformConcurrencyValidation(table_name, record_id, transaction_id, true);
}

std::vector<ValidationResult> RecordBoundaryValidator::ValidateRecords(const std::string& table_name,
                                                                     const std::vector<std::vector<std::string>>& record_batch,
                                                                     int32_t transaction_id) const {

    std::vector<ValidationResult> results;

    for (const auto& record : record_batch) {
        // 简化实现：假设所有记录都有相同的字段结构
        std::vector<std::string> field_names;
        std::vector<std::string> field_types;

        auto metadata = GetTableMetadata(table_name);
        if (metadata) {
            for (const auto& column : metadata->columns) {
                field_names.push_back(column.name);
                field_types.push_back(column.type);
            }
        }

        ValidationResult result = ValidateRecord(table_name, field_names, field_types,
                                               record, transaction_id, -1);
        results.push_back(result);
    }

    return results;
}

void RecordBoundaryValidator::SetValidationConfig(const RecordValidationConfig& config) {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    config_ = config;
}

const RecordValidationConfig& RecordBoundaryValidator::GetValidationConfig() const {
    return config_;
}

RecordBoundaryValidator::ValidationStats RecordBoundaryValidator::GetValidationStats() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    return stats_;
}

void RecordBoundaryValidator::AddFieldValidationRule(const std::string& table_name,
                                                   const FieldValidationRule& rule) {
    constraint_validator_.AddValidationRule(table_name, rule);
}

void RecordBoundaryValidator::RemoveFieldValidationRule(const std::string& table_name,
                                                      const std::string& field_name) {
    constraint_validator_.RemoveValidationRule(table_name, field_name);
}

void RecordBoundaryValidator::ClearValidationRules(const std::string& table_name) {
    // 简化实现：重新创建空的规则列表
    // 实际实现应该在DataIntegrityConstraintValidator中添加Clear方法
}

// 私有辅助方法实现
ValidationResult RecordBoundaryValidator::PerformSizeValidation(const std::vector<std::string>& values,
                                                              const std::shared_ptr<TableMetadata>& metadata) const {
    if (!metadata) {
        return INVALID_VALUE;
    }

    // 计算记录大小
    size_t record_size = size_validator_.CalculateRecordSize({}, metadata);

    return size_validator_.ValidateRecordSize(record_size, config_.max_record_size);
}

ValidationResult RecordBoundaryValidator::PerformTypeValidation(
    const std::vector<std::string>& field_names,
    const std::vector<std::string>& field_types,
    const std::vector<std::string>& values,
    const std::vector<FieldValidationRule>& rules) const {

    auto results = type_validator_.ValidateFields(field_names, field_types, values, rules);

    // 返回第一个失败的结果
    for (ValidationResult result : results) {
        if (result != VALID) {
            return result;
        }
    }

    return VALID;
}

ValidationResult RecordBoundaryValidator::PerformConstraintValidation(
    const std::string& table_name,
    const std::vector<std::string>& field_names,
    const std::vector<std::string>& values,
    int32_t record_id) const {

    return constraint_validator_.ValidateRecordConstraints(table_name, field_names, values, record_id);
}

ValidationResult RecordBoundaryValidator::PerformConcurrencyValidation(
    const std::string& table_name,
    int32_t record_id,
    int32_t transaction_id,
    bool is_write) const {

    return access_validator_.ValidateConcurrentAccess(table_name, record_id, transaction_id, is_write);
}

void RecordBoundaryValidator::UpdateValidationStats(ValidationResult result,
                                                  std::chrono::microseconds duration) const {
    std::lock_guard<std::mutex> lock(stats_mutex_);

    stats_.total_validations++;
    validation_times_.push_back(duration.count());

    if (result == VALID) {
        stats_.successful_validations++;
    } else {
        stats_.failed_validations++;

        // 分类统计失败原因
        switch (result) {
            case INVALID_SIZE:
                stats_.size_validation_failures++;
                break;
            case INVALID_TYPE:
            case INVALID_VALUE:
            case INVALID_FORMAT:
            case ENCODING_ERROR:
                stats_.type_validation_failures++;
                break;
            case NULL_VIOLATION:
            case UNIQUE_VIOLATION:
            case FOREIGN_KEY_VIOLATION:
            case CHECK_CONSTRAINT_VIOLATION:
                stats_.constraint_violations++;
                break;
            default:
                stats_.concurrency_conflicts++;
                break;
        }
    }

    // 计算平均验证时间
    if (!validation_times_.empty()) {
        double sum = 0.0;
        for (double time : validation_times_) {
            sum += time;
        }
        stats_.average_validation_time_us = sum / validation_times_.size();
    }

    // 限制时间记录数量
    if (validation_times_.size() > 1000) {
        validation_times_.erase(validation_times_.begin());
    }

    stats_.last_validation_time = std::chrono::steady_clock::now();
}

std::shared_ptr<TableMetadata> RecordBoundaryValidator::GetTableMetadata(const std::string& table_name) const {
    // 简化实现：假设有一个全局的表元数据管理器
    // 实际实现应该通过StorageEngine获取
    return nullptr;
}

// 验证器工厂实现
std::shared_ptr<RecordBoundaryValidator> RecordBoundaryValidatorFactory::CreateBasicValidator(
    std::shared_ptr<StorageEngine> storage_engine) {

    return std::make_shared<RecordBoundaryValidator>(storage_engine);
}

std::shared_ptr<RecordBoundaryValidator> RecordBoundaryValidatorFactory::CreateStrictValidator(
    std::shared_ptr<StorageEngine> storage_engine,
    std::shared_ptr<TransactionManager> transaction_manager) {

    auto validator = std::make_shared<RecordBoundaryValidator>(storage_engine, transaction_manager);

    RecordValidationConfig config;
    config.strict_mode = true;
    config.enable_constraint_checking = true;
    config.enable_foreign_key_checking = true;
    config.enable_format_validation = true;
    config.enable_encoding_validation = true;

    validator->SetValidationConfig(config);

    return validator;
}

std::shared_ptr<RecordBoundaryValidator> RecordBoundaryValidatorFactory::CreateEnterpriseValidator(
    std::shared_ptr<StorageEngine> storage_engine,
    std::shared_ptr<TransactionManager> transaction_manager,
    const RecordValidationConfig& config) {

    auto validator = std::make_shared<RecordBoundaryValidator>(storage_engine, transaction_manager);
    validator->SetValidationConfig(config);

    return validator;
}

// 验证结果格式化器实现
std::string ValidationResultFormatter::FormatResult(ValidationResult result) {
    switch (result) {
        case VALID: return "VALID";
        case INVALID_SIZE: return "INVALID_SIZE";
        case INVALID_TYPE: return "INVALID_TYPE";
        case INVALID_VALUE: return "INVALID_VALUE";
        case NULL_VIOLATION: return "NULL_VIOLATION";
        case UNIQUE_VIOLATION: return "UNIQUE_VIOLATION";
        case FOREIGN_KEY_VIOLATION: return "FOREIGN_KEY_VIOLATION";
        case CHECK_CONSTRAINT_VIOLATION: return "CHECK_CONSTRAINT_VIOLATION";
        case INVALID_FORMAT: return "INVALID_FORMAT";
        case ENCODING_ERROR: return "ENCODING_ERROR";
        case BOUNDARY_VIOLATION: return "BOUNDARY_VIOLATION";
        default: return "UNKNOWN";
    }
}

std::string ValidationResultFormatter::FormatDetailedResult(ValidationResult result,
                                                          const std::string& field_name,
                                                          const std::string& details) {
    std::string message = FormatResult(result);

    if (!field_name.empty()) {
        message += " in field '" + field_name + "'";
    }

    if (!details.empty()) {
        message += ": " + details;
    }

    return message;
}

std::string ValidationResultFormatter::GetResultSeverity(ValidationResult result) {
    switch (result) {
        case VALID:
            return "INFO";
        case INVALID_SIZE:
        case INVALID_TYPE:
        case INVALID_VALUE:
        case INVALID_FORMAT:
        case ENCODING_ERROR:
        case BOUNDARY_VIOLATION:
            return "WARNING";
        case NULL_VIOLATION:
        case UNIQUE_VIOLATION:
        case FOREIGN_KEY_VIOLATION:
        case CHECK_CONSTRAINT_VIOLATION:
            return "ERROR";
        default:
            return "UNKNOWN";
    }
}

bool ValidationResultFormatter::IsCriticalError(ValidationResult result) {
    return result == NULL_VIOLATION || result == UNIQUE_VIOLATION ||
           result == FOREIGN_KEY_VIOLATION || result == CHECK_CONSTRAINT_VIOLATION;
}

} // namespace sqlcc
