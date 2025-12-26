// Simple test for AST node compilation
#include "include/sql_parser/ast_node.h"
#include <iostream>
#include <memory>

int main() {
    using namespace sqlcc::sql_parser;

    // Test TokenType enum usage
    TokenType op = TokenType::OPERATOR_PLUS;
    std::cout << "TokenType test: " << static_cast<int>(op) << std::endl;

    // Test Expression creation
    auto expr = std::make_unique<Expression>();
    std::cout << "Expression created: " << expr->getTypeName() << std::endl;

    // Test BinaryExpression creation
    auto left = std::make_unique<Expression>();
    auto right = std::make_unique<Expression>();
    auto binaryExpr = std::make_unique<BinaryExpression>(
        std::move(left), std::move(right), TokenType::OPERATOR_PLUS);

    std::cout << "BinaryExpression created: " << binaryExpr->getTypeName() << std::endl;
    std::cout << "Operator: " << static_cast<int>(binaryExpr->getOperator()) << std::endl;

    return 0;
}
