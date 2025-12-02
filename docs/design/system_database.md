# 系统数据库设计文档

## 1. 概述

### 1.1 设计目标

设计并实现一个完整的系统数据库，用于存储和管理SQLCC的所有元数据信息。系统数据库将作为一个自包含的元数据管理系统，存储所有数据库对象的信息，包括数据库、用户、角色、表、列、索引、约束、视图、存储过程、触发器和权限等。

### 1.2 核心理念

系统数据库采用自包含的设计方式，即所有元数据都存储在数据库表中，而不是内存数据结构中。这样做的好处包括：

1. 持久化存储：元数据在系统重启后不会丢失
2. 一致性：所有元数据操作都通过标准的SQL语句执行
3. 可查询性：可以通过标准SQL查询系统信息
4. 可维护性：便于备份、恢复和迁移

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

### 2.2 类设计

#### 2.2.1 SystemDatabase类

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

## 6. 未来扩展

1. **分区支持**: 支持表分区的元数据管理
2. **复制机制**: 支持主从复制的元数据同步
3. **分布式支持**: 支持分布式数据库的元数据管理
4. **监控统计**: 添加系统性能监控和统计信息