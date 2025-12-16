#include <iostream>
#include "sql_parser/token_new.h"

using namespace sqlcc;
using namespace sqlcc::sql_parser;

int main() {
    std::cout << "Minimal Token test...\n";
    
    try {
        // 测试Token构造函数
        std::cout << "Creating Token with keyword 'ALTER'..." << std::endl;
        Token token(Token::KEYWORD_ALTER, "ALTER", static_cast<size_t>(1), static_cast<size_t>(1));
        std::cout << "Token created successfully" << std::endl;
        std::cout << "Token lexeme: '" << token.getLexeme() << "'" << std::endl;
        std::cout << "Token type: " << static_cast<int>(token.getType()) << std::endl;
        
        // 直接返回，不测试复制构造函数
        std::cout << "Test completed successfully." << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cout << "Exception occurred: " << e.what() << std::endl;
        return 1;
    }
}