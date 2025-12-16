#include "sql_parser/lexer.h"
#include <iostream>
#include <memory>

int main() {
    std::string sql = "INSERT INTO users (id, name) VALUES (1, 'Alice');";
    
    std::cout << "Creating lexer with SQL: " << sql << std::endl;
    
    sqlcc::sql_parser::Lexer lexer(sql);
    
    std::cout << "Getting tokens:" << std::endl;
    
    int token_count = 0;
    while (token_count < 20) {  // 限制token数量以避免无限循环
        auto token = lexer.nextToken();
        std::cout << "Token " << token_count << ": '" << token.getLexeme() 
                  << "' (type: " << static_cast<int>(token.getType()) << ")" << std::endl;
        
        if (token.getType() == sqlcc::sql_parser::Token::END_OF_INPUT) {
            break;
        }
        
        token_count++;
    }
    
    return 0;
}