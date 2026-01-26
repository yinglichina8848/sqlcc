# JsonValue 和 JsonArray 设计文档

## 1. 概述

JsonValue 是 SQLCC 数据库系统中 JSON 库的核心抽象基类，用于表示 JSON 值。JsonArray 是 JsonValue 的具体实现类，用于表示 JSON 数组。这两个类构成了 SQLCC JSON 库的基础，提供了完整的 JSON 数据类型支持和操作功能。

## 2. 核心功能

### 2.1 JsonValue 核心功能

- **类型抽象**：为所有 JSON 类型提供统一的抽象接口
- **类型检查**：支持查询 JSON 值的类型
- **字符串转换**：将 JSON 值转换为字符串表示
- **格式化输出**：支持缩进格式化的 JSON 字符串输出
- **克隆功能**：支持 JSON 值的深拷贝
- **相等比较**：支持 JSON 值之间的相等比较

### 2.2 JsonArray 核心功能

- **数组操作**：支持添加、访问和修改数组元素
- **类型安全**：确保数组元素都是有效的 JSON 值
- **嵌套支持**：支持数组嵌套和复杂 JSON 结构
- **格式化输出**：支持格式化的数组字符串输出

### 2.3 设计优势

- **抽象基类设计**：实现了数据类型的统一抽象，便于扩展和使用
- **多态支持**：通过多态实现不同 JSON 类型的统一处理
- **值语义**：通过智能指针管理内存，实现值语义的接口
- **完整的 JSON 支持**：支持所有标准 JSON 类型（null、boolean、number、string、array、object）
- **内存安全**：使用智能指针确保内存的安全管理

## 3. 类定义

### 3.1 JsonValue 抽象基类

```cpp
class JsonValue {
public:
  virtual ~JsonValue() = default;
  virtual Json::Type type() const = 0;
  virtual std::string to_string() const = 0;
  virtual std::string to_string_formatted(int indent) const = 0;
  virtual std::unique_ptr<JsonValue> clone() const = 0;
  virtual bool equals(const JsonValue* other) const = 0;
};
```

### 3.2 JsonArray 类

```cpp
class JsonArray : public JsonValue {
public:
  Json::Type type() const override { return Json::ARRAY; }
  std::string to_string() const override;
  std::string to_string_formatted(int indent) const override;
  std::unique_ptr<JsonValue> clone() const override;
  bool equals(const JsonValue* other) const override;

  std::vector<std::unique_ptr<JsonValue>>& values() { return values_; }
  const std::vector<std::unique_ptr<JsonValue>>& values() const { return values_; }

private:
  std::vector<std::unique_ptr<JsonValue>> values_;
};
```

### 3.3 相关类

#### 3.3.1 JsonNull 类

```cpp
class JsonNull : public JsonValue {
public:
  Json::Type type() const override { return Json::NULL_VALUE; }
  std::string to_string() const override { return "null"; }
  std::string to_string_formatted(int indent) const override { return "null"; }
  std::unique_ptr<JsonValue> clone() const override { return std::make_unique<JsonNull>(); }
  bool equals(const JsonValue* other) const override { return other->type() == Json::NULL_VALUE; }
};
```

#### 3.3.2 JsonBoolean 类

```cpp
class JsonBoolean : public JsonValue {
public:
  explicit JsonBoolean(bool value) : value_(value) {}
  Json::Type type() const override { return Json::BOOLEAN; }
  std::string to_string() const override { return value_ ? "true" : "false"; }
  std::string to_string_formatted(int indent) const override { return value_ ? "true" : "false"; }
  std::unique_ptr<JsonValue> clone() const override { return std::make_unique<JsonBoolean>(value_); }
  bool equals(const JsonValue* other) const override;
  bool value() const { return value_; }

private:
  bool value_;
};
```

#### 3.3.3 JsonNumber 类

```cpp
class JsonNumber : public JsonValue {
public:
  explicit JsonNumber(double value) : value_(value) {}
  Json::Type type() const override { return Json::NUMBER; }
  std::string to_string() const override;
  std::string to_string_formatted(int indent) const override { return to_string(); }
  std::unique_ptr<JsonValue> clone() const override { return std::make_unique<JsonNumber>(value_); }
  bool equals(const JsonValue* other) const override;
  double value() const { return value_; }

private:
  double value_;
};
```

#### 3.3.4 JsonString 类

```cpp
class JsonString : public JsonValue {
public:
  explicit JsonString(const std::string& value) : value_(value) {}
  Json::Type type() const override { return Json::STRING; }
  std::string to_string() const override;
  std::string to_string_formatted(int indent) const override { return to_string(); }
  std::unique_ptr<JsonValue> clone() const override { return std::make_unique<JsonString>(value_); }
  bool equals(const JsonValue* other) const override;
  const std::string& value() const { return value_; }

private:
  std::string value_;
};
```

#### 3.3.5 JsonObject 类

```cpp
class JsonObject : public JsonValue {
public:
  Json::Type type() const override { return Json::OBJECT; }
  std::string to_string() const override;
  std::string to_string_formatted(int indent) const override;
  std::unique_ptr<JsonValue> clone() const override;
  bool equals(const JsonValue* other) const override;

  std::unordered_map<std::string, std::unique_ptr<JsonValue>>& values() { return values_; }
  const std::unordered_map<std::string, std::unique_ptr<JsonValue>>& values() const { return values_; }

private:
  std::unordered_map<std::string, std::unique_ptr<JsonValue>> values_;
};
```

## 4. 核心组件

### 4.1 JsonValue 核心接口

#### 4.1.1 类型查询

```cpp
virtual Json::Type type() const = 0;
```

- **功能**：返回 JSON 值的类型
- **返回值**：Json::Type 枚举值，表示 JSON 值的类型

#### 4.1.2 字符串转换

```cpp
virtual std::string to_string() const = 0;
```

- **功能**：将 JSON 值转换为紧凑的字符串表示
- **返回值**：JSON 值的字符串表示

#### 4.1.3 格式化输出

```cpp
virtual std::string to_string_formatted(int indent) const = 0;
```

- **功能**：将 JSON 值转换为带有缩进的格式化字符串表示
- **参数**：`indent` - 缩进级别
- **返回值**：格式化的 JSON 字符串

#### 4.1.4 克隆功能

```cpp
virtual std::unique_ptr<JsonValue> clone() const = 0;
```

- **功能**：创建 JSON 值的深拷贝
- **返回值**：JSON 值的深拷贝

#### 4.1.5 相等比较

```cpp
virtual bool equals(const JsonValue* other) const = 0;
```

- **功能**：比较两个 JSON 值是否相等
- **参数**：`other` - 要比较的另一个 JSON 值
- **返回值**：如果两个 JSON 值相等则返回 true，否则返回 false

### 4.2 JsonArray 核心接口

#### 4.2.1 数组元素访问

```cpp
std::vector<std::unique_ptr<JsonValue>>& values() { return values_; }
const std::vector<std::unique_ptr<JsonValue>>& values() const { return values_; }
```

- **功能**：获取数组中的元素列表
- **返回值**：数组元素的引用

## 5. 实现细节

### 5.1 JsonValue 实现

JsonValue 是一个抽象基类，定义了所有 JSON 类型必须实现的接口。具体的 JSON 类型（JsonNull、JsonBoolean、JsonNumber、JsonString、JsonArray、JsonObject）通过继承 JsonValue 并实现这些接口来提供具体的功能。

### 5.2 JsonArray 实现

JsonArray 使用 `std::vector<std::unique_ptr<JsonValue>>` 来存储数组元素，确保内存的安全管理和自动释放。

#### 5.2.1 字符串转换实现

```cpp
std::string JsonArray::to_string() const {
  std::stringstream ss;
  ss << "[";
  for (size_t i = 0; i < values_.size(); ++i) {
    if (i > 0) ss << ",";
    ss << values_[i]->to_string();
  }
  ss << "]";
  return ss.str();
}
```

#### 5.2.2 格式化输出实现

```cpp
std::string JsonArray::to_string_formatted(int indent) const {
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
```

#### 5.2.3 克隆实现

```cpp
std::unique_ptr<JsonValue> JsonArray::clone() const {
  auto result = std::make_unique<JsonArray>();
  for (const auto& value : values_) {
    result->values_.push_back(value->clone());
  }
  return result;
}
```

#### 5.2.4 相等比较实现

```cpp
bool JsonArray::equals(const JsonValue* other) const {
  if (other->type() != Json::ARRAY) return false;
  const JsonArray* other_array = static_cast<const JsonArray*>(other);
  if (values_.size() != other_array->values_.size()) return false;
  for (size_t i = 0; i < values_.size(); ++i) {
    if (!values_[i]->equals(other_array->values_[i].get())) return false;
  }
  return true;
}
```

## 6. 性能优化

### 6.1 内存管理

使用智能指针（`std::unique_ptr`）管理 JSON 值的内存，避免内存泄漏和手动内存管理的复杂性。

### 6.2 字符串转换

使用 `std::stringstream` 进行字符串转换，确保高效的字符串拼接和格式化。

### 6.3 深拷贝优化

克隆功能使用深拷贝，确保 JSON 值的独立性和一致性。

## 7. 扩展点

### 7.1 新 JSON 类型支持

可以通过继承 JsonValue 并实现其接口来添加新的 JSON 类型支持。

### 7.2 自定义序列化

可以通过重写 to_string() 和 to_string_formatted() 方法来实现自定义的 JSON 序列化格式。

### 7.3 自定义比较逻辑

可以通过重写 equals() 方法来实现自定义的 JSON 值比较逻辑。

## 8. 错误处理

JsonValue 和 JsonArray 类本身不进行复杂的错误处理，而是依赖于上层的 Json 类来处理类型转换错误和其他异常情况。当进行不适当的类型转换时，会抛出类型错误异常。

## 9. 测试支持

JsonValue 和 JsonArray 类提供了全面的单元测试支持，确保其功能的正确性和稳定性。测试覆盖了所有主要的 JSON 类型和操作功能。

## 10. 使用示例

### 10.1 基本用法

```cpp
// 创建 JSON 数组
Json json_array;
json_array.set_array(); // 确保是数组类型

// 添加元素
json_array[0] = 123;
json_array[1] = "hello";
json_array[2] = true;

// 获取元素
int int_value = json_array[0].as_int(); // 123
std::string string_value = json_array[1].as_string(); // "hello"
bool bool_value = json_array[2].as_bool(); // true

// 转换为字符串
std::string json_str = json_array.to_string(); // [123,"hello",true]
std::string formatted_str = json_array.to_string_formatted(0); // 格式化输出
```

### 10.2 嵌套数组

```cpp
// 创建嵌套数组
Json nested_array;
nested_array.set_array();
nested_array[0] = 1;
nested_array[1] = 2;

// 在数组中添加另一个数组
Json inner_array;
inner_array.set_array();
inner_array[0] = "a";
inner_array[1] = "b";
nested_array[2] = inner_array;

// 访问嵌套元素
std::string value = nested_array[2][0].as_string(); // "a"
```

## 11. 总结

JsonValue 和 JsonArray 类是 SQLCC 数据库系统中 JSON 库的核心组件，提供了完整的 JSON 数据类型支持和操作功能。JsonValue 作为抽象基类，实现了数据类型的统一抽象，而 JsonArray 作为具体实现类，提供了 JSON 数组的完整功能。这两个类的设计和实现确保了 JSON 库的灵活性、可扩展性和高性能。