#ifndef SQLCC_SQL_PARSER_LEXER_H
#define SQLCC_SQL_PARSER_LEXER_H

#include <string>
#include "token_new.h"

namespace sqlcc {
namespace sql_parser {

class Lexer {
public:
    Lexer(const std::string& input);
    Token nextToken();

private:
    std::string input_;
    int position_;
    int line_;
    int column_;

    bool isAtEnd() const;
    char advance();
    char peek() const;
    char peekNext() const;
    Token readString();
    Token readNumber();
    Token readIdentifier();
    void skipLineComment();     // 跳过单行注释
    void skipBlockComment();    // 跳过多行注释
};

} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_LEXER_H
