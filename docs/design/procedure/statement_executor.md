# StatementExecutor 设计文档

## 1. 概述

StatementExecutor 是 SQLCC 数据库系统中 ProcedureVM 类的内部访问者类，用于实现存储过程语句的执行。该类采用访问者模式设计，负责根据语句类型调用相应的执行方法，确保存储过程中各种语句的正确执行。

## 2. 核心功能

### 2.1 主要功能

- **语句访问与执行**：使用访问者模式访问并执行存储过程中的各种语句
- **类型分派**：根据语句类型将执行请求分派到相应的处理方法
- **执行结果管理**：跟踪语句执行的成功或失败状态
- **上下文传递**：维护语句执行所需的上下文信息

### 2.2 设计优势

- **访问者模式**：实现了数据结构与操作的分离，便于扩展新的语句类型
- **类型安全**：在编译时确保所有语句类型都有相应的处理方法
- **集中控制**：将所有语句执行的入口集中在一个类中，便于管理和维护
- **低耦合**：与具体的语句执行逻辑分离，提高了代码的可维护性和可测试性

## 3. 类定义

```cpp
class StatementExecutor : public ProcedureVisitor {
public:
    StatementExecutor(ProcedureVM& vm, ProcedureContext& ctx)
        : vm_(vm), context_(ctx), success_(true) {}

    bool getResult() const { return success_; }

    void visitVariableDeclaration(VariableDeclaration& node) override;
    void visitAssignmentStatement(AssignmentStatement& node) override;
    void visitIfStatement(IfStatement& node) override;
    void visitWhileStatement(WhileStatement& node) override;
    void visitCallStatement(CallStatement& node) override;
    void visitProcedureDefinition(ProcedureDefinition& node) override;

private:
    ProcedureVM& vm_;
    ProcedureContext& context_;
    bool success_;
};
```

## 4. 核心组件

### 4.1 构造函数

```cpp
StatementExecutor(ProcedureVM& vm, ProcedureContext& ctx)
    : vm_(vm), context_(ctx), success_(true) {}
```

- **参数**：
  - `vm_`：ProcedureVM实例的引用，用于调用具体的语句执行方法
  - `context_`：ProcedureContext实例的引用，提供语句执行所需的上下文
  - `success_`：执行结果标志，初始化为true

### 4.2 执行结果获取

```cpp
bool getResult() const { return success_; }
```

- **返回值**：语句执行的结果，true表示成功，false表示失败

### 4.3 语句访问方法

#### 4.3.1 变量声明语句

```cpp
void visitVariableDeclaration(VariableDeclaration& node) override {
    success_ = vm_.executeVariableDeclaration(&node, context_);
}
```

- **功能**：处理变量声明语句，调用ProcedureVM的executeVariableDeclaration方法执行具体逻辑

#### 4.3.2 赋值语句

```cpp
void visitAssignmentStatement(AssignmentStatement& node) override {
    success_ = vm_.executeAssignment(&node, context_);
}
```

- **功能**：处理赋值语句，调用ProcedureVM的executeAssignment方法执行具体逻辑

#### 4.3.3 IF语句

```cpp
void visitIfStatement(IfStatement& node) override {
    success_ = vm_.executeIfStatement(&node, context_);
}
```

- **功能**：处理IF条件语句，调用ProcedureVM的executeIfStatement方法执行具体逻辑

#### 4.3.4 WHILE语句

```cpp
void visitWhileStatement(WhileStatement& node) override {
    success_ = vm_.executeWhileStatement(&node, context_);
}
```

- **功能**：处理WHILE循环语句，调用ProcedureVM的executeWhileStatement方法执行具体逻辑

#### 4.3.5 调用语句

```cpp
void visitCallStatement(CallStatement& node) override {
    success_ = vm_.executeCallStatement(&node, context_);
}
```

- **功能**：处理过程调用语句，调用ProcedureVM的executeCallStatement方法执行具体逻辑

#### 4.3.6 过程定义语句

```cpp
void visitProcedureDefinition(ProcedureDefinition& node) override {
    // 不应该直接执行过程定义
    success_ = false;
    vm_.setError("Cannot execute procedure definition directly");
}
```

- **功能**：处理过程定义语句，直接返回失败，因为过程定义不能直接执行

## 5. 实现细节

### 5.1 访问者模式实现

StatementExecutor 实现了 ProcedureVisitor 接口，该接口定义了访问各种存储过程语句的方法。当调用 Statement 对象的 accept 方法时，会根据语句类型自动调用相应的 visit 方法。

### 5.2 执行流程

1. **创建 StatementExecutor 实例**：ProcedureVM 的 executeStatement 方法创建 StatementExecutor 实例
2. **调用 accept 方法**：将 StatementExecutor 实例传递给 Statement 对象的 accept 方法
3. **类型分派**：根据语句类型调用相应的 visit 方法
4. **执行具体逻辑**：visit 方法调用 ProcedureVM 的相应执行方法
5. **设置执行结果**：根据执行结果设置 success_ 标志
6. **返回执行结果**：通过 getResult 方法返回执行结果

### 5.3 错误处理

当语句执行失败时，相应的 visit 方法会将 success_ 标志设置为 false，并通过 ProcedureVM 的 setError 方法记录错误信息。

## 6. 性能优化

### 6.1 轻量级设计

StatementExecutor 是一个轻量级的类，每次执行语句时创建新实例，执行完成后销毁，避免了长时间占用资源。

### 6.2 直接方法调用

使用直接方法调用而不是反射或其他动态机制，确保了语句执行的高效性。

## 7. 扩展点

### 7.1 新语句类型支持

要添加对新语句类型的支持，需要：

1. 在 ProcedureVisitor 接口中添加新的 visit 方法
2. 在 StatementExecutor 类中实现该方法
3. 在 ProcedureVM 类中添加相应的执行方法

### 7.2 自定义执行逻辑

可以通过重写 StatementExecutor 类的 visit 方法来实现自定义的语句执行逻辑。

## 8. 错误处理

StatementExecutor 通过设置 success_ 标志和调用 ProcedureVM 的 setError 方法来处理执行错误。当任何语句执行失败时，整个存储过程的执行会停止，并返回失败状态。

## 9. 测试支持

StatementExecutor 作为 ProcedureVM 的内部类，与 ProcedureVM 一起进行测试。测试覆盖了所有支持的语句类型和边界情况，确保其功能的正确性和稳定性。

## 10. 使用示例

### 10.1 基本用法

```cpp
bool ProcedureVM::executeStatement(Statement* statement, ProcedureContext& context) {
    if (!statement) {
        setError("Null statement");
        return false;
    }

    // 创建 StatementExecutor 实例
    class StatementExecutor : public ProcedureVisitor {
        // ... 类定义 ...
    };

    // 使用访问者模式执行语句
    StatementExecutor executor(*this, context);
    statement->accept(executor);
    return executor.getResult();
}
```

### 10.2 扩展新语句类型

```cpp
// 在 ProcedureVisitor 接口中添加新方法
class ProcedureVisitor {
    // ... 现有方法 ...
    virtual void visitForStatement(ForStatement& node) = 0;
};

// 在 StatementExecutor 中实现新方法
class StatementExecutor : public ProcedureVisitor {
    // ... 现有方法 ...
    void visitForStatement(ForStatement& node) override {
        success_ = vm_.executeForStatement(&node, context_);
    }
};

// 在 ProcedureVM 中添加执行方法
bool ProcedureVM::executeForStatement(ForStatement* for_stmt, ProcedureContext& context) {
    // 实现 FOR 语句的执行逻辑
}
```

## 11. 总结

StatementExecutor 是 SQLCC 数据库系统中存储过程执行的核心组件之一，采用访问者模式设计，实现了对各种存储过程语句的高效执行。其轻量级设计和清晰的类型分派机制使其成为系统中可靠和高效的语句执行器。