#ifndef SQLCC_SQL_PARSER_AST_NODE_H
#define SQLCC_SQL_PARSER_AST_NODE_H

#include <memory>
#include <string>
#include <vector>

namespace sqlcc {
namespace sql_parser {

// 前置声明
class NodeVisitor;

/**
 * AST节点基类
 */
class Node {
public:
  virtual ~Node() = default;

  /**
   * 接受访问者的访问，实现访问者模式
   * @param visitor 访问者对象
   */
  virtual void accept(NodeVisitor &visitor) = 0;
};

/**
 * AST节点访问者接口
 */
class NodeVisitor {
public:
  virtual ~NodeVisitor() = default;

  // 语句访问方法
  virtual void visit(class CreateStatement &node) = 0;
  virtual void visit(class SelectStatement &node) = 0;
  virtual void visit(class InsertStatement &node) = 0;
  virtual void visit(class UpdateStatement &node) = 0;
  virtual void visit(class DeleteStatement &node) = 0;
  virtual void visit(class DropStatement &node) = 0;
  virtual void visit(class AlterStatement &node) = 0;
  virtual void visit(class UseStatement &node) = 0;
  virtual void visit(class CreateIndexStatement &node) = 0;
  virtual void visit(class DropIndexStatement &node) = 0;
  // 事务语句访问方法
  virtual void visit(class BeginTransactionStatement &node) = 0;
  virtual void visit(class CommitStatement &node) = 0;
  virtual void visit(class RollbackStatement &node) = 0;
  virtual void visit(class SavepointStatement &node) = 0;
  virtual void visit(class SetTransactionStatement &node) = 0;

  // 表达式访问方法
  virtual void visit(class IdentifierExpression &node) = 0;
  virtual void visit(class StringLiteralExpression &node) = 0;
  virtual void visit(class NumericLiteralExpression &node) = 0;
  virtual void visit(class BinaryExpression &node) = 0;
  virtual void visit(class UnaryExpression &node) = 0;
  virtual void visit(class FunctionExpression &node) = 0;
  virtual void visit(class ExistsExpression &node) = 0;
  virtual void visit(class InExpression &node) = 0;

  // 子句访问方法
  virtual void visit(class WhereClause &node) = 0;
  virtual void visit(class JoinClause &node) = 0;
  virtual void visit(class GroupByClause &node) = 0;
  virtual void visit(class OrderByClause &node) = 0;
};

/**
 * 语句节点基类
 */
class Statement : public Node {
public:
  /**
   * 语句类型枚举
   */
  enum Type {
    CREATE,
    SELECT,
    INSERT,
    UPDATE,
    DELETE,
    DROP,
    ALTER,
    USE,
    CREATE_INDEX,
    DROP_INDEX,
    // 事务相关语句
    BEGIN_TRANSACTION,
    COMMIT,
    ROLLBACK,
    SAVEPOINT,
    SET_TRANSACTION,
    OTHER
  };

  /**
   * 构造函数
   */
  Statement(Type type = OTHER) : type_(type) {}
  
  /**
   * 获取语句类型
   */
  virtual Type getType() const { return type_; }

  /**
   * 获取语句类型的名称
   */
  std::string getTypeName() const;
  
private:
  Type type_; // 语句类型
};

/**
 * 表达式节点基类
 */
class Expression : public Node {
public:
  /**
   * 表达式类型枚举
   */
  enum Type {
    IDENTIFIER,
    STRING_LITERAL,
    NUMERIC_LITERAL,
    BINARY,
    UNARY,
    FUNCTION,
    EXISTS,
    IN
  };

  /**
   * 获取表达式类型
   */
  virtual Type getType() const = 0;

  /**
   * 获取表达式类型的名称
   */
  std::string getTypeName() const;
};

// 以下是具体节点类的前向声明
class CreateStatement;
class SelectStatement;
class InsertStatement;
class UpdateStatement;
class DeleteStatement;
class DropStatement;
class AlterStatement;
class UseStatement;
class BeginTransactionStatement;
class CommitStatement;
class RollbackStatement;
class SavepointStatement;
class SetTransactionStatement;

class IdentifierExpression;
class StringLiteralExpression;
class NumericLiteralExpression;
class BinaryExpression;
class UnaryExpression;
class FunctionExpression;

class WhereClause;
class JoinClause;
class GroupByClause;
class OrderByClause;

} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_AST_NODE_H
