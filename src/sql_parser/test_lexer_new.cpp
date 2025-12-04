#include <iostream>
#include "sql_parser/lexer_new.h"

using namespace sqlcc::sql_parser;

void printTokens(const std::string& input) {
    std::cout << "Input: \"" << input << "\"" << std::endl;
    std::cout << "Tokens:" << std::endl;
    
    try {
        Lexer lexer(input);
        Token token = lexer.nextToken();
        
        while (token.getType() != Token::END_OF_INPUT) {
            std::cout << "  " << Token::getTypeName(token.getType()) 
                      << " \"" << token.getLexeme() << "\""
                      << " at line " << token.getLine() 
                      << ", column " << token.getColumn() << std::endl;
            token = lexer.nextToken();
        }
    } catch (const LexicalError& e) {
        std::cout << "Error: " << e.what() 
                  << " at line " << e.getLine() 
                  << ", column " << e.getColumn() << std::endl;
    }
    
    std::cout << std::endl;
}

int main() {
    // Test basic keywords
    printTokens("SELECT FROM WHERE");
    
    // Test identifiers
    printTokens("user_name table123 _private");
    
    // Test numbers
    printTokens("123 45.67");
    
    // Test strings
    printTokens("'hello' \"world\"");
    
    // Test operators
    printTokens("+ - * / = != < <= > >=");
    
    // Test punctuation
    printTokens("( ) , . ;");
    
    // Test simple SQL statement
    printTokens("SELECT id, name FROM users WHERE age > 18");
    
    // Test function calls
    printTokens("COUNT(DISTINCT id)");
    
    // Test comments
    printTokens("SELECT id -- This is a comment\nFROM users");
    
    printTokens("SELECT id /* This is a block comment */ FROM users");
    
    // Test error cases
    printTokens("SELECT id FROM users WHERE age >");
    
    // Test invalid characters
    printTokens("SELECT id @#$ FROM users");
    
    // Test incomplete string
    printTokens("SELECT id FROM users WHERE name = 'unclosed");
    
    return 0;
}