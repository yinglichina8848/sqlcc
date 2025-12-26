#include "sql_parser/token.h"
#include <iostream>
#include "sql_parser/lexer.h"

int main() {
    std::string input = "SELECT * FROM users WHERE id = 1;";
    sqlcc::sql_parser::LexerNew lexer(input);
    
    std::cout << "Parsing input: " << input << std::endl;
    
    int token_count = 0;
    while (!lexer.isAtEnd()) {
        auto token = lexer.nextToken();
        std::cout << "Token " << token_count << ": type=" << token.getType() 
                  << ", lexeme='" << token.getLexeme() << "'" << std::endl;
        token_count++;
        if (token.getType() == sqlcc::sql_parser::Token::END_OF_INPUT)
            break;
    }
    
    return 0;
}