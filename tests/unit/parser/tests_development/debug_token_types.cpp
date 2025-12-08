#include <iostream>
#include "include/sql_parser/token_new.h"

int main() {
    std::cout << "Token Type Values:" << std::endl;
    std::cout << "SEMICOLON: " << sqlcc::sql_parser::Token::SEMICOLON << std::endl;
    std::cout << "LPAREN: " << sqlcc::sql_parser::Token::LPAREN << std::endl;
    std::cout << "RPAREN: " << sqlcc::sql_parser::Token::RPAREN << std::endl;
    std::cout << "COMMA: " << sqlcc::sql_parser::Token::COMMA << std::endl;
    std::cout << "DOT: " << sqlcc::sql_parser::Token::DOT << std::endl;
    std::cout << "INTEGER_LITERAL: " << sqlcc::sql_parser::Token::INTEGER_LITERAL << std::endl;
    std::cout << "IDENTIFIER: " << sqlcc::sql_parser::Token::IDENTIFIER << std::endl;
    std::cout << "KEYWORD_SELECT: " << sqlcc::sql_parser::Token::KEYWORD_SELECT << std::endl;
    std::cout << "OPERATOR_MULTIPLY: " << sqlcc::sql_parser::Token::OPERATOR_MULTIPLY << std::endl;
    std::cout << "KEYWORD_FROM: " << sqlcc::sql_parser::Token::KEYWORD_FROM << std::endl;
    std::cout << "KEYWORD_WHERE: " << sqlcc::sql_parser::Token::KEYWORD_WHERE << std::endl;
    std::cout << "OPERATOR_EQUAL: " << sqlcc::sql_parser::Token::OPERATOR_EQUAL << std::endl;
    return 0;
}