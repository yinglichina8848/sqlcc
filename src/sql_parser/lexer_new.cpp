#include "sql_parser/lexer_new.h"
#include <cctype>
#include <iostream>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

namespace sqlcc {
namespace sql_parser {

// DFA State definitions
enum class LexerState {
    START,           // Initial state
    IDENTIFIER,      // Identifier state
    NUMBER,          // Number literal state
    NUMBER_DECIMAL,  // Decimal part of number
    NUMBER_EXPONENT, // Exponent part of number
    STRING_SINGLE,   // Single-quoted string
    STRING_DOUBLE,   // Double-quoted identifier
    STRING_ESCAPE,   // Escape sequence in string
    COMMENT_LINE,    // Single-line comment (--)
    COMMENT_BLOCK,   // Multi-line comment (/* */)
    COMMENT_BLOCK_STAR, // In block comment, saw *
    OPERATOR,        // Operator state
    PUNCTUATION,     // Punctuation state
    ERROR            // Error state
};

// Transition table type
using TransitionMap = std::unordered_map<LexerState, std::unordered_map<char, LexerState>>;

// Character classification helpers
bool isIdentifierStart(char c) {
    return std::isalpha(c) || c == '_' || static_cast<unsigned char>(c) > 127; // Unicode support
}

bool isIdentifierPart(char c) {
    return std::isalnum(c) || c == '_' || static_cast<unsigned char>(c) > 127; // Unicode support
}

bool isDigit(char c) {
    return std::isdigit(c);
}

bool isWhitespace(char c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

// SQL Keywords map
const std::unordered_set<std::string>& getSQLKeywords() {
    static std::unordered_set<std::string> keywords = {
        // DDL Keywords
        "create", "alter", "drop", "truncate", "rename", "comment",

        // DML Keywords
        "select", "insert", "update", "delete", "merge",

        // DCL Keywords
        "grant", "revoke", "deny",

        // TCL Keywords
        "begin", "commit", "rollback", "savepoint", "set", "transaction",

        // Data Types
        "int", "integer", "smallint", "bigint", "tinyint",
        "varchar", "char", "text", "blob", "clob",
        "decimal", "numeric", "float", "double", "real",
        "date", "time", "timestamp", "datetime", "year",
        "boolean", "bool",

        // Constraints
        "primary", "key", "foreign", "references", "unique",
        "check", "not", "null", "default", "auto_increment",

        // Query Keywords
        "from", "where", "group", "by", "having", "order",
        "limit", "offset", "distinct", "all", "as",
        "join", "inner", "left", "right", "full", "outer", "on", "using",

        // Aggregate Functions
        "count", "sum", "avg", "min", "max", "group_concat",

        // Logical Operators
        "and", "or", "in", "exists", "between", "like", "is",

        // Set Operations
        "union", "intersect", "except",

        // Control Flow
        "case", "when", "then", "else", "end",
        "if", "while", "for", "do",

        // Boolean Values
        "true", "false",

        // Database Objects
        "database", "schema", "table", "view", "index",
        "trigger", "procedure", "function",

        // Permission Keywords
        "with", "password", "user", "identified", "privileges",
        "show", "columns", "indexes", "grants", "databases", "tables",

        // Miscellaneous
        "use", "into", "values", "set"
    };
    return keywords;
}

LexerNew::LexerNew(const std::string& input)
    : input_(input), position_(0), line_(1), column_(1), current_state_(LexerState::START) {
    setupTransitionTable();
}

void LexerNew::setupTransitionTable() {
    // Initialize transition table for DFA
    transitions_[LexerState::START] = {
        {' ', LexerState::START}, {'\t', LexerState::START},
        {'\r', LexerState::START}, {'\n', LexerState::START},
        {'-', LexerState::OPERATOR}, {'+', LexerState::OPERATOR},
        {'*', LexerState::OPERATOR}, {'/', LexerState::OPERATOR},
        {'=', LexerState::OPERATOR}, {'!', LexerState::OPERATOR},
        {'<', LexerState::OPERATOR}, {'>', LexerState::OPERATOR},
        {'(', LexerState::PUNCTUATION}, {')', LexerState::PUNCTUATION},
        {',', LexerState::PUNCTUATION}, {';', LexerState::PUNCTUATION},
        {'.', LexerState::PUNCTUATION}, {'\'', LexerState::STRING_SINGLE},
        {'"', LexerState::STRING_DOUBLE}
    };

    // Add transitions for identifiers and numbers
    for (char c = 'a'; c <= 'z'; ++c) {
        transitions_[LexerState::START][c] = LexerState::IDENTIFIER;
        transitions_[LexerState::START][c - 32] = LexerState::IDENTIFIER; // uppercase
    }
    transitions_[LexerState::START]['_'] = LexerState::IDENTIFIER;

    // Add Unicode support for identifier start
    for (unsigned char c = 128; c <= 255; ++c) {
        transitions_[LexerState::START][static_cast<char>(c)] = LexerState::IDENTIFIER;
    }

    // Number transitions
    for (char c = '0'; c <= '9'; ++c) {
        transitions_[LexerState::START][c] = LexerState::NUMBER;
    }

    // Identifier continuation
    transitions_[LexerState::IDENTIFIER] = std::unordered_map<char, LexerState>();
    for (char c = 'a'; c <= 'z'; ++c) {
        transitions_[LexerState::IDENTIFIER][c] = LexerState::IDENTIFIER;
        transitions_[LexerState::IDENTIFIER][c - 32] = LexerState::IDENTIFIER;
    }
    for (char c = '0'; c <= '9'; ++c) {
        transitions_[LexerState::IDENTIFIER][c] = LexerState::IDENTIFIER;
    }
    transitions_[LexerState::IDENTIFIER]['_'] = LexerState::IDENTIFIER;

    // Add Unicode support for identifier continuation
    for (unsigned char c = 128; c <= 255; ++c) {
        transitions_[LexerState::IDENTIFIER][static_cast<char>(c)] = LexerState::IDENTIFIER;
    }

    // Number transitions
    transitions_[LexerState::NUMBER] = std::unordered_map<char, LexerState>();
    for (char c = '0'; c <= '9'; ++c) {
        transitions_[LexerState::NUMBER][c] = LexerState::NUMBER;
    }
    transitions_[LexerState::NUMBER]['.'] = LexerState::NUMBER_DECIMAL;
    transitions_[LexerState::NUMBER]['e'] = LexerState::NUMBER_EXPONENT;
    transitions_[LexerState::NUMBER]['E'] = LexerState::NUMBER_EXPONENT;

    transitions_[LexerState::NUMBER_DECIMAL] = std::unordered_map<char, LexerState>();
    for (char c = '0'; c <= '9'; ++c) {
        transitions_[LexerState::NUMBER_DECIMAL][c] = LexerState::NUMBER_DECIMAL;
    }
    transitions_[LexerState::NUMBER_DECIMAL]['e'] = LexerState::NUMBER_EXPONENT;
    transitions_[LexerState::NUMBER_DECIMAL]['E'] = LexerState::NUMBER_EXPONENT;

    transitions_[LexerState::NUMBER_EXPONENT] = std::unordered_map<char, LexerState>();
    for (char c = '0'; c <= '9'; ++c) {
        transitions_[LexerState::NUMBER_EXPONENT][c] = LexerState::NUMBER_EXPONENT;
    }
    transitions_[LexerState::NUMBER_EXPONENT]['+'] = LexerState::NUMBER_EXPONENT;
    transitions_[LexerState::NUMBER_EXPONENT]['-'] = LexerState::NUMBER_EXPONENT;

    // String transitions
    transitions_[LexerState::STRING_SINGLE] = std::unordered_map<char, LexerState>();
    transitions_[LexerState::STRING_SINGLE]['\\'] = LexerState::STRING_ESCAPE;
    transitions_[LexerState::STRING_SINGLE]['\''] = LexerState::START; // End of string

    transitions_[LexerState::STRING_DOUBLE] = std::unordered_map<char, LexerState>();
    transitions_[LexerState::STRING_DOUBLE]['"'] = LexerState::START; // End of identifier

    transitions_[LexerState::STRING_ESCAPE] = std::unordered_map<char, LexerState>();
    transitions_[LexerState::STRING_ESCAPE]['\''] = LexerState::STRING_SINGLE;
    transitions_[LexerState::STRING_ESCAPE]['"'] = LexerState::STRING_SINGLE;
    transitions_[LexerState::STRING_ESCAPE]['\\'] = LexerState::STRING_SINGLE;
    transitions_[LexerState::STRING_ESCAPE]['n'] = LexerState::STRING_SINGLE;
    transitions_[LexerState::STRING_ESCAPE]['t'] = LexerState::STRING_SINGLE;
    transitions_[LexerState::STRING_ESCAPE]['r'] = LexerState::STRING_SINGLE;
}

bool LexerNew::isAtEnd() const {
    return position_ >= input_.length();
}

char LexerNew::advance() {
    if (isAtEnd()) return '\0';
    char ch = input_[position_++];
    if (ch == '\n') {
        line_++;
        column_ = 1;
    } else {
        column_++;
    }
    return ch;
}

char LexerNew::peek() const {
    if (isAtEnd()) return '\0';
    return input_[position_];
}

char LexerNew::peekNext() const {
    if (position_ + 1 >= input_.length()) return '\0';
    return input_[position_ + 1];
}

Token LexerNew::nextToken() {
    while (!isAtEnd()) {
        char ch = peek();

        // Handle comments first
        if (ch == '-' && peekNext() == '-') {
            skipLineComment();
            continue;
        }
        if (ch == '/' && peekNext() == '*') {
            skipBlockComment();
            continue;
        }

        // Skip whitespace
        if (isWhitespace(ch)) {
            advance();
            continue;
        }

        // Reset to start state
        current_state_ = LexerState::START;
        int start_pos = position_;
        int start_line = line_;
        int start_column = column_;

        // DFA processing
        while (!isAtEnd()) {
            ch = peek();
            auto state_it = transitions_.find(current_state_);
            if (state_it == transitions_.end()) {
                // No transitions from this state, token complete
                break;
            }

            auto char_it = state_it->second.find(ch);
            if (char_it == state_it->second.end()) {
                // No transition for this character, token complete
                break;
            }

            // Transition to new state
            current_state_ = char_it->second;
            advance();
        }

        // Create token based on final state
        std::string lexeme = input_.substr(start_pos, position_ - start_pos);
        return createToken(current_state_, lexeme, start_line, start_column);
    }

    return Token(Token::END_OF_INPUT, "", line_, column_);
}

Token LexerNew::createToken(LexerState state, const std::string& lexeme, int line, int column) {
    switch (state) {
        case LexerState::IDENTIFIER:
            return createIdentifierToken(lexeme, line, column);

        case LexerState::NUMBER:
        case LexerState::NUMBER_DECIMAL:
        case LexerState::NUMBER_EXPONENT:
            return createNumberToken(lexeme, line, column);

        case LexerState::STRING_SINGLE:
            // Remove quotes and return string literal
            return Token(Token::STRING_LITERAL, lexeme.substr(1, lexeme.length() - 2), line, column);

        case LexerState::STRING_DOUBLE:
            // Remove quotes and return identifier
            return Token(Token::IDENTIFIER, lexeme.substr(1, lexeme.length() - 2), line, column);

        case LexerState::OPERATOR:
            return createOperatorToken(lexeme, line, column);

        case LexerState::PUNCTUATION:
            return createPunctuationToken(lexeme, line, column);

        default:
            return Token(Token::UNKNOWN, lexeme, line, column);
    }
}

Token LexerNew::createIdentifierToken(const std::string& lexeme, int line, int column) {
    // Convert to lowercase for keyword comparison
    std::string lower_lexeme = lexeme;
    for (char& c : lower_lexeme) {
        c = std::tolower(c);
    }

    const auto& keywords = getSQLKeywords();
    if (keywords.find(lower_lexeme) != keywords.end()) {
        return createKeywordToken(lower_lexeme, line, column);
    }

    return Token(Token::IDENTIFIER, lexeme, line, column);
}

Token LexerNew::createKeywordToken(const std::string& keyword, int line, int column) {
    static std::unordered_map<std::string, Token::Type> keywordMap;

    // Initialize on first use
    if (keywordMap.empty()) {
        // DDL Keywords
        keywordMap["create"] = Token::KEYWORD_CREATE;
        keywordMap["alter"] = Token::KEYWORD_ALTER;
        keywordMap["drop"] = Token::KEYWORD_DROP;
        keywordMap["table"] = Token::KEYWORD_TABLE;
        keywordMap["index"] = Token::KEYWORD_INDEX;
        keywordMap["view"] = Token::KEYWORD_VIEW;
        keywordMap["database"] = Token::KEYWORD_DATABASE;

        // DML Keywords
        keywordMap["select"] = Token::KEYWORD_SELECT;
        keywordMap["insert"] = Token::KEYWORD_INSERT;
        keywordMap["update"] = Token::KEYWORD_UPDATE;
        keywordMap["delete"] = Token::KEYWORD_DELETE;
        keywordMap["from"] = Token::KEYWORD_FROM;
        keywordMap["into"] = Token::KEYWORD_INTO;
        keywordMap["values"] = Token::KEYWORD_VALUES;
        keywordMap["set"] = Token::KEYWORD_SET;

        // Query Keywords
        keywordMap["where"] = Token::KEYWORD_WHERE;
        keywordMap["group"] = Token::KEYWORD_GROUP;
        keywordMap["by"] = Token::KEYWORD_BY;
        keywordMap["having"] = Token::KEYWORD_HAVING;
        keywordMap["order"] = Token::KEYWORD_ORDER;
        keywordMap["limit"] = Token::KEYWORD_LIMIT;
        keywordMap["offset"] = Token::KEYWORD_OFFSET;
        keywordMap["distinct"] = Token::KEYWORD_DISTINCT;

        // Join Keywords
        keywordMap["join"] = Token::KEYWORD_JOIN;
        keywordMap["inner"] = Token::KEYWORD_INNER;
        keywordMap["left"] = Token::KEYWORD_LEFT;
        keywordMap["right"] = Token::KEYWORD_RIGHT;
        keywordMap["full"] = Token::KEYWORD_FULL;
        keywordMap["outer"] = Token::KEYWORD_OUTER;
        keywordMap["on"] = Token::KEYWORD_ON;

        // Constraint Keywords
        keywordMap["primary"] = Token::KEYWORD_PRIMARY;
        keywordMap["key"] = Token::KEYWORD_KEY;
        keywordMap["foreign"] = Token::KEYWORD_FOREIGN;
        keywordMap["references"] = Token::KEYWORD_REFERENCES;
        keywordMap["unique"] = Token::KEYWORD_UNIQUE;
        keywordMap["not"] = Token::KEYWORD_NOT;
        keywordMap["null"] = Token::KEYWORD_NULL;
        keywordMap["default"] = Token::KEYWORD_DEFAULT;
        keywordMap["auto_increment"] = Token::KEYWORD_AUTO_INCREMENT;

        // Permission Keywords
        keywordMap["grant"] = Token::KEYWORD_GRANT;
        keywordMap["revoke"] = Token::KEYWORD_REVOKE;
        keywordMap["to"] = Token::KEYWORD_TO;
        keywordMap["user"] = Token::KEYWORD_USER;
        keywordMap["with"] = Token::KEYWORD_WITH;
        keywordMap["password"] = Token::KEYWORD_PASSWORD;
        keywordMap["identified"] = Token::KEYWORD_IDENTIFIED;
        keywordMap["show"] = Token::KEYWORD_SHOW;

        // Logical Operators
        keywordMap["and"] = Token::KEYWORD_AND;
        keywordMap["or"] = Token::KEYWORD_OR;
        keywordMap["in"] = Token::KEYWORD_IN;
        keywordMap["exists"] = Token::KEYWORD_EXISTS;
        keywordMap["between"] = Token::KEYWORD_BETWEEN;
        keywordMap["like"] = Token::KEYWORD_LIKE;

        // Aggregate Functions
        keywordMap["count"] = Token::KEYWORD_COUNT;
        keywordMap["sum"] = Token::KEYWORD_SUM;
        keywordMap["avg"] = Token::KEYWORD_AVG;
        keywordMap["min"] = Token::KEYWORD_MIN;
        keywordMap["max"] = Token::KEYWORD_MAX;
    }

    auto it = keywordMap.find(keyword);
    if (it != keywordMap.end()) {
        return Token(it->second, keyword, line, column);
    }

    return Token(Token::IDENTIFIER, keyword, line, column);
}

Token LexerNew::createNumberToken(const std::string& lexeme, int line, int column) {
    // Check if it contains a decimal point or exponent
    bool hasDot = lexeme.find('.') != std::string::npos;
    bool hasExponent = lexeme.find('e') != std::string::npos || lexeme.find('E') != std::string::npos;

    if (hasDot || hasExponent) {
        return Token(Token::FLOAT_LITERAL, lexeme, line, column);
    } else {
        return Token(Token::INTEGER_LITERAL, lexeme, line, column);
    }
}

Token LexerNew::createOperatorToken(const std::string& lexeme, int line, int column) {
    static std::unordered_map<std::string, Token::Type> operatorMap = {
        {"+", Token::OPERATOR_PLUS},
        {"-", Token::OPERATOR_MINUS},
        {"*", Token::OPERATOR_MULTIPLY},
        {"/", Token::OPERATOR_DIVIDE},
        {"=", Token::OPERATOR_EQUAL},
        {"!=", Token::OPERATOR_NOT_EQUAL},
        {"<", Token::OPERATOR_LESS_THAN},
        {"<=", Token::OPERATOR_LESS_EQUAL},
        {">", Token::OPERATOR_GREATER_THAN},
        {">=", Token::OPERATOR_GREATER_EQUAL},
        {"&&", Token::OPERATOR_AND},
        {"||", Token::OPERATOR_OR},
        {"!", Token::OPERATOR_NOT}
    };

    auto it = operatorMap.find(lexeme);
    if (it != operatorMap.end()) {
        return Token(it->second, lexeme, line, column);
    }

    return Token(Token::OPERATOR, lexeme, line, column);
}

Token LexerNew::createPunctuationToken(const std::string& lexeme, int line, int column) {
    static std::unordered_map<std::string, Token::Type> punctuationMap = {
        {";", Token::SEMICOLON},
        {"(", Token::LPAREN},
        {")", Token::RPAREN},
        {",", Token::COMMA},
        {".", Token::DOT}
    };

    auto it = punctuationMap.find(lexeme);
    if (it != punctuationMap.end()) {
        return Token(it->second, lexeme, line, column);
    }

    return Token(Token::UNKNOWN, lexeme, line, column);
}

void LexerNew::skipLineComment() {
    // Skip '--' and rest of line
    advance(); // skip first '-'
    advance(); // skip second '-'

    while (!isAtEnd() && peek() != '\n') {
        advance();
    }
}

void LexerNew::skipBlockComment() {
    // Skip '/*' ... '*/'
    advance(); // skip '/'
    advance(); // skip '*'

    while (!isAtEnd()) {
        if (peek() == '*' && peekNext() == '/') {
            advance(); // skip '*'
            advance(); // skip '/'
            break;
        }
        advance();
    }
}

} // namespace sql_parser
} // namespace sqlcc
