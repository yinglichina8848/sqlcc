/**
 * SelectParser - SQL SELECT语句解析器
 *
 * WHY: 为什么需要专门的SELECT语句解析器？
 *
 * SELECT语句是SQL中最复杂的语句类型，包含多个子句：
 * - SELECT子句：列选择列表（*, 列名, 函数, 别名）
 * - FROM子句：数据源（表名, 子查询, JOIN）
 * - WHERE子句：条件过滤（复杂表达式）
 * - GROUP BY子句：分组聚合
 * - HAVING子句：分组条件过滤
 * - ORDER BY子句：结果排序
 * - LIMIT子句：结果限制（待实现）
 *
 * 将SELECT解析逻辑分离出来，可以：
 * - 简化主解析器的职责，主解析器只负责语句分派
 * - 提高代码的可维护性，每个子句有独立的解析方法
 * - 便于测试，SELECT语句的复杂逻辑可以独立测试
 * - 支持扩展，新的SELECT特性可以轻松添加
 * - 解决花括号匹配问题，避免结构性编译错误
 *
 * WHAT: SelectParser 的职责
 *
 * SelectParser 专门负责解析完整的SELECT语句：
 * - 解析SELECT关键字和DISTINCT修饰符
 * - 解析选择列表（列名、函数调用、通配符）
 * - 解析FROM子句和JOIN操作
 * - 解析WHERE条件表达式
 * - 解析GROUP BY分组列
 * - 解析HAVING条件
 * - 解析ORDER BY排序规范
 * - 构造完整的SelectStatement AST
 *
 * HOW: SELECT语句解析的实现机制
 *
 * 采用分步骤解析策略，每个子句都有专门的处理方法：
 * 1. **初始化**: 创建SelectStatement对象
 * 2. **关键字处理**: 消费SELECT, 处理DISTINCT
 * 3. **选择列表**: 解析列名、函数、别名
 * 4. **FROM子句**: 解析表名和JOIN操作
 * 5. **条件子句**: WHERE, GROUP BY, HAVING, ORDER BY
 * 6. **结果返回**: 返回完整的AST节点
 *
 * 解析顺序严格按照SQL语法：
 * SELECT → DISTINCT → 选择列表 → FROM → JOIN → WHERE → GROUP BY → HAVING → ORDER BY
 *
 * 依赖关系设计：
 * - 依赖TokenStream获取token流
 * - 依赖ExpressionParser解析复杂表达式
 * - 依赖JoinClause解析JOIN操作
 * - 通过依赖注入获得所需组件
 *
 * SOLID原则体现：
 * - 单一职责：只负责SELECT语句解析
 * - 开闭原则：新SELECT特性通过扩展实现
 * - 里氏替换：可被其他SELECT解析器替换
 * - 接口隔离：提供精确的SELECT解析接口
 * - 依赖倒置：依赖抽象接口而非具体实现
 */

#ifndef SQLCC_SQL_PARSER_SELECT_PARSER_H
#define SQLCC_SQL_PARSER_SELECT_PARSER_H

#include "token_stream.h"
#include "expression_parser.h"
#include "ast/ast_node.h"
#include <memory>
#include <vector>

namespace sqlcc {
namespace sql_parser {

/**
 * SelectParser - SQL SELECT语句解析器
 *
 * 专门负责解析复杂的SELECT语句，包括所有子句的处理。
 * 通过依赖注入获得TokenStream和ExpressionParser，确保松耦合和高可测试性。
 */
class SelectParser {
public:
  /**
   * 构造函数 - 依赖注入所需的解析器
   * @param tokens Token流，提供token访问接口
   * @param expr_parser 表达式解析器，用于解析WHERE等条件表达式
   */
  SelectParser(TokenStream& tokens, ExpressionParser& expr_parser);

  /**
   * parse - 解析完整的SELECT语句
   *
   * 这是主要的入口方法，解析从SELECT关键字开始的完整语句。
   * 处理所有SELECT子句：DISTINCT、选择列表、FROM、JOIN、WHERE、GROUP BY、HAVING、ORDER BY。
   *
   * @return 解析成功的SelectStatement AST节点
   */
  std::unique_ptr<SelectStatement> parse();

private:
  // 依赖注入的组件
  TokenStream& tokens_;
  ExpressionParser& expr_parser_;

  /**
   * parseSelectClause - 解析SELECT子句
   *
   * 处理SELECT关键字、DISTINCT修饰符和选择列表。
   * 支持*, 列名, 函数调用, 别名等多种选择项。
   *
   * @param stmt 要填充的SelectStatement对象
   */
  void parseSelectClause(SelectStatement& stmt);

  /**
   * parseFromClause - 解析FROM子句
   *
   * 处理FROM关键字、表名和JOIN操作。
   * 支持多个JOIN的复杂表连接。
   *
   * @param stmt 要填充的SelectStatement对象
   */
  void parseFromClause(SelectStatement& stmt);

  /**
   * parseJoinClause - 解析JOIN子句
   *
   * 处理各种类型的JOIN：INNER, LEFT, RIGHT, FULL OUTER。
   * 解析JOIN表名和ON条件表达式。
   *
   * @return 解析成功的JoinClause对象
   */
  std::unique_ptr<JoinClause> parseJoinClause();

  /**
   * parseWhereClause - 解析WHERE子句
   *
   * 处理WHERE关键字和条件表达式。
   * 条件可以是任意复杂的表达式。
   *
   * @param stmt 要填充的SelectStatement对象
   */
  void parseWhereClause(SelectStatement& stmt);

  /**
   * parseGroupByClause - 解析GROUP BY子句
   *
   * 处理GROUP BY关键字和分组列列表。
   * 支持多个列的分组。
   *
   * @param stmt 要填充的SelectStatement对象
   */
  void parseGroupByClause(SelectStatement& stmt);

  /**
   * parseHavingClause - 解析HAVING子句
   *
   * 处理HAVING关键字和分组条件表达式。
   * HAVING条件只能引用分组列或聚合函数。
   *
   * @param stmt 要填充的SelectStatement对象
   */
  void parseHavingClause(SelectStatement& stmt);

  /**
   * parseOrderByClause - 解析ORDER BY子句
   *
   * 处理ORDER BY关键字、排序列和方向。
   * 支持ASC/DESC排序方向。
   *
   * @param stmt 要填充的SelectStatement对象
   */
  void parseOrderByClause(SelectStatement& stmt);

  /**
   * parseSelectItem - 解析选择列表项
   *
   * 处理单个选择项：列名、函数调用、通配符。
   * 支持AS关键字定义别名。
   *
   * @return 解析的选择项字符串表示
   */
  std::string parseSelectItem();
};

} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_SELECT_PARSER_H
