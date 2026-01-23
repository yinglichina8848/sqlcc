/**
 * ExpressionParser - SQL表达式解析器
 *
 * WHY: 为什么需要专门的表达式解析器？
 *
 * SQL表达式解析是解析器中最复杂的部分，涉及：
 * - 运算符优先级：正确处理不同优先级的运算符
 * - 递归嵌套：表达式可以无限嵌套（如子查询、函数调用）
 * - 类型多样性：算术、逻辑、比较、函数调用等多种表达式类型
 * - 括号处理：正确处理括号的嵌套和优先级覆盖
 *
 * 将表达式解析逻辑分离出来，可以：
 * - 提高代码的可维护性，每个类职责单一
 * - 便于测试，表达式解析逻辑可以独立测试
 * - 支持扩展，新的表达式类型可以轻松添加
 * - 减少耦合，主解析器不需要关心表达式细节
 *
 * WHAT: ExpressionParser 的职责
 *
 * ExpressionParser 专门负责解析SQL中的各种表达式：
 * - 算术表达式：a + b * c, a - b / c
 * - 逻辑表达式：a > b AND c < d, NOT condition
 * - 比较表达式：a = b, a LIKE 'pattern', a BETWEEN x AND y
 * - 函数调用：COUNT(*), MAX(price), SUBSTRING(name, 1, 3)
 * - 列引用：table.column, alias.column
 * - 字面量：数字、字符串、NULL、TRUE/FALSE
 * - 子查询：(SELECT ... FROM ...)
 *
 * HOW: 表达式解析的实现机制
 *
 * 使用经典的递归下降解析算法：
 * 1. 优先级驱动：从低优先级到高优先级逐层解析
 * 2. 左结合：大多数运算符使用左结合解析
 * 3. 预测分析：根据当前token预测解析路径
 * 4. 错误恢复：遇到错误时提供有意义的错误信息
 *
 * 解析顺序（从低到高优先级）：
 * - OR (逻辑或)
 * - AND (逻辑与)
 * - =, !=, <, >, <=, >=, LIKE, IN, BETWEEN (比较)
 * - +, - (加减)
 * - *, /, % (乘除模)
 * - NOT, unary +/-/~ (一元运算符)
 * - 函数调用、列引用、字面量、括号表达式 (原子表达式)
 *
 * SOLID原则体现：
 * - 单一职责：只负责表达式解析
 * - 开闭原则：新表达式类型通过扩展实现
 * - 里氏替换：可被其他表达式解析器替换
 * - 接口隔离：提供精确的表达式解析接口
 * - 依赖倒置：依赖TokenStream抽象而非具体实现
 */

#ifndef SQLCC_SQL_PARSER_EXPRESSION_PARSER_H
#define SQLCC_SQL_PARSER_EXPRESSION_PARSER_H

#include "token_stream.h"
#include "ast/ast_node.h"
#include <memory>
#include <functional>
#include <vector>

namespace sqlcc {
namespace sql_parser {

/**
 * ExpressionParser - SQL表达式解析器
 *
 * 专门负责解析SQL中的各种表达式类型，实现单一职责原则。
 * 通过依赖注入接收TokenStream，确保松耦合和高可测试性。
 */
class ExpressionParser {
public:
  /**
   * 构造函数 - 依赖注入TokenStream
   * @param tokens Token流，提供token访问接口
   */
  explicit ExpressionParser(TokenStream& tokens);

  /**
   * parseExpression - 表达式解析主入口
   *
   * 解析完整的SQL表达式，从最低优先级的OR运算符开始。
   * 这是一个递归下降解析器的入口点。
   *
   * @return 解析后的表达式AST节点
   */
  std::unique_ptr<Expression> parseExpression();

  /**
   * parseLogicalOr - 解析逻辑或表达式 (最低优先级)
   *
   * 处理OR运算符，具有最低的优先级。
   * 表达式形式：expr1 OR expr2 OR expr3
   *
   * @return 逻辑或表达式AST节点
   */
  std::unique_ptr<Expression> parseLogicalOr();

  /**
   * parseLogicalAnd - 解析逻辑与表达式
   *
   * 处理AND运算符，优先级高于OR。
   * 表达式形式：expr1 AND expr2 AND expr3
   *
   * @return 逻辑与表达式AST节点
   */
  std::unique_ptr<Expression> parseLogicalAnd();

  /**
   * parseEquality - 解析等式表达式
   *
   * 处理=, !=等比较运算符。
   * 表达式形式：expr1 = expr2, expr1 != expr2
   *
   * @return 等式表达式AST节点
   */
  std::unique_ptr<Expression> parseEquality();

  /**
   * parseComparison - 解析比较表达式
   *
   * 处理<, >, <=, >=等比较运算符。
   * 表达式形式：expr1 < expr2, expr1 >= expr2
   *
   * @return 比较表达式AST节点
   */
  std::unique_ptr<Expression> parseComparison();

  /**
   * parseTerm - 解析项表达式（加减法）
   *
   * 处理+和-运算符。
   * 表达式形式：expr1 + expr2, expr1 - expr2
   *
   * @return 项表达式AST节点
   */
  std::unique_ptr<Expression> parseTerm();

  /**
   * parseFactor - 解析因子表达式（乘除法）
   *
   * 处理*, /和%运算符。
   * 表达式形式：expr1 * expr2, expr1 / expr2, expr1 % expr2
   *
   * @return 因子表达式AST节点
   */
  std::unique_ptr<Expression> parseFactor();

  /**
   * parseUnary - 解析一元表达式
   *
   * 处理NOT, unary +/-/~等一元运算符。
   * 表达式形式：NOT expr, -expr, +expr
   *
   * @return 一元表达式AST节点
   */
  std::unique_ptr<Expression> parseUnary();

  /**
   * parsePrimary - 解析基本表达式（原子表达式）
   *
   * 处理最基本的表达式单元：
   * - 字面量：数字、字符串、NULL、TRUE/FALSE
   * - 列引用：table.column, column
   * - 函数调用：func(args)
   * - 括号表达式：(expr)
   * - 子查询：(SELECT ...)
   *
   * @return 基本表达式AST节点
   */
  std::unique_ptr<Expression> parsePrimary();

  /**
   * parseIdentifierExpression - 解析标识符表达式
   *
   * 处理列引用和函数调用：
   * - 简单列：column
   * - 限定列：table.column, alias.column
   * - 函数调用：func(expr1, expr2, ...)
   *
   * @return 标识符表达式AST节点
   */
  std::unique_ptr<Expression> parseIdentifierExpression();

private:
  // Token流引用，通过依赖注入获得
  TokenStream& tokens_;

  // 二元运算符解析辅助函数
  std::unique_ptr<Expression> parseBinaryOp(
      std::function<std::unique_ptr<Expression>()> parseNextLevel,
      const std::vector<Type>& operators);
};

} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_EXPRESSION_PARSER_H
