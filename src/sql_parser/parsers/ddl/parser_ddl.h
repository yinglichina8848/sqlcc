/**
 * ParserDDL - DDL语句解析器头文件
 * 
 * 包含数据定义语言（DDL）语句的解析方法，包括：
 * - CREATE语句：CREATE TABLE, CREATE VIEW, CREATE INDEX等
 * - DROP语句：DROP TABLE, DROP VIEW, DROP INDEX等
 * - ALTER语句：ALTER TABLE等
 * 
 * 设计原则：
 * - 单一职责：专门处理DDL语句解析
 * - 模块化：每个DDL语句类型有独立的解析方法
 * - 可扩展：易于添加新的DDL语句支持
 */

#ifndef SQLCC_SQL_PARSER_PARSERS_DDL_PARSER_DDL_H
#define SQLCC_SQL_PARSER_PARSERS_DDL_PARSER_DDL_H

#include "../parser_core.h"
#include "../../ast/ast_node.h"
#include "../../ast/ddl/ast_ddl_nodes.h"
#include <memory>
#include <vector>

namespace sqlcc {
namespace sql_parser {

class ParserDDL : public ParserCore {
public:
    ParserDDL(TokenStream& tokens);
    
    // DDL语句解析方法
    std::unique_ptr<ASTNode> parseCreateStatement();
    std::unique_ptr<ASTNode> parseCreateTableStatement();
    std::unique_ptr<ASTNode> parseCreateDatabaseStatement();
    std::unique_ptr<ASTNode> parseCreateIndexStatement();
    std::unique_ptr<ASTNode> parseCreateViewStatement();
    std::unique_ptr<ASTNode> parseCreateUserStatement();
    std::unique_ptr<ASTNode> parseCreateProcedureStatement();
    std::unique_ptr<ASTNode> parseCreateTriggerStatement();
    
    std::unique_ptr<ASTNode> parseDropStatement();
    std::unique_ptr<ASTNode> parseDropTableStatement();
    std::unique_ptr<ASTNode> parseDropDatabaseStatement();
    std::unique_ptr<ASTNode> parseDropIndexStatement();
    std::unique_ptr<ASTNode> parseDropViewStatement();
    std::unique_ptr<ASTNode> parseDropUserStatement();
    std::unique_ptr<ASTNode> parseDropProcedureStatement();
    std::unique_ptr<ASTNode> parseDropTriggerStatement();
    
    std::unique_ptr<ASTNode> parseAlterStatement();
    std::unique_ptr<ASTNode> parseAlterTableStatement();
    std::unique_ptr<ASTNode> parseAlterDatabaseStatement();
    
    // DDL辅助解析方法
    std::unique_ptr<ColumnDefinition> parseColumnDefinition();
    std::unique_ptr<TableConstraint> parseTableConstraint();
    std::vector<std::unique_ptr<ColumnDefinition>> parseColumnDefinitions();
    std::vector<std::unique_ptr<TableConstraint>> parseTableConstraints();
};

} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_PARSERS_DDL_PARSER_DDL_H