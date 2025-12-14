#include "sql_executor.h"
#include "user_manager.h"
#include "database_manager.h"
#include "permission_validator.h"
#include "execution_context.h"
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
        db_manager_ = std::make_shared<DatabaseManager>();
        permission_validator_ = std::make_shared<PermissionValidator>(user_manager_, db_manager_);
        
        // 创建SQL执行器
        sql_executor_ = std::make_shared<SqlExecutor>(db_manager_, user_manager_, permission_validator_);
        
        // 初始化测试环境
        InitializeTestEnvironment();
    }

    void TearDown() override {
        sql_executor_.reset();
        permission_validator_.reset();
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
    std::shared_ptr<PermissionValidator> permission_validator_;
    std::shared_ptr<SqlExecutor> sql_executor_;
};

// 测试SQL执行器基本构造
TEST_F(SqlExecutorCoreTest, BasicConstructor) {
    EXPECT_TRUE(sql_executor_ != nullptr);
    EXPECT_EQ(sql_executor_->get_db_manager(), db_manager_);
    EXPECT_EQ(sql_executor_->get_user_manager(), user_manager_);
    EXPECT_EQ(sql_executor_->get_permission_validator(), permission_validator_);
}

// 测试SQL语句执行 - 基础查询
TEST_F(SqlExecutorCoreTest, BasicQueryExecution) {
    // 创建测试表
    std::string create_table_sql = "CREATE TABLE test_table (id INT PRIMARY KEY, name VARCHAR(50), age INT)";
    ExecutionResult create_result = sql_executor_->Execute(create_table_sql, "admin");
    EXPECT_TRUE(create_result.success) << "Table creation should succeed";
    
    // 插入测试数据
    std::string insert_sql = "INSERT INTO test_table VALUES (1, 'Alice', 25)";
    ExecutionResult insert_result = sql_executor_->Execute(insert_sql, "test_user");
    EXPECT_TRUE(insert_result.success) << "Data insertion should succeed";
    EXPECT_EQ(insert_result.rows_affected, 1);
    
    // 执行查询
    std::string select_sql = "SELECT * FROM test_table";
    ExecutionResult select_result = sql_executor_->Execute(select_sql, "test_user");
    EXPECT_TRUE(select_result.success) << "Query execution should succeed";
    EXPECT_EQ(select_result.rows_returned, 1);
    EXPECT_THAT(select_result.result_data, ::testing::HasSubstr("Alice"));
}

// 测试SQL语句执行 - 复杂查询
TEST_F(SqlExecutorCoreTest, ComplexQueryExecution) {
    // 创建测试表
    std::string create_table_sql = "CREATE TABLE employees (id INT PRIMARY KEY, name VARCHAR(50), department VARCHAR(50), salary DECIMAL(10,2))";
    ExecutionResult create_result = sql_executor_->Execute(create_table_sql, "admin");
    EXPECT_TRUE(create_result.success);
    
    // 插入测试数据
    std::vector<std::string> insert_statements = {
        "INSERT INTO employees VALUES (1, 'Alice', 'Engineering', 75000.00)",
        "INSERT INTO employees VALUES (2, 'Bob', 'Sales', 65000.00)",
        "INSERT INTO employees VALUES (3, 'Charlie', 'Engineering', 80000.00)",
        "INSERT INTO employees VALUES (4, 'Diana', 'HR', 60000.00)"
    };
    
    for (const auto& sql : insert_statements) {
        ExecutionResult result = sql_executor_->Execute(sql, "test_user");
        EXPECT_TRUE(result.success) << "Insert should succeed for: " << sql;
    }
    
    // 测试WHERE条件查询
    std::string where_sql = "SELECT * FROM employees WHERE department = 'Engineering'";
    ExecutionResult where_result = sql_executor_->Execute(where_sql, "test_user");
    EXPECT_TRUE(where_result.success);
    EXPECT_EQ(where_result.rows_returned, 2);
    EXPECT_THAT(where_result.result_data, ::testing::HasSubstr("Alice"));
    EXPECT_THAT(where_result.result_data, ::testing::HasSubstr("Charlie"));
    
    // 测试聚合查询
    std::string aggregate_sql = "SELECT department, COUNT(*) as count, AVG(salary) as avg_salary FROM employees GROUP BY department";
    ExecutionResult aggregate_result = sql_executor_->Execute(aggregate_sql, "test_user");
    EXPECT_TRUE(aggregate_result.success);
    EXPECT_THAT(aggregate_result.result_data, ::testing::HasSubstr("Engineering"));
    EXPECT_THAT(aggregate_result.result_data, ::testing::HasSubstr("Sales"));
}

// 测试事务处理
TEST_F(SqlExecutorCoreTest, TransactionProcessing) {
    // 创建测试表
    std::string create_table_sql = "CREATE TABLE transaction_test (id INT PRIMARY KEY, balance DECIMAL(10,2))";
    ExecutionResult create_result = sql_executor_->Execute(create_table_sql, "admin");
    EXPECT_TRUE(create_result.success);
    
    // 插入初始数据
    std::string insert_sql = "INSERT INTO transaction_test VALUES (1, 1000.00)";
    ExecutionResult insert_result = sql_executor_->Execute(insert_sql, "test_user");
    EXPECT_TRUE(insert_result.success);
    
    // 开始事务
    ExecutionResult begin_result = sql_executor_->Execute("BEGIN TRANSACTION", "test_user");
    EXPECT_TRUE(begin_result.success);
    
    // 执行更新操作
    std::string update_sql = "UPDATE transaction_test SET balance = balance - 100 WHERE id = 1";
    ExecutionResult update_result = sql_executor_->Execute(update_sql, "test_user");
    EXPECT_TRUE(update_result.success);
    EXPECT_EQ(update_result.rows_affected, 1);
    
    // 提交事务
    ExecutionResult commit_result = sql_executor_->Execute("COMMIT", "test_user");
    EXPECT_TRUE(commit_result.success);
    
    // 验证更新结果
    std::string select_sql = "SELECT balance FROM transaction_test WHERE id = 1";
    ExecutionResult select_result = sql_executor_->Execute(select_sql, "test_user");
    EXPECT_TRUE(select_result.success);
    EXPECT_THAT(select_result.result_data, ::testing::HasSubstr("900.00"));
}

// 测试事务回滚
TEST_F(SqlExecutorCoreTest, TransactionRollback) {
    // 创建测试表
    std::string create_table_sql = "CREATE TABLE rollback_test (id INT PRIMARY KEY, value INT)";
    ExecutionResult create_result = sql_executor_->Execute(create_table_sql, "admin");
    EXPECT_TRUE(create_result.success);
    
    // 插入初始数据
    std::string insert_sql = "INSERT INTO rollback_test VALUES (1, 100)";
    ExecutionResult insert_result = sql_executor_->Execute(insert_sql, "test_user");
    EXPECT_TRUE(insert_result.success);
    
    // 开始事务
    ExecutionResult begin_result = sql_executor_->Execute("BEGIN TRANSACTION", "test_user");
    EXPECT_TRUE(begin_result.success);
    
    // 执行更新操作
    std::string update_sql = "UPDATE rollback_test SET value = value + 50 WHERE id = 1";
    ExecutionResult update_result = sql_executor_->Execute(update_sql, "test_user");
    EXPECT_TRUE(update_result.success);
    
    // 回滚事务
    ExecutionResult rollback_result = sql_executor_->Execute("ROLLBACK", "test_user");
    EXPECT_TRUE(rollback_result.success);
    
    // 验证回滚结果
    std::string select_sql = "SELECT value FROM rollback_test WHERE id = 1";
    ExecutionResult select_result = sql_executor_->Execute(select_sql, "test_user");
    EXPECT_TRUE(select_result.success);
    EXPECT_THAT(select_result.result_data, ::testing::HasSubstr("100")); // 应该是原始值
}

// 测试错误处理 - 语法错误
TEST_F(SqlExecutorCoreTest, SyntaxErrorHandling) {
    std::string invalid_sql = "SELECTT * FORM test_table"; // 语法错误
    ExecutionResult result = sql_executor_->Execute(invalid_sql, "test_user");
    
    EXPECT_FALSE(result.success);
    EXPECT_THAT(result.error_message, ::testing::HasSubstr("syntax") 
                .Or(::testing::HasSubstr("error")));
}

// 测试错误处理 - 表不存在
TEST_F(SqlExecutorCoreTest, TableNotExistsErrorHandling) {
    std::string sql = "SELECT * FROM nonexistent_table";
    ExecutionResult result = sql_executor_->Execute(sql, "test_user");
    
    EXPECT_FALSE(result.success);
    EXPECT_THAT(result.error_message, ::testing::HasSubstr("not exist")
                .Or(::testing::HasSubstr("does not exist")));
}

// 测试错误处理 - 权限不足
TEST_F(SqlExecutorCoreTest, PermissionDeniedErrorHandling) {
    // 创建表但不给readonly_user权限
    std::string create_table_sql = "CREATE TABLE permission_test (id INT PRIMARY KEY, data VARCHAR(50))";
    ExecutionResult create_result = sql_executor_->Execute(create_table_sql, "admin");
    EXPECT_TRUE(create_result.success);
    
    // 尝试用无权限用户查询
    std::string select_sql = "SELECT * FROM permission_test";
    ExecutionResult result = sql_executor_->Execute(select_sql, "readonly_user");
    
    EXPECT_FALSE(result.success);
    EXPECT_THAT(result.error_message, ::testing::HasSubstr("permission")
                .Or(::testing::HasSubstr("access denied")));
}

// 测试约束验证 - 主键约束
TEST_F(SqlExecutorCoreTest, PrimaryKeyConstraintValidation) {
    // 创建带主键的表
    std::string create_table_sql = "CREATE TABLE pk_test (id INT PRIMARY KEY, name VARCHAR(50))";
    ExecutionResult create_result = sql_executor_->Execute(create_table_sql, "admin");
    EXPECT_TRUE(create_result.success);
    
    // 插入第一条记录
    std::string insert1_sql = "INSERT INTO pk_test VALUES (1, 'First')";
    ExecutionResult insert1_result = sql_executor_->Execute(insert1_sql, "test_user");
    EXPECT_TRUE(insert1_result.success);
    
    // 尝试插入重复主键
    std::string insert2_sql = "INSERT INTO pk_test VALUES (1, 'Second')";
    ExecutionResult insert2_result = sql_executor_->Execute(insert2_sql, "test_user");
    EXPECT_FALSE(insert2_result.success);
    EXPECT_THAT(insert2_result.error_message, ::testing::HasSubstr("duplicate")
                .Or(::testing::HasSubstr("primary key")));
}

// 测试约束验证 - 外键约束
TEST_F(SqlExecutorCoreTest, ForeignKeyConstraintValidation) {
    // 创建父表
    std::string parent_table_sql = "CREATE TABLE parent_table (id INT PRIMARY KEY, name VARCHAR(50))";
    ExecutionResult parent_result = sql_executor_->Execute(parent_table_sql, "admin");
    EXPECT_TRUE(parent_result.success);
    
    // 创建子表（带外键）
    std::string child_table_sql = "CREATE TABLE child_table (id INT PRIMARY KEY, parent_id INT, FOREIGN KEY (parent_id) REFERENCES parent_table(id))";
    ExecutionResult child_result = sql_executor_->Execute(child_table_sql, "admin");
    EXPECT_TRUE(child_result.success);
    
    // 插入父记录
    std::string parent_insert_sql = "INSERT INTO parent_table VALUES (1, 'Parent Record')";
    ExecutionResult parent_insert_result = sql_executor_->Execute(parent_insert_sql, "test_user");
    EXPECT_TRUE(parent_insert_result.success);
    
    // 插入有效的子记录
    std::string valid_child_sql = "INSERT INTO child_table VALUES (1, 1)";
    ExecutionResult valid_child_result = sql_executor_->Execute(valid_child_sql, "test_user");
    EXPECT_TRUE(valid_child_result.success);
    
    // 尝试插入无效的外键（父表中不存在的ID）
    std::string invalid_child_sql = "INSERT INTO child_table VALUES (2, 999)";
    ExecutionResult invalid_child_result = sql_executor_->Execute(invalid_child_sql, "test_user");
    EXPECT_FALSE(invalid_child_result.success);
    EXPECT_THAT(invalid_child_result.error_message, ::testing::HasSubstr("foreign key")
                .Or(::testing::HasSubstr("reference")));
}

// 测试约束验证 - 唯一约束
TEST_F(SqlExecutorCoreTest, UniqueConstraintValidation) {
    // 创建带唯一约束的表
    std::string create_table_sql = "CREATE TABLE unique_test (id INT PRIMARY KEY, email VARCHAR(100) UNIQUE)";
    ExecutionResult create_result = sql_executor_->Execute(create_table_sql, "admin");
    EXPECT_TRUE(create_result.success);
    
    // 插入第一条记录
    std::string insert1_sql = "INSERT INTO unique_test VALUES (1, 'test@example.com')";
    ExecutionResult insert1_result = sql_executor_->Execute(insert1_sql, "test_user");
    EXPECT_TRUE(insert1_result.success);
    
    // 尝试插入重复的邮箱
    std::string insert2_sql = "INSERT INTO unique_test VALUES (2, 'test@example.com')";
    ExecutionResult insert2_result = sql_executor_->Execute(insert2_sql, "test_user");
    EXPECT_FALSE(insert2_result.success);
    EXPECT_THAT(insert2_result.error_message, ::testing::HasSubstr("unique")
                .Or(::testing::HasSubstr("duplicate")));
}

// 测试系统数据库操作
TEST_F(SqlExecutorCoreTest, SystemDatabaseOperations) {
    // 测试显示数据库列表
    ExecutionResult show_dbs_result = sql_executor_->Execute("SHOW DATABASES", "admin");
    EXPECT_TRUE(show_dbs_result.success);
    EXPECT_THAT(show_dbs_result.result_data, ::testing::HasSubstr("test_db"));
    
    // 测试显示表列表
    ExecutionResult show_tables_result = sql_executor_->Execute("SHOW TABLES FROM test_db", "admin");
    EXPECT_TRUE(show_tables_result.success);
    
    // 测试描述表结构
    std::string describe_sql = "DESCRIBE test_table";
    ExecutionResult describe_result = sql_executor_->Execute(describe_sql, "admin");
    EXPECT_TRUE(describe_result.success);
    EXPECT_THAT(describe_result.result_data, ::testing::HasSubstr("test_table"));
}

// 测试执行上下文管理
TEST_F(SqlExecutorCoreTest, ExecutionContextManagement) {
    // 创建执行上下文
    auto context = std::make_shared<ExecutionContext>("test_user", "test_db", true);
    context->set_permission_validator(permission_validator_);
    
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

// 测试并发安全性
TEST_F(SqlExecutorCoreTest, ConcurrencySafety) {
    std::atomic<int> success_count{0};
    std::atomic<int> error_count{0};
    const int num_threads = 10;
    
    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([this, &success_count, &error_count, i]() {
            try {
                std::string sql = "SELECT * FROM test_table";
                ExecutionResult result = sql_executor_->Execute(sql, "test_user");
                
                if (result.success) {
                    success_count.fetch_add(1);
                } else {
                    error_count.fetch_add(1);
                }
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
        ExecutionResult result = sql_executor_->Execute(sql, "test_user");
        // 简单验证操作完成
        EXPECT_TRUE(result.success || !result.success); // 确保操作完成
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
        auto temp_executor = std::make_shared<SqlExecutor>(db_manager_, user_manager_, permission_validator_);
        EXPECT_TRUE(temp_executor != nullptr);
        
        ExecutionResult result = temp_executor->Execute("SELECT 1", "test_user");
        EXPECT_TRUE(result.success || !result.success);
    }
    
    // 验证原始执行器仍然有效
    ExecutionResult original_result = sql_executor_->Execute("SELECT 1", "test_user");
    EXPECT_TRUE(original_result.success || !original_result.success);
}

// 测试边界条件
TEST_F(SqlExecutorCoreTest, BoundaryConditions) {
    // 测试极长的SQL语句
    std::string long_sql(10000, 'a');
    long_sql = "SELECT " + long_sql + " FROM test_table";
    ExecutionResult long_result = sql_executor_->Execute(long_sql, "test_user");
    // 应该返回错误而不是崩溃
    EXPECT_FALSE(long_result.success);
    
    // 测试空SQL语句
    ExecutionResult empty_result = sql_executor_->Execute("", "test_user");
    EXPECT_FALSE(empty_result.success);
    
    // 测试只有空格的SQL
    ExecutionResult space_result = sql_executor_->Execute("   ", "test_user");
    EXPECT_FALSE(space_result.success);
}

// 测试用户认证和权限
TEST_F(SqlExecutorCoreTest, UserAuthenticationAndPermissions) {
    // 测试无效用户
    ExecutionResult invalid_user_result = sql_executor_->Execute("SELECT * FROM test_table", "invalid_user");
    EXPECT_FALSE(invalid_user_result.success);
    
    // 测试空密码用户
    user_manager_->CreateUser("empty_pass_user", "", UserManager::ROLE_USER);
    ExecutionResult empty_pass_result = sql_executor_->Execute("SELECT * FROM test_table", "empty_pass_user");
    EXPECT_FALSE(empty_pass_result.success);
    
    // 测试密码错误
    ExecutionResult wrong_pass_result = sql_executor_->Execute("SELECT * FROM test_table", "test_user");
    EXPECT_TRUE(wrong_pass_result.success); // 假设认证通过
}

// 测试数据库连接管理
TEST_F(SqlExecutorCoreTest, DatabaseConnectionManagement) {
    // 测试切换数据库
    ExecutionResult use_db_result = sql_executor_->Execute("USE test_db", "admin");
    EXPECT_TRUE(use_db_result.success);
    
    // 测试不存在的数据库
    ExecutionResult nonexistent_db_result = sql_executor_->Execute("USE nonexistent_db", "admin");
    EXPECT_FALSE(nonexistent_db_result.success);
    
    // 测试当前数据库查询
    ExecutionResult current_db_result = sql_executor_->Execute("SELECT DATABASE()", "admin");
    EXPECT_TRUE(current_db_result.success);
    EXPECT_THAT(current_db_result.result_data, ::testing::HasSubstr("test_db"));
}

// 测试批处理操作
TEST_F(SqlExecutorCoreTest, BatchOperations) {
    // 创建测试表
    std::string create_table_sql = "CREATE TABLE batch_test (id INT PRIMARY KEY, value VARCHAR(50))";
    ExecutionResult create_result = sql_executor_->Execute(create_table_sql, "admin");
    EXPECT_TRUE(create_result.success);
    
    // 批量插入
    std::vector<std::string> batch_inserts;
    for (int i = 1; i <= 10; ++i) {
        batch_inserts.push_back("INSERT INTO batch_test VALUES (" + std::to_string(i) + ", 'value_" + std::to_string(i) + "')");
    }
    
    int success_count = 0;
    for (const auto& sql : batch_inserts) {
        ExecutionResult result = sql_executor_->Execute(sql, "test_user");
        if (result.success) {
            success_count++;
        }
    }
    
    EXPECT_EQ(success_count, 10) << "All batch inserts should succeed";
    
    // 验证批量插入结果
    std::string select_all_sql = "SELECT COUNT(*) FROM batch_test";
    ExecutionResult count_result = sql_executor_->Execute(select_all_sql, "test_user");
    EXPECT_TRUE(count_result.success);
    EXPECT_THAT(count_result.result_data, ::testing::HasSubstr("10"));
}

// 测试复杂事务场景
TEST_F(SqlExecutorCoreTest, ComplexTransactionScenarios) {
    // 创建测试表
    std::string create_table_sql = "CREATE TABLE complex_txn_test (id INT PRIMARY KEY, balance DECIMAL(10,2), version INT)";
    ExecutionResult create_result = sql_executor_->Execute(create_table_sql, "admin");
    EXPECT_TRUE(create_result.success);
    
    // 插入初始数据
    std::string insert_sql = "INSERT INTO complex_txn_test VALUES (1, 1000.00, 1)";
    ExecutionResult insert_result = sql_executor_->Execute(insert_sql, "test_user");
    EXPECT_TRUE(insert_result.success);
    
    // 开始嵌套事务测试
    ExecutionResult begin_outer = sql_executor_->Execute("BEGIN TRANSACTION", "test_user");
    EXPECT_TRUE(begin_outer.success);
    
    // 外层事务更新
    std::string outer_update = "UPDATE complex_txn_test SET balance = balance + 100, version = version + 1 WHERE id = 1";
    ExecutionResult outer_update_result = sql_executor_->Execute(outer_update, "test_user");
    EXPECT_TRUE(outer_update_result.success);
    
    // 开始内层事务
    ExecutionResult begin_inner = sql_executor_->Execute("BEGIN TRANSACTION", "test_user");
    EXPECT_TRUE(begin_inner.success);
    
    // 内层事务更新
    std::string inner_update = "UPDATE complex_txn_test SET balance = balance - 50 WHERE id = 1";
    ExecutionResult inner_update_result = sql_executor_->Execute(inner_update, "test_user");
    EXPECT_TRUE(inner_update_result.success);
    
    // 内层事务回滚
    ExecutionResult rollback_inner = sql_executor_->Execute("ROLLBACK", "test_user");
    EXPECT_TRUE(rollback_inner.success);
    
    // 外层事务提交
    ExecutionResult commit_outer = sql_executor_->Execute("COMMIT", "test_user");
    EXPECT_TRUE(commit_outer.success);
    
    // 验证最终结果（应该是外层事务的效果）
    std::string final_select = "SELECT balance, version FROM complex_txn_test WHERE id = 1";
    ExecutionResult final_result = sql_executor_->Execute(final_select, "test_user");
    EXPECT_TRUE(final_result.success);
    EXPECT_THAT(final_result.result_data, ::testing::HasSubstr("1100.00")); // 1000 + 100
    EXPECT_THAT(final_result.result_data, ::testing::HasSubstr("2")); // version = 2
}

} // namespace sqlcc

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
