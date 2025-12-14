#include <iostream>
#include "sql_parser/lexer.h"
#include "sql_parser/parser.h"

int main() {
    // 测试包含'%'字符的SQL语句
    std::string sql = "SELECT id % 2 FROM users WHERE name LIKE '%john%';";
    
    std::cout << "测试SQL: " << sql << std::endl;
    
    try {
        // 测试lexer
        sqlcc::sql_parser::Lexer lexer(sql);
        std::cout << "Lexer初始化成功" << std::endl;
        
        // 测试token解析
        sqlcc::sql_parser::Token token;
        int tokenCount = 0;
        
        while (!lexer.isAtEnd()) {
            token = lexer.nextToken();
            std::cout << "Token " << tokenCount++ << ": '" 
                      << token.getLexeme() << "' (" 
                      << sqlcc::sql_parser::Token::getTypeName(token.getType()) 
                      << ")" << std::endl;
            
            if (token.getType() == sqlcc::sql_parser::Token::END_OF_INPUT) {
                break;
            }
        }
        
        std::cout << "Lexer测试成功，共解析了 " << tokenCount << " 个token" << std::endl;
        
        // 测试parser
        sqlcc::sql_parser::Parser parser(sql);
        std::cout << "Parser初始化成功" << std::endl;
        
        auto statements = parser.parse();
        std::cout << "Parser测试成功，解析了 " << statements.size() << " 个语句" << std::endl;
        
        std::cout << "解析成功，语句数量: " << statements.size() << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}