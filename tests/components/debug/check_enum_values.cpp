#include "sql_parser/token.h"
#include <iostream>

int main() {
    std::cout << "Checking Token enum values:" << std::endl;
    
    std::cout << "KEYWORD_INSERT = " << static_cast<int>(sqlcc::sql_parser::Token::KEYWORD_INSERT) << std::endl;
    std::cout << "STRING_LITERAL = " << static_cast<int>(sqlcc::sql_parser::Token::STRING_LITERAL) << std::endl;
    std::cout << "IDENTIFIER = " << static_cast<int>(sqlcc::sql_parser::Token::IDENTIFIER) << std::endl;
    std::cout << "END_OF_INPUT = " << static_cast<int>(sqlcc::sql_parser::Token::END_OF_INPUT) << std::endl;
    std::cout << "ERROR = " << static_cast<int>(sqlcc::sql_parser::Token::ERROR) << std::endl;
    
    return 0;
}