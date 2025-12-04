#include "sql_parser/lexer_new.h"
#include <iostream>

int main() {
    std::cout << "🧪 Simple DFA Lexer Test" << std::endl;
    std::cout << "========================" << std::endl;

    try {
        // Test basic tokenization
        sqlcc::sql_parser::LexerNew lexer("SELECT id, name FROM users WHERE age > 25;");

        std::cout << "Tokenizing: SELECT id, name FROM users WHERE age > 25;" << std::endl;
        std::cout << std::endl;

        sqlcc::sql_parser::Token token = lexer.nextToken();
        int tokenCount = 0;

        while (token.getType() != sqlcc::sql_parser::Token::END_OF_INPUT) {
            tokenCount++;
            std::cout << "Token " << tokenCount << ": "
                      << sqlcc::sql_parser::Token::getTypeName(token.getType())
                      << " ('" << token.getLexeme() << "')"
                      << " at line " << token.getLine() << ", col " << token.getColumn()
                      << std::endl;

            token = lexer.nextToken();
        }

        std::cout << std::endl;
        std::cout << "✅ Successfully tokenized " << tokenCount << " tokens!" << std::endl;
        std::cout << "========================" << std::endl;
        std::cout << "🎉 DFA Lexer basic functionality test PASSED!" << std::endl;

    } catch (const std::exception& e) {
        std::cout << "========================" << std::endl;
        std::cout << "❌ Test FAILED: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
