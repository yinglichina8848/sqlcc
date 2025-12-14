#include "sql_parser/set_operation.h"
#include "sql_parser/ast_nodes.h"
#include <stdexcept>

namespace sqlcc {
namespace sql_parser {

SetOperation::SetOperation(SetOperationType operationType,
                           std::unique_ptr<SelectStatement> leftOperand,
                           std::unique_ptr<SelectStatement> rightOperand,
                           bool allFlag)
        : Statement(Statement::COMPOSITE_SELECT),
            operationType_(operationType),
      leftOperand_(std::move(leftOperand)),
      rightOperand_(std::move(rightOperand)),
      allFlag_(allFlag) {
    switch (operationType_) {
        case SetOperationType::UNION:
            operationName_ = "UNION";
            break;
        case SetOperationType::INTERSECT:
            operationName_ = "INTERSECT";
            break;
        case SetOperationType::EXCEPT:
            operationName_ = "EXCEPT";
            break;
        default:
            throw std::invalid_argument("Invalid set operation type");
    }
}

SetOperation::~SetOperation() = default;

SetOperationType SetOperation::getOperationType() const {
    return operationType_;
}

const std::string& SetOperation::getOperationName() const {
    return operationName_;
}

SelectStatement* SetOperation::getLeftOperand() const {
    return leftOperand_.get();
}

SelectStatement* SetOperation::getRightOperand() const {
    return rightOperand_.get();
}

bool SetOperation::isAll() const {
    return allFlag_;
}

void SetOperation::accept(NodeVisitor& visitor) {
    visitor.visit(*this);
}

} // namespace sql_parser
} // namespace sqlcc