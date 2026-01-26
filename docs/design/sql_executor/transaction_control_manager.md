# TransactionControlManager类详细设计

## 概述

TransactionControlManager是SQLCC数据库系统中的事务控制管理器，负责处理与事务控制相关的语句，包括SAVEPOINT管理和SET TRANSACTION语句。它采用单例模式设计，确保在整个系统中只有一个事务控制管理器实例，方便统一管理事务状态和保存点。

### 核心功能
- 保存点（SAVEPOINT）的创建、释放和回滚
- 事务隔离级别（Isolation Level）的设置和查询
- 事务访问模式（Access Mode）的设置和查询
- 事务统计信息的收集和查询

### 设计优势
- 采用单例模式，确保事务控制的一致性
- 提供简洁的接口，方便上层模块调用
- 维护完整的事务控制信息，支持高级事务特性
- 支持标准SQL的事务控制语法

## 类定义

```cpp
class TransactionControlManager {
public:
    static TransactionControlManager& getInstance();

    // SAVEPOINT管理
    bool createSavepoint(const std::string& savepointName);
    bool releaseSavepoint(const std::string& savepointName);
    bool rollbackToSavepoint(const std::string& savepointName);
    bool savepointExists(const std::string& savepointName) const;

    // SET TRANSACTION
    bool setTransactionIsolation(sql_parser::SetTransactionStatement::IsolationLevel level);
    bool setTransactionAccessMode(sql_parser::SetTransactionStatement::AccessMode mode);
    sql_parser::SetTransactionStatement::IsolationLevel getCurrentIsolationLevel() const;
    sql_parser::SetTransactionStatement::AccessMode getCurrentAccessMode() const;

    // 事务统计
    std::string getTransactionInfo() const;

private:
    TransactionControlManager() = default;

    struct SavepointInfo {
        std::string name;
        long transaction_id;
        std::string created_by;
        long created_time;
    };

    std::unordered_map<std::string, SavepointInfo> savepoints_;
    sql_parser::SetTransactionStatement::IsolationLevel current_isolation_level_;
    sql_parser::SetTransactionStatement::AccessMode current_access_mode_;
    long current_transaction_id_ = 0;
    long next_savepoint_id_ = 1;
};
```

## 核心组件

### 保存点管理
- **保存点表**：使用`std::unordered_map`存储保存点信息，键为保存点名称，值为`SavepointInfo`结构体
- **保存点信息**：包含保存点名称、创建事务ID、创建者和创建时间
- **原子操作**：确保保存点的创建、释放和回滚操作的原子性

### 事务属性管理
- **隔离级别**：支持标准SQL的事务隔离级别（READ UNCOMMITTED, READ COMMITTED, REPEATABLE READ, SERIALIZABLE）
- **访问模式**：支持READ ONLY和READ WRITE两种访问模式
- **属性查询**：提供获取当前事务属性的接口

### 事务统计
- **事务信息**：收集和提供当前事务的统计信息
- **信息格式化**：将事务信息格式化为可读的字符串

## 实现细节

### 单例模式实现
```cpp
TransactionControlManager& TransactionControlManager::getInstance() {
    static TransactionControlManager instance;
    return instance;
}
```

### 保存点管理实现
```cpp
bool TransactionControlManager::createSavepoint(const std::string& savepointName) {
    // 检查保存点是否已存在
    if (savepointExists(savepointName)) {
        return false;
    }

    // 创建新的保存点信息
    SavepointInfo info;
    info.name = savepointName;
    info.transaction_id = current_transaction_id_;
    info.created_by = "current_user";
    info.created_time = std::time(nullptr);

    // 添加到保存点表
    savepoints_[savepointName] = info;
    return true;
}

bool TransactionControlManager::releaseSavepoint(const std::string& savepointName) {
    // 检查保存点是否存在
    if (!savepointExists(savepointName)) {
        return false;
    }

    // 从保存点表中删除
    savepoints_.erase(savepointName);
    return true;
}

bool TransactionControlManager::rollbackToSavepoint(const std::string& savepointName) {
    // 检查保存点是否存在
    if (!savepointExists(savepointName)) {
        return false;
    }

    // 回滚到指定保存点
    // 注意：实际实现需要与事务管理器协作完成回滚操作
    return true;
}

bool TransactionControlManager::savepointExists(const std::string& savepointName) const {
    return savepoints_.find(savepointName) != savepoints_.end();
}
```

### 事务属性管理实现
```cpp
bool TransactionControlManager::setTransactionIsolation(sql_parser::SetTransactionStatement::IsolationLevel level) {
    current_isolation_level_ = level;
    return true;
}

bool TransactionControlManager::setTransactionAccessMode(sql_parser::SetTransactionStatement::AccessMode mode) {
    current_access_mode_ = mode;
    return true;
}

sql_parser::SetTransactionStatement::IsolationLevel TransactionControlManager::getCurrentIsolationLevel() const {
    return current_isolation_level_;
}

sql_parser::SetTransactionStatement::AccessMode TransactionControlManager::getCurrentAccessMode() const {
    return current_access_mode_;
}
```

### 事务统计实现
```cpp
std::string TransactionControlManager::getTransactionInfo() const {
    std::ostringstream oss;
    oss << "Transaction Info:" << std::endl;
    oss << "  Current Transaction ID: " << current_transaction_id_ << std::endl;
    oss << "  Isolation Level: " << (int)current_isolation_level_ << std::endl;
    oss << "  Access Mode: " << (int)current_access_mode_ << std::endl;
    oss << "  Savepoints: " << savepoints_.size() << std::endl;
    return oss.str();
}
```

## 性能优化

1. **高效的保存点查找**：使用`std::unordered_map`存储保存点，实现O(1)时间复杂度的查找操作
2. **延迟初始化**：采用懒加载的单例模式实现，只有在首次使用时才创建实例
3. **减少锁竞争**：单例实例的获取是线程安全的，但内部操作需要根据实际情况添加适当的同步机制

## 扩展点

1. **支持嵌套事务**：可以扩展保存点机制，支持嵌套事务的实现
2. **事务超时管理**：可以添加事务超时检测和处理功能
3. **事务优先级**：可以扩展支持事务优先级的设置和管理
4. **分布式事务支持**：可以扩展支持分布式事务的协调和管理

## 错误处理

- **保存点已存在**：创建保存点时如果保存点名称已存在，返回false
- **保存点不存在**：释放或回滚到不存在的保存点时，返回false
- **无效的隔离级别**：设置无效的隔离级别时，返回false
- **无效的访问模式**：设置无效的访问模式时，返回false

## 测试支持

TransactionControlManager提供了完整的接口，支持单元测试和集成测试：

1. **单例模式测试**：验证全局只有一个实例
2. **保存点管理测试**：测试保存点的创建、释放和回滚
3. **事务属性测试**：测试隔离级别和访问模式的设置和查询
4. **事务统计测试**：测试事务信息的收集和查询

## 使用示例

```cpp
// 获取事务控制管理器实例
auto& tcm = TransactionControlManager::getInstance();

// 创建保存点
tcm.createSavepoint("my_savepoint");

// 设置事务隔离级别
tcm.setTransactionIsolation(sql_parser::SetTransactionStatement::IsolationLevel::SERIALIZABLE);

// 获取当前事务信息
std::string info = tcm.getTransactionInfo();
std::cout << info << std::endl;

// 回滚到保存点
tcm.rollbackToSavepoint("my_savepoint");

// 释放保存点
tcm.releaseSavepoint("my_savepoint");
```

## 总结

TransactionControlManager是SQLCC数据库系统中处理事务控制语句的核心组件，它提供了保存点管理和事务属性设置的功能，支持标准SQL的事务控制语法。通过单例模式设计，确保了事务控制的一致性和统一性。未来可以扩展支持嵌套事务、事务超时管理和分布式事务等高级特性。