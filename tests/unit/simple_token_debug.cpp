#include "sql_parser/token.h"
#include <iostream>
#include "sql_parser/token_new.h"

using namespace sqlcc;
using namespace sqlcc::sql_parser;

int main() {
    std::cout << "Simple Token debugging...\n";
    
    try {
        // 测试Token构造函数
        std::cout << "Creating Token with keyword 'ALTER'..." << std::endl;
        Token token(Token::KEYWORD_ALTER, "ALTER", static_cast<size_t>(1), static_cast<size_t>(1));
        std::cout << "Token created successfully" << std::endl;
        std::cout << "Token lexeme: '" << token.getLexeme() << "'" << std::endl;
        std::cout << "Token type: " << static_cast<int>(token.getType()) << std::endl;
        std::cout << "Token line: " << token.getLine() << std::endl;
        std::cout << "Token column: " << token.getColumn() << std::endl;
        
        // 测试复制构造
        std::cout << "Testing copy constructor..." << std::endl;
        Token copy_token = token;
        std::cout << "Copy token lexeme: '" << copy_token.getLexeme() << "'" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "Exception occurred: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "Simple Token debug completed." << std::endl;
    return 0;
}