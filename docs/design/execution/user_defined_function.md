# UserDefinedFunction 设计文档

## 1. 概述

UserDefinedFunction 是 SQLCC 数据库系统中的用户定义函数抽象基类，用于表示和执行用户定义的函数。该类提供了统一的接口，支持标量函数和表值函数的执行，同时管理函数的元数据和特性。

## 2. 核心功能

### 2.1 主要功能

- **函数元数据管理**：存储和管理用户定义函数的名称、定义、返回类型等元数据
- **函数执行接口**：提供标量函数和表值函数的执行接口
- **函数特性检查**：支持检查函数的特性，如确定性、SQL操作类型等
- **扩展性**：支持不同类型的用户定义函数（SQL、C++等）

### 2.2 设计优势

- **抽象接口**：提供统一的函数接口，便于扩展和使用
- **多态支持**：通过多态实现不同类型用户函数的统一处理
- **类型安全**：确保函数执行的类型安全
- **特性检查**：支持函数特性的检查，便于优化和安全性控制
- **可扩展**：可以轻松添加新类型的用户定义函数

## 3. 类定义

```cpp
class UserDefinedFunction {
public:
    UserDefinedFunction(std::unique_ptr<sqlcc::sql_parser::FunctionDefinition> definition);
    virtual ~UserDefinedFunction() = default;

    const std::string& getName() const;
    const sqlcc::sql_parser::FunctionDefinition& getDefinition() const;
    FunctionReturnType getReturnType() const;

    // 执行函数
    virtual Value executeScalar(const std::vector<Value>& arguments,
                              std::shared_ptr<SqlExecutor> executor) const = 0;

    virtual std::vector<std::unordered_map<std::string, Value>> executeTable(
        const std::vector<Value>& arguments,
        std::shared_ptr<SqlExecutor> executor) const = 0;

    // 函数特性检查
    bool isDeterministic() const;
    bool containsSql() const;
    bool readsSqlData() const;
    bool modifiesSqlData() const;

protected:
    std::unique_ptr<sqlcc::sql_parser::FunctionDefinition> definition_;
};
```

## 4. 核心组件

### 4.1 构造函数

```cpp
UserDefinedFunction(std::unique_ptr<sqlcc::sql_parser::FunctionDefinition> definition);
```

- **功能**：初始化用户定义函数
- **参数**：`definition` - 函数定义对象

### 4.2 元数据访问

#### 4.2.1 获取函数名称

```cpp
const std::string& getName() const;
```

- **功能**：获取函数的名称
- **返回值**：函数名称的常量引用

#### 4.2.2 获取函数定义

```cpp
const sqlcc::sql_parser::FunctionDefinition& getDefinition() const;
```

- **功能**：获取函数的完整定义
- **返回值**：函数定义对象的常量引用

#### 4.2.3 获取函数返回类型

```cpp
FunctionReturnType getReturnType() const;
```

- **功能**：获取函数的返回类型
- **返回值**：函数返回类型枚举值

### 4.3 函数执行接口

#### 4.3.1 执行标量函数

```cpp
virtual Value executeScalar(const std::vector<Value>& arguments,
                          std::shared_ptr<SqlExecutor> executor) const = 0;
```

- **功能**：执行标量函数，返回单个值
- **参数**：
  - `arguments` - 函数参数列表
  - `executor` - SQL执行器指针
- **返回值**：函数执行结果值

#### 4.3.2 执行表值函数

```cpp
virtual std::vector<std::unordered_map<std::string, Value>> executeTable(
    const std::vector<Value>& arguments,
    std::shared_ptr<SqlExecutor> executor) const = 0;
```

- **功能**：执行表值函数，返回结果集
- **参数**：
  - `arguments` - 函数参数列表
  - `executor` - SQL执行器指针
- **返回值**：函数执行结果集，每个元素是一个行记录的键值对映射

### 4.4 函数特性检查

#### 4.4.1 检查函数是否确定性

```cpp
bool isDeterministic() const;
```

- **功能**：检查函数是否是确定性的（相同输入总是产生相同输出）
- **返回值**：如果函数是确定性的则返回true，否则返回false

#### 4.4.2 检查函数是否包含SQL

```cpp
bool containsSql() const;
```

- **功能**：检查函数是否包含SQL语句
- **返回值**：如果函数包含SQL语句则返回true，否则返回false

#### 4.4.3 检查函数是否读取SQL数据

```cpp
bool readsSqlData() const;
```

- **功能**：检查函数是否读取SQL数据
- **返回值**：如果函数读取SQL数据则返回true，否则返回false

#### 4.4.4 检查函数是否修改SQL数据

```cpp
bool modifiesSqlData() const;
```

- **功能**：检查函数是否修改SQL数据
- **返回值**：如果函数修改SQL数据则返回true，否则返回false

## 5. 实现细节

### 5.1 构造函数实现

```cpp
UserDefinedFunction::UserDefinedFunction(
    std::unique_ptr<sqlcc::sql_parser::FunctionDefinition> definition)
    : definition_(std::move(definition)) {
    // 初始化函数元数据
}
```

### 5.2 函数特性检查实现

函数特性检查通过访问函数定义对象的属性来实现：

```cpp
bool UserDefinedFunction::isDeterministic() const {
    return definition_->isDeterministic();
}

bool UserDefinedFunction::containsSql() const {
    return definition_->containsSql();
}

bool UserDefinedFunction::readsSqlData() const {
    return definition_->readsSqlData();
}

bool UserDefinedFunction::modifiesSqlData() const {
    return definition_->modifiesSqlData();
}
```

## 6. 派生类

### 6.1 SqlUserDefinedFunction

SqlUserDefinedFunction 是 UserDefinedFunction 的派生类，用于执行SQL语言定义的用户函数：

```cpp
class SqlUserDefinedFunction : public UserDefinedFunction {
public:
    SqlUserDefinedFunction(std::unique_ptr<sqlcc::sql_parser::FunctionDefinition> definition);

    Value executeScalar(const std::vector<Value>& arguments,
                       std::shared_ptr<SqlExecutor> executor) const override;

    std::vector<std::unordered_map<std::string, Value>> executeTable(
        const std::vector<Value>& arguments,
        std::shared_ptr<SqlExecutor> executor) const override;

private:
    // 解析函数体中的参数引用
    std::string substituteParameters(const std::string& body,
                                   const std::vector<Value>& arguments) const;

    // 执行SQL查询
    Value executeSqlQuery(const std::string& sql,
                          std::shared_ptr<SqlExecutor> executor) const;
};
```

### 6.2 扩展新的用户函数类型

可以通过继承 UserDefinedFunction 类来实现新类型的用户定义函数，例如C++函数：

```cpp
class CppUserDefinedFunction : public UserDefinedFunction {
public:
    CppUserDefinedFunction(std::unique_ptr<sqlcc::sql_parser::FunctionDefinition> definition,
                          std::function<Value(const std::vector<Value>&)> func);

    Value executeScalar(const std::vector<Value>& arguments,
                       std::shared_ptr<SqlExecutor> executor) const override;

    std::vector<std::unordered_map<std::string, Value>> executeTable(
        const std::vector<Value>& arguments,
        std::shared_ptr<SqlExecutor> executor) const override;

private:
    std::function<Value(const std::vector<Value>&)> function_;
};
```

## 7. 性能优化

### 7.1 确定性函数缓存

对于确定性函数，可以缓存执行结果，提高性能：

```cpp
Value CachedUserDefinedFunction::executeScalar(
    const std::vector<Value>& arguments,
    std::shared_ptr<SqlExecutor> executor) const {
    if (isDeterministic()) {
        auto key = makeCacheKey(arguments);
        auto it = cache_.find(key);
        if (it != cache_.end()) {
            return it->second;
        }
        Value result = UserDefinedFunction::executeScalar(arguments, executor);
        cache_[key] = result;
        return result;
    }
    return UserDefinedFunction::executeScalar(arguments, executor);
}
```

### 7.2 预编译SQL

对于SQL用户定义函数，可以预编译SQL语句，提高执行效率：

```cpp
SqlUserDefinedFunction::SqlUserDefinedFunction(
    std::unique_ptr<sqlcc::sql_parser::FunctionDefinition> definition)
    : UserDefinedFunction(std::move(definition)) {
    // 预编译SQL语句
    precompiled_sql_ = compileSql(definition_->getBody());
}
```

## 8. 扩展点

### 8.1 新函数类型支持

可以通过继承 UserDefinedFunction 类来实现新类型的用户定义函数：

1. 继承 UserDefinedFunction 类
2. 实现 executeScalar() 和 executeTable() 方法
3. 注册到 FunctionExecutor

### 8.2 函数特性扩展

可以扩展函数特性检查，支持更多的函数特性：

```cpp
bool UserDefinedFunction::supportsParallelExecution() const {
    return definition_->supportsParallelExecution();
}

bool UserDefinedFunction::isStrict() const {
    return definition_->isStrict();
}
```

## 9. 错误处理

UserDefinedFunction 通过抛出异常来处理执行错误。当函数执行失败时，会抛出相应的异常，包含错误信息。

## 10. 测试支持

UserDefinedFunction 类提供了全面的单元测试和集成测试支持，确保其功能的正确性和稳定性。测试覆盖了所有主要的函数类型和操作功能。

## 11. 使用示例

### 11.1 注册用户定义函数

```cpp
// 创建函数定义
auto func_def = std::make_unique<sql_parser::FunctionDefinition>(
    "my_function",                          // 函数名
    FunctionReturnType::SCALAR,             // 返回类型
    "INT",                                  // 返回类型名称
    std::vector<sql_parser::ParameterDefinition>(),  // 参数列表
    "SELECT 1",                            // 函数体
    true,                                   // 确定性
    true,                                   // 包含SQL
    true,                                   // 读取SQL数据
    false                                   // 修改SQL数据
);

// 创建用户定义函数
auto user_func = std::make_unique<SqlUserDefinedFunction>(std::move(func_def));

// 注册到函数执行器
auto& func_executor = FunctionExecutor::getInstance();
func_executor.registerFunction(std::move(user_func));
```

### 11.2 执行用户定义函数

```cpp
// 获取函数执行器
auto& func_executor = FunctionExecutor::getInstance();

// 准备参数
std::vector<Value> args;

// 执行标量函数
Value result = func_executor.executeScalarFunction(
    "my_function",                          // 函数名
    args,                                   // 参数列表
    sql_executor                             // SQL执行器
);

// 执行表值函数
std::vector<std::unordered_map<std::string, Value>> table_result =
    func_executor.executeTableFunction(
        "my_function",                      // 函数名
        args,                               // 参数列表
        sql_executor                         // SQL执行器
    );
```

## 12. 总结

UserDefinedFunction 是 SQLCC 数据库系统中用户定义函数的核心抽象基类，提供了统一的接口，支持标量函数和表值函数的执行。该类设计灵活，易于扩展，可以支持不同类型的用户定义函数，为 SQLCC 数据库系统提供了强大的函数扩展能力。