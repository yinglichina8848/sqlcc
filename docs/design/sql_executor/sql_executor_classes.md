# SQL Executor 类设计文档

## 类列表

### ProcedureFunctionManager

**定义位置**: `src/sql_executor/advanced_sql92_executor.cpp`

**定义**:
```cpp
class ProcedureFunctionManager {
public:
  static ProcedureFunctionManager& getInstance();

  // 存储过程管理
  bool createProcedure(const sql_parser::CreateProcedureStatement& stmt);
  bool dropProcedure(c...
```

**构造函数**:
- `getInstance`
- `createProcedure`
- `dropProcedure`
- `callProcedure`
- `createFunction`
- `dropFunction`
- `executeProcedure`
- `executeFunction`
- `setVariable`

**公有方法**:
- `存储过程管理
  bool createProcedure`
- `bool dropProcedure`
- `函数管理
  bool createFunction`
- `bool dropFunction`
- `string executeProcedure`
- `string executeFunction`
- `变量管理
  void setVariable`

---

### TransactionControlManager

**定义位置**: `src/sql_executor/advanced_sql92_executor.cpp`

**定义**:
```cpp
class TransactionControlManager {
public:
  static TransactionControlManager& getInstance();

  // SAVEPOINT管理
  bool createSavepoint(const std::string& savepointName);
  bool releaseSavepoint(const s...
```

**构造函数**:
- `getInstance`
- `createSavepoint`
- `releaseSavepoint`
- `rollbackToSavepoint`
- `setTransactionIsolation`
- `setTransactionAccessMode`

**公有方法**:
- `SAVEPOINT管理
  bool createSavepoint`
- `bool releaseSavepoint`
- `bool rollbackToSavepoint`
- `TRANSACTION
  bool setTransactionIsolation`
- `bool setTransactionAccessMode`

---

### EnhancedTriggerManager

**定义位置**: `src/sql_executor/advanced_sql92_executor.cpp`

**定义**:
```cpp
class EnhancedTriggerManager {
public:
  static EnhancedTriggerManager& getInstance();

  // 触发器管理
  bool createTrigger(const sql_parser::CreateTriggerStatement& stmt);
  bool dropTrigger(const std::s...
```

**构造函数**:
- `getInstance`
- `createTrigger`
- `dropTrigger`
- `enableTrigger`
- `disableTrigger`
- `executeTriggers`

**公有方法**:
- `触发器管理
  bool createTrigger`
- `bool dropTrigger`
- `bool enableTrigger`
- `bool disableTrigger`
- `触发器执行
  void executeTriggers`

---

### EnhancedAlterTableManager

**定义位置**: `src/sql_executor/advanced_sql92_executor.cpp`

**定义**:
```cpp
class EnhancedAlterTableManager {
public:
  static EnhancedAlterTableManager& getInstance();

  // 增强ALTER TABLE操作
  bool executeAlterTable(const sql_parser::EnhancedAlterTableStatement& stmt);
  
  /...
```

**构造函数**:
- `getInstance`
- `executeAlterTable`
- `addColumn`
- `dropColumn`
- `alterColumn`
- `renameColumn`
- `addConstraint`
- `dropConstraint`
- `enableTrigger`
- `disableTrigger`

**公有方法**:
- `TABLE操作
  bool executeAlterTable`
- `具体操作支持
  bool addColumn`
- `bool dropColumn`
- `bool alterColumn`
- `bool renameColumn`
- `bool addConstraint`
- `bool dropConstraint`
- `bool enableTrigger`
- `bool disableTrigger`

---

### WindowFunctionExecutor

**定义位置**: `include/sql_executor/window_function_executor.h`

**定义**:
```cpp
class WindowFunctionExecutor {
public:
    WindowFunctionExecutor(std::shared_ptr<DatabaseManager> db_manager);
    ~WindowFunctionExecutor() = default;

    /**
     * @brief 执行窗口函数
     * @param win...
```

**构造函数**:
- `WindowFunctionExecutor`
- `executeWindowFunction`
- `executeRowNumber`
- `executeRank`
- `executeDenseRank`
- `sortData`

**公有方法**:
- `WindowFunctionResult executeWindowFunction`
- `WindowFunctionResult executeRowNumber`
- `WindowFunctionResult executeRank`
- `WindowFunctionResult executeDenseRank`

---

### DomainManager

**定义位置**: `include/sql_executor/domain_manager.h`

**定义**:
```cpp
class DomainManager {
public:
    static DomainManager& getInstance();

    // 域管理
    bool createDomain(const sql_parser::CreateDomainStatement& stmt);
    bool alterDomain(const sql_parser::AlterDom...
```

**构造函数**:
- `getInstance`
- `createDomain`
- `alterDomain`
- `dropDomain`

**公有方法**:
- `域管理
    bool createDomain`
- `bool alterDomain`
- `bool dropDomain`

---

### SqlTriggerExecutor

**定义位置**: `include/sql_executor/sql_trigger_executor.h`

**定义**:
```cpp
class SqlTriggerExecutor {
public:
    // 构造函数
    SqlTriggerExecutor();
    explicit SqlTriggerExecutor(const std::string& name);

    // 析构函数
    ~SqlTriggerExecutor();

    // 禁用拷贝
    SqlTriggerEx...
```

**构造函数**:
- `SqlTriggerExecutor`
- `SqlTriggerExecutor`
- `SqlTriggerExecutor`
- `initialize`
- `shutdown`
- `set_name`

**析构函数**:
- `SqlTriggerExecutor`

**公有方法**:
- `构造函数
    SqlTriggerExecutor`
- `explicit SqlTriggerExecutor`
- `公共方法
    void initialize`
- `void shutdown`
- `void set_name`

---

### SetTransactionStatement

**定义位置**: `include/sql_executor/transaction_control_manager.h`

**定义**:
```cpp
class SetTransactionStatement;
} // namespace sql_parser

/**
 * Transaction Control Manager - 事务控制管理器
 * 处理SAVEPOINT、SET TRANSACTION等事务控制语句
 */
class TransactionControlManager {
public:
    static Tr...
```

**构造函数**:
- `getInstance`
- `createSavepoint`
- `releaseSavepoint`
- `rollbackToSavepoint`
- `setTransactionIsolation`
- `setTransactionAccessMode`

**公有方法**:
- `SAVEPOINT管理
    bool createSavepoint`
- `bool releaseSavepoint`
- `bool rollbackToSavepoint`
- `TRANSACTION
    bool setTransactionIsolation`
- `bool setTransactionAccessMode`

---

### CreateFunctionStatement

**定义位置**: `include/sql_executor/procedure_function_manager.h`

**定义**:
```cpp
class CreateFunctionStatement;
class AlterFunctionStatement;
class DropFunctionStatement;
class CreateProcedureStatement;
class CallStatement;
} // namespace sql_parser

/**
 * Procedure and Function ...
```

**构造函数**:
- `getInstance`
- `createFunction`
- `alterFunction`
- `dropFunction`
- `createProcedure`
- `dropProcedure`
- `callProcedure`
- `callFunction`

**公有方法**:
- `函数管理
    bool createFunction`
- `bool alterFunction`
- `bool dropFunction`
- `过程管理
    bool createProcedure`
- `bool dropProcedure`
- `调用执行
    bool callProcedure`
- `string callFunction`

---

### AlterTableStatement

**定义位置**: `include/sql_executor/enhanced_alter_table_manager.h`

**定义**:
```cpp
class AlterTableStatement;
class AddColumnAction;
class DropColumnAction;
class ModifyColumnAction;
class RenameColumnAction;
class AddConstraintAction;
class DropConstraintAction;
class RenameTableAc...
```

**构造函数**:
- `getInstance`
- `alterTable`
- `addColumn`
- `dropColumn`
- `modifyColumn`
- `renameColumn`
- `addConstraint`
- `dropConstraint`
- `renameTable`
- `validateAlterTable`
- `executeColumnAction`
- `executeConstraintAction`
- `recordOperation`

**公有方法**:
- `TABLE操作
    bool alterTable`
- `列操作
    bool addColumn`
- `bool dropColumn`
- `bool modifyColumn`
- `bool renameColumn`
- `约束操作
    bool addConstraint`
- `bool dropConstraint`
- `表操作
    bool renameTable`
- `验证操作
    bool validateAlterTable`
- `内部辅助方法
    bool executeColumnAction`
- `bool executeConstraintAction`
- `void recordOperation`

---

### TriggerExecutor

**定义位置**: `include/sql_executor/trigger_executor.h`

**定义**:
```cpp
class TriggerExecutor {
public:
    /**
     * @brief 构造函数
     */
    TriggerExecutor();

    /**
     * @brief 析构函数
     */
    ~TriggerExecutor();

    /**
     * @brief 创建触发器
     * @param trigger...
```

**构造函数**:
- `TriggerExecutor`
- `TriggerExecutor`
- `触发器类型`
- `DropTrigger`
- `ExecuteTrigger`
- `TriggerExists`
- `ListTriggers`
- `EnableTrigger`
- `DisableTrigger`
- `GetTriggerDefinition`
- `ValidateTriggerSyntax`
- `HandleTableEvent`
- `CheckTriggerRecursion`
- `GetTriggerStatistics`
- `IsValidTriggerType`
- `IsValidTriggerName`
- `GenerateTriggerId`
- `LogTriggerExecution`
- `ExecuteTriggerBody`

**析构函数**:
- `TriggerExecutor`

**公有方法**:
- `param trigger_type 触发器类型`
- `bool DropTrigger`
- `bool ExecuteTrigger`
- `bool TriggerExists`
- `bool EnableTrigger`
- `bool DisableTrigger`
- `string GetTriggerDefinition`
- `bool ValidateTriggerSyntax`
- `bool HandleTableEvent`
- `bool CheckTriggerRecursion`
- `私有辅助方法
    bool IsValidTriggerType`
- `bool IsValidTriggerName`
- `string GenerateTriggerId`
- `void LogTriggerExecution`
- `bool ExecuteTriggerBody`

---

### DatabaseManager

**定义位置**: `include/sql_executor/procedure_executor.h`

**定义**:
```cpp
class DatabaseManager;
class TransactionManager;

// 存储过程参数
struct ProcedureParameter {
    std::string name;
    std::string type;
    std::variant<int64_t, double, std::string, bool> value;
    bool...
```

---

### ProcedureExecutor

**定义位置**: `include/sql_executor/procedure_executor.h`

**定义**:
```cpp
class ProcedureExecutor {
public:
    ProcedureExecutor();
    ~ProcedureExecutor();

    // 初始化执行器
    void initialize(DatabaseManager* db_manager, TransactionManager* txn_manager);

    // 执行存储过程
  ...
```

**构造函数**:
- `ProcedureExecutor`
- `ProcedureExecutor`
- `initialize`
- `execute_procedure`
- `compile_procedure`
- `validate_procedure`
- `get_procedure_names`
- `get_procedure_definition`
- `enable_performance_monitoring`
- `get_execution_stats`
- `execute_procedure_ast`
- `execute_statement`
- `declare_variable`
- `assign_variable`
- `execute_if_statement`
- `execute_while_statement`
- `execute_call_statement`
- `evaluate_expression`
- `convert_value`
- `is_valid_identifier`
- `is_valid_type`
- `get_error_context`

**析构函数**:
- `ProcedureExecutor`

**公有方法**:
- `初始化执行器
    void initialize`
- `执行存储过程
    ProcedureResult execute_procedure`
- `编译存储过程
    bool compile_procedure`
- `验证存储过程
    bool validate_procedure`
- `string get_procedure_definition`
- `性能监控
    void enable_performance_monitoring`
- `内部执行方法
    ProcedureResult execute_procedure_ast`
- `语句执行
    void execute_statement`
- `变量管理
    void declare_variable`
- `void assign_variable`
- `控制流
    void execute_if_statement`
- `void execute_while_statement`
- `调用执行
    void execute_call_statement`
- `T convert_value`
- `工具方法
    bool is_valid_identifier`
- `bool is_valid_type`
- `string get_error_context`

---

### CreateTriggerStatement

**定义位置**: `include/sql_executor/enhanced_trigger_manager.h`

**定义**:
```cpp
class CreateTriggerStatement;
class AlterTriggerStatement;
class DropTriggerStatement;
} // namespace sql_parser

/**
 * Enhanced Trigger Manager - 增强触发器管理器
 * 处理CREATE/ALTER/DROP TRIGGER语句的高级功能
 */
c...
```

**构造函数**:
- `getInstance`
- `createTrigger`
- `alterTrigger`
- `dropTrigger`
- `executeTrigger`
- `executeTriggersForTable`
- `enableTrigger`
- `disableTrigger`

**公有方法**:
- `触发器管理
    bool createTrigger`
- `bool alterTrigger`
- `bool dropTrigger`
- `触发器执行
    bool executeTrigger`
- `bool executeTriggersForTable`
- `触发器状态管理
    bool enableTrigger`
- `bool disableTrigger`

---

### JSONExecutor

**定义位置**: `include/sql_executor/json_executor.h`

**定义**:
```cpp
class JSONExecutor {
public:
    /**
     * @brief 构造函数
     */
    JSONExecutor();

    /**
     * @brief 析构函数
     */
    ~JSONExecutor();

    /**
     * @brief 执行JSON查询操作
     * @param json_data J...
```

**构造函数**:
- `JSONExecutor`
- `JSONExecutor`
- `ExecuteJSONQuery`
- `ExecuteJSONValue`
- `ExecuteJSONArrayAgg`
- `ExecuteJSONObjectAgg`
- `ExecuteJSONModify`
- `ValidateJSON`
- `FormatJSON`
- `MinifyJSON`
- `GetJSONLength`
- `GetJSONType`
- `IsValidJSONPath`
- `EscapeJSON`
- `UnescapeJSON`

**析构函数**:
- `JSONExecutor`

**公有方法**:
- `string ExecuteJSONQuery`
- `string ExecuteJSONValue`
- `string ExecuteJSONArrayAgg`
- `string ExecuteJSONObjectAgg`
- `string ExecuteJSONModify`
- `bool ValidateJSON`
- `string FormatJSON`
- `string MinifyJSON`
- `size_t GetJSONLength`
- `string GetJSONType`
- `私有辅助方法
    bool IsValidJSONPath`
- `string EscapeJSON`
- `string UnescapeJSON`

---

