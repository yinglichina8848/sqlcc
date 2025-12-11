#include "include/sql_parser/token_new.h"
#include <iostream>

int main() {
    std::cout << "Testing basic Token functionality..." << std::endl;
    
    try {
        // 测试创建STRING_LITERAL类型的Token
        std::cout << "Creating STRING_LITERAL token..." << std::endl;
        sqlcc::sql_parser::Token token(sqlcc::sql_parser::Token::STRING_LITERAL, "test", 1, 1);
        
        std::cout << "Token created successfully!" << std::endl;
        
        // 测试获取类型
        std::cout << "Getting token type..." << std::endl;
        sqlcc::sql_parser::Token::Type type = token.getType();
        std::cout << "Token type: " << static_cast<int>(type) << std::endl;
        
        // 测试获取lexeme
        std::cout << "Getting token lexeme..." << std::endl;
        std::string lexeme = token.getLexeme();
        std::cout << "Token lexeme: " << lexeme << std::endl;
        
        std::cout << "All basic tests passed!" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}