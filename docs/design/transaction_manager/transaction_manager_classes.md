# Transaction Manager 类设计文档

## 类列表

### Savepoint

**定义位置**: `src/transaction/savepoint_manager.h`

**定义**:
```cpp
class Savepoint {
public:
    Savepoint(const std::string& name, TransactionId txn_id, size_t undo_position);

    ~Savepoint();

    // Getters
    const std::string& getName() const { return name_; ...
```

**构造函数**:
- `Savepoint`
- `Savepoint`
- `addLockedResource`
- `removeLockedResource`

**析构函数**:
- `Savepoint`

**公有方法**:
- `锁资源管理
    void addLockedResource`
- `void removeLockedResource`

---

### SavepointManager

**定义位置**: `src/transaction/savepoint_manager.h`

**定义**:
```cpp
class SavepointManager {
public:
    static SavepointManager& getInstance();

    /**
     * 创建保存点
     * @param txn_id 事务ID
     * @param savepoint_name 保存点名称
     * @return 是否成功
     */
    bool cre...
```

**构造函数**:
- `getInstance`
- `createSavepoint`
- `releaseSavepoint`
- `rollbackToSavepoint`
- `clearTransactionSavepoints`
- `SavepointManager`
- `SavepointManager`

**析构函数**:
- `SavepointManager`

**公有方法**:
- `bool createSavepoint`
- `bool releaseSavepoint`
- `bool rollbackToSavepoint`
- `void clearTransactionSavepoints`

---

