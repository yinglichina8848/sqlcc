#include "sql_parser/token.h"
#include "sql_parser/lexer.h"
#include <iostream>

int main() {
    std::cout << "=== Detailed Lexer Test ===" << std::endl;
    
    // 测试简单的INSERT语句
    std::string sql = "insert into users (id, name) values (1, 'Alice');";
    std::cout << "Input SQL: " << sql << std::endl;
    
    sqlcc::sql_parser::Lexer lexer(sql);
    
    std::cout << "\n=== Getting tokens ===" << std::endl;
    int token_count = 0;
    while (true) {
        sqlcc::sql_parser::Token token = lexer.nextToken();
        token_count++;
        
        std::cout << "Token " << token_count 
                  << ": Type=" << static_cast<int>(token.getType())
                  << ", Lexeme='" << token.getLexeme() << "'" << std::endl;
        
        if (token.getType() == sqlcc::sql_parser::Token::END_OF_INPUT) {
            break;
        }
        
        // 限制token数量以防止无限循环
        if (token_count > 20) {
            std::cout << "Too many tokens, stopping..." << std::endl;
            break;
        }
    }
    
    std::cout << "Total tokens: " << token_count << std::endl;
    std::cout << "Test completed." << std::endl;
    
    return 0;
}