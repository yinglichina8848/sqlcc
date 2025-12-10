#include "sql_parser/token.h"
#include <iostream>

int main() {
    std::cout << "Token::KEYWORD_INSERT = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_INSERT) << std::endl;
    std::cout << "Token::KEYWORD_VALUES = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_VALUES) << std::endl;
    std::cout << "Token::KEYWORD_INTO = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_INTO) << std::endl;
    std::cout << "Token::KEYWORD_CREATE = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_CREATE) << std::endl;
    std::cout << "Token::KEYWORD_TABLE = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_TABLE) << std::endl;
    std::cout << "Token::IDENTIFIER = " << static_cast<int>(sqlcc::sql_parser::Token::IDENTIFIER) << std::endl;
    std::cout << "Token::STRING = " << static_cast<int>(sqlcc::sql_parser::Token::STRING) << std::endl;
    std::cout << "Token::NUMBER = " << static_cast<int>(sqlcc::sql_parser::Token::NUMBER) << std::endl;
    std::cout << "Token::SEMICOLON = " << static_cast<int>(sqlcc::sql_parser::Token::SEMICOLON) << std::endl;
    std::cout << "Token::LPAREN = " << static_cast<int>(sqlcc::sql_parser::Token::LPAREN) << std::endl;
    std::cout << "Token::RPAREN = " << static_cast<int>(sqlcc::sql_parser::Token::RPAREN) << std::endl;
    std::cout << "Token::COMMA = " << static_cast<int>(sqlcc::sql_parser::Token::COMMA) << std::endl;
    
    return 0;
}