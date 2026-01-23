#include "binary_expression.h"
#include "literal_expressions.h"
#include "node_visitor.h"
#include "debug_printer.h"

#include <memory>
#include <iostream>

using namespace sqlcc::sql_parser;
using namespace sqlcc::sql_parser::ast;

// 简单 DebugPrintVisitor，只输出 BinaryExpression 操作符
class TestPrintVisitor : public NodeVisitor {
public:
    void visitBinaryExpression(BinaryExpression& expr) override {
        std::cout << "BinaryExpression(";
        switch(expr.getOperator()) {
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
    auto lhs = std::make_unique<NumericLiteralExpression>(1.0);
    auto rhs = std::make_unique<NumericLiteralExpression>(2.0);

    BinaryExpression expr(std::move(lhs), OperatorKind::Add, std::move(rhs));

    // 调试输出
    TestPrintVisitor visitor;
    expr.accept(visitor);

    std::cout << "BinaryExpression minimal test passed.\n";
    return 0;
}
