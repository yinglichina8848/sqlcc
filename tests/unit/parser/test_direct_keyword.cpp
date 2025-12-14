#include "sql_parser/lexer.h"
#include "sql_parser/token.h"
#include <iostream>

int main() {
    // 测试Token::KEYWORD_INSERT的值
    std::cout << "Token::KEYWORD_INSERT = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_INSERT) << std::endl;
    std::cout << "Token::IDENTIFIER = " << static_cast<int>(sqlcc::sql_parser::Token::IDENTIFIER) << std::endl;
    std::cout << "Token::SEMICOLON = " << static_cast<int>(sqlcc::sql_parser::Token::SEMICOLON) << std::endl;
    
    // 创建一个简单的lexer来测试
    std::string sql = "INSERT";
    sqlcc::sql_parser::Lexer lexer(sql);
    sqlcc::sql_parser::Token token = lexer.nextToken();
    
    std::cout << "Token for 'INSERT': " << token.getLexeme() 
              << " Type: " << static_cast<int>(token.getType()) << std::endl;
    
    // 检查token类型是否正确
    if (token.getType() == sqlcc::sql_parser::Token::KEYWORD_INSERT) {
        std::cout << "SUCCESS: Token type is KEYWORD_INSERT!" << std::endl;
    } else {
        std::cout << "ERROR: Token type is NOT KEYWORD_INSERT! Expected: " 
                  << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_INSERT)
                  << " Got: " << static_cast<int>(token.getType()) << std::endl;
    }
    
    return 0;
}