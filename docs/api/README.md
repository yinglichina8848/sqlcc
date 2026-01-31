# SQLCC API 文档

## 概述

SQLCC (SQL Cloud Computing Database System) 是一个企业级内存安全的云原生数据库系统。本文档提供了完整的API参考和使用指南。

## 核心模块

### 存储引擎 (Storage Engine)
- **BufferPool**: 缓存管理器，支持多种替换策略
- **BPlusTree**: B+树索引实现，支持高效查找和范围查询
- **TableStorage**: 表存储管理，支持定长和变长记录
- **DiskManager**: 磁盘I/O管理器
- **WALManager**: 预写日志管理器

### SQL解析器 (SQL Parser)
- **Parser**: SQL语法解析器
- **Lexer**: 词法分析器
- **AST Nodes**: 抽象语法树节点定义

### 执行引擎 (Execution Engine)
- **UnifiedExecutor**: 统一查询执行器
- **DDLExecutor**: DDL语句执行器
- **DMLExecutor**: DML语句执行器
- **DCLExecutor**: DCL语句执行器

### 网络通信 (Network)
- **ServerNetworkManager**: 服务器网络管理器
- **ClientNetworkManager**: 客户端网络管理器
- **Encryption**: SSL/TLS加密支持

## 主要类接口

### 主要类接口

### StorageEngine 类

```cpp
class StorageEngine : public std::enable_shared_from_this<StorageEngine> {
public:
    // 构造函数
    explicit StorageEngine(ConfigManager &config_manager, const std::string& db_path = "./data");

    // 页面管理
    std::unique_ptr<Page> NewPage(int32_t *page_id = nullptr);
    std::shared_ptr<Page> FetchPage(int32_t page_id);
    bool UnpinPage(int32_t page_id, bool is_dirty = false);
    bool FlushPage(int32_t page_id);
    bool DeletePage(int32_t page_id);
    void FlushAllPages();

    // 组件访问
    DiskManager *GetDiskManager() const;
    BufferPoolSharded *GetBufferPool() const;
    IndexManager *GetIndexManager() const;

    // 统计信息
    std::string GetStats() const;
};
```

### UnifiedExecutor 类

```cpp
class UnifiedExecutor : public ExecutionEngine {
public:
    UnifiedExecutor(std::shared_ptr<DatabaseManager> db_manager);
    UnifiedExecutor(std::shared_ptr<DatabaseManager> db_manager,
                    std::shared_ptr<UserManager> user_manager,
                    std::shared_ptr<SystemDatabase> system_db);

    // 执行SQL语句
    ExecutionResult execute(std::unique_ptr<sql_parser::Statement> stmt) override;
    ExecutionResult execute(std::unique_ptr<sql_parser::Statement> stmt,
                           std::shared_ptr<ExecutionContext> context);

    // 获取数据库管理器
    std::shared_ptr<DatabaseManager> getDatabaseManager() const;
};
```

### Parser 类

```cpp
class Parser final : public ParserCore {
public:
    /**
     * @brief Parser构造函数
     * @param input SQL输入字符串
     */
    Parser(const std::string& input);

    /**
     * 解析SQL语句的主入口
     * @return 解析后的AST节点列表
     */
    std::vector<std::unique_ptr<Statement>> parse();

    // 错误处理
    std::vector<std::string> getDetailedErrors() const;
    void clearErrors();
    bool hadError() const;
};
```

## 使用示例

### 基本查询执行

```cpp
#include "database_manager/database_manager.h"
#include "sql_executor/sql_executor.h"

// 初始化数据库管理器
auto db_manager = std::make_shared<DatabaseManager>();
db_manager->createDatabase("test_db");
db_manager->useDatabase("test_db");

// 创建表
std::string createTableSQL = R"(
    CREATE TABLE users (
        id INT PRIMARY KEY,
        name VARCHAR(100),
        email VARCHAR(100)
    )
)";
db_manager->executeQuery(createTableSQL);

// 插入数据
std::string insertSQL = "INSERT INTO users VALUES (1, 'John Doe', 'john@example.com')";
db_manager->executeQuery(insertSQL);

// 查询数据
std::string selectSQL = "SELECT * FROM users WHERE id = 1";
auto result = db_manager->executeQuery(selectSQL);
```

### 索引操作

```cpp
#include "storage_engine/storage_engine.h"

// 获取存储引擎实例
auto storage_engine = db_manager->getStorageEngine();

// 创建索引
storage_engine->createIndex("users", "email");

// 索引查询会自动使用
std::string indexedQuery = "SELECT * FROM users WHERE email = 'john@example.com'";
auto result = db_manager->executeQuery(indexedQuery);
```

### 事务管理

```cpp
// 开始事务
db_manager->beginTransaction();

try {
    // 执行多个操作
    db_manager->executeQuery("INSERT INTO users VALUES (2, 'Jane Doe', 'jane@example.com')");
    db_manager->executeQuery("UPDATE users SET name = 'Jane Smith' WHERE id = 2");

    // 提交事务
    db_manager->commitTransaction();
} catch (const std::exception& e) {
    // 回滚事务
    db_manager->rollbackTransaction();
}
```

## 配置管理

### ConfigManager 类

```cpp
#include "utils/config_manager.h"

class ConfigManager {
public:
    // 配置加载
    bool loadConfig(const std::string& configFile);

    // 获取配置值
    template<typename T>
    T getValue(const std::string& key, const T& defaultValue = T());

    // 设置配置值
    template<typename T>
    void setValue(const std::string& key, const T& value);

    // 保存配置
    bool saveConfig(const std::string& configFile);
};
```

### 配置文件示例

```json
{
    "database": {
        "name": "sqlcc_db",
        "port": 8080,
        "max_connections": 100
    },
    "storage": {
        "buffer_pool_size": 1048576,
        "page_size": 8192,
        "data_directory": "/var/lib/sqlcc/data"
    },
    "logging": {
        "level": "INFO",
        "file": "/var/log/sqlcc/sqlcc.log"
    }
}
```

## 错误处理

SQLCC 使用异常机制进行错误处理：

```cpp
#include "exception/base_exception.h"
#include "exception/database_exception.h"

try {
    auto result = db_manager->executeQuery("SELECT * FROM nonexistent_table");
} catch (const TableNotFoundException& e) {
    std::cerr << "Table not found: " << e.what() << std::endl;
} catch (const DatabaseException& e) {
    std::cerr << "Database error: " << e.what() << std::endl;
} catch (const SQLException& e) {
    std::cerr << "SQL error: " << e.what() << std::endl;
} catch (const std::exception& e) {
    std::cerr << "General error: " << e.what() << std::endl;
}
```

## 性能优化

### 索引优化

```cpp
// 使用执行器的索引优化查询
auto result = db_manager->executeQuery(
    "SELECT * FROM orders WHERE customer_id = ? AND order_date > ?");

// 查看执行计划
auto& context = executor.getLastExecutionContext();
std::cout << "Records affected: " << context.getRecordsAffected() << std::endl;
```

### 连接池管理

```cpp
#include "network/connection_pool.h"

class ConnectionPool {
public:
    // 获取连接
    std::shared_ptr<Connection> getConnection();

    // 归还连接
    void returnConnection(std::shared_ptr<Connection> conn);

    // 配置连接池
    void setMaxConnections(size_t maxConn);
    void setMaxIdleTime(std::chrono::seconds idleTime);
};
```

## 扩展开发

### 自定义存储引擎

```cpp
class CustomStorageEngine : public StorageEngine {
public:
    bool createTable(const std::string& tableName, const TableSchema& schema) override {
        // 自定义表创建逻辑
        return true;
    }

    std::shared_ptr<Table> getTable(const std::string& tableName) override {
        // 自定义表获取逻辑
        return nullptr;
    }
};
```

### 自定义SQL函数

```cpp
#include "sql_parser/function/function_registry.h"

class CustomFunction : public ScalarFunction {
public:
    Value evaluate(const std::vector<Value>& args) override {
        // 自定义函数逻辑
        return Value();
    }
};

// 注册自定义函数
FunctionRegistry::instance().registerFunction("MY_FUNC", 
    std::make_shared<CustomFunction>());
```

### 自定义执行策略

```cpp
class CustomExecutionStrategy : public ExecutionStrategy {
public:
    ExecutionResult execute(std::unique_ptr<sql_parser::Statement> stmt,
                           ExecutionContext &context) override {
        // 自定义执行逻辑
        return ExecutionResult::Success();
    }
};
```

## 参考资料

- [项目主页](https://gitee.com/yinglichina/sqlcc)
- [开发指南](docs/development/guides/DEVELOPMENT_GUIDE.md)
- [构建和测试指南](docs/development/guides/BUILD_AND_TEST_GUIDE.md)
- [测试驱动开发指南](docs/development/guides/TEST_DRIVEN_DEVELOPMENT_GUIDE.md)
- [AI辅助开发指南](docs/ai_tools/AI_DEVELOPMENT_GUIDELINES.md)
- [API代码规范](docs/api/code/coding_standards.md)
- [源码注释指南](docs/api/code/source_code_comments_guide.md)

## 版本兼容性

- **v1.3.9**: 当前最新版本，Level 1 Foundation完整单元测试（~160个测试用例，100%通过率）
- **v1.3.8**: SQL Parser模块化重构
- **v1.3.7**: Bazel构建系统重构
- **v1.3.6**: LLVM覆盖率工具链完善
- **v1.2.6**: SQL-92标准完整支持

## 技术支持

如需技术支持，请联系开发团队或在项目Issues中提交问题。
