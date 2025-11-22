#ifndef LEXER_H
#define LEXER_H

#include "token.h"
#include <string>

namespace sqlcc {
namespace sql_parser {

class Lexer {
public:
    explicit Lexer(const std::string& input);
    
    // 获取下一个token
    Token nextToken();
    
    // 检查是否还有更多token
    bool hasMoreTokens() const;
    
private:
    std::string input_;
    size_t position_;
    
    // 辅助方法
    char advance();
    char peek() const;
    bool isAtEnd() const;
    bool isDigit(char c) const;
    bool isLetter(char c) const;
    bool isWhitespace(char c) const;
    
    // 跳过空白字符
    void skipWhitespace();
    
    // 解析标识符或关键字
    Token parseIdentifierOrKeyword();
    
    // 解析数字
    Token parseNumber();
    
    // 解析字符串
    Token parseString();
};

} // namespace sql_parser
} // namespace sqlcc

#endif // LEXER_H