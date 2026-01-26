/**
 * @file parser_dml.cpp
 * @brief DML语句解析器实现
 * 
 * 专门处理数据操作语言（DML）语句的解析，包括：
 * - INSERT语句
 * - UPDATE语句
 * - DELETE语句
 * - SELECT语句
 */

#include "parser_dml.h"
#include "../../token.h"
#include "../../ast/ast_nodes.h"
#include "../../select_parser.h"
#include <iostream>
#include <stdexcept>

namespace sqlcc {
namespace sql_parser {

ParserDML::ParserDML(TokenStream& tokens) : ParserCore(tokens) {
    // 初始化DML解析器
}

// ==================== INSERT语句解析 ====================

std::unique_ptr<ASTNode> ParserDML::parseInsertStatement() {
    std::cout << "[PARSER DEBUG] parseInsertStatement() called" << std::endl;
    
    consume(Type::KEYWORD_INSERT);
    consume(Type::KEYWORD_INTO);
    
    std::string tableName = parseIdentifier();
    auto stmt = std::make_unique<InsertStatement>(tableName);
    
    // 解析列列表（可选）
    if (match(Type::LPAREN)) {
        std::vector<std::string> columns;
        bool first = true;
        
        while (!check(Type::RPAREN) && !isAtEnd()) {
            if (!first) {
                consume(Type::COMMA);
            }
            first = false;
            
            std::string columnName = parseIdentifier();
            columns.push_back(columnName);
        }
        
        consume(Type::RPAREN);
        stmt->setColumnNames(columns);
    }
    
    // 解析VALUES子句
    consume(Type::KEYWORD_VALUES);
    consume(Type::LPAREN);
    
    std::vector<std::string> values;
    bool firstValue = true;
    
    while (!check(Type::RPAREN) && !isAtEnd()) {
        if (!firstValue) {
            consume(Type::COMMA);
        }
        firstValue = false;
        
        std::string value;
        if (check(Type::STRING_LITERAL)) {
            value = current().getLexeme();
            advance();
        } else if (check(Type::INTEGER_LITERAL)) {
            value = current().getLexeme();
            advance();
        } else {
            throw std::runtime_error("Expected string or integer literal in VALUES clause");
        }
        
        values.push_back(value);
    }
    
    consume(Type::RPAREN);
    stmt->setValues(values);
    
    return stmt;
}

// ==================== UPDATE语句解析 ====================

std::unique_ptr<ASTNode> ParserDML::parseUpdateStatement() {
    throw std::runtime_error("parseUpdateStatement not yet implemented");
}

// ==================== DELETE语句解析 ====================

std::unique_ptr<ASTNode> ParserDML::parseDeleteStatement() {
    std::cout << "[PARSER DEBUG] parseDeleteStatement() called" << std::endl;
    
    consume(Type::KEYWORD_DELETE);
    consume(Type::KEYWORD_FROM);
    
    std::string tableName = parseIdentifier();
    auto stmt = std::make_unique<DeleteStatement>(tableName);
    
    // 解析WHERE子句（可选）
    if (match(Type::KEYWORD_WHERE)) {
        std::string columnName = parseIdentifier();
        std::string op = current().getLexeme();
        advance(); // 消费操作符
        
        std::string value;
        if (check(Type::STRING_LITERAL)) {
            value = current().getLexeme();
            advance();
        } else if (check(Type::INTEGER_LITERAL)) {
            value = current().getLexeme();
            advance();
        } else {
            throw std::runtime_error("Expected string or integer literal in WHERE clause");
        }
        
        auto whereClause = std::make_unique<WhereClause>(columnName, op, value);
        stmt->setWhereClause(std::move(whereClause));
    }
    
    return stmt;
}

// ==================== SELECT语句解析 ====================

std::unique_ptr<ASTNode> ParserDML::parseSelectStatement() {
    std::cout << "[PARSER DEBUG] parseSelectStatement() called" << std::endl;
    
    // 委托给SelectParser处理
    SelectParser selectParser(tokens_);
    return selectParser.parse();
}

std::unique_ptr<ASTNode> ParserDML::parseCompositeSelectStatement() {
    auto left = parseSelectStatement();
    if (!left) return nullptr;

    SetOperationType op = parseSetOperationType();
    if (op == SetOperationType::NONE) return left;

    consume(Type::KEYWORD_ALL); // Optional ALL

    auto right = parseSelectStatement();
    if (!right) return nullptr;

    auto setOp = std::make_unique<SetOperation>(op, std::move(left), std::move(right));
    return setOp;
}

} // namespace sql_parser
} // namespace sqlcc