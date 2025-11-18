#ifndef SQLCC_SQL_PARSER_LEXER_H
#define SQLCC_SQL_PARSER_LEXER_H

#include "token.h"
#include <string>

namespace sqlcc {
namespace sql_parser {

/**
 * SQL词法分析器
 * 将输入的SQL字符串转换为Token序列
 */
class Lexer {
public:
    /**
     * 构造函数
     * @param input 输入的SQL字符串
     */
    explicit Lexer(const std::string& input);
    
    /**
     * 析构函数
     */
    ~Lexer() = default;
    
    /**
     * 获取下一个Token
     * @return 解析出的Token
     */
    Token nextToken();
    
private:
    /**
     * 扫描下一个Token（内部实现）
     * @return 解析出的Token
     */
    Token scanNextToken();
    
    /**
     * 查看当前Token但不消费
     * @return 当前Token对象
     */
    Token peekToken() const;
    
    /**
     * 位置结构体
     */
    struct Position {
        int line;   // 行号
        int column; // 列号
    };
    
    /**
     * 获取当前位置
     * @return 行号和列号
     */
    Position getPosition() const;
    
private:
    /**
     * 跳过空白字符
     */
    void skipWhitespace();
    
    /**
     * 跳过注释
     */
    void skipComment();
    
    /**
     * 读取标识符或关键字
     * @return 标识符Token
     */
    Token readIdentifier();
    
    /**
     * 读取数值字面量
     * @return 数值字面量Token
     */
    Token readNumber();
    
    /**
     * 读取字符串字面量
     * @return 字符串字面量Token
     */
    Token readString();
    
    /**
     * 检查当前字符是否为标识符开始
     * @param c 字符
     * @return 是否为标识符开始
     */
    bool isIdentifierStart(char c) const;
    
    /**
     * 检查当前字符是否为标识符部分
     * @param c 字符
     * @return 是否为标识符部分
     */
    bool isIdentifierPart(char c) const;
    
    /**
     * 检查当前字符是否为数字
     * @param c 字符
     * @return 是否为数字
     */
    bool isDigit(char c) const;
    
    /**
     * 检查当前字符是否为字母
     * @param c 字符
     * @return 是否为字母
     */
    bool isLetter(char c) const;
    
    /**
     * 检查当前字符是否为空白
     * @param c 字符
     * @return 是否为空白
     */
    bool isWhitespace(char c) const;
    
    /**
     * 获取当前字符
     * @return 当前字符
     */
    char currentChar() const;
    
    /**
     * 向前移动一个字符
     */
    void advance();
    
    /**
     * 向前移动多个字符
     * @param count 移动的字符数
     */
    void advance(int count);
    
    /**
     * 检查是否到达输入末尾
     * @return 是否已到达末尾
     */
    bool isEOF() const;
    
    /**
     * 检查当前字符是否为特定字符
     * @param c 要检查的字符
     * @return 是否匹配
     */
    bool match(char c) const;
    
    /**
     * 检查当前字符序列是否匹配特定字符串
     * @param str 要检查的字符串
     * @return 是否匹配
     */
    bool match(const std::string& str) const;
    
    /**
     * 获取关键字对应的Token类型
     * @param identifier 标识符
     * @return Token类型，如果不是关键字则返回IDENTIFIER
     */
    Token::Type getKeywordType(const std::string& identifier) const;
    
private:
    const std::string input_; // 输入的SQL字符串
    size_t position_;         // 当前位置
    int line_;                // 当前行号
    int column_;              // 当前列号
    Token currentToken_;      // 当前Token（用于peek）
    bool hasCachedToken_;     // 是否有缓存的Token
};

} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_LEXER_H