#include "sql_parser/lexer.h"
#include <cctype>
#include <stdexcept>
#include <string>

namespace sqlcc {
namespace sql_parser {

Lexer::Lexer(const std::string& input) : input_(input), position_(0), line_(1), column_(1) {
}

bool Lexer::isAtEnd() const {
    return position_ >= static_cast<int>(input_.length());
}

char Lexer::advance() {
    char ch = input_[position_++];
    if (ch == '\n') {
        line_++;
        column_ = 1;
    } else {
        column_++;
    }
    return ch;
}

char Lexer::peek() const {
    if (isAtEnd()) {
        return '\0';
    }
    return input_[position_];
}

char Lexer::peekNext() const {
    if (position_ + 1 >= static_cast<int>(input_.length())) {
        return '\0';
    }
    return input_[position_ + 1];
}

Token Lexer::nextToken() {
    while (!isAtEnd()) {
        char ch = advance();
        
        // 处理注释
        if (ch == '-' && peek() == '-') {
            // 单行注释: --
            skipLineComment();
            continue;
        }
        
        if (ch == '/' && peek() == '*') {
            // 多行注释: /* */
            skipBlockComment();
            continue;
        }
        
        switch (ch) {
            case ' ':
            case '\t':
            case '\r':
            case '\n':
                // 忽略空白字符
                break;
                
            case ';':
                return Token(Token::SEMICOLON, ";", line_, column_);
                
            case '(':
                return Token(Token::LPAREN, "(", line_, column_);
                
            case ')':
                return Token(Token::RPAREN, ")", line_, column_);
                
            case ',':
                return Token(Token::COMMA, ",", line_, column_);
                
            case '.':
                return Token(Token::DOT, ".", line_, column_);
                
            case '*':
                return Token(Token::MULTIPLY, "*", line_, column_);
                
            case '=':
                return Token(Token::EQUAL, "=", line_, column_);
                
            case '!':
                if (peek() == '=') {
                    advance();
                    return Token(Token::NOT_EQUAL, "!=", line_, column_);
                } else {
                    throw std::runtime_error("Unexpected character: !");
                }
                
            case '<':
                if (peek() == '=') {
                    advance();
                    return Token(Token::LESS_EQUAL, "<=", line_, column_);
                } else {
                    return Token(Token::LESS, "<", line_, column_);
                }
                
            case '>':
                if (peek() == '=') {
                    advance();
                    return Token(Token::GREATER_EQUAL, ">=", line_, column_);
                } else {
                    return Token(Token::GREATER, ">", line_, column_);
                }
                
            case '\'':
                return readString();
                
            case '"':
                return readString();
                
            case '-':
                // 单独的减号或负号
                if (std::isdigit(peek())) {
                    return readNumber();
                } else {
                    return Token(Token::MINUS, "-", line_, column_);
                }
                
            case '+':
                return Token(Token::PLUS, "+", line_, column_);
                
            case '/':
                return Token(Token::SLASH, "/", line_, column_);
                
            default:
                if (std::isdigit(ch)) {
                    return readNumber();
                } else if (std::isalpha(ch) || ch == '_') {
                    return readIdentifier();
                } else {
                    throw std::runtime_error("Unexpected character: " + std::string(1, ch));
                }
        }
    }
    
    return Token(Token::END_OF_INPUT, "", line_, column_);
}

Token Lexer::readString() {
    std::string value;
    char quote = input_[position_ - 1]; // 记住开始的引号类型
    
    while (!isAtEnd() && peek() != quote) {
        value += advance();
    }
    
    if (isAtEnd()) {
        throw std::runtime_error("Unterminated string literal");
    }
    
    advance(); // 跳过结束引号
    
    return Token(Token::STRING, value, line_, column_);
}

Token Lexer::readNumber() {
    int start = position_ - 1;
    
    while (!isAtEnd() && (std::isdigit(peek()) || peek() == '.')) {
        advance();
    }
    
    std::string value = input_.substr(start, position_ - start);
    
    return Token(Token::NUMBER, value, line_, column_);
}

Token Lexer::readIdentifier() {
    int start = position_ - 1;
    
    while (!isAtEnd() && (std::isalnum(peek()) || peek() == '_')) {
        advance();
    }
    
    std::string value = input_.substr(start, position_ - start);
    
    // 将标识符转换为大写，用于关键字比较
    std::string upper_value;
    for (char c : value) {
        upper_value += std::toupper(c);
    }
    
    Token::Type type = Token::IDENTIFIER;
    
    // 检查关键字（不区分大小写）
    if (upper_value == "CREATE") type = Token::KEYWORD_CREATE;
    else if (upper_value == "SELECT") type = Token::KEYWORD_SELECT;
    else if (upper_value == "INSERT") type = Token::KEYWORD_INSERT;
    else if (upper_value == "UPDATE") type = Token::KEYWORD_UPDATE;
    else if (upper_value == "DELETE") type = Token::KEYWORD_DELETE;
    else if (upper_value == "DROP") type = Token::KEYWORD_DROP;
    else if (upper_value == "ALTER") type = Token::KEYWORD_ALTER;
    else if (upper_value == "USE") type = Token::KEYWORD_USE;
    else if (upper_value == "DATABASE") type = Token::KEYWORD_DATABASE;
    else if (upper_value == "TABLE") type = Token::KEYWORD_TABLE;
    else if (upper_value == "INDEX") type = Token::KEYWORD_INDEX;
    else if (upper_value == "WHERE") type = Token::KEYWORD_WHERE;
    else if (upper_value == "JOIN") type = Token::KEYWORD_JOIN;
    else if (upper_value == "ON") type = Token::KEYWORD_ON;
    else if (upper_value == "IF") type = Token::KEYWORD_IF;
    else if (upper_value == "EXISTS") type = Token::KEYWORD_EXISTS;
    else if (upper_value == "GROUP") type = Token::KEYWORD_GROUP;
    else if (upper_value == "BY") type = Token::KEYWORD_BY;
    else if (upper_value == "HAVING") type = Token::KEYWORD_HAVING;
    else if (upper_value == "ORDER") type = Token::KEYWORD_ORDER;
    else if (upper_value == "INTO") type = Token::KEYWORD_INTO;
    else if (upper_value == "VALUES") type = Token::KEYWORD_VALUES;
    else if (upper_value == "SET") type = Token::KEYWORD_SET;
    else if (upper_value == "PRIMARY") type = Token::KEYWORD_PRIMARY;
    else if (upper_value == "KEY") type = Token::KEYWORD_KEY;
    else if (upper_value == "FOREIGN") type = Token::KEYWORD_FOREIGN;
    else if (upper_value == "REFERENCES") type = Token::KEYWORD_REFERENCES;
    else if (upper_value == "CONSTRAINT") type = Token::KEYWORD_CONSTRAINT;
    else if (upper_value == "NOT") type = Token::KEYWORD_NOT;
    else if (upper_value == "NULL") type = Token::KEYWORD_NULL;
    else if (upper_value == "UNIQUE") type = Token::KEYWORD_UNIQUE;
    else if (upper_value == "CHECK") type = Token::KEYWORD_CHECK;
    else if (upper_value == "DEFAULT") type = Token::KEYWORD_DEFAULT;
    else if (upper_value == "AUTO_INCREMENT") type = Token::KEYWORD_AUTO_INCREMENT;
    else if (upper_value == "EXISTS") type = Token::KEYWORD_EXISTS;
    else if (upper_value == "USER") type = Token::KEYWORD_USER;
    else if (upper_value == "GRANT") type = Token::KEYWORD_GRANT;
    else if (upper_value == "REVOKE") type = Token::KEYWORD_REVOKE;
    else if (upper_value == "PRIVILEGES") type = Token::KEYWORD_PRIVILEGES;
    else if (upper_value == "TO") type = Token::KEYWORD_TO;
    else if (upper_value == "FROM") type = Token::KEYWORD_FROM;
    else if (upper_value == "WITH") type = Token::KEYWORD_WITH;
    else if (upper_value == "PASSWORD") type = Token::KEYWORD_PASSWORD;
    else if (upper_value == "IDENTIFIED") type = Token::KEYWORD_IDENTIFIED;
    else if (upper_value == "SHOW") type = Token::KEYWORD_SHOW;
    else if (upper_value == "COLUMNS") type = Token::KEYWORD_COLUMNS;
    else if (upper_value == "INDEXES") type = Token::KEYWORD_INDEXES;
    else if (upper_value == "GRANTS") type = Token::KEYWORD_GRANTS;
    else if (upper_value == "DATABASES") type = Token::KEYWORD_DATABASES;
    else if (upper_value == "TABLES") type = Token::KEYWORD_TABLES;
    
    return Token(type, value, line_, column_);
}

void Lexer::skipLineComment() {
    // 跳过 '--' 之后的所有字符直到行尾
    advance(); // 跳过第二个 '-'
    while (!isAtEnd() && peek() != '\n') {
        advance();
    }
}

void Lexer::skipBlockComment() {
    // 跳过 '/*' 之后的所有字符直到找到 '*/'
    advance(); // 跳过 '*'
    
    while (!isAtEnd()) {
        if (peek() == '*' && peekNext() == '/') {
            advance(); // 跳过 '*'
            advance(); // 跳过 '/'
            break;
        }
        advance();
    }
}

} // namespace sql_parser
} // namespace sqlcc