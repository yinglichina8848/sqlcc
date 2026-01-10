#include "sql_executor.h"
#include <iostream>
#include <vector>

int main() {
    std::cout << "=== SQLCC SqlExecutor功能测试 ===" << std::endl;
    std::cout << "" << std::endl;

    sqlcc::SqlExecutor executor;

    // 测试SQL语句列表
    std::vector<std::string> test_sqls = {
        // DDL测试
        "CREATE DATABASE testdb",
        "CREATE TABLE users (id INT PRIMARY KEY, name VARCHAR(50), email VARCHAR(100))",
        "CREATE INDEX idx_users_name ON users (name)",
        "ALTER TABLE users ADD COLUMN created_at TIMESTAMP",
        "DROP INDEX idx_users_name",
        "DROP TABLE users",
        "DROP DATABASE testdb",

        // DML测试
        "INSERT INTO users (id, name, email) VALUES (1, 'Alice', 'alice@example.com')",
        "UPDATE users SET email = 'alice.smith@example.com' WHERE id = 1",
        "DELETE FROM users WHERE id = 1",

        // DQL测试
        "SELECT * FROM users",
        "SELECT name, email FROM users WHERE id = 1",

        // DCL测试
        "CREATE USER testuser IDENTIFIED BY 'password'",
        "GRANT SELECT, INSERT ON users TO testuser",
        "REVOKE INSERT ON users FROM testuser",
        "DROP USER testuser"
    };

    int test_count = 0;
    int success_count = 0;

    for (const auto& sql : test_sqls) {
        test_count++;
        std::cout << "Test " << test_count << ": " << sql << std::endl;

        try {
            std::string result = executor.Execute(sql);
            std::cout << "✅ Result: " << result << std::endl;

            // 检查是否包含错误信息
            if (result.find("syntax error") == std::string::npos &&
                result.find("Error:") == std::string::npos) {
                success_count++;
            }
        } catch (const std::exception& e) {
            std::cout << "❌ Exception: " << e.what() << std::endl;
        }

        std::cout << "" << std::endl;
    }

    // 测试结果统计
    std::cout << "=== 测试结果统计 ===" << std::endl;
    std::cout << "总测试数: " << test_count << std::endl;
    std::cout << "成功数: " << success_count << std::endl;
    std::cout << "成功率: " << (test_count > 0 ? (success_count * 100.0 / test_count) : 0) << "%" << std::endl;

    if (success_count == test_count) {
        std::cout << "🎉 所有SQL语句类型测试通过！" << std::endl;
    } else {
        std::cout << "⚠️ 部分测试失败，需要进一步调试。" << std::endl;
    }

    return success_count == test_count ? 0 : 1;
}