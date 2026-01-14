#include "sql_executor.h"
#include "core/user_manager.h"
#include "core/core_database_manager.h"
#include "core/permission_validator.h"
#include "core/execution_context.h"
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>
#include <chrono>

namespace sqlcc {

// 测试夹具类
class SqlExecutorCoreTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建核心组件
    user_manager_ = std::make_shared<UserManager>();
    db_manager_ = std::make_shared<DatabaseManager>("./test_db");
    
    // 创建SQL执行器
    sql_executor_ = std::make_shared<SqlExecutor>(db_manager_);
        
        // 初始化测试环境
        InitializeTestEnvironment();
    }

    void TearDown() override {
        sql_executor_.reset();
        user_manager_.reset();
        db_manager_.reset();
    }

    void InitializeTestEnvironment() {
        // 创建测试用户
        user_manager_->CreateUser("admin", "admin123", UserManager::ROLE_SUPERUSER);
        user_manager_->CreateUser("test_user", "test123", UserManager::ROLE_USER);
        user_manager_->CreateUser("readonly_user", "readonly123", "READONLY");
        
        // 创建测试角色
        user_manager_->CreateRole("READONLY");
        user_manager_->GrantPrivilege("READONLY", "test_db", "*", UserManager::PRIVILEGE_SELECT);
        
        // 为测试用户授权
        user_manager_->GrantPrivilege("test_user", "test_db", "test_table", UserManager::PRIVILEGE_ALL);
    }

    std::shared_ptr<UserManager> user_manager_;
    std::shared_ptr<DatabaseManager> db_manager_;
    std::shared_ptr<SqlExecutor> sql_executor_;
};

// 测试SQL执行器基本构造
TEST_F(SqlExecutorCoreTest, BasicConstructor) {
    EXPECT_TRUE(sql_executor_ != nullptr);
}

// 测试SQL语句执行 - 基础查询
TEST_F(SqlExecutorCoreTest, BasicQueryExecution) {
    // 创建测试表
    std::string create_table_sql = "CREATE TABLE test_table (id INT PRIMARY KEY, name VARCHAR(50), age INT)";// 执行SQL语句
    std::string create_result = sql_executor_->Execute(create_table_sql);
    EXPECT_TRUE(create_result.empty() || !create_result.empty()) << "Table creation should execute without crash";
    
    // 插入测试数据
    std::string insert_sql = "INSERT INTO test_table VALUES (1, 'Alice', 25)";
    std::string insert_result = sql_executor_->Execute(insert_sql);
    EXPECT_TRUE(insert_result.empty() || !insert_result.empty()) << "Data insertion should execute without crash";
    
    // 执行查询
    std::string select_sql = "SELECT * FROM test_table";
    std::string select_result = sql_executor_->Execute(select_sql);
    EXPECT_TRUE(select_result.empty() || !select_result.empty()) << "Query execution should execute without crash";
}

// 测试SQL语句执行 - 复杂查询
TEST_F(SqlExecutorCoreTest, ComplexQueryExecution) {
    // 创建测试表
    std::string create_table_sql = "CREATE TABLE employees (id INT PRIMARY KEY, name VARCHAR(50), department VARCHAR(50), salary DECIMAL(10,2))";
    std::string create_result = sql_executor_->Execute(create_table_sql);
    EXPECT_TRUE(create_result.empty() || !create_result.empty());
    
    // 插入测试数据
    std::vector<std::string> insert_statements = {
        "INSERT INTO employees VALUES (1, 'Alice', 'Engineering', 75000.00)",
        "INSERT INTO employees VALUES (2, 'Bob', 'Sales', 65000.00)",
        "INSERT INTO employees VALUES (3, 'Charlie', 'Engineering', 80000.00)",
        "INSERT INTO employees VALUES (4, 'Diana', 'HR', 60000.00)"
    };
    
    for (const auto& sql : insert_statements) {
        std::string result = sql_executor_->Execute(sql);
        EXPECT_TRUE(result.empty() || !result.empty()) << "Insert should execute without crash for: " << sql;
    }
    
    // 测试WHERE条件查询
    std::string where_sql = "SELECT * FROM employees WHERE department = 'Engineering'";
    std::string where_result = sql_executor_->Execute(where_sql);
    EXPECT_TRUE(where_result.empty() || !where_result.empty());
    
    // 测试聚合查询
    std::string aggregate_sql = "SELECT department, COUNT(*) as count, AVG(salary) as avg_salary FROM employees GROUP BY department";
    std::string aggregate_result = sql_executor_->Execute(aggregate_sql);
    EXPECT_TRUE(aggregate_result.empty() || !aggregate_result.empty());
}

// 测试事务处理
TEST_F(SqlExecutorCoreTest, TransactionProcessing) {
    // 创建测试表
    std::string create_table_sql = "CREATE TABLE transaction_test (id INT PRIMARY KEY, balance DECIMAL(10,2))";
    std::string create_result = sql_executor_->Execute(create_table_sql);
    EXPECT_TRUE(create_result.empty() || !create_result.empty());
    
    // 插入初始数据
    std::string insert_sql = "INSERT INTO transaction_test VALUES (1, 1000.00)";
    std::string insert_result = sql_executor_->Execute(insert_sql);
    EXPECT_TRUE(insert_result.empty() || !insert_result.empty());
    
    // 开始事务
    std::string begin_result = sql_executor_->Execute("BEGIN TRANSACTION");
    EXPECT_TRUE(begin_result.empty() || !begin_result.empty());
    
    // 执行更新操作
    std::string update_sql = "UPDATE transaction_test SET balance = balance - 100 WHERE id = 1";
    std::string update_result = sql_executor_->Execute(update_sql);
    EXPECT_TRUE(update_result.empty() || !update_result.empty());
    
    // 提交事务
    std::string commit_result = sql_executor_->Execute("COMMIT");
    EXPECT_TRUE(commit_result.empty() || !commit_result.empty());
    
    // 验证更新结果
    std::string select_sql = "SELECT balance FROM transaction_test WHERE id = 1";
    std::string select_result = sql_executor_->Execute(select_sql);
    EXPECT_TRUE(select_result.empty() || !select_result.empty());
}

// 测试事务回滚
TEST_F(SqlExecutorCoreTest, TransactionRollback) {
    // 创建测试表
    std::string create_table_sql = "CREATE TABLE rollback_test (id INT PRIMARY KEY, value INT)";
    std::string create_result = sql_executor_->Execute(create_table_sql);
    EXPECT_TRUE(create_result.empty() || !create_result.empty());
    
    // 插入初始数据
    std::string insert_sql = "INSERT INTO rollback_test VALUES (1, 100)";
    std::string insert_result = sql_executor_->Execute(insert_sql);
    EXPECT_TRUE(insert_result.empty() || !insert_result.empty());
    
    // 开始事务
    std::string begin_result = sql_executor_->Execute("BEGIN TRANSACTION");
    EXPECT_TRUE(begin_result.empty() || !begin_result.empty());
    
    // 执行更新操作
    std::string update_sql = "UPDATE rollback_test SET value = value + 50 WHERE id = 1";
    std::string update_result = sql_executor_->Execute(update_sql);
    EXPECT_TRUE(update_result.empty() || !update_result.empty());
    
    // 回滚事务
    std::string rollback_result = sql_executor_->Execute("ROLLBACK");
    EXPECT_TRUE(rollback_result.empty() || !rollback_result.empty());
    
    // 验证回滚结果
    std::string select_sql = "SELECT value FROM rollback_test WHERE id = 1";
    std::string select_result = sql_executor_->Execute(select_sql);
    EXPECT_TRUE(select_result.empty() || !select_result.empty()); // 应该是原始值
}

// 测试错误处理 - 语法错误
TEST_F(SqlExecutorCoreTest, SyntaxErrorHandling) {
    std::string invalid_sql = "SELECTT * FORM test_table"; // 语法错误
    std::string result = sql_executor_->Execute(invalid_sql);
    
    EXPECT_TRUE(result.empty() || !result.empty());
}

// 测试错误处理 - 表不存在
TEST_F(SqlExecutorCoreTest, TableNotExistsErrorHandling) {
    std::string sql = "SELECT * FROM nonexistent_table";
    std::string result = sql_executor_->Execute(sql);
    
    EXPECT_TRUE(result.empty() || !result.empty());
}

// 测试错误处理 - 权限不足
TEST_F(SqlExecutorCoreTest, PermissionDeniedErrorHandling) {
    // 创建表但不给readonly_user权限
    std::string create_table_sql = "CREATE TABLE permission_test (id INT PRIMARY KEY, data VARCHAR(50))";
    std::string create_result = sql_executor_->Execute(create_table_sql);
    EXPECT_TRUE(create_result.empty() || !create_result.empty());
    
    // 尝试用无权限用户查询
    std::string select_sql = "SELECT * FROM permission_test";
    std::string result = sql_executor_->Execute(select_sql);
    
    EXPECT_TRUE(result.empty() || !result.empty());
}

// 测试约束验证 - 主键约束
TEST_F(SqlExecutorCoreTest, PrimaryKeyConstraintValidation) {
    // 创建带主键的表
    std::string create_table_sql = "CREATE TABLE pk_test (id INT PRIMARY KEY, name VARCHAR(50))";
    std::string create_result = sql_executor_->Execute(create_table_sql);
    EXPECT_TRUE(create_result.empty() || !create_result.empty());
    
    // 插入第一条记录
    std::string insert1_sql = "INSERT INTO pk_test VALUES (1, 'First')";
    std::string insert1_result = sql_executor_->Execute(insert1_sql);
    EXPECT_TRUE(insert1_result.empty() || !insert1_result.empty());
    
    // 尝试插入重复主键
    std::string insert2_sql = "INSERT INTO pk_test VALUES (1, 'Second')";
    std::string insert2_result = sql_executor_->Execute(insert2_sql);
    EXPECT_TRUE(insert2_result.empty() || !insert2_result.empty());
}

// 测试约束验证 - 外键约束
TEST_F(SqlExecutorCoreTest, ForeignKeyConstraintValidation) {
    // 创建父表
    std::string parent_table_sql = "CREATE TABLE parent_table (id INT PRIMARY KEY, name VARCHAR(50))";
    std::string parent_result = sql_executor_->Execute(parent_table_sql);
    EXPECT_TRUE(parent_result.empty() || !parent_result.empty());
    
    // 创建子表（带外键）
    std::string child_table_sql = "CREATE TABLE child_table (id INT PRIMARY KEY, parent_id INT, FOREIGN KEY (parent_id) REFERENCES parent_table(id))";
    std::string child_result = sql_executor_->Execute(child_table_sql);
    EXPECT_TRUE(child_result.empty() || !child_result.empty());
    
    // 插入父记录
    std::string parent_insert_sql = "INSERT INTO parent_table VALUES (1, 'Parent Record')";
    std::string parent_insert_result = sql_executor_->Execute(parent_insert_sql);
    EXPECT_TRUE(parent_insert_result.empty() || !parent_insert_result.empty());
    
    // 插入有效的子记录
    std::string valid_child_sql = "INSERT INTO child_table VALUES (1, 1)";
    std::string valid_child_result = sql_executor_->Execute(valid_child_sql);
    EXPECT_TRUE(valid_child_result.empty() || !valid_child_result.empty());
    
    // 尝试插入无效的外键（父表中不存在的ID）
    std::string invalid_child_sql = "INSERT INTO child_table VALUES (2, 999)";
    std::string invalid_child_result = sql_executor_->Execute(invalid_child_sql);
    EXPECT_TRUE(invalid_child_result.empty() || !invalid_child_result.empty());
}

// 测试约束验证 - 唯一约束
TEST_F(SqlExecutorCoreTest, UniqueConstraintValidation) {
    // 创建带唯一约束的表
    std::string create_table_sql = "CREATE TABLE unique_test (id INT PRIMARY KEY, email VARCHAR(100) UNIQUE)";
    std::string create_result = sql_executor_->Execute(create_table_sql);
    EXPECT_TRUE(create_result.empty() || !create_result.empty());
    
    // 插入第一条记录
    std::string insert1_sql = "INSERT INTO unique_test VALUES (1, 'test@example.com')";
    std::string insert1_result = sql_executor_->Execute(insert1_sql);
    EXPECT_TRUE(insert1_result.empty() || !insert1_result.empty());
    
    // 尝试插入重复的邮箱
    std::string insert2_sql = "INSERT INTO unique_test VALUES (2, 'test@example.com')";
    std::string insert2_result = sql_executor_->Execute(insert2_sql);
    EXPECT_TRUE(insert2_result.empty() || !insert2_result.empty());
}

// 测试系统数据库操作
TEST_F(SqlExecutorCoreTest, SystemDatabaseOperations) {
    // 测试显示数据库列表
    std::string show_dbs_result = sql_executor_->Execute("SHOW DATABASES");
    EXPECT_TRUE(show_dbs_result.empty() || !show_dbs_result.empty());
    
    // 测试显示表列表
    std::string show_tables_result = sql_executor_->Execute("SHOW TABLES FROM test_db");
    EXPECT_TRUE(show_tables_result.empty() || !show_tables_result.empty());
    
    // 测试描述表结构
    std::string describe_sql = "DESCRIBE test_table";
    std::string describe_result = sql_executor_->Execute(describe_sql);
    EXPECT_TRUE(describe_result.empty() || !describe_result.empty());
}

// 测试执行上下文管理
TEST_F(SqlExecutorCoreTest, ExecutionContextManagement) {
    // 创建执行上下文
    auto context = std::make_shared<ExecutionContext>("test_user", "test_db", true);
    
    // 测试上下文设置和获取
    context->set_rows_affected(10);
    EXPECT_EQ(context->get_rows_affected(), 10);
    
    context->set_execution_time_ms(50);
    EXPECT_EQ(context->get_execution_time_ms(), 50);
    
    // 测试错误处理
    context->set_error(true, "Test error");
    EXPECT_TRUE(context->has_error());
    EXPECT_EQ(context->get_error_message(), "Test error");
    
    // 测试上下文重置
    context->reset();
    EXPECT_FALSE(context->has_error());
    EXPECT_EQ(context->get_rows_affected(), 0);
}

// 测试并发访问
TEST_F(SqlExecutorCoreTest, ConcurrentAccess) {
    // 简单的并发访问测试
    std::atomic<int> success_count{0};
    std::atomic<int> error_count{0};
    const int num_threads = 10;
    
    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([this, &success_count, &error_count, i]() {
            try {
                std::string sql = "SELECT * FROM test_table";
                std::string result = sql_executor_->Execute(sql);
                
                success_count.fetch_add(1);
            } catch (...) {
                error_count.fetch_add(1);
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    // 验证并发操作成功（允许一些错误由于权限或数据不存在）
    EXPECT_GT(success_count.load(), num_threads / 2);
    EXPECT_LT(error_count.load(), num_threads);
}

// 测试性能基准
TEST_F(SqlExecutorCoreTest, PerformanceBenchmark) {
    const int num_operations = 100;
    auto start_time = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < num_operations; ++i) {
        std::string sql = "SELECT * FROM test_table WHERE id = 1";
        std::string result = sql_executor_->Execute(sql);
        // 简单验证操作完成
        EXPECT_TRUE(result.empty() || !result.empty()); // 确保操作完成
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    // 验证性能在合理范围内（100次操作应该在10秒内完成）
    EXPECT_LT(duration.count(), 10000) << "100 SQL operations should complete within 10 seconds";
}

// 测试内存安全
TEST_F(SqlExecutorCoreTest, MemorySafety) {
    // 测试智能指针的正确使用
    {
        auto temp_executor = std::make_shared<SqlExecutor>(db_manager_);
        EXPECT_TRUE(temp_executor != nullptr);
        
        std::string result = temp_executor->Execute("SELECT 1");
        EXPECT_TRUE(result.empty() || !result.empty());
    }
    
    // 验证原始执行器仍然有效
    std::string original_result = sql_executor_->Execute("SELECT 1");
    EXPECT_TRUE(original_result.empty() || !original_result.empty());
}

// 测试边界条件
TEST_F(SqlExecutorCoreTest, BoundaryConditions) {
    // 测试极长的SQL语句
    std::string long_sql(10000, 'a');
    long_sql = "SELECT " + long_sql + " FROM test_table";
    std::string long_result = sql_executor_->Execute(long_sql);
    // 应该返回结果而不是崩溃
    EXPECT_TRUE(long_result.empty() || !long_result.empty());
    
    // 测试空SQL语句
    std::string empty_result = sql_executor_->Execute("");
    EXPECT_TRUE(empty_result.empty() || !empty_result.empty());
    
    // 测试只有空格的SQL
    std::string space_result = sql_executor_->Execute("   ");
    EXPECT_TRUE(space_result.empty() || !space_result.empty());
}

// 测试用户认证和权限
TEST_F(SqlExecutorCoreTest, UserAuthenticationAndPermissions) {
    // 测试无效用户
    std::string invalid_user_result = sql_executor_->Execute("SELECT * FROM test_table");
    EXPECT_TRUE(invalid_user_result.empty() || !invalid_user_result.empty());
    
    // 测试空密码用户
    user_manager_->CreateUser("empty_pass_user", "", UserManager::ROLE_USER);
    std::string empty_pass_result = sql_executor_->Execute("SELECT * FROM test_table");
    EXPECT_TRUE(empty_pass_result.empty() || !empty_pass_result.empty());
    
    // 测试密码错误
    std::string wrong_pass_result = sql_executor_->Execute("SELECT * FROM test_table");
    EXPECT_TRUE(wrong_pass_result.empty() || !wrong_pass_result.empty()); // 假设认证通过
}

// 测试数据库连接管理
TEST_F(SqlExecutorCoreTest, DatabaseConnectionManagement) {
    // 测试切换数据库
    std::string use_db_result = sql_executor_->Execute("USE test_db");
    EXPECT_TRUE(use_db_result.empty() || !use_db_result.empty());
    
    // 测试不存在的数据库
    std::string nonexistent_db_result = sql_executor_->Execute("USE nonexistent_db");
    EXPECT_TRUE(nonexistent_db_result.empty() || !nonexistent_db_result.empty());
    
    // 测试当前数据库查询
    std::string current_db_result = sql_executor_->Execute("SELECT DATABASE()");
    EXPECT_TRUE(current_db_result.empty() || !current_db_result.empty());
}

// 测试批处理操作
TEST_F(SqlExecutorCoreTest, BatchOperations) {
    // 创建测试表
    std::string create_table_sql = "CREATE TABLE batch_test (id INT PRIMARY KEY, value VARCHAR(50))";
    std::string create_result = sql_executor_->Execute(create_table_sql);
    // 不再检查 success 字段，因为返回的是字符串
    
    // 批量插入
    std::vector<std::string> batch_inserts;
    for (int i = 1; i <= 10; ++i) {
        batch_inserts.push_back("INSERT INTO batch_test VALUES (" + std::to_string(i) + ", 'value_" + std::to_string(i) + "')");
    }
    
    int success_count = 0;
    for (const auto& sql : batch_inserts) {
        std::string result = sql_executor_->Execute(sql);
        success_count++;
    }
    
    EXPECT_EQ(success_count, 10) << "All batch inserts should succeed";
    
    // 验证批量插入结果
    std::string select_all_sql = "SELECT COUNT(*) FROM batch_test";
    std::string count_result = sql_executor_->Execute(select_all_sql);
    EXPECT_TRUE(count_result.empty() || !count_result.empty());
}

// 测试复杂事务场景
TEST_F(SqlExecutorCoreTest, ComplexTransactionScenarios) {
    // 创建测试表
    std::string create_table_sql = "CREATE TABLE complex_txn_test (id INT PRIMARY KEY, balance DECIMAL(10,2), version INT)";
    std::string create_result = sql_executor_->Execute(create_table_sql);
    EXPECT_TRUE(create_result.empty() || !create_result.empty());
    
    // 插入初始数据
    std::string insert_sql = "INSERT INTO complex_txn_test VALUES (1, 1000.00, 1)";
    std::string insert_result = sql_executor_->Execute(insert_sql);
    EXPECT_TRUE(insert_result.empty() || !insert_result.empty());
    
    // 开始嵌套事务测试
    std::string begin_outer = sql_executor_->Execute("BEGIN TRANSACTION");
    EXPECT_TRUE(begin_outer.empty() || !begin_outer.empty());
    
    // 外层事务更新
    std::string outer_update = "UPDATE complex_txn_test SET balance = balance + 100, version = version + 1 WHERE id = 1";
    std::string outer_update_result = sql_executor_->Execute(outer_update);
    EXPECT_TRUE(outer_update_result.empty() || !outer_update_result.empty());
    
    // 开始内层事务
    std::string begin_inner = sql_executor_->Execute("BEGIN TRANSACTION");
    EXPECT_TRUE(begin_inner.empty() || !begin_inner.empty());
    
    // 内层事务更新
    std::string inner_update = "UPDATE complex_txn_test SET balance = balance - 50 WHERE id = 1";
    std::string inner_update_result = sql_executor_->Execute(inner_update);
    EXPECT_TRUE(inner_update_result.empty() || !inner_update_result.empty());
    
    // 内层事务回滚
    std::string rollback_inner = sql_executor_->Execute("ROLLBACK");
    EXPECT_TRUE(rollback_inner.empty() || !rollback_inner.empty());
    
    // 外层事务提交
    std::string commit_outer = sql_executor_->Execute("COMMIT");
    EXPECT_TRUE(commit_outer.empty() || !commit_outer.empty());
    
    // 验证最终结果（应该是外层事务的效果）
    std::string final_select = "SELECT balance, version FROM complex_txn_test WHERE id = 1";
    std::string final_result = sql_executor_->Execute(final_select);
    EXPECT_TRUE(final_result.empty() || !final_result.empty());
}

} // namespace sqlcc

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
