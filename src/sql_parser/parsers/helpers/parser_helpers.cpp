/**
 * @file parser_helpers.cpp
 * @brief 解析器辅助方法实现
 * 
 * 包含各种辅助解析方法，如标识符解析、限定名解析、表达式列表解析等。
 * 这些方法被多个解析模块共享使用。
 */

#include "src/sql_parser/parsers/helpers/parser_helpers.h"
#include "../parser_core.h"
#include "../../expression_parser.h"
#include "../../token.h"
#include <iostream>
#include <stdexcept>

namespace sqlcc {
namespace sql_parser {

ParserHelpers::ParserHelpers(TokenStream& tokens) : ParserCore(tokens) {
    expression_parser_ = std::make_unique<ExpressionParser>(tokens);
}

// ==================== 辅助解析方法 ====================

std::vector<std::string> ParserHelpers::parseColumnNames() {
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

std::vector<std::unique_ptr<Expression>> ParserHelpers::parseExpressions() {
    std::vector<std::unique_ptr<Expression>> expressions;
    bool first = true;
    while (!check(Type::KEYWORD_FROM) && !check(Type::KEYWORD_WHERE) &&
           !check(Type::KEYWORD_GROUP) && !check(Type::KEYWORD_ORDER) &&
           !check(Type::KEYWORD_LIMIT) && !check(Type::SEMICOLON) && !isAtEnd()) {
        if (!first) {
            if (!match(Type::COMMA)) {
                break;
            }
        }
        first = false;
        auto expr = parseExpression();
        if (expr) {
            expressions.push_back(std::move(expr));
        }
    }
    return expressions;
}

std::string ParserHelpers::parseQualifiedName() {
    std::string name = parseIdentifier();
    if (match(Type::DOT)) {
        name += "." + parseIdentifier();
    }
    return name;
}

std::string ParserHelpers::parseStringLiteral() {
    if (check(Type::STRING_LITERAL)) {
        std::string value = current().getLexeme();
        advance();
        return value;
    }
    return "";
}

int64_t ParserHelpers::parseIntLiteral() {
    if (check(Type::INTEGER_LITERAL)) {
        std::string lexeme = current().getLexeme();
        advance();
        try {
            return std::stoll(lexeme);
        } catch (const std::exception&) {
            return 0;
        }
    }
    return 0;
}

double ParserHelpers::parseFloatLiteral() {
    if (check(Type::FLOAT_LITERAL)) {
        std::string lexeme = current().getLexeme();
        advance();
        try {
            return std::stod(lexeme);
        } catch (const std::exception&) {
            return 0.0;
        }
    }
    return 0.0;
}

std::string ParserHelpers::parseIdentifier() {
    if (check(Type::IDENTIFIER)) {
        std::string identifier = current().getLexeme();
        advance();
        return identifier;
    }
    return "";
}

// ==================== 集合操作解析 ====================

std::unique_ptr<SetOperation> ParserHelpers::parseSetOperation() {
    SetOperationType type = parseSetOperationType();
    if (type == SetOperationType::NONE) return nullptr;

    consume(Type::KEYWORD_ALL); // Optional ALL
    // SetOperation needs operands, this method seems incomplete
    // For now, return nullptr to avoid compilation error
    return nullptr;
}

std::unique_ptr<SetOperation> ParserHelpers::parseUnion() {
    if (match(Type::KEYWORD_UNION)) {
        consume(Type::KEYWORD_ALL); // Optional ALL
        // SetOperation needs operands, this method seems incomplete
        // For now, return nullptr to avoid compilation error
        return nullptr;
    }
    return nullptr;
}

std::unique_ptr<SetOperation> ParserHelpers::parseIntersect() {
    if (match(Type::KEYWORD_INTERSECT)) {
        // SetOperation needs operands, this method seems incomplete
        // For now, return nullptr to avoid compilation error
        return nullptr;
    }
    return nullptr;
}

std::unique_ptr<SetOperation> ParserHelpers::parseExcept() {
    if (match(Type::KEYWORD_EXCEPT)) {
        // SetOperation needs operands, this method seems incomplete
        // For now, return nullptr to avoid compilation error
        return nullptr;
    }
    return nullptr;
}

SetOperationType ParserHelpers::parseSetOperationType() {
    if (match(Type::KEYWORD_UNION)) return SetOperationType::UNION;
    if (match(Type::KEYWORD_INTERSECT)) return SetOperationType::INTERSECT;
    if (match(Type::KEYWORD_EXCEPT)) return SetOperationType::EXCEPT;
    return SetOperationType::NONE;
}

bool ParserHelpers::isSetOperation() {
    return check(Type::KEYWORD_UNION) || check(Type::KEYWORD_INTERSECT) || check(Type::KEYWORD_EXCEPT);
}

// ==================== 其他辅助语句解析 ====================

std::unique_ptr<ASTNode> ParserHelpers::parseUseStatement() {
    std::cout << "[PARSER DEBUG] parseUseStatement() called" << std::endl;
    
    consume(Type::KEYWORD_USE);
    std::string databaseName = parseIdentifier();
    
    auto stmt = std::make_unique<UseStatement>(databaseName);
    
    return stmt;
}

std::unique_ptr<ASTNode> ParserHelpers::parseShowStatement() {
    throw std::runtime_error("parseShowStatement not yet implemented");
}

std::unique_ptr<ASTNode> ParserHelpers::parseLoadDataStatement() {
    throw std::runtime_error("parseLoadDataStatement not yet implemented");
}

// ==================== 表达式解析委托 ====================

std::unique_ptr<Expression> ParserHelpers::parseExpression() {
    return expression_parser_->parseExpression();
}

} // namespace sql_parser
} // namespace sqlcc