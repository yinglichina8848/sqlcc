#include <iostream>
#include <string>
#include "sql_parser/lexer.h"
#include "sql_parser/token_new.h"

using namespace sqlcc;
using namespace sql_parser;

int main() {
    // 测试冒号解析
    std::string test_input = "159:";
    
    std::cout << "测试冒号解析..." << std::endl;
    std::cout << "输入: '" << test_input << "'" << std::endl;
    
    LexerNew lexer(test_input);
    
    int token_count = 0;
    while (!lexer.isAtEnd()) {
        Token token = lexer.nextToken();
        token_count++;
        
        std::cout << "Token " << token_count << ": " 
                  << "类型=" << token.getType() 
                  << ", 词素='" << token.getLexeme() << "'"
                  << ", 行=" << token.getLine()
                  << ", 列=" << token.getColumn() << std::endl;
        
        // 检查是否到达结束
        if (token.getType() == Token::END_OF_INPUT) {
            break;
        }
    }
    
    std::cout << "测试完成，共生成 " << token_count << " 个tokens" << std::endl;
    
    return 0;
}