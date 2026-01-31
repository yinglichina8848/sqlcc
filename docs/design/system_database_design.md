# SystemDatabase (Metadata Catalog) Design Document

**Document Version**: 1.0  
**Last Updated**: 2026-01-31  
**Author**: Gemini AI Agent  
**Related Files**: `src/sql_executor/system_database.h`, `src/sql_executor/system_database.cpp`, `src/core/database_manager.h`, `src/sql_executor/sql_executor.h`

---

## 1. WHY: 为什么要设计 SystemDatabase（系统数据库）？

在任何复杂的数据库管理系统 (DBMS) 中，除了存储用户数据之外，还需要存储关于数据库自身的“数据”，即**元数据 (Metadata)**。这些元数据描述了数据库的结构（模式），包括数据库、表、列、索引、约束、视图、存储过程、触发器等对象的信息，还包括用户、角色、权限、审计日志、事务状态、集群节点等系统级管理信息。

`SystemDatabase` 的设计旨在解决以下核心挑战：

1.  **自托管元数据 (Self-Hosting Metadata)**:
    *   将数据库的元数据作为普通数据存储在特殊的“系统表”中。这些系统表位于一个名为 `system` 的专用数据库内。
    *   **优点**: 使得元数据本身可以像普通数据一样被查询、管理、备份和恢复，极大简化了数据库内部的实现和维护。数据库的工具（如查询优化器、执行器）可以直接使用 SQL 来操作元数据，而不是特殊的、硬编码的接口。
2.  **统一管理界面**: 提供统一的接口来访问和操作所有类型的元数据，避免了元数据管理逻辑的碎片化。无论管理用户还是表结构，都通过 `SystemDatabase` 的统一 API 进行。
3.  **动态可扩展性**: 当需要添加新的数据库对象类型、新的元数据属性或新的系统级管理信息时，只需在 `system` 数据库中创建新的系统表或修改现有表结构。这种扩展性是高度灵活且低成本的。
4.  **一致性与持久性**: 作为系统核心，其元数据必须始终保持一致并能抵抗系统故障。`SystemDatabase` 依赖于底层 `DatabaseManager` 和事务机制来保证这些特性。
5.  **与其他组件集成**: `SystemDatabase` 作为一个元数据服务层，方便其他核心组件（如 `UserManager`, `PermissionValidator`, `QueryOptimizer`, `ExecutionEngine` 等）查询和更新其所需的元数据信息。

---

## 2. WHAT: SystemDatabase 的核心功能和组件？

`SystemDatabase` 类是 SQLCC 数据库的元数据目录。它不直接存储数据，而是通过构建和执行 SQL 语句，间接操作存储在 `system` 数据库中的一系列系统表来管理所有元数据。

### 2.1. 核心功能

*   **初始化 (`Initialize`)**: 负责在系统启动时，检查 `system` 数据库是否存在，若不存在则创建，并创建所有必需的系统表和初始化默认数据。
*   **元数据 CRUD (创建、读取、更新、删除)**: 为各种数据库对象（数据库、用户、角色、表、列、索引、约束、视图、存储过程、触发器、权限、审计日志、事务、集群节点、分布式对象、时态表）提供创建、删除、更新和查询的接口。
*   **一致性检查 (`CheckDatabaseConsistency`, `CheckTableConsistency` 等)**: 提供元数据层面的基本一致性检查功能。
*   **辅助工具**: 提供唯一 ID 生成 (`GenerateId`) 和时间字符串生成 (`GetCurrentTimeString`) 等辅助功能。
*   **错误管理**: 记录并报告操作过程中遇到的错误 (`SetError`, `GetLastError`)。

### 2.2. 核心系统表 (Schema 示例)

`SystemDatabase` 通过在 `system` 数据库中创建和维护一系列系统表来存储元数据。以下是一些关键系统表的示例及其主要字段：

*   **`sys_databases`**: 存储所有数据库的基本信息。
    *   `db_id` (BIGINT PRIMARY KEY), `db_name` (VARCHAR UNIQUE), `owner`, `created_at`, `description`
*   **`sys_users`**: 存储用户认证和角色信息。
    *   `user_id` (BIGINT PRIMARY KEY), `username` (VARCHAR UNIQUE), `password_hash`, `role`, `current_role`, `is_active`, `created_at`
*   **`sys_roles`**: 存储角色定义。
    *   `role_id` (BIGINT PRIMARY KEY), `role_name` (VARCHAR UNIQUE), `created_at`
*   **`sys_tables`**: 存储所有表的结构信息。
    *   `table_id` (BIGINT PRIMARY KEY), `db_id`, `schema_name`, `table_name`, `owner`, `created_at`, `table_type`
*   **`sys_columns`**: 存储所有列的详细信息。
    *   `column_id` (BIGINT PRIMARY KEY), `table_id`, `column_name`, `data_type`, `is_nullable`, `default_value`, `ordinal_position`
*   **`sys_indexes`**: 存储索引信息。
    *   `index_id` (BIGINT PRIMARY KEY), `table_id`, `index_name`, `column_name`, `is_unique`, `index_type`, `created_at`
*   **`sys_constraints`**: 存储约束信息（主键、外键、唯一、检查）。
    *   `constraint_id` (BIGINT PRIMARY KEY), `table_id`, `constraint_name`, `constraint_type`, `column_name`, `check_expression`, `referenced_table`, `referenced_column`
*   **`sys_privileges`**: 存储权限授予信息。
    *   `privilege_id` (BIGINT PRIMARY KEY), `grantee_type`, `grantee_name`, `db_name`, `table_name`, `privilege`, `grantor`, `granted_at`
*   **`sys_audit_logs`**: 存储审计日志。
    *   `log_id` (BIGINT PRIMARY KEY), `user_name`, `operation_type`, `object_type`, `object_name`, `operation_time`, `client_ip`, `session_id`, `sql_text`, `affected_rows`, `execution_result`
*   **`sys_transactions`**: 存储活跃事务状态。
    *   `transaction_id` (VARCHAR PRIMARY KEY), `session_id`, `user_name`, `start_time`, `end_time`, `status`, `isolation_level`, `client_ip`
*   **`sys_cluster_nodes`**: 存储集群节点信息。
    *   `node_id` (VARCHAR PRIMARY KEY), `node_name`, `host_address`, `port`, `status`, `role`, `joined_at`, `last_heartbeat`

### 2.3. 依赖关系

*   **`DatabaseManager`**: `SystemDatabase` 依赖 `DatabaseManager` 提供最基本的数据库操作（如创建数据库、创建表、执行 SQL）。
*   **`SqlExecutor`**: `SystemDatabase` 内部通过 `SqlExecutor` 来解析和执行操作系统表的 SQL 语句。

---

## 3. HOW: SystemDatabase 的工作流程和实现细节？

### 3.1. 初始化流程 (`Initialize()`)

1.  **检查数据库**: 调用 `Exists()` 检查 `system` 数据库是否已存在。
2.  **创建数据库**: 如果 `system` 数据库不存在，则通过 `db_manager_->CreateDatabase(SYSTEM_DB_NAME)` 创建它。
3.  **切换上下文**: 通过 `db_manager_->UseDatabase(SYSTEM_DB_NAME)` 切换到 `system` 数据库，确保所有后续操作都针对正确的数据库。
4.  **创建系统表**: 调用 `CreateSystemTables()`，该方法会依次调用每个 `CreateSysXxxTable()` 函数，确保所有必要的系统表都被创建。
5.  **初始化默认数据**: 调用 `InitializeDefaultData()` 插入默认的用户、角色等关键数据（`TODO(#SYSDB-002)` 待完善）。

### 3.2. 元数据操作模式

`SystemDatabase` 中大部分元数据操作（例如 `CreateDatabaseRecord`, `DropTableRecord`, `UpdateUserRecord` 等）遵循以下模式：

1.  **构建 SQL 语句**: 根据操作类型和参数，使用 `std::stringstream` 动态构建相应的 `INSERT`, `UPDATE`, `DELETE`, `SELECT` SQL 语句。
2.  **数据库上下文切换**: 
    *   保存当前的数据库上下文 (`db_manager_->GetCurrentDatabase()`)。
    *   通过 `db_manager_->UseDatabase(SYSTEM_DB_NAME)` 临时切换到 `system` 数据库。
3.  **执行 SQL**: 调用 `ExecuteSQL(sql_string)` 或 `ExecuteSelectQuery(sql_string)` 来执行构建的 SQL 语句。
4.  **结果处理**: 
    *   `ExecuteSQL` 检查返回字符串中的错误标志（`"Error"` 或 `"ERROR"`)。（`TODO(#SYSDB-004)`: 期望结构化结果）。
    *   `ExecuteSelectQuery` 返回查询结果字符串。（`TODO(#SYSDB-005)`: 期望结构化结果）。
    *   对于需要解析结构化数据的查询（如 `GetDatabaseRecord`, `ListUsers`），目前是 `TODO(#SYSDB-006)` 和 `TODO(#SYSDB-007)`。
5.  **恢复上下文**: 切换回之前保存的数据库上下文。
6.  **错误报告**: 使用 `SetError()` 记录内部错误信息。

### 3.3. 关键实现细节

*   **ID 生成 (`GenerateId`)**: 目前使用时间戳来生成唯一 ID。**`TODO(#SYSDB-003)`**: 生产环境需要更健壮的分布式唯一 ID 生成器。
*   **时间字符串 (`GetCurrentTimeString`)**: 用于记录创建和更新的时间戳。
*   **`ExecuteSQL` & `ExecuteSelectQuery`**: 这是执行 SQL 的核心辅助方法。**`TODO(#SYSDB-004, #SYSDB-005)`**: 它们当前返回字符串或布尔值，期望更结构化的结果类型，以便于错误处理和元数据解析。
*   **错误处理**: 许多方法中对数据库上下文切换和 SQL 执行的错误处理都标记为 `TODO(#SYSDB-008)`，表示需要更健壮的错误处理机制。

### 3.4. 简化的类图

```mermaid
classDiagram
    class SystemDatabase {
        -std::shared_ptr<DatabaseManager> db_manager_
        -std::string last_error_
        +SystemDatabase(db_manager)
        +Initialize(): bool
        +Exists(): bool
        +CreateSystemTables(): bool
        +InitializeDefaultData(): bool // TODO(#SYSDB-002)
        +CreateDatabaseRecord(...): bool
        +DropDatabaseRecord(...): bool
        +GetDatabaseRecord(...): SysDatabase // TODO(#SYSDB-006)
        +ListDatabases(): vector<SysDatabase> // TODO(#SYSDB-007)
        +DatabaseExists(...): bool
        +CreateUserRecord(...): bool
        +DropUserRecord(...): bool
        +UpdateUserRecord(...): bool
        +GetUserRecord(...): SysUser // TODO(#SYSDB-006)
        +ListUsers(): vector<SysUser> // TODO(#SYSDB-007)
        +UserExists(...): bool
        // ... many other CRUD and consistency check methods for system tables ...
        -ExecuteSQL(sql): bool // TODO(#SYSDB-004)
        -ExecuteSelectQuery(sql): string // TODO(#SYSDB-005)
        -GenerateId(table_name): int64_t // TODO(#SYSDB-003)
        -GetCurrentTimeString(): string
        -SetError(msg): void
    }

    class DatabaseManager {
        +CreateDatabase(name): bool
        +UseDatabase(name): bool
        +GetCurrentDatabase(): string
        +CreateTable(name, cols): bool
        +TableExists(name): bool
        // ... other low-level database operations
    }

    class SqlExecutor {
        +Execute(sql): string
        // ... methods to parse and execute SQL
    }

    SystemDatabase "1" --> "1" DatabaseManager : uses
    SystemDatabase "1" --> "1" SqlExecutor : uses
    SystemDatabase ..> SysDatabase : manages
    SystemDatabase ..> SysUser : manages
    // ... many other SysXxx structures
```

---

## 4. 总结与 TODO 列表

`SystemDatabase` 作为 SQLCC 的元数据目录，是实现数据库自省和管理自身模式的关键。本设计文档和对应的代码注释详细阐述了其构建和操作核心系统表以管理元数据的方法。

**当前版本的关键 TODO 列表：**

*   **`SystemDatabase` 构造函数依赖 (`#SYSDB-001`)**: `SystemDatabase` 内部应直接与存储层交互，而不是通过 `SqlExecutor` 执行字符串 SQL。这需要一个更底层的元数据存储接口。
*   **初始化默认数据 (`#SYSDB-002`)**: `InitializeDefaultData()` 需要完整实现向系统表插入默认用户、角色等数据的逻辑。
*   **唯一 ID 生成 (`#SYSDB-003`)**: `GenerateId()` 在生产环境中需要升级为更健壮的分布式唯一 ID 生成器（如雪花算法）。
*   **`ExecuteSQL` 结果处理 (`#SYSDB-004`)**: `ExecuteSQL` 预期返回结构化结果而非布尔值，以便更准确地判断成功/失败和错误详情。
*   **`ExecuteSelectQuery` 结果解析 (`#SYSDB-005`)**: `ExecuteSelectQuery` 预期返回结构化结果而非字符串，以便 `SystemDatabase` 能够可靠地解析和转换元数据。目前的字符串解析逻辑不健壮。
*   **结构化查询结果解析 (`#SYSDB-006`, `#SYSDB-007`)**: 所有 `GetXxxRecord` 和 `ListXxx` 方法都需要完整实现 SELECT 查询、执行及将结果解析为 `SysXxx` 结构体或其向量的逻辑。
*   **健壮的错误处理和数据库上下文切换 (`#SYSDB-008`)**: 多数元数据操作中涉及的数据库上下文切换和 SQL 执行的错误处理需要更健壮的机制，以确保在失败时能正确回滚或恢复。
*   **视图、审计、事务、分布式、时态元数据操作 (`#SYSDB-009` ~ `#SYSDB-021`)**: 许多针对这些高级功能的元数据操作方法仍是 `TODO` 状态，需要完整实现其 CRUD 逻辑。

未来的工作将集中在完成上述 TODO 项，尤其是在 `QueryExecutor` 返回结构化结果类型之后，将大大简化 `SystemDatabase` 的实现，使其能更可靠地管理数据库元数据。
