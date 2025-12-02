# 系统数据库设计

## 1. 概述

### 1.1 设计目标

设计并实现一个完整的系统数据库，用于存储和管理SQLCC的所有元数据信息。系统数据库将作为一个自包含的元数据管理系统，存储所有数据库对象的信息，包括数据库、用户、角色、表、列、索引、约束、视图、存储过程、触发器和权限等。

### 1.2 核心理念

系统数据库采用自包含的设计方式，即所有元数据都存储在数据库表中，而不是内存数据结构中。这样做的好处包括：

1. 持久化存储：元数据在系统重启后不会丢失
2. 一致性：所有元数据操作都通过标准的SQL语句执行
3. 可查询性：可以通过标准SQL查询系统信息
4. 可维护性：便于备份、恢复和迁移

### 1.3 SQL标准兼容性考量

本设计充分考虑了SQL标准的发展历程，从SQL-92到SQL:2019，确保系统数据库具备良好的标准兼容性：

1. **SQL-92兼容性**：支持基本的元数据管理、用户权限管理等核心功能
2. **SQL:1999兼容性**：为用户定义类型和对象关系特性预留扩展空间
3. **SQL:2003兼容性**：支持事务管理、保存点等高级事务特性
4. **SQL:2011兼容性**：为时态数据库支持做好准备
5. **SQL:2016/2019兼容性**：支持JSON/XML数据类型和分区管理

## 2. 系统架构

### 2.1 系统数据库结构

系统数据库名为"system"，包含以下系统表：

#### 2.1.1 sys_databases - 数据库元数据表
存储所有数据库的基本信息

| 字段名 | 数据类型 | 描述 |
|--------|---------|------|
| db_id | BIGINT PRIMARY KEY | 数据库ID |
| db_name | VARCHAR(255) UNIQUE NOT NULL | 数据库名 |
| owner | VARCHAR(255) NOT NULL | 所有者 |
| created_at | TIMESTAMP NOT NULL | 创建时间 |
| description | TEXT | 描述 |

#### 2.1.2 sys_users - 用户元数据表
存储所有用户信息

| 字段名 | 数据类型 | 描述 |
|--------|---------|------|
| user_id | BIGINT PRIMARY KEY | 用户ID |
| username | VARCHAR(255) UNIQUE NOT NULL | 用户名 |
| password_hash | VARCHAR(255) NOT NULL | 密码哈希 |
| role | VARCHAR(255) NOT NULL | 默认角色 |
| current_role | VARCHAR(255) | 当前角色 |
| is_active | BOOLEAN DEFAULT TRUE | 是否激活 |
| created_at | TIMESTAMP NOT NULL | 创建时间 |

#### 2.1.3 sys_roles - 角色元数据表
存储所有角色信息

| 字段名 | 数据类型 | 描述 |
|--------|---------|------|
| role_id | BIGINT PRIMARY KEY | 角色ID |
| role_name | VARCHAR(255) UNIQUE NOT NULL | 角色名 |
| created_at | TIMESTAMP NOT NULL | 创建时间 |

#### 2.1.4 sys_tables - 表元数据表
存储所有表的基本信息

| 字段名 | 数据类型 | 描述 |
|--------|---------|------|
| table_id | BIGINT PRIMARY KEY | 表ID |
| db_id | BIGINT NOT NULL | 所属数据库ID |
| schema_name | VARCHAR(255) NOT NULL | 模式名 |
| table_name | VARCHAR(255) NOT NULL | 表名 |
| owner | VARCHAR(255) NOT NULL | 所有者 |
| created_at | TIMESTAMP NOT NULL | 创建时间 |
| table_type | VARCHAR(50) DEFAULT 'BASE TABLE' | 表类型 |

#### 2.1.5 sys_columns - 列元数据表
存储所有列的信息

| 字段名 | 数据类型 | 描述 |
|--------|---------|------|
| column_id | BIGINT PRIMARY KEY | 列ID |
| table_id | BIGINT NOT NULL | 所属表ID |
| column_name | VARCHAR(255) NOT NULL | 列名 |
| data_type | VARCHAR(100) NOT NULL | 数据类型 |
| is_nullable | BOOLEAN DEFAULT TRUE | 是否可空 |
| default_value | TEXT | 默认值 |
| ordinal_position | INT NOT NULL | 列顺序 |

#### 2.1.6 sys_indexes - 索引元数据表
存储所有索引的信息

| 字段名 | 数据类型 | 描述 |
|--------|---------|------|
| index_id | BIGINT PRIMARY KEY | 索引ID |
| table_id | BIGINT NOT NULL | 所属表ID |
| index_name | VARCHAR(255) NOT NULL | 索引名 |
| column_name | VARCHAR(255) NOT NULL | 索引列名 |
| is_unique | BOOLEAN DEFAULT FALSE | 是否唯一 |
| index_type | VARCHAR(50) DEFAULT 'BTREE' | 索引类型 |
| created_at | TIMESTAMP NOT NULL | 创建时间 |

#### 2.1.7 sys_constraints - 约束元数据表
存储所有约束的信息

| 字段名 | 数据类型 | 描述 |
|--------|---------|------|
| constraint_id | BIGINT PRIMARY KEY | 约束ID |
| table_id | BIGINT NOT NULL | 所属表ID |
| constraint_name | VARCHAR(255) NOT NULL | 约束名 |
| constraint_type | VARCHAR(50) NOT NULL | 约束类型 |
| column_name | VARCHAR(255) | 约束列名 |
| check_expression | TEXT | 检查表达式 |
| referenced_table | VARCHAR(255) | 外键引用表 |
| referenced_column | VARCHAR(255) | 外键引用列 |

#### 2.1.8 sys_views - 视图元数据表
存储所有视图的信息

| 字段名 | 数据类型 | 描述 |
|--------|---------|------|
| view_id | BIGINT PRIMARY KEY | 视图ID |
| db_id | BIGINT NOT NULL | 所属数据库ID |
| schema_name | VARCHAR(255) NOT NULL | 模式名 |
| view_name | VARCHAR(255) NOT NULL | 视图名 |
| definition | TEXT NOT NULL | 视图定义SQL |
| owner | VARCHAR(255) NOT NULL | 所有者 |
| created_at | TIMESTAMP NOT NULL | 创建时间 |

#### 2.1.9 sys_procedures - 存储过程元数据表
存储所有存储过程的信息

| 字段名 | 数据类型 | 描述 |
|--------|---------|------|
| proc_id | BIGINT PRIMARY KEY | 过程ID |
| db_id | BIGINT NOT NULL | 所属数据库ID |
| schema_name | VARCHAR(255) NOT NULL | 模式名 |
| proc_name | VARCHAR(255) NOT NULL | 过程名 |
| definition | TEXT NOT NULL | 过程定义 |
| owner | VARCHAR(255) NOT NULL | 所有者 |
| created_at | TIMESTAMP NOT NULL | 创建时间 |

#### 2.1.10 sys_triggers - 触发器元数据表
存储所有触发器的信息

| 字段名 | 数据类型 | 描述 |
|--------|---------|------|
| trigger_id | BIGINT PRIMARY KEY | 触发器ID |
| table_id | BIGINT NOT NULL | 所属表ID |
| trigger_name | VARCHAR(255) NOT NULL | 触发器名 |
| trigger_type | VARCHAR(100) NOT NULL | 触发类型 |
| trigger_body | TEXT NOT NULL | 触发器体 |
| owner | VARCHAR(255) NOT NULL | 所有者 |
| created_at | TIMESTAMP NOT NULL | 创建时间 |

#### 2.1.11 sys_privileges - 权限元数据表
存储所有权限信息

| 字段名 | 数据类型 | 描述 |
|--------|---------|------|
| privilege_id | BIGINT PRIMARY KEY | 权限ID |
| grantee_type | VARCHAR(10) NOT NULL | 被授权者类型 |
| grantee_name | VARCHAR(255) NOT NULL | 被授权者名 |
| db_name | VARCHAR(255) | 数据库名 |
| table_name | VARCHAR(255) | 表名 |
| privilege | VARCHAR(50) NOT NULL | 权限 |
| grantor | VARCHAR(255) NOT NULL | 授权者 |

### 2.2 扩展系统表（为未来SQL标准兼容性做准备）

以下系统表是为了更好地兼容SQL标准而设计，将在后续版本中实现：

#### 2.2.1 sys_audit_logs - 审计日志表
存储系统操作的审计日志，符合SQL:1999及以上标准对审计的要求

| 字段名 | 数据类型 | 描述 |
|--------|---------|------|
| log_id | BIGINT PRIMARY KEY | 日志ID |
| user_name | VARCHAR(255) NOT NULL | 用户名 |
| operation_type | VARCHAR(50) NOT NULL | 操作类型 |
| object_type | VARCHAR(50) | 对象类型 |
| object_name | VARCHAR(255) | 对象名 |
| operation_time | TIMESTAMP NOT NULL | 操作时间 |
| client_ip | VARCHAR(45) | 客户端IP |
| session_id | VARCHAR(255) | 会话ID |
| sql_text | TEXT | SQL文本 |
| affected_rows | INT | 影响行数 |
| execution_result | VARCHAR(20) | 执行结果 |

#### 2.2.2 sys_audit_policies - 审计策略表
存储审计策略配置，用于管理哪些操作需要审计

| 字段名 | 数据类型 | 描述 |
|--------|---------|------|
| policy_id | BIGINT PRIMARY KEY | 策略ID |
| object_type | VARCHAR(50) NOT NULL | 对象类型 |
| object_name | VARCHAR(255) | 对象名 |
| operation_type | VARCHAR(50) NOT NULL | 操作类型 |
| is_enabled | BOOLEAN DEFAULT TRUE | 是否启用 |
| created_at | TIMESTAMP NOT NULL | 创建时间 |
| updated_at | TIMESTAMP NOT NULL | 更新时间 |

#### 2.2.3 sys_transactions - 事务元数据表
存储事务信息，支持SQL:2003标准的事务管理特性

| 字段名 | 数据类型 | 描述 |
|--------|---------|------|
| transaction_id | VARCHAR(255) PRIMARY KEY | 事务ID |
| session_id | VARCHAR(255) | 会话ID |
| user_name | VARCHAR(255) | 用户名 |
| start_time | TIMESTAMP NOT NULL | 开始时间 |
| end_time | TIMESTAMP | 结束时间 |
| status | VARCHAR(20) NOT NULL | 状态 |
| isolation_level | VARCHAR(20) | 隔离级别 |
| client_ip | VARCHAR(45) | 客户端IP |

#### 2.2.4 sys_savepoints - 保存点元数据表
存储事务保存点信息，支持SQL:2003标准的保存点特性

| 字段名 | 数据类型 | 描述 |
|--------|---------|------|
| savepoint_id | BIGINT PRIMARY KEY | 保存点ID |
| transaction_id | VARCHAR(255) NOT NULL | 事务ID |
| savepoint_name | VARCHAR(255) NOT NULL | 保存点名称 |
| created_at | TIMESTAMP NOT NULL | 创建时间 |

#### 2.2.5 sys_cluster_nodes - 集群节点表
存储分布式节点信息，为分布式数据库支持做准备

| 字段名 | 数据类型 | 描述 |
|--------|---------|------|
| node_id | VARCHAR(255) PRIMARY KEY | 节点ID |
| node_name | VARCHAR(255) NOT NULL | 节点名 |
| host_address | VARCHAR(255) NOT NULL | 主机地址 |
| port | INT NOT NULL | 端口 |
| status | VARCHAR(20) NOT NULL | 状态 |
| role | VARCHAR(20) NOT NULL | 角色 |
| joined_at | TIMESTAMP NOT NULL | 加入时间 |
| last_heartbeat | TIMESTAMP | 最后心跳时间 |

#### 2.2.6 sys_distributed_transactions - 分布式事务表
存储分布式事务信息，支持分布式事务管理

| 字段名 | 数据类型 | 描述 |
|--------|---------|------|
| dt_id | VARCHAR(255) PRIMARY KEY | 分布式事务ID |
| coordinator_node | VARCHAR(255) NOT NULL | 协调节点 |
| status | VARCHAR(20) NOT NULL | 状态 |
| created_at | TIMESTAMP NOT NULL | 创建时间 |
| updated_at | TIMESTAMP NOT NULL | 更新时间 |
| timeout_seconds | INT DEFAULT 30 | 超时秒数 |

#### 2.2.7 sys_distributed_objects - 分布式对象映射表
存储分布式对象映射信息，管理分布式环境中对象的分布

| 字段名 | 数据类型 | 描述 |
|--------|---------|------|
| object_id | BIGINT PRIMARY KEY | 对象ID |
| object_type | VARCHAR(50) NOT NULL | 对象类型 |
| object_name | VARCHAR(255) NOT NULL | 对象名 |
| database_name | VARCHAR(255) NOT NULL | 数据库名 |
| shard_key | VARCHAR(255) | 分片键 |
| node_mapping | TEXT | 节点分布信息(JSON) |
| replication_factor | INT DEFAULT 1 | 复制因子 |
| created_at | TIMESTAMP NOT NULL | 创建时间 |

#### 2.2.8 sys_temporal_tables - 时态表信息表
存储时态表信息，符合SQL:2011标准要求

| 字段名 | 数据类型 | 描述 |
|--------|---------|------|
| temporal_id | BIGINT PRIMARY KEY | 时态ID |
| table_id | BIGINT NOT NULL | 表ID |
| system_time_start_column | VARCHAR(255) NOT NULL | 系统时间开始列 |
| system_time_end_column | VARCHAR(255) NOT NULL | 系统时间结束列 |
| period_start | TIMESTAMP NOT NULL | 有效期开始 |
| period_end | TIMESTAMP | 有效期结束 |
| retention_period_days | INT | 保留天数 |
| created_at | TIMESTAMP NOT NULL | 创建时间 |

### 2.3 类设计

#### 2.3.1 SystemDatabase类

SystemDatabase类是系统数据库的核心管理类，负责初始化系统数据库、管理系统表以及提供元数据操作接口。

```cpp
class SystemDatabase {
public:
    SystemDatabase(std::shared_ptr<DatabaseManager> db_manager);
    ~SystemDatabase();
    
    bool Initialize();
    bool Exists();
    
    // 数据库元数据操作
    bool CreateDatabaseRecord(const std::string& db_name, const std::string& owner, const std::string& description = "");
    bool DropDatabaseRecord(const std::string& db_name);
    SysDatabase GetDatabaseRecord(const std::string& db_name);
    std::vector<SysDatabase> ListDatabases();
    bool DatabaseExists(const std::string& db_name);
    
    // 用户元数据操作
    bool CreateUserRecord(const std::string& username, const std::string& password_hash, const std::string& role);
    bool DropUserRecord(const std::string& username);
    bool UpdateUserRecord(const SysUser& user);
    SysUser GetUserRecord(const std::string& username);
    std::vector<SysUser> ListUsers();
    bool UserExists(const std::string& username);
    
    // 角色元数据操作
    bool CreateRoleRecord(const std::string& role_name);
    bool DropRoleRecord(const std::string& role_name);
    SysRole GetRoleRecord(const std::string& role_name);
    std::vector<SysRole> ListRoles();
    bool RoleExists(const std::string& role_name);
    
    // 表元数据操作
    bool CreateTableRecord(int64_t db_id, const std::string& schema_name, const std::string& table_name,
                          const std::string& owner, const std::string& table_type = "BASE TABLE");
    bool DropTableRecord(const std::string& schema_name, const std::string& table_name);
    SysTable GetTableRecord(const std::string& schema_name, const std::string& table_name);
    std::vector<SysTable> ListTables(int64_t db_id);
    bool TableExists(const std::string& schema_name, const std::string& table_name);
    
    // 列元数据操作
    bool CreateColumnRecord(int64_t table_id, const std::string& column_name, const std::string& data_type,
                           bool is_nullable, const std::string& default_value, int ordinal_position);
    bool DropColumnRecord(int64_t table_id, const std::string& column_name);
    std::vector<SysColumn> GetTableColumns(int64_t table_id);
    
    // 索引元数据操作
    bool CreateIndexRecord(int64_t table_id, const std::string& index_name, const std::string& column_name,
                          bool is_unique, const std::string& index_type);
    bool DropIndexRecord(int64_t table_id, const std::string& index_name);
    std::vector<SysIndex> GetTableIndexes(int64_t table_id);
    
    // 约束元数据操作
    bool CreateConstraintRecord(int64_t table_id, const std::string& constraint_name, const std::string& constraint_type,
                               const std::string& column_name, const std::string& check_expression = "",
                               const std::string& referenced_table = "", const std::string& referenced_column = "");
    bool DropConstraintRecord(int64_t table_id, const std::string& constraint_name);
    std::vector<SysConstraint> GetTableConstraints(int64_t table_id);
    
    // 视图元数据操作
    bool CreateViewRecord(int64_t db_id, const std::string& schema_name, const std::string& view_name,
                         const std::string& definition, const std::string& owner);
    bool DropViewRecord(const std::string& schema_name, const std::string& view_name);
    SysView GetViewRecord(const std::string& schema_name, const std::string& view_name);
    std::vector<SysView> ListViews(int64_t db_id);
    
    // 权限元数据操作
    bool GrantPrivilegeRecord(const std::string& grantee_type, const std::string& grantee_name,
                             const std::string& db_name, const std::string& table_name,
                             const std::string& privilege, const std::string& grantor);
    bool RevokePrivilegeRecord(const std::string& grantee_type, const std::string& grantee_name,
                              const std::string& db_name, const std::string& table_name,
                              const std::string& privilege);
    std::vector<SysPrivilege> GetUserPrivileges(const std::string& username);
    
    // 审计功能
    bool CreateAuditLog(const std::string& user_name, const std::string& operation_type,
                       const std::string& object_type, const std::string& object_name,
                       const std::string& client_ip, const std::string& session_id,
                       const std::string& sql_text, int affected_rows,
                       const std::string& execution_result);
    bool CreateAuditPolicy(const std::string& object_type, const std::string& object_name,
                          const std::string& operation_type, bool is_enabled);
    std::vector<SysAuditLog> GetAuditLogs(time_t start_time, time_t end_time);
    std::vector<SysAuditPolicy> GetAuditPolicies();
    
    // 事务功能
    bool CreateTransactionRecord(const std::string& transaction_id, const std::string& session_id,
                               const std::string& user_name, const std::string& client_ip,
                               const std::string& isolation_level);
    bool UpdateTransactionStatus(const std::string& transaction_id, const std::string& status,
                                time_t end_time);
    bool CreateSavepointRecord(const std::string& transaction_id, const std::string& savepoint_name);
    std::vector<SysTransaction> GetActiveTransactions();
    
    // 分布式功能
    bool RegisterClusterNode(const std::string& node_id, const std::string& node_name,
                           const std::string& host_address, int port,
                           const std::string& role);
    bool UpdateNodeStatus(const std::string& node_id, const std::string& status,
                         time_t last_heartbeat);
    bool CreateDistributedTransaction(const std::string& dt_id, const std::string& coordinator_node);
    bool UpdateDistributedTransactionStatus(const std::string& dt_id, const std::string& status);
    bool RegisterDistributedObject(int64_t object_id, const std::string& object_type,
                                  const std::string& object_name, const std::string& database_name,
                                  const std::string& shard_key, const std::string& node_mapping,
                                  int replication_factor);
    std::vector<SysClusterNode> GetClusterNodes();
    std::vector<SysDistributedTransaction> GetActiveDistributedTransactions();
    
    std::shared_ptr<DatabaseManager> GetDatabaseManager() { return db_manager_; }
    const std::string& GetLastError() const { return last_error_; }

private:
    std::shared_ptr<DatabaseManager> db_manager_;
    std::string last_error_;
    
    bool CreateSystemTables();
    bool CreateSysDatabasesTable();
    bool CreateSysUsersTable();
    bool CreateSysRolesTable();
    bool CreateSysTablesTable();
    bool CreateSysColumnsTable();
    bool CreateSysIndexesTable();
    bool CreateSysConstraintsTable();
    bool CreateSysViewsTable();
    bool CreateSysProceduresTable();
    bool CreateSysTriggersTable();
    bool CreateSysPrivilegesTable();
    
    // 扩展表创建方法
    bool CreateSysAuditLogsTable();
    bool CreateSysAuditPoliciesTable();
    bool CreateSysTransactionsTable();
    bool CreateSysSavepointsTable();
    bool CreateSysClusterNodesTable();
    bool CreateSysDistributedTransactionsTable();
    bool CreateSysDistributedObjectsTable();
    bool CreateSysTemporalTablesTable();
    
    bool InitializeDefaultData();
    std::string GetCurrentTimeString() const;
    int64_t GenerateId(const std::string& table_name);
    
    void SetError(const std::string& error) { last_error_ = error; }
};
```

## 3. 实现细节

### 3.1 初始化流程

1. 检查system数据库是否存在
2. 如果不存在，则创建system数据库
3. 切换到system数据库
4. 创建所有系统表
5. 初始化默认数据（如root用户、admin角色等）

### 3.2 自包含方式的优势

1. **持久化**: 所有元数据都存储在磁盘上，系统重启后不会丢失
2. **一致性**: 所有元数据操作都通过标准SQL执行，保证ACID特性
3. **可查询**: 可以通过标准SQL查询系统信息，便于调试和管理
4. **可扩展**: 添加新的元数据类型只需创建新的系统表和相应接口
5. **标准化**: 使用标准SQL操作，符合数据库设计原则

### 3.3 与其他模块的交互

1. **DatabaseManager**: SystemDatabase依赖DatabaseManager执行实际的数据库操作
2. **UserManager**: UserManager将使用SystemDatabase进行用户管理
3. **SchemaManager**: SchemaManager将使用SystemDatabase进行模式管理
4. **DCL/DDL执行器**: DCL和DDL执行器将通过SystemDatabase执行真实的元数据操作

## 4. 安全考虑

1. **访问控制**: 只有特权用户才能直接访问系统表
2. **审计日志**: 所有系统表的修改都应该记录审计日志
3. **数据完整性**: 使用外键约束保证元数据的一致性
4. **备份恢复**: 系统数据库应定期备份，支持灾难恢复

## 5. 性能优化

1. **索引优化**: 为常用的查询字段创建索引
2. **缓存机制**: 在内存中缓存热点元数据，减少磁盘I/O
3. **批量操作**: 支持批量插入和更新操作，提高性能
4. **查询优化**: 优化常用查询语句，减少执行时间

## 6. SQL标准兼容性发展路线

### 6.1 当前状态（SQL-92级别）
- 基本的元数据管理功能
- 用户和权限管理
- 表、列、索引、约束管理

### 6.2 近期目标（SQL:1999/2003级别）
- 增强审计功能
- 完善事务管理
- 支持保存点

### 6.3 中期目标（SQL:2008/2011级别）
- 支持时态数据库
- 增强分布式支持

### 6.4 长期目标（SQL:2016/2019级别）
- 支持JSON/XML数据类型
- 完善分区管理
- 支持高级分析功能

## 7. 未来扩展

1. **分区支持**: 支持表分区的元数据管理
2. **复制机制**: 支持主从复制的元数据同步
3. **分布式支持**: 支持分布式数据库的元数据管理
4. **监控统计**: 添加系统性能监控和统计信息