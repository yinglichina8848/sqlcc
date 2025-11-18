#pragma once

#include <memory>
#include <stdexcept>
#include <string>
#include <variant>

namespace sqlcc {

/**
 * @brief SQL数据类型体系
 *
 * 支持完整的SQL-92数据类型，包括精确数值、近似数值、字符串、日期时间等类型
 */
class DataType {
public:
  /**
   * @brief 数据类型枚举
   */
  enum Type {
    // 精确数值类型
    INTEGER,  // INT, INTEGER - 32位整数
    SMALLINT, // SMALLINT - 16位整数
    BIGINT,   // BIGINT - 64位整数（预留）

    // 近似数值类型
    DECIMAL, // DECIMAL(p,s) - 定点小数
    DOUBLE,  // DOUBLE, REAL - 双精度浮点数

    // 字符串类型
    CHAR,    // CHAR(n) - 定长字符串
    VARCHAR, // VARCHAR(n) - 变长字符串
    TEXT,    // TEXT - 长文本（预留）

    // 日期时间类型
    DATE,      // DATE - 日期
    TIME,      // TIME - 时间
    TIMESTAMP, // TIMESTAMP - 时间戳

    // 布尔类型
    BOOLEAN // BOOLEAN - 布尔值

    // 未来扩展类型:
    // BLOB, CLOB, BINARY, VARBINARY, etc.
  };

  /**
   * @brief 默认构造函数
   */
  DataType() = default;

  /**
   * @brief 构造函数 - 基础类型
   * @param type 数据类型
   */
  explicit DataType(Type type);

  /**
   * @brief 构造函数 - 带长度/精度的类型
   * @param type 数据类型
   * @param param1 第一个参数（如长度、精度）
   * @param param2 第二个参数（如小数位数）
   */
  DataType(Type type, int param1, int param2 = 0);

  /**
   * @brief 构造函数 - 从字符串创建
   * @param typeStr 类型字符串（如"INT", "VARCHAR(100)", "DECIMAL(10,2)"）
   */
  explicit DataType(const std::string &typeStr);

  // 运算符重载
  bool operator==(const DataType &other) const;
  bool operator!=(const DataType &other) const;

  // 获取方法
  Type getType() const { return type_; }
  bool hasLength() const { return param1_ > 0; }
  int getLength() const { return param1_; }
  bool hasPrecision() const { return param1_ > 0; }
  int getPrecision() const { return param1_; }
  bool hasScale() const { return param2_ > 0; }
  int getScale() const { return param2_; }

  // 类型信息查询
  bool isNumeric() const;
  bool isString() const;
  bool isDateTime() const;
  bool isBoolean() const;

  // 类型转换和验证
  std::string toString() const;
  size_t estimateSize() const;

  /**
   * @brief 数据值表示
   *
   * 使用std::variant支持多种数据类型的统一表示
   */
  using Value =
      std::variant<std::monostate, // NULL值
                   int32_t,        // INTEGER
                   int16_t,        // SMALLINT
                   int64_t,        // BIGINT
                   double,         // DECIMAL/DOUBLE
                   std::string,    // CHAR/VARCHAR/TEXT/DATE/TIME/TIMESTAMP
                   bool>;          // BOOLEAN

private:
  Type type_ = INTEGER;
  int param1_ = 0; // 长度、精度等参数
  int param2_ = 0; // 小数位数等第二个参数

  // 解析类型字符串
  static Type parseTypeString(const std::string &str, int &param1, int &param2);
  static std::string extractParams(const std::string &str, int &param1,
                                   int &param2);
};

/**
 * @brief 数据类型异常
 */
class DataTypeException : public std::runtime_error {
public:
  explicit DataTypeException(const std::string &message)
      : std::runtime_error(message) {}
};

/**
 * @brief 数据类型帮助函数
 */
class DataTypeUtils {
public:
  /**
   * @brief 获取类型的字符串表示
   */
  static std::string typeToString(DataType::Type type);

  /**
   * @brief 从字符串解析类型
   */
  static DataType::Type stringToType(const std::string &str);

  /**
   * @brief 值转换为字符串
   */
  static std::string valueToString(const DataType::Value &value,
                                   DataType::Type type);

  /**
   * @brief 字符串转换为值
   */
  static DataType::Value stringToValue(const std::string &str,
                                       DataType::Type type);

  /**
   * @brief 检查类型兼容性
   */
  static bool areCompatible(DataType::Type from, DataType::Type to);

  /**
   * @brief 执行类型转换
   */
  static DataType::Value convertValue(const DataType::Value &value,
                                      DataType::Type fromType,
                                      DataType::Type toType);
};

} // namespace sqlcc
