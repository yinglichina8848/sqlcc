#include "sql_parser/lexer.h"
#include <iostream>

int main() {
    std::cout << "Testing createKeywordToken function:" << std::endl;
    
    // 创建一个LexerNew实例
    sqlcc::sql_parser::LexerNew lexer("INSERT INTO users (id, name) VALUES (1, 'Alice');");
    
    // 手动调用createKeywordToken函数测试
    std::cout << "\nTesting 'insert' keyword:" << std::endl;
    // 注意：由于createKeywordToken是私有函数，我们无法直接调用它
    
    // 我们可以通过nextToken间接测试
    std::cout << "\nTesting lexer.nextToken() for 'INSERT':" << std::endl;
    auto token = lexer.nextToken();
    std::cout << "Token type: " << static_cast<int>(token.getType()) << std::endl;
    std::cout << "Token lexeme: " << token.getLexeme() << std::endl;
    
    return 0;
}