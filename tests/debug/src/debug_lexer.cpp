#include "sql_parser/token.h"
#include <iostream>
#include "include/sql_parser/lexer.h"

int main() {
    std::string input = "SELECT * FROM users WHERE id = 1;";
    std::cout << "Input: " << input << std::endl;
    
    sqlcc::sql_parser::Lexer lexer(input);
    
    int tokenCount = 0;
    while (!lexer.isAtEnd() && tokenCount < 20) {  // 限制循环次数以防无限循环
        auto token = lexer.nextToken();
        std::cout << "Token " << tokenCount << ": '" << token.getLexeme() 
                  << "' Type: " << static_cast<int>(token.getType()) 
                  << " Line: " << token.getLine() 
                  << " Column: " << token.getColumn() << std::endl;
        
        if (token.getType() == sqlcc::sql_parser::Token::END_OF_INPUT) {
            std::cout << "Reached end of input" << std::endl;
            break;
        }
        
        tokenCount++;
    }
    
    std::cout << "Total tokens: " << tokenCount << std::endl;
    return 0;
}