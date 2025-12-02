#ifndef SQLCC_SQL_PARSER_TOKEN_H
#define SQLCC_SQL_PARSER_TOKEN_H

#include <string>

namespace sqlcc {
namespace sql_parser {

class Token {
public:
    enum Type {
        // 单字符符号
        LPAREN, RPAREN, LEFT_BRACE, RIGHT_BRACE,
        COMMA, DOT, MINUS, PLUS, SEMICOLON, SLASH, STAR,
        EQUAL, NOT_EQUAL, LESS, LESS_EQUAL, GREATER, GREATER_EQUAL,
        
        // 字面量
        IDENTIFIER, STRING, NUMBER,
        
        // 关键字
        KEYWORD_AND, KEYWORD_BREAK, KEYWORD_CLASS, KEYWORD_CONTINUE, KEYWORD_DEF, 
        KEYWORD_DEL, KEYWORD_DO, KEYWORD_ELSE, KEYWORD_FALSE, KEYWORD_FOR, 
        KEYWORD_FROM, KEYWORD_IF, KEYWORD_IN, KEYWORD_IS, KEYWORD_NULL, 
        KEYWORD_OR, KEYWORD_PRINT, KEYWORD_RETURN, KEYWORD_SUPER, KEYWORD_THIS, 
        KEYWORD_TRUE, KEYWORD_VAR, KEYWORD_WHILE, KEYWORD_SELECT, KEYWORD_INSERT, 
        KEYWORD_UPDATE, KEYWORD_DELETE, KEYWORD_CREATE, KEYWORD_DROP, KEYWORD_ALTER,
        KEYWORD_DATABASE, KEYWORD_TABLE, KEYWORD_INDEX, KEYWORD_PRIMARY, KEYWORD_KEY,
        KEYWORD_NOT, KEYWORD_UNIQUE, KEYWORD_CHECK, KEYWORD_DEFAULT, KEYWORD_AUTO_INCREMENT,
        KEYWORD_REFERENCES, KEYWORD_FOREIGN, KEYWORD_USE, KEYWORD_VALUES, KEYWORD_SET,
        KEYWORD_WHERE, KEYWORD_GROUP, KEYWORD_BY, KEYWORD_ORDER, KEYWORD_ASC, KEYWORD_DESC,
        KEYWORD_INTO, KEYWORD_USER, KEYWORD_GRANT, KEYWORD_REVOKE, KEYWORD_TO, KEYWORD_ON,
        KEYWORD_EXISTS, KEYWORD_JOIN, KEYWORD_HAVING, KEYWORD_CONSTRAINT, KEYWORD_PRIVILEGES,
        KEYWORD_WITH, KEYWORD_PASSWORD, KEYWORD_IDENTIFIED, KEYWORD_SHOW, KEYWORD_COLUMNS,
        KEYWORD_INDEXES, KEYWORD_GRANTS, KEYWORD_DATABASES, KEYWORD_TABLES,
        
        MULTIPLY, // *
        
        END_OF_INPUT, // 文件结束
        
        ERROR // 错误标记
    };

    // 默认构造函数
    Token();
    
    Token(Type type, const std::string& lexeme, int line, int column);
    ~Token() = default;

    Type getType() const;
    const std::string& getLexeme() const;
    int getLine() const;
    int getColumn() const;
    static std::string getTypeName(Type type);

private:
    Type type_;
    std::string lexeme_;
    int line_;
    int column_;
};

} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_TOKEN_H