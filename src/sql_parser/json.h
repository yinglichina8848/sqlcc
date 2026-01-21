/**
 * WHY: 为什么需要专门的JSON数据处理系统？
 *
 * 现代数据库系统需要支持JSON数据类型作为SQL标准的一部分，传统方案存在诸多问题：
 * - 数据格式限制：只能存储简单数据类型，无法处理复杂嵌套结构
 * - 查询能力不足：缺少对JSON内部结构的查询和索引支持
 * - 类型安全缺失：运行时JSON数据缺乏类型验证和约束
 * - 性能开销大：JSON解析和序列化效率低下，影响整体性能
 * - 标准兼容性差：不支持SQL/JSON标准路径查询和函数
 *
 * JSON数据处理系统的核心价值：
 * 1. 数据灵活性：支持任意嵌套的JSON文档存储和查询
 * 2. 查询能力：提供强大的JSON路径查询和过滤功能
 * 3. 类型安全：运行时JSON模式验证和类型约束
 * 4. 性能优化：高效的JSON解析、索引和缓存机制
 * 5. 标准兼容：完全支持SQL/JSON标准规范
 *
 * 🏗️ 设计模式：指针到实现的模式(PIMPL Pattern)
 *
 * JSON类作为PIMPL模式的经典应用：
 * - 接口稳定性：JSON类的接口稳定，不受内部实现变化影响
 * - 编译防火墙：减少头文件依赖，提高编译速度
 * - 二进制兼容性：内部实现变化不会破坏现有代码
 * - 实现隐藏：隐藏复杂的JSON解析和存储实现细节
 * - 内存管理：智能指针管理复杂的JSON对象生命周期
 *
 * SOLID原则体现：
 * - 单一职责：JSON类负责JSON数据的存储和基本操作
 * - 开闭原则：新JSON功能通过扩展现有类实现
 * - 里氏替换：JSON子类可以替换基类使用
 * - 接口隔离：JSON接口精确定义所需方法
 * - 依赖倒置：高层模块依赖JSON接口而非实现
 *
 * WHAT: JSON数据处理系统 - 完整的JSON文档管理和查询框架
 *
 * 核心功能：
 * - JSON数据类型：支持null、boolean、number、string、array、object
 * - 数据操作：完整的JSON对象创建、修改、查询、删除操作
 * - 路径查询：JSONPath标准的路径表达式查询支持
 * - 模式验证：JSON Schema标准的结构和类型验证
 * - 序列化：高效的JSON字符串序列化和反序列化
 * - 文件操作：JSON数据的文件读写和批量处理
 *
 * 系统组件：
 * - Json：主要的JSON数据类，支持所有JSON数据类型
 * - JsonPath：JSON路径查询类，实现JSONPath标准
 * - JsonSchema：JSON模式验证类，支持结构验证
 * - JsonBuilder：流式JSON构建器，提供链式API
 * - 工具函数：JSON字符串转义、格式化和实用工具
 *
 * JSON数据类型支持：
 * - null：JSON null值，表示缺失或未定义的数据
 * - boolean：布尔值true/false，提供逻辑判断
 * - number：数值类型，支持整数和浮点数
 * - string：字符串类型，支持Unicode字符
 * - array：有序数组，支持任意类型元素的集合
 * - object：键值对对象，支持嵌套结构
 *
 * 路径查询支持：
 * - 基本查询：$.store.book[0].title等简单路径
 * - 过滤查询：$.store.book[?(@.price < 10)]等条件过滤
 * - 通配符查询：$.store.*等通配符匹配
 * - 递归查询：$..author等递归下降查询
 * - 数组操作：$.store.book[-1]等数组索引操作
 *
 * 接口设计：
 * - 构造函数：多种数据类型的JSON对象构造
 * - 类型检查：安全的数据类型验证和转换
 * - 数据访问：索引和键访问的统一接口
 * - 序列化：字符串和文件的双向序列化
 * - 查询接口：路径查询和模式验证功能
 *
 * HOW: JSON数据处理系统的实现机制
 *
 * JSON数据存储实现：
 * 1. 值类型枚举：定义所有支持的JSON数据类型
 * 2. 联合体存储：使用void*指针存储不同类型的值
 * 3. 类型标识：运行时类型标识确保类型安全
 * 4. 引用计数：智能指针管理内存生命周期
 * 5. 惰性拷贝：写时复制优化性能
 *
 * JSON解析实现：
 * 1. 词法分析：将JSON字符串分解为token流
 * 2. 语法分析：递归下降解析构建JSON对象
 * 3. 错误处理：详细的解析错误信息和位置指示
 * 4. 性能优化：跳过空白字符和注释处理
 * 5. 内存管理：栈上分配减少堆内存分配
 *
 * JSON序列化实现：
 * 1. 递归遍历：深度优先遍历JSON对象结构
 * 2. 格式控制：支持紧凑和美化格式输出
 * 3. 转义处理：正确转义特殊字符和Unicode
 * 4. 流式输出：减少内存使用的流式序列化
 * 5. 类型转换：精确的数值和字符串转换
 *
 * 路径查询实现：
 * 1. 路径解析：将路径字符串解析为查询片段
 * 2. 查询执行：递归执行路径查询逻辑
 * 3. 结果收集：收集所有匹配的JSON节点
 * 4. 性能优化：索引和缓存加速查询
 * 5. 错误处理：路径语法错误和查询异常处理
 *
 * 模式验证实现：
 * 1. 模式编译：预编译JSON模式提高验证效率
 * 2. 递归验证：深度优先验证JSON结构
 * 3. 类型检查：精确的数据类型和范围验证
 * 4. 约束验证：自定义验证规则和业务约束
 * 5. 错误报告：详细的验证错误信息和位置
 *
 * 内存管理策略：
 * - 对象池：复用JSON对象的内存分配
 * - 引用计数：自动内存管理避免泄漏
 * - 拷贝优化：写时复制减少不必要的拷贝
 - 垃圾回收：定期清理未使用的JSON对象
 * - 内存监控：跟踪JSON处理的内存使用情况
 *
 * 性能优化策略：
 * - 解析缓存：缓存常用JSON字符串的解析结果
 * - 索引加速：对频繁查询的路径建立索引
 * - 惰性求值：延迟计算提高响应性能
 * - SIMD加速：向量化字符串和数值处理
 * - 并行处理：多核并行JSON处理和查询
 *
 * 错误处理机制：
 * - 解析错误：JSON语法错误的详细诊断
 * - 类型错误：数据类型不匹配的验证
 * - 路径错误：JSON路径查询语法的检查
 * - 模式错误：JSON模式验证失败的报告
 * - 资源错误：内存不足等资源错误的处理
 *
 * 扩展性设计：
 * - 插件架构：支持自定义JSON类型和函数
 * - 配置化：JSON处理的配置化管理
 * - 多格式支持：扩展支持YAML、XML等格式
 * - 标准扩展：支持JSON Schema扩展和自定义类型
 * - 向后兼容：保持与现有JSON系统的兼容性
 *
 * 调试和诊断：
 * - 可视化：JSON结构的图形化展示工具
 * - 性能分析：JSON操作的详细性能统计
 * - 内存分析：JSON对象的内存使用分析
 * - 查询调试：JSON路径查询的执行跟踪
 * - 测试工具：JSON功能的全覆盖测试框架
 */

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
