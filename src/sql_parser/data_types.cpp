#include "sql_parser/data_types.h"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <regex>

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
    from_string(str);
}

DecimalValue::DecimalValue(double value, int precision, int scale)
    : precision_(precision), scale_(scale) {
    from_double(value);
}

DecimalValue DecimalValue::operator+(const DecimalValue& other) const {
    // 简化的加法实现：假设相同精度和刻度
    return DecimalValue(value_ + other.value_, precision_, scale_);
}

DecimalValue DecimalValue::operator-(const DecimalValue& other) const {
    return DecimalValue(value_ - other.value_, precision_, scale_);
}

DecimalValue DecimalValue::operator*(const DecimalValue& other) const {
    return DecimalValue(value_ * other.value_, precision_, scale_);
}

DecimalValue DecimalValue::operator/(const DecimalValue& other) const {
    if (other.value_ == 0) {
        throw std::runtime_error("Division by zero");
    }
    return DecimalValue(value_ / other.value_, precision_, scale_);
}

bool DecimalValue::operator==(const DecimalValue& other) const {
    return value_ == other.value_ && scale_ == other.scale_;
}

bool DecimalValue::operator!=(const DecimalValue& other) const {
    return !(*this == other);
}

bool DecimalValue::operator<(const DecimalValue& other) const {
    return value_ < other.value_;
}

bool DecimalValue::operator<=(const DecimalValue& other) const {
    return *this < other || *this == other;
}

bool DecimalValue::operator>(const DecimalValue& other) const {
    return !(*this <= other);
}

bool DecimalValue::operator>=(const DecimalValue& other) const {
    return !(*this < other);
}

std::string DecimalValue::toString() const {
    std::stringstream ss;
    int64_t divisor = 1;
    for (int i = 0; i < scale_; ++i) {
        divisor *= 10;
    }

    int64_t int_part = value_ / divisor;
    int64_t frac_part = std::abs(value_ % divisor);

    ss << int_part;
    if (scale_ > 0) {
        ss << "." << std::setfill('0') << std::setw(scale_) << frac_part;
    }

    return ss.str();
}

double DecimalValue::toDouble() const {
    return static_cast<double>(value_) / std::pow(10, scale_);
}

int64_t DecimalValue::toInt64() const {
    int64_t divisor = 1;
    for (int i = 0; i < scale_; ++i) {
        divisor *= 10;
    }
    return value_ / divisor;
}

void DecimalValue::normalize() {
    // 确保精度和刻度在合理范围内
    if (precision_ < 1) precision_ = 18;
    if (precision_ > 38) precision_ = 38;
    if (scale_ < 0) scale_ = 0;
    if (scale_ > precision_) scale_ = precision_;
}

void DecimalValue::from_string(const std::string& str) {
    std::regex decimal_regex(R"(^(-?\d+)(\.(\d+))?$)");
    std::smatch match;

    if (std::regex_match(str, match, decimal_regex)) {
        int64_t int_part = std::stoll(match[1].str());
        std::string frac_str = match[3].str();

        scale_ = frac_str.length();
        value_ = int_part;

        // 将小数部分添加到整数值中
        if (scale_ > 0) {
            int64_t frac_part = std::stoll(frac_str);
            for (int i = 0; i < scale_; ++i) {
                value_ *= 10;
            }
            if (int_part >= 0) {
                value_ += frac_part;
            } else {
                value_ -= frac_part;
            }
        }
    } else {
        throw std::runtime_error("Invalid decimal format: " + str);
    }
}

void DecimalValue::from_double(double value) {
    // 简化的double转换
    std::stringstream ss;
    ss << std::fixed << std::setprecision(scale_) << value;
    from_string(ss.str());
}

// ==================== DateTimeValue 实现 ====================

DateTimeValue::DateTimeValue() : time_point_(std::chrono::system_clock::now()), format_(Format::TIMESTAMP) {}

DateTimeValue::DateTimeValue(const std::string& str, Format format)
    : format_(format) {
    time_point_ = parseString(str, format);
}

DateTimeValue::DateTimeValue(std::chrono::system_clock::time_point tp)
    : time_point_(tp), format_(Format::TIMESTAMP) {}

DateTimeValue DateTimeValue::operator+(const DateTimeValue& other) const {
    return DateTimeValue(time_point_ + (other.time_point_ - std::chrono::system_clock::now()));
}

DateTimeValue DateTimeValue::operator-(const DateTimeValue& other) const {
    return DateTimeValue(time_point_ - (other.time_point_ - std::chrono::system_clock::now()));
}

bool DateTimeValue::operator==(const DateTimeValue& other) const {
    return time_point_ == other.time_point_;
}

bool DateTimeValue::operator!=(const DateTimeValue& other) const {
    return !(*this == other);
}

bool DateTimeValue::operator<(const DateTimeValue& other) const {
    return time_point_ < other.time_point_;
}

bool DateTimeValue::operator<=(const DateTimeValue& other) const {
    return *this < other || *this == other;
}

bool DateTimeValue::operator>(const DateTimeValue& other) const {
    return !(*this <= other);
}

bool DateTimeValue::operator>=(const DateTimeValue& other) const {
    return !(*this < other);
}

std::string DateTimeValue::toString() const {
    return formatTimePoint(time_point_, format_);
}

std::chrono::system_clock::time_point DateTimeValue::toTimePoint() const {
    return time_point_;
}

int DateTimeValue::getYear() const {
    auto time = std::chrono::system_clock::to_time_t(time_point_);
    std::tm* tm = std::localtime(&time);
    return tm->tm_year + 1900;
}

int DateTimeValue::getMonth() const {
    auto time = std::chrono::system_clock::to_time_t(time_point_);
    std::tm* tm = std::localtime(&time);
    return tm->tm_mon + 1;
}

int DateTimeValue::getDay() const {
    auto time = std::chrono::system_clock::to_time_t(time_point_);
    std::tm* tm = std::localtime(&time);
    return tm->tm_mday;
}

int DateTimeValue::getHour() const {
    auto time = std::chrono::system_clock::to_time_t(time_point_);
    std::tm* tm = std::localtime(&time);
    return tm->tm_hour;
}

int DateTimeValue::getMinute() const {
    auto time = std::chrono::system_clock::to_time_t(time_point_);
    std::tm* tm = std::localtime(&time);
    return tm->tm_min;
}

int DateTimeValue::getSecond() const {
    auto time = std::chrono::system_clock::to_time_t(time_point_);
    std::tm* tm = std::localtime(&time);
    return tm->tm_sec;
}

DateTimeValue DateTimeValue::now() {
    return DateTimeValue(std::chrono::system_clock::now());
}

DateTimeValue DateTimeValue::today() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm* tm = std::localtime(&time);
    tm->tm_hour = 0;
    tm->tm_min = 0;
    tm->tm_sec = 0;
    return DateTimeValue(std::chrono::system_clock::from_time_t(std::mktime(tm)));
}

std::chrono::system_clock::time_point DateTimeValue::parseString(const std::string& str, Format format) {
    std::tm tm = {};
    std::istringstream ss(str);

    switch (format) {
        case Format::DATE:
            ss >> std::get_time(&tm, "%Y-%m-%d");
            break;
        case Format::TIME:
            ss >> std::get_time(&tm, "%H:%M:%S");
            break;
        case Format::TIMESTAMP:
        case Format::DATETIME:
            ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
            break;
    }

    if (ss.fail()) {
        throw std::runtime_error("Invalid datetime format: " + str);
    }

    return std::chrono::system_clock::from_time_t(std::mktime(&tm));
}

std::string DateTimeValue::formatTimePoint(const std::chrono::system_clock::time_point& tp, Format format) {
    auto time = std::chrono::system_clock::to_time_t(tp);
    std::tm* tm = std::localtime(&time);
    std::ostringstream ss;

    switch (format) {
        case Format::DATE:
            ss << std::put_time(tm, "%Y-%m-%d");
            break;
        case Format::TIME:
            ss << std::put_time(tm, "%H:%M:%S");
            break;
        case Format::TIMESTAMP:
        case Format::DATETIME:
            ss << std::put_time(tm, "%Y-%m-%d %H:%M:%S");
            break;
    }

    return ss.str();
}

// ==================== DataValue 实现 ====================

DataValue::DataValue() : type_(DataType::UNKNOWN), is_null_(true) {}

DataValue::DataValue(DataType type) : type_(type), is_null_(true) {}

DataValue::DataValue(int64_t int_val) : type_(DataType::BIGINT), is_null_(false) {
    new (&int_val_) int64_t(int_val);
}

DataValue::DataValue(double double_val) : type_(DataType::DOUBLE), is_null_(false) {
    new (&double_val_) double(double_val);
}

DataValue::DataValue(const std::string& str_val) : type_(DataType::VARCHAR), is_null_(false) {
    string_val_ = std::make_unique<std::string>(str_val);
}

DataValue::DataValue(bool bool_val) : type_(DataType::BOOLEAN), is_null_(false) {
    new (&bool_val_) bool(bool_val);
}

DataValue::DataValue(const DecimalValue& decimal_val) : type_(DataType::DECIMAL), is_null_(false) {
    decimal_val_ = std::make_unique<DecimalValue>(decimal_val);
}

DataValue::DataValue(const DateTimeValue& datetime_val) : type_(DataType::TIMESTAMP), is_null_(false) {
    datetime_val_ = std::make_unique<DateTimeValue>(datetime_val);
}

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
    if (is_null_ || type_ != DataType::DOUBLE) {
        throw std::runtime_error("Cannot convert to double");
    }
    return double_val_;
}

std::string DataValue::asString() const {
    if (is_null_ || type_ != DataType::VARCHAR) {
        throw std::runtime_error("Cannot convert to string");
    }
    return *string_val_;
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
    if (is_null_ || type_ != DataType::TIMESTAMP) {
        throw std::runtime_error("Cannot convert to datetime");
    }
    return *datetime_val_;
}

void DataValue::setInt64(int64_t value) {
    cleanup();
    type_ = DataType::BIGINT;
    is_null_ = false;
    new (&int_val_) int64_t(value);
}

void DataValue::setDouble(double value) {
    cleanup();
    type_ = DataType::DOUBLE;
    is_null_ = false;
    new (&double_val_) double(value);
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
    new (&bool_val_) bool(value);
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

DataValue DataValue::operator+(const DataValue& other) const {
    // 简化的实现，仅支持数值类型
    if (type_ == DataType::BIGINT && other.type_ == DataType::BIGINT) {
        return DataValue(asInt64() + other.asInt64());
    }
    if (type_ == DataType::DOUBLE && other.type_ == DataType::DOUBLE) {
        return DataValue(asDouble() + other.asDouble());
    }
    if (type_ == DataType::DECIMAL && other.type_ == DataType::DECIMAL) {
        return DataValue(asDecimal() + other.asDecimal());
    }
    throw std::runtime_error("Unsupported operation");
}

DataValue DataValue::operator-(const DataValue& other) const {
    if (type_ == DataType::BIGINT && other.type_ == DataType::BIGINT) {
        return DataValue(asInt64() - other.asInt64());
    }
    if (type_ == DataType::DOUBLE && other.type_ == DataType::DOUBLE) {
        return DataValue(asDouble() - other.asDouble());
    }
    if (type_ == DataType::DECIMAL && other.type_ == DataType::DECIMAL) {
        return DataValue(asDecimal() - other.asDecimal());
    }
    throw std::runtime_error("Unsupported operation");
}

DataValue DataValue::operator*(const DataValue& other) const {
    if (type_ == DataType::BIGINT && other.type_ == DataType::BIGINT) {
        return DataValue(asInt64() * other.asInt64());
    }
    if (type_ == DataType::DOUBLE && other.type_ == DataType::DOUBLE) {
        return DataValue(asDouble() * other.asDouble());
    }
    if (type_ == DataType::DECIMAL && other.type_ == DataType::DECIMAL) {
        return DataValue(asDecimal() * other.asDecimal());
    }
    throw std::runtime_error("Unsupported operation");
}

DataValue DataValue::operator/(const DataValue& other) const {
    if (type_ == DataType::BIGINT && other.type_ == DataType::BIGINT) {
        if (other.asInt64() == 0) throw std::runtime_error("Division by zero");
        return DataValue(asInt64() / other.asInt64());
    }
    if (type_ == DataType::DOUBLE && other.type_ == DataType::DOUBLE) {
        if (other.asDouble() == 0.0) throw std::runtime_error("Division by zero");
        return DataValue(asDouble() / other.asDouble());
    }
    if (type_ == DataType::DECIMAL && other.type_ == DataType::DECIMAL) {
        return DataValue(asDecimal() / other.asDecimal());
    }
    throw std::runtime_error("Unsupported operation");
}

bool DataValue::operator==(const DataValue& other) const {
    if (type_ != other.type_ || is_null_ != other.is_null_) {
        return false;
    }
    if (is_null_) return true;

    switch (type_) {
        case DataType::BIGINT: return asInt64() == other.asInt64();
        case DataType::DOUBLE: return asDouble() == other.asDouble();
        case DataType::VARCHAR: return asString() == other.asString();
        case DataType::BOOLEAN: return asBool() == other.asBool();
        case DataType::DECIMAL: return asDecimal() == other.asDecimal();
        case DataType::TIMESTAMP: return asDateTime() == other.asDateTime();
        default: return false;
    }
}

bool DataValue::operator!=(const DataValue& other) const {
    return !(*this == other);
}

bool DataValue::operator<(const DataValue& other) const {
    if (type_ != other.type_ || is_null_ || other.is_null_) {
        throw std::runtime_error("Cannot compare different types or null values");
    }

    switch (type_) {
        case DataType::BIGINT: return asInt64() < other.asInt64();
        case DataType::DOUBLE: return asDouble() < other.asDouble();
        case DataType::VARCHAR: return asString() < other.asString();
        case DataType::DECIMAL: return asDecimal() < other.asDecimal();
        case DataType::TIMESTAMP: return asDateTime() < other.asDateTime();
        default: throw std::runtime_error("Unsupported comparison");
    }
}

bool DataValue::operator<=(const DataValue& other) const {
    return *this < other || *this == other;
}

bool DataValue::operator>(const DataValue& other) const {
    return !(*this <= other);
}

bool DataValue::operator>=(const DataValue& other) const {
    return !(*this < other);
}

std::string DataValue::serialize() const {
    if (is_null_) return "NULL";

    std::stringstream ss;
    ss << static_cast<int>(type_) << ":";

    switch (type_) {
        case DataType::BIGINT: ss << asInt64(); break;
        case DataType::DOUBLE: ss << asDouble(); break;
        case DataType::VARCHAR: ss << asString(); break;
        case DataType::BOOLEAN: ss << (asBool() ? "true" : "false"); break;
        case DataType::DECIMAL: ss << asDecimal().toString(); break;
        case DataType::TIMESTAMP: ss << asDateTime().toString(); break;
        default: throw std::runtime_error("Unsupported serialization");
    }

    return ss.str();
}

DataValue DataValue::deserialize(const std::string& data, DataType type) {
    if (data == "NULL") return DataValue(type);

    size_t colon_pos = data.find(':');
    if (colon_pos == std::string::npos) {
        throw std::runtime_error("Invalid serialized data");
    }

    std::string value_str = data.substr(colon_pos + 1);

    switch (type) {
        case DataType::BIGINT: {
            int64_t val = std::stoll(value_str);
            return DataValue(val);
        }
        case DataType::DOUBLE: {
            double val = std::stod(value_str);
            return DataValue(val);
        }
        case DataType::VARCHAR: return DataValue(value_str);
        case DataType::BOOLEAN: {
            bool val = (value_str == "true");
            return DataValue(val);
        }
        case DataType::DECIMAL: return DataValue(DecimalValue(value_str));
        case DataType::TIMESTAMP: return DataValue(DateTimeValue(value_str));
        default: throw std::runtime_error("Unsupported deserialization");
    }
}

void DataValue::copyFrom(const DataValue& other) {
    switch (other.type_) {
        case DataType::BIGINT: new (&int_val_) int64_t(other.int_val_); break;
        case DataType::DOUBLE: new (&double_val_) double(other.double_val_); break;
        case DataType::BOOLEAN: new (&bool_val_) bool(other.bool_val_); break;
        case DataType::VARCHAR:
            string_val_ = std::make_unique<std::string>(*other.string_val_);
            break;
        case DataType::DECIMAL:
            decimal_val_ = std::make_unique<DecimalValue>(*other.decimal_val_);
            break;
        case DataType::TIMESTAMP:
            datetime_val_ = std::make_unique<DateTimeValue>(*other.datetime_val_);
            break;
        default: break;
    }
}

void DataValue::moveFrom(DataValue&& other) {
    switch (other.type_) {
        case DataType::BIGINT: new (&int_val_) int64_t(other.int_val_); break;
        case DataType::DOUBLE: new (&double_val_) double(other.double_val_); break;
        case DataType::BOOLEAN: new (&bool_val_) bool(other.bool_val_); break;
        case DataType::VARCHAR: string_val_ = std::move(other.string_val_); break;
        case DataType::DECIMAL: decimal_val_ = std::move(other.decimal_val_); break;
        case DataType::TIMESTAMP: datetime_val_ = std::move(other.datetime_val_); break;
        default: break;
    }
    other.type_ = DataType::UNKNOWN;
    other.is_null_ = true;
}

void DataValue::cleanup() {
    switch (type_) {
        case DataType::VARCHAR: string_val_.reset(); break;
        case DataType::DECIMAL: decimal_val_.reset(); break;
        case DataType::TIMESTAMP: datetime_val_.reset(); break;
        default: break;
    }
}

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
    // 初始化基本数据类型信息
    type_infos_.emplace(DataType::INTEGER, DataTypeInfo(DataType::INTEGER, "INT", 4, true, true, false));
    type_infos_.emplace(DataType::BIGINT, DataTypeInfo(DataType::BIGINT, "BIGINT", 8, true, true, false));
    type_infos_.emplace(DataType::SMALLINT, DataTypeInfo(DataType::SMALLINT, "SMALLINT", 2, true, true, false));
    type_infos_.emplace(DataType::TINYINT, DataTypeInfo(DataType::TINYINT, "TINYINT", 1, true, true, false));
    type_infos_.emplace(DataType::DECIMAL, DataTypeInfo(DataType::DECIMAL, "DECIMAL", 0, false, true, false, 18, 0));
    type_infos_.emplace(DataType::FLOAT, DataTypeInfo(DataType::FLOAT, "FLOAT", 4, true, true, false));
    type_infos_.emplace(DataType::DOUBLE, DataTypeInfo(DataType::DOUBLE, "DOUBLE", 8, true, true, false));
    type_infos_.emplace(DataType::VARCHAR, DataTypeInfo(DataType::VARCHAR, "VARCHAR", 0, false, false, false));
    type_infos_.emplace(DataType::CHAR, DataTypeInfo(DataType::CHAR, "CHAR", 0, false, false, false));
    type_infos_.emplace(DataType::TEXT, DataTypeInfo(DataType::TEXT, "TEXT", 0, false, false, false));
    type_infos_.emplace(DataType::DATE, DataTypeInfo(DataType::DATE, "DATE", 3, true, false, true));
    type_infos_.emplace(DataType::TIME, DataTypeInfo(DataType::TIME, "TIME", 3, true, false, true));
    type_infos_.emplace(DataType::TIMESTAMP, DataTypeInfo(DataType::TIMESTAMP, "TIMESTAMP", 8, true, false, true));
    type_infos_.emplace(DataType::DATETIME, DataTypeInfo(DataType::DATETIME, "DATETIME", 8, true, false, true));
    type_infos_.emplace(DataType::BOOLEAN, DataTypeInfo(DataType::BOOLEAN, "BOOLEAN", 1, true, false, false));
    type_infos_.emplace(DataType::BLOB, DataTypeInfo(DataType::BLOB, "BLOB", 0, false, false, false));
    type_infos_.emplace(DataType::CLOB, DataTypeInfo(DataType::CLOB, "CLOB", 0, false, false, false));

    // 构建名称到类型的映射
    for (const auto& pair : type_infos_) {
        name_to_type_[pair.second.name] = pair.first;
    }
}

const DataTypeInfo* DataTypeManager::getTypeInfo(DataType type) const {
    auto it = type_infos_.find(type);
    return it != type_infos_.end() ? &it->second : nullptr;
}

const DataTypeInfo* DataTypeManager::getTypeInfo(const std::string& typeName) const {
    DataType type = getTypeFromName(typeName);
    return type != DataType::UNKNOWN ? getTypeInfo(type) : nullptr;
}

DataType DataTypeManager::getTypeFromName(const std::string& typeName) const {
    auto it = name_to_type_.find(typeName);
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
    if (from == to) return true;

    // 数字类型之间的转换
    if (isNumericType(from) && isNumericType(to)) return true;

    // 字符串与其他类型之间的转换
    if ((from == DataType::VARCHAR && (isNumericType(to) || isTemporalType(to))) ||
        (to == DataType::VARCHAR && (isNumericType(from) || isTemporalType(from)))) {
        return true;
    }

    // 时间戳与日期时间之间的转换
    if ((from == DataType::TIMESTAMP && to == DataType::DATETIME) ||
        (from == DataType::DATETIME && to == DataType::TIMESTAMP)) {
        return true;
    }

    return false;
}

DataValue DataTypeManager::convertValue(const DataValue& value, DataType targetType) const {
    if (value.getType() == targetType) return value;

    if (!canConvert(value.getType(), targetType)) {
        throw std::runtime_error("Cannot convert between these types");
    }

    DataValue result(targetType);

    // 数值类型转换
    if (isNumericType(value.getType()) && isNumericType(targetType)) {
        switch (targetType) {
            case DataType::BIGINT:
                if (value.getType() == DataType::DOUBLE) result.setInt64(static_cast<int64_t>(value.asDouble()));
                else if (value.getType() == DataType::DECIMAL) result.setInt64(value.asDecimal().toInt64());
                break;
            case DataType::DOUBLE:
                if (value.getType() == DataType::BIGINT) result.setDouble(static_cast<double>(value.asInt64()));
                else if (value.getType() == DataType::DECIMAL) result.setDouble(value.asDecimal().toDouble());
                break;
            case DataType::DECIMAL:
                if (value.getType() == DataType::BIGINT) result.setDecimal(DecimalValue(value.asInt64()));
                else if (value.getType() == DataType::DOUBLE) result.setDecimal(DecimalValue(value.asDouble()));
                break;
            default: break;
        }
    }

    // 字符串转换
    if (value.getType() == DataType::VARCHAR) {
        std::string str = value.asString();
        switch (targetType) {
            case DataType::BIGINT: result.setInt64(std::stoll(str)); break;
            case DataType::DOUBLE: result.setDouble(std::stod(str)); break;
            case DataType::DECIMAL: result.setDecimal(DecimalValue(str)); break;
            case DataType::TIMESTAMP: result.setDateTime(DateTimeValue(str)); break;
            default: break;
        }
    } else if (targetType == DataType::VARCHAR) {
        switch (value.getType()) {
            case DataType::BIGINT: result.setString(std::to_string(value.asInt64())); break;
            case DataType::DOUBLE: result.setString(std::to_string(value.asDouble())); break;
            case DataType::DECIMAL: result.setString(value.asDecimal().toString()); break;
            case DataType::TIMESTAMP: result.setString(value.asDateTime().toString()); break;
            case DataType::BOOLEAN: result.setString(value.asBool() ? "true" : "false"); break;
            default: break;
        }
    }

    return result;
}

bool DataTypeManager::parseDecimalType(const std::string& typeStr, int& precision, int& scale) const {
    std::regex decimal_regex(R"(DECIMAL\s*\(\s*(\d+)\s*,\s*(\d+)\s*\))", std::regex_constants::icase);
    std::smatch match;

    if (std::regex_match(typeStr, match, decimal_regex)) {
        precision = std::stoi(match[1].str());
        scale = std::stoi(match[2].str());
        return true;
    }

    // 检查是否只有精度
    std::regex decimal_precision_regex(R"(DECIMAL\s*\(\s*(\d+)\s*\))", std::regex_constants::icase);
    if (std::regex_match(typeStr, match, decimal_precision_regex)) {
        precision = std::stoi(match[1].str());
        scale = 0;  // 默认刻度
        return true;
    }

    return false;
}

} // namespace sql_parser
} // namespace sqlcc