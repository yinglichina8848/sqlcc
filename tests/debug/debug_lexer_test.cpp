#include "sql_parser/lexer.h"
#include "sql_parser/token.h"
#include <iostream>
#include <memory>

using namespace sqlcc::sql_parser;

int main() {
    try {
        // 测试INSERT INTO VALUES关键字识别
        std::string sql = "INSERT INTO users (id, name, email) VALUES (1, 'John Doe', 'john@example.com');";
        
        std::cout << "Testing lexer with SQL: " << sql << std::endl;
        
        Lexer lexer(sql);
        Token token = lexer.nextToken();
        
        int tokenCount = 0;
        while (token.getType() != Token::END_OF_INPUT && tokenCount < 20) {
            std::cout << "Token " << tokenCount << ": " << token.getLexeme() 
                      << " (Type: " << token.getType() << ")" << std::endl;
            token = lexer.nextToken();
            tokenCount++;
        }
        
        std::cout << "Lexer test completed." << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}