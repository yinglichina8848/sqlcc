#include "sql_parser/token.h"

namespace sqlcc {
namespace sql_parser {

Token::Token() : type_(END_OF_INPUT), lexeme_(""), line_(0), column_(0) {
}

Token::Token(Type type, const std::string& lexeme, int line, int column)
    : type_(type), lexeme_(lexeme), line_(line), column_(column) {
}

Token::Type Token::getType() const {
    return type_;
}

const std::string& Token::getLexeme() const {
    return lexeme_;
}

int Token::getLine() const {
    return line_;
}

int Token::getColumn() const {
    return column_;
}

std::string Token::getTypeName(Type type) {
    switch (type) {
        case LPAREN: return "LEFT_PAREN";
        case RPAREN: return "RIGHT_PAREN";
        case LEFT_BRACE: return "LEFT_BRACE";
        case RIGHT_BRACE: return "RIGHT_BRACE";
        case COMMA: return "COMMA";
        case DOT: return "DOT";
        case MINUS: return "MINUS";
        case PLUS: return "PLUS";
        case SEMICOLON: return "SEMICOLON";
        case SLASH: return "SLASH";
        case EQUAL: return "EQUAL";
        case NOT_EQUAL: return "NOT_EQUAL";
        case LESS: return "LESS";
        case LESS_EQUAL: return "LESS_EQUAL";
        case GREATER: return "GREATER";
        case GREATER_EQUAL: return "GREATER_EQUAL";
        case IDENTIFIER: return "IDENTIFIER";
        case STRING: return "STRING";
        case NUMBER: return "NUMBER";
        case KEYWORD_AND: return "AND";
        case KEYWORD_BREAK: return "BREAK";
        case KEYWORD_CLASS: return "CLASS";
        case KEYWORD_CONTINUE: return "CONTINUE";
        case KEYWORD_DEF: return "DEF";
        case KEYWORD_DEL: return "DEL";
        case KEYWORD_DO: return "DO";
        case KEYWORD_ELSE: return "ELSE";
        case KEYWORD_FALSE: return "FALSE";
        case KEYWORD_FOR: return "FOR";
        case KEYWORD_FROM: return "FROM";
        case KEYWORD_IF: return "IF";
        case KEYWORD_IN: return "IN";
        case KEYWORD_IS: return "IS";
        case KEYWORD_NULL: return "NULL";
        case KEYWORD_OR: return "OR";
        case KEYWORD_PRINT: return "PRINT";
        case KEYWORD_RETURN: return "RETURN";
        case KEYWORD_SUPER: return "SUPER";
        case KEYWORD_THIS: return "THIS";
        case KEYWORD_TRUE: return "TRUE";
        case KEYWORD_VAR: return "VAR";
        case KEYWORD_WHILE: return "WHILE";
        case KEYWORD_SELECT: return "SELECT";
        case KEYWORD_DISTINCT: return "DISTINCT";
        case KEYWORD_INSERT: return "INSERT";
        case KEYWORD_UPDATE: return "UPDATE";
        case KEYWORD_DELETE: return "DELETE";
        case KEYWORD_CREATE: return "CREATE";
        case KEYWORD_DROP: return "DROP";
        case KEYWORD_ALTER: return "ALTER";
        case KEYWORD_DATABASE: return "DATABASE";
        case KEYWORD_TABLE: return "TABLE";
        case KEYWORD_INDEX: return "INDEX";
        case KEYWORD_PRIMARY: return "PRIMARY";
        case KEYWORD_KEY: return "KEY";
        case KEYWORD_NOT: return "NOT";
        case KEYWORD_UNIQUE: return "UNIQUE";
        case KEYWORD_CHECK: return "CHECK";
        case KEYWORD_DEFAULT: return "DEFAULT";
        case KEYWORD_AUTO_INCREMENT: return "AUTO_INCREMENT";
        case KEYWORD_REFERENCES: return "REFERENCES";
        case KEYWORD_FOREIGN: return "FOREIGN";
        case KEYWORD_USE: return "USE";
        case KEYWORD_VALUES: return "VALUES";
        case KEYWORD_SET: return "SET";
        case KEYWORD_WHERE: return "WHERE";
        case KEYWORD_GROUP: return "GROUP";
        case KEYWORD_BY: return "BY";
        case KEYWORD_ORDER: return "ORDER";
        case KEYWORD_ASC: return "ASC";
        case KEYWORD_DESC: return "DESC";
        case KEYWORD_INTO: return "INTO";
        case KEYWORD_USER: return "USER";
        case KEYWORD_GRANT: return "GRANT";
        case KEYWORD_REVOKE: return "REVOKE";
        case KEYWORD_TO: return "TO";
        case KEYWORD_ON: return "ON";
        case KEYWORD_EXISTS: return "EXISTS";
        case KEYWORD_JOIN: return "JOIN";
        case KEYWORD_HAVING: return "HAVING";
        case KEYWORD_CONSTRAINT: return "CONSTRAINT";
        case KEYWORD_PRIVILEGES: return "PRIVILEGES";
        case KEYWORD_WITH: return "WITH";
        case KEYWORD_PASSWORD: return "PASSWORD";
        case KEYWORD_IDENTIFIED: return "IDENTIFIED";
        case MULTIPLY: return "MULTIPLY";
        case END_OF_INPUT: return "END_OF_INPUT";
        case ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

} // namespace sql_parser
} // namespace sqlcc