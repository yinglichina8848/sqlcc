/**
 * ParserHelpers - 辅助解析器头文件
 * 
 * 包含各种辅助解析方法，包括：
 * - 表达式解析辅助方法
 * - 集合操作解析方法
 * - 其他辅助语句解析方法
 * 
 * 设计原则：
 * - 单一职责：专门处理辅助解析功能
 * - 模块化：按功能分类组织辅助方法
 * - 可复用：可以被其他解析器模块调用
 */

#ifndef SQLCC_SQL_PARSER_PARSERS_HELPERS_PARSER_HELPERS_H
#define SQLCC_SQL_PARSER_PARSERS_HELPERS_PARSER_HELPERS_H

#include "../parser_core.h"
#include "../../ast/ast_node.h"
#include "../../expression_parser.h"
#include "../../set_operation.h"
#include "../../ast/utilities/ast_utility_nodes.h"

namespace sqlcc {
namespace sql_parser {

class ParserHelpers : public ParserCore {
private:
    std::unique_ptr<ExpressionParser> expression_parser_;

public:
    ParserHelpers(TokenStream& tokens);
    
    // 辅助解析方法
    std::vector<std::string> parseColumnNames();
    std::vector<std::unique_ptr<Expression>> parseExpressions();
    std::string parseQualifiedName();
    std::string parseIdentifier();
    std::string parseStringLiteral();
    int64_t parseIntLiteral();
    double parseFloatLiteral();
    
    // 集合操作解析方法
    std::unique_ptr<ASTNode> parseCompositeSelectStatement();
    std::unique_ptr<SetOperation> parseSetOperation();
    std::unique_ptr<SetOperation> parseUnion();
    std::unique_ptr<SetOperation> parseIntersect();
    std::unique_ptr<SetOperation> parseExcept();
    SetOperationType parseSetOperationType();
    bool isSetOperation();
    
    // 其他辅助语句解析方法
    std::unique_ptr<ASTNode> parseUseStatement();
    std::unique_ptr<ASTNode> parseShowStatement();
    std::unique_ptr<ASTNode> parseLoadDataStatement();
    
    // 表达式解析委托给ExpressionParser
    std::unique_ptr<Expression> parseExpression();
};

} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_PARSERS_HELPERS_PARSER_HELPERS_H