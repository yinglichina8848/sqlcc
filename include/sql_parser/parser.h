#ifndef SQLCC_SQL_PARSER_PARSER_H
#define SQLCC_SQL_PARSER_PARSER_H

#include "ast_node.h"
#include "ast_nodes.h"
#include "lexer.h"
#include <memory>
#include <vector>

namespace sqlcc {
namespace sql_parser {

/**
 * SQL语法分析器
 * 将Token序列解析为AST
 */
class Parser {
public:
  /**
   * 构造函数
   * @param lexer 词法分析器
   */
  explicit Parser(Lexer &lexer);

  /**
   * 析构函数
   */
  ~Parser() = default;

  /**
   * 解析SQL语句
   * @return 解析得到的语句节点
   */
  std::unique_ptr<Statement> parseStatement();

  /**
   * 解析多个SQL语句
   * @return 解析得到的语句节点列表
   */
  std::vector<std::unique_ptr<Statement>> parseStatements();

  /**
   * 解析单个SQL语句
   * @return 解析得到的语句节点
   */
  std::unique_ptr<Statement> parseSingleStatement();

  /**
   * 设置严格模式
   * @param strict 是否使用严格模式
   */
  void setStrictMode(bool strict);

  /**
   * 获取错误信息
   * @return 错误信息字符串
   */
  const std::string &getErrorMessage() const;

private:
  /**
   * 匹配并消费Token
   * @param expected 期望的Token类型
   * @return 是否匹配成功
   */
  bool match(Token::Type expected);

  /**
   * 尝试匹配Token但不消费
   * @param expected 期望的Token类型
   * @return 是否匹配成功
   */
  bool peek(Token::Type expected) const;

  /**
   * 消费当前Token
   */
  void consume();

  /**
   * 解析表达式
   * @return 表达式节点
   */
  std::unique_ptr<Expression> parseExpression();

  /**
   * 解析条件表达式
   * @return 条件表达式节点
   */
  std::unique_ptr<Expression> parseCondition();

  /**
   * 解析逻辑表达式（AND/OR）
   * @return 逻辑表达式节点
   */
  std::unique_ptr<Expression> parseLogical();

  /**
   * 解析比较表达式
   * @return 比较表达式节点
   */
  std::unique_ptr<Expression> parseComparison();

  /**
   * 解析加减表达式
   * @return 加减表达式节点
   */
  std::unique_ptr<Expression> parseAdditive();

  /**
   * 解析乘除表达式
   * @return 乘除表达式节点
   */
  std::unique_ptr<Expression> parseMultiplicative();

  /**
   * 解析一元表达式
   * @return 一元表达式节点
   */
  std::unique_ptr<Expression> parseUnary();

  /**
   * 解析原子表达式
   * @return 原子表达式节点
   */
  std::unique_ptr<Expression> parseAtomic();

  /**
   * 解析函数调用
   * @return 函数调用表达式节点
   */
  std::unique_ptr<Expression> parseFunctionCall();

  /**
   * 解析主键表达式
   * @return 主键表达式节点
   */
  std::unique_ptr<Expression> parsePrimaryExpression();

  /**
   * 解析SELECT语句（用于子查询）
   * @return SELECT语句节点
   */
  std::unique_ptr<SelectStatement> parseSelectStatement();

  /**
   * 解析SELECT语句
   * @return SELECT语句节点
   */
  std::unique_ptr<Statement> parseSelect();

  /**
   * 解析INSERT语句
   * @return INSERT语句节点
   */
  std::unique_ptr<Statement> parseInsert();

  /**
   * 解析UPDATE语句
   * @return UPDATE语句节点
   */
  std::unique_ptr<Statement> parseUpdate();

  /**
   * 解析DELETE语句
   * @return DELETE语句节点
   */
  std::unique_ptr<Statement> parseDelete();

  /**
   * 解析CREATE语句
   * @return CREATE语句节点
   */
  std::unique_ptr<Statement> parseCreate();

  /**
   * 解析DROP语句
   * @return DROP语句节点
   */
  std::unique_ptr<Statement> parseDrop();

  /**
   * 解析ALTER语句
   * @return ALTER语句节点
   */
  std::unique_ptr<Statement> parseAlter();

  /**
   * 解析USE语句
   * @return USE语句节点
   */
  std::unique_ptr<Statement> parseUse();

  /**
   * 解析CREATE INDEX语句
   * @return CREATE INDEX语句节点
   */
  std::unique_ptr<CreateIndexStatement> parseCreateIndex();

  /**
   * 解析DROP INDEX语句
   * @return DROP INDEX语句节点
   */
  std::unique_ptr<DropIndexStatement> parseDropIndex();

  /**
   * 解析事务相关语句
   * @return 事务语句节点
   */
  std::unique_ptr<Statement> parseTransactionStatement();

  /**
   * 解析BEGIN TRANSACTION语句
   * @return BEGIN TRANSACTION语句节点
   */
  std::unique_ptr<BeginTransactionStatement> parseBeginTransaction();

  /**
   * 解析COMMIT语句
   * @return COMMIT语句节点
   */
  std::unique_ptr<CommitStatement> parseCommit();

  /**
   * 解析ROLLBACK语句
   * @return ROLLBACK语句节点
   */
  std::unique_ptr<RollbackStatement> parseRollback();

  /**
   * 解析SAVEPOINT语句
   * @return SAVEPOINT语句节点
   */
  std::unique_ptr<SavepointStatement> parseSavepoint();

  /**
   * 解析SET TRANSACTION语句
   * @return SET TRANSACTION语句节点
   */
  std::unique_ptr<SetTransactionStatement> parseSetTransaction();

  /**
   * 解析CREATE DATABASE语句
   * @return CREATE语句节点
   */
  std::unique_ptr<CreateStatement> parseCreateDatabase();

  /**
   * 解析CREATE TABLE语句
   * @return CREATE语句节点
   */
  std::unique_ptr<CreateStatement> parseCreateTable();

  /**
   * 解析表引用
   * @return 表引用
   */
  TableReference parseTableReference();

  /**
   * 解析WHERE子句
   * @return WHERE子句节点
   */
  std::unique_ptr<WhereClause> parseWhereClause();

  /**
   * 解析GROUP BY子句
   * @return GROUP BY子句节点
   */
  std::unique_ptr<GroupByClause> parseGroupByClause();

  /**
   * 解析ORDER BY子句
   * @return ORDER BY子句节点
   */
  std::unique_ptr<OrderByClause> parseOrderByClause();

  /**
   * 解析列定义
   * @return 列定义
   */
  ColumnDefinition parseColumnDefinition();

  /**
   * 解析表级约束
   * @return 表级约束指针
   */
  std::unique_ptr<TableConstraint> parseTableConstraint();

  /**
   * 解析PRIMARY KEY表级约束
   * @return 主键约束指针
   */
  std::unique_ptr<PrimaryKeyConstraint> parsePrimaryKeyConstraint();

  /**
   * 解析UNIQUE表级约束
   * @return 唯一约束指针
   */
  std::unique_ptr<UniqueConstraint> parseUniqueConstraint();

  /**
   * 解析FOREIGN KEY表级约束
   * @return 外键约束指针
   */
  std::unique_ptr<ForeignKeyConstraint> parseForeignKeyConstraint();

  /**
   * 解析CHECK表级约束
   * @return CHECK约束指针
   */
  std::unique_ptr<CheckConstraint> parseCheckConstraint();

  /**
   * 解析选择项列表
   * @return 选择项列表
   */
  std::vector<SelectItem> parseSelectItems();

  /**
   * 解析JOIN子句
   * @return JOIN子句节点
   */
  std::unique_ptr<JoinClause> parseJoinClause();

  /**
   * 解析值列表
   * @return 表达式列表
   */
  std::vector<std::unique_ptr<Expression>> parseValueList();

  /**
   * 解析多行值
   * @return 多行值列表
   */
  std::vector<std::vector<std::unique_ptr<Expression>>> parseMultipleValues();

  /**
   * 报告语法错误
   * @param message 错误消息
   */
  void reportError(const std::string &message);

private:
  Lexer &lexer_;             // 词法分析器
  Token currentToken_;       // 当前Token
  bool strictMode_;          // 严格模式
  std::string errorMessage_; // 错误信息
};

} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_PARSER_H
