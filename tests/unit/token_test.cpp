#include "sql_parser/token.h"
#include "include/sql_parser/token.h"
#include <iostream>

using namespace sqlcc::sql_parser;

int main() {
    std::cout << "Testing Token class..." << std::endl;
    
    // 测试默认构造函数
    Token defaultToken;
    std::cout << "Default token type: " << defaultToken.getType() << std::endl;
    std::cout << "Default token lexeme: '" << defaultToken.getLexeme() << "'" << std::endl;
    
    // 测试带参数的构造函数
    Token paramToken(Token::KEYWORD_ALTER, "alter", 1, 1);
    std::cout << "Param token type: " << paramToken.getType() << std::endl;
    std::cout << "Param token lexeme: '" << paramToken.getLexeme() << "'" << std::endl;
    
    // 测试getTypeName
    std::cout << "ALTER type name: " << Token::getTypeName(Token::KEYWORD_ALTER) << std::endl;
    
    std::cout << "Token test completed successfully!" << std::endl;
    return 0;
}