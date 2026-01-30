#include "src/storage_engine/table_storage/record_validator.h"
#include "src/utils/logger.h"
#include <limits>
#include <cmath>

namespace sqlcc {
namespace storage_engine {
namespace table_storage {

bool RecordValidator::ValidateRecordSize(size_t record_size, size_t max_record_size) {
    if (record_size == 0) {
        SQLCC_LOG_ERROR("Record size cannot be zero");
        return false;
    }
    if (record_size > max_record_size) {
        SQLCC_LOG_ERROR("Record size " + std::to_string(record_size) +
                       " exceeds maximum allowed size " + std::to_string(max_record_size));
        return false;
    }
    return true;
}

bool RecordValidator::ValidateFieldValue(const std::string& field_name,
                                        const std::string& field_type,
                                        const std::string& value) {
    if (field_name.empty()) {
        SQLCC_LOG_ERROR("Field name cannot be empty");
        return false;
    }

    // 根据字段类型验证值
    if (field_type == "INT" || field_type == "INTEGER") {
        try {
            // 验证整数值范围
            long long int_value = std::stoll(value);
            if (int_value < INT32_MIN || int_value > INT32_MAX) {
                SQLCC_LOG_ERROR("Integer value " + value + " out of range for field " + field_name);
                return false;
            }
        } catch (const std::exception& e) {
            SQLCC_LOG_ERROR("Invalid integer value '" + value + "' for field " + field_name + ": " + e.what());
            return false;
        }
    } else if (field_type == "BIGINT") {
        try {
            // 验证长整数值范围
            long long bigint_value = std::stoll(value);
            if (bigint_value < INT64_MIN || bigint_value > INT64_MAX) {
                SQLCC_LOG_ERROR("Bigint value " + value + " out of range for field " + field_name);
                return false;
            }
        } catch (const std::exception& e) {
            SQLCC_LOG_ERROR("Invalid bigint value '" + value + "' for field " + field_name + ": " + e.what());
            return false;
        }
    } else if (field_type == "FLOAT" || field_type == "DOUBLE") {
        try {
            // 验证浮点数值
            double double_value = std::stod(value);
            if (std::isnan(double_value) || std::isinf(double_value)) {
                SQLCC_LOG_ERROR("Invalid floating point value '" + value + "' for field " + field_name);
                return false;
            }
        } catch (const std::exception& e) {
            SQLCC_LOG_ERROR("Invalid floating point value '" + value + "' for field " + field_name + ": " + e.what());
            return false;
        }
    } else if (field_type == "VARCHAR" || field_type == "TEXT") {
        // 验证变长字段
        if (value.length() > 65535) {
            SQLCC_LOG_ERROR("VARCHAR/TEXT value length " + std::to_string(value.length()) +
                           " exceeds maximum for field " + field_name);
            return false;
        }
    }

    // 检查空字符和不可打印字符
    for (size_t i = 0; i < value.length(); ++i) {
        char c = value[i];
        if (c == '\0') {
            SQLCC_LOG_ERROR("Null character not allowed in field " + field_name);
            return false;
        }
        // 可以添加更多不可打印字符的检查
    }

    return true;
}

bool RecordValidator::ValidateDataIntegrity(const std::vector<std::string>& field_names,
                                          const std::vector<std::string>& field_types,
                                          const std::vector<std::string>& values) {
    if (field_names.size() != field_types.size() || field_types.size() != values.size()) {
        SQLCC_LOG_ERROR("Field count mismatch: names=" + std::to_string(field_names.size()) +
                       ", types=" + std::to_string(field_types.size()) +
                       ", values=" + std::to_string(values.size()));
        return false;
    }

    // 验证每个字段
    for (size_t i = 0; i < values.size(); ++i) {
        if (!ValidateFieldValue(field_names[i], field_types[i], values[i])) {
            return false;
        }
    }

    return true;
}

} // namespace table_storage
} // namespace storage_engine
} // namespace sqlcc
