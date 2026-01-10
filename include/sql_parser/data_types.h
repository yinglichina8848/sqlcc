#ifndef SQLCC_SQL_PARSER_DATA_TYPES_H
#define SQLCC_SQL_PARSER_DATA_TYPES_H

#include <string>
#include <memory>
#include <chrono>
#include <cmath>
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <regex>
#include <unordered_map>

namespace sqlcc {
namespace sql_parser {

/**
 * @brief 数据类型枚举
 */
enum class DataType {
    UNKNOWN,
    INTEGER,
    BIGINT,
    SMALLINT,
    TINYINT,
    DECIMAL,
    FLOAT,
    DOUBLE,
    VARCHAR,
    CHAR,
    TEXT,
    DATE,
    TIME,
    TIMESTAMP,
    DATETIME,
    BOOLEAN,
    BLOB,
    CLOB
};

/**
 * @brief DECIMAL数据类型实现
 */
class DecimalValue {
public:
    DecimalValue();
    DecimalValue(int64_t value, int precision = 18, int scale = 0);
    DecimalValue(const std::string& str);
    DecimalValue(double value, int precision = 18, int scale = 2);

    // 运算符重载
    DecimalValue operator+(const DecimalValue& other) const;
    DecimalValue operator-(const DecimalValue& other) const;
    DecimalValue operator*(const DecimalValue& other) const;
    DecimalValue operator/(const DecimalValue& other) const;

    bool operator==(const DecimalValue& other) const;
    bool operator!=(const DecimalValue& other) const;
    bool operator<(const DecimalValue& other) const;
    bool operator<=(const DecimalValue& other) const;
    bool operator>(const DecimalValue& other) const;
    bool operator>=(const DecimalValue& other) const;

    // 类型转换
    std::string toString() const;
    double toDouble() const;
    int64_t toInt64() const;

    // 获取属性
    int getPrecision() const { return precision_; }
    int getScale() const { return scale_; }
    int64_t getValue() const { return value_; }

    // 设置精度和小数位
    void setPrecision(int precision) { precision_ = precision; }
    void setScale(int scale) { scale_ = scale; }

private:
    int64_t value_;      // 存储为整数，单位为10^(-scale)
    int precision_;      // 总精度
    int scale_;          // 小数位数

    void normalize();
    void from_string(const std::string& str);
    void from_double(double value);
    static int64_t stringToInt64(const std::string& str, int scale);
};

/**
 * @brief 日期时间数据类型实现
 */
class DateTimeValue {
public:
    enum class Format {
        DATE,           // YYYY-MM-DD
        TIME,           // HH:MM:SS
        TIMESTAMP,      // YYYY-MM-DD HH:MM:SS
        DATETIME        // YYYY-MM-DD HH:MM:SS (与TIMESTAMP相同)
    };

    DateTimeValue();
    DateTimeValue(const std::string& str, Format format = Format::TIMESTAMP);
    DateTimeValue(std::chrono::system_clock::time_point tp);

    // 运算符重载
    DateTimeValue operator+(const DateTimeValue& other) const;
    DateTimeValue operator-(const DateTimeValue& other) const;

    bool operator==(const DateTimeValue& other) const;
    bool operator!=(const DateTimeValue& other) const;
    bool operator<(const DateTimeValue& other) const;
    bool operator<=(const DateTimeValue& other) const;
    bool operator>(const DateTimeValue& other) const;
    bool operator>=(const DateTimeValue& other) const;

    // 类型转换
    std::string toString() const;
    std::chrono::system_clock::time_point toTimePoint() const;

    // 日期时间操作
    int getYear() const;
    int getMonth() const;
    int getDay() const;
    int getHour() const;
    int getMinute() const;
    int getSecond() const;

    // 格式设置
    Format getFormat() const { return format_; }
    void setFormat(Format format) { format_ = format; }

    // 静态方法
    static DateTimeValue now();
    static DateTimeValue today();

private:
    std::chrono::system_clock::time_point time_point_;
    Format format_;

    static std::chrono::system_clock::time_point parseString(const std::string& str, Format format);
    static std::string formatTimePoint(const std::chrono::system_clock::time_point& tp, Format format);
};

/**
 * @brief 数据类型值容器
 */
class DataValue {
public:
    DataValue();
    DataValue(DataType type);
    DataValue(int64_t int_val);
    DataValue(double double_val);
    DataValue(const std::string& str_val);
    DataValue(bool bool_val);
    DataValue(const DecimalValue& decimal_val);
    DataValue(const DateTimeValue& datetime_val);

    // 拷贝构造函数和赋值运算符
    DataValue(const DataValue& other);
    DataValue& operator=(const DataValue& other);

    // 移动构造函数和赋值运算符
    DataValue(DataValue&& other) noexcept;
    DataValue& operator=(DataValue&& other) noexcept;

    ~DataValue();

    // 获取类型
    DataType getType() const { return type_; }

    // 类型检查
    bool isNull() const { return is_null_; }
    void setNull(bool null = true) { is_null_ = null; }

    // 值获取（带类型检查）
    int64_t asInt64() const;
    double asDouble() const;
    std::string asString() const;
    bool asBool() const;
    DecimalValue asDecimal() const;
    DateTimeValue asDateTime() const;

    // 值设置
    void setInt64(int64_t value);
    void setDouble(double value);
    void setString(const std::string& value);
    void setBool(bool value);
    void setDecimal(const DecimalValue& value);
    void setDateTime(const DateTimeValue& value);

    // 运算符重载
    DataValue operator+(const DataValue& other) const;
    DataValue operator-(const DataValue& other) const;
    DataValue operator*(const DataValue& other) const;
    DataValue operator/(const DataValue& other) const;

    bool operator==(const DataValue& other) const;
    bool operator!=(const DataValue& other) const;
    bool operator<(const DataValue& other) const;
    bool operator<=(const DataValue& other) const;
    bool operator>(const DataValue& other) const;
    bool operator>=(const DataValue& other) const;

    // 序列化/反序列化
    std::string serialize() const;
    static DataValue deserialize(const std::string& data, DataType type);

private:
    DataType type_;
    bool is_null_;
    union {
        int64_t int_val_;
        double double_val_;
        bool bool_val_;
        char string_placeholder_[sizeof(std::string)];  // 占位符
        char decimal_placeholder_[sizeof(DecimalValue)];
        char datetime_placeholder_[sizeof(DateTimeValue)];
    };

    // 使用智能指针存储复杂类型
    std::unique_ptr<std::string> string_val_;
    std::unique_ptr<DecimalValue> decimal_val_;
    std::unique_ptr<DateTimeValue> datetime_val_;

    void copyFrom(const DataValue& other);
    void moveFrom(DataValue&& other);
    void cleanup();
};

/**
 * @brief 数据类型信息
 */
struct DataTypeInfo {
    DataType type;
    std::string name;
    size_t size;              // 固定大小（对于变长类型为0）
    bool isFixedSize;
    bool isNumeric;
    bool isTemporal;

    // DECIMAL特殊属性
    int defaultPrecision;
    int defaultScale;

    // 构造函数
    DataTypeInfo(DataType t, const std::string& n, size_t s = 0,
                bool fixed = true, bool numeric = false, bool temporal = false,
                int precision = 18, int scale = 0);
};

/**
 * @brief 数据类型管理器
 */
class DataTypeManager {
public:
    static DataTypeManager& getInstance();

    // 类型信息查询
    const DataTypeInfo* getTypeInfo(DataType type) const;
    const DataTypeInfo* getTypeInfo(const std::string& typeName) const;
    DataType getTypeFromName(const std::string& typeName) const;
    std::string getTypeName(DataType type) const;

    // 类型验证
    bool isValidTypeName(const std::string& typeName) const;
    bool isNumericType(DataType type) const;
    bool isTemporalType(DataType type) const;

    // 类型转换
    bool canConvert(DataType from, DataType to) const;
    DataValue convertValue(const DataValue& value, DataType targetType) const;

    // DECIMAL类型解析
    bool parseDecimalType(const std::string& typeStr, int& precision, int& scale) const;

private:
    DataTypeManager();
    std::unordered_map<DataType, DataTypeInfo> type_infos_;
    std::unordered_map<std::string, DataType> name_to_type_;
};

} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_DATA_TYPES_H
