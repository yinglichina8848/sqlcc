#include <iostream>
#include "include/sql_parser/lexer.h"

int main() {
    std::cout << "Starting simple test" << std::endl;
    
    std::string input = "SELECT";
    sqlcc::sql_parser::Lexer lexer(input);
    
    std::cout << "Created lexer, input length: " << input.length() << std::endl;
    std::cout << "isAtEnd: " << lexer.isAtEnd() << std::endl;
    
    auto token = lexer.nextToken();
    std::cout << "Got token: '" << token.getLexeme() 
              << "' Type: " << static_cast<int>(token.getType()) << std::endl;
    
    std::cout << "isAtEnd after token: " << lexer.isAtEnd() << std::endl;
    
    token = lexer.nextToken();
    std::cout << "Second token: '" << token.getLexeme() 
              << "' Type: " << static_cast<int>(token.getType()) << std::endl;
    
    std::cout << "Finished" << std::endl;
    return 0;
}