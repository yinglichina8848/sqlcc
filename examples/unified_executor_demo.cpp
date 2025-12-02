#include "../include/unified_executor.h"
#include "../include/database_manager.h"
#include "../include/sql_parser/parser.h"
#include <iostream>
#include <memory>
#include <string>
#include <vector>

/**
 * 统一执行器架构演示程序
 * 展示新的策略模式执行器架构的优势
 */
int main() {
    std::cout << "=== SQLCC 统一执行器架构演示 ===\n" << std::endl;

    try {
        // 1. 创建统一执行器
        auto db_manager = std::make_shared<sqlcc::DatabaseManager>();
        auto unified_executor = std::make_shared<sqlcc::UnifiedExecutor>(db_manager);

        std::cout << "✓ 创建统一执行器（策略模式架构）" << std::endl;

        // 2. 演示DDL语句执行
        std::cout << "\n=== DDL语句执行演示 ===\n" << std::endl;

        // CREATE DATABASE
        {
            sqlcc::sql_parser::Parser parser("CREATE DATABASE test_unified;");
            auto stmt = parser.Parse();
            if (stmt) {
                auto result = unified_executor->execute(std::move(stmt));
                std::cout << "CREATE DATABASE: " << (result.success ? "成功" : "失败")
                         << " - " << result.message << std::endl;
            }
        }

        // USE DATABASE
        {
            sqlcc::sql_parser::Parser parser("USE test_unified;");
            auto stmt = parser.Parse();
            if (stmt) {
                auto result = unified_executor->execute(std::move(stmt));
                std::cout << "USE DATABASE: " << (result.success ? "成功" : "失败")
                         << " - " << result.message << std::endl;
            }
        }

        // CREATE TABLE
        {
            sqlcc::sql_parser::Parser parser("CREATE TABLE employees (id INT, name VARCHAR(50), salary INT);");
            auto stmt = parser.Parse();
            if (stmt) {
                auto result = unified_executor->execute(std::move(stmt));
                std::cout << "CREATE TABLE: " << (result.success ? "成功" : "失败")
                         << " - " << result.message << std::endl;
            }
        }

        // 3. 演示DML语句执行（包含索引优化）
        std::cout << "\n=== DML语句执行演示（带索引优化）===\n" << std::endl;

        // INSERT数据
        std::vector<std::string> insert_sqls = {
            "INSERT INTO employees VALUES (1, 'Alice', 50000);",
            "INSERT INTO employees VALUES (2, 'Bob', 55000);",
            "INSERT INTO employees VALUES (3, 'Charlie', 60000);",
            "INSERT INTO employees VALUES (4, 'David', 65000);",
            "INSERT INTO employees VALUES (5, 'Eve', 70000);"
        };

        for (const auto& sql : insert_sqls) {
            sqlcc::sql_parser::Parser parser(sql);
            auto stmt = parser.Parse();
            if (stmt) {
                auto result = unified_executor->execute(std::move(stmt));
                std::cout << "INSERT: " << (result.success ? "成功" : "失败") << std::endl;
            }
        }

        // UPDATE语句（演示索引优化）
        {
            sqlcc::sql_parser::Parser parser("UPDATE employees SET salary = 75000 WHERE id = 1;");
            auto stmt = parser.Parse();
            if (stmt) {
                auto result = unified_executor->execute(std::move(stmt));
                std::cout << "\nUPDATE (索引优化): " << (result.success ? "成功" : "失败")
                         << " - " << result.message << std::endl;

                // 显示执行上下文信息
                const auto& context = unified_executor->getLastExecutionContext();
                std::cout << "  执行计划: " << context.execution_plan << std::endl;
                std::cout << "  是否使用索引: " << (context.used_index ? "是" : "否") << std::endl;
                std::cout << "  影响记录数: " << context.records_affected << std::endl;
            }
        }

        // DELETE语句（演示索引优化）
        {
            sqlcc::sql_parser::Parser parser("DELETE FROM employees WHERE id = 5;");
            auto stmt = parser.Parse();
            if (stmt) {
                auto result = unified_executor->execute(std::move(stmt));
                std::cout << "\nDELETE (索引优化): " << (result.success ? "成功" : "失败")
                         << " - " << result.message << std::endl;

                const auto& context = unified_executor->getLastExecutionContext();
                std::cout << "  执行计划: " << context.execution_plan << std::endl;
                std::cout << "  是否使用索引: " << (context.used_index ? "是" : "否") << std::endl;
                std::cout << "  影响记录数: " << context.records_affected << std::endl;
            }
        }

        // 4. 演示工具语句执行
        std::cout << "\n=== 工具语句执行演示 ===\n" << std::endl;

        // SHOW TABLES
        {
            sqlcc::sql_parser::Parser parser("SHOW TABLES;");
            auto stmt = parser.Parse();
            if (stmt) {
                auto result = unified_executor->execute(std::move(stmt));
                std::cout << "SHOW TABLES:\n" << result.message << std::endl;
            }
        }

        // 5. 演示DCL语句执行
        std::cout << "\n=== DCL语句执行演示 ===\n" << std::endl;

        // CREATE USER
        {
            sqlcc::sql_parser::Parser parser("CREATE USER test_user IDENTIFIED BY 'password123';");
            auto stmt = parser.Parse();
            if (stmt) {
                auto result = unified_executor->execute(std::move(stmt));
                std::cout << "CREATE USER: " << (result.success ? "成功" : "失败")
                         << " - " << result.message << std::endl;
            }
        }

        // GRANT权限
        {
            sqlcc::sql_parser::Parser parser("GRANT SELECT, INSERT ON TABLE employees TO test_user;");
            auto stmt = parser.Parse();
            if (stmt) {
                auto result = unified_executor->execute(std::move(stmt));
                std::cout << "GRANT: " << (result.success ? "成功" : "失败")
                         << " - " << result.message << std::endl;
            }
        }

        // 6. 架构优势展示
        std::cout << "\n=== 统一执行器架构优势 ===\n" << std::endl;

        std::cout << "✅ 消除重复代码：" << std::endl;
        std::cout << "  - 单一权限检查入口：checkGlobalPermission()" << std::endl;
        std::cout << "  - 统一上下文验证：validateGlobalContext()" << std::endl;
        std::cout << "  - 集中化错误处理和结果格式化" << std::endl;

        std::cout << "\n✅ 策略模式解耦：" << std::endl;
        std::cout << "  - DDL执行策略：DDLExecutionStrategy" << std::endl;
        std::cout << "  - DML执行策略：DMLExecutionStrategy" << std::endl;
        std::cout << "  - DCL执行策略：DCLExecutionStrategy" << std::endl;
        std::cout << "  - 工具执行策略：UtilityExecutionStrategy" << std::endl;

        std::cout << "\n✅ 易于扩展：" << std::endl;
        std::cout << "  - 新增语句类型只需添加对应策略" << std::endl;
        std::cout << "  - 高级执行器预留JOIN、子查询、窗口函数接口" << std::endl;
        std::cout << "  - 插件化架构支持自定义执行策略" << std::endl;

        std::cout << "\n✅ 统一优化：" << std::endl;
        std::cout << "  - ExecutionContext统一管理执行状态" << std::endl;
        std::cout << "  - 索引优化信息在上下文中传递" << std::endl;
        std::cout << "  - 执行统计和性能监控接口" << std::endl;

        // 7. 清理
        db_manager->DropDatabase("test_unified");

        std::cout << "\n=== 演示完成 ===" << std::endl;
        std::cout << "统一执行器架构成功解决了执行器设计的冗余问题！" << std::endl;
        std::cout << "新的架构为复杂查询、高级JOIN、子查询和查询优化器奠定了基础。" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "演示过程中发生错误: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
