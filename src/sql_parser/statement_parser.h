/**
 * StatementParser - SQL语句解析器
 *
 * WHY: 为什么需要专门的语句解析器？
 *
 * 语句解析器负责解析除SELECT以外的所有SQL语句：
 * - DML语句：INSERT, UPDATE, DELETE
 * - DDL语句：CREATE, DROP, ALTER
 * - 权限语句：GRANT, REVOKE
 * - 管理语句：USE, SHOW
 *
 * 将这些语句解析逻辑分离出来，可以：
 * - 简化主解析器的职责，主解析器只负责语句分派
 * - 提高代码的可维护性，每种语句有独立的解析方法
 * - 便于测试，语句解析逻辑可以独立测试
 * - 支持扩展，新的语句类型可以轻松添加
 * - 解决Parser类的过度膨胀问题
 *
 * WHAT: StatementParser 的职责
 *
 * StatementParser 专门负责解析各种SQL语句：
 * - 识别语句类型（通过关键字分派）
 * - 调用相应的专用解析方法
 * - 构造正确的AST节点
 * - 处理语句级别的语法验证
 *
 * 解析的语句类型：
 * - INSERT INTO 语句
 * - UPDATE 语句
 * - DELETE FROM 语句
 * - CREATE TABLE/INDEX/USER/VIEW 等
 * - DROP TABLE/INDEX/USER 等
 * - ALTER TABLE 语句
 * - GRANT/REVOKE 权限语句
 * - USE/SHOW 管理语句
 *
 * HOW: 语句解析的实现机制
 *
 * 采用分派模式实现：
 * 1. **入口方法**: parseStatement() 识别语句类型
 * 2. **专用方法**: 每种语句有独立的解析方法
 * 3. **辅助方法**: 提供通用的解析辅助功能
 * 4. **结果返回**: 返回相应的Statement AST节点
 *
 * 依赖关系设计：
 * - 依赖TokenStream获取token流
 * - 可能依赖ExpressionParser处理复杂表达式
 * - 通过依赖注入获得所需组件
 *
 * SOLID原则体现：
 * - 单一职责：只负责语句解析，不处理表达式
 * - 开闭原则：新语句类型通过扩展实现
 * - 里氏替换：可被其他语句解析器替换
 * - 接口隔离：提供精确的语句解析接口
 * - 依赖倒置：依赖抽象接口而非具体实现
 */

#ifndef SQLCC_SQL_PARSER_STATEMENT_PARSER_H
#define SQLCC_SQL_PARSER_STATEMENT_PARSER_H

#include "src/sql_parser/token_stream.h"
#include "src/sql_parser/expression_parser.h"
#include "ast/ast_node.h"
#include "ast/ast_nodes.h"
#include <memory>

namespace sqlcc {
namespace sql_parser {

/**
 * StatementParser - SQL语句解析器
 *
 * 专门负责解析各种SQL语句的解析器类。
 * 通过依赖注入获得TokenStream和ExpressionParser。
 */
class StatementParser {
public:
  /**
   * 构造函数 - 依赖注入所需的解析器
   * @param tokens Token流，提供token访问接口
   * @param expr_parser 表达式解析器，用于解析条件表达式
   */
  StatementParser(TokenStream& tokens, ExpressionParser& expr_parser);

  /**
   * parseStatement - 解析SQL语句
   *
   * 根据当前token识别语句类型，并调用相应的专用解析方法。
   * 支持所有主要的SQL语句类型。
   *
   * @return 解析成功的Statement AST节点
   */
  std::unique_ptr<Statement> parseStatement();

private:
  // 依赖注入的组件
  TokenStream& tokens_;
  ExpressionParser& expr_parser_;

  /**
   * parseInsertStatement - 解析INSERT语句
   *
   * 处理INSERT INTO table_name [(columns)] VALUES (values)语法。
   * 支持指定列和批量插入。
   *
   * @return InsertStatement AST节点
   */
  std::unique_ptr<InsertStatement> parseInsertStatement();

  /**
   * parseUpdateStatement - 解析UPDATE语句
   *
   * 处理UPDATE table_name SET assignments WHERE condition语法。
   *
   * @return UpdateStatement AST节点
   */
  std::unique_ptr<UpdateStatement> parseUpdateStatement();

  /**
   * parseDeleteStatement - 解析DELETE语句
   *
   * 处理DELETE FROM table_name WHERE condition语法。
   *
   * @return DeleteStatement AST节点
   */
  std::unique_ptr<DeleteStatement> parseDeleteStatement();

  /**
   * parseCreateStatement - 解析CREATE语句
   *
   * 处理各种CREATE语句：TABLE, INDEX, USER, VIEW, PROCEDURE, TRIGGER等。
   *
   * @return CreateStatement AST节点
   */
  std::unique_ptr<Statement> parseCreateStatement();

  /**
   * parseDropStatement - 解析DROP语句
   *
   * 处理各种DROP语句：TABLE, INDEX, USER等。
   *
   * @return DropStatement AST节点
   */
  std::unique_ptr<DropStatement> parseDropStatement();

  /**
   * parseAlterStatement - 解析ALTER语句
   *
   * 处理ALTER TABLE语句，支持ADD/DROP/MODIFY/RENAME操作。
   *
   * @return AlterStatement AST节点
   */
  std::unique_ptr<AlterStatement> parseAlterStatement();

  /**
   * parseGrantStatement - 解析GRANT语句
   *
   * 处理GRANT权限语句。
   *
   * @return GrantStatement AST节点
   */
  std::unique_ptr<GrantStatement> parseGrantStatement();

  /**
   * parseRevokeStatement - 解析REVOKE语句
   *
   * 处理REVOKE权限语句。
   *
   * @return RevokeStatement AST节点
   */
  std::unique_ptr<RevokeStatement> parseRevokeStatement();

  /**
   * parseUseStatement - 解析USE语句
   *
   * 处理USE database语句。
   *
   * @return UseStatement AST节点
   */
  std::unique_ptr<UseStatement> parseUseStatement();

  /**
   * parseShowStatement - 解析SHOW语句
   *
   * 处理SHOW语句。
   *
   * @return ShowStatement AST节点
   */
  std::unique_ptr<ShowStatement> parseShowStatement();

  /**
   * parseLoadDataStatement - 解析LOAD DATA语句
   *
   * 处理LOAD DATA语句。
   *
   * @return Statement AST节点
   */
  std::unique_ptr<Statement> parseLoadDataStatement();

  // 辅助方法
  /**
   * parseIdentifier - 解析标识符
   *
   * @return 标识符字符串
   */
  std::string parseIdentifier();

  /**
   * parseColumnDefinition - 解析列定义
   *
   * @return ColumnDefinition对象
   */
  std::unique_ptr<ColumnDefinition> parseColumnDefinition();

  /**
   * parseDataType - 解析数据类型
   *
   * @return 数据类型字符串
   */
  std::string parseDataType();

  /**
   * parseDefaultValue - 解析默认值
   *
   * @return 默认值字符串
   */
  std::string parseDefaultValue();

  /**
   * parseTableConstraint - 解析表级约束
   *
   * @param stmt 要添加约束的CreateStatement
   */
  void parseTableConstraint(CreateStatement& stmt);

  /**
   * parseCreateTableStatement - 解析CREATE TABLE语句
   *
   * @return CreateStatement AST节点
   */
  std::unique_ptr<CreateStatement> parseCreateTableStatement();

  /**
   * parseCreateDatabaseStatement - 解析CREATE DATABASE语句
   *
   * @return CreateStatement AST节点
   */
  std::unique_ptr<CreateStatement> parseCreateDatabaseStatement();

  /**
   * parseCreateUserStatement - 解析CREATE USER语句
   *
   * @return CreateUserStatement AST节点
   */
  std::unique_ptr<CreateUserStatement> parseCreateUserStatement();

  /**
   * parseDropUserStatement - 解析DROP USER语句
   *
   * @return DropUserStatement AST节点
   */
  std::unique_ptr<DropUserStatement> parseDropUserStatement();

  /**
   * parseCreateProcedureStatement - 解析CREATE PROCEDURE语句
   *
   * @return CreateStatement AST节点
   */
  std::unique_ptr<CreateStatement> parseCreateProcedureStatement();

  /**
   * parseCreateTriggerStatement - 解析CREATE TRIGGER语句
   *
   * @return CreateStatement AST节点
   */
  std::unique_ptr<CreateStatement> parseCreateTriggerStatement();

  /**
   * parseCreateViewStatement - 解析CREATE VIEW语句
   *
   * @return Statement AST节点
   */
  std::unique_ptr<Statement> parseCreateViewStatement();

  /**
   * parseCreateIndexStatement - 解析CREATE INDEX语句
   *
   * @return CreateIndexStatement AST节点
   */
  std::unique_ptr<CreateIndexStatement> parseCreateIndexStatement();

  /**
   * parseDropIndexStatement - 解析DROP INDEX语句
   *
   * @return DropIndexStatement AST节点
   */
  std::unique_ptr<DropIndexStatement> parseDropIndexStatement();
};

} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_STATEMENT_PARSER_H
