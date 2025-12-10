#include "include/sql_parser/lexer_new.h"
#include <iostream>

using namespace sqlcc::sql_parser;

int main() {
    std::cout << "Testing minimal lexer..." << std::endl;
    
    try {
        // 创建一个简单的词法分析器实例
        LexerNew lexer("ALTER");
        std::cout << "Lexer created successfully" << std::endl;
        
        // 获取第一个token
        Token token = lexer.nextToken();
        std::cout << "Token created successfully" << std::endl;
        
        // 检查token类型而不直接打印
        if (token.getType() == Token::KEYWORD_ALTER) {
            std::cout << "Token is ALTER keyword" << std::endl;
        } else {
            std::cout << "Token is not ALTER keyword, type: " << token.getType() << std::endl;
        }
        
        std::cout << "About to access lexeme..." << std::endl;
        std::string lexeme = token.getLexeme();
        std::cout << "Token lexeme: '" << lexeme << "'" << std::endl;
        
        std::cout << "Minimal lexer test completed successfully!" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Exception occurred: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}