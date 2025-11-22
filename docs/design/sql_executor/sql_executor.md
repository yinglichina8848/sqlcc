# SqlExecutor类详细设计

## 概述

SqlExecutor是SQL执行引擎的核心类，负责将SQL解析器生成的抽象语法树（AST）转换为实际的数据库操作。它协调各种SQL语句的执行，管理表元数据，执行约束验证，并与存储引擎和事务管理器交互。

## 类定义

```cpp
class SqlExecutor {
public:
    SqlExecutor();
    explicit SqlExecutor(StorageEngine &storage_engine);
    ~SqlExecutor() = default;
    
    SqlExecutor(const SqlExecutor &) = delete;
    SqlExecutor &operator=(const SqlExecutor &) = delete;
    
    std::string Execute(const std::string &sql);
    std::string ExecuteFile(const std::string &file_path);
    const std::string &GetLastError() const;
    std::string ShowTableSchema(const std::string &table_name);
    std::string ListTables();

private:
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
    
    void SetError(const std::string &error);
    bool CreateTable(const std::string &table_name, 
                    const std::vector<sql_parser::ColumnDefinition> &columns,
                    const std::vector<sql_parser::TableConstraint> &constraints);
    bool DropTable(const std::string &table_name);
    const TableMetadata *GetTableMetadata(const std::string &table_name) const;
    bool InsertRecord(const std::string &table_name, const Record &record, uint64_t &rid);
    bool UpdateRecord(const std::string &table_name, uint64_t rid, const Record &new_record);
    bool DeleteRecord(const std::string &table_name, uint64_t rid);
    Record GetRecord(const std::string &table_name, uint64_t rid) const;
    std::vector<Record> GetAllRecords(const std::string &table_name) const;
    std::vector<Record> QueryRecords(const std::string &table_name, 
                                    const WhereCondition &condition) const;
    
    bool ValidateInsertConstraints(const std::string &table_name, 
                                  const std::vector<std::string> &record,
                                  const std::vector<sql_parser::ColumnDefinition> &table_schema);
    bool ValidateUpdateConstraints(const std::string &table_name, 
                                  const std::vector<std::string> &old_record,
                                  const std::vector<std::string> &new_record,
                                  const std::vector<sql_parser::ColumnDefinition> &table_schema);
    bool ValidateDeleteConstraints(const std::string &table_name, 
                                  const std::vector<std::string> &record,
                                  const std::vector<sql_parser::ColumnDefinition> &table_schema);
    bool ValidateConstraintDefinition(const sql_parser::TableConstraint &constraint,
                                     const std::vector<sql_parser::ColumnDefinition> &columns) const;
    void CreateTableConstraints(const std::string &table_name,
                               const std::vector<sql_parser::TableConstraint> &constraints);
    
    std::vector<sql_parser::ColumnDefinition> GetTableSchema(const std::string &table_name) const;
    std::string FormatQueryResults(const std::vector<Record> &results,
                                  const std::vector<size_t> &column_indices,
                                  const TableMetadata &meta) const;
    std::string FormatTableSchema(const TableMetadata &meta) const;
    std::string FormatTableList(const std::vector<std::string> &tables) const;
    std::string NormalizeTableName(const std::string &name) const;
    std::vector<WhereCondition> ParseWhereClause(const sql_parser::WhereClause &where_clause) const;

private:
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

## 构造函数

### SqlExecutor()

默认构造函数：

1. 初始化存储引擎为空指针
2. 初始化当前数据库为"default"
3. 初始化其他成员变量

### SqlExecutor(StorageEngine &storage_engine)

带参数的构造函数：

1. 初始化存储引擎引用
2. 初始化当前数据库为"default"
3. 初始化其他成员变量

## 析构函数

### ~SqlExecutor()

默认析构函数：

1. 使用默认析构函数自动清理资源

## 禁用的拷贝操作

### SqlExecutor(const SqlExecutor &) = delete

禁用拷贝构造函数：

1. 防止意外复制SqlExecutor对象

### SqlExecutor &operator=(const SqlExecutor &) = delete

禁用赋值操作符：

1. 防止意外赋值SqlExecutor对象

## 公共接口方法

### std::string Execute(const std::string &sql)

执行SQL语句：

1. 将SQL文本转换为大写进行初步识别
2. 根据SQL类型调用相应的处理函数
3. 对于SHOW TABLES等特殊命令直接处理
4. 返回执行结果

### std::string ExecuteFile(const std::string &file_path)

执行SQL脚本文件：

1. 读取文件内容
2. 解析并执行文件中的SQL语句
3. 返回执行结果

### const std::string &GetLastError() const

获取最后一次执行的错误信息：

1. 返回last_error_成员变量

### std::string ShowTableSchema(const std::string &table_name)

显示表结构信息：

1. 查找表元数据
2. 格式化表结构信息
3. 返回格式化结果

### std::string ListTables()

列出所有表：

1. 遍历表目录
2. 格式化表列表
3. 返回格式化结果

## 私有执行方法

### std::string ExecuteStatement(const sql_parser::Statement* stmt)

执行单个SQL语句：

1. 根据语句的具体类型分发到相应的执行方法

### std::string ExecuteSelect(const sql_parser::SelectStatement &select_stmt)

执行SELECT语句：

1. 解析查询条件
2. 执行查询操作
3. 格式化查询结果

### std::string ExecuteInsert(const sql_parser::InsertStatement &insert_stmt)

执行INSERT语句：

1. 解析插入数据
2. 验证约束
3. 执行插入操作

### std::string ExecuteUpdate(const sql_parser::UpdateStatement &update_stmt)

执行UPDATE语句：

1. 解析更新条件和数据
2. 验证约束
3. 执行更新操作

### std::string ExecuteDelete(const sql_parser::DeleteStatement &delete_stmt)

执行DELETE语句：

1. 解析删除条件
2. 验证约束
3. 执行删除操作

### std::string ExecuteCreate(const sql_parser::CreateStatement &create_stmt)

执行CREATE语句：

1. 解析创建对象类型和定义
2. 执行创建操作

### std::string ExecuteDrop(const sql_parser::DropStatement &drop_stmt)

执行DROP语句：

1. 解析删除对象类型和名称
2. 执行删除操作

### std::string ExecuteAlter(const sql_parser::AlterStatement &alter_stmt)

执行ALTER语句：

1. 解析修改对象类型和定义
2. 执行修改操作

### std::string ExecuteUse(const sql_parser::UseStatement &use_stmt)

执行USE语句：

1. 切换当前数据库

### std::string ExecuteCreateIndex(const sql_parser::CreateIndexStatement &create_index_stmt)

执行CREATE INDEX语句：

1. 解析索引定义
2. 执行索引创建操作

### std::string ExecuteDropIndex(const sql_parser::DropIndexStatement &drop_index_stmt)

执行DROP INDEX语句：

1. 解析索引名称
2. 执行索引删除操作

## 错误处理方法

### void SetError(const std::string &error)

设置错误信息：

1. 设置last_error_成员变量

## 数据操作方法

### bool CreateTable(const std::string &table_name, const std::vector<sql_parser::ColumnDefinition> &columns, const std::vector<sql_parser::TableConstraint> &constraints)

创建表：

1. 验证表名和定义
2. 创建表结构
3. 注册表元数据

### bool DropTable(const std::string &table_name)

删除表：

1. 验证表是否存在
2. 删除表数据和结构
3. 清理元数据

### const TableMetadata *GetTableMetadata(const std::string &table_name) const

获取表元数据：

1. 在表目录中查找表元数据

### bool InsertRecord(const std::string &table_name, const Record &record, uint64_t &rid)

插入记录：

1. 验证记录格式
2. 调用存储引擎插入记录
3. 返回记录ID

### bool UpdateRecord(const std::string &table_name, uint64_t rid, const Record &new_record)

更新记录：

1. 验证记录格式
2. 调用存储引擎更新记录

### bool DeleteRecord(const std::string &table_name, uint64_t rid)

删除记录：

1. 调用存储引擎删除记录

### Record GetRecord(const std::string &table_name, uint64_t rid) const

获取记录：

1. 调用存储引擎获取记录

### std::vector<Record> GetAllRecords(const std::string &table_name) const

获取所有记录：

1. 调用存储引擎获取表中所有记录

### std::vector<Record> QueryRecords(const std::string &table_name, const WhereCondition &condition) const

查询记录：

1. 根据条件查询记录
2. 返回匹配的记录列表

## 约束验证方法

### bool ValidateInsertConstraints(const std::string &table_name, const std::vector<std::string> &record, const std::vector<sql_parser::ColumnDefinition> &table_schema)

验证插入约束：

1. 验证新记录是否满足表约束

### bool ValidateUpdateConstraints(const std::string &table_name, const std::vector<std::string> &old_record, const std::vector<std::string> &new_record, const std::vector<sql_parser::ColumnDefinition> &table_schema)

验证更新约束：

1. 验证更新后的记录是否满足表约束

### bool ValidateDeleteConstraints(const std::string &table_name, const std::vector<std::string> &record, const std::vector<sql_parser::ColumnDefinition> &table_schema)

验证删除约束：

1. 验证删除操作是否满足约束要求

### bool ValidateConstraintDefinition(const sql_parser::TableConstraint &constraint, const std::vector<sql_parser::ColumnDefinition> &columns) const

验证约束定义：

1. 验证约束定义是否有效

### void CreateTableConstraints(const std::string &table_name, const std::vector<sql_parser::TableConstraint> &constraints)

创建表约束：

1. 为表创建约束执行器

## 辅助方法

### std::vector<sql_parser::ColumnDefinition> GetTableSchema(const std::string &table_name) const

获取表架构：

1. 返回表的列定义列表

### std::string FormatQueryResults(const std::vector<Record> &results, const std::vector<size_t> &column_indices, const TableMetadata &meta) const

格式化查询结果：

1. 将查询结果格式化为表格形式

### std::string FormatTableSchema(const TableMetadata &meta) const

格式化表结构：

1. 将表结构信息格式化为可读形式

### std::string FormatTableList(const std::vector<std::string> &tables) const

格式化表列表：

1. 将表列表格式化为表格形式

### std::string NormalizeTableName(const std::string &name) const

表名标准化：

1. 将表名转换为小写

### std::vector<WhereCondition> ParseWhereClause(const sql_parser::WhereClause &where_clause) const

解析WHERE子句：

1. 将WHERE子句解析为条件列表

## 成员变量

### std::shared_ptr<StorageEngine> storage_engine_

存储引擎智能指针：

1. 指向存储引擎实例
2. 用于执行实际的数据操作

### std::string last_error_

最后一次错误信息：

1. 存储最近一次执行的错误信息

### std::string current_database_

当前选中的数据库名称：

1. 存储当前操作的数据库名称

### std::unordered_map<std::string, TableMetadata> table_catalog_

表元数据管理器：

1. 存储所有表的元数据信息

### std::unique_ptr<Record> record_manager_

记录管理器：

1. 负责记录的CRUD操作（预留）

### IndexManager* index_executor_

索引执行器：

1. 管理表的索引操作
2. 从StorageEngine获取，使用原始指针避免类型不完整问题

### std::unique_ptr<TransactionManager> transaction_manager_

事务管理器：

1. 处理事务ACID保证（预留）

### std::unordered_map<std::string, std::vector<std::unique_ptr<ConstraintExecutor>>> table_constraints_

表约束管理器：

1. 存储每个表的约束执行器列表
2. 键：表名（小写），值：约束执行器列表

### std::mutex execution_mutex_

并发控制锁：

1. 保护执行过程中的并发访问