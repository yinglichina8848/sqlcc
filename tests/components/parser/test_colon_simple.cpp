#include "sql_parser/lexer_new.h"
#include "sql_parser/token_new.h"
#include <iostream>
#include <vector>

int main() {
    std::cout << "Testing colon ':' parsing..." << std::endl;
    
    // Test single colon
    std::string input = ":";
    sqlcc::sql_parser::LexerNew lexer(input);
    
    sqlcc::sql_parser::Token token = lexer.nextToken();
    
    std::cout << "Input: '" << input << "'" << std::endl;
    std::cout << "Token type: " << sqlcc::sql_parser::Token::getTypeName(token.getType()) << std::endl;
    std::cout << "Token lexeme: '" << token.getLexeme() << "'" << std::endl;
    std::cout << "Expected type: COLON" << std::endl;
    
    if (token.getType() == sqlcc::sql_parser::Token::COLON && token.getLexeme() == ":") {
        std::cout << "✅ Single colon test PASSED!" << std::endl;
    } else {
        std::cout << "❌ Single colon test FAILED!" << std::endl;
        return 1;
    }
    
    // Test colon in context
    std::string context_input = "159:";
    sqlcc::sql_parser::LexerNew lexer2(context_input);
    
    std::vector<sqlcc::sql_parser::Token> tokens;
    sqlcc::sql_parser::Token token2 = lexer2.nextToken();
    while (token2.getType() != sqlcc::sql_parser::Token::END_OF_INPUT) {
        tokens.push_back(token2);
        token2 = lexer2.nextToken();
    }
    
    std::cout << "\nContext test - Input: '" << context_input << "'" << std::endl;
    std::cout << "Tokens found: " << tokens.size() << std::endl;
    
    for (size_t i = 0; i < tokens.size(); i++) {
        std::cout << "Token " << i << ": " << sqlcc::sql_parser::Token::getTypeName(tokens[i].getType()) 
                  << " = '" << tokens[i].getLexeme() << "'" << std::endl;
    }
    
    if (tokens.size() == 2 && 
        tokens[0].getType() == sqlcc::sql_parser::Token::INTEGER_LITERAL &&
        tokens[1].getType() == sqlcc::sql_parser::Token::COLON) {
        std::cout << "✅ Context test PASSED!" << std::endl;
    } else {
        std::cout << "❌ Context test FAILED!" << std::endl;
        return 1;
    }
    
    std::cout << "\n🎉 All colon tests PASSED!" << std::endl;
    return 0;
}