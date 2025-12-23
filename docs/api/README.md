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

### StorageEngine 类

```cpp
class StorageEngine {
public:
    // 缓冲池管理
    std::shared_ptr<BufferPool> getBufferPool();

    // 表存储操作
    bool createTable(const std::string& tableName, const TableSchema& schema);
    bool dropTable(const std::string& tableName);
    std::shared_ptr<Table> getTable(const std::string& tableName);

    // 索引操作
    bool createIndex(const std::string& tableName, const std::string& columnName);
    bool dropIndex(const std::string& tableName, const std::string& columnName);
};
```

### DatabaseManager 类

```cpp
class DatabaseManager {
public:
    // 数据库操作
    bool createDatabase(const std::string& dbName);
    bool dropDatabase(const std::string& dbName);
    bool useDatabase(const std::string& dbName);

    // 查询执行
    QueryResult executeQuery(const std::string& sql);

    // 事务管理
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();
};
```

### SQLExecutor 类

```cpp
class SQLExecutor {
public:
    // SQL执行
    ExecutionResult execute(const std::string& sql);

    // 解析和优化
    std::shared_ptr<QueryPlan> parseAndOptimize(const std::string& sql);

    // 执行计划
    QueryResult executePlan(const std::shared_ptr<QueryPlan>& plan);
};
```

## 使用示例

### 基本查询执行

```cpp
#include "core/database_manager.h"
#include "core/sql_executor.h"

// 初始化数据库管理器
auto dbManager = std::make_shared<DatabaseManager>();
dbManager->createDatabase("test_db");
dbManager->useDatabase("test_db");

// 创建表
std::string createTableSQL = R"(
    CREATE TABLE users (
        id INT PRIMARY KEY,
        name VARCHAR(100),
        email VARCHAR(100)
    )
)";
dbManager->executeQuery(createTableSQL);

// 插入数据
std::string insertSQL = "INSERT INTO users VALUES (1, 'John Doe', 'john@example.com')";
dbManager->executeQuery(insertSQL);

// 查询数据
std::string selectSQL = "SELECT * FROM users WHERE id = 1";
auto result = dbManager->executeQuery(selectSQL);
```

### 索引操作

```cpp
#include "storage_engine/storage_engine.h"

// 获取存储引擎实例
auto storageEngine = dbManager->getStorageEngine();

// 创建索引
storageEngine->createIndex("users", "email");

// 索引查询会自动使用
std::string indexedQuery = "SELECT * FROM users WHERE email = 'john@example.com'";
auto result = dbManager->executeQuery(indexedQuery);
```

### 事务管理

```cpp
// 开始事务
dbManager->beginTransaction();

try {
    // 执行多个操作
    dbManager->executeQuery("INSERT INTO users VALUES (2, 'Jane Doe', 'jane@example.com')");
    dbManager->executeQuery("UPDATE users SET name = 'Jane Smith' WHERE id = 2");

    // 提交事务
    dbManager->commitTransaction();
} catch (const std::exception& e) {
    // 回滚事务
    dbManager->rollbackTransaction();
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

try {
    auto result = dbManager->executeQuery("SELECT * FROM nonexistent_table");
} catch (const TableNotFoundException& e) {
    std::cerr << "Table not found: " << e.what() << std::endl;
} catch (const SQLException& e) {
    std::cerr << "SQL error: " << e.what() << std::endl;
} catch (const std::exception& e) {
    std::cerr << "General error: " << e.what() << std::endl;
}
```

## 性能优化

### 索引优化

```cpp
// 创建复合索引
storageEngine->createCompositeIndex("orders", {"customer_id", "order_date"});

// 分析查询性能
auto queryPlan = sqlExecutor->parseAndOptimize("SELECT * FROM orders WHERE customer_id = ? AND order_date > ?");
std::cout << "Query cost: " << queryPlan->getEstimatedCost() << std::endl;
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
#include "sql_parser/function_registry.h"

class CustomFunction : public ScalarFunction {
public:
    Value evaluate(const std::vector<Value>& args) override {
        // 自定义函数逻辑
        return Value();
    }
};

// 注册自定义函数
FunctionRegistry::instance().registerFunction("MY_FUNC", std::make_shared<CustomFunction>());
```

## 参考资料

- [项目主页](https://gitee.com/yinglichina/sqlcc)
- [开发指南](docs/guides/DEVELOPMENT_ENVIRONMENT_SETUP.md)
- [设计文档](docs/design/Architecture.md)
- [测试文档](docs/guides/TEST_DRIVEN_DEVELOPMENT_GUIDE.md)

## 版本兼容性

- **v1.2.6**: 当前最新版本，支持完整的SQL-92标准
- **v1.2.5**: 系统性测试重构，现代化编译技术栈
- **v1.2.4**: 企业级特性评估与系统优化增强
- **v1.2.3**: SQL-92完整性大幅提升

## 技术支持

如需技术支持，请联系开发团队或在项目Issues中提交问题。
