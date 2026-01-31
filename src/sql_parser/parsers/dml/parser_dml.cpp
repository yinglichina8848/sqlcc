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
#include "../../expression_parser.h"
#include <iostream>
#include <stdexcept>

namespace sqlcc {
namespace sql_parser {

// PIMPL实现类
class ParserDML::Impl {
public:
    explicit Impl(TokenStream& tokens) 
        : expr_parser_(tokens), select_parser_(tokens, expr_parser_) {}
    
    std::unique_ptr<SelectStatement> parseSelectStatement() {
        std::cout << "[PARSER DEBUG] parseSelectStatement() called" << std::endl;
        return select_parser_.parse();
    }
    
private:
    ExpressionParser expr_parser_;
    SelectParser select_parser_;
};

ParserDML::ParserDML(TokenStream& tokens) : ParserCore(tokens), impl_(std::make_unique<Impl>(tokens)) {
    // 初始化DML解析器
}

ParserDML::~ParserDML() = default;

// ==================== INSERT语句解析 ====================

std::unique_ptr<ASTNode> ParserDML::parseInsertStatement() {
    // WHY: INSERT 语句负责向表添加新数据。解析器必须准确识别目标表、目标列及对应的值。
    // WHAT: 解析 INSERT INTO <table> (cols...) VALUES (vals...) 结构。
    // HOW:
    // 1. 消费 INSERT 和 INTO 关键字。
    // 2. 提取表名标识符。
    // 3. （可选）解析圆括号内的列名列表。
    // 4. 消费 VALUES 关键字。
    // 5. 解析圆括号内的字面量列表，并校验其类型。
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
    
    bool firstValue = true;
    
    while (!check(Type::RPAREN) && !isAtEnd()) {
        if (!firstValue) {
            consume(Type::COMMA);
        }
        firstValue = false;
        
        if (check(Type::STRING_LITERAL) || check(Type::INTEGER_LITERAL)) {
            stmt->addValue(current().getLexeme());
            advance();
        } else {
            throw std::runtime_error("Expected string or integer literal in VALUES clause");
        }
    }
    
    consume(Type::RPAREN);
    
    return stmt;
}

// ==================== UPDATE语句解析 ====================

std::unique_ptr<ASTNode> ParserDML::parseUpdateStatement() {
    throw std::runtime_error("parseUpdateStatement not yet implemented");
}

// ==================== DELETE语句解析 ====================

std::unique_ptr<ASTNode> ParserDML::parseDeleteStatement() {
    // WHY: DELETE 语句用于移除表中满足条件的记录。
    // WHAT: 解析 DELETE FROM <table> WHERE <expr> 结构。
    // HOW:
    // 1. 消费 DELETE 和 FROM 关键字。
    // 2. 提取目标表名。
    // 3. （可选）解析 WHERE 子句中的过滤表达式（目前待完整 Expression 架构接入）。
    std::cout << "[PARSER DEBUG] parseDeleteStatement() called" << std::endl;
    
    consume(Type::KEYWORD_DELETE);
    consume(Type::KEYWORD_FROM);
    
    std::string tableName = parseIdentifier();
    std::vector<std::string> tableNames = {tableName};
    auto stmt = std::make_unique<DeleteStatement>(tableNames);
    
    // 解析WHERE子句（可选）
    if (match(Type::KEYWORD_WHERE)) {
        // 临时方案：暂时注释WHERE子句解析，待完整实现Expression架构
        // TODO: 实现基于Expression的WHERE子句解析
        throw std::runtime_error("WHERE clause parsing not yet adapted to new Expression architecture");
    }
    
    return stmt;
}

// ==================== SELECT语句解析 ====================

std::unique_ptr<SelectStatement> ParserDML::parseSelectStatement() {
    std::cout << "[PARSER DEBUG] ParserDML::parseSelectStatement() called" << std::endl;
    return impl_->parseSelectStatement();
}

std::unique_ptr<ASTNode> ParserDML::parseCompositeSelectStatement() {
    auto left = parseSelectStatement();
    if (!left) return nullptr;

    SetOperationType op = parseSetOperationType();
    if (op == SetOperationType::NONE) return left;

    bool all = match(Type::KEYWORD_ALL);

    auto right = parseSelectStatement();
    if (!right) return nullptr;

    auto setOp = std::make_unique<SetOperation>(op, std::move(left), std::move(right), all);
    return setOp;
}

SetOperationType ParserDML::parseSetOperationType() {
    if (match(Type::KEYWORD_UNION)) return SetOperationType::UNION;
    if (match(Type::KEYWORD_INTERSECT)) return SetOperationType::INTERSECT;
    if (match(Type::KEYWORD_EXCEPT)) return SetOperationType::EXCEPT;
    return SetOperationType::NONE;
}

std::unique_ptr<SetOperation> ParserDML::parseSetOperation() {
    auto node = parseCompositeSelectStatement();
    // 尝试将 ASTNode 转换为 SetOperation
    if (auto setOp = dynamic_cast<SetOperation*>(node.get())) {
        // 由于 node 是 unique_ptr，我们不能直接 release 并返回，
        // 且 parseCompositeSelectStatement 可能返回 SelectStatement。
        // 这里简化处理，如果确实需要 SetOperation，调用方应该知道。
        // 但由于接口限制，这里可能需要重构。
    }
    return nullptr; // 临时占位，待进一步处理
}

} // namespace sql_parser
} // namespace sqlcc