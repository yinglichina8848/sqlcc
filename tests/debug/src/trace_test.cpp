#include <iostream>
#include "include/sql_parser/lexer.h"

// 重载<<操作符以便打印LexerState
std::ostream& operator<<(std::ostream& os, const sqlcc::sql_parser::LexerState& state) {
    // 简单的打印函数
    os << static_cast<int>(state);
    return os;
}

int main() {
    std::cout << "Starting trace test" << std::endl;
    
    std::string input = "SELECT";
    std::cout << "Input: '" << input << "'" << std::endl;
    
    std::cout << "About to create lexer" << std::endl;
    sqlcc::sql_parser::Lexer lexer(input);
    std::cout << "Lexer created successfully" << std::endl;
    
    std::cout << "About to call nextToken()" << std::endl;
    auto token = lexer.nextToken();
    std::cout << "nextToken() returned" << std::endl;
    std::cout << "Token: '" << token.getLexeme() 
              << "' Type: " << static_cast<int>(token.getType()) << std::endl;
    
    std::cout << "Test completed" << std::endl;
    return 0;
}