#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <memory>
#include <vector>

#include "../include/sql_parser/token.h"
#include "../include/sql_parser/ast_nodes.h"
#include "../include/sql_parser/ast_node.h"
#include "../include/sql_parser/lexer.h"

namespace sqlcc {
namespace sql_parser {

// 前向声明
class Statement;
class SavepointStatement;
class SetTransactionStatement;

class Parser {
public:
    Parser();
    /**
     * 构造函数，接收Lexer引用
     * @param lexer Lexer引用，用于获取token
     */
    explicit Parser(Lexer& lexer);
    
    /**
     * 构造函数，接收SQL字符串
     * @param sql SQL字符串
     */
    Parser(const std::string& sql);
    
    /**
     * 析构函数
     */
    ~Parser();
    
    bool match(Token::Type expectedType);
    void reportError(const std::string& message);
    
    std::vector<std::unique_ptr<Statement>> parseStatements();
    std::unique_ptr<Statement> parseSingleStatement();
    std::unique_ptr<Statement> parseStatement();
    
    std::unique_ptr<Statement> parseRollback();
    std::unique_ptr<Statement> parseSavepoint();
    std::unique_ptr<Statement> parseSetTransaction();
    
    std::unique_ptr<Statement> parseCreate();
    std::unique_ptr<Statement> parseSelect();
    
private:
    // Lexer指针
    Lexer* lexer_;
    
    // 是否拥有lexer对象
    bool ownsLexer_;
    
    // 当前token
    Token currentToken_;
    
    // 严格模式标志
    bool strictMode_ = false;
    
    // 错误消息
    std::string errorMessage_;
    
    void consume();
};

} // namespace sql_parser
} // namespace sqlcc

#endif // PARSER_H