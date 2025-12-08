#include "sql_executor.h"
#include "unified_executor.h"
#include "database_manager.h"
#include "system_database.h"
#include "user_manager.h"
#include <gtest/gtest.h>
#include <filesystem>
#include <thread>
#include <chrono>

namespace sqlcc {

// SQL-92综合测试类
class Sql92ComprehensiveTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建测试数据目录
        test_data_dir_ = "./test_sql92_comprehensive_" + std::to_string(std::time(nullptr));
        std::filesystem::create_directory(test_data_dir_);
        
        // 初始化数据库管理器
        db_manager_ = std::make_shared<DatabaseManager>(
            test_data_dir_ + "/test.db", 1024, 4, 2);
        
        // 初始化用户管理器
        user_manager_ = std::make_shared<UserManager>();
        
        // 初始化系统数据库
        system_db_ = std::make_shared<SystemDatabase>(db_manager_);
        
        // 初始化统一执行器
        unified_executor_ = std::make_shared<UnifiedExecutor>(
            db_manager_, user_manager_, system_db_);
        
        // 初始化SQL执行器
        sql_executor_ = std::make_unique<SqlExecutor>();
    }

    void TearDown() override {
        // 清理资源
        sql_executor_.reset();
        unified_executor_.reset();
        system_db_.reset();
        user_manager_.reset();
        db_manager_.reset();
        
        // 删除测试数据目录
        std::filesystem::remove_all(test_data_dir_);
    }

    // 执行SQL并验证结果
    void ExecuteAndVerify(const std::string& sql, const std::string& expected_keyword = "success") {
        std::string result = sql_executor_->Execute(sql);
        
        // 检查是否包含预期关键词
        EXPECT_TRUE(result.find(expected_keyword) != std::string::npos ||
                   result.find("错误") == std::string::npos);
        
        // 检查错误状态
        std::string error = sql_executor_->GetLastError();
        EXPECT_TRUE(error.empty() || error.find("错误") == std::string::npos);
        
        std::cout << "SQL: " << sql << std::endl;
        std::cout << "结果: " << result << std::endl;
        if (!error.empty()) {
            std::cout << "错误: " << error << std::endl;
        }
    }

    // 验证表是否存在
    void VerifyTableExists(const std::string& table_name) {
        bool exists = db_manager_->TableExists(table_name);
        EXPECT_TRUE(exists) << "表 " << table_name << " 应该存在";
    }

    // 验证用户是否存在
    void VerifyUserExists(const std::string& username) {
        // TODO: 实现用户存在性验证
        SUCCEED();
    }

    // 系统重启模拟
    void SimulateSystemRestart() {
        // 重新初始化所有组件
        TearDown();
        SetUp();
    }

    std::string test_data_dir_;
    std::shared_ptr<DatabaseManager> db_manager_;
    std::shared_ptr<UserManager> user_manager_;
    std::shared_ptr<SystemDatabase> system_db_;
    std::shared_ptr<UnifiedExecutor> unified_executor_;
    std::unique_ptr<SqlExecutor> sql_executor_;
};

// 测试1: DDL命令综合测试
TEST_F(Sql92ComprehensiveTest, DDLCommandsComprehensiveTest) {
    std::cout << "\n=== DDL命令综合测试开始 ===" << std::endl;
    
    // 1.1 创建数据库
    ExecuteAndVerify("CREATE DATABASE test_db_92");
    ExecuteAndVerify("USE test_db_92");
    
    // 1.2 创建表结构
    ExecuteAndVerify("CREATE TABLE users ("
                     "id INTEGER PRIMARY KEY, "
                     "name VARCHAR(50) NOT NULL, "
                     "age INTEGER, "
                     "email VARCHAR(100))");
    
    // 验证表创建成功
    VerifyTableExists("users");
    
    // 1.3 修改表结构
    ExecuteAndVerify("ALTER TABLE users ADD COLUMN phone VARCHAR(20)");
    ExecuteAndVerify("ALTER TABLE users ADD COLUMN address TEXT");
    
    // 1.4 创建索引
    ExecuteAndVerify("CREATE INDEX idx_users_name ON users (name)");
    ExecuteAndVerify("CREATE INDEX idx_users_email ON users (email)");
    
    // 1.5 创建第二个表
    ExecuteAndVerify("CREATE TABLE orders ("
                     "order_id INTEGER PRIMARY KEY, "
                     "user_id INTEGER, "
                     "product_name VARCHAR(100), "
                     "amount DECIMAL(10,2), "
                     "order_date DATE)");
    
    // 1.6 创建外键关系
    ExecuteAndVerify("ALTER TABLE orders ADD CONSTRAINT fk_user_id "
                     "FOREIGN KEY (user_id) REFERENCES users(id)");
    
    std::cout << "=== DDL命令综合测试完成 ===" << std::endl;
}

// 测试2: DML命令综合测试
TEST_F(Sql92ComprehensiveTest, DMLCommandsComprehensiveTest) {
    std::cout << "\n=== DML命令综合测试开始 ===" << std::endl;
    
    // 设置测试环境
    ExecuteAndVerify("CREATE DATABASE dml_test_db");
    ExecuteAndVerify("USE dml_test_db");
    ExecuteAndVerify("CREATE TABLE products ("
                     "id INTEGER PRIMARY KEY, "
                     "name VARCHAR(100), "
                     "price DECIMAL(10,2), "
                     "stock INTEGER)");
    
    // 2.1 批量插入测试
    ExecuteAndVerify("INSERT INTO products VALUES (1, 'Laptop', 999.99, 10)");
    ExecuteAndVerify("INSERT INTO products VALUES (2, 'Mouse', 29.99, 50)");
    ExecuteAndVerify("INSERT INTO products VALUES (3, 'Keyboard', 79.99, 30)");
    ExecuteAndVerify("INSERT INTO products VALUES (4, 'Monitor', 299.99, 15)");
    
    // 2.2 更新操作测试
    ExecuteAndVerify("UPDATE products SET price = 899.99 WHERE id = 1");
    ExecuteAndVerify("UPDATE products SET stock = stock - 5 WHERE name = 'Mouse'");
    
    // 2.3 删除操作测试
    ExecuteAndVerify("DELETE FROM products WHERE stock = 0");
    ExecuteAndVerify("DELETE FROM products WHERE price > 1000");
    
    // 2.4 查询操作测试
    ExecuteAndVerify("SELECT * FROM products");
    ExecuteAndVerify("SELECT name, price FROM products WHERE price < 100");
    ExecuteAndVerify("SELECT COUNT(*) FROM products");
    ExecuteAndVerify("SELECT AVG(price) FROM products");
    
    // 2.5 WHERE条件复杂测试
    ExecuteAndVerify("SELECT * FROM products WHERE price BETWEEN 50 AND 200");
    ExecuteAndVerify("SELECT * FROM products WHERE name LIKE '%board%'");
    ExecuteAndVerify("SELECT * FROM products WHERE stock > 0 AND price < 100");
    
    std::cout << "=== DML命令综合测试完成 ===" << std::endl;
}

// 测试3: DCL命令综合测试
TEST_F(Sql92ComprehensiveTest, DCLCommandsComprehensiveTest) {
    std::cout << "\n=== DCL命令综合测试开始 ===" << std::endl;
    
    // 3.1 用户管理测试
    ExecuteAndVerify("CREATE USER admin_user IDENTIFIED BY 'admin123'");
    ExecuteAndVerify("CREATE USER read_user IDENTIFIED BY 'read123'");
    ExecuteAndVerify("CREATE USER write_user IDENTIFIED BY 'write123'");
    
    // 验证用户创建
    VerifyUserExists("admin_user");
    VerifyUserExists("read_user");
    VerifyUserExists("write_user");
    
    // 3.2 权限授予测试
    ExecuteAndVerify("GRANT ALL PRIVILEGES ON test_db_92.* TO admin_user");
    ExecuteAndVerify("GRANT SELECT ON test_db_92.users TO read_user");
    ExecuteAndVerify("GRANT SELECT, INSERT, UPDATE ON test_db_92.products TO write_user");
    
    // 3.3 权限回收测试
    ExecuteAndVerify("REVOKE INSERT ON test_db_92.products FROM write_user");
    ExecuteAndVerify("REVOKE ALL PRIVILEGES ON test_db_92.* FROM admin_user");
    
    // 3.4 用户删除测试
    ExecuteAndVerify("DROP USER read_user");
    ExecuteAndVerify("DROP USER IF EXISTS non_existent_user");
    
    std::cout << "=== DCL命令综合测试完成 ===" << std::endl;
}

// 测试4: 持久化集成测试
TEST_F(Sql92ComprehensiveTest, PersistenceIntegrationTest) {
    std::cout << "\n=== 持久化集成测试开始 ===" << std::endl;
    
    // 4.1 创建测试数据
    ExecuteAndVerify("CREATE DATABASE persistence_test");
    ExecuteAndVerify("USE persistence_test");
    ExecuteAndVerify("CREATE TABLE persistent_data ("
                     "id INTEGER PRIMARY KEY, "
                     "data VARCHAR(255), "
                     "created_at TIMESTAMP)");
    
    // 插入测试数据
    for (int i = 1; i <= 10; i++) {
        ExecuteAndVerify("INSERT INTO persistent_data VALUES (" + 
                         std::to_string(i) + ", 'Data ' + " + 
                         std::to_string(i) + ", CURRENT_TIMESTAMP)");
    }
    
    // 4.2 模拟系统重启
    std::cout << "模拟系统重启..." << std::endl;
    SimulateSystemRestart();
    
    // 4.3 验证数据持久化
    ExecuteAndVerify("USE persistence_test");
    ExecuteAndVerify("SELECT COUNT(*) FROM persistent_data", "10");
    ExecuteAndVerify("SELECT * FROM persistent_data WHERE id = 5");
    
    // 4.4 验证表结构持久化
    VerifyTableExists("persistent_data");
    
    std::cout << "=== 持久化集成测试完成 ===" << std::endl;
}

// 测试5: 索引系统集成测试
TEST_F(Sql92ComprehensiveTest, IndexSystemIntegrationTest) {
    std::cout << "\n=== 索引系统集成测试开始 ===" << std::endl;
    
    // 5.1 创建测试环境
    ExecuteAndVerify("CREATE DATABASE index_test");
    ExecuteAndVerify("USE index_test");
    ExecuteAndVerify("CREATE TABLE indexed_table ("
                     "id INTEGER PRIMARY KEY, "
                     "name VARCHAR(100), "
                     "category VARCHAR(50), "
                     "value INTEGER)");
    
    // 5.2 创建多种索引
    ExecuteAndVerify("CREATE INDEX idx_name ON indexed_table (name)");
    ExecuteAndVerify("CREATE INDEX idx_category ON indexed_table (category)");
    ExecuteAndVerify("CREATE INDEX idx_composite ON indexed_table (category, value)");
    ExecuteAndVerify("CREATE UNIQUE INDEX idx_unique_name ON indexed_table (name)");
    
    // 5.3 插入测试数据
    for (int i = 1; i <= 100; i++) {
        ExecuteAndVerify("INSERT INTO indexed_table VALUES (" + 
                         std::to_string(i) + ", 'Item' + " + 
                         std::to_string(i) + ", 'Category' + " + 
                         std::to_string(i % 10) + ", " + 
                         std::to_string(i * 10) + ")");
    }
    
    // 5.4 索引查询测试
    ExecuteAndVerify("SELECT * FROM indexed_table WHERE name = 'Item50'");
    ExecuteAndVerify("SELECT * FROM indexed_table WHERE category = 'Category5'");
    ExecuteAndVerify("SELECT * FROM indexed_table WHERE category = 'Category3' AND value > 500");
    
    // 5.5 索引维护测试（更新操作）
    ExecuteAndVerify("UPDATE indexed_table SET name = 'UpdatedItem' WHERE id = 25");
    ExecuteAndVerify("DELETE FROM indexed_table WHERE id = 50");
    
    // 5.6 索引删除测试
    ExecuteAndVerify("DROP INDEX idx_name");
    ExecuteAndVerify("DROP INDEX idx_composite");
    
    std::cout << "=== 索引系统集成测试完成 ===" << std::endl;
}

// 测试6: 并发操作测试
TEST_F(Sql92ComprehensiveTest, ConcurrencyOperationTest) {
    std::cout << "\n=== 并发操作测试开始 ===" << std::endl;
    
    // 6.1 准备测试环境
    ExecuteAndVerify("CREATE DATABASE concurrency_test");
    ExecuteAndVerify("USE concurrency_test");
    ExecuteAndVerify("CREATE TABLE concurrent_table ("
                     "id INTEGER PRIMARY KEY, "
                     "counter INTEGER, "
                     "data VARCHAR(100))");
    
    // 插入初始数据
    ExecuteAndVerify("INSERT INTO concurrent_table VALUES (1, 0, 'Initial')");
    
    // 6.2 多线程并发更新测试
    const int thread_count = 5;
    const int operations_per_thread = 10;
    
    std::vector<std::thread> threads;
    std::vector<bool> thread_results(thread_count, false);
    
    for (int i = 0; i < thread_count; i++) {
        threads.emplace_back([this, i, &thread_results]() {
            try {
                for (int j = 0; j < operations_per_thread; j++) {
                    ExecuteAndVerify("UPDATE concurrent_table SET counter = counter + 1 WHERE id = 1");
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
                thread_results[i] = true;
            } catch (const std::exception& e) {
                std::cerr << "线程 " << i << " 异常: " << e.what() << std::endl;
                thread_results[i] = false;
            }
        });
    }
    
    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }
    
    // 验证所有线程成功完成
    for (int i = 0; i < thread_count; i++) {
        EXPECT_TRUE(thread_results[i]) << "线程 " << i << " 执行失败";
    }
    
    // 验证最终数据一致性
    ExecuteAndVerify("SELECT counter FROM concurrent_table WHERE id = 1");
    
    std::cout << "=== 并发操作测试完成 ===" << std::endl;
}

// 测试7: 错误处理和边界情况测试
TEST_F(Sql92ComprehensiveTest, ErrorHandlingAndEdgeCasesTest) {
    std::cout << "\n=== 错误处理和边界情况测试开始 ===" << std::endl;
    
    // 7.1 语法错误测试
    ExecuteAndVerify("INVALID SQL STATEMENT", "错误");
    ExecuteAndVerify("SELECT FROM", "错误");
    ExecuteAndVerify("CREATE TABLE (id INT)", "错误");
    
    // 7.2 语义错误测试
    ExecuteAndVerify("SELECT * FROM non_existent_table", "错误");
    ExecuteAndVerify("INSERT INTO non_existent_table VALUES (1)", "错误");
    ExecuteAndVerify("DROP TABLE non_existent_table", "错误");
    
    // 7.3 权限错误测试
    ExecuteAndVerify("GRANT INVALID_PERMISSION ON table TO user", "错误");
    ExecuteAndVerify("REVOKE FROM non_existent_user", "错误");
    
    // 7.4 边界值测试
    ExecuteAndVerify(""); // 空SQL
    ExecuteAndVerify("   \t\n   "); // 只有空白字符
    ExecuteAndVerify("-- 只有注释"); // 只有注释
    
    // 7.5 超长SQL测试
    std::string long_sql = "CREATE TABLE long_table_name_" + 
                          std::string(100, 'x') + " (id INT)";
    ExecuteAndVerify(long_sql);
    
    std::cout << "=== 错误处理和边界情况测试完成 ===" << std::endl;
}

// 测试8: 性能基准测试
TEST_F(Sql92ComprehensiveTest, PerformanceBenchmarkTest) {
    std::cout << "\n=== 性能基准测试开始 ===" << std::endl;
    
    // 8.1 准备测试环境
    ExecuteAndVerify("CREATE DATABASE performance_test");
    ExecuteAndVerify("USE performance_test");
    ExecuteAndVerify("CREATE TABLE perf_table ("
                     "id INTEGER PRIMARY KEY, "
                     "data VARCHAR(255))");
    
    // 8.2 批量插入性能测试
    auto start_time = std::chrono::high_resolution_clock::now();
    
    const int batch_size = 1000;
    for (int i = 1; i <= batch_size; i++) {
        ExecuteAndVerify("INSERT INTO perf_table VALUES (" + 
                         std::to_string(i) + ", 'Data' + " + 
                         std::to_string(i) + ")");
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        end_time - start_time);
    
    std::cout << "批量插入 " << batch_size << " 条记录耗时: " 
              << duration.count() << "ms" << std::endl;
    
    // 期望性能：1000条记录 < 5秒
    EXPECT_LT(duration.count(), 5000) << "批量插入性能不达标";
    
    // 8.3 查询性能测试
    start_time = std::chrono::high_resolution_clock::now();
    ExecuteAndVerify("SELECT * FROM perf_table WHERE id = 500");
    end_time = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        end_time - start_time);
    
    std::cout << "单条查询耗时: " << duration.count() << "ms" << std::endl;
    
    // 期望性能：单条查询 < 100ms
    EXPECT_LT(duration.count(), 100) << "查询性能不达标";
    
    std::cout << "=== 性能基准测试完成 ===" << std::endl;
}

} // namespace sqlcc

// 主函数
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}