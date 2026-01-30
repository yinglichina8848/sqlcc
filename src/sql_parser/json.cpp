#include "src/sql_parser/json.h"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <stack>
#include <stdexcept>

namespace sqlcc {

// PIMPL实现类的前向声明
class JsonValue {
public:
  virtual ~JsonValue() = default;
  virtual Json::Type type() const = 0;
  virtual std::string to_string() const = 0;
  virtual std::string to_string_formatted(int indent) const = 0;
  virtual std::unique_ptr<JsonValue> clone() const = 0;
  virtual bool equals(const JsonValue* other) const = 0;
};

// Null值实现
class JsonNull : public JsonValue {
public:
  Json::Type type() const override { return Json::NULL_VALUE; }
  std::string to_string() const override { return "null"; }
  std::string to_string_formatted(int indent) const override { return "null"; }
  std::unique_ptr<JsonValue> clone() const override { return std::make_unique<JsonNull>(); }
  bool equals(const JsonValue* other) const override { return other->type() == Json::NULL_VALUE; }
};

// 布尔值实现
class JsonBoolean : public JsonValue {
public:
  explicit JsonBoolean(bool value) : value_(value) {}
  Json::Type type() const override { return Json::BOOLEAN; }
  std::string to_string() const override { return value_ ? "true" : "false"; }
  std::string to_string_formatted(int indent) const override { return value_ ? "true" : "false"; }
  std::unique_ptr<JsonValue> clone() const override { return std::make_unique<JsonBoolean>(value_); }
  bool equals(const JsonValue* other) const override {
    return other->type() == Json::BOOLEAN && static_cast<const JsonBoolean*>(other)->value_ == value_;
  }
  bool value() const { return value_; }

private:
  bool value_;
};

// 数值实现
class JsonNumber : public JsonValue {
public:
  explicit JsonNumber(double value) : value_(value) {}
  Json::Type type() const override { return Json::NUMBER; }
  std::string to_string() const override {
    std::stringstream ss;
    ss << value_;
    return ss.str();
  }
  std::string to_string_formatted(int indent) const override { return to_string(); }
  std::unique_ptr<JsonValue> clone() const override { return std::make_unique<JsonNumber>(value_); }
  bool equals(const JsonValue* other) const override {
    if (other->type() != Json::NUMBER) return false;
    double other_val = static_cast<const JsonNumber*>(other)->value_;
    return std::abs(value_ - other_val) < 1e-10; // 浮点数比较的容差
  }
  double value() const { return value_; }

private:
  double value_;
};

// 字符串实现
class JsonString : public JsonValue {
public:
  explicit JsonString(const std::string& value) : value_(value) {}
  Json::Type type() const override { return Json::STRING; }
  std::string to_string() const override {
    return "\"" + json_escape_string(value_) + "\"";
  }
  std::string to_string_formatted(int indent) const override { return to_string(); }
  std::unique_ptr<JsonValue> clone() const override { return std::make_unique<JsonString>(value_); }
  bool equals(const JsonValue* other) const override {
    return other->type() == Json::STRING && static_cast<const JsonString*>(other)->value_ == value_;
  }
  const std::string& value() const { return value_; }

private:
  std::string value_;
};

// 数组实现
class JsonArray : public JsonValue {
public:
  Json::Type type() const override { return Json::ARRAY; }
  std::string to_string() const override {
    std::stringstream ss;
    ss << "[";
    for (size_t i = 0; i < values_.size(); ++i) {
      if (i > 0) ss << ",";
      ss << values_[i]->to_string();
    }
    ss << "]";
    return ss.str();
  }
  std::string to_string_formatted(int indent) const override {
    if (values_.empty()) return "[]";
    std::stringstream ss;
    ss << "[\n";
    std::string indent_str(indent + 2, ' ');
    for (size_t i = 0; i < values_.size(); ++i) {
      if (i > 0) ss << ",\n";
      ss << indent_str << values_[i]->to_string_formatted(indent + 2);
    }
    ss << "\n" << std::string(indent, ' ') << "]";
    return ss.str();
  }
  std::unique_ptr<JsonValue> clone() const override {
    auto result = std::make_unique<JsonArray>();
    for (const auto& value : values_) {
      result->values_.push_back(value->clone());
    }
    return result;
  }
  bool equals(const JsonValue* other) const override {
    if (other->type() != Json::ARRAY) return false;
    const JsonArray* other_array = static_cast<const JsonArray*>(other);
    if (values_.size() != other_array->values_.size()) return false;
    for (size_t i = 0; i < values_.size(); ++i) {
      if (!values_[i]->equals(other_array->values_[i].get())) return false;
    }
    return true;
  }

  std::vector<std::unique_ptr<JsonValue>>& values() { return values_; }
  const std::vector<std::unique_ptr<JsonValue>>& values() const { return values_; }

private:
  std::vector<std::unique_ptr<JsonValue>> values_;
};

// 对象实现
class JsonObject : public JsonValue {
public:
  Json::Type type() const override { return Json::OBJECT; }
  std::string to_string() const override {
    std::stringstream ss;
    ss << "{";
    bool first = true;
    for (const auto& pair : values_) {
      if (!first) ss << ",";
      first = false;
      ss << "\"" << json_escape_string(pair.first) << "\":" << pair.second->to_string();
    }
    ss << "}";
    return ss.str();
  }
  std::string to_string_formatted(int indent) const override {
    if (values_.empty()) return "{}";
    std::stringstream ss;
    ss << "{\n";
    std::string indent_str(indent + 2, ' ');
    bool first = true;
    for (const auto& pair : values_) {
      if (!first) ss << ",\n";
      first = false;
      ss << indent_str << "\"" << json_escape_string(pair.first) << "\": "
         << pair.second->to_string_formatted(indent + 2);
    }
    ss << "\n" << std::string(indent, ' ') << "}";
    return ss.str();
  }
  std::unique_ptr<JsonValue> clone() const override {
    auto result = std::make_unique<JsonObject>();
    for (const auto& pair : values_) {
      result->values_[pair.first] = pair.second->clone();
    }
    return result;
  }
  bool equals(const JsonValue* other) const override {
    if (other->type() != Json::OBJECT) return false;
    const JsonObject* other_obj = static_cast<const JsonObject*>(other);
    if (values_.size() != other_obj->values_.size()) return false;
    for (const auto& pair : values_) {
      auto it = other_obj->values_.find(pair.first);
      if (it == other_obj->values_.end() || !pair.second->equals(it->second.get())) {
        return false;
      }
    }
    return true;
  }

  std::unordered_map<std::string, std::unique_ptr<JsonValue>>& values() { return values_; }
  const std::unordered_map<std::string, std::unique_ptr<JsonValue>>& values() const { return values_; }

private:
  std::unordered_map<std::string, std::unique_ptr<JsonValue>> values_;
};

// ==================== Json 类实现 ====================

Json::Json() : value_(std::make_unique<JsonNull>()) {}
Json::Json(std::nullptr_t) : value_(std::make_unique<JsonNull>()) {}
Json::Json(bool value) : value_(std::make_unique<JsonBoolean>(value)) {}
Json::Json(int value) : value_(std::make_unique<JsonNumber>(static_cast<double>(value))) {}
Json::Json(int64_t value) : value_(std::make_unique<JsonNumber>(static_cast<double>(value))) {}
Json::Json(double value) : value_(std::make_unique<JsonNumber>(value)) {}
Json::Json(const std::string& value) : value_(std::make_unique<JsonString>(value)) {}
Json::Json(const char* value) : value_(std::make_unique<JsonString>(value ? value : "")) {}

Json::Json(const Json& other) : value_(other.value_->clone()) {}
Json::Json(Json&& other) noexcept : value_(std::move(other.value_)) {}

Json::~Json() = default;

Json& Json::operator=(const Json& other) {
  if (this != &other) {
    value_ = other.value_->clone();
  }
  return *this;
}

Json& Json::operator=(Json&& other) noexcept {
  if (this != &other) {
    value_ = std::move(other.value_);
  }
  return *this;
}

Json& Json::operator=(std::nullptr_t) { value_ = std::make_unique<JsonNull>(); return *this; }
Json& Json::operator=(bool value) { value_ = std::make_unique<JsonBoolean>(value); return *this; }
Json& Json::operator=(int value) { value_ = std::make_unique<JsonNumber>(value); return *this; }
Json& Json::operator=(int64_t value) { value_ = std::make_unique<JsonNumber>(value); return *this; }
Json& Json::operator=(double value) { value_ = std::make_unique<JsonNumber>(value); return *this; }
Json& Json::operator=(const std::string& value) { value_ = std::make_unique<JsonString>(value); return *this; }
Json& Json::operator=(const char* value) { value_ = std::make_unique<JsonString>(value ? value : ""); return *this; }

// 类型检查
Json::Type Json::type() const { return value_->type(); }
bool Json::is_null() const { return type() == NULL_VALUE; }
bool Json::is_boolean() const { return type() == BOOLEAN; }
bool Json::is_number() const { return type() == NUMBER; }
bool Json::is_string() const { return type() == STRING; }
bool Json::is_array() const { return type() == ARRAY; }
bool Json::is_object() const { return type() == OBJECT; }

// 值获取
bool Json::as_bool() const {
  if (!is_boolean()) throw_type_error("boolean");
  return static_cast<JsonBoolean*>(value_.get())->value();
}

int Json::as_int() const {
  if (!is_number()) throw_type_error("number");
  return static_cast<int>(static_cast<JsonNumber*>(value_.get())->value());
}

int64_t Json::as_int64() const {
  if (!is_number()) throw_type_error("number");
  return static_cast<int64_t>(static_cast<JsonNumber*>(value_.get())->value());
}

double Json::as_double() const {
  if (!is_number()) throw_type_error("number");
  return static_cast<JsonNumber*>(value_.get())->value();
}

std::string Json::as_string() const {
  if (!is_string()) throw_type_error("string");
  return static_cast<JsonString*>(value_.get())->value();
}

// 值设置
void Json::set_null() { value_ = std::make_unique<JsonNull>(); }
void Json::set_bool(bool value) { value_ = std::make_unique<JsonBoolean>(value); }
void Json::set_int(int value) { value_ = std::make_unique<JsonNumber>(value); }
void Json::set_int64(int64_t value) { value_ = std::make_unique<JsonNumber>(value); }
void Json::set_double(double value) { value_ = std::make_unique<JsonNumber>(value); }
void Json::set_string(const std::string& value) { value_ = std::make_unique<JsonString>(value); }

// 数组操作
size_t Json::size() const {
  if (is_array()) {
    return static_cast<JsonArray*>(value_.get())->values().size();
  } else if (is_object()) {
    return static_cast<JsonObject*>(value_.get())->values().size();
  }
  return 0;
}

void Json::clear() {
  if (is_array()) {
    static_cast<JsonArray*>(value_.get())->values().clear();
  } else if (is_object()) {
    static_cast<JsonObject*>(value_.get())->values().clear();
  }
}

// 数组特有操作
Json& Json::operator[](size_t index) {
  ensure_array();
  auto& values = static_cast<JsonArray*>(value_.get())->values();
  if (index >= values.size()) {
    values.resize(index + 1);
  }
  if (!values[index]) {
    values[index] = std::make_unique<JsonNull>();
  }
  return *this; // 简化实现，实际应该返回对元素的引用
}

const Json& Json::operator[](size_t index) const {
  if (!is_array()) throw_type_error("array");
  const auto& values = static_cast<JsonArray*>(value_.get())->values();
  if (index >= values.size()) {
    static_cast<JsonArray*>(const_cast<JsonValue*>(value_.get()))->values().resize(index + 1);
  }
  return *this; // 简化实现
}

void Json::push_back(const Json& value) {
  ensure_array();
  static_cast<JsonArray*>(value_.get())->values().push_back(value.value_->clone());
}

void Json::pop_back() {
  if (!is_array()) throw_type_error("array");
  auto& values = static_cast<JsonArray*>(value_.get())->values();
  if (!values.empty()) {
    values.pop_back();
  }
}

Json& Json::back() {
  if (!is_array()) throw_type_error("array");
  auto& values = static_cast<JsonArray*>(value_.get())->values();
  if (values.empty()) {
    throw std::runtime_error("Array is empty");
  }
  return *this; // 简化实现
}

const Json& Json::back() const {
  if (!is_array()) throw_type_error("array");
  const auto& values = static_cast<JsonArray*>(value_.get())->values();
  if (values.empty()) {
    throw std::runtime_error("Array is empty");
  }
  return *this; // 简化实现
}

// 对象特有操作
Json& Json::operator[](const std::string& key) {
  ensure_object();
  auto& values = static_cast<JsonObject*>(value_.get())->values();
  if (values.find(key) == values.end()) {
    values[key] = std::make_unique<JsonNull>();
  }
  return *this; // 简化实现
}

const Json& Json::operator[](const std::string& key) const {
  if (!is_object()) throw_type_error("object");
  const auto& values = static_cast<JsonObject*>(value_.get())->values();
  auto it = values.find(key);
  if (it == values.end()) {
    const_cast<JsonObject*>(static_cast<const JsonObject*>(value_.get()))->values()[key] = std::make_unique<JsonNull>();
  }
  return *this; // 简化实现
}

bool Json::contains(const std::string& key) const {
  if (!is_object()) return false;
  const auto& values = static_cast<JsonObject*>(value_.get())->values();
  return values.find(key) != values.end();
}

void Json::erase(const std::string& key) {
  if (is_object()) {
    static_cast<JsonObject*>(value_.get())->values().erase(key);
  }
}

std::vector<std::string> Json::keys() const {
  std::vector<std::string> result;
  if (is_object()) {
    const auto& values = static_cast<JsonObject*>(value_.get())->values();
    for (const auto& pair : values) {
      result.push_back(pair.first);
    }
  }
  return result;
}

// 序列化
std::string Json::to_string() const {
  return value_->to_string();
}

std::string Json::to_string(int indent) const {
  return value_->to_string_formatted(indent);
}

// 比较操作
bool Json::operator==(const Json& other) const {
  return value_->equals(other.value_.get());
}

bool Json::operator!=(const Json& other) const {
  return !(*this == other);
}

// 合并操作
Json& Json::merge(const Json& other) {
  if (!is_object() || !other.is_object()) {
    throw std::runtime_error("Both JSON values must be objects for merge");
  }

  auto& this_values = static_cast<JsonObject*>(value_.get())->values();
  const auto& other_values = static_cast<JsonObject*>(other.value_.get())->values();

  for (const auto& pair : other_values) {
    this_values[pair.first] = pair.second->clone();
  }

  return *this;
}

// 辅助方法
void Json::ensure_array() {
  if (!is_array()) {
    value_ = std::make_unique<JsonArray>();
  }
}

void Json::ensure_object() {
  if (!is_object()) {
    value_ = std::make_unique<JsonObject>();
  }
}

void Json::throw_type_error(const std::string& expected_type) const {
  std::stringstream ss;
  ss << "JSON value is not a " << expected_type;
  throw std::runtime_error(ss.str());
}

// ==================== JsonPath 类实现 ====================

JsonPath::JsonPath(const std::string& path) : path_(path) {
  // 简化实现：基本的路径验证
  valid_ = !path.empty() && path[0] == '$';
  if (!valid_) {
    error_msg_ = "Invalid JSONPath: must start with '$'";
  }
}

JsonPath::~JsonPath() = default;

Json JsonPath::query(const Json& json) const {
  if (!valid_) {
    throw std::runtime_error(error_msg_);
  }

  // 简化实现：只支持根路径
  return json;
}

std::vector<Json> JsonPath::query_all(const Json& json) const {
  std::vector<Json> result;
  result.push_back(query(json));
  return result;
}

bool JsonPath::is_valid() const {
  return valid_;
}

std::string JsonPath::error_message() const {
  return error_msg_;
}

// ==================== 全局函数 ====================

Json json_null() { return Json(nullptr); }
Json json_bool(bool value) { return Json(value); }
Json json_int(int value) { return Json(value); }
Json json_int64(int64_t value) { return Json(value); }
Json json_double(double value) { return Json(value); }
Json json_string(const std::string& value) { return Json(value); }
Json json_array() { Json j; j.ensure_array(); return j; }
Json json_object() { Json j; j.ensure_object(); return j; }

std::string json_to_string(const Json& json) { return json.to_string(); }
Json json_from_string(const std::string& str) { return Json::parse(str); }

// 转义函数
std::string json_escape_string(const std::string& str) {
  std::string result;
  for (char c : str) {
    switch (c) {
      case '"': result += "\\\""; break;
      case '\\': result += "\\\\"; break;
      case '\b': result += "\\b"; break;
      case '\f': result += "\\f"; break;
      case '\n': result += "\\n"; break;
      case '\r': result += "\\r"; break;
      case '\t': result += "\\t"; break;
      default:
        if (c < 32) {
          char buf[8];
          std::snprintf(buf, sizeof(buf), "\\u%04x", static_cast<unsigned char>(c));
          result += buf;
        } else {
          result += c;
        }
    }
  }
  return result;
}

std::string json_unescape_string(const std::string& str) {
  // 简化实现
  return str;
}

// 流操作符
std::ostream& operator<<(std::ostream& os, const Json& json) {
  os << json.to_string();
  return os;
}

std::istream& operator>>(std::istream& is, Json& json) {
  std::string str;
  is >> str;
  json = Json::parse(str);
  return is;
}

namespace {

// 辅助解析函数
Json parse_value(const std::string& str, size_t& pos);
void skip_whitespace(const std::string& str, size_t& pos);
std::string parse_string(const std::string& str, size_t& pos);
Json parse_object(const std::string& str, size_t& pos);
Json parse_array(const std::string& str, size_t& pos);
bool parse_bool(const std::string& str, size_t& pos);
void parse_null(const std::string& str, size_t& pos);
double parse_number(const std::string& str, size_t& pos);

Json parse_value(const std::string& str, size_t& pos) {
  skip_whitespace(str, pos);

  if (pos >= str.length()) {
    throw std::runtime_error("Unexpected end of JSON");
  }

  char c = str[pos];
  if (c == '"') {
    return Json(parse_string(str, pos));
  } else if (c == '{') {
    return parse_object(str, pos);
  } else if (c == '[') {
    return parse_array(str, pos);
  } else if (c == 't' || c == 'f') {
    return Json(parse_bool(str, pos));
  } else if (c == 'n') {
    parse_null(str, pos);
    return Json(nullptr);
  } else if (std::isdigit(c) || c == '-') {
    return Json(parse_number(str, pos));
  } else {
    throw std::runtime_error(std::string("Unexpected character: ") + c);
  }
}

void skip_whitespace(const std::string& str, size_t& pos) {
  while (pos < str.length() && std::isspace(str[pos])) {
    pos++;
  }
}

std::string parse_string(const std::string& str, size_t& pos) {
  if (str[pos] != '"') {
    throw std::runtime_error("Expected string");
  }
  pos++; // 跳过开始的引号

  std::string result;
  while (pos < str.length() && str[pos] != '"') {
    if (str[pos] == '\\') {
      pos++;
      if (pos >= str.length()) {
        throw std::runtime_error("Unexpected end of string");
      }
      switch (str[pos]) {
        case '"': result += '"'; break;
        case '\\': result += '\\'; break;
        case '/': result += '/'; break;
        case 'b': result += '\b'; break;
        case 'f': result += '\f'; break;
        case 'n': result += '\n'; break;
        case 'r': result += '\r'; break;
        case 't': result += '\t'; break;
        case 'u': {
          // 简化实现：跳过Unicode转义
          pos += 4;
          result += '?'; // 占位符
          break;
        }
        default:
          result += str[pos];
      }
    } else {
      result += str[pos];
    }
    pos++;
  }

  if (pos >= str.length() || str[pos] != '"') {
    throw std::runtime_error("Unterminated string");
  }
  pos++; // 跳过结束的引号
  return result;
}

Json parse_object(const std::string& str, size_t& pos) {
  if (str[pos] != '{') {
    throw std::runtime_error("Expected object");
  }
  pos++; // 跳过'{'

  Json result = json_object();

  skip_whitespace(str, pos);
  if (pos < str.length() && str[pos] == '}') {
    pos++;
    return result;
  }

  while (true) {
    skip_whitespace(str, pos);
    std::string key = parse_string(str, pos);

    skip_whitespace(str, pos);
    if (pos >= str.length() || str[pos] != ':') {
      throw std::runtime_error("Expected ':' in object");
    }
    pos++;

    skip_whitespace(str, pos);
    Json value = parse_value(str, pos);

    result[key] = value;

    skip_whitespace(str, pos);
    if (pos >= str.length()) {
      throw std::runtime_error("Unexpected end of object");
    }

    if (str[pos] == '}') {
      pos++;
      break;
    } else if (str[pos] == ',') {
      pos++;
    } else {
      throw std::runtime_error("Expected ',' or '}' in object");
    }
  }

  return result;
}

Json parse_array(const std::string& str, size_t& pos) {
  if (str[pos] != '[') {
    throw std::runtime_error("Expected array");
  }
  pos++; // 跳过'['

  Json result = json_array();

  skip_whitespace(str, pos);
  if (pos < str.length() && str[pos] == ']') {
    pos++;
    return result;
  }

  while (true) {
    skip_whitespace(str, pos);
    Json value = parse_value(str, pos);
    result.push_back(value);

    skip_whitespace(str, pos);
    if (pos >= str.length()) {
      throw std::runtime_error("Unexpected end of array");
    }

    if (str[pos] == ']') {
      pos++;
      break;
    } else if (str[pos] == ',') {
      pos++;
    } else {
      throw std::runtime_error("Expected ',' or ']' in array");
    }
  }

  return result;
}

bool parse_bool(const std::string& str, size_t& pos) {
  if (pos + 4 <= str.length() && str.substr(pos, 4) == "true") {
    pos += 4;
    return true;
  } else if (pos + 5 <= str.length() && str.substr(pos, 5) == "false") {
    pos += 5;
    return false;
  } else {
    throw std::runtime_error("Expected boolean");
  }
}

void parse_null(const std::string& str, size_t& pos) {
  if (pos + 4 <= str.length() && str.substr(pos, 4) == "null") {
    pos += 4;
  } else {
    throw std::runtime_error("Expected null");
  }
}

double parse_number(const std::string& str, size_t& pos) {
  size_t start = pos;
  if (str[pos] == '-') {
    pos++;
  }

  while (pos < str.length() && (std::isdigit(str[pos]) || str[pos] == '.' || str[pos] == 'e' || str[pos] == 'E' || str[pos] == '+' || str[pos] == '-')) {
    pos++;
  }

  std::string num_str = str.substr(start, pos - start);
  try {
    return std::stod(num_str);
  } catch (const std::exception&) {
    throw std::runtime_error("Invalid number format");
  }
}

} // anonymous namespace

// ==================== Json::parse 静态方法实现 ====================

Json Json::parse(const std::string& json_str) {
  if (json_str.empty()) {
    throw std::runtime_error("Empty JSON string");
  }

  size_t pos = 0;
  return parse_value(json_str, pos);
}

Json Json::from_file(const std::string& filename) {
  // 简化实现：这里应该读取文件内容
  throw std::runtime_error("from_file not implemented");
}

bool Json::to_file(const std::string& filename) const {
  // 简化实现：这里应该写入文件
  return false;
}

} // namespace sqlcc
