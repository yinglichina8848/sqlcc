/**
 * ParserTCL - TCL语句解析器头文件
 * 
 * 包含事务控制语言（TCL）语句的解析方法，包括：
 * - COMMIT语句：提交事务
 * - ROLLBACK语句：回滚事务
 * - BEGIN语句：开始事务
 * 
 * 设计原则：
 * - 单一职责：专门处理TCL语句解析
 * - 模块化：每个TCL语句类型有独立的解析方法
 * - 可扩展：易于添加新的TCL语句支持
 */

#ifndef SQLCC_SQL_PARSER_PARSERS_TCL_PARSER_TCL_H
#define SQLCC_SQL_PARSER_PARSERS_TCL_PARSER_TCL_H

#include "../parser_core.h"
#include "../../ast/ast_node.h"

namespace sqlcc {
namespace sql_parser {

class ParserTCL : public ParserCore {
public:
    ParserTCL(TokenStream& tokens);
    
    // TCL语句解析方法
    std::unique_ptr<ASTNode> parseCommitStatement();
    std::unique_ptr<ASTNode> parseRollbackStatement();
    std::unique_ptr<ASTNode> parseBeginStatement();
    
    // 事务选项解析
    std::string parseTransactionMode();
    std::string parseIsolationLevel();
};

} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_PARSERS_TCL_PARSER_TCL_H