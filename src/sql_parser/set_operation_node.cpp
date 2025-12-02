#include "../../include/sql_parser/set_operation_node.h"
#include "../../include/sql_parser/ast_nodes.h"
#include <string>

namespace sqlcc {
namespace sql_parser {

// ==================== SetOperationNode ====================

SetOperationNode::SetOperationNode(SetOperationType operationType,
                                  std::unique_ptr<SelectStatement> leftOperand,
                                  std::unique_ptr<SelectStatement> rightOperand,
                                  bool allFlag)
    : Statement(Statement::SELECT), // 使用SELECT类型作为基础类型
      operationType_(operationType),
      leftOperand_(std::move(leftOperand)),
      rightOperand_(std::move(rightOperand)),
      allFlag_(allFlag) {
}

SetOperationNode::~SetOperationNode() {
}

SetOperationType SetOperationNode::getOperationType() const {
    return operationType_;
}

const std::string& SetOperationNode::getOperationName() const {
    static const std::string unionName = "UNION";
    static const std::string intersectName = "INTERSECT";
    static const std::string exceptName = "EXCEPT";
    
    switch (operationType_) {
        case SetOperationType::UNION:
            return unionName;
        case SetOperationType::INTERSECT:
            return intersectName;
        case SetOperationType::EXCEPT:
            return exceptName;
        default:
            return unionName; // 默认返回UNION
    }
}

SelectStatement* SetOperationNode::getLeftOperand() const {
    return leftOperand_.get();
}

SelectStatement* SetOperationNode::getRightOperand() const {
    return rightOperand_.get();
}

bool SetOperationNode::isAll() const {
    return allFlag_;
}

std::string SetOperationNode::getOperationTypeName(SetOperationType type) {
    switch (type) {
        case SetOperationType::UNION:
            return "UNION";
        case SetOperationType::INTERSECT:
            return "INTERSECT";
        case SetOperationType::EXCEPT:
            return "EXCEPT";
        default:
            return "UNKNOWN";
    }
}

// ==================== CompositeSelectStatement ====================

CompositeSelectStatement::CompositeSelectStatement()
    : Statement(Statement::SELECT) {
}

CompositeSelectStatement::~CompositeSelectStatement() {
}

void CompositeSelectStatement::addSelectStatement(std::unique_ptr<SelectStatement> selectStmt) {
    selectStatements_.push_back(std::move(selectStmt));
}

void CompositeSelectStatement::addSetOperation(std::unique_ptr<SetOperationNode> setOperation) {
    setOperations_.push_back(std::move(setOperation));
}

const std::vector<std::unique_ptr<SelectStatement>>& CompositeSelectStatement::getSelectStatements() const {
    return selectStatements_;
}

const std::vector<std::unique_ptr<SetOperationNode>>& CompositeSelectStatement::getSetOperations() const {
    return setOperations_;
}

bool CompositeSelectStatement::isSimpleSelect() const {
    return selectStatements_.size() == 1 && setOperations_.empty();
}

bool CompositeSelectStatement::hasSetOperations() const {
    return !setOperations_.empty();
}

size_t CompositeSelectStatement::getStatementCount() const {
    return selectStatements_.size();
}

size_t CompositeSelectStatement::getOperationCount() const {
    return setOperations_.size();
}

} // namespace sql_parser
} // namespace sqlcc