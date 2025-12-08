#include <iostream>
#include <string>
#include "include/sql_parser/parser_new.h"
#include "include/sql_parser/lexer.h"
#include "include/sql_parser/token.h"

int main() {
    // 测试UPDATE语句
    std::string updateSql = "UPDATE test_table SET name = 'world' WHERE id = 1;";
    std::cout << "=== 测试UPDATE语句 ===" << std::endl;
    std::cout << "SQL: " << updateSql << std::endl;
    
    sqlcc::sql_parser::Lexer lexer(updateSql);
    auto lexerPtr = std::make_shared<sqlcc::sql_parser::Lexer>(lexer);
    sqlcc::sql_parser::ParserNew parser(lexerPtr);
    
    try {
        auto stmt = parser.parseStatement();
        if (stmt) {
            std::cout << "UPDATE解析: SUCCESS" << std::endl;
        } else {
            std::cout << "UPDATE解析: FAILED" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << "UPDATE解析异常: " << e.what() << std::endl;
    }
    
    std::cout << std::endl;
    
    // 测试DELETE语句
    std::string deleteSql = "DELETE FROM test_table WHERE id = 1;";
    std::cout << "=== 测试DELETE语句 ===" << std::endl;
    std::cout << "SQL: " << deleteSql << std::endl;
    
    sqlcc::sql_parser::Lexer lexer2(deleteSql);
    auto lexerPtr2 = std::make_shared<sqlcc::sql_parser::Lexer>(lexer2);
    sqlcc::sql_parser::ParserNew parser2(lexerPtr2);
    
    try {
        auto stmt = parser2.parseStatement();
        if (stmt) {
            std::cout << "DELETE解析: SUCCESS" << std::endl;
        } else {
            std::cout << "DELETE解析: FAILED" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << "DELETE解析异常: " << e.what() << std::endl;
    }
    
    return 0;
}