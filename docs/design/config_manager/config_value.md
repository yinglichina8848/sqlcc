# ConfigValue类详细设计

## 概述

ConfigValue类是配置管理器中用于表示配置值的类，支持多种数据类型（布尔、整数、双精度浮点、字符串）。它提供了类型安全的配置值存储和访问机制。

## 类定义

```cpp
class ConfigValue {
public:
    enum class Type {
        BOOLEAN,
        INTEGER,
        DOUBLE,
        STRING
    };
    
    ConfigValue();
    ConfigValue(bool value);
    ConfigValue(int value);
    ConfigValue(double value);
    ConfigValue(const std::string& value);
    ConfigValue(const char* value);
    
    Type GetType() const;
    bool IsBool() const;
    bool IsInt() const;
    bool IsDouble() const;
    bool IsString() const;
    
    bool GetBool() const;
    int GetInt() const;
    double GetDouble() const;
    const std::string& GetString() const;
    
    void SetBool(bool value);
    void SetInt(int value);
    void SetDouble(double value);
    void SetString(const std::string& value);
    
    std::string ToString() const;
    
private:
    Type type_;
    bool bool_value_;
    int int_value_;
    double double_value_;
    std::string string_value_;
};
```

## 构造函数

### ConfigValue()

默认构造函数：

1. 初始化类型为STRING
2. 初始化字符串值为空字符串

### ConfigValue(bool value)

布尔类型构造函数：

1. 初始化类型为BOOLEAN
2. 存储布尔值

### ConfigValue(int value)

整数类型构造函数：

1. 初始化类型为INTEGER
2. 存储整数值

### ConfigValue(double value)

双精度浮点类型构造函数：

1. 初始化类型为DOUBLE
2. 存储双精度浮点值

### ConfigValue(const std::string& value)

字符串类型构造函数：

1. 初始化类型为STRING
2. 存储字符串值

### ConfigValue(const char* value)

C风格字符串类型构造函数：

1. 初始化类型为STRING
2. 存储字符串值

## 类型检查方法

### Type GetType() const

获取值类型：

1. 返回当前存储的值类型

### bool IsBool() const

检查是否为布尔类型：

1. 返回类型是否为BOOLEAN

### bool IsInt() const

检查是否为整数类型：

1. 返回类型是否为INTEGER

### bool IsDouble() const

检查是否为双精度浮点类型：

1. 返回类型是否为DOUBLE

### bool IsString() const

检查是否为字符串类型：

1. 返回类型是否为STRING

## 值访问方法

### bool GetBool() const

获取布尔值：

1. 检查类型是否为BOOLEAN
2. 如果是则返回布尔值，否则抛出异常或返回默认值

### int GetInt() const

获取整数值：

1. 检查类型是否为INTEGER
2. 如果是则返回整数值，否则抛出异常或返回默认值

### double GetDouble() const

获取双精度浮点值：

1. 检查类型是否为DOUBLE
2. 如果是则返回双精度浮点值，否则抛出异常或返回默认值

### const std::string& GetString() const

获取字符串值：

1. 检查类型是否为STRING
2. 如果是则返回字符串值，否则抛出异常或返回默认值

## 值设置方法

### void SetBool(bool value)

设置布尔值：

1. 设置类型为BOOLEAN
2. 存储布尔值

### void SetInt(int value)

设置整数值：

1. 设置类型为INTEGER
2. 存储整数值

### void SetDouble(double value)

设置双精度浮点值：

1. 设置类型为DOUBLE
2. 存储双精度浮点值

### void SetString(const std::string& value)

设置字符串值：

1. 设置类型为STRING
2. 存储字符串值

## 辅助方法

### std::string ToString() const

转换为字符串表示：

1. 根据当前类型将值转换为字符串形式
2. 布尔值转换为"true"/"false"
3. 数值类型转换为相应的字符串表示
4. 字符串类型直接返回存储的值

## 成员变量

### Type type_

值类型：

1. 记录当前存储的值类型
2. 用于类型检查和安全访问

### bool bool_value_

布尔值存储：

1. 当type_为BOOLEAN时存储布尔值

### int int_value_

整数值存储：

1. 当type_为INTEGER时存储整数值

### double double_value_

双精度浮点值存储：

1. 当type_为DOUBLE时存储双精度浮点值

### std::string string_value_

字符串值存储：

1. 当type_为STRING时存储字符串值
2. 默认初始化为空字符串