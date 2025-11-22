#ifndef TOKEN_H
#define TOKEN_H

#include <string>

namespace sqlcc {
namespace sql_parser {

class Token {
public:
    enum Type {
        // 关键字
        BEGIN,
        COMMIT,
        ROLLBACK,
        SAVEPOINT,
        SET,
        TRANSACTION,
        CREATE,
        
        // 标识符和字面量
        IDENTIFIER,
        STRING,
        NUMBER,
        
        // 操作符和标点符号
        LPAREN,
        RPAREN,
        SEMICOLON,
        
        // 特殊标记
        END_OF_INPUT,
        UNKNOWN
    };
    
    // 默认构造函数
    Token() : type_(END_OF_INPUT), lexeme_(""), line_(0), column_(0) {}
    
    Token(Type type, const std::string& lexeme, int line, int column)
        : type_(type), lexeme_(lexeme), line_(line), column_(column) {}
    
    Type getType() const { return type_; }
    const std::string& getLexeme() const { return lexeme_; }
    int getLine() const { return line_; }
    int getColumn() const { return column_; }
    
private:
    Type type_;
    std::string lexeme_;
    int line_;
    int column_;
};

} // namespace sql_parser
} // namespace sqlcc

#endif // TOKEN_H