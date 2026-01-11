#include "sql_executor.h"
#include "unified_executor.h"
#include "database_manager.h"
#include "system_database.h"
#include "user_manager.h"
#include <gtest/gtest.h>
#include <filesystem>

namespace sqlcc {

// 错误处理和边界情况测试类
class ErrorHandlingTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建测试数据目录
        test_data_dir_ = "./test_error_" + std::to_string(std::time(nullptr));
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

    // 执行SQL并验证结果（用于错误测试）
    void ExecuteAndExpectError(const std::string& sql, const std::string& expected_error = "错误") {
        std::string result = sql_executor_->Execute(sql);

        // 检查是否包含预期错误关键词
        EXPECT_TRUE(result.find(expected_error) != std::string::npos ||
                   !sql_executor_->GetLastError().empty());

        std::cout << "SQL: " << sql << std::endl;
        std::cout << "结果: " << result << std::endl;
        std::string error = sql_executor_->GetLastError();
        if (!error.empty()) {
            std::cout << "错误: " << error << std::endl;
        }
    }

    std::string test_data_dir_;
    std::shared_ptr<DatabaseManager> db_manager_;
    std::shared_ptr<UserManager> user_manager_;
    std::shared_ptr<SystemDatabase> system_db_;
    std::shared_ptr<UnifiedExecutor> unified_executor_;
    std::unique_ptr<SqlExecutor> sql_executor_;
};

// 测试语法错误
TEST_F(ErrorHandlingTest, SyntaxErrorsTest) {
    std::cout << "\n=== 语法错误测试开始 ===" << std::endl;

    // 无效的SQL语句
    ExecuteAndExpectError("INVALID SQL STATEMENT");
    ExecuteAndExpectError("SELECT FROM");
    ExecuteAndExpectError("CREATE TABLE (id INT)");
    ExecuteAndExpectError("INSERT INTO VALUES (1)");
    ExecuteAndExpectError("UPDATE SET name='test'");

    std::cout << "=== 语法错误测试完成 ===" << std::endl;
}

// 测试语义错误
TEST_F(ErrorHandlingTest, SemanticErrorsTest) {
    std::cout << "\n=== 语义错误测试开始 ===" << std::endl;

    // 引用不存在的表
    ExecuteAndExpectError("SELECT * FROM non_existent_table");
    ExecuteAndExpectError("INSERT INTO non_existent_table VALUES (1)");
    ExecuteAndExpectError("UPDATE non_existent_table SET col=1");
    ExecuteAndExpectError("DELETE FROM non_existent_table");

    // 引用不存在的列
    // 首先创建表
    sql_executor_->Execute("CREATE DATABASE semantic_test");
    sql_executor_->Execute("USE semantic_test");
    sql_executor_->Execute("CREATE TABLE test_table (id INTEGER, name VARCHAR(50))");

    // 然后测试不存在的列
    ExecuteAndExpectError("SELECT nonexistent_column FROM test_table");
    ExecuteAndExpectError("UPDATE test_table SET nonexistent_column = 'value'");

    // 清理
    sql_executor_->Execute("DROP TABLE test_table");
    sql_executor_->Execute("DROP DATABASE semantic_test");

    std::cout << "=== 语义错误测试完成 ===" << std::endl;
}

// 测试权限错误
TEST_F(ErrorHandlingTest, PermissionErrorsTest) {
    std::cout << "\n=== 权限错误测试开始 ===" << std::endl;

    // 权限相关错误（如果系统支持权限控制）
    ExecuteAndExpectError("GRANT INVALID_PERMISSION ON table TO user");
    ExecuteAndExpectError("REVOKE FROM non_existent_user");
    ExecuteAndExpectError("GRANT SELECT ON nonexistent_db.table TO user");

    std::cout << "=== 权限错误测试完成 ===" << std::endl;
}

// 测试边界情况
TEST_F(ErrorHandlingTest, BoundaryCasesTest) {
    std::cout << "\n=== 边界情况测试开始 ===" << std::endl;

    // 空SQL
    ExecuteAndExpectError("");

    // 只有空白字符
    ExecuteAndExpectError("   \t\n   ");

    // 只有注释
    ExecuteAndExpectError("-- 只有注释");
    ExecuteAndExpectError("/* 只有注释 */");

    // 超长SQL（创建非常长的表名或列名）
    std::string long_name = "very_long_table_name_" + std::string(200, 'x');
    std::string long_sql = "CREATE TABLE " + long_name + " (id INT)";
    ExecuteAndExpectError(long_sql);

    // 特殊字符
    ExecuteAndExpectError("SELECT * FROM table; DROP TABLE users; --");

    std::cout << "=== 边界情况测试完成 ===" << std::endl;
}

// 测试数据类型错误
TEST_F(ErrorHandlingTest, DataTypeErrorsTest) {
    std::cout << "\n=== 数据类型错误测试开始 ===" << std::endl;

    sql_executor_->Execute("CREATE DATABASE datatype_error_test");
    sql_executor_->Execute("USE datatype_error_test");
    sql_executor_->Execute("CREATE TABLE test_table (id INTEGER, number_col INTEGER, text_col VARCHAR(50))");

    // 类型不匹配的插入
    ExecuteAndExpectError("INSERT INTO test_table VALUES ('not_a_number', 'text')");
    ExecuteAndExpectError("UPDATE test_table SET number_col = 'not_a_number'");

    // 超出范围的值
    ExecuteAndExpectError("INSERT INTO test_table VALUES (999999999999999999, 'big_number')");

    // 清理
    sql_executor_->Execute("DROP TABLE test_table");
    sql_executor_->Execute("DROP DATABASE datatype_error_test");

    std::cout << "=== 数据类型错误测试完成 ===" << std::endl;
}

// 测试约束违反
TEST_F(ErrorHandlingTest, ConstraintViolationTest) {
    std::cout << "\n=== 约束违反测试开始 ===" << std::endl;

    sql_executor_->Execute("CREATE DATABASE constraint_test");
    sql_executor_->Execute("USE constraint_test");
    sql_executor_->Execute("CREATE TABLE test_table ("
                          "id INTEGER PRIMARY KEY, "
                          "unique_col VARCHAR(50) UNIQUE, "
                          "not_null_col VARCHAR(50) NOT NULL)");

    // 主键重复
    sql_executor_->Execute("INSERT INTO test_table VALUES (1, 'value1', 'notnull1')");
    ExecuteAndExpectError("INSERT INTO test_table VALUES (1, 'value2', 'notnull2')");

    // 唯一约束违反
    ExecuteAndExpectError("INSERT INTO test_table VALUES (2, 'value1', 'notnull3')");

    // 非空约束违反
    ExecuteAndExpectError("INSERT INTO test_table VALUES (3, 'value3', NULL)");

    // 外键约束（如果支持）
    sql_executor_->Execute("CREATE TABLE child_table (id INTEGER, parent_id INTEGER, "
                          "FOREIGN KEY (parent_id) REFERENCES test_table(id))");
    ExecuteAndExpectError("INSERT INTO child_table VALUES (1, 999)"); // 引用不存在的主键

    // 清理
    sql_executor_->Execute("DROP TABLE child_table");
    sql_executor_->Execute("DROP TABLE test_table");
    sql_executor_->Execute("DROP DATABASE constraint_test");

    std::cout << "=== 约束违反测试完成 ===" << std::endl;
}

// 测试资源耗尽情况
TEST_F(ErrorHandlingTest, ResourceExhaustionTest) {
    std::cout << "\n=== 资源耗尽测试开始 ===" << std::endl;

    sql_executor_->Execute("CREATE DATABASE resource_test");
    sql_executor_->Execute("USE resource_test");

    // 尝试创建过多列的表
    std::string many_columns = "CREATE TABLE wide_table (";
    for (int i = 0; i < 1000; ++i) {
        many_columns += "col" + std::to_string(i) + " VARCHAR(100)";
        if (i < 999) many_columns += ", ";
    }
    many_columns += ")";
    ExecuteAndExpectError(many_columns);

    // 尝试超长字符串
    std::string long_string(100000, 'x');
    std::string long_insert = "INSERT INTO wide_table VALUES ('" + long_string + "')";
    ExecuteAndExpectError(long_insert);

    // 清理
    sql_executor_->Execute("DROP DATABASE resource_test");

    std::cout << "=== 资源耗尽测试完成 ===" << std::endl;
}

} // namespace sqlcc

// 主函数
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}