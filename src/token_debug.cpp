#include "include/sql_parser/token_new.h"
#include <iostream>

int main() {
    std::cout << "Testing Token creation directly..." << std::endl;
    
    try {
        // 直接创建Token对象
        std::cout << "Creating Token with KEYWORD_INSERT..." << std::endl;
        sqlcc::sql_parser::Token token(sqlcc::sql_parser::Token::KEYWORD_INSERT, "insert", 1, 1);
        
        std::cout << "Token created successfully!" << std::endl;
        std::cout << "Token type: " << static_cast<int>(token.getType()) << std::endl;
        std::cout << "Token lexeme: " << token.getLexeme() << std::endl;
        
        std::cout << "All tests passed!" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}