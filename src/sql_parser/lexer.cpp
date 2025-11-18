#include "sql_parser/lexer.h"
#include <cctype>
#include <unordered_map>
#include <stdexcept>

namespace sqlcc {
namespace sql_parser {

Lexer::Lexer(const std::string& input)
    : input_(input), position_(0), line_(1), column_(1), currentToken_(Token::END_OF_INPUT, "", 1, 1), hasCachedToken_(false) {
    // 初始化关键字映射表
}

Token Lexer::nextToken() {
    if (hasCachedToken_) {
        hasCachedToken_ = false;
        return currentToken_;
    }
    
    Token token = scanNextToken();
    return token;
}

Token Lexer::peekToken() const {
    if (!hasCachedToken_) {
        // 这里需要一个非const版本来缓存token
        // 实际实现中可能需要调整设计
        throw std::runtime_error("peekToken not implemented yet");
    }
    return currentToken_;
}

Lexer::Position Lexer::getPosition() const {
    return {line_, column_};
}

void Lexer::skipWhitespace() {
    while (!isEOF() && isWhitespace(currentChar())) {
        if (currentChar() == '\n') {
            line_++;
            column_ = 1;
        } else {
            column_++;
        }
        advance();
    }
}

void Lexer::skipComment() {
    // 跳过单行注释
    if (match("--")) {
        while (!isEOF() && currentChar() != '\n') {
            advance();
        }
        if (!isEOF()) {
            advance();
            line_++;
            column_ = 1;
        }
    }
    // 跳过多行注释
    else if (match("/*")) {
        advance(2);
        while (!isEOF() && !match("*/")) {
            if (currentChar() == '\n') {
                line_++;
                column_ = 1;
            }
            advance();
        }
        if (match("*/")) {
            advance(2);
        }
    }
}

Token Lexer::readIdentifier() {
    int start = position_;
    int startColumn = column_;
    
    while (!isEOF() && isIdentifierPart(currentChar())) {
        advance();
        column_++;
    }
    
    std::string identifier = input_.substr(start, position_ - start);
    Token::Type type = getKeywordType(identifier);
    
    return Token(type, identifier, line_, startColumn);
}

Token Lexer::readNumber() {
    int start = position_;
    int startColumn = column_;
    
    // 读取整数部分
    while (!isEOF() && isDigit(currentChar())) {
        advance();
        column_++;
    }
    
    // 读取小数部分
    if (!isEOF() && currentChar() == '.' && isDigit(input_[position_ + 1])) {
        advance();
        column_++;
        while (!isEOF() && isDigit(currentChar())) {
            advance();
            column_++;
        }
    }
    
    std::string number = input_.substr(start, position_ - start);
    
    return Token(Token::NUMERIC_LITERAL, number, line_, startColumn);
}

Token Lexer::readString() {
    int start = position_ + 1; // 跳过引号
    int startColumn = column_;
    char quoteChar = currentChar();
    
    advance();
    column_++;
    
    while (!isEOF() && currentChar() != quoteChar) {
        // 处理转义字符
        if (currentChar() == '\\' && position_ + 1 < input_.size()) {
            advance();
            column_++;
        }
        advance();
        column_++;
    }
    
    if (isEOF()) {
        // 未闭合的字符串
        return Token(Token::INVALID_TOKEN, "Unclosed string", line_, startColumn);
    }
    
    advance(); // 跳过结束引号
    column_++;
    
    std::string value = input_.substr(start, position_ - start - 1);
    return Token(Token::STRING_LITERAL, value, line_, startColumn);
}

bool Lexer::isIdentifierStart(char c) const {
    return isLetter(c) || c == '_';
}

bool Lexer::isIdentifierPart(char c) const {
    return isLetter(c) || isDigit(c) || c == '_';
}

bool Lexer::isDigit(char c) const {
    return std::isdigit(c);
}

bool Lexer::isLetter(char c) const {
    return std::isalpha(c);
}

bool Lexer::isWhitespace(char c) const {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

char Lexer::currentChar() const {
    if (isEOF()) return '\0';
    return input_[position_];
}

void Lexer::advance() {
    if (!isEOF()) {
        position_++;
    }
}

void Lexer::advance(int count) {
    for (int i = 0; i < count && !isEOF(); i++) {
        advance();
    }
}

bool Lexer::isEOF() const {
    return position_ >= input_.size();
}

bool Lexer::match(char c) const {
    if (isEOF()) return false;
    return input_[position_] == c;
}

bool Lexer::match(const std::string& str) const {
    if (position_ + str.size() > input_.size()) {
        return false;
    }
    
    for (size_t i = 0; i < str.size(); i++) {
        if (input_[position_ + i] != str[i]) {
            return false;
        }
    }
    
    return true;
}

Token::Type Lexer::getKeywordType(const std::string& identifier) const {
    // 关键字映射表
    static const std::unordered_map<std::string, Token::Type> keywords = {
        {"CREATE", Token::KEYWORD_CREATE},
        {"SELECT", Token::KEYWORD_SELECT},
        {"INSERT", Token::KEYWORD_INSERT},
        {"UPDATE", Token::KEYWORD_UPDATE},
        {"DELETE", Token::KEYWORD_DELETE},
        {"DROP", Token::KEYWORD_DROP},
        {"ALTER", Token::KEYWORD_ALTER},
        {"USE", Token::KEYWORD_USE},
        {"DATABASE", Token::KEYWORD_DATABASE},
        {"TABLE", Token::KEYWORD_TABLE},
        {"WHERE", Token::KEYWORD_WHERE},
        {"JOIN", Token::KEYWORD_JOIN},
        {"ON", Token::KEYWORD_ON},
        {"GROUP", Token::KEYWORD_GROUP},
        {"BY", Token::KEYWORD_BY},
        {"HAVING", Token::KEYWORD_HAVING},
        {"ORDER", Token::KEYWORD_ORDER},
        {"INTO", Token::KEYWORD_INTO},
        {"VALUES", Token::KEYWORD_VALUES},
        {"SET", Token::KEYWORD_SET},
        {"FROM", Token::KEYWORD_FROM},
        {"AS", Token::KEYWORD_AS},
        {"DISTINCT", Token::KEYWORD_DISTINCT},
        {"AND", Token::KEYWORD_AND},
        {"OR", Token::KEYWORD_OR},
        {"NOT", Token::KEYWORD_NOT},
        {"NULL", Token::KEYWORD_NULL},
        {"IS", Token::KEYWORD_IS},
        {"LIKE", Token::KEYWORD_LIKE},
        {"IN", Token::KEYWORD_IN},
        {"BETWEEN", Token::KEYWORD_BETWEEN},
        {"ASC", Token::KEYWORD_ASC},
        {"DESC", Token::KEYWORD_DESC}
    };
    
    auto it = keywords.find(identifier);
    if (it != keywords.end()) {
        return it->second;
    }
    
    return Token::IDENTIFIER;
}

// 扫描下一个Token的实现
Token Lexer::scanNextToken() {
    // 跳过空白字符
    skipWhitespace();
    
    // 检查是否到达文件末尾
    if (isEOF()) {
        return Token(Token::END_OF_INPUT, "", line_, column_);
    }
    
    // 跳过注释
    if (match("--") || match("/*")) {
        skipComment();
        return scanNextToken();
    }
    
    char c = currentChar();
    int currentColumn = column_;
    
    // 标识符或关键字
    if (isIdentifierStart(c)) {
        return readIdentifier();
    }
    
    // 数字字面量
    if (isDigit(c)) {
        return readNumber();
    }
    
    // 字符串字面量
    if (c == '\'' || c == '"') {
        return readString();
    }
    
    // 运算符和标点符号
    switch (c) {
        case '=':
            advance();
            column_++;
            return Token(Token::OPERATOR_EQUAL, "=", line_, currentColumn);
        case '!':
            advance();
            column_++;
            if (match('=')) {
                advance();
                column_++;
                return Token(Token::OPERATOR_NOT_EQUAL, "!=", line_, currentColumn);
            }
            return Token(Token::KEYWORD_NOT, "!", line_, currentColumn);
        case '<':
            advance();
            column_++;
            if (match('=')) {
                advance();
                column_++;
                return Token(Token::OPERATOR_LESS_EQUAL, "<=", line_, currentColumn);
            }
            return Token(Token::OPERATOR_LESS, "<", line_, currentColumn);
        case '>':
            advance();
            column_++;
            if (match('=')) {
                advance();
                column_++;
                return Token(Token::OPERATOR_GREATER_EQUAL, ">=", line_, currentColumn);
            }
            return Token(Token::OPERATOR_GREATER, ">", line_, currentColumn);
        case '+':
            advance();
            column_++;
            return Token(Token::OPERATOR_PLUS, "+", line_, currentColumn);
        case '-':
            advance();
            column_++;
            return Token(Token::OPERATOR_MINUS, "-", line_, currentColumn);
        case '*':
            advance();
            column_++;
            return Token(Token::OPERATOR_MULTIPLY, "*", line_, currentColumn);
        case '/':
            advance();
            column_++;
            return Token(Token::OPERATOR_DIVIDE, "/", line_, currentColumn);
        case '(': 
            advance();
            column_++;
            return Token(Token::PUNCTUATION_LEFT_PAREN, "(", line_, currentColumn);
        case ')':
            advance();
            column_++;
            return Token(Token::PUNCTUATION_RIGHT_PAREN, ")", line_, currentColumn);
        case ',':
            advance();
            column_++;
            return Token(Token::PUNCTUATION_COMMA, ",", line_, currentColumn);
        case ';':
            advance();
            column_++;
            return Token(Token::PUNCTUATION_SEMICOLON, ";", line_, currentColumn);
        case '.':
            advance();
            column_++;
            return Token(Token::PUNCTUATION_DOT, ".", line_, currentColumn);
        default:
            // 无效字符
            advance();
            column_++;
            return Token(Token::INVALID_TOKEN, std::string(1, c), line_, currentColumn);
    }
}

} // namespace sql_parser
} // namespace sqlcc