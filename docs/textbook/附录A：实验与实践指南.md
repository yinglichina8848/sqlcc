# 附录A：实验与实践指南

**从理论学习到工程实践的完整路径**

---

## A.1 SQLCC环境搭建

### A.1.1 系统要求

**硬件要求**：
- CPU：双核及以上（推荐4核）
- 内存：4GB及以上（推荐8GB）
- 磁盘：20GB可用空间

**软件要求**：
- 操作系统：Linux（Ubuntu 20.04+、CentOS 8+）、macOS、Windows（WSL2）
- 编译器：GCC 11+、Clang 13+
- 构建工具：Bazel 6.0+
- Python：3.8+（用于测试脚本）

### A.1.2 依赖安装

#### Ubuntu/Debian

```bash
# 更新包管理器
sudo apt update

# 安装基础依赖
sudo apt install -y \
    git \
    build-essential \
    cmake \
    ninja-build \
    python3 \
    python3-pip \
    pkg-config

# 安装Bazel
wget https://github.com/bazelbuild/bazel/releases/download/6.0.0/bazel-6.0.0-linux-x86_64
chmod +x bazel-6.0.0-linux-x86_64
sudo mv bazel-6.0.0-linux-x86_64 /usr/local/bin/bazel

# 验证安装
bazel --version
```

#### macOS

```bash
# 安装Homebrew（如果未安装）
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# 安装依赖
brew install \
    git \
    bazel \
    cmake \
    ninja \
    python3

# 验证安装
bazel --version
```

#### Windows（WSL2）

```bash
# 在WSL2中执行
sudo apt update
sudo apt install -y \
    git \
    build-essential \
    cmake \
    ninja-build \
    python3 \
    python3-pip \
    pkg-config

# 下载并安装Bazel
wget https://github.com/bazelbuild/bazel/releases/download/6.0.0/bazel-6.0.0-linux-x86_64
chmod +x bazel-6.0.0-linux-x86_64
sudo mv bazel-6.0.0-linux-x86_64 /usr/local/bin/bazel
```

### A.1.3 克隆SQLCC项目

```bash
# 克隆项目
git clone git@gitee.com:yinglichina/sqlcc.git
cd sqlcc

# 查看版本
cat VERSION
```

### A.1.4 编译SQLCC

```bash
# 构建项目
bazel build //...

# 运行测试
bazel test //...

# 构建可执行文件
bazel build //src:sqlcc

# 可执行文件位置
./bazel-bin/src/sqlcc
```

### A.1.5 配置SQLCC

```bash
# 创建配置文件
cat > config/sqlcc.conf << EOF
[database]
data_dir = /path/to/data
log_dir = /path/to/logs
max_connections = 100

[storage]
buffer_pool_size = 256MB
max_page_size = 16KB
enable_compression = true

[execution]
max_concurrent_queries = 10
query_timeout = 30s
enable_query_cache = true

[logging]
level = INFO
file = /path/to/logs/sqlcc.log
max_size = 100MB
EOF
```

### A.1.6 启动SQLCC

```bash
# 启动SQLCC服务器
./bazel-bin/src/sqlcc --config config/sqlcc.conf

# 验证服务
psql -h localhost -p 5432 -U sqlcc -d testdb
```

---

## A.2 代码阅读指南

### A.2.1 项目结构导航

```
sqlcc/
├── src/                    # 源代码目录
│   ├── sql_parser/         # SQL解析器模块
│   ├── execution/          # 执行引擎模块
│   ├── storage_engine/     # 存储引擎模块
│   ├── core/               # 核心管理模块
│   ├── procedure/          # 存储过程模块
│   ├── trigger/            # 触发器模块
│   ├── transaction_manager/# 事务管理模块
│   └── network/            # 网络通信模块
├── include/                # 公共头文件
├── tests/                  # 测试代码
├── examples/               # 示例代码
└── docs/                   # 文档
```

### A.2.2 推荐阅读顺序

**初学者路径**：

1. **从main.cpp开始**（`src/main.cpp`）
   - 理解程序入口点
   - 了解初始化流程
   - 掌握命令行参数处理

2. **核心管理模块**（`src/core/`）
   - `database_manager.h`：数据库管理
   - `user_manager.h`：用户权限管理
   - `unified_executor.h`：统一执行器

3. **SQL解析器**（`src/sql_parser/`）
   - `lexer.h`：词法分析器
   - `parser.h`：语法分析器
   - `ast/`：AST节点定义

4. **执行引擎**（`src/execution/`）
   - `execution_strategy.h`：执行策略基类
   - `dml_execution_strategy.h`：DML执行策略
   - `ddl_execution_strategy.h`：DDL执行策略

5. **存储引擎**（`src/storage_engine/`）
   - `storage_engine.h`：存储引擎接口
   - `b_plus_tree/`：B+树索引
   - `buffer_pool_sharded.h`：分片缓冲池

**进阶路径**：

1. **事务管理**（`src/transaction_manager/`）
   - `transaction_manager.cpp`：事务管理器
   - `wal_writer.cpp`：WAL日志
   - `concurrency_control.h`：并发控制

2. **高级特性**（`src/procedure/`, `src/trigger/`）
   - `procedure_vm.cpp`：存储过程虚拟机
   - `trigger_manager.h`：触发器管理器

3. **网络通信**（`src/network/`）
   - `server_network_manager.cpp`：服务器网络管理
   - `encryption.cpp`：加密模块

### A.2.3 代码阅读技巧

**1. 使用IDE的跳转功能**
- VSCode + C/C++插件
- CLion
- Vim + ctags/cscope

**2. 理解UML图**
- 查看章节中的类图和序列图
- 理解类之间的关系
- 掌握调用流程

**3. 调试代码**
```bash
# 使用GDB调试
gdb ./bazel-bin/src/sqlcc

# GDB常用命令
(gdb) break main
(gdb) run
(gdb) next
(gdb) print variable_name
(gdb) continue
```

**4. 查看日志**
```bash
# 查看SQLCC日志
tail -f /path/to/logs/sqlcc.log

# 搜索特定错误
grep "ERROR" /path/to/logs/sqlcc.log
```

---

## A.3 调试技巧

### A.3.1 编译时调试

```bash
# 启用调试符号
bazel build -c dbg //...

# 编译特定目标
bazel build //src/sql_parser:parser

# 查看编译错误
bazel build //... 2>&1 | grep error
```

### A.3.2 运行时调试

**1. 使用GDB**

```bash
# 启动GDB
gdb --args ./bazel-bin/src/sqlcc --config config/sqlcc.conf

# 设置断点
(gdb) break sql_parser::Parser::parse

# 运行程序
(gdb) run

# 单步执行
(gdb) step
(gdb) next

# 查看变量
(gdb) print token
(gdb) print stmt->getType()

# 查看调用栈
(gdb) backtrace

# 查看内存
(gdb) x/10x pointer

# 退出
(gdb) quit
```

**2. 使用Valgrind检查内存泄漏**

```bash
# 安装Valgrind
sudo apt install valgrind

# 运行内存检查
valgrind --leak-check=full ./bazel-bin/src/sqlcc --config config/sqlcc.conf

# 查看内存泄漏报告
# 等程序退出后，Valgrind会输出内存泄漏信息
```

**3. 使用AddressSanitizer**

```bash
# 启用AddressSanitizer编译
bazel build --config=asan //...

# 运行程序
./bazel-bin/src/sqlcc --config config/sqlcc.conf

# AddressSanitizer会检测内存错误并输出详细报告
```

### A.3.3 日志调试

```cpp
// 在代码中添加日志
#include "core/logger.h"

void executeQuery(const std::string& sql) {
    LOG_INFO("Executing query: " << sql);
    
    try {
        // 执行查询
        auto result = execute(sql);
        LOG_DEBUG("Query executed successfully");
    } catch (const std::exception& e) {
        LOG_ERROR("Query execution failed: " << e.what());
    }
}
```

```bash
# 查看日志
tail -f /path/to/logs/sqlcc.log | grep "Executing query"
```

---

## A.4 实验案例设计

### A.4.1 实验1：SQL解析器基础

**目标**：理解SQL解析器的工作原理

**步骤**：

1. **创建测试SQL文件**
```sql
-- test.sql
SELECT name, age 
FROM users 
WHERE age > 18 
ORDER BY age DESC 
LIMIT 10;
```

2. **编写测试程序**
```cpp
// test_parser.cpp
#include "sql_parser/parser.h"
#include <iostream>

int main() {
    std::string sql = "SELECT name FROM users WHERE age > 18";
    
    sqlcc::sql_parser::Parser parser(sql);
    auto statements = parser.parse();
    
    if (parser.hadError()) {
        std::cerr << "Parse errors:" << std::endl;
        for (const auto& error : parser.getDetailedErrors()) {
            std::cerr << "  " << error << std::endl;
        }
        return 1;
    }
    
    std::cout << "Parsed " << statements.size() << " statement(s)" << std::endl;
    
    return 0;
}
```

3. **编译和运行**
```bash
bazel build //test_parser
./bazel-bin/test_parser
```

4. **分析输出**
- 查看解析后的AST结构
- 理解Token的生成过程
- 观察错误处理机制

**思考题**：
1. 词法分析和语法分析的区别是什么？
2. 如何扩展SQL解析器支持新的语法？
3. 错误恢复机制如何工作？

### A.4.2 实验2：B+树索引实现

**目标**：理解B+树索引的工作原理

**步骤**：

1. **创建测试数据**
```sql
CREATE TABLE users (
    id INT PRIMARY KEY,
    name VARCHAR(100),
    age INT,
    email VARCHAR(100)
);

-- 插入测试数据
INSERT INTO users (id, name, age, email)
SELECT 
    generate_series(1, 10000),
    'User ' || generate_series(1, 10000),
    random() * 100,
    'user' || generate_series(1, 10000) || '@example.com';
```

2. **创建索引**
```sql
-- 创建B+树索引
CREATE INDEX idx_users_age ON users(age);

-- 创建复合索引
CREATE INDEX idx_users_name_age ON users(name, age);
```

3. **测试索引性能**
```cpp
// test_index.cpp
#include "storage_engine/b_plus_tree.h"
#include <chrono>

void testIndexPerformance() {
    auto storage_engine = createStorageEngine();
    auto b_plus_tree = createBPlusTree(storage_engine, "users", "age");
    
    // 测试点查询
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 1000; ++i) {
        auto result = b_plus_tree->Lookup(std::to_string(i));
    }
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Point query time: " << duration.count() << " microseconds" << std::endl;
    
    // 测试范围查询
    start = std::chrono::high_resolution_clock::now();
    auto results = b_plus_tree->RangeLookup("18", "30");
    end = std::chrono::high_resolution_clock::now();
    
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Range query time: " << duration.count() << " microseconds" << std::endl;
    std::cout << "Results count: " << results.size() << std::endl;
}
```

4. **分析索引性能**
- 对比有索引和无索引的查询性能
- 测试不同数据量下的性能表现
- 观察B+树的分裂和合并过程

**思考题**：
1. B+树相比B树有什么优势？
2. 什么时候使用B+树索引，什么时候使用哈希索引？
3. 如何优化B+树的性能？

### A.4.3 实验3：事务处理与并发控制

**目标**：理解ACID特性和并发控制机制

**步骤**：

1. **创建测试表**
```sql
CREATE TABLE accounts (
    id INT PRIMARY KEY,
    balance DECIMAL(10,2)
);

INSERT INTO accounts (id, balance) VALUES
(1, 1000.00),
(2, 1000.00);
```

2. **测试事务的ACID特性**
```cpp
// test_transaction.cpp
#include "transaction_manager/transaction_manager.h"
#include <thread>
#include <vector>

void testAtomicity() {
    auto tx_manager = createTransactionManager();
    
    // 开启事务
    auto tx = tx_manager->beginTransaction();
    
    try {
        // 执行转账操作
        tx->execute("UPDATE accounts SET balance = balance - 100 WHERE id = 1");
        tx->execute("UPDATE accounts SET balance = balance + 100 WHERE id = 2");
        
        // 提交事务
        tx->commit();
        std::cout << "Transaction committed successfully" << std::endl;
    } catch (const std::exception& e) {
        // 回滚事务
        tx->rollback();
        std::cout << "Transaction rolled back: " << e.what() << std::endl;
    }
}

void testConcurrency() {
    auto tx_manager = createTransactionManager();
    std::vector<std::thread> threads;
    
    // 创建多个并发事务
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&tx_manager, i]() {
            auto tx = tx_manager->beginTransaction();
            
            try {
                tx->execute("UPDATE accounts SET balance = balance + 10 WHERE id = 1");
                tx->execute("UPDATE accounts SET balance = balance - 10 WHERE id = 2");
                tx->commit();
            } catch (const std::exception& e) {
                tx->rollback();
                std::cerr << "Transaction " << i << " failed: " << e.what() << std::endl;
            }
        });
    }
    
    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }
    
    // 验证数据一致性
    auto result = tx_manager->execute("SELECT SUM(balance) FROM accounts");
    std::cout << "Total balance: " << result.getSingleValue() << std::endl;
}
```

3. **测试隔离级别**
```sql
-- 测试READ COMMITTED隔离级别
SET TRANSACTION ISOLATION LEVEL READ COMMITTED;
BEGIN;
SELECT balance FROM accounts WHERE id = 1;
-- 在另一个会话中更新balance
SELECT balance FROM accounts WHERE id = 1;
COMMIT;

-- 测试REPEATABLE READ隔离级别
SET TRANSACTION ISOLATION LEVEL REPEATABLE READ;
BEGIN;
SELECT balance FROM accounts WHERE id = 1;
-- 在另一个会话中更新balance
SELECT balance FROM accounts WHERE id = 1;
COMMIT;
```

4. **分析并发控制**
- 观察锁的获取和释放
- 测试死锁检测和恢复
- 理解MVCC的工作原理

**思考题**：
1. 为什么需要事务隔离级别？
2. MVCC如何解决读写冲突？
3. 如何检测和解决死锁？

### A.4.4 实验4：查询优化与执行计划

**目标**：理解查询优化器的工作原理

**步骤**：

1. **创建测试表和数据**
```sql
CREATE TABLE orders (
    id INT PRIMARY KEY,
    customer_id INT,
    product_id INT,
    quantity INT,
    total_amount DECIMAL(10,2),
    order_date DATE
);

CREATE TABLE customers (
    id INT PRIMARY KEY,
    name VARCHAR(100),
    email VARCHAR(100)
);

CREATE TABLE products (
    id INT PRIMARY KEY,
    name VARCHAR(100),
    price DECIMAL(10,2),
    category VARCHAR(50)
);

-- 插入测试数据
INSERT INTO customers (id, name, email)
SELECT 
    generate_series(1, 1000),
    'Customer ' || generate_series(1, 1000),
    'customer' || generate_series(1, 1000) || '@example.com';

INSERT INTO products (id, name, price, category)
SELECT 
    generate_series(1, 100),
    'Product ' || generate_series(1, 100),
    random() * 1000,
    CASE random() * 10 
        WHEN 0 THEN 'Electronics'
        WHEN 1 THEN 'Books'
        WHEN 2 THEN 'Clothing'
        ELSE 'Other'
    END;

INSERT INTO orders (id, customer_id, product_id, quantity, total_amount, order_date)
SELECT 
    generate_series(1, 10000),
    (random() * 999)::int + 1,
    (random() * 99)::int + 1,
    (random() * 10)::int + 1,
    0,
    CURRENT_DATE - (random() * 365)::int;
```

2. **创建索引**
```sql
CREATE INDEX idx_orders_customer ON orders(customer_id);
CREATE INDEX idx_orders_product ON orders(product_id);
CREATE INDEX idx_orders_date ON orders(order_date);
CREATE INDEX idx_products_category ON products(category);
```

3. **测试查询优化**
```cpp
// test_query_optimizer.cpp
#include "execution/cost_estimator.h"
#include <iostream>

void testQueryOptimization() {
    auto cost_estimator = createCostEstimator();
    
    // 测试查询1：简单查询
    std::string query1 = "SELECT * FROM orders WHERE customer_id = 100";
    auto plan1 = cost_estimator->optimizeQuery(query1);
    std::cout << "Query 1 cost: " << plan1.getCost() << std::endl;
    std::cout << "Query 1 plan: " << plan1.toString() << std::endl;
    
    // 测试查询2：连接查询
    std::string query2 = "SELECT c.name, p.name, o.quantity "
                        "FROM orders o "
                        "JOIN customers c ON o.customer_id = c.id "
                        "JOIN products p ON o.product_id = p.id "
                        "WHERE o.total_amount > 100";
    auto plan2 = cost_estimator->optimizeQuery(query2);
    std::cout << "Query 2 cost: " << plan2.getCost() << std::endl;
    std::cout << "Query 2 plan: " << plan2.toString() << std::endl;
    
    // 测试查询3：聚合查询
    std::string query3 = "SELECT p.category, COUNT(*), SUM(o.total_amount) "
                        "FROM orders o "
                        "JOIN products p ON o.product_id = p.id "
                        "GROUP BY p.category";
    auto plan3 = cost_estimator->optimizeQuery(query3);
    std::cout << "Query 3 cost: " << plan3.getCost() << std::endl;
    std::cout << "Query 3 plan: " << plan3.toString() << std::endl;
}
```

4. **分析执行计划**
- 查看查询优化器选择的执行计划
- 对比不同查询的执行代价
- 理解索引选择策略

**思考题**：
1. 查询优化器如何选择最优执行计划？
2. 什么情况下索引不会被使用？
3. 如何优化慢查询？

### A.4.5 实验5：分布式数据库部署

**目标**：理解分布式数据库的部署和运维

**步骤**：

1. **准备多台机器**
- 3台虚拟机或物理机
- 每台机器配置相同的环境
- 确保网络连通性

2. **配置分布式集群**
```yaml
# sqlcc-cluster.yaml
cluster:
  name: "sqlcc-distributed"
  
nodes:
  - id: "node1"
    host: "10.0.1.10"
    port: 5432
    shards: ["shard1", "shard2", "shard3"]
    role: ["data", "coordinator"]
    
  - id: "node2"
    host: "10.0.1.11"
    port: 5432
    shards: ["shard4", "shard5", "shard6"]
    role: ["data"]
    
  - id: "node3"
    host: "10.0.1.12"
    port: 5432
    shards: ["shard7", "shard8", "shard9"]
    role: ["data"]

sharding:
  strategy: "hash"
  numShards: 9
  replicationFactor: 3

monitoring:
  enabled: true
  metricsPort: 9090
```

3. **部署集群**
```bash
# 在每台机器上部署SQLCC
for host in 10.0.1.10 10.0.1.11 10.0.1.12; do
    ssh $host "cd /opt/sqlcc && ./scripts/deploy.sh"
done

# 初始化集群
./scripts/init-cluster.sh --config sqlcc-cluster.yaml
```

4. **测试集群**
```sql
-- 创建分布式表
CREATE TABLE distributed_users (
    id INT PRIMARY KEY,
    name VARCHAR(100),
    email VARCHAR(100)
) SHARD BY HASH(id);

-- 插入数据
INSERT INTO distributed_users (id, name, email)
SELECT generate_series(1, 100000),
       'User ' || generate_series(1, 100000),
       'user' || generate_series(1, 100000) || '@example.com';

-- 测试分布式查询
SELECT COUNT(*) FROM distributed_users;
SELECT * FROM distributed_users WHERE id = 50000;
```

5. **监控集群**
```bash
# 查看集群状态
./scripts/cluster-status.sh

# 查看分片分布
./scripts/shard-status.sh

# 监控性能指标
curl http://10.0.1.10:9090/metrics
```

6. **测试故障恢复**
```bash
# 模拟节点故障
ssh 10.0.1.10 "systemctl stop sqlcc"

# 观察集群自动故障转移
./scripts/cluster-status.sh

# 恢复节点
ssh 10.0.1.10 "systemctl start sqlcc"

# 观察集群自动恢复
./scripts/cluster-status.sh
```

**思考题**：
1. 如何选择合适的分片策略？
2. 分布式事务如何保证ACID特性？
3. 如何处理网络分区？

---

## A.5 扩展开发指南

### A.5.1 添加新的SQL特性

**目标**：为SQLCC添加新的SQL语法支持

**步骤**：

1. **定义AST节点**
```cpp
// src/sql_parser/ast/new_statement.h
class NewStatement : public Statement {
public:
    NewStatement();
    ~NewStatement() override;
    
    void accept(NodeVisitor& visitor) override;
    
    // 新语句特有的属性
    std::string getNewProperty() const { return newProperty_; }
    void setNewProperty(const std::string& value) { newProperty_ = value; }
    
private:
    std::string newProperty_;
};
```

2. **扩展词法分析器**
```cpp
// src/sql_parser/lexer.h
enum class TokenType {
    // 现有类型...
    NEW_KEYWORD  // 新增关键字
};

// src/sql_parser/lexer.cpp
Token Lexer::createKeywordToken(const std::string& lexeme) {
    // 现有关键字...
    if (lexeme == "NEW") {
        return createToken(TokenType::NEW_KEYWORD, lexeme, line_, column_);
    }
}
```

3. **扩展语法分析器**
```cpp
// src/sql_parser/parser.h
std::unique_ptr<Statement> parseNewStatement() {
    auto stmt = std::make_unique<NewStatement>();
    
    // 解析新语句的语法
    consumeToken(TokenType::NEW_KEYWORD);
    consumeToken(TokenType::IDENTIFIER);
    
    stmt->setNewProperty(currentToken().lexeme);
    
    return stmt;
}
```

4. **扩展执行策略**
```cpp
// src/execution/new_execution_strategy.h
class NewExecutionStrategy : public ExecutionStrategy {
public:
    ExecutionResult execute(
        std::unique_ptr<sql_parser::Statement> stmt,
        ExecutionContext& context) override {
        
        auto newStmt = dynamic_cast<NewStatement*>(stmt.get());
        
        // 执行新语句的逻辑
        return executeNewStatement(newStmt, context);
    }
    
private:
    ExecutionResult executeNewStatement(
        NewStatement* stmt,
        ExecutionContext& context);
};
```

5. **注册执行策略**
```cpp
// src/core/unified_executor.cpp
void UnifiedExecutor::registerStrategies() {
    strategies_[Statement::NEW] = 
        std::make_unique<NewExecutionStrategy>();
}
```

6. **编写测试**
```cpp
// tests/test_new_statement.cpp
TEST(NewStatementTest, BasicExecution) {
    std::string sql = "NEW test_property";
    
    Parser parser(sql);
    auto statements = parser.parse();
    
    ASSERT_EQ(1, statements.size());
    
    ExecutionContext context;
    ExecutionStrategy strategy;
    auto result = strategy.execute(std::move(statements[0]), context);
    
    ASSERT_TRUE(result.isSuccess());
}
```

### A.5.2 添加新的索引类型

**目标**：为SQLCC添加新的索引算法

**步骤**：

1. **定义索引接口**
```cpp
// src/storage_engine/index/new_index.h
class NewIndex : public Index {
public:
    NewIndex(const std::string& table_name,
             const std::string& column_name);
    
    ~NewIndex() override;
    
    // 索引操作
    bool Insert(const std::string& key, int32_t page_id, size_t offset) override;
    bool Delete(const std::string& key) override;
    bool Lookup(const std::string& key, int32_t& page_id, size_t& offset) const override;
    std::vector<std::pair<int32_t, size_t>> RangeLookup(
        const std::string& start_key,
        const std::string& end_key) const override;
    
    // 索引维护
    bool Rebuild() override;
    size_t GetMemoryUsage() const override;
    
private:
    // 新索引的数据结构
    std::unordered_map<std::string, std::pair<int32_t, size_t>> index_;
};
```

2. **实现索引操作**
```cpp
// src/storage_engine/index/new_index.cpp
bool NewIndex::Insert(const std::string& key, int32_t page_id, size_t offset) {
    // 检查键是否已存在
    if (index_.find(key) != index_.end()) {
        return false;  // 键已存在
    }
    
    // 插入键值对
    index_[key] = {page_id, offset};
    
    return true;
}

bool NewIndex::Lookup(const std::string& key, 
                     int32_t& page_id, size_t& offset) const {
    auto it = index_.find(key);
    
    if (it == index_.end()) {
        return false;  // 键不存在
    }
    
    page_id = it->second.first;
    offset = it->second.second;
    
    return true;
}
```

3. **注册索引类型**
```cpp
// src/storage_engine/index_manager.h
enum class IndexType {
    B_PLUS_TREE,
    HASH,
    NEW_INDEX  // 新增索引类型
};

class IndexManager {
public:
    std::unique_ptr<Index> createIndex(
        IndexType type,
        const std::string& table_name,
        const std::string& column_name) {
        
        switch (type) {
            case IndexType::B_PLUS_TREE:
                return std::make_unique<BPlusTreeIndex>(table_name, column_name);
            case IndexType::HASH:
                return std::make_unique<HashIndex>(table_name, column_name);
            case IndexType::NEW_INDEX:
                return std::make_unique<NewIndex>(table_name, column_name);
        }
    }
};
```

4. **编写测试**
```cpp
// tests/test_new_index.cpp
TEST(NewIndexTest, BasicOperations) {
    NewIndex index("test_table", "test_column");
    
    // 测试插入
    ASSERT_TRUE(index.Insert("key1", 1, 0));
    ASSERT_TRUE(index.Insert("key2", 2, 0));
    
    // 测试查找
    int32_t page_id;
    size_t offset;
    ASSERT_TRUE(index.Lookup("key1", page_id, offset));
    ASSERT_EQ(1, page_id);
    
    // 测试删除
    ASSERT_TRUE(index.Delete("key1"));
    ASSERT_FALSE(index.Lookup("key1", page_id, offset));
}
```

---

## A.6 常见问题与解决方案

### A.6.1 编译问题

**问题1：Bazel编译失败**

```bash
# 错误信息
ERROR: /home/user/sqlcc/BUILD.bazel:10:1: no such package '@org_sqlparser//'

# 解决方案
# 更新WORKSPACE文件中的依赖
bazel sync --configure
```

**问题2：链接错误**

```bash
# 错误信息
undefined reference to `sqlcc::sql_parser::Parser::parse()`

# 解决方案
# 检查BUILD.bazel文件中的依赖关系
bazel build //src/sql_parser:parser --verbose
```

### A.6.2 运行时问题

**问题1：数据库启动失败**

```bash
# 错误信息
FATAL: could not create lock file "/var/run/sqlcc.pid"

# 解决方案
# 检查权限或删除已存在的锁文件
sudo rm /var/run/sqlcc.pid
./bazel-bin/src/sqlcc
```

**问题2：查询超时**

```bash
# 错误信息
ERROR: query timeout after 30 seconds

# 解决方案
# 增加查询超时时间
# 在配置文件中设置
[execution]
query_timeout = 60s
```

### A.6.3 性能问题

**问题1：查询性能差**

```sql
-- 分析慢查询
EXPLAIN ANALYZE SELECT * FROM orders WHERE customer_id = 100;

-- 解决方案
-- 创建适当的索引
CREATE INDEX idx_orders_customer ON orders(customer_id);
```

**问题2：内存占用过高**

```bash
# 查看内存使用
ps aux | grep sqlcc

# 解决方案
-- 调整缓冲池大小
-- 在配置文件中设置
[storage]
buffer_pool_size = 128MB
```

---

## A.7 学习资源推荐

### A.7.1 书籍

1. **《数据库系统概念》** - Abraham Silberschatz
2. **《高性能MySQL》** - Baron Schwartz
3. **《数据库事务处理的艺术》** - Jim Gray
4. **《设计数据密集型应用》** - Martin Kleppmann

### A.7.2 在线资源

1. **SQLCC项目文档**：`docs/`目录
2. **PostgreSQL文档**：https://www.postgresql.org/docs/
3. **MySQL文档**：https://dev.mysql.com/doc/
4. **数据库学术论文**：https://dblp.org/

### A.7.3 工具和框架

1. **数据库测试工具**：sysbench, tpcc-mysql
2. **性能分析工具**：perf, valgrind, gprof
3. **监控工具**：Prometheus, Grafana
4. **调试工具**：gdb, lldb

---

通过本附录的实践指南，读者可以从理论学习平滑过渡到工程实践，深入理解SQLCC的实现细节，并掌握数据库系统的核心技术和最佳实践。建议读者按照附录的步骤，逐步完成每个实验，并思考其中的问题和挑战，从而建立扎实的数据库系统知识体系。

---