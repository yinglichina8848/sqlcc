#ifndef SQLCC_SQL_PARSER_LEXER_NEW_H
#define SQLCC_SQL_PARSER_LEXER_NEW_H

#include "token_new.h"
#include <string>
#include <unordered_map>

namespace sqlcc {
namespace sql_parser {

// Forward declaration for LexerState enum
enum class LexerState;

class LexerNew {
public:
    // Constructor
    LexerNew(const std::string& input);

    // Destructor
    ~LexerNew() = default;

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
    Token createKeywordToken(const std::string& keyword, int line, int column);
    Token createNumberToken(const std::string& lexeme, int line, int column);
    Token createOperatorToken(const std::string& lexeme, int line, int column);
    Token createPunctuationToken(const std::string& lexeme, int line, int column);

    // Comment handling
    void skipLineComment();
    void skipBlockComment();

    // Error handling
    void reportError(const std::string& message) {}
};

} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_LEXER_NEW_H
