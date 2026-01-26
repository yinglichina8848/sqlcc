/**
 * ParserCore - 核心解析方法头文件
 * 
 * 包含Parser类的核心解析方法,包括：
 * - 基础token匹配方法(match/consume/check等)
 * - 错误处理方法(reportError/synchronize)
 * - 语句类型判断方法(isCreateViewStatement/isDropIndexStatement等)
 * - 前瞻token管理方法
 * 
 * 设计原则：
 * - 单一职责：只包含核心解析逻辑,不涉及具体语句解析
 * - 高内聚：相关功能方法集中管理
 * - 低耦合：通过接口与其他解析器模块交互
 */

#ifndef SQLCC_SQL_PARSER_PARSERS_PARSER_CORE_H
#define SQLCC_SQL_PARSER_PARSERS_PARSER_CORE_H

#include "../ast/ast_node.h"
#include "../token.h"
#include "../lexer.h"
#include "../token_stream.h"
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

namespace sqlcc {
namespace sql_parser {

class ParserCore {
protected:
    // Single source of truth for parser state
    TokenStream& tokens_;

    // Error recovery
    std::vector<std::string> errors_;
    bool panic_mode_;
    std::unordered_set<Type> sync_tokens_;

public:
    ParserCore(TokenStream& tokens);
    
    // 基础token匹配方法
    bool match(Type type);
    void consume(Type type, const std::string& message = "");
    bool check(Type type) const;
    
    // 前瞻token管理
    void advance();
    const Token& current() const;
    Token previous() const;
    const Token& peek();
    
    // 错误处理方法
    void reportError(const std::string& message);
    void synchronize();
    
    // 语句类型判断方法
    bool isCreateViewStatement() const;
    bool isDropIndexStatement() const;
    bool isCreateIndexStatement() const;
    bool isShowStatement() const;
    bool isUseStatement() const;
    bool isLoadDataStatement() const;
    bool isGrantStatement() const;
    bool isRevokeStatement() const;
    bool isCommitStatement() const;
    bool isRollbackStatement() const;
    bool isBeginStatement() const;
    bool isCreateUserStatement() const;
    bool isDropUserStatement() const;
    bool isCreateProcedureStatement() const;
    bool isCreateTriggerStatement() const;
    bool isSetOperation() const;
    
    // 错误信息获取
    const std::vector<std::string>& getErrors() const;
    bool hasErrors() const;
    
    // 状态管理
    bool isAtEnd() const;
    void resetPanicMode();
    void setSyncTokens(const std::unordered_set<Type>& tokens);
    
    // 通用解析助手方法
    std::string parseIdentifier();
    std::string parseQualifiedName();
    std::string parseStringLiteral();
    int parseIntLiteral();
    std::vector<std::string> parseColumnNames();
};

} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_PARSERS_PARSER_CORE_H