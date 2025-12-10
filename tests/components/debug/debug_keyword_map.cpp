#include "sql_parser/lexer_new.h"
#include <iostream>

int main() {
    std::cout << "Testing keyword map initialization..." << std::endl;
    
    // 创建一个 lexer 实例来触发 keywordMap 的初始化
    sqlcc::sql_parser::LexerNew lexer("insert into users (id, name) values (1, 'Alice');");
    
    // 手动调用 createKeywordToken 来检查映射
    sqlcc::sql_parser::Token insertToken = lexer.createKeywordToken("insert", 1, 1);
    sqlcc::sql_parser::Token valuesToken = lexer.createKeywordToken("values", 1, 1);
    
    std::cout << "insert token type: " << static_cast<int>(insertToken.getType()) << std::endl;
    std::cout << "values token type: " << static_cast<int>(valuesToken.getType()) << std::endl;
    
    return 0;
}