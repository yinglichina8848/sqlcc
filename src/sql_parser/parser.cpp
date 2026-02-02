#include "ast/ast_node.h"
#include "parser.h"
#include "lexer.h"
#include "token.h"
#include "ast/ast_nodes.h"
#include "set_operation.h"
#include "architecture_safeguards.h"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <iterator>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

// ============================================================================
// ARCHITECTURE SAFETY COMPILE-TIME ASSERTIONS
// ============================================================================

// 架构安全断言：验证核心AST类型存在性
static_assert(sizeof(sqlcc::sql_parser::ast::Expression) > 0,
              "Expression type must exist for parsing results");

// 架构安全断言：验证Parser类是final的，防止继承
static_assert(std::is_final_v<sqlcc::sql_parser::Parser>,
              "Parser class must be final to prevent inheritance bypass of architecture safeguards");

// 架构安全断言：验证ExpressionParser相关类型存在
static_assert(sizeof(sqlcc::sql_parser::TokenStream) > 0,
              "TokenStream must exist for ExpressionParser integration");

// 架构安全断言：验证核心解析器类型约束
static_assert(!std::is_abstract_v<sqlcc::sql_parser::Parser>,
              "Parser must be instantiable for SQL parsing functionality");

// ============================================================================
// END OF COMPILE-TIME ASSERTIONS
// ============================================================================

namespace sqlcc {
namespace sql_parser {

/**
 * @class Parser
 * @brief SQL 解析器门面类 - 实现 SQL 脚本的完整生命周期解析
 *
 * WHY层 - 设计意图：
 *   一个 SQL 脚本可能包含多个不同类型的语句（DDL, DML 等）。
 *   Parser 作为主控制器，负责协调各个子解析器，并提供统一的错误同步机制，
 *   确保即使脚本中某个语句有误，也能尝试继续解析后续语句，提升解析效率和容错性。
 *
 * WHAT层 - 功能说明：
 coordinators 词法分析 (Lexer) 和标记流 (TokenStream) 的集成。
 *   主入口 parse() 循环解析完整的 SQL 文本并返回 AST 语句列表。
 *   parseStatement() 负责基于首标记（Lookahead）分发到对应的 DDL/DML/DCL/TCL 子解析器。
 *   实现恐慌模式（Panic Mode）下的自动同步（Synchronize）。
 *
 * HOW层 - 实现机制：
 *   1. 组合模式：持有 ParserDDL, ParserDML 等专项解析器的 unique_ptr。
 *   2. 递归下降：按照 SQL 文法从高层语句向下递归。
 *   3. 错误同步：initializeSyncTokens 定义了语句边界标记（如分号或关键字），同步逻辑跳过错误 Token 直至这些安全点。
 *   4. PIMPL 风格：核心逻辑委托给 ParserCore 基类和子解析器实现。
 */
Parser::Parser(const std::string& input)
    : ParserCore(tokenStream_), 
      lexer_(input), 
      tokenStream_(lexer_) {
    
    ddl_parser_ = std::make_unique<ParserDDL>(tokenStream_);
    dml_parser_ = std::make_unique<ParserDML>(tokenStream_);
    dcl_parser_ = std::make_unique<ParserDCL>(tokenStream_);
    tcl_parser_ = std::make_unique<ParserTCL>(tokenStream_);
    
    initializeSyncTokens();
}

std::vector<std::unique_ptr<Statement>> Parser::parse() {
    std::vector<std::unique_ptr<Statement>> statements;
    while (!tokenStream_.isAtEnd()) {
        try {
            if (tokenStream_.match(Type::SEMICOLON)) continue;
            
            auto stmt = parseStatement();
            if (stmt) {
                statements.push_back(std::move(stmt));
            }
            
            if (tokenStream_.check(Type::SEMICOLON)) {
                tokenStream_.expect(Type::SEMICOLON);
            }
        } catch (const std::exception& e) {
            if (!panic_mode_) {
                reportError(e.what());
            }
            synchronize();
        }
    }
    return statements;
}

std::unique_ptr<Statement> Parser::parseStatement() {
    // DDL Statements
    if (tokenStream_.check(Type::KEYWORD_CREATE)) {
        auto node = ddl_parser_->parseCreateStatement();
        return std::unique_ptr<Statement>(static_cast<Statement*>(node.release()));
    }
    if (tokenStream_.check(Type::KEYWORD_DROP)) {
        auto node = ddl_parser_->parseDropStatement();
        return std::unique_ptr<Statement>(static_cast<Statement*>(node.release()));
    }
    if (tokenStream_.check(Type::KEYWORD_ALTER)) {
        auto node = ddl_parser_->parseAlterStatement();
        return std::unique_ptr<Statement>(static_cast<Statement*>(node.release()));
    }

    // DML Statements
    if (tokenStream_.check(Type::KEYWORD_SELECT)) {
        auto node = dml_parser_->parseCompositeSelectStatement();
        return std::unique_ptr<Statement>(static_cast<Statement*>(node.release()));
    }
    if (tokenStream_.check(Type::KEYWORD_INSERT)) {
        auto node = dml_parser_->parseInsertStatement();
        return std::unique_ptr<Statement>(static_cast<Statement*>(node.release()));
    }
    if (tokenStream_.check(Type::KEYWORD_UPDATE)) {
        auto node = dml_parser_->parseUpdateStatement();
        return std::unique_ptr<Statement>(static_cast<Statement*>(node.release()));
    }
    if (tokenStream_.check(Type::KEYWORD_DELETE)) {
        auto node = dml_parser_->parseDeleteStatement();
        return std::unique_ptr<Statement>(static_cast<Statement*>(node.release()));
    }

    // DCL Statements (Grant/Revoke)
    if (isGrantStatement()) {
        auto node = dcl_parser_->parseGrantStatement();
        return std::unique_ptr<Statement>(static_cast<Statement*>(node.release()));
    }
    if (isRevokeStatement()) {
        auto node = dcl_parser_->parseRevokeStatement();
        return std::unique_ptr<Statement>(static_cast<Statement*>(node.release()));
    }

    // TCL Statements (Commit/Rollback/Begin)
    if (isCommitStatement()) {
        auto node = tcl_parser_->parseCommitStatement();
        return std::unique_ptr<Statement>(static_cast<Statement*>(node.release()));
    }
    if (isRollbackStatement()) {
        auto node = tcl_parser_->parseRollbackStatement();
        return std::unique_ptr<Statement>(static_cast<Statement*>(node.release()));
    }
    if (isBeginStatement()) {
        auto node = tcl_parser_->parseBeginStatement();
        return std::unique_ptr<Statement>(static_cast<Statement*>(node.release()));
    }

    throw std::runtime_error("Unknown statement type: " + tokenStream_.current().getLexeme());
}

void Parser::synchronize() {
    ParserCore::synchronize();
}

std::vector<std::string> Parser::getDetailedErrors() const {
    return errors_;
}

void Parser::clearErrors() {
    errors_.clear();
    panic_mode_ = false;
}

bool Parser::hadError() const {
    return !errors_.empty();
}

void Parser::initializeSyncTokens() {
    std::unordered_set<Type> syncTokens = {
        Type::SEMICOLON,
        Type::KEYWORD_SELECT,
        Type::KEYWORD_INSERT,
        Type::KEYWORD_UPDATE,
        Type::KEYWORD_DELETE,
        Type::KEYWORD_CREATE,
        Type::KEYWORD_DROP,
        Type::KEYWORD_ALTER,
        Type::KEYWORD_USE,
        Type::KEYWORD_SHOW,
        Type::KEYWORD_COMMIT,
        Type::KEYWORD_ROLLBACK,
        Type::KEYWORD_GRANT,
        Type::KEYWORD_REVOKE,
        Type::KEYWORD_BEGIN
    };
    setSyncTokens(syncTokens);
}

} // namespace sql_parser
} // namespace sqlcc
