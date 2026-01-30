/**
 * @file parser_tcl.cpp
 * @brief TCL语句解析器实现
 * 
 * 专门处理事务控制语言（TCL）语句的解析，包括：
 * - COMMIT语句（提交事务）
 * - ROLLBACK语句（回滚事务）
 * - BEGIN语句（开始事务）
 */

#include "src/sql_parser/parsers/tcl/parser_tcl.h"
#include "../../token.h"
#include "../../ast/ast_nodes.h"
#include <iostream>
#include <stdexcept>

namespace sqlcc {
namespace sql_parser {

ParserTCL::ParserTCL(TokenStream& tokens) : ParserCore(tokens) {}

// ==================== COMMIT语句解析 ====================

std::unique_ptr<ASTNode> ParserTCL::parseCommitStatement() {
    std::cout << "[PARSER DEBUG] parseCommitStatement() called" << std::endl;
    
    consume(Type::KEYWORD_COMMIT);
    
    auto stmt = std::make_unique<CommitStatement>();
    
    return stmt;
}

// ==================== ROLLBACK语句解析 ====================

std::unique_ptr<ASTNode> ParserTCL::parseRollbackStatement() {
    std::cout << "[PARSER DEBUG] parseRollbackStatement() called" << std::endl;
    
    consume(Type::KEYWORD_ROLLBACK);
    
    auto stmt = std::make_unique<RollbackStatement>();
    
    return stmt;
}

// ==================== BEGIN语句解析 ====================

std::unique_ptr<ASTNode> ParserTCL::parseBeginStatement() {
    std::cout << "[PARSER DEBUG] parseBeginStatement() called" << std::endl;
    
    consume(Type::KEYWORD_BEGIN);
    
    auto stmt = std::make_unique<BeginStatement>();
    
    return stmt;
}

} // namespace sql_parser
} // namespace sqlcc