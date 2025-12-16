#pragma once

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <cstdint>
#include <regex>

namespace sqlcc {

class JsonValue;

/**
 * @brief JSON数据类型
 */
class Json {
public:
  // JSON类型枚举
  enum Type {
    NULL_VALUE,
    BOOLEAN,
    NUMBER,
    STRING,
    ARRAY,
    OBJECT
  };

  // 构造函数
  Json();
  Json(std::nullptr_t);  // null值
  Json(bool value);      // 布尔值
  Json(int value);       // 整数
  Json(int64_t value);   // 长整数
  Json(double value);    // 浮点数
  Json(const std::string& value); // 字符串
  Json(const char* value);        // C字符串
  Json(const Json& other);        // 拷贝构造
  Json(Json&& other) noexcept;    // 移动构造

  // 析构函数
  ~Json();

  // 赋值操作符
  Json& operator=(const Json& other);
  Json& operator=(Json&& other) noexcept;
  Json& operator=(std::nullptr_t);
  Json& operator=(bool value);
  Json& operator=(int value);
  Json& operator=(int64_t value);
  Json& operator=(double value);
  Json& operator=(const std::string& value);
  Json& operator=(const char* value);

  // 类型检查
  Type type() const;
  bool is_null() const;
  bool is_boolean() const;
  bool is_number() const;
  bool is_string() const;
  bool is_array() const;
  bool is_object() const;

  // 值获取（带类型检查）
  bool as_bool() const;
  int as_int() const;
  int64_t as_int64() const;
  double as_double() const;
  std::string as_string() const;

  // 值设置
  void set_null();
  void set_bool(bool value);
  void set_int(int value);
  void set_int64(int64_t value);
  void set_double(double value);
  void set_string(const std::string& value);

  // 数组操作
  size_t size() const;  // 数组大小或对象键值对数量
  void clear();         // 清空数组或对象

  // 数组特有操作
  Json& operator[](size_t index);           // 数组元素访问
  const Json& operator[](size_t index) const;
  void push_back(const Json& value);        // 添加数组元素
  void pop_back();                          // 删除最后一个元素
  Json& back();                             // 获取最后一个元素
  const Json& back() const;

  // 对象特有操作
  Json& operator[](const std::string& key);           // 对象属性访问
  const Json& operator[](const std::string& key) const;
  bool contains(const std::string& key) const;        // 检查键是否存在
  void erase(const std::string& key);                 // 删除键值对
  std::vector<std::string> keys() const;              // 获取所有键

  // JSON路径查询
  Json path(const std::string& json_path) const;      // JSONPath查询
  std::vector<Json> find(const std::string& json_path) const; // 查找多个匹配项

  // 序列化
  std::string to_string() const;                      // 转换为JSON字符串
  std::string to_string(int indent) const;           // 格式化输出

  // 解析
  static Json parse(const std::string& json_str);    // 从字符串解析JSON
  static Json from_file(const std::string& filename); // 从文件加载JSON

  // 文件操作
  bool to_file(const std::string& filename) const;    // 保存到文件

  // 比较操作
  bool operator==(const Json& other) const;
  bool operator!=(const Json& other) const;

  // 合并操作
  Json& merge(const Json& other);  // 合并两个JSON对象

private:
  std::unique_ptr<JsonValue> value_;  // PIMPL模式实现

  // 内部辅助方法
  void throw_type_error(const std::string& expected_type) const;

public:
  // 数组/对象初始化辅助方法
  void ensure_array();
  void ensure_object();
};

/**
 * @brief JSON路径查询类
 */
class JsonPath {
public:
  JsonPath(const std::string& path);
  ~JsonPath();

  // 查询执行
  Json query(const Json& json) const;
  std::vector<Json> query_all(const Json& json) const;

  // 路径验证
  bool is_valid() const;
  std::string error_message() const;

private:
  std::string path_;
  bool valid_;
  std::string error_msg_;

  // 路径解析和执行
  Json execute_path(const Json& root) const;
  std::vector<Json> execute_path_all(const Json& root) const;

  // 路径段解析
  std::vector<std::string> parse_path_segments() const;
  std::vector<size_t> parse_array_indices(const std::string& segment) const;
};

/**
 * @brief JSON模式验证类
 */
class JsonSchema {
public:
  JsonSchema(const Json& schema);
  ~JsonSchema();

  // 验证
  bool validate(const Json& json) const;
  std::string validation_error() const;

  // 模式检查
  bool is_valid_schema() const;

private:
  Json schema_;
  mutable std::string last_error_;

  // 递归验证
  bool validate_value(const Json& value, const Json& schema_part) const;
  bool validate_object(const Json& obj, const Json& schema_part) const;
  bool validate_array(const Json& arr, const Json& schema_part) const;
};

/**
 * @brief JSON构建器类（流式API）
 */
class JsonBuilder {
public:
  JsonBuilder();
  ~JsonBuilder();

  // 开始构建对象
  JsonBuilder& object();
  JsonBuilder& end_object();

  // 开始构建数组
  JsonBuilder& array();
  JsonBuilder& end_array();

  // 添加属性
  JsonBuilder& key(const std::string& key);
  JsonBuilder& value(const Json& value);

  // 直接添加值
  JsonBuilder& add_null();
  JsonBuilder& add_bool(bool value);
  JsonBuilder& add_int(int value);
  JsonBuilder& add_int64(int64_t value);
  JsonBuilder& add_double(double value);
  JsonBuilder& add_string(const std::string& value);

  // 获取结果
  Json build();

private:
  Json result_;
  std::vector<std::string> key_stack_;
  std::vector<size_t> array_indices_;
  bool in_object_;
  bool in_array_;
  std::string current_key_;
};

// 流操作符
std::ostream& operator<<(std::ostream& os, const Json& json);
std::istream& operator>>(std::istream& is, Json& json);

// 工具函数
Json json_null();
Json json_bool(bool value);
Json json_int(int value);
Json json_int64(int64_t value);
Json json_double(double value);
Json json_string(const std::string& value);
Json json_array();
Json json_object();

// 字符串转换函数
std::string json_to_string(const Json& json);
Json json_from_string(const std::string& str);

// 转义和反转义
std::string json_escape_string(const std::string& str);
std::string json_unescape_string(const std::string& str);

} // namespace sqlcc
