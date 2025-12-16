#include "sql_parser/lexer.h"
#include "sql_parser/token.h"
#include <iostream>
#include <memory>

using namespace sqlcc::sql_parser;

int main() {
    std::cout << "=== 测试PRIVILEGES关键字识别 ===" << std::endl;
    
    // 单独测试PRIVILEGES关键字
    std::string sql = "PRIVILEGES";
    std::cout << "测试SQL: " << sql << std::endl;
    
    Lexer lexer(sql);
    Token token = lexer.nextToken();
    std::cout << "Token类型: " << Token::getTypeName(token.getType()) 
              << ", 值: " << token.getLexeme() << std::endl;
    
    std::cout << "\n=== 测试privileges小写 ===" << std::endl;
    
    // 测试小写privileges
    std::string sql2 = "privileges";
    std::cout << "测试SQL: " << sql2 << std::endl;
    
    Lexer lexer2(sql2);
    Token token2 = lexer2.nextToken();
    std::cout << "Token类型: " << Token::getTypeName(token2.getType()) 
              << ", 值: " << token2.getLexeme() << std::endl;
    
    std::cout << "\n=== 测试混合大小写Privileges ===" << std::endl;
    
    // 测试混合大小写
    std::string sql3 = "Privileges";
    std::cout << "测试SQL: " << sql3 << std::endl;
    
    Lexer lexer3(sql3);
    Token token3 = lexer3.nextToken();
    std::cout << "Token类型: " << Token::getTypeName(token3.getType()) 
              << ", 值: " << token3.getLexeme() << std::endl;
    
    return 0;
}