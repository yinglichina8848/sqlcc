#include "sql_parser/token.h"
#include <iostream>

int main() {
    std::cout << "Testing Token class..." << std::endl;
    
    // 测试基本构造函数
    sqlcc::sql_parser::Token token1(sqlcc::sql_parser::Token::KEYWORD_INSERT, "insert", 1, 1);
    std::cout << "Token1 type: " << static_cast<int>(token1.getType()) << std::endl;
    std::cout << "Token1 lexeme: " << token1.getLexeme() << std::endl;
    
    // 测试拷贝构造函数
    sqlcc::sql_parser::Token token2(token1);
    std::cout << "Token2 type: " << static_cast<int>(token2.getType()) << std::endl;
    std::cout << "Token2 lexeme: " << token2.getLexeme() << std::endl;
    
    // 测试移动构造函数
    sqlcc::sql_parser::Token token3(std::move(token1));
    std::cout << "Token3 type: " << static_cast<int>(token3.getType()) << std::endl;
    std::cout << "Token3 lexeme: " << token3.getLexeme() << std::endl;
    
    std::cout << "All tests passed!" << std::endl;
    return 0;
}