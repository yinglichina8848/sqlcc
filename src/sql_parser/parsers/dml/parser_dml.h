/**
 * ParserDML - DML语句解析器头文件
 * 
 * 包含数据操作语言（DML）语句的解析方法，包括：
 * - SELECT语句：基本查询、子查询、集合操作等
 * - INSERT语句：单行插入、多行插入、子查询插入等
 * - UPDATE语句：单表更新、多表更新等
 * - DELETE语句：单表删除、多表删除等
 * 
 * 设计原则：
 * - 单一职责：专门处理DML语句解析
 * - 模块化：每个DML语句类型有独立的解析方法
 * - 可扩展：易于添加新的DML语句支持
 */

#ifndef SQLCC_SQL_PARSER_PARSERS_DML_PARSER_DML_H
#define SQLCC_SQL_PARSER_PARSERS_DML_PARSER_DML_H

#include "../parser_core.h"
#include "../../ast/ast_node.h"
#include "../../select_parser.h"

namespace sqlcc {
namespace sql_parser {

class ParserDML : public ParserCore {
private:
    std::unique_ptr<SelectParser> select_parser_;

public:
    ParserDML(TokenStream& tokens);
    
    // DML语句解析方法
    std::unique_ptr<ASTNode> parseSelectStatement();
    std::unique_ptr<ASTNode> parseInsertStatement();
    std::unique_ptr<ASTNode> parseUpdateStatement();
    std::unique_ptr<ASTNode> parseDeleteStatement();
    
    // DML辅助解析方法
    std::unique_ptr<WhereClause> parseWhereClause();
    std::unique_ptr<JoinClause> parseJoinClause();
    std::unique_ptr<SetOperation> parseSetOperation();
    
    // INSERT语句辅助方法
    std::vector<std::string> parseInsertColumnNames();
    std::vector<std::vector<std::unique_ptr<Expression>>> parseInsertValues();
    std::unique_ptr<SelectStatement> parseInsertSelect();
    
    // UPDATE语句辅助方法
    std::vector<std::pair<std::string, std::unique_ptr<Expression>>> parseUpdateAssignments();
    
    // DELETE语句辅助方法
    std::vector<std::string> parseDeleteTableNames();
};

} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_PARSERS_DML_PARSER_DML_H