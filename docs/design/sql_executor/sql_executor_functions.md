# SQL Executor 函数设计文档

## 函数列表

### system_db_

**定义位置**: `src/sql_executor/unified_query_plan.cpp`

**签名**:
```cpp
      system_db_(system_db), status_(QueryPlanStatus::PENDING),
      current_database_(""), current...
```

---

### constexpr

**定义位置**: `src/sql_executor/procedure_executor.cpp`

**签名**:
```cpp
    if constexpr (std::is_same_v<T, int64_t>) {
```

---

### current_table_name_

**定义位置**: `src/sql_executor/constraint_executor.cpp`

**签名**:
```cpp
      current_table_name_("unknown") {
  // 将外键列名转换为小写，以便匹配
  for (const auto &col : constraint_.get...
```

---

### table_name_

**定义位置**: `src/sql_executor/constraint_executor.cpp`

**签名**:
```cpp
      table_name_(table_name), is_primary_key_(is_primary_key) {
```

---

### ProcedureParameter

**定义位置**: `include/sql_executor/procedure_executor.h`

**签名**:
```cpp

    ProcedureParameter(const std::string& n, const std::string& t, bool out = false)
        : name...
```

---

### ProcedureResult

**定义位置**: `include/sql_executor/procedure_executor.h`

**签名**:
```cpp
    ProcedureResult(bool s) : success(s) {
```

---

