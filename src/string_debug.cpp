#include "include/sql_parser/lexer_new.h"
#include <iostream>

int main() {
    std::cout << "Testing string handling in lexer..." << std::endl;
    
    try {
        // 测试包含单引号的字符串
        std::string test_input = "'Alice'";
        std::cout << "Input: " << test_input << std::endl;
        
        sqlcc::sql_parser::LexerNew lexer(test_input);
        sqlcc::sql_parser::Token token = lexer.nextToken();
        
        std::cout << "Token type: " << static_cast<int>(token.getType()) << std::endl;
        std::cout << "Token lexeme: " << token.getLexeme() << std::endl;
        
        std::cout << "String test passed!" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}