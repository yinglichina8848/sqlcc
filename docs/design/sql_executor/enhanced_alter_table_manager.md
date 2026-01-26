# EnhancedAlterTableManager 设计文档

## 1. 概述

EnhancedAlterTableManager 是 SQLCC 数据库系统中的增强型 ALTER TABLE 语句管理器，负责处理各种表结构修改操作，包括列操作、约束操作和表重命名等。该类采用单例模式实现，提供了完整的 ALTER TABLE 功能支持，确保表结构修改的安全性和一致性。

## 2. 核心功能

### 2.1 主要功能

- **表结构修改**：支持对表进行全面的结构修改操作
- **列操作**：添加、删除、修改和重命名列
- **约束操作**：添加和删除表约束
- **表重命名**：支持表名的修改
- **操作验证**：在执行修改前验证操作的合法性
- **依赖管理**：跟踪列之间的依赖关系，确保修改的安全性
- **操作历史记录**：记录所有执行的 ALTER TABLE 操作
- **统计信息收集**：提供 ALTER TABLE 操作的统计信息

### 2.2 设计优势

- **单例模式**：确保系统中只有一个管理器实例，避免状态不一致
- **模块化设计**：将不同类型的 ALTER TABLE 操作分离为独立方法
- **操作验证**：在执行前进行全面验证，减少错误发生
- **依赖检查**：防止破坏性操作影响系统一致性
- **历史记录**：便于审计和问题追踪

## 3. 类定义

```cpp
class EnhancedAlterTableManager {
public:
    static EnhancedAlterTableManager& getInstance();

    // 主要ALTER TABLE操作
    bool alterTable(const sql_parser::AlterTableStatement& stmt);

    // 列操作
    bool addColumn(const std::string& tableName, const sql_parser::AddColumnAction& action);
    bool dropColumn(const std::string& tableName, const sql_parser::DropColumnAction& action);
    bool modifyColumn(const std::string& tableName, const sql_parser::ModifyColumnAction& action);
    bool renameColumn(const std::string& tableName, const sql_parser::RenameColumnAction& action);

    // 约束操作
    bool addConstraint(const std::string& tableName, const sql_parser::AddConstraintAction& action);
    bool dropConstraint(const std::string& tableName, const sql_parser::DropConstraintAction& action);

    // 表操作
    bool renameTable(const sql_parser::RenameTableAction& action);

    // 验证操作
    bool validateAlterTable(const sql_parser::AlterTableStatement& stmt);
    bool canAddColumn(const std::string& tableName, const std::string& columnName) const;
    bool canDropColumn(const std::string& tableName, const std::string& columnName) const;
    bool canModifyColumn(const std::string& tableName, const std::string& columnName) const;

    // 依赖管理
    std::vector<std::string> getColumnDependencies(const std::string& tableName, const std::string& columnName) const;
    bool hasColumnDependencies(const std::string& tableName, const std::string& columnName) const;

    // 统计信息
    std::string getAlterTableStatistics() const;

private:
    EnhancedAlterTableManager() = default;

    struct AlterOperation {
        std::string tableName;
        std::string operationType;
        std::string details;
        long timestamp;
        bool success;
    };

    std::vector<AlterOperation> operation_history_;
    long next_operation_id_ = 1;

    // 内部辅助方法
    bool executeColumnAction(const std::string& tableName, const std::string& actionType, const std::string& columnName);
    bool executeConstraintAction(const std::string& tableName, const std::string& actionType, const std::string& constraintName);
    void recordOperation(const std::string& tableName, const std::string& operationType,
                        const std::string& details, bool success);
};
```

## 4. 核心组件

### 4.1 单例实例管理

- **getInstance()**：获取EnhancedAlterTableManager的单例实例

### 4.2 ALTER TABLE 操作处理

- **alterTable()**：处理完整的ALTER TABLE语句，根据语句类型调用相应的处理方法

### 4.3 列操作处理

- **addColumn()**：向表中添加新列
- **dropColumn()**：从表中删除列
- **modifyColumn()**：修改表中现有列的定义
- **renameColumn()**：重命名表中的列

### 4.4 约束操作处理

- **addConstraint()**：向表中添加约束
- **dropConstraint()**：从表中删除约束

### 4.5 表操作处理

- **renameTable()**：重命名表

### 4.6 操作验证

- **validateAlterTable()**：验证ALTER TABLE语句的合法性
- **canAddColumn()**：检查是否可以向表中添加指定列
- **canDropColumn()**：检查是否可以从表中删除指定列
- **canModifyColumn()**：检查是否可以修改表中的指定列

### 4.7 依赖管理

- **getColumnDependencies()**：获取指定列的依赖关系
- **hasColumnDependencies()**：检查指定列是否存在依赖关系

### 4.8 操作历史和统计

- **getAlterTableStatistics()**：获取ALTER TABLE操作的统计信息
- **recordOperation()**：记录执行的ALTER TABLE操作

## 5. 实现细节

### 5.1 单例模式实现

EnhancedAlterTableManager 采用单例模式实现，确保系统中只有一个管理器实例：

```cpp
static EnhancedAlterTableManager& getInstance() {
    static EnhancedAlterTableManager instance;
    return instance;
}
```

### 5.2 操作执行流程

1. **验证阶段**：使用 validateAlterTable() 方法验证操作的合法性
2. **依赖检查阶段**：检查操作是否会影响其他数据库对象的完整性
3. **执行阶段**：根据操作类型调用相应的处理方法
4. **记录阶段**：使用 recordOperation() 方法记录操作历史

### 5.3 操作历史记录

系统使用 AlterOperation 结构体记录所有执行的 ALTER TABLE 操作：

```cpp
struct AlterOperation {
    std::string tableName;    // 表名
    std::string operationType;// 操作类型
    std::string details;      // 操作详情
    long timestamp;           // 操作时间戳
    bool success;             // 操作是否成功
};
```

## 6. 性能优化

### 6.1 批量操作支持

虽然当前实现主要支持单操作执行，但设计上预留了批量操作的扩展空间，可以通过添加批量处理方法提高大量表结构修改的效率。

### 6.2 延迟约束检查

对于某些复杂的约束操作，系统支持延迟约束检查，提高操作执行效率。

## 7. 扩展点

### 7.1 新操作类型支持

系统设计支持轻松添加新的 ALTER TABLE 操作类型：

1. 在 sql_parser 命名空间中定义新的操作类型
2. 在 EnhancedAlterTableManager 中添加对应的处理方法
3. 在 alterTable() 方法中添加对新操作类型的处理逻辑

### 7.2 自定义验证规则

系统支持通过扩展验证方法添加自定义的表结构修改验证规则。

## 8. 错误处理

系统采用返回布尔值的方式表示操作是否成功，并通过日志系统记录详细的错误信息。对于关键操作，系统会在执行前进行全面验证，防止破坏性操作的执行。

## 9. 测试支持

EnhancedAlterTableManager 提供了全面的单元测试和集成测试支持，确保其功能的正确性和稳定性。测试覆盖了所有主要的 ALTER TABLE 操作类型和边界情况。

## 10. 使用示例

### 10.1 基本用法

```cpp
// 获取单例实例
auto& alterTableManager = EnhancedAlterTableManager::getInstance();

// 执行 ALTER TABLE 语句
bool success = alterTableManager.alterTable(alterTableStmt);

// 检查操作是否成功
if (success) {
    std::cout << "Table altered successfully." << std::endl;
} else {
    std::cout << "Failed to alter table." << std::endl;
}
```

### 10.2 列操作示例

```cpp
// 获取单例实例
auto& alterTableManager = EnhancedAlterTableManager::getInstance();

// 验证是否可以添加列
if (alterTableManager.canAddColumn("users", "email")) {
    // 添加列
    bool success = alterTableManager.addColumn("users", addColumnAction);
    if (success) {
        std::cout << "Column added successfully." << std::endl;
    }
}
```

## 11. 总结

EnhancedAlterTableManager 是 SQLCC 数据库系统中功能强大的 ALTER TABLE 语句管理器，提供了全面的表结构修改功能，确保操作的安全性和一致性。其模块化设计和单例模式实现使其成为系统中可靠的核心组件之一。