#include "sql_parser/token_new.h"
#include <iostream>

int main() {
    std::cout << "Token::KEYWORD_INSERT value: " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_INSERT) << std::endl;
    std::cout << "Token::KEYWORD_SELECT value: " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_SELECT) << std::endl;
    std::cout << "Token::KEYWORD_UPDATE value: " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_UPDATE) << std::endl;
    std::cout << "Token::KEYWORD_DELETE value: " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_DELETE) << std::endl;
    std::cout << "Token::KEYWORD_VALUES value: " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_VALUES) << std::endl;
    return 0;
}