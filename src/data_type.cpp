#include "data_type.h"
#include <algorithm>
#include <iomanip>
#include <regex>
#include <sstream>

namespace sqlcc {

// 辅助函数
std::string DataTypeUtils::getStringValue(const DataType::Value &value) {
  try {
    return std::get<std::string>(value);
  } catch (const std::bad_variant_access &) {
    throw DataTypeException("Value is not a string type");
  }
}

// DataType 实现

DataType::DataType(Type type) : type_(type), param1_(0), param2_(0) {
  // 为某些类型设置默认参数
  switch (type) {
  case VARCHAR:
    param1_ = 255; // 默认VARCHAR长度
    break;
  case CHAR:
    param1_ = 1; // 默认CHAR长度
    break;
  case DECIMAL:
    param1_ = 10, param2_ = 2; // 默认DECIMAL精度和小数位数
    break;
  default:
    break;
  }
}

DataType::DataType(Type type, int param1, int param2)
    : type_(type), param1_(param1), param2_(param2) {
  // 基本参数验证
  if (param1 < 0 || param2 < 0) {
    throw DataTypeException("Data type parameters cannot be negative");
  }
}

DataType::DataType(const std::string &typeStr)
    : type_(INTEGER), param1_(0), param2_(0) {
  int param1 = 0, param2 = 0;
  DataType::Type parsedType = parseTypeString(typeStr, param1, param2);
  type_ = parsedType;
  param1_ = param1;
  param2_ = param2;
}

bool DataType::operator==(const DataType &other) const {
  return type_ == other.type_ && param1_ == other.param1_ &&
         param2_ == other.param2_;
}

bool DataType::operator!=(const DataType &other) const {
  return !(*this == other);
}

bool DataType::isNumeric() const {
  return type_ == INTEGER || type_ == SMALLINT || type_ == BIGINT ||
         type_ == DECIMAL || type_ == DOUBLE;
}

bool DataType::isString() const {
  return type_ == CHAR || type_ == VARCHAR || type_ == TEXT;
}

bool DataType::isDateTime() const {
  return type_ == DATE || type_ == TIME || type_ == TIMESTAMP;
}

bool DataType::isBoolean() const { return type_ == BOOLEAN; }

std::string DataType::toString() const {
  std::string result = DataTypeUtils::typeToString(type_);

  if (hasLength() || hasPrecision()) {
    result += "(" + std::to_string(param1_);
    if (hasScale()) {
      result += "," + std::to_string(param2_);
    }
    result += ")";
  }

  return result;
}

size_t DataType::estimateSize() const {
  switch (type_) {
  case INTEGER:
    return sizeof(int32_t);
  case SMALLINT:
    return sizeof(int16_t);
  case BIGINT:
    return sizeof(int64_t);
  case DECIMAL:
  case DOUBLE:
    return sizeof(double);
  case BOOLEAN:
    return sizeof(bool);
  case CHAR:
    return param1_; // 定长字符串
  case VARCHAR:
    return param1_ + 4; // 变长字符串需要额外的长度字段
  case TEXT:
    return 256; // 预估长文本大小（可调）
  case DATE:
    return 10; // "YYYY-MM-DD"
  case TIME:
    return 8; // "HH:MM:SS"
  case TIMESTAMP:
    return 19; // "YYYY-MM-DD HH:MM:SS"
  default:
    return 8; // 默认大小
  }
}

DataType::Type DataType::parseTypeString(const std::string &str, int &param1,
                                         int &param2) {
  param1 = 0;
  param2 = 0;

  // 移除参数部分，提取基本类型名
  std::string baseType = extractParams(str, param1, param2);
  return DataTypeUtils::stringToType(baseType);
}

std::string DataType::extractParams(const std::string &str, int &param1,
                                    int &param2) {
  std::string result = str;
  param1 = 0;
  param2 = 0;

  // 查找参数
  size_t openParen = str.find('(');
  if (openParen != std::string::npos) {
    size_t closeParen = str.find(')', openParen);
    if (closeParen != std::string::npos) {
      // 提取参数字符串
      std::string params =
          str.substr(openParen + 1, closeParen - openParen - 1);

      // 分离第一个和第二个参数
      size_t comma = params.find(',');
      if (comma != std::string::npos) {
        // 有两个参数
        try {
          param1 = std::stoi(params.substr(0, comma));
          param2 = std::stoi(params.substr(comma + 1));
        } catch (const std::exception &) {
          throw DataTypeException("Invalid numeric parameters in type: " + str);
        }
      } else {
        // 只有一个参数
        try {
          param1 = std::stoi(params);
        } catch (const std::exception &) {
          throw DataTypeException("Invalid numeric parameter in type: " + str);
        }
      }

      // 移除参数部分得到基本类型
      result = str.substr(0, openParen);
    }
  }

  return result;
}

// DataTypeUtils 实现

std::string DataTypeUtils::typeToString(DataType::Type type) {
  switch (type) {
  case DataType::INTEGER:
    return "INTEGER";
  case DataType::SMALLINT:
    return "SMALLINT";
  case DataType::BIGINT:
    return "BIGINT";
  case DataType::DECIMAL:
    return "DECIMAL";
  case DataType::DOUBLE:
    return "DOUBLE";
  case DataType::CHAR:
    return "CHAR";
  case DataType::VARCHAR:
    return "VARCHAR";
  case DataType::TEXT:
    return "TEXT";
  case DataType::DATE:
    return "DATE";
  case DataType::TIME:
    return "TIME";
  case DataType::TIMESTAMP:
    return "TIMESTAMP";
  case DataType::BOOLEAN:
    return "BOOLEAN";
  default:
    return "UNKNOWN";
  }
}

DataType::Type DataTypeUtils::stringToType(const std::string &str) {
  std::string upper = str;
  std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

  if (upper == "INT" || upper == "INTEGER")
    return DataType::INTEGER;
  if (upper == "SMALLINT")
    return DataType::SMALLINT;
  if (upper == "BIGINT")
    return DataType::BIGINT;
  if (upper == "DECIMAL" || upper == "NUMERIC")
    return DataType::DECIMAL;
  if (upper == "DOUBLE" || upper == "REAL" || upper == "FLOAT")
    return DataType::DOUBLE;
  if (upper == "CHAR")
    return DataType::CHAR;
  if (upper == "VARCHAR")
    return DataType::VARCHAR;
  if (upper == "TEXT")
    return DataType::TEXT;
  if (upper == "DATE")
    return DataType::DATE;
  if (upper == "TIME")
    return DataType::TIME;
  if (upper == "TIMESTAMP" || upper == "DATETIME")
    return DataType::TIMESTAMP;
  if (upper == "BOOLEAN" || upper == "BOOL")
    return DataType::BOOLEAN;

  throw DataTypeException("Unknown data type: " + str);
}

std::string DataTypeUtils::valueToString(const DataType::Value &value,
                                         DataType::Type type) {
  if (std::holds_alternative<std::monostate>(value)) {
    return "NULL";
  }

  try {
    switch (type) {
    case DataType::INTEGER: {
      return std::to_string(std::get<int32_t>(value));
    }
    case DataType::SMALLINT: {
      return std::to_string(std::get<int16_t>(value));
    }
    case DataType::BIGINT: {
      // BIGINT简化处理为string，因为variant不支持int64_t
      return std::to_string(std::get<int64_t>(value));
    }
    case DataType::DECIMAL:
    case DataType::DOUBLE: {
      std::ostringstream oss;
      oss << std::fixed << std::setprecision(2) << std::get<double>(value);
      return oss.str();
    }
    case DataType::CHAR:
    case DataType::VARCHAR:
    case DataType::TEXT:
    case DataType::DATE:
    case DataType::TIME:
    case DataType::TIMESTAMP: {
      return DataTypeUtils::getStringValue(value);
    }
    case DataType::BOOLEAN: {
      return std::get<bool>(value) ? "TRUE" : "FALSE";
    }
    default: {
      return "UNKNOWN";
    }
    }
  } catch (const std::bad_variant_access &) {
    return "TYPE_MISMATCH";
  }
}

DataType::Value DataTypeUtils::stringToValue(const std::string &str,
                                             DataType::Type type) {
  if (str == "NULL") {
    return std::monostate{};
  }

  try {
    switch (type) {
    case DataType::INTEGER: {
      return static_cast<int32_t>(std::stoi(str));
    }
    case DataType::SMALLINT: {
      return static_cast<int16_t>(std::stoi(str));
    }
    case DataType::BIGINT: {
      return static_cast<int64_t>(std::stoll(str));
    }
    case DataType::DECIMAL:
    case DataType::DOUBLE: {
      return std::stod(str);
    }
    case DataType::CHAR:
    case DataType::VARCHAR:
    case DataType::TEXT: {
      // 对于字符串类型，直接存储字符串值
      return std::string(str);
    }
    case DataType::BOOLEAN: {
      std::string upper = str;
      std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
      if (upper == "TRUE" || upper == "1" || upper == "YES") {
        return true;
      } else if (upper == "FALSE" || upper == "0" || upper == "NO") {
        return false;
      } else {
        throw DataTypeException("Invalid boolean value: " + str);
      }
    }
    case DataType::DATE:
    case DataType::TIME:
    case DataType::TIMESTAMP: {
      // 日期时间验证可以在这里添加
      // 目前简化处理，信任输入
      return std::string(str);
    }
    default: {
      return std::monostate{};
    }
    }
  } catch (const std::exception &e) {
    throw DataTypeException("Failed to convert string '" + str + "' to type '" +
                            typeToString(type) + "': " + e.what());
  }
}

bool DataTypeUtils::areCompatible(DataType::Type from, DataType::Type to) {
  // 相同类型总是兼容的
  if (from == to)
    return true;

  // 数值类型之间的兼容性
  if (isNumericType(from) && isNumericType(to)) {
    return true;
  }

  // 字符串类型兼容性
  if (isStringType(from) && isStringType(to)) {
    return true;
  }

  // 布尔类型的特殊处理
  if ((from == DataType::BOOLEAN && isNumericType(to)) ||
      (isNumericType(from) && to == DataType::BOOLEAN)) {
    return true;
  }

  return false;
}

DataType::Value DataTypeUtils::convertValue(const DataType::Value &value,
                                            DataType::Type fromType,
                                            DataType::Type toType) {
  if (fromType == toType) {
    return value;
  }

  // 如果是NULL值，直接返回
  if (std::holds_alternative<std::monostate>(value)) {
    return value;
  }

  // 数字类型转换
  if (isNumericType(fromType) && isNumericType(toType)) {
    double numVal = extractNumericValue(value, fromType);
    return convertToNumericType(numVal, toType);
  }

  // 布尔类型转换
  if (fromType == DataType::BOOLEAN && isNumericType(toType)) {
    bool boolVal = std::get<bool>(value);
    return convertToNumericType(boolVal ? 1.0 : 0.0, toType);
  }

  if (isNumericType(fromType) && toType == DataType::BOOLEAN) {
    double numVal = extractNumericValue(value, fromType);
    return numVal != 0.0;
  }

  // 不支持的转换返回原值（错误处理应该在调用者处）
  return value;
}

bool DataTypeUtils::isNumericType(DataType::Type type) {
  return type == DataType::INTEGER || type == DataType::SMALLINT ||
         type == DataType::BIGINT || type == DataType::DECIMAL ||
         type == DataType::DOUBLE;
}

bool DataTypeUtils::isStringType(DataType::Type type) {
  return type == DataType::CHAR || type == DataType::VARCHAR ||
         type == DataType::TEXT;
}

double DataTypeUtils::extractNumericValue(const DataType::Value &value,
                                          DataType::Type type) {
  switch (type) {
  case DataType::INTEGER:
    return static_cast<double>(std::get<int32_t>(value));
  case DataType::SMALLINT:
    return static_cast<double>(std::get<int16_t>(value));
  case DataType::BIGINT:
    return static_cast<double>(std::get<int64_t>(value));
  case DataType::DECIMAL:
  case DataType::DOUBLE:
    return std::get<double>(value);
  default:
    return 0.0;
  }
}

DataType::Value DataTypeUtils::convertToNumericType(double value,
                                                    DataType::Type toType) {
  switch (toType) {
  case DataType::INTEGER:
    return static_cast<int32_t>(value);
  case DataType::SMALLINT:
    return static_cast<int16_t>(value);
  case DataType::BIGINT:
    return static_cast<int64_t>(value);
  case DataType::DECIMAL:
  case DataType::DOUBLE:
    return value;
  default:
    return value;
  }
}

} // namespace sqlcc
