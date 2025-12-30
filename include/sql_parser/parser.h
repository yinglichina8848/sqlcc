#include "sql_parser/ast_node.h"
#ifndef SQLCC_SQL_PARSER_PARSER_H
#define SQLCC_SQL_PARSER_PARSER_H

#include "sql_parser/ast_nodes.h"
#include "sql_parser/constraint.h"
#include "sql_parser/set_operation.h"
#include "sql_parser/token.h"
#include "sql_parser/window_function.h"
#include "sql_parser/lexer.h"
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

namespace sqlcc {
namespace sql_parser {

  /**
   * WHY: 为什么选择递归下降解析器？
   *
   * 数据库系统需要处理复杂的SQL语法，支持递归嵌套结构（如子查询、嵌套表达式）。
   * 递归下降解析器具有以下优势：
   * 1. 代码结构清晰，每个非终结符对应一个函数
   * 2. 错误恢复能力强，便于提供精确的错误信息
   * 3. 易于维护和扩展，支持新语法规则的添加
   * 4. 执行效率高，无需复杂的解析表
   *
   * 设计权衡：
   * - 优点：实现简单，错误定位准确
   * - 缺点：可能存在左递归问题（通过重构文法解决）
   * - 替代方案：LL/LR解析器，但复杂度更高
   */
class Parser {
public:
  /**
   * @brief Parser构造函数
   * @param input SQL输入字符串，由词法分析器处理
   */
  Parser(const std::string& input);

  /**
   * WHAT: parse - 解析SQL语句的主入口

   * 处理完整的SQL脚本，包含多个语句。返回解析后的AST节点列表。

   * HOW: 循环调用parseStatement()直到输入结束
   * 1. 初始化词法分析器
   * 2. 循环解析每个语句
   * 3. 处理语句分隔符（分号）
   * 4. 收集所有解析结果
   */
  std::vector<std::unique_ptr<Statement>> parse();

private:
  // Token stream management
  Lexer lexer_;
  Token currentToken_;
  Token lookaheadToken_;
  bool hasLookahead_;

  // Error recovery
  std::vector<std::string> errors_;
  bool panicMode_;
  std::unordered_set<Token::Type> syncTokens_;

  // Core parsing methods
  void advance();
  bool match(Token::Type type);
  void consume(Token::Type type);
  bool check(Token::Type type) const;
  bool isAtEnd() const;
  Token peek() const;
  Token previous() const;

  // Error handling
  void reportError(const std::string &message);

  /**
   * WHAT: synchronize - 错误恢复机制
   *
   * 当解析遇到语法错误时，不是简单停止，而是尝试恢复到可以继续解析的状态。
   * 这使得解析器能够报告多个错误，而不是只报告第一个错误。
   *
   * HOW: 使用同步词集合
   * - 维护当前语句的同步词列表（如SELECT, FROM, WHERE等关键字）
   * - 跳过错误token直到遇到同步词
   * - 重新开始解析下一个语句
   */
  void synchronize();

  bool hadError() const;

  // Helper method to check if current statement is CREATE VIEW
  bool isCreateViewStatement();

  // Statement parsing (strict BNF compliance)
  std::unique_ptr<Statement> parseStatement();
  std::unique_ptr<CreateStatement> parseCreateStatement();
  std::unique_ptr<CreateStatement> parseCreateTableStatement();
  std::unique_ptr<CreateStatement> parseCreateDatabaseStatement();
  std::unique_ptr<CreateStatement> parseCreateProcedureStatement();
  std::unique_ptr<CreateStatement> parseCreateTriggerStatement();
  std::unique_ptr<Statement> parseCreateViewStatement();
  std::unique_ptr<DropStatement> parseDropStatement();
  std::unique_ptr<AlterStatement> parseAlterStatement();
  std::unique_ptr<SelectStatement> parseSelectStatement();
  std::unique_ptr<InsertStatement> parseInsertStatement();
  std::unique_ptr<UpdateStatement> parseUpdateStatement();
  std::unique_ptr<DeleteStatement> parseDeleteStatement();
  std::unique_ptr<UseStatement> parseUseStatement();
  std::unique_ptr<ShowStatement> parseShowStatement();
  std::unique_ptr<CreateIndexStatement> parseCreateIndexStatement();
  std::unique_ptr<DropIndexStatement> parseDropIndexStatement();
  std::unique_ptr<CreateUserStatement> parseCreateUserStatement();
  std::unique_ptr<DropUserStatement> parseDropUserStatement();
  std::unique_ptr<GrantStatement> parseGrantStatement();
  std::unique_ptr<RevokeStatement> parseRevokeStatement();

  // LOAD DATA statement parsing
  std::unique_ptr<Statement> parseLoadDataStatement();

  // Clause parsing
  std::vector<std::string> parseColumnNames();
  std::vector<std::unique_ptr<Expression>> parseExpressions();

  /**
   * WHAT: parseExpression - 解析SQL表达式
   *
   * 处理SQL中的各种表达式类型：
   * - 算术表达式：a + b * c
   * - 逻辑表达式：a > b AND c < d
   * - 函数调用：COUNT(*), MAX(price)
   * - 子查询：(SELECT ... FROM ...)
   *
   * HOW: 使用运算符优先级驱动的递归解析
   * 1. 从低优先级开始解析（OR, AND, NOT）
   * 2. 递归调用更高优先级的解析函数
   * 3. 处理括号和函数调用
   * 4. 构建AST节点表示表达式树
   */
  std::unique_ptr<Expression> parseExpression();

  /**
   * WHAT: parseLogicalOr - 解析逻辑或表达式
   *
   * 处理OR运算符，具有最低的优先级。
   * 表达式：expr1 OR expr2 OR expr3
   *
   * HOW: 左结合解析
   * 1. 先解析左边的AND表达式
   * 2. 如果遇到OR，递归解析右边
   * 3. 构建二元运算符节点
   */
  std::unique_ptr<Expression> parseLogicalOr();
  std::unique_ptr<Expression> parseLogicalAnd();
  std::unique_ptr<Expression> parseEquality();
  std::unique_ptr<Expression> parseComparison();
  std::unique_ptr<Expression> parseTerm();
  std::unique_ptr<Expression> parseFactor();
  std::unique_ptr<Expression> parseUnary();
  std::unique_ptr<Expression> parsePrimary();
  std::unique_ptr<Expression> parseIdentifierExpression();

  // JOIN clause parsing
  std::unique_ptr<JoinClause> parseJoinClause();

  std::vector<std::unique_ptr<ColumnDefinition>> parseColumnDefinitions();
  std::unique_ptr<ColumnDefinition> parseColumnDefinition();
  std::string parseDataType();
  std::string parseDefaultValue();
  void parseTableConstraint(CreateStatement& stmt);

  // Helper methods
  void initializeSyncTokens();
  std::string parseQualifiedName();
  std::string parseIdentifier();
  std::string parseStringLiteral();
  int parseIntLiteral();

  // Set operation parsing
  std::unique_ptr<Statement> parseCompositeSelectStatement();
  std::unique_ptr<SetOperation> parseSetOperation();
  std::unique_ptr<SetOperation> parseUnion();
  std::unique_ptr<SetOperation> parseIntersect();
  std::unique_ptr<SetOperation> parseExcept();
  // Helpers for set-operation parsing
  SetOperationType parseSetOperationType();
  bool isSetOperation() const;
};

} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_PARSER_H
