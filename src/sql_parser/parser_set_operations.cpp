#include "ast/ast_node.h"
#include "token.h"
#include "parser.h"
#include "set_operation.h"
#include <memory>
#include <stdexcept>

namespace sqlcc {
namespace sql_parser {

// ==================== 集合操作解析方法 ====================

std::unique_ptr<Statement> Parser::parseCompositeSelectStatement() {
  // 解析第一个SELECT语句
  auto firstSelect = parseSelectStatement();
  if (!firstSelect) {
    return nullptr;
  }

  // 检查是否有集合操作
  if (!isSetOperation()) {
    // 如果没有集合操作，返回简单的SELECT语句
    return firstSelect;
  }

  // 创建复合SELECT语句
  auto compositeStmt = std::make_unique<CompositeSelectStatement>();
  compositeStmt->addSelectStatement(std::move(firstSelect));

  // 解析所有集合操作
  while (isSetOperation()) {
    auto setOperation = parseSetOperation();
    if (setOperation) {
      compositeStmt->addSetOperation(std::move(setOperation));
    } else {
      reportError("Failed to parse set operation");
      return nullptr;
    }
  }

  return compositeStmt;
}

std::unique_ptr<SetOperation> Parser::parseSetOperation() {
  // 解析集合操作类型
  SetOperationType operationType = parseSetOperationType();

  // 解析可选的ALL关键字
  bool allFlag = false;
  if (match(Token::KEYWORD_ALL)) {
    allFlag = true;
    consume(Token::KEYWORD_ALL);
  }

  // 解析右侧的SELECT语句
  auto rightOperand = parseSelectStatement();
  if (!rightOperand) {
    reportError("Expected SELECT statement after set operation");
    return nullptr;
  }

  // 创建集合操作节点
  // 注意：左侧操作数由CompositeSelectStatement在添加操作时设置
  return std::make_unique<SetOperation>(operationType, 
                                        nullptr,
                                        std::move(rightOperand), 
                                        allFlag);
}

SetOperationType Parser::parseSetOperationType() {
  if (match(Token::KEYWORD_UNION)) {
    consume(Token::KEYWORD_UNION);
    return SetOperationType::UNION;
  } else if (match(Token::KEYWORD_INTERSECT)) {
    consume(Token::KEYWORD_INTERSECT);
    return SetOperationType::INTERSECT;
  } else if (match(Token::KEYWORD_EXCEPT)) {
    consume(Token::KEYWORD_EXCEPT);
    return SetOperationType::EXCEPT;
  } else {
    reportError("Expected set operation keyword (UNION, INTERSECT, or EXCEPT)");
    return SetOperationType::UNION; // 默认返回UNION
  }
}

bool Parser::isSetOperation() const {
  return check(Token::KEYWORD_UNION) || 
         check(Token::KEYWORD_INTERSECT) || 
         check(Token::KEYWORD_EXCEPT);
}

// ==================== 修改现有的SELECT语句解析 ====================

// 需要修改parseStatement方法以支持复合SELECT语句
// 在parseStatement方法中，将SELECT语句的解析改为：
// } else if (match(Token::KEYWORD_SELECT)) {
//     return parseCompositeSelectStatement();

} // namespace sql_parser
} // namespace sqlcc
