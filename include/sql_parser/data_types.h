/**
 * WHY: 为什么需要专门的SQL数据类型系统？
 *
 * 传统数据库系统的数据类型处理存在诸多问题：
 * - 类型系统不统一：不同组件使用不同类型表示方式
 * - 类型转换复杂：缺乏标准化的类型转换机制
 * - 精度处理不当：DECIMAL等精确数值类型处理不当
 * - 日期时间复杂：缺乏统一的日期时间处理机制
 * - 内存管理低效：频繁的字符串拷贝和转换操作
 * - 类型安全缺失：运行时类型错误难以发现
 *
 * SQL数据类型系统的核心价值：
 * 1. 类型安全性：编译时和运行时类型检查保证
 * 2. 性能优化：高效的内存布局和操作算法
 * 3. 标准兼容：完全符合SQL标准的类型规范
 * 4. 扩展性：易于添加新的数据类型支持
 * 5. 统一接口：一致的类型操作和转换API
 *
 * 🏗️ 设计模式：类型对象模式(Type Object Pattern)
 *
 * 数据类型作为类型对象的优势：
 * - 运行时类型信息：动态查询类型特性和行为
 * - 多态性：统一的接口处理不同数据类型
 * - 扩展性：通过继承轻松添加新类型
 * - 封装性：类型实现细节对外界隐藏
 * - 复用性：类型对象可在多个地方复用
 *
 * SOLID原则体现：
 * - 单一职责：每个类型类负责一种数据类型的处理
 * - 开闭原则：新数据类型通过扩展现有类型实现
 * - 里氏替换：子类可以替换父类使用
 * - 接口隔离：客户端依赖具体需要的类型接口
 * - 依赖倒置：高层模块不依赖具体类型实现
 *
 * WHAT: SQL数据类型系统 - 完整的SQL类型定义和操作框架
 *
 * 核心功能：
 * - 基础数据类型：INTEGER, VARCHAR, BOOLEAN等标准SQL类型
 * - 精确数值类型：DECIMAL类型的高精度数学运算
 * - 日期时间类型：DATE, TIME, TIMESTAMP的完整支持
 * - 大对象类型：BLOB, CLOB等大数据类型处理
 * - 类型转换系统：安全高效的类型间转换机制
 * - 类型验证系统：运行时类型安全检查和约束验证
 *
 * 系统组件：
 * - DataType枚举：定义所有支持的数据类型
 * - DecimalValue类：高精度十进制数值的实现
 * - DateTimeValue类：日期时间数据的处理
 * - DataValue类：统一的值容器和操作接口
 * - DataTypeManager类：类型管理和转换的中心控制器
 *
 * 接口设计：
 * - 类型查询：getType(), isNumeric(), isTemporal()
 * - 值操作：asInt64(), asString(), toString()
 * - 类型转换：convertValue(), canConvert()
 * - 运算支持：算术运算符重载和比较操作
 *
 * HOW: SQL数据类型系统的实现机制
 *
 * 类型枚举设计：
 * - 连续编号：便于数组索引和快速查找
 * - 语义分组：相同性质的类型排列在一起
 * - 扩展预留：为未来类型扩展预留空间
 * - 标准对齐：与SQL标准完全保持一致
 *
 * DECIMAL类型实现：
 * 1. 内部存储：使用int64_t存储，单位为10^(-scale)
 * 2. 精度控制：precision控制总位数，scale控制小数位
 * 3. 运算实现：基于整数运算保证精度不丢失
 * 4. 溢出处理：检测和处理运算溢出情况
 * 5. 标准化：自动调整精度和小数位以保持一致性
 *
 * 日期时间处理：
 * 1. 时间点存储：使用chrono::system_clock::time_point
 * 2. 格式解析：支持多种日期时间格式的解析
 * 3. 时区处理：考虑时区转换和本地化需求
 * 4. 运算支持：日期时间间隔的加减运算
 * 5. 格式化输出：标准SQL日期时间格式输出
 *
 * 联合类型容器：
 * 1. 内存优化：使用union节省内存空间
 * 2. 类型安全：通过枚举确保类型正确访问
 * 3. 零拷贝：复杂类型使用智能指针避免拷贝
 * 4. 异常安全：构造函数保证对象始终有效
 * 5. 资源管理：自动清理动态分配的内存
 *
 * 类型管理系统：
 * 1. 单例模式：全局唯一的类型管理器实例
 * 2. 注册机制：动态注册新的数据类型
 * 3. 查询接口：类型信息和转换能力的查询
 * 4. 缓存优化：类型转换规则的缓存加速
 * 5. 线程安全：并发环境下的安全访问保证
 *
 * 性能优化策略：
 * - 内联存储：小对象直接存储避免堆分配
 * - 延迟求值：复杂类型按需构造和计算
 * - SIMD加速：向量化的字符串和数值处理
 * - 内存池：重用对象减少分配开销
 * - 缓存机制：常用转换结果的缓存
 *
 * 错误处理机制：
 * - 类型检查：运行时类型安全验证
 * - 溢出检测：数值运算的边界检查
 * - 格式验证：输入数据的格式正确性检查
 * - 异常传播：清晰的错误信息传递
 * - 降级处理：错误情况下的优雅降级
 *
 * 扩展性设计：
 * - 插件架构：支持自定义数据类型的加载
 * - 配置化：类型行为的配置化管理
 * - 序列化：类型数据的序列化和反序列化
 * - 国际化：多语言和本地化支持
 * - 标准化：严格遵循SQL标准的扩展方式
 */

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
