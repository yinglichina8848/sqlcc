#include "sql_parser/lexer_new.h"
#include "sql_parser/token_new.h"
#include <iostream>
#include <vector>

int main() {
    std::cout << "Testing percent '%' parsing..." << std::endl;
    
    // Test single percent
    std::string input = "%";
    sqlcc::sql_parser::LexerNew lexer(input);
    
    std::cout << "Input: '" << input << "'" << std::endl;
    
    try {
        sqlcc::sql_parser::Token token = lexer.nextToken();
        
        std::cout << "Token type: " << sqlcc::sql_parser::Token::getTypeName(token.getType()) << std::endl;
        std::cout << "Token lexeme: '" << token.getLexeme() << "'" << std::endl;
        
        // Check if it's UNKNOWN (which could cause infinite loop)
        if (token.getType() == sqlcc::sql_parser::Token::UNKNOWN) {
            std::cout << "⚠️  Percent parsed as UNKNOWN - may cause infinite loop!" << std::endl;
        } else {
            std::cout << "✅ Percent parsed successfully as: " 
                      << sqlcc::sql_parser::Token::getTypeName(token.getType()) << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Exception during percent parsing: " << e.what() << std::endl;
        return 1;
    }
    
    // Test percent in context
    std::string context_input = "123%";
    std::cout << "\nContext test - Input: '" << context_input << "'" << std::endl;
    
    try {
        sqlcc::sql_parser::LexerNew lexer2(context_input);
        std::vector<sqlcc::sql_parser::Token> tokens;
        sqlcc::sql_parser::Token token = lexer2.nextToken();
        
        int token_count = 0;
        const int max_tokens = 10; // Prevent infinite loop
        
        while (token.getType() != sqlcc::sql_parser::Token::END_OF_INPUT && token_count < max_tokens) {
            tokens.push_back(token);
            std::cout << "Token " << token_count << ": " 
                      << sqlcc::sql_parser::Token::getTypeName(token.getType()) 
                      << " = '" << token.getLexeme() << "'" << std::endl;
            
            token = lexer2.nextToken();
            token_count++;
        }
        
        if (token_count >= max_tokens) {
            std::cout << "⚠️  Reached maximum token count - possible infinite loop!" << std::endl;
        } else {
            std::cout << "✅ Context test completed with " << token_count << " tokens" << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Exception during context test: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "\n🎉 Percent parsing test completed!" << std::endl;
    return 0;
}