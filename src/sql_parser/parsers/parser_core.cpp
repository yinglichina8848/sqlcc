/**
 * @file parser_core.cpp
 * @brief SQL语法解析器核心方法实现
 * 
 * 包含Parser类的核心解析方法，如token匹配、错误处理等基础功能。
 * 遵循单一职责原则，只包含与解析流程控制相关的方法。
 */

#include "parser_core.h"
#include "../token.h"
#include <iostream>
#include <stdexcept>

namespace sqlcc {
namespace sql_parser {

// ==================== 构造函数 ====================

ParserCore::ParserCore(TokenStream& tokens)
    : tokens_(tokens), panic_mode_(false) {
    // Note: The TokenStream should be initialized by the owner (Parser)
}

// ==================== 核心解析方法 ====================

bool ParserCore::match(Type type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

void ParserCore::consume(Type expected, const std::string& message) {
    if (check(expected)) {
        advance();
        return;
    }
    
    std::string errorMsg = message.empty() 
        ? "Expected token: " + std::to_string(static_cast<int>(expected)) + 
          " but found: " + std::to_string(static_cast<int>(tokens_.current().getType()))
        : message;
    
    throw std::runtime_error(errorMsg);
}

bool ParserCore::check(Type type) const {
    if (isAtEnd()) return false;
    return tokens_.current().getType() == type;
}

bool ParserCore::isAtEnd() const {
    return tokens_.current().getType() == Type::END_OF_INPUT;
}

void ParserCore::advance() {
    if (!isAtEnd()) {
        tokens_.advance();
    }
}

const Token& ParserCore::current() const {
    return tokens_.current();
}

Token ParserCore::previous() const {
    return tokens_.previous();
}

const Token& ParserCore::peek() {
    return tokens_.peek();
}

// ==================== 错误处理方法 ====================

void ParserCore::reportError(const std::string& message) {
    std::cerr << "[PARSER ERROR] " << message << std::endl;
    panic_mode_ = true;
}

void ParserCore::synchronize() {
    panic_mode_ = false;
    
    // 跳过token直到找到同步点
    while (!isAtEnd()) {
        if (previous().getType() == Type::SEMICOLON) {
            return;
        }
        
        // 检查当前token是否是同步点
        if (sync_tokens_.find(current().getType()) != sync_tokens_.end()) {
            return;
        }
        
        advance();
    }
}

// ==================== 语句类型判断方法 ====================

bool ParserCore::isCreateViewStatement() const {
    // CREATE VIEW - 简化判断，只检查CREATE关键字
    return check(Type::KEYWORD_CREATE);
}

bool ParserCore::isCreateUserStatement() const {
    // CREATE USER - 简化判断，只检查CREATE关键字
    return check(Type::KEYWORD_CREATE);
}

bool ParserCore::isDropUserStatement() const {
    return check(Type::KEYWORD_DROP);
}

bool ParserCore::isCreateProcedureStatement() const {
    // CREATE PROCEDURE - 简化判断，只检查CREATE关键字
    return check(Type::KEYWORD_CREATE);
}

bool ParserCore::isCreateTriggerStatement() const {
    // CREATE TRIGGER - 简化判断，只检查CREATE关键字
    return check(Type::KEYWORD_CREATE);
}

bool ParserCore::isCreateIndexStatement() const {
    return check(Type::KEYWORD_CREATE);
}

bool ParserCore::isDropIndexStatement() const {
    // DROP INDEX - 简化判断，只检查DROP关键字
    return check(Type::KEYWORD_DROP);
}

bool ParserCore::isGrantStatement() const {
    return check(Type::KEYWORD_GRANT);
}

bool ParserCore::isRevokeStatement() const {
    return check(Type::KEYWORD_REVOKE);
}

bool ParserCore::isSetOperation() const {
    return check(Type::KEYWORD_UNION) || 
           check(Type::KEYWORD_INTERSECT) || 
           check(Type::KEYWORD_EXCEPT);
}

bool ParserCore::isShowStatement() const {
    return check(Type::KEYWORD_SHOW);
}

bool ParserCore::isUseStatement() const {
    return check(Type::KEYWORD_USE);
}

bool ParserCore::isLoadDataStatement() const {
    return check(Type::KEYWORD_LOAD);
}

bool ParserCore::isCommitStatement() const {
    return check(Type::KEYWORD_COMMIT);
}

bool ParserCore::isRollbackStatement() const {
    return check(Type::KEYWORD_ROLLBACK);
}

bool ParserCore::isBeginStatement() const {
    return check(Type::KEYWORD_BEGIN);
}

// ==================== 错误信息获取 ====================

const std::vector<std::string>& ParserCore::getErrors() const {
    return errors_;
}

bool ParserCore::hasErrors() const {
    return !errors_.empty();
}

// ==================== 状态管理 ====================

void ParserCore::resetPanicMode() {
    panic_mode_ = false;
}

void ParserCore::setSyncTokens(const std::unordered_set<Type>& tokens) {
    sync_tokens_ = tokens;
}

// ==================== 通用解析助手方法 ====================

std::string ParserCore::parseIdentifier() {
    if (check(Type::IDENTIFIER)) {
        std::string identifier = tokens_.current().getLexeme();
        advance();
        return identifier;
    }
    reportError("Expected identifier");
    return "";
}

std::string ParserCore::parseQualifiedName() {
    std::string name = parseIdentifier();
    if (match(Type::DOT)) {
        name += "." + parseIdentifier();
    }
    return name;
}

std::string ParserCore::parseStringLiteral() {
    if (check(Type::STRING_LITERAL)) {
        std::string value = tokens_.current().getLexeme();
        advance();
        return value;
    }
    return "";
}

int ParserCore::parseIntLiteral() {
    if (check(Type::INTEGER_LITERAL)) {
        std::string lexeme = tokens_.current().getLexeme();
        advance();
        try {
            return std::stoi(lexeme);
        } catch (const std::exception&) {
            return 0;
        }
    }
    return 0;
}

std::vector<std::string> ParserCore::parseColumnNames() {
    std::vector<std::string> columns;
    if (match(Type::LPAREN)) {
        bool first = true;
        while (!check(Type::RPAREN) && !isAtEnd()) {
            if (!first) {
                if (!match(Type::COMMA)) {
                    break;
                }
            }
            first = false;
            std::string columnName = parseIdentifier();
            if (!columnName.empty()) {
                columns.push_back(columnName);
            }
        }
        consume(Type::RPAREN);
    }
    return columns;
}

} // namespace sql_parser
} // namespace sqlcc