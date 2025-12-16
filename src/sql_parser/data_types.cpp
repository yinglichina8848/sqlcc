#include "sql_parser/data_types.h"
#include <algorithm>
#include <limits>
#include <ctime>
#include <iomanip>

namespace sqlcc {
namespace sql_parser {

// ==================== DecimalValue 实现 ====================

DecimalValue::DecimalValue() : value_(0), precision_(18), scale_(0) {}

DecimalValue::DecimalValue(int64_t value, int precision, int scale)
    : value_(value), precision_(precision), scale_(scale) {
    normalize();
}

DecimalValue::DecimalValue(const std::string& str)
    : value_(0), precision_(18), scale_(0) {
    // 解析字符串格式，如 "123.45" 或 "123.4500"
    size_t dot_pos = str.find('.');
    if (dot_pos != std::string::npos) {
        std::string int_part = str.substr(0, dot_pos);
        std::string frac_part = str.substr(dot_pos + 1);

        // 计算小数位数
        scale_ = frac_part.length();

        // 将字符串转换为整数值
        value_ = stringToInt64(str, scale_);
    } else {
        value_ = stringToInt64(str, 0);
        scale_ = 0;
    }

    precision_ = str.length();
    if (dot_pos != std::string::npos) {
        precision_--;  // 减去小数点
    }
    normalize();
}

DecimalValue::DecimalValue(double value, int precision, int scale)
    : precision_(precision), scale_(scale) {
    // 将double转换为内部表示
    double multiplier = std::pow(10.0, scale);
    value_ = static_cast<int64_t>(value * multiplier + 0.5);  // 四舍五入
    normalize();
}

DecimalValue DecimalValue::operator+(const DecimalValue& other) const {
    // 对齐小数位
    int max_scale = std::max(scale_, other.scale_);
    int64_t left_val = value_ * static_cast<int64_t>(std::pow(10, max_scale - scale_));
    int64_t right_val = other.value_ * static_cast<int64_t>(std::pow(10, max_scale - other.scale_));

    return DecimalValue(left_val + right_val, std::max(precision_, other.precision_), max_scale);
}

DecimalValue DecimalValue::operator-(const DecimalValue& other) const {
    int max_scale = std::max(scale_, other.scale_);
    int64_t left_val = value_ * static_cast<int64_t>(std::pow(10, max_scale - scale_));
    int64_t right_val = other.value_ * static_cast<int64_t>(std::pow(10, max_scale - other.scale_));

    return DecimalValue(left_val - right_val, std::max(precision_, other.precision_), max_scale);
}

DecimalValue DecimalValue::operator*(const DecimalValue& other) const {
    int64_t result_val = value_ * other.value_;
    int result_scale = scale_ + other.scale_;
    int result_precision = precision_ + other.precision_;

    return DecimalValue(result_val, result_precision, result_scale);
}

DecimalValue DecimalValue::operator/(const DecimalValue& other) const {
    if (other.value_ == 0) {
        throw std::runtime_error("Division by zero in DecimalValue");
    }

    // 扩大被除数以保持精度
    int64_t dividend = value_ * static_cast<int64_t>(std::pow(10, other.scale_ + scale_));
    int64_t result_val = dividend / other.value_;
    int result_scale = scale_ + other.scale_;
    int result_precision = precision_ + other.precision_;

    return DecimalValue(result_val, result_precision, result_scale);
}

bool DecimalValue::operator==(const DecimalValue& other) const {
    // 对齐小数位后比较
    int max_scale = std::max(scale_, other.scale_);
    int64_t left_val = value_ * static_cast<int64_t>(std::pow(10, max_scale - scale_));
    int64_t right_val = other.value_ * static_cast<int64_t>(std::pow(10, max_scale - other.scale_));

    return left_val == right_val;
}

bool DecimalValue::operator<(const DecimalValue& other) const {
    int max_scale = std::max(scale_, other.scale_);
    int64_t left_val = value_ * static_cast<int64_t>(std::pow(10, max_scale - scale_));
    int64_t right_val = other.value_ * static_cast<int64_t>(std::pow(10, max_scale - other.scale_));

    return left_val < right_val;
}

std::string DecimalValue::toString() const {
    if (scale_ == 0) {
        return std::to_string(value_);
    }

    std::string str = std::to_string(value_);
    size_t len = str.length();

    if (len <= static_cast<size_t>(scale_)) {
        // 需要在前面补0
        str = std::string(scale_ - len + 1, '0') + str;
        len = str.length();
    }

    // 插入小数点
    str.insert(len - scale_, 1, '.');
    return str;
}

double DecimalValue::toDouble() const {
    return static_cast<double>(value_) / std::pow(10.0, scale_);
}

int64_t DecimalValue::toInt64() const {
    return value_ / static_cast<int64_t>(std::pow(10, scale_));
}

void DecimalValue::normalize() {
    // 确保精度不小于小数位数
    if (precision_ < scale_) {
        precision_ = scale_;
    }

    // 检查溢出
    int64_t max_val = static_cast<int64_t>(std::pow(10, precision_)) - 1;
    if (std::abs(value_) > max_val) {
        throw std::runtime_error("Decimal value exceeds precision limit");
    }
}

int64_t DecimalValue::stringToInt64(const std::string& str, int scale) {
    std::string clean_str = str;
    // 移除小数点
    size_t dot_pos = clean_str.find('.');
    if (dot_pos != std::string::npos) {
        clean_str.erase(dot_pos, 1);
    }

    // 移除前导零
    size_t start = clean_str.find_first_not_of('0');
    if (start != std::string::npos) {
        clean_str = clean_str.substr(start);
    } else {
        clean_str = "0";
    }

    // 如果有小数位，需要补齐
    while (static_cast<int>(clean_str.length()) < scale + 1) {
        clean_str = "0" + clean_str;
    }

    return std::stoll(clean_str);
}

// 其他比较运算符
bool DecimalValue::operator!=(const DecimalValue& other) const { return !(*this == other); }
bool DecimalValue::operator<=(const DecimalValue& other) const { return !(other < *this); }
bool DecimalValue::operator>(const DecimalValue& other) const { return other < *this; }
bool DecimalValue::operator>=(const DecimalValue& other) const { return !(other > *this); }

// ==================== DateTimeValue 实现 ====================

DateTimeValue::DateTimeValue() : time_point_(std::chrono::system_clock::now()), format_(Format::TIMESTAMP) {}

DateTimeValue::DateTimeValue(const std::string& str, Format format)
    : format_(format) {
    time_point_ = parseString(str, format);
}

DateTimeValue::DateTimeValue(std::chrono::system_clock::time_point tp)
    : time_point_(tp), format_(Format::TIMESTAMP) {}

DateTimeValue DateTimeValue::operator+(const DateTimeValue& other) const {
    auto duration1 = time_point_.time_since_epoch();
    auto duration2 = other.time_point_.time_since_epoch();
    return DateTimeValue(std::chrono::system_clock::time_point(duration1 + duration2));
}

DateTimeValue DateTimeValue::operator-(const DateTimeValue& other) const {
    auto duration1 = time_point_.time_since_epoch();
    auto duration2 = other.time_point_.time_since_epoch();
    return DateTimeValue(std::chrono::system_clock::time_point(duration1 - duration2));
}

bool DateTimeValue::operator==(const DateTimeValue& other) const {
    return time_point_ == other.time_point_;
}

bool DateTimeValue::operator<(const DateTimeValue& other) const {
    return time_point_ < other.time_point_;
}

std::string DateTimeValue::toString() const {
    return formatTimePoint(time_point_, format_);
}

std::chrono::system_clock::time_point DateTimeValue::toTimePoint() const {
    return time_point_;
}

int DateTimeValue::getYear() const {
    auto time_t = std::chrono::system_clock::to_time_t(time_point_);
    std::tm tm = *std::localtime(&time_t);
    return tm.tm_year + 1900;
}

int DateTimeValue::getMonth() const {
    auto time_t = std::chrono::system_clock::to_time_t(time_point_);
    std::tm tm = *std::localtime(&time_t);
    return tm.tm_mon + 1;
}

int DateTimeValue::getDay() const {
    auto time_t = std::chrono::system_clock::to_time_t(time_point_);
    std::tm tm = *std::localtime(&time_t);
    return tm.tm_mday;
}

int DateTimeValue::getHour() const {
    auto time_t = std::chrono::system_clock::to_time_t(time_point_);
    std::tm tm = *std::localtime(&time_t);
    return tm.tm_hour;
}

int DateTimeValue::getMinute() const {
    auto time_t = std::chrono::system_clock::to_time_t(time_point_);
    std::tm tm = *std::localtime(&time_t);
    return tm.tm_min;
}

int DateTimeValue::getSecond() const {
    auto time_t = std::chrono::system_clock::to_time_t(time_point_);
    std::tm tm = *std::localtime(&time_t);
    return tm.tm_sec;
}

DateTimeValue DateTimeValue::now() {
    return DateTimeValue(std::chrono::system_clock::now());
}

DateTimeValue DateTimeValue::today() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&time_t);
    tm.tm_hour = 0;
    tm.tm_min = 0;
    tm.tm_sec = 0;
    DateTimeValue result(std::chrono::system_clock::from_time_t(std::mktime(&tm)));
    result.setFormat(Format::DATE);
    return result;
}

std::chrono::system_clock::time_point DateTimeValue::parseString(const std::string& str, Format format) {
    std::tm tm = {};
    std::istringstream ss(str);

    switch (format) {
        case Format::DATE: {
            ss >> std::get_time(&tm, "%Y-%m-%d");
            if (ss.fail()) {
                throw std::runtime_error("Invalid date format: " + str);
            }
            break;
        }
        case Format::TIME: {
            ss >> std::get_time(&tm, "%H:%M:%S");
            if (ss.fail()) {
                throw std::runtime_error("Invalid time format: " + str);
            }
            break;
        }
        case Format::TIMESTAMP:
        case Format::DATETIME: {
            ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
            if (ss.fail()) {
                throw std::runtime_error("Invalid timestamp format: " + str);
            }
            break;
        }
    }

    return std::chrono::system_clock::from_time_t(std::mktime(&tm));
}

std::string DateTimeValue::formatTimePoint(const std::chrono::system_clock::time_point& tp, Format format) {
    auto time_t = std::chrono::system_clock::to_time_t(tp);
    std::tm tm = *std::localtime(&time_t);

    std::ostringstream ss;
    switch (format) {
        case Format::DATE:
            ss << std::put_time(&tm, "%Y-%m-%d");
            break;
        case Format::TIME:
            ss << std::put_time(&tm, "%H:%M:%S");
            break;
        case Format::TIMESTAMP:
        case Format::DATETIME:
            ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
            break;
    }

    return ss.str();
}

// 其他比较运算符
bool DateTimeValue::operator!=(const DateTimeValue& other) const { return !(*this == other); }
bool DateTimeValue::operator<=(const DateTimeValue& other) const { return !(other < *this); }
bool DateTimeValue::operator>(const DateTimeValue& other) const { return other < *this; }
bool DateTimeValue::operator>=(const DateTimeValue& other) const { return !(other > *this); }

// ==================== DataValue 实现 ====================

DataValue::DataValue() : type_(DataType::UNKNOWN), is_null_(true) {}

DataValue::DataValue(DataType type) : type_(type), is_null_(true) {}

DataValue::DataValue(int64_t int_val) : type_(DataType::BIGINT), is_null_(false), int_val_(int_val) {}

DataValue::DataValue(double double_val) : type_(DataType::DOUBLE), is_null_(false), double_val_(double_val) {}

DataValue::DataValue(const std::string& str_val)
    : type_(DataType::VARCHAR), is_null_(false), string_val_(std::make_unique<std::string>(str_val)) {}

DataValue::DataValue(bool bool_val) : type_(DataType::BOOLEAN), is_null_(false), bool_val_(bool_val) {}

DataValue::DataValue(const DecimalValue& decimal_val)
    : type_(DataType::DECIMAL), is_null_(false), decimal_val_(std::make_unique<DecimalValue>(decimal_val)) {}

DataValue::DataValue(const DateTimeValue& datetime_val)
    : type_(DataType::TIMESTAMP), is_null_(false), datetime_val_(std::make_unique<DateTimeValue>(datetime_val)) {}

DataValue::DataValue(const DataValue& other) : type_(other.type_), is_null_(other.is_null_) {
    copyFrom(other);
}

DataValue& DataValue::operator=(const DataValue& other) {
    if (this != &other) {
        cleanup();
        type_ = other.type_;
        is_null_ = other.is_null_;
        copyFrom(other);
    }
    return *this;
}

DataValue::DataValue(DataValue&& other) noexcept : type_(other.type_), is_null_(other.is_null_) {
    moveFrom(std::move(other));
}

DataValue& DataValue::operator=(DataValue&& other) noexcept {
    if (this != &other) {
        cleanup();
        type_ = other.type_;
        is_null_ = other.is_null_;
        moveFrom(std::move(other));
    }
    return *this;
}

DataValue::~DataValue() {
    cleanup();
}

int64_t DataValue::asInt64() const {
    if (is_null_ || type_ != DataType::BIGINT) {
        throw std::runtime_error("Cannot convert to int64");
    }
    return int_val_;
}

double DataValue::asDouble() const {
    if (is_null_) {
        throw std::runtime_error("Cannot convert null to double");
    }
    switch (type_) {
        case DataType::DOUBLE:
            return double_val_;
        case DataType::BIGINT:
            return static_cast<double>(int_val_);
        case DataType::DECIMAL:
            return decimal_val_->toDouble();
        default:
            throw std::runtime_error("Cannot convert to double");
    }
}

std::string DataValue::asString() const {
    if (is_null_) {
        return "NULL";
    }
    switch (type_) {
        case DataType::VARCHAR:
        case DataType::CHAR:
        case DataType::TEXT:
            return string_val_ ? *string_val_ : "";
        case DataType::BIGINT:
            return std::to_string(int_val_);
        case DataType::DOUBLE:
            return std::to_string(double_val_);
        case DataType::DECIMAL:
            return decimal_val_ ? decimal_val_->toString() : "0";
        case DataType::TIMESTAMP:
        case DataType::DATE:
        case DataType::TIME:
            return datetime_val_ ? datetime_val_->toString() : "";
        case DataType::BOOLEAN:
            return bool_val_ ? "true" : "false";
        default:
            throw std::runtime_error("Cannot convert to string");
    }
}

bool DataValue::asBool() const {
    if (is_null_ || type_ != DataType::BOOLEAN) {
        throw std::runtime_error("Cannot convert to bool");
    }
    return bool_val_;
}

DecimalValue DataValue::asDecimal() const {
    if (is_null_ || type_ != DataType::DECIMAL) {
        throw std::runtime_error("Cannot convert to decimal");
    }
    return *decimal_val_;
}

DateTimeValue DataValue::asDateTime() const {
    if (is_null_ || (type_ != DataType::TIMESTAMP && type_ != DataType::DATE && type_ != DataType::TIME)) {
        throw std::runtime_error("Cannot convert to datetime");
    }
    return *datetime_val_;
}

void DataValue::setInt64(int64_t value) {
    cleanup();
    type_ = DataType::BIGINT;
    is_null_ = false;
    int_val_ = value;
}

void DataValue::setDouble(double value) {
    cleanup();
    type_ = DataType::DOUBLE;
    is_null_ = false;
    double_val_ = value;
}

void DataValue::setString(const std::string& value) {
    cleanup();
    type_ = DataType::VARCHAR;
    is_null_ = false;
    string_val_ = std::make_unique<std::string>(value);
}

void DataValue::setBool(bool value) {
    cleanup();
    type_ = DataType::BOOLEAN;
    is_null_ = false;
    bool_val_ = value;
}

void DataValue::setDecimal(const DecimalValue& value) {
    cleanup();
    type_ = DataType::DECIMAL;
    is_null_ = false;
    decimal_val_ = std::make_unique<DecimalValue>(value);
}

void DataValue::setDateTime(const DateTimeValue& value) {
    cleanup();
    type_ = DataType::TIMESTAMP;
    is_null_ = false;
    datetime_val_ = std::make_unique<DateTimeValue>(value);
}

// 运算符重载的简化实现
DataValue DataValue::operator+(const DataValue& other) const {
    if (is_null_ || other.is_null_) {
        return DataValue(type_);
    }

    switch (type_) {
        case DataType::BIGINT:
            if (other.type_ == DataType::BIGINT) {
                return DataValue(int_val_ + other.int_val_);
            }
            break;
        case DataType::DOUBLE:
            return DataValue(asDouble() + other.asDouble());
        case DataType::DECIMAL:
            if (other.type_ == DataType::DECIMAL) {
                return DataValue(*decimal_val_ + *other.decimal_val_);
            }
            break;
        default:
            break;
    }
    throw std::runtime_error("Unsupported operation");
}

DataValue DataValue::operator-(const DataValue& other) const {
    if (is_null_ || other.is_null_) {
        return DataValue(type_);
    }

    switch (type_) {
        case DataType::BIGINT:
            if (other.type_ == DataType::BIGINT) {
                return DataValue(int_val_ - other.int_val_);
            }
            break;
        case DataType::DOUBLE:
            return DataValue(asDouble() - other.asDouble());
        case DataType::DECIMAL:
            if (other.type_ == DataType::DECIMAL) {
                return DataValue(*decimal_val_ - *other.decimal_val_);
            }
            break;
        default:
            break;
    }
    throw std::runtime_error("Unsupported operation");
}

DataValue DataValue::operator*(const DataValue& other) const {
    if (is_null_ || other.is_null_) {
        return DataValue(type_);
    }

    switch (type_) {
        case DataType::BIGINT:
            if (other.type_ == DataType::BIGINT) {
                return DataValue(int_val_ * other.int_val_);
            }
            break;
        case DataType::DOUBLE:
            return DataValue(asDouble() * other.asDouble());
        case DataType::DECIMAL:
            if (other.type_ == DataType::DECIMAL) {
                return DataValue(*decimal_val_ * *other.decimal_val_);
            }
            break;
        default:
            break;
    }
    throw std::runtime_error("Unsupported operation");
}

DataValue DataValue::operator/(const DataValue& other) const {
    if (is_null_ || other.is_null_) {
        return DataValue(type_);
    }

    switch (type_) {
        case DataType::BIGINT:
            if (other.type_ == DataType::BIGINT && other.int_val_ != 0) {
                return DataValue(int_val_ / other.int_val_);
            }
            break;
        case DataType::DOUBLE:
            if (other.asDouble() != 0.0) {
                return DataValue(asDouble() / other.asDouble());
            }
            break;
        case DataType::DECIMAL:
            if (other.type_ == DataType::DECIMAL) {
                return DataValue(*decimal_val_ / *other.decimal_val_);
            }
            break;
        default:
            break;
    }
    throw std::runtime_error("Unsupported operation");
}

bool DataValue::operator==(const DataValue& other) const {
    if (is_null_ != other.is_null_) {
        return false;
    }
    if (is_null_) {
        return true;  // 两个都是NULL
    }

    switch (type_) {
        case DataType::BIGINT:
            return other.type_ == DataType::BIGINT && int_val_ == other.int_val_;
        case DataType::DOUBLE:
            return asDouble() == other.asDouble();
        case DataType::VARCHAR:
            return other.type_ == DataType::VARCHAR && *string_val_ == *other.string_val_;
        case DataType::BOOLEAN:
            return other.type_ == DataType::BOOLEAN && bool_val_ == other.bool_val_;
        case DataType::DECIMAL:
            return other.type_ == DataType::DECIMAL && *decimal_val_ == *other.decimal_val_;
        case DataType::TIMESTAMP:
            return other.type_ == DataType::TIMESTAMP && *datetime_val_ == *other.datetime_val_;
        default:
            return false;
    }
}

bool DataValue::operator<(const DataValue& other) const {
    if (is_null_ || other.is_null_) {
        return false;  // NULL值不参与比较
    }

    switch (type_) {
        case DataType::BIGINT:
            return other.type_ == DataType::BIGINT && int_val_ < other.int_val_;
        case DataType::DOUBLE:
            return asDouble() < other.asDouble();
        case DataType::VARCHAR:
            return other.type_ == DataType::VARCHAR && *string_val_ < *other.string_val_;
        case DataType::DECIMAL:
            return other.type_ == DataType::DECIMAL && *decimal_val_ < *other.decimal_val_;
        case DataType::TIMESTAMP:
            return other.type_ == DataType::TIMESTAMP && *datetime_val_ < *other.datetime_val_;
        default:
            return false;
    }
}

std::string DataValue::serialize() const {
    if (is_null_) {
        return "NULL";
    }

    switch (type_) {
        case DataType::BIGINT:
            return std::to_string(int_val_);
        case DataType::DOUBLE:
            return std::to_string(double_val_);
        case DataType::VARCHAR:
            return *string_val_;
        case DataType::BOOLEAN:
            return bool_val_ ? "true" : "false";
        case DataType::DECIMAL:
            return decimal_val_->toString();
        case DataType::TIMESTAMP:
            return datetime_val_->toString();
        default:
            return "";
    }
}

DataValue DataValue::deserialize(const std::string& data, DataType type) {
    if (data == "NULL") {
        DataValue result(type);
        result.setNull();
        return result;
    }

    switch (type) {
        case DataType::BIGINT:
            return DataValue(static_cast<int64_t>(std::stoll(data)));
        case DataType::DOUBLE:
            return DataValue(std::stod(data));
        case DataType::VARCHAR:
            return DataValue(data);
        case DataType::BOOLEAN:
            return DataValue(data == "true");
        case DataType::DECIMAL:
            return DataValue(DecimalValue(data));
        case DataType::TIMESTAMP:
            return DataValue(DateTimeValue(data));
        default:
            return DataValue();
    }
}

void DataValue::copyFrom(const DataValue& other) {
    switch (other.type_) {
        case DataType::VARCHAR:
            string_val_ = std::make_unique<std::string>(*other.string_val_);
            break;
        case DataType::DECIMAL:
            decimal_val_ = std::make_unique<DecimalValue>(*other.decimal_val_);
            break;
        case DataType::TIMESTAMP:
        case DataType::DATE:
        case DataType::TIME:
            datetime_val_ = std::make_unique<DateTimeValue>(*other.datetime_val_);
            break;
        default:
            // 基本类型已经在union中复制
            break;
    }
}

void DataValue::moveFrom(DataValue&& other) {
    switch (other.type_) {
        case DataType::VARCHAR:
            string_val_ = std::move(other.string_val_);
            break;
        case DataType::DECIMAL:
            decimal_val_ = std::move(other.decimal_val_);
            break;
        case DataType::TIMESTAMP:
        case DataType::DATE:
        case DataType::TIME:
            datetime_val_ = std::move(other.datetime_val_);
            break;
        default:
            // 基本类型已经在union中
            break;
    }
    other.type_ = DataType::UNKNOWN;
    other.is_null_ = true;
}

void DataValue::cleanup() {
    string_val_.reset();
    decimal_val_.reset();
    datetime_val_.reset();
}

// 其他比较运算符
bool DataValue::operator!=(const DataValue& other) const { return !(*this == other); }
bool DataValue::operator<=(const DataValue& other) const { return !(other < *this); }
bool DataValue::operator>(const DataValue& other) const { return other < *this; }
bool DataValue::operator>=(const DataValue& other) const { return !(other > *this); }

// ==================== DataTypeInfo 实现 ====================

DataTypeInfo::DataTypeInfo(DataType t, const std::string& n, size_t s, bool fixed, bool numeric, bool temporal, int precision, int scale)
    : type(t), name(n), size(s), isFixedSize(fixed), isNumeric(numeric), isTemporal(temporal),
      defaultPrecision(precision), defaultScale(scale) {}

// ==================== DataTypeManager 实现 ====================

DataTypeManager& DataTypeManager::getInstance() {
    static DataTypeManager instance;
    return instance;
}

DataTypeManager::DataTypeManager() {
    // 初始化所有数据类型信息
    type_infos_ = {
        {DataType::INTEGER, DataTypeInfo(DataType::INTEGER, "INT", 4, true, true, false)},
        {DataType::BIGINT, DataTypeInfo(DataType::BIGINT, "BIGINT", 8, true, true, false)},
        {DataType::SMALLINT, DataTypeInfo(DataType::SMALLINT, "SMALLINT", 2, true, true, false)},
        {DataType::TINYINT, DataTypeInfo(DataType::TINYINT, "TINYINT", 1, true, true, false)},
        {DataType::DECIMAL, DataTypeInfo(DataType::DECIMAL, "DECIMAL", 16, true, true, false, 18, 2)},
        {DataType::FLOAT, DataTypeInfo(DataType::FLOAT, "FLOAT", 4, true, true, false)},
        {DataType::DOUBLE, DataTypeInfo(DataType::DOUBLE, "DOUBLE", 8, true, true, false)},
        {DataType::VARCHAR, DataTypeInfo(DataType::VARCHAR, "VARCHAR", 0, false, false, false)},
        {DataType::CHAR, DataTypeInfo(DataType::CHAR, "CHAR", 0, false, false, false)},
        {DataType::TEXT, DataTypeInfo(DataType::TEXT, "TEXT", 0, false, false, false)},
        {DataType::DATE, DataTypeInfo(DataType::DATE, "DATE", 8, true, false, true)},
        {DataType::TIME, DataTypeInfo(DataType::TIME, "TIME", 8, true, false, true)},
        {DataType::TIMESTAMP, DataTypeInfo(DataType::TIMESTAMP, "TIMESTAMP", 8, true, false, true)},
        {DataType::DATETIME, DataTypeInfo(DataType::DATETIME, "DATETIME", 8, true, false, true)},
        {DataType::BOOLEAN, DataTypeInfo(DataType::BOOLEAN, "BOOLEAN", 1, true, false, false)},
        {DataType::BLOB, DataTypeInfo(DataType::BLOB, "BLOB", 0, false, false, false)},
        {DataType::CLOB, DataTypeInfo(DataType::CLOB, "CLOB", 0, false, false, false)}
    };

    // 初始化名称到类型的映射
    for (const auto& pair : type_infos_) {
        name_to_type_[pair.second.name] = pair.first;
    }

    // 添加别名
    name_to_type_["INT"] = DataType::INTEGER;
    name_to_type_["BOOL"] = DataType::BOOLEAN;
}

const DataTypeInfo* DataTypeManager::getTypeInfo(DataType type) const {
    auto it = type_infos_.find(type);
    return it != type_infos_.end() ? &it->second : nullptr;
}

const DataTypeInfo* DataTypeManager::getTypeInfo(const std::string& typeName) const {
    std::string upper_name = typeName;
    std::transform(upper_name.begin(), upper_name.end(), upper_name.begin(), ::toupper);

    DataType type = getTypeFromName(upper_name);
    if (type != DataType::UNKNOWN) {
        return getTypeInfo(type);
    }
    return nullptr;
}

DataType DataTypeManager::getTypeFromName(const std::string& typeName) const {
    std::string upper_name = typeName;
    std::transform(upper_name.begin(), upper_name.end(), upper_name.begin(), ::toupper);

    auto it = name_to_type_.find(upper_name);
    return it != name_to_type_.end() ? it->second : DataType::UNKNOWN;
}

std::string DataTypeManager::getTypeName(DataType type) const {
    const DataTypeInfo* info = getTypeInfo(type);
    return info ? info->name : "UNKNOWN";
}

bool DataTypeManager::isValidTypeName(const std::string& typeName) const {
    return getTypeFromName(typeName) != DataType::UNKNOWN;
}

bool DataTypeManager::isNumericType(DataType type) const {
    const DataTypeInfo* info = getTypeInfo(type);
    return info && info->isNumeric;
}

bool DataTypeManager::isTemporalType(DataType type) const {
    const DataTypeInfo* info = getTypeInfo(type);
    return info && info->isTemporal;
}

bool DataTypeManager::canConvert(DataType from, DataType to) const {
    if (from == to) {
        return true;
    }

    // 数字类型之间可以转换
    if (isNumericType(from) && isNumericType(to)) {
        return true;
    }

    // 字符串可以转换为其他类型
    if (from == DataType::VARCHAR || from == DataType::TEXT) {
        return true;
    }

    // 其他类型转换为字符串
    if (to == DataType::VARCHAR || to == DataType::TEXT) {
        return true;
    }

    return false;
}

DataValue DataTypeManager::convertValue(const DataValue& value, DataType targetType) const {
    if (value.getType() == targetType) {
        return value;
    }

    if (!canConvert(value.getType(), targetType)) {
        throw std::runtime_error("Cannot convert between these data types");
    }

    // 简化实现，实际应该有更完整的转换逻辑
    switch (targetType) {
        case DataType::BIGINT:
            return DataValue(static_cast<int64_t>(value.asDouble()));
        case DataType::DOUBLE:
            return DataValue(value.asDouble());
        case DataType::VARCHAR:
            return DataValue(value.asString());
        case DataType::DECIMAL:
            return DataValue(DecimalValue(value.asString()));
        default:
            throw std::runtime_error("Unsupported type conversion");
    }
}

bool DataTypeManager::parseDecimalType(const std::string& typeStr, int& precision, int& scale) const {
    // 解析DECIMAL(precision, scale)格式
    std::regex decimal_regex(R"(DECIMAL\s*\(\s*(\d+)\s*(?:,\s*(\d+)\s*)?\))", std::regex_constants::icase);
    std::smatch matches;

    if (std::regex_match(typeStr, matches, decimal_regex)) {
        precision = std::stoi(matches[1].str());
        scale = matches.size() > 2 && matches[2].matched ? std::stoi(matches[2].str()) : 0;
        return true;
    }

    return false;
}

} // namespace sql_parser
} // namespace sqlcc
