#include "sql_parser/token.h"
#include <iostream>

int main() {
    std::cout << "Token enum values:" << std::endl;
    
    // 关键字类型
    std::cout << "KEYWORD_INSERT = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_INSERT) << std::endl;
    std::cout << "KEYWORD_INTO = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_INTO) << std::endl;
    std::cout << "KEYWORD_VALUES = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_VALUES) << std::endl;
    std::cout << "KEYWORD_CREATE = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_CREATE) << std::endl;
    std::cout << "KEYWORD_TABLE = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_TABLE) << std::endl;
    std::cout << "KEYWORD_SELECT = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_SELECT) << std::endl;
    
    // 标识符和字面量类型
    std::cout << "IDENTIFIER = " << static_cast<int>(sqlcc::sql_parser::Token::IDENTIFIER) << std::endl;
    std::cout << "STRING_LITERAL = " << static_cast<int>(sqlcc::sql_parser::Token::STRING_LITERAL) << std::endl;
    std::cout << "INTEGER_LITERAL = " << static_cast<int>(sqlcc::sql_parser::Token::INTEGER_LITERAL) << std::endl;
    
    // 标点符号类型
    std::cout << "SEMICOLON = " << static_cast<int>(sqlcc::sql_parser::Token::SEMICOLON) << std::endl;
    std::cout << "LPAREN = " << static_cast<int>(sqlcc::sql_parser::Token::LPAREN) << std::endl;
    std::cout << "RPAREN = " << static_cast<int>(sqlcc::sql_parser::Token::RPAREN) << std::endl;
    std::cout << "COMMA = " << static_cast<int>(sqlcc::sql_parser::Token::COMMA) << std::endl;
    
    return 0;
}