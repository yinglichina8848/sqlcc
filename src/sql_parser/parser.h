/**
 * WHY: 为什么需要专门的SQL解析器？
 *
 * 数据库系统需要将用户输入的SQL字符串转换为可执行的内部表示，传统解析方案存在诸多问题：
 * - 语法复杂：SQL语法包含递归嵌套结构、运算符优先级、多重子句
 * - 错误定位：解析错误时需要提供精确的位置和原因信息
 * - 性能要求：解析过程需要高效，避免成为系统瓶颈
 * - 扩展性：需要支持新SQL特性的快速添加
 * - 标准兼容：必须完全符合SQL标准语法规范
 *
 * SQL解析器的核心价值：
 * 1. 语法正确性：严格验证SQL语句的语法和语义正确性
 * 2. 结构化表示：将SQL字符串转换为AST树结构
 * 3. 错误恢复：提供友好的错误信息和解析继续能力
 * 4. 性能优化：高效的解析算法和内存管理
 * 5. 标准兼容：完全支持SQL标准的各种语法特性
 *
 * 🏗️ 设计模式：递归下降解析器(Recursive Descent Parser)
 *
 * 递归下降解析器作为编译原理经典模式的应用：
 * - 自顶向下：从语法树的根节点开始逐步构建
 * - 函数映射：每个非终结符对应一个解析函数
 * - 预测分析：根据当前token预测解析路径
 * - 错误恢复：遇到错误时尝试恢复继续解析
 * - 状态管理：维护解析状态和错误信息
 *
 * SOLID原则体现：
 * - 单一职责：解析器类负责SQL语句的语法分析
 * - 开闭原则：新语法规则通过扩展现有函数实现
 * - 里氏替换：解析器子类可以替换基类使用
 * - 接口隔离：解析接口精确定义所需方法
 * - 依赖倒置：高层模块依赖解析接口而非实现
 *
 * WHAT: SQL解析器系统 - 完整的SQL语法分析框架
 *
 * 核心功能：
 * - 词法分析：将SQL字符串转换为token流
 * - 语法分析：将token流转换为AST树结构
 * - 语义验证：检查SQL语句的语义正确性
 * - 错误处理：提供详细的语法错误信息和恢复机制
 * - 多种语句：支持DDL、DML、DCL、TCL等多种SQL语句类型
 *
 * 系统组件：
 * - Parser：主要的解析器类，协调整个解析过程
 * - Lexer：词法分析器，生成token流
 * - AST节点：各种SQL语法结构的树节点表示
 * - 错误处理器：收集和报告解析过程中的错误信息
 * - 同步机制：错误恢复时用于重新同步的token集合
 *
 * 支持的SQL语句类型：
 * - DDL语句：CREATE、ALTER、DROP等数据定义语句
 * - DML语句：SELECT、INSERT、UPDATE、DELETE等数据操作语句
 * - DCL语句：GRANT、REVOKE等数据控制语句
 * - TCL语句：COMMIT、ROLLBACK等事务控制语句
 * - 扩展语句：SHOW、USE、LOAD DATA等数据库特定语句
 *
 * 解析流程：
 * - 预处理：初始化词法分析器和解析状态
 * - 循环解析：逐个解析SQL脚本中的语句
 * - 语法检查：验证每个语句的语法正确性
 * - 语义分析：进行必要的语义验证和类型检查
 * - 错误收集：收集所有解析过程中的错误信息
 * - 结果返回：返回解析成功的AST节点列表
 *
 * 错误处理机制：
 * - 语法错误：检测不符合SQL语法规则的情况
 * - 语义错误：检测语义不正确但语法正确的情况
 * - 位置信息：提供错误的精确位置信息
 * - 错误恢复：尝试从错误中恢复继续解析
 * - 多重错误：收集多个错误而不立即停止解析
 *
 * 接口设计：
 * - 构造函数：使用SQL字符串初始化解析器
 * - 解析方法：将SQL字符串转换为AST节点列表
 * - 错误查询：获取解析过程中产生的错误信息
 * - 状态管理：管理解析器的内部状态和同步信息
 *
 * HOW: SQL解析器系统的实现机制
 *
 * 解析器初始化流程：
 * 1. 词法分析器：创建Lexer对象处理输入字符串
 * 2. 状态初始化：设置当前位置和前瞻token
 * 3. 同步集合：初始化用于错误恢复的同步token集合
 * 4. 错误列表：准备收集解析错误的容器
 * 5. 模式设置：设置正常解析模式和恐慌模式标志
 *
 * 主解析循环流程：
 * 1. 语句识别：检查当前token是否表示语句开始
 * 2. 语句解析：根据语句类型调用相应的解析函数
 * 3. 语句分隔：处理语句间的分号分隔符
 * 4. 错误处理：记录解析失败时的错误信息
 * 5. 继续循环：继续解析下一个语句直到输入结束
 *
 * 表达式解析实现：
 * 1. 优先级驱动：使用运算符优先级控制解析顺序
 * 2. 递归下降：从低优先级到高优先级逐层解析
 * 3. 括号处理：正确处理括号表达式和嵌套结构
 * 4. 函数调用：识别和解析函数调用表达式
 * 5. 类型转换：构建相应的AST节点表示表达式结构
 *
 * 语句解析实现：
 * 1. 关键字识别：根据第一个关键字确定语句类型
 * 2. 子句解析：按顺序解析语句的各个组成部分
 * 3. 约束检查：验证语句的语法约束和语义要求
 * 4. 节点构建：创建相应的AST节点表示语句结构
 * 5. 属性设置：为AST节点设置必要的属性和子节点
 *
 * 错误恢复机制：
 * 1. 恐慌模式：检测到错误时进入恐慌模式
 * 2. 同步跳转：跳过错误token直到遇到同步token
 * 3. 状态重置：重置解析状态准备继续解析
 * 4. 错误记录：记录错误信息但不中断解析过程
 * 5. 继续尝试：尝试从下一个有效位置继续解析
 *
 * 内存管理策略：
 * 1. 对象池：复用常用的AST节点对象
 * 2. 延迟释放：延迟释放不再使用的中间对象
 * 3. 引用计数：使用智能指针管理对象生命周期
 * 4. 内存预估：预估解析过程的内存需求
 * 5. 垃圾回收：及时清理解析过程中的临时对象
 *
 * 性能优化策略：
 * 1. 缓存机制：缓存常用解析结果和中间状态
 * 2. 预编译：对频繁使用的SQL进行预编译优化
 * 3. 并行解析：支持多个独立语句的并行解析
 * 4. 增量解析：支持SQL片段的增量解析和验证
 * 5. 状态复用：复用解析状态避免重复初始化
 *
 * 扩展性设计：
 * 1. 插件架构：支持自定义语法规则的动态加载
 * 2. 配置化：解析行为的配置化管理
 * 3. 多语言支持：不同SQL方言的语法规则支持
 * 4. 标准化：严格遵循SQL标准的语法规范
 * 5. 向后兼容：保持与现有解析器的兼容性
 *
 * 调试和诊断：
 * 1. 解析树可视化：图形化展示AST树结构
 * 2. 错误追踪：详细记录解析过程和错误位置
 * 3. 性能分析：解析过程的详细性能统计
 * 4. 测试工具：解析器的单元测试和集成测试
 * 5. 诊断工具：解析问题的自动化诊断和修复建议
 */


#ifndef SQLCC_SQL_PARSER_PARSER_H
#define SQLCC_SQL_PARSER_PARSER_H

#include "ast/ast_node.h"
#include "constraint.h"
#include "set_operation.h"
#include "token.h"
#include "window_function.h"
#include "lexer.h"
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

  // Error handling (public interface for testing)
  std::vector<std::string> getDetailedErrors() const;
  void clearErrors();
  bool hadError() const;

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
  std::string getErrorContext() const;

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

  // Helper method to check if current statement is CREATE VIEW
  bool isCreateViewStatement();

  // Helper method to check if current statement is CREATE USER
  bool isCreateUserStatement();

  // Helper method to check if current statement is DROP USER
  bool isDropUserStatement();

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
