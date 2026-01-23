#include "binary_expression.h"
#include "node_visitor.h"
#include "debug_printer.h"

#include <memory>
#include <iostream>

using namespace sqlcc::sql_parser;
using namespace sqlcc::sql_parser::ast;

// 最小化数字字面量实现，用于测试
class NumericLiteralExpression : public Expression {
public:
    explicit NumericLiteralExpression(int v) : value(v) {}
    int value;
    void accept(NodeVisitor& visitor) override {
        // 空实现即可
    }
};

// 简单 DebugPrintVisitor，只输出 BinaryExpression 操作符
class TestPrintVisitor : public NodeVisitor {
public:
    void visit(BinaryExpression& expr) override {
        std::cout << "BinaryExpression(";
        switch(expr.op()) {
            case OperatorKind::Add: std::cout << "Add"; break;
            case OperatorKind::Subtract: std::cout << "Subtract"; break;
            case OperatorKind::Multiply: std::cout << "Multiply"; break;
            case OperatorKind::Divide: std::cout << "Divide"; break;
            default: std::cout << "Other"; break;
        }
        std::cout << ")\n";
    }
};

int main() {
    // 构造测试 BinaryExpression
    ExprPtr lhs = std::make_unique<NumericLiteralExpression>(1);
    ExprPtr rhs = std::make_unique<NumericLiteralExpression>(2);

    BinaryExpression expr(OperatorKind::Add, std::move(lhs), std::move(rhs));

    // 调试输出
    TestPrintVisitor visitor;
    expr.accept(visitor);

    std::cout << "BinaryExpression minimal test passed.\n";
    return 0;
}
