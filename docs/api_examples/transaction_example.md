# 事务管理API使用示例

## 基本事务操作

```cpp
#include "transaction_manager/transaction_manager.h"

// 创建事务管理器实例
auto tm = std::make_shared<TransactionManager>();

// 开始事务
auto txn = tm->Begin();
try {
    // 执行事务操作
    txn->Update("table1", key1, value1);
    txn->Update("table2", key2, value2);
    
    // 提交事务
    tm->Commit(txn);
} catch (...) {
    // 回滚事务
    tm->Abort(txn);
}
```

## 事务隔离级别

```cpp
// 设置隔离级别
auto txn = tm->Begin(IsolationLevel::READ_COMMITTED);

// 或者使用快照隔离
auto snapshot_txn = tm->Begin(IsolationLevel::SNAPSHOT_ISOLATION);
```
