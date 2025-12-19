#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include "../../../include/sql_parser/parser_new.h"

using namespace sqlcc;

int main() {
    std::cout << "=== CREATE VIEW功能测试 ===" << std::endl;

    try {
        // 测试1: 简单的CREATE VIEW语句解析
        std::cout << "\n--- 测试1: 简单CREATE VIEW解析 ---" << std::endl;
        std::string sql1 = "CREATE VIEW test_view AS SELECT id, name FROM test_table;";
        std::cout << "测试SQL: " << sql1 << std::endl;

        // 解析SQL
        sql_parser::ParserNew parser(sql1);
        auto statements = parser.parse();

        if (!statements.empty()) {
            auto& parsed_stmt = statements[0];
            std::cout << "✅ 解析成功" << std::endl;
            std::cout << "语句类型: " << static_cast<int>(parsed_stmt->getType()) << std::endl;

            if (parsed_stmt->getType() == sql_parser::Statement::Type::CREATE_VIEW) {
                std::cout << "✅ 正确识别为CREATE VIEW语句" << std::endl;
            } else {
                std::cout << "❌ 语句类型不正确" << std::endl;
            }
        } else {
            std::cout << "❌ 解析失败: 没有解析到语句" << std::endl;
        }

        // 测试2: 带列名的CREATE VIEW语句解析
        std::cout << "\n--- 测试2: 带列名的CREATE VIEW解析 ---" << std::endl;
        std::string sql2 = "CREATE VIEW test_view (col1, col2) AS SELECT id, name FROM test_table;";
        std::cout << "测试SQL: " << sql2 << std::endl;

        sql_parser::ParserNew parser2(sql2);
        auto statements2 = parser2.parse();

        if (!statements2.empty()) {
            auto& parsed_stmt = statements2[0];
            std::cout << "✅ 解析成功" << std::endl;
            std::cout << "语句类型: " << static_cast<int>(parsed_stmt->getType()) << std::endl;

            if (parsed_stmt->getType() == sql_parser::Statement::Type::CREATE_VIEW) {
                std::cout << "✅ 正确识别为CREATE VIEW语句" << std::endl;
            } else {
                std::cout << "❌ 语句类型不正确" << std::endl;
            }
        } else {
            std::cout << "❌ 解析失败: 没有解析到语句" << std::endl;
        }

        // 测试3: 复杂SELECT语句的CREATE VIEW
        std::cout << "\n--- 测试3: 复杂SELECT的CREATE VIEW解析 ---" << std::endl;
        std::string sql3 = "CREATE VIEW complex_view AS SELECT t1.id, t1.name, t2.salary FROM employees t1 JOIN salaries t2 ON t1.id = t2.emp_id WHERE t1.age > 30;";
        std::cout << "测试SQL: " << sql3 << std::endl;

        sql_parser::ParserNew parser3(sql3);
        auto statements3 = parser3.parse();

        if (!statements3.empty()) {
            auto& parsed_stmt = statements3[0];
            std::cout << "✅ 解析成功" << std::endl;
            std::cout << "语句类型: " << static_cast<int>(parsed_stmt->getType()) << std::endl;

            if (parsed_stmt->getType() == sql_parser::Statement::Type::CREATE_VIEW) {
                std::cout << "✅ 正确识别为CREATE VIEW语句" << std::endl;
            } else {
                std::cout << "❌ 语句类型不正确" << std::endl;
            }
        } else {
            std::cout << "❌ 解析失败: 没有解析到语句" << std::endl;
        }

        // 测试4: 验证AST节点内容
        std::cout << "\n--- 测试4: CREATE VIEW AST验证 ---" << std::endl;
        std::string sql4 = "CREATE VIEW simple_view (col1, col2) AS SELECT id, name FROM test_table;";
        std::cout << "测试SQL: " << sql4 << std::endl;

        sql_parser::ParserNew parser4(sql4);
        auto statements4 = parser4.parse();

        if (!statements4.empty()) {
            auto& parsed_stmt = statements4[0];
            std::cout << "✅ 解析成功" << std::endl;

            // 尝试转换为CreateViewStatement
            auto* create_view_stmt = dynamic_cast<sql_parser::CreateViewStatement*>(parsed_stmt.get());
            if (create_view_stmt) {
                std::cout << "✅ 成功转换为CreateViewStatement" << std::endl;
                std::cout << "视图名: " << create_view_stmt->getViewName() << std::endl;
                std::cout << "列名数量: " << create_view_stmt->getColumnNames().size() << std::endl;
                std::cout << "是否有列名: " << (create_view_stmt->hasColumnNames() ? "是" : "否") << std::endl;

                // 显示列名
                if (create_view_stmt->hasColumnNames()) {
                    std::cout << "列名列表: ";
                    for (const auto& col : create_view_stmt->getColumnNames()) {
                        std::cout << col << " ";
                    }
                    std::cout << std::endl;
                }
            } else {
                std::cout << "❌ 无法转换为CreateViewStatement" << std::endl;
            }
        } else {
            std::cout << "❌ 解析失败: 没有解析到语句" << std::endl;
        }

    } catch (const std::exception& e) {
        std::cout << "❌ 发生异常: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "\n=== CREATE VIEW测试完成 ===" << std::endl;
    return 0;
}
