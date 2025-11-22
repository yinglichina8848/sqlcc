# SQL执行器设计文档

## 1. 总体设计

### 1.1 设计目标

SQL执行器（SqlExecutor）是SQLCC数据库管理系统的核心组件之一，负责将SQL解析器生成的抽象语法树（AST）转换为实际的数据库操作。设计目标包括：

1. **完整的SQL支持**：支持DDL（数据定义语言）、DML（数据操作语言）、DCL（数据控制语言）和事务处理
2. **模块化设计**：将不同类型的SQL语句执行分离到独立的方法中，便于维护和扩展
3. **事务支持**：与事务管理器集成，提供ACID事务保证
4. **约束验证**：在数据操作前验证表约束（主键、唯一性、检查约束等）
5. **错误处理**：提供清晰的错误信息和异常处理机制

### 1.2 系统架构

SQL执行器作为数据库系统的核心执行组件，位于SQL解析器和存储引擎之间：

```
SQL文本 -> SQL解析器 -> AST -> SQL执行器 -> 存储引擎 -> 磁盘
              ↑           ↓
            词法分析    执行结果
            语法分析
```

### 1.3 核心组件

1. **SqlExecutor类**：主要执行器类，负责协调各种SQL语句的执行
2. **TableMetadata**：表元数据结构，存储表的结构信息
3. **Record**：数据记录结构，表示表中的一行数据
4. **ConstraintExecutor**：约束执行器，处理表约束验证
5. **TransactionManager**：事务管理器，处理事务ACID特性

### 1.4 支持的SQL语句类型

1. **DDL（数据定义语言）**
   - CREATE DATABASE, CREATE TABLE
   - ALTER DATABASE, ALTER TABLE
   - DROP DATABASE, DROP TABLE
   - USE DATABASE

2. **DML（数据操作语言）**
   - INSERT
   - SELECT
   - UPDATE
   - DELETE

3. **DCL（数据控制语言）**
   - GRANT
   - REVOKE

4. **事务控制语句**
   - BEGIN TRANSACTION
   - COMMIT
   - ROLLBACK
   - SAVEPOINT

## 2. 详细设计

### 2.1 SqlExecutor类设计

#### 2.1.1 类结构

```cpp
class SqlExecutor {
public:
  // 构造和析构
  SqlExecutor();
  explicit SqlExecutor(StorageEngine &storage_engine);
  ~SqlExecutor() = default;

  // 禁止拷贝
  SqlExecutor(const SqlExecutor &) = delete;
  SqlExecutor &operator=(const SqlExecutor &) = delete;

  // 公共接口
  std::string Execute(const std::string &sql);
  std::string ExecuteFile(const std::string &file_path);
  const std::string &GetLastError() const;
  std::string ShowTableSchema(const std::string &table_name);
  std::string ListTables();

private:
  // 执行方法
  std::string ExecuteStatement(const sql_parser::Statement* stmt);
  std::string ExecuteSelect(const sql_parser::SelectStatement &select_stmt);
  std::string ExecuteInsert(const sql_parser::InsertStatement &insert_stmt);
  std::string ExecuteUpdate(const sql_parser::UpdateStatement &update_stmt);
  std::string ExecuteDelete(const sql_parser::DeleteStatement &delete_stmt);
  std::string ExecuteCreate(const sql_parser::CreateStatement &create_stmt);
  std::string ExecuteDrop(const sql_parser::DropStatement &drop_stmt);
  std::string ExecuteAlter(const sql_parser::AlterStatement &alter_stmt);
  std::string ExecuteUse(const sql_parser::UseStatement &use_stmt);
  std::string ExecuteCreateIndex(const sql_parser::CreateIndexStatement &create_index_stmt);
  std::string ExecuteDropIndex(const sql_parser::DropIndexStatement &drop_index_stmt);

  // 辅助方法
  void SetError(const std::string &error);
  bool CreateTable(const std::string &table_name, const std::vector<sql_parser::ColumnDefinition> &columns, const std::vector<sql_parser::TableConstraint> &constraints);
  bool DropTable(const std::string &table_name);
  const TableMetadata *GetTableMetadata(const std::string &table_name) const;
  bool InsertRecord(const std::string &table_name, const Record &record, uint64_t &rid);
  bool UpdateRecord(const std::string &table_name, uint64_t rid, const Record &new_record);
  bool DeleteRecord(const std::string &table_name, uint64_t rid);
  Record GetRecord(const std::string &table_name, uint64_t rid) const;
  std::vector<Record> GetAllRecords(const std::string &table_name) const;
  std::vector<Record> QueryRecords(const std::string &table_name, const WhereCondition &condition) const;

  // 约束验证
  bool ValidateInsertConstraints(const std::string &table_name, const std::vector<std::string> &record, const std::vector<sql_parser::ColumnDefinition> &table_schema);
  bool ValidateUpdateConstraints(const std::string &table_name, const std::vector<std::string> &old_record, const std::vector<std::string> &new_record, const std::vector<sql_parser::ColumnDefinition> &table_schema);
  bool ValidateDeleteConstraints(const std::string &table_name, const std::vector<std::string> &record, const std::vector<sql_parser::ColumnDefinition> &table_schema);
  bool ValidateConstraintDefinition(const sql_parser::TableConstraint &constraint, const std::vector<sql_parser::ColumnDefinition> &columns) const;
  void CreateTableConstraints(const std::string &table_name, const std::vector<sql_parser::TableConstraint> &constraints);

  // 格式化方法
  std::string FormatQueryResults(const std::vector<Record> &results, const std::vector<size_t> &column_indices, const TableMetadata &meta) const;
  std::string FormatTableSchema(const TableMetadata &meta) const;
  std::string FormatTableList(const std::vector<std::string> &tables) const;

  // 成员变量
  std::shared_ptr<StorageEngine> storage_engine_;
  std::string last_error_;
  std::string current_database_;
  std::unordered_map<std::string, TableMetadata> table_catalog_;
  std::unique_ptr<Record> record_manager_;
  IndexManager* index_executor_;
  std::unique_ptr<TransactionManager> transaction_manager_;
  std::unordered_map<std::string, std::vector<std::unique_ptr<ConstraintExecutor>>> table_constraints_;
  std::mutex execution_mutex_;
};
```

### 2.2 DDL（数据定义语言）实现

#### 2.2.1 CREATE语句执行

CREATE语句用于创建数据库对象，如数据库和表：

1. **CREATE DATABASE**
   - 验证数据库名称是否合法
   - 检查数据库是否已存在
   - 创建数据库目录结构
   - 初始化数据库元数据

2. **CREATE TABLE**
   - 解析表定义，包括列定义和约束
   - 验证列名唯一性
   - 验证约束定义的有效性
   - 在存储引擎中创建表结构
   - 注册表元数据到目录系统
   - 创建约束执行器

#### 2.2.2 ALTER语句执行

ALTER语句用于修改现有数据库对象的结构：

1. **ALTER DATABASE**
   - 修改数据库属性
   - 更新数据库元数据

2. **ALTER TABLE**
   - 添加/删除列
   - 添加/删除约束
   - 修改列定义
   - 重建表结构（如需要）

#### 2.2.3 DROP语句执行

DROP语句用于删除数据库对象：

1. **DROP DATABASE**
   - 验证数据库是否存在
   - 删除数据库目录结构
   - 清理相关元数据

2. **DROP TABLE**
   - 验证表是否存在
   - 删除表数据和结构
   - 清理相关约束和索引
   - 从目录系统中移除表元数据

### 2.3 DML（数据操作语言）实现

#### 2.3.1 INSERT语句执行

INSERT语句用于向表中插入新记录：

1. 解析插入数据
2. 验证表是否存在
3. 验证列数和数据类型匹配
4. 执行约束验证（主键、唯一性等）
5. 在存储引擎中插入记录
6. 更新索引（如适用）
7. 返回插入结果

#### 2.3.2 SELECT语句执行

SELECT语句用于查询表中的数据：

1. 解析查询条件
2. 验证表和列是否存在
3. 应用WHERE条件过滤
4. 执行排序和分组（如指定）
5. 从存储引擎获取数据
6. 格式化查询结果

#### 2.3.3 UPDATE语句执行

UPDATE语句用于修改表中的现有记录：

1. 解析更新条件和新值
2. 验证表和列是否存在
3. 应用WHERE条件定位记录
4. 执行约束验证
5. 在存储引擎中更新记录
6. 更新相关索引
7. 返回更新结果

#### 2.3.4 DELETE语句执行

DELETE语句用于删除表中的记录：

1. 解析删除条件
2. 验证表是否存在
3. 应用WHERE条件定位记录
4. 执行约束验证（外键约束等）
5. 在存储引擎中删除记录
6. 更新相关索引
7. 返回删除结果

### 2.4 DCL（数据控制语言）实现

#### 2.4.1 GRANT语句执行

GRANT语句用于授予用户权限：

1. 验证用户是否存在
2. 验证权限类型是否有效
3. 检查执行者是否具有授予权限
4. 更新用户权限表
5. 持久化权限信息

#### 2.4.2 REVOKE语句执行

REVOKE语句用于撤销用户权限：

1. 验证用户是否存在
2. 验证权限类型是否有效
3. 检查执行者是否具有撤销权限
4. 从用户权限表中移除权限
5. 持久化权限信息

### 2.5 事务处理实现

#### 2.5.1 事务控制语句

1. **BEGIN TRANSACTION**
   - 创建新的事务上下文
   - 设置事务隔离级别
   - 初始化事务状态

2. **COMMIT**
   - 验证事务状态
   - 将事务中所有操作持久化
   - 释放事务资源
   - 更新事务状态为已提交

3. **ROLLBACK**
   - 验证事务状态
   - 回滚事务中所有操作
   - 释放事务资源
   - 更新事务状态为已回滚

4. **SAVEPOINT**
   - 在事务中创建保存点
   - 允许部分回滚到保存点

#### 2.5.2 事务隔离级别

支持以下隔离级别：
1. READ UNCOMMITTED - 读未提交
2. READ COMMITTED - 读已提交
3. REPEATABLE READ - 可重复读
4. SERIALIZABLE - 可串行化

#### 2.5.3 并发控制

1. 使用两阶段锁协议（2PL）保证事务串行化
2. 实现死锁检测和预防机制
3. 使用多版本并发控制（MVCC）提高并发性能

### 2.6 约束管理

#### 2.6.1 约束类型支持

1. **主键约束（PRIMARY KEY）**
   - 自动创建唯一索引
   - 验证主键值唯一性
   - 不允许NULL值

2. **唯一约束（UNIQUE）**
   - 创建唯一索引
   - 验证唯一性约束

3. **外键约束（FOREIGN KEY）**
   - 验证引用完整性
   - 支持级联操作

4. **检查约束（CHECK）**
   - 验证数据满足指定条件

#### 2.6.2 约束验证时机

1. INSERT操作：验证新记录满足所有约束
2. UPDATE操作：验证更新后的记录满足所有约束
3. DELETE操作：验证删除操作不违反外键约束

### 2.7 错误处理和异常管理

1. **SqlExecutionException**：专门的SQL执行异常类
2. 详细的错误信息和错误码
3. 错误信息的记录和报告
4. 异常安全保证，确保事务一致性

## 3. 性能优化考虑

1. **查询优化**：使用索引优化查询性能
2. **批量操作**：支持批量插入和更新操作
3. **缓存机制**：缓存表元数据和查询计划
4. **并发执行**：支持多线程并发执行不同事务
5. **延迟写入**：使用WAL机制优化写入性能

## 4. 安全性考虑

1. **SQL注入防护**：使用参数化查询防止SQL注入
2. **权限控制**：基于用户权限控制数据库访问
3. **数据加密**：支持敏感数据加密存储
4. **审计日志**：记录重要操作的审计日志