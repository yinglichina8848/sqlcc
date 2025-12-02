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
    std::string ExecuteDCL(const sql_parser::Statement* stmt); // 新增DCL语句执行方法
    
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
    std::shared_ptr<SystemDatabase> system_database_; // 系统数据库管理器
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
2. 初始化系统数据库管理器
3. 初始化当前数据库为"default"
4. 初始化其他成员变量

### SqlExecutor(StorageEngine &storage_engine)

带参数的构造函数：

1. 初始化存储引擎引用
2. 初始化系统数据库管理器
3. 初始化当前数据库为"default"
4. 初始化其他成员变量

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

### std::string ExecuteDCL(const sql_parser::Statement* stmt)

执行DCL（数据控制语言）语句：

1. 检查用户权限
2. 根据具体语句类型分发到相应的处理函数
3. 调用SystemDatabase执行真实的用户和权限管理操作
4. 返回执行结果

支持的DCL语句包括：
- CREATE USER - 创建用户
- DROP USER - 删除用户
- GRANT - 授予权限
- REVOKE - 撤销权限

## 系统数据库集成

### SystemDatabase集成

SqlExecutor集成了SystemDatabase管理器，用于执行真实的元数据操作：

1. 在初始化时创建SystemDatabase实例
2. 所有的DDL/DCL操作都会通过SystemDatabase执行
3. SystemDatabase提供了所有元数据的持久化存储
4. 通过SystemDatabase可以查询用户、表、列等各种元数据信息

### 元数据管理

所有的元数据操作都通过SystemDatabase进行：

1. 表元数据：表结构、列信息、索引信息等
2. 用户元数据：用户信息、角色信息、权限信息等
3. 数据库元数据：数据库信息、视图信息等

这种方式确保了元数据的一致性和持久化。
