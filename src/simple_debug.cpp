#include "include/sql_parser/token_new.h"
#include <iostream>

int main() {
    std::cout << "Testing Token creation..." << std::endl;
    
    try {
        // 测试默认构造函数
        sqlcc::sql_parser::Token token1;
        std::cout << "Default token created successfully" << std::endl;
        
        // 测试参数化构造函数
        sqlcc::sql_parser::Token token2(sqlcc::sql_parser::Token::KEYWORD_INSERT, "insert", 1, 1);
        std::cout << "Parameterized token created successfully" << std::endl;
        
        // 测试拷贝构造函数
        sqlcc::sql_parser::Token token3(token2);
        std::cout << "Copy token created successfully" << std::endl;
        
        // 测试移动构造函数
        sqlcc::sql_parser::Token token4(std::move(token2));
        std::cout << "Move token created successfully" << std::endl;
        
        std::cout << "All tests passed!" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << std::endl;
    }
    
    return 0;
}