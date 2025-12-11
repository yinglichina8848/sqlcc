#include "include/sql_parser/lexer_new.h"
#include "include/sql_parser/token_new.h"
#include <iostream>

int main() {
    std::cout << "Testing lexer string handling..." << std::endl;
    
    try {
        // 测试包含单引号的字符串，模拟词法分析器的行为
        std::string test_input = "'Alice'";
        std::cout << "Input: " << test_input << std::endl;
        
        sqlcc::sql_parser::LexerNew lexer(test_input);
        
        // 直接调用createToken方法模拟词法分析器的行为
        std::cout << "Calling createToken with STRING_SINGLE state..." << std::endl;
        sqlcc::sql_parser::Token token = lexer.createToken(
            sqlcc::sql_parser::LexerState::STRING_SINGLE, 
            test_input, 
            1, 
            1
        );
        
        std::cout << "Token created successfully!" << std::endl;
        std::cout << "Token type: " << static_cast<int>(token.getType()) << std::endl;
        std::cout << "Token lexeme: " << token.getLexeme() << std::endl;
        
        std::cout << "Lexer string test passed!" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}