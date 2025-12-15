#include "sql_parser/lexer_new.h"
#include <iostream>

int main() {
    std::cout << "Testing keyword map initialization..." << std::endl;
    
    // 创建一个 lexer 实例来触发 keywordMap 的初始化
    sqlcc::sql_parser::Lexer lexer("insert into users (id, name) values (1, 'Alice');");
    
    // 通过nextToken来测试关键字映射
    sqlcc::sql_parser::Token token = lexer.nextToken();
    
    std::cout << "First token type: " << static_cast<int>(token.getType()) << std::endl;
    std::cout << "First token lexeme: " << token.getLexeme() << std::endl;
    
    // 继续获取更多token
    while (token.getType() != sqlcc::sql_parser::Token::Type::END_OF_INPUT) {
        token = lexer.nextToken();
        std::cout << "Token type: " << static_cast<int>(token.getType()) << ", lexeme: " << token.getLexeme() << std::endl;
        
        // 如果遇到VALUES关键字就退出循环
        if (token.getType() == sqlcc::sql_parser::Token::Type::KEYWORD_VALUES) {
            break;
        }
    }
    
    return 0;
}