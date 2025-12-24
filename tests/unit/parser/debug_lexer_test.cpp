#include "sql_parser/lexer.h"
#include "sql_parser/token.h"
#include <iostream>
#include <memory>

using namespace sqlcc::sql_parser;

int main() {
    std::cout << "=== 测试Lexer对GRANT和REVOKE关键字的识别 ===" << std::endl;
    
    // 测试GRANT语句
    std::string grantSql = "GRANT ALL PRIVILEGES ON TABLE users TO testuser";
    std::cout << "测试SQL: " << grantSql << std::endl;
    
    Lexer grantLexer(grantSql);
    Token token;
    do {
        token = grantLexer.nextToken();
        std::cout << "Token类型: " << Token::getTypeName(token.getType()) 
                  << ", 值: " << token.getLexeme() << std::endl;
    } while (token.getType() != Token::END_OF_INPUT);
    
    std::cout << "\n=== 分隔线 ===\n" << std::endl;
    
    // 测试REVOKE语句
    std::string revokeSql = "REVOKE ALL PRIVILEGES ON TABLE users FROM testuser";
    std::cout << "测试SQL: " << revokeSql << std::endl;
    
    Lexer revokeLexer(revokeSql);
    do {
        token = revokeLexer.nextToken();
        std::cout << "Token类型: " << Token::getTypeName(token.getType()) 
                  << ", 值: " << token.getLexeme() << std::endl;
    } while (token.getType() != Token::END_OF_INPUT);
    
    return 0;
}