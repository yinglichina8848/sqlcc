#include "include/sql_parser/lexer_new.h"
#include "include/sql_parser/token_new.h"
#include <iostream>

using namespace sqlcc::sql_parser;

int main() {
    std::cout << "Testing lexer with INSERT statement..." << std::endl;
    
    try {
        // 测试包含单引号的字符串，模拟测试中的INSERT语句
        std::string test_input = "INSERT INTO employees (id, name, age, salary) VALUES (1, 'Alice', 30, 50000.00);";
        std::cout << "Input: " << test_input << std::endl;
        
        LexerNew lexer(test_input);
        
        // 逐个获取token
        Token token = lexer.nextToken();
        int token_count = 0;
        
        while (token.getType() != Token::END_OF_INPUT && token_count < 20) {
            std::cout << "Token " << token_count << ": Type=" << static_cast<int>(token.getType()) 
                      << ", Lexeme='" << token.getLexeme() << "'" << std::endl;
            
            if (token.getType() == Token::STRING_LITERAL) {
                std::cout << "  STRING_LITERAL content: '" << token.getLexeme() << "'" << std::endl;
            }
            
            token = lexer.nextToken();
            token_count++;
        }
        
        std::cout << "Lexer test completed successfully!" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}