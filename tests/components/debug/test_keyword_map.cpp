#include "sql_parser/lexer.h"
#include <iostream>

int main() {
    std::cout << "=== Testing Keyword Map ===" << std::endl;
    
    // 创建一个lexer实例来触发keywordMap的初始化
    sqlcc::sql_parser::LexerNew lexer("insert into test");
    
    // 通过nextToken来测试关键字映射
    sqlcc::sql_parser::Token token = lexer.nextToken();
    
    std::cout << "Token type: " << static_cast<int>(token.getType()) << std::endl;
    std::cout << "Token lexeme: " << token.getLexeme() << std::endl;
    
    return 0;
}