#ifndef SQLCC_SQL_PARSER_LEXER_H
#define SQLCC_SQL_PARSER_LEXER_H

#include "token.h"
#include <string>
#include <unordered_map>

namespace sqlcc {
namespace sql_parser {

// DFA State definitions
enum class LexerState {
  START,              // Initial state
  IDENTIFIER,         // Identifier state
  NUMBER,             // Number literal state
  NUMBER_DECIMAL,     // Decimal part of number
  NUMBER_EXPONENT,    // Exponent part of number
  STRING_SINGLE,      // Single-quoted string
  STRING_DOUBLE,      // Double-quoted identifier
  STRING_ESCAPE,      // Escape sequence in string
  COMMENT_LINE,       // Single-line comment (--)
  COMMENT_BLOCK,      // Multi-line comment (/* */)
  COMMENT_BLOCK_STAR, // In block comment, saw *
  OPERATOR,           // Operator state
  PUNCTUATION,        // Punctuation state
  ERROR               // Error state
};

class Lexer {
public:
    // Constructor
    Lexer(const std::string& input);

    // Destructor
    ~Lexer() = default;

    // Main tokenization method
    Token nextToken();

    // Utility methods
    bool isAtEnd() const;
    char peek() const;
    char peekNext() const;

    // Position tracking
    size_t getLine() const { return line_; }
    size_t getColumn() const { return column_; }

private:
    // DFA state transition table
    std::unordered_map<LexerState, std::unordered_map<char, LexerState>> transitions_;

    // Input string and position
    std::string input_;
    size_t position_;
    size_t line_;
    size_t column_;
    LexerState current_state_;

    // Helper methods
    void setupTransitionTable();
    char advance();

    // Token creation methods
    Token createToken(LexerState state, const std::string& lexeme, int line, int column);
    Token createIdentifierToken(const std::string& lexeme, int line, int column);
    Token createNumberToken(const std::string& lexeme, int line, int column);
    Token createStringToken(const std::string& lexeme, int line, int column);
    Token createOperatorToken(const std::string& lexeme, int line, int column);
    Token createPunctuationToken(const std::string& lexeme, int line, int column);
    Token createKeywordToken(const std::string& lexeme);

    // Comment handling
    void handleLineComment();
    void handleBlockComment();

    // Error handling
    void reportError(const std::string& message);
};

} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_LEXER_H