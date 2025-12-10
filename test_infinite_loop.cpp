#include <iostream>
#include "include/sql_parser/lexer_new.h"
#include "include/sql_parser/token_new.h"

using namespace sqlcc::sql_parser;

void testLexer(const std::string& input) {
    std::cout << "=== 测试输入: '" << input << "' ===" << std::endl;
    
    try {
        LexerNew lexer(input);
        int tokenCount = 0;
        const int maxTokens = 100; // 防止无限循环
        
        while (tokenCount < maxTokens) {
            Token token = lexer.nextToken();
            std::cout << "Token " << tokenCount << ": '" << token.getLexeme() 
                      << "' 类型: " << Token::getTypeName(token.getType()) 
                      << " (" << token.getType() << ")" << std::endl;
            
            tokenCount++;
            
            if (token.getType() == Token::END_OF_INPUT) {
                std::cout << "已到达输入末尾" << std::endl;
                break;
            }
        }
        
        if (tokenCount >= maxTokens) {
            std::cout << "警告: 可能存在无限循环，已达到最大token数限制" << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "异常: " << e.what() << std::endl;
    }
    
    std::cout << std::endl;
}

int main() {
    // 测试各种可能导致无限循环的情况
    testLexer("%");
    testLexer("123%");
    testLexer("SELECT id % 2 FROM users;");
    testLexer("INSERT INTO test (id) VALUES (1%);");
    testLexer(";;;");
    testLexer("...");
    testLexer("abc def");
    testLexer("");
    
    return 0;
}