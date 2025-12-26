#include "sql_parser/token.h"
#include "include/sql_parser/token_new.h"
#include <iostream>

using namespace sqlcc::sql_parser;

int main() {
    std::cout << "Testing super minimal token..." << std::endl;
    
    try {
        // 创建一个简单的Token对象
        Token token(Token::KEYWORD_ALTER, "alter", 1, 1);
        std::cout << "Token created successfully" << std::endl;
        
        // 检查token类型
        if (token.getType() == Token::KEYWORD_ALTER) {
            std::cout << "Token is ALTER keyword" << std::endl;
        } else {
            std::cout << "Token is not ALTER keyword, type: " << token.getType() << std::endl;
        }
        
        std::cout << "About to access lexeme..." << std::endl;
        std::string lexeme = token.getLexeme();
        std::cout << "Token lexeme: '" << lexeme << "'" << std::endl;
        
        std::cout << "Super minimal test completed successfully!" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Exception occurred: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}