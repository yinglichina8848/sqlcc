#include "ast/ast_node.h"
#include "src/sql_parser/parser.h"
#include "src/sql_parser/lexer.h"
#include "src/sql_parser/token.h"
#include "ast/ast_nodes.h"
#include "src/sql_parser/set_operation.h"
#include "src/sql_parser/architecture_safeguards.h"
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
