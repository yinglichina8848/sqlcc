#include <iostream>
#include "include/sql_parser/lexer.h"

int main() {
    std::cout << "Starting debug simple test" << std::endl;
    
    std::string input = "SELECT";
    std::cout << "Input: '" << input << "', length: " << input.length() << std::endl;
    
    sqlcc::sql_parser::Lexer lexer(input);
    std::cout << "Created lexer" << std::endl;
    
    std::cout << "Before first nextToken call" << std::endl;
    auto token = lexer.nextToken();
    std::cout << "First token: '" << token.getLexeme() 
              << "' Type: " << static_cast<int>(token.getType()) << std::endl;
    
    std::cout << "Before second nextToken call" << std::endl;
    token = lexer.nextToken();
    std::cout << "Second token: '" << token.getLexeme() 
              << "' Type: " << static_cast<int>(token.getType()) << std::endl;
    
    std::cout << "Finished" << std::endl;
    return 0;
}