# SqlExecutionException类详细设计

## 概述

SqlExecutionException类是SQL执行引擎中的异常类，用于处理SQL语句执行过程中出现的各种错误情况。它继承自std::runtime_error，提供统一的异常处理机制。

## 类定义

```cpp
class SqlExecutionException : public std::runtime_error {
public:
  explicit SqlExecutionException(const std::string &message)
      : std::runtime_error(message) {}
};
```

## 构造函数

### SqlExecutionException(const std::string &message)

构造函数：

1. 接收错误信息字符串
2. 调用std::runtime_error的构造函数
3. 存储错误信息供后续处理和显示

## 继承关系

### public std::runtime_error

继承自标准运行时异常类：

1. 提供标准的异常接口
2. 支持what()方法获取错误信息
3. 与C++标准异常处理机制兼容

## 使用场景

1. **语法错误**：SQL语句解析失败时抛出
2. **语义错误**：如表不存在、列不存在等
3. **约束违反**：违反主键、唯一性等约束时抛出
4. **执行错误**：存储引擎操作失败时抛出
5. **事务错误**：事务处理过程中出现的错误
6. **权限错误**：用户权限不足时抛出

## 异常处理策略

1. **捕获和转换**：在执行器各方法中捕获底层异常并转换为SqlExecutionException
2. **错误信息记录**：将错误信息存储在SqlExecutor的last_error_成员中
3. **用户友好**：提供清晰、准确的错误信息便于用户理解问题
4. **调试支持**：在开发模式下提供详细的调试信息