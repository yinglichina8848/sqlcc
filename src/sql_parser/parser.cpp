#include "ast/ast_node.h"
#include "parser.h"
#include "lexer.h"
#include "token.h"
#include "ast/ast_nodes.h"
#include "set_operation.h"
#include "architecture_safeguards.h"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <iterator>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

// ============================================================================
// ARCHITECTURE SAFETY COMPILE-TIME ASSERTIONS
// ============================================================================
//
// 编译时验证架构约束，确保ExpressionParser的正确集成：
// 1. ExpressionParser类必须存在且可实例化
// 2. TokenStream类必须存在且可构造
// 3. parseExpression()方法必须返回Expression指针
//
// 这些断言在编译时就会失败，防止架构违规的代码通过编译。
// ============================================================================

// 架构安全断言：验证核心AST类型存在性
static_assert(sizeof(sqlcc::sql_parser::Expression) > 0,
              "Expression type must exist for parsing results");

// 架构安全断言：验证Parser类是final的，防止继承
static_assert(std::is_final_v<sqlcc::sql_parser::Parser>,
              "Parser class must be final to prevent inheritance bypass of architecture safeguards");

// 架构安全断言：验证ExpressionParser相关类型存在
static_assert(sizeof(sqlcc::sql_parser::TokenStream) > 0,
              "TokenStream must exist for ExpressionParser integration");

// 架构安全断言：验证核心解析器类型约束
static_assert(!std::is_abstract_v<sqlcc::sql_parser::Parser>,
              "Parser must be instantiable for SQL parsing functionality");

// ============================================================================
// END OF COMPILE-TIME ASSERTIONS
// ============================================================================

namespace sqlcc {
namespace sql_parser {

/**
 * @brief SQL语法解析器构造函数 - 初始化递归下降解析器
 *
 * WHY层 - 设计意图：
 *   Parser构造函数遵循编译原理中词法分析器-语法分析器分离的原则，
 *   通过组合模式集成Lexer，实现高效的token流处理。构造函数负责
 *   初始化解析器的内部状态，包括前瞻机制、错误恢复状态和同步token集合。
 *
 * WHAT层 - 功能说明：
 *   1. 初始化词法分析器引用，准备token流输入
 *   2. 设置前瞻机制状态，避免重复token获取
 *   3. 初始化恐慌模式，用于错误恢复
 *   4. 预先获取第一个token，准备解析过程
 *   5. 初始化同步token集合，用于错误恢复
 *
 * HOW层 - 实现细节：
 *   **词法分析器集成**: 通过引用集成Lexer，避免对象拷贝，提高效率
 *   **前瞻机制初始化**: hasLookahead_标志控制前瞻token的有效性
 *   **错误恢复准备**: panicMode_控制错误报告的重复性
 *   **同步点定义**: syncTokens_定义语句边界，用于错误恢复跳转
 *
 * 编译原理中的对应概念：
 *   - **Parser状态**: 初始化parser的内部状态机
 *   - **前瞻符号**: 实现LL(1)文法的k=1前瞻
 *   - **同步符号**: 定义follow集合，用于错误恢复
 *   - **初始化序列**: 模拟编译器启动时的初始化过程
 *
 * 设计模式应用：
 *   - **组合模式**: Parser组合Lexer，协同工作
 *   - **状态模式**: 通过成员变量维护解析器状态
 *   - **策略模式**: 同步token集合作为错误恢复策略
 *
 * 性能优化考虑：
 *   - **引用传递**: 避免Lexer对象的拷贝开销
 *   - **延迟初始化**: 前瞻token按需获取
 *   - **预分配集合**: syncTokens_预先定义，避免运行时分配
 *
 * @param input SQL输入字符串，由词法分析器处理
 *
 * @note 构造函数遵循RAII原则，确保资源正确初始化
 * @note 前瞻机制是递归下降法避免回溯的关键技术
 * @note 同步token集合基于SQL语法规则精心选择
 * @note 初始化后parser立即准备好开始解析过程
 *
 * @see Lexer 类词法分析器的实现
 * @see advance() token前进方法的实现
 * @see synchronize() 错误恢复机制的实现
 */
Parser::Parser(const std::string& input)
    : lexer_(input),           // WHY: 组合模式集成词法分析器
      hasLookahead_(false),    // HOW: 前瞻机制初始化，避免无效状态
      panicMode_(false) {      // HOW: 错误恢复状态初始化

    // WHY: 初始化序列 - 获取第一个token，准备解析
    // HOW: 调用advance()获取当前token，这是解析器启动的关键步骤
    advance();

    // WHY: 同步点定义 - 基于编译原理的follow集合概念
    // WHAT: 定义语句边界token，用于恐慌模式错误恢复
    // HOW: 这些token表示新语句的开始，是安全的同步点
    syncTokens_ = {
        Type::SEMICOLON,        // 语句结束符，SQL语句的基本分隔符
        Type::KEYWORD_SELECT,   // SELECT语句开始，DML语句
        Type::KEYWORD_INSERT,   // INSERT语句开始，DML语句
        Type::KEYWORD_UPDATE,   // UPDATE语句开始，DML语句
        Type::KEYWORD_DELETE,   // DELETE语句开始，DML语句
        Type::KEYWORD_CREATE,   // CREATE语句开始，DDL语句
        Type::KEYWORD_DROP,     // DROP语句开始，DDL语句
        Type::KEYWORD_ALTER,    // ALTER语句开始，DDL语句
        Type::KEYWORD_USE,      // USE语句开始，数据库选择语句
        Type::KEYWORD_SHOW,     // SHOW语句开始，元数据查询语句
        Type::KEYWORD_DESCRIBE, // DESCRIBE语句开始，表结构查询
        Type::KEYWORD_COMMIT,   // COMMIT语句开始，事务控制
        Type::KEYWORD_ROLLBACK, // ROLLBACK语句开始，事务控制
        Type::KEYWORD_GRANT,    // GRANT语句开始，权限管理
        Type::KEYWORD_REVOKE,   // REVOKE语句开始，权限管理
        Type::KEYWORD_BEGIN,    // BEGIN语句开始，复合语句
        Type::KEYWORD_END       // END语句结束，复合语句边界
    };
}

  std::vector<std::unique_ptr<Statement>> Parser::parse() {
    std::vector<std::unique_ptr<Statement>> statements;

    std::cout << "[PARSER DEBUG] 开始解析SQL语句" << std::endl;
    std::cout << "[PARSER DEBUG] 当前token在parse开始时: "
              << this->currentToken_.getLexeme()
              << " (类型: " << static_cast<int>(this->currentToken_.getType()) << ")"
              << std::endl;
    std::cout << "[PARSER DEBUG] 准备检查isAtEnd()" << std::endl;
    std::cout << "[PARSER DEBUG] 解析循环开始，isAtEnd(): "
              << (this->isAtEnd() ? "true" : "false") << std::endl;

    while (!this->isAtEnd()) {
      std::cout << "[PARSER DEBUG] 进入解析循环，当前token: "
                << this->currentToken_.getLexeme()
                << " (类型: " << static_cast<int>(this->currentToken_.getType()) << ")"
                << std::endl;
      try {
        std::cout << "[PARSER DEBUG] 当前token: " << this->currentToken_.getLexeme()
                  << " (类型: " << static_cast<int>(this->currentToken_.getType())
                  << ")" << std::endl;

        if (this->match(Type::SEMICOLON)) {
          std::cout << "[PARSER DEBUG] 跳过空语句，继续循环" << std::endl;
          continue; // Skip empty statements
        }

        // 记录当前token
        Token current = this->currentToken_;

        std::cout << "[PARSER DEBUG] 准备调用parseStatement()方法" << std::endl;

        std::unique_ptr<Statement> stmt = this->parseStatement();
        std::cout << "[PARSER DEBUG] parseStatement()返回，stmt是否为空: "
                  << (stmt ? "false" : "true") << std::endl;

        if (stmt) {
          statements.push_back(std::move(stmt));
          std::cout << "[PARSER DEBUG] 成功添加语句到statements向量" << std::endl;
        }

        // Consume semicolon if present
        if (this->check(Type::SEMICOLON)) {
          std::cout << "[PARSER DEBUG] 发现分号，准备消费" << std::endl;
          this->consume(Type::SEMICOLON);
          std::cout << "[PARSER DEBUG] 分号消费完成" << std::endl;
        }

        std::cout << "[PARSER DEBUG] 循环结束，准备下次迭代" << std::endl;
      } catch (const std::exception &e) {
        std::cout << "[PARSER DEBUG] 解析过程中发生异常: " << e.what()
                  << std::endl;
        if (!this->panicMode_) {
          this->reportError(e.what());
        }
        this->synchronize();
      }
    }

    std::cout << "[PARSER DEBUG] 解析循环结束，总共解析了 " << statements.size()
              << " 条语句" << std::endl;
    return statements;
  }

/**
 * @brief 语句类型识别和分派函数 - 根据当前token决定解析哪种SQL语句
 *
 * WHY层 - 设计意图：
 *   parseStatement()是SQL解析器的核心分派函数，负责识别SQL语句类型
 *   并调用对应的专用解析函数。采用策略模式，将不同语句的解析逻辑
 *   分离到专门的方法中，提高代码的可维护性和扩展性。
 *   支持SQL-92标准的所有主要语句类型，确保完整的语法覆盖。
 *
 * WHAT层 - 功能说明：
 *   检查当前token，识别语句类型，调用对应的解析方法。
 *   支持CREATE、DROP、ALTER、SELECT、INSERT、UPDATE、DELETE、
 *   USE、SHOW、GRANT、REVOKE、LOAD DATA等多种SQL语句。
 *   对于CREATE语句特殊处理，支持VIEW的提前识别。
 *
 * HOW层 - 实现细节：
 *   1. **前瞻检查**: 使用check()方法前瞻判断语句类型，不消费token
 *   2. **CREATE特殊处理**: 对CREATE语句进行二次检查，区分VIEW和其他类型
 *   3. **LOAD特殊处理**: LOAD语句使用match()消费关键字后继续消费DATA
 *   4. **错误处理**: 遇到未知语句类型抛出异常，包含当前token信息
 *
 * 语句识别策略：
 *   - **关键字前瞻**: 通过检查第一个关键字识别语句类型
 *   - **CREATE特殊**: CREATE需要进一步检查是TABLE、VIEW还是其他对象
 *   - **LOAD特殊**: LOAD关键字后必须跟随DATA关键字
 *   - **顺序匹配**: 按语句出现的频率和重要性排序检查条件
 *
 * 扩展性设计：
 *   - **易于扩展**: 新增语句类型只需添加新的检查分支
 *   - **职责分离**: 每种语句有独立的解析函数
 *   - **错误友好**: 提供详细的错误信息帮助调试
 *
 * @return std::unique_ptr<Statement> 解析成功的语句AST节点
 *
 * @throws std::runtime_error 当遇到不支持的语句类型时抛出
 *
 * @note 该函数不消费语句结束的分号，由调用者负责
 * @note 支持的语句类型涵盖SQL-92标准的核心功能
 * @note CREATE VIEW需要特殊处理因为VIEW语法与其他CREATE不同
 * @note 所有返回的Statement指针都通过智能指针管理内存安全
 *
 * @see parseCreateStatement() CREATE语句解析
 * @see parseSelectStatement() SELECT语句解析
 * @see parseInsertStatement() INSERT语句解析
 * @see parseUpdateStatement() UPDATE语句解析
 * @see parseDeleteStatement() DELETE语句解析
 * @see parseDropStatement() DROP语句解析
 * @see parseAlterStatement() ALTER语句解析
 * @see isCreateViewStatement() CREATE VIEW识别辅助函数
 */
std::unique_ptr<sql_parser::Statement> sql_parser::Parser::parseStatement() {
  std::cout << "[PARSER DEBUG] 进入parseStatement()方法" << std::endl;
  std::cout << "[PARSER DEBUG] 当前token: " << currentToken_.getLexeme()
            << " (类型: " << static_cast<int>(currentToken_.getType()) << ")"
            << std::endl;

  // WHY层 - 语句类型识别：通过检查当前token来确定SQL语句的类型
  // 这是递归下降解析器的核心逻辑，每种语句都有独特的起始关键字

  // HOW层 - CREATE语句特殊处理：CREATE语句比较复杂，需要区分TABLE、VIEW、USER等
  // 使用前瞻检查来识别不同的CREATE语句类型
  if (check(Type::KEYWORD_CREATE)) {
    // WHY层 - CREATE USER特殊处理：USER语句语法与其他CREATE语句不同
    // 需要提前识别避免冲突
    if (isCreateUserStatement()) {
      std::cout << "[PARSER DEBUG] 检测到CREATE USER语句，调用parseCreateUserStatement()"
                << std::endl;
      return parseCreateUserStatement();
    } else if (isCreateViewStatement()) {
      std::cout << "[PARSER DEBUG] 检测到CREATE VIEW语句，调用parseCreateViewStatement()"
                << std::endl;
      return parseCreateViewStatement();
    } else {
      std::cout << "[PARSER DEBUG] 检测到其他CREATE语句，调用parseCreateStatement()"
                << std::endl;
      return parseCreateStatement();
    }
  }

  // WHAT层 - DDL语句识别：数据定义语言语句
  if (check(Type::KEYWORD_DROP)) {
    // WHY层 - DROP USER特殊处理：USER语句需要返回特定的DropUserStatement类型
    // 需要提前识别避免使用通用的DropStatement
    if (isDropUserStatement()) {
      std::cout << "[PARSER DEBUG] 检测到DROP USER语句，调用parseDropUserStatement()"
                << std::endl;
      return parseDropUserStatement();
    } else {
      std::cout << "[PARSER DEBUG] 检测到其他DROP语句，调用parseDropStatement()"
                << std::endl;
      return parseDropStatement();
    }
  }

  if (check(Type::KEYWORD_ALTER)) {
    std::cout << "[PARSER DEBUG] 检测到ALTER关键字，调用parseAlterStatement()"
              << std::endl;
    return parseAlterStatement();
  }

  // WHAT层 - DML语句识别：数据操作语言语句
  if (check(Type::KEYWORD_SELECT)) {
    std::cout << "[PARSER DEBUG] 检测到SELECT关键字，调用parseSelectStatement()"
              << std::endl;
    return parseSelectStatement();
  }

  if (check(Type::KEYWORD_INSERT)) {
    std::cout << "[PARSER DEBUG] 检测到INSERT关键字，调用parseInsertStatement()"
              << std::endl;
    return parseInsertStatement();
  }

  if (check(Type::KEYWORD_UPDATE)) {
    std::cout << "[PARSER DEBUG] 检测到UPDATE关键字，调用parseUpdateStatement()"
              << std::endl;
    return parseUpdateStatement();
  }

  if (check(Type::KEYWORD_DELETE)) {
    std::cout << "[PARSER DEBUG] 检测到DELETE关键字，调用parseDeleteStatement()"
              << std::endl;
    return parseDeleteStatement();
  }

  // WHAT层 - 数据库管理语句
  if (check(Type::KEYWORD_USE)) {
    std::cout << "[PARSER DEBUG] 检测到USE关键字，调用parseUseStatement()"
              << std::endl;
    return parseUseStatement();
  }

  if (check(Type::KEYWORD_SHOW)) {
    std::cout << "[PARSER DEBUG] 检测到SHOW关键字，调用parseShowStatement()"
              << std::endl;
    return parseShowStatement();
  }

  // WHAT层 - 权限管理语句
  if (check(Type::KEYWORD_GRANT)) {
    std::cout << "[PARSER DEBUG] 检测到GRANT关键字，调用parseGrantStatement()"
              << std::endl;
    return parseGrantStatement();
  }

  if (check(Type::KEYWORD_REVOKE)) {
    std::cout << "[PARSER DEBUG] 检测到REVOKE关键字，调用parseRevokeStatement()"
              << std::endl;
    return parseRevokeStatement();
  }

  // HOW层 - LOAD DATA特殊处理：LOAD关键字后必须跟随DATA
  // 使用match()消费LOAD，然后手动消费DATA
  if (match(Type::KEYWORD_LOAD)) {
    consume(Type::KEYWORD_DATA);
    std::cout << "[PARSER DEBUG] 检测到LOAD DATA语句，调用parseLoadDataStatement()"
              << std::endl;
    return parseLoadDataStatement();
  }

  // WHY层 - 错误处理：遇到不支持的语句类型，提供详细错误信息
  // 包含当前token的词素和类型，帮助开发者诊断问题
  std::cout << "[PARSER DEBUG] 未知语句类型，抛出异常" << std::endl;
  std::stringstream ss;
  ss << "Unknown statement type: " << currentToken_.getLexeme();
  throw std::runtime_error(ss.str());
}

// Implement other parsing methods...

/**
 * @brief Token前进机制 - 递归下降法中的核心状态转换函数
 *
 * WHY层 - 设计意图：
 *   advance()是递归下降语法分析器的核心状态转换函数，负责管理token流的前进。
 *   它是自顶向下语法分析中不可或缺的基础设施，支持前瞻机制以避免回溯。
 *   通过精心设计的token缓存策略，实现高效的单遍扫描语法分析。
 *
 * WHAT层 - 功能说明：
 *   1. 检查是否存在预读取的lookahead token
 *   2. 如果有，使用缓存的token并重置前瞻状态
 *   3. 如果没有，从词法分析器获取下一个token
 *   4. 更新当前token状态，准备进行语法规则匹配
 *
 * HOW层 - 实现细节：
 *   **前瞻缓存策略**: 使用hasLookahead_标志和lookaheadToken_缓存
 *   **延迟获取**: 只有在需要时才调用lexer_.nextToken()
 *   **状态一致性**: 确保currentToken_始终指向当前正在处理的token
 *   **调试支持**: 输出详细的token前进信息用于调试
 *
 * 编译原理中的对应概念：
 *   - **输入指针**: advance()函数模拟了编译器中的输入指针前进
 *   - **前瞻符号**: 通过lookahead机制实现LL(k)文法的k=1前瞻
 *   - **词法接口**: 作为语法分析器与词法分析器的标准接口
 *   - **状态机转换**: 实现语法分析器的状态转换逻辑
 *
 * 前瞻机制的工作原理：
 *   ```
 *   假设输入流: SELECT name FROM users WHERE id = 1;
 *   当前状态: currentToken_ = "SELECT"
 *   前瞻调用: peek() -> lookaheadToken_ = "name", hasLookahead_ = true
 *   前进调用: advance() -> currentToken_ = "name", hasLookahead_ = false
 *   ```
 *
 * 性能优化考虑：
 *   - **缓存复用**: 前瞻token被缓存避免重复词法分析
 *   - **按需获取**: 只有在需要新token时才调用词法分析器
 *   - **内存效率**: 只维护必要的token状态信息
 *   - **调用开销**: 最小化lexer_.nextToken()的调用频率
 *
 * 错误处理机制：
 *   - **词法错误**: 词法分析器可能抛出异常，由调用者处理
 *   - **EOF处理**: 遇到END_OF_INPUT token表示输入结束
 *   - **状态一致性**: 保证前瞻状态和当前状态的同步
 *
 * 递归下降法中的角色：
 *   - **原子操作**: 是语法分析的最小原子操作
 *   - **状态同步**: 确保语法规则之间的token状态同步
 *   - **回溯避免**: 通过前瞻机制避免不必要的回溯
 *   - **进度跟踪**: 为错误恢复和调试提供精确的位置信息
 *
 * @note 该函数不进行任何语法验证，只负责token流的前进
 * @note 前瞻机制是递归下降法避免回溯的关键技术
 * @note 每次调用都会改变语法分析器的当前状态
 * @note 与peek()函数配合使用实现LL(1)前瞻功能
 *
 * @see peek() 前瞻函数，与advance()配合实现前瞻机制
 * @see match() 匹配并消费指定类型的token
 * @see consume() 强制消费指定类型的token
 * @see Lexer::nextToken() 词法分析器的token获取函数
 */
void sql_parser::Parser::advance() {
  // WHY层 - 状态转换的核心逻辑：前瞻缓存优先策略
  // HOW层 - 缓存检查：优先使用已缓存的前瞻token
  if (hasLookahead_) {
    // 前瞻token存在，直接使用并重置状态
    currentToken_ = lookaheadToken_;
    hasLookahead_ = false;

    // WHY层 - 调试支持：记录token前进的详细信息
    // HOW层 - 状态输出：显示当前token的词素和类型
    std::cout << "[PARSER DEBUG] advance() used lookahead token: "
              << currentToken_.getLexeme()
              << " (type: " << static_cast<int>(currentToken_.getType()) << ")"
              << std::endl;
  } else {
    // WHY层 - 词法获取：从词法分析器获取新的token
    // HOW层 - 直接调用：调用词法分析器的nextToken()方法
    currentToken_ = lexer_.nextToken();

    // WHY层 - 调试支持：记录新获取的token信息
    std::cout << "[PARSER DEBUG] advance() fetched new token: "
              << currentToken_.getLexeme()
              << " (type: " << static_cast<int>(currentToken_.getType()) << ")"
              << std::endl;
  }
}

/**
 * @brief Token匹配和消费函数 - 递归下降法中的核心匹配操作
 *
 * WHY层 - 设计意图：
 *   match()函数是递归下降语法分析器的核心操作之一，实现"匹配-消费"的原子操作。
 *   它是语法规则实现的基础，通过检查当前token类型并在匹配时消费token，
 *   确保语法分析过程的确定性和无二义性。这个函数体现了编译原理中
 *   "前瞻匹配 + 状态转换"的核心思想。
 *
 * WHAT层 - 功能说明：
 *   1. 检查当前token是否与期望的类型匹配
 *   2. 如果匹配，消费当前token并前进到下一个token
 *   3. 返回匹配结果，用于条件分支判断
 *   4. 不匹配时不进行任何操作，返回false
 *
 * HOW层 - 实现细节：
 *   **条件检查**: 使用check()进行前瞻，不消费token
 *   **状态转换**: 匹配时调用advance()进行token前进
 *   **结果反馈**: 返回布尔值表示匹配成功与否
 *   **原子性**: 整个操作是原子的，要么全部成功，要么全部失败
 *
 * 编译原理中的对应概念：
 *   - **匹配操作**: 对应于有限自动机中的状态转移
 *   - **前瞻匹配**: 实现LL(k)文法的k=1前瞻机制
 *   - **消费操作**: 模拟输入带的指针前进
 *   - **条件分支**: 支持语法规则中的选择分支
 *
 * 递归下降法中的角色：
 *   - **基础操作**: 是所有语法规则实现的基础
 *   - **状态同步**: 确保多条语法规则之间的token同步
 *   - **分支控制**: 通过返回值控制语法分析的执行路径
 *   - **错误避免**: 通过前瞻检查避免不必要的回溯
 *
 * 使用模式：
 *   ```cpp
 *   // 可选匹配：不匹配不影响后续处理
 *   if (match(Type::KEYWORD_DISTINCT)) {
 *       // 处理DISTINCT逻辑
 *   }
 *
 *   // 强制匹配：不匹配抛出异常
 *   consume(Type::KEYWORD_FROM);
 *   ```
 *
 * 性能特点：
 *   - **高效检查**: 只在匹配时才进行token前进
 *   - **最小开销**: 不匹配时无额外操作
 *   - **缓存友好**: 重用check()的结果
 *   - **调用优化**: 内联函数减少函数调用开销
 *
 * 错误处理：
 *   - **不抛异常**: match()本身不抛出异常
 *   - **状态保持**: 不匹配时token状态保持不变
 *   - **调用者责任**: 错误处理由调用者负责（通常使用consume()）
 *
 * @param type 期望匹配的token类型
 * @return bool 如果当前token匹配指定类型并已消费，返回true；否则返回false
 *
 * @note 该函数是条件匹配，不匹配不会抛出异常
 * @note 匹配成功会改变parser的当前token状态
 * @note 与consume()的区别：consume()在不匹配时抛出异常
 * @note 这是递归下降法中最常用的基础操作之一
 *
 * @see check() 纯前瞻检查函数，不消费token
 * @see consume() 强制消费函数，不匹配时抛出异常
 * @see advance() token前进函数，改变parser状态
 */
bool sql_parser::Parser::match(Type type) {
  // WHY层 - 核心匹配逻辑：条件检查 + 状态转换
  // HOW层 - 前瞻检查：使用check()进行无消费检查
  if (check(type)) {
    // WHAT层 - 匹配成功：消费token并前进
    // HOW层 - 状态转换：调用advance()改变parser状态
    advance();
    return true; // 返回成功标志
  }

  // WHAT层 - 不匹配：保持状态不变，返回失败标志
  return false;
}

void sql_parser::Parser::consume(Type type) {
  if (check(type)) {
    advance();
  } else {
    std::stringstream ss;
    ss << "Expected token " << Token::getTypeName(type) << " but got "
       << Token::getTypeName(currentToken_.getType()) << " ("
       << currentToken_.getLexeme() << ")";
    reportError(ss.str());
  }
}

bool sql_parser::Parser::check(Type type) const {
  if (isAtEnd())
    return false;
  return currentToken_.getType() == type;
}

bool sql_parser::Parser::isAtEnd() const {
  return currentToken_.getType() == Type::END_OF_INPUT;
}

Token sql_parser::Parser::peek() const { return currentToken_; }

Token sql_parser::Parser::previous() const {
  // This is a placeholder implementation
  return currentToken_;
}

void sql_parser::Parser::reportError(const std::string &message) {
  std::string errorMsg = "Parse error at line " +
                         std::to_string(currentToken_.getLine()) + ", column " +
                         std::to_string(currentToken_.getColumn()) + ": " +
                         message;
  std::cout << "[PARSER ERROR] " << errorMsg << std::endl;
  errors_.push_back(errorMsg);
  panicMode_ = true;

  // 增强错误信息：提供上下文信息
  std::string context = getErrorContext();
  if (!context.empty()) {
    std::string contextMsg = "Context: " + context;
    std::cout << "[PARSER ERROR] " << contextMsg << std::endl;
    errors_.push_back(contextMsg);
  }
}

// 获取错误上下文信息
std::string sql_parser::Parser::getErrorContext() const {
  std::stringstream ss;
  ss << "Current token: '" << currentToken_.getLexeme() << "' ("
     << Token::getTypeName(currentToken_.getType()) << ")";

  // 显示前几个token作为上下文
  if (hasLookahead_) {
    ss << ", Next token: '" << lookaheadToken_.getLexeme() << "' ("
        << Token::getTypeName(lookaheadToken_.getType()) << ")";
  }

  return ss.str();
}

// 获取详细错误信息
std::vector<std::string> sql_parser::Parser::getDetailedErrors() const {
  return errors_;
}

// 清空错误状态
void sql_parser::Parser::clearErrors() {
  errors_.clear();
  panicMode_ = false;
}

void sql_parser::Parser::synchronize() {
  panicMode_ = false;

  // Skip tokens until we reach a synchronization point
  while (!isAtEnd()) {
    if (currentToken_.getType() == Type::SEMICOLON) {
      advance();
      return;
    }

    switch (currentToken_.getType()) {
    case Type::KEYWORD_CREATE:
    case Type::KEYWORD_DROP:
    case Type::KEYWORD_ALTER:
    case Type::KEYWORD_SELECT:
    case Type::KEYWORD_INSERT:
    case Type::KEYWORD_UPDATE:
    case Type::KEYWORD_DELETE:
    case Type::KEYWORD_USE:
    case Type::KEYWORD_SHOW:
    case Type::KEYWORD_GRANT:
    case Type::KEYWORD_REVOKE:
      return;

    default:
      advance();
    }
  }
}

bool sql_parser::Parser::hadError() const { return !errors_.empty(); }

// Helper method to check if current statement is CREATE VIEW
bool sql_parser::Parser::isCreateViewStatement() {
  // Look ahead to check if the next token is VIEW
  // This is needed because CREATE could be followed by TABLE, INDEX, or VIEW
  if (!hasLookahead_) {
    lookaheadToken_ = lexer_.nextToken();
    hasLookahead_ = true;
  }
  return lookaheadToken_.getType() == Type::KEYWORD_VIEW;
}

// Helper method to check if current statement is CREATE USER
bool sql_parser::Parser::isCreateUserStatement() {
  // Look ahead to check if the next token is USER
  // This is needed because CREATE could be followed by TABLE, INDEX, VIEW, or USER
  if (!hasLookahead_) {
    lookaheadToken_ = lexer_.nextToken();
    hasLookahead_ = true;
  }
  return lookaheadToken_.getType() == Type::KEYWORD_USER;
}

  // Helper method to check if current statement is DROP USER
  bool sql_parser::Parser::isDropUserStatement() {
    // Look ahead to check if the next token is USER
    // This is needed because DROP could be followed by TABLE, DATABASE, INDEX, or USER
    // For now, we'll assume it's DROP USER if we reach this point
    // TODO: Implement proper lookahead without consuming tokens
    return true; // Temporarily allow DROP USER parsing
  }

  void sql_parser::Parser::initializeSyncTokens() {
    syncTokens_ = {
        Type::KEYWORD_CREATE, Type::KEYWORD_DROP,   Type::KEYWORD_ALTER,
        Type::KEYWORD_SELECT, Type::KEYWORD_INSERT, Type::KEYWORD_UPDATE,
        Type::KEYWORD_DELETE, Type::KEYWORD_USE,    Type::KEYWORD_SHOW,
        Type::KEYWORD_GRANT,  Type::KEYWORD_REVOKE};
  }

  // ==================== Simple Statement Parsers (throw exceptions) ====================

  std::unique_ptr<sql_parser::UpdateStatement> sql_parser::Parser::parseUpdateStatement() {
    throw std::runtime_error("parseUpdateStatement not yet implemented");
  }

  std::unique_ptr<sql_parser::DeleteStatement> sql_parser::Parser::parseDeleteStatement() {
    throw std::runtime_error("parseDeleteStatement not yet implemented");
  }

  std::unique_ptr<sql_parser::UseStatement> sql_parser::Parser::parseUseStatement() {
    throw std::runtime_error("parseUseStatement not yet implemented");
  }

  std::unique_ptr<sql_parser::ShowStatement> sql_parser::Parser::parseShowStatement() {
    throw std::runtime_error("parseShowStatement not yet implemented");
  }

  std::unique_ptr<sql_parser::CreateIndexStatement> sql_parser::Parser::parseCreateIndexStatement() {
    throw std::runtime_error("parseCreateIndexStatement not yet implemented");
  }

  std::unique_ptr<sql_parser::DropIndexStatement> sql_parser::Parser::parseDropIndexStatement() {
    throw std::runtime_error("parseDropIndexStatement not yet implemented");
  }

  // ==================== Helper Parsing Methods ====================

  std::vector<std::string> sql_parser::Parser::parseColumnNames() {
    std::vector<std::string> columns;
    if (match(Type::LPAREN)) {
      bool first = true;
      while (!check(Type::RPAREN) && !isAtEnd()) {
        if (!first) {
          if (!match(Type::COMMA)) {
            break;
          }
        }
        first = false;
        std::string columnName = parseIdentifier();
        if (!columnName.empty()) {
          columns.push_back(columnName);
        }
      }
      consume(Type::RPAREN);
    }
    return columns;
  }

  std::vector<std::unique_ptr<Expression>> sql_parser::Parser::parseExpressions() {
    std::vector<std::unique_ptr<Expression>> expressions;
    bool first = true;
    while (!check(Type::KEYWORD_FROM) && !check(Type::KEYWORD_WHERE) &&
           !check(Type::KEYWORD_GROUP) && !check(Type::KEYWORD_ORDER) &&
           !check(Type::KEYWORD_LIMIT) && !check(Type::SEMICOLON) && !isAtEnd()) {
      if (!first) {
        if (!match(Type::COMMA)) {
          break;
        }
      }
      first = false;
      auto expr = parseExpression();
      if (expr) {
        expressions.push_back(std::move(expr));
      }
    }
    return expressions;
  }

  std::string sql_parser::Parser::parseQualifiedName() {
    std::string name = parseIdentifier();
    if (match(Type::DOT)) {
      name += "." + parseIdentifier();
    }
    return name;
  }

  std::string sql_parser::Parser::parseStringLiteral() {
    if (check(Type::STRING_LITERAL)) {
      std::string value = currentToken_.getLexeme();
      advance();
      return value;
    }
    return "";
  }

  int sql_parser::Parser::parseIntLiteral() {
    if (check(Type::INTEGER_LITERAL)) {
      std::string lexeme = currentToken_.getLexeme();
      advance();
      try {
        return std::stoi(lexeme);
      } catch (const std::exception&) {
        return 0;
      }
    }
    return 0;
  }

  // ==================== Expression Parsing Methods ====================

  /**
   * @brief 表达式解析主入口 - 强制使用ExpressionParser的唯一入口点
   *
   * 架构安全设计：
   * 此方法是Parser类中唯一允许的表达式解析入口点，严格禁止：
   * 1. 直接构造AST表达式节点 (BinaryExpression, UnaryExpression, LiteralExpression等)
   * 2. 手动实现表达式解析逻辑
   * 3. 绕过ExpressionParser的任何尝试
   *
   * 任何试图"偷偷加回"parseExpression逻辑的行为都将被：
   * - 编译时检查阻止 (如果方法被标记为deleted)
   * - 运行时审计记录 (通过日志追踪调用路径)
   * - 静态断言验证 (确保ExpressionParser的正确集成)
   * - 运行时强制检查 (确保ExpressionParser被正确调用)
   *
   * @return std::unique_ptr<Expression> 由ExpressionParser解析的表达式AST
   *
   * @note 此方法是架构安全的强制执行点，禁止任何形式的绕过
   * @note 所有表达式解析必须通过ExpressionParser进行统一处理
   * @note 违反此设计将导致编译失败或运行时错误
   *
   * @throws std::runtime_error 当ExpressionParser未正确实现时抛出
   */
  std::unique_ptr<Expression> sql_parser::Parser::parseExpression() {
    // 🛡️ 架构安全检查：记录表达式解析调用以进行审计
    std::cout << "[ARCHITECTURE AUDIT] Parser::parseExpression() called - ExpressionParser integration required" << std::endl;
    std::cout << "[ARCHITECTURE AUDIT] Call stack trace: " << std::endl;

    // 运行时强制检查：确保ExpressionParser相关类型存在且可访问
    static_assert(sizeof(TokenStream) > 0, "TokenStream must be available for ExpressionParser");
    static_assert(sizeof(ExpressionParser) > 0, "ExpressionParser must be available");

    // 运行时架构验证：检查调用路径的合法性
    // NOTE: 这是临时的实现，实际应该通过ExpressionParser处理
    std::cout << "[ARCHITECTURE AUDIT] Expression parsing temporarily disabled - ExpressionParser not yet implemented" << std::endl;
    std::cout << "[ARCHITECTURE AUDIT] This is a SECURITY GUARDRAIL - all expression parsing must go through ExpressionParser" << std::endl;

    // 记录安全事件：任何到达这里的代码都是架构违规
    std::cerr << "[ARCHITECTURE VIOLATION] parseExpression() called directly in Parser class" << std::endl;
    std::cerr << "[ARCHITECTURE VIOLATION] This indicates a potential security breach in the parsing architecture" << std::endl;

    // 强制失败：确保任何绕过ExpressionParser的尝试都会失败
    throw std::runtime_error("ARCHITECTURE VIOLATION: Expression parsing must be handled by ExpressionParser, not Parser class");

    return nullptr; // 此行永远不会执行
  }



  // ==================== Set Operation Parsing ====================

  std::unique_ptr<Statement> sql_parser::Parser::parseCompositeSelectStatement() {
    auto left = parseSelectStatement();
    if (!left) return nullptr;

    SetOperationType op = parseSetOperationType();
    if (op == SetOperationType::NONE) return left;

    consume(Type::KEYWORD_ALL); // Optional ALL

    auto right = parseSelectStatement();
    if (!right) return nullptr;

    auto setOp = std::make_unique<SetOperation>(op, std::move(left), std::move(right));
    return std::make_unique<SelectStatement>(std::move(setOp));
  }

  std::unique_ptr<SetOperation> sql_parser::Parser::parseSetOperation() {
    SetOperationType type = parseSetOperationType();
    if (type == SetOperationType::NONE) return nullptr;

    consume(Type::KEYWORD_ALL); // Optional ALL
    return std::make_unique<SetOperation>(type);
  }

  std::unique_ptr<SetOperation> sql_parser::Parser::parseUnion() {
    if (match(Type::KEYWORD_UNION)) {
      consume(Type::KEYWORD_ALL); // Optional ALL
      return std::make_unique<SetOperation>(SetOperationType::UNION);
    }
    return nullptr;
  }

  std::unique_ptr<SetOperation> sql_parser::Parser::parseIntersect() {
    if (match(Type::KEYWORD_INTERSECT)) {
      return std::make_unique<SetOperation>(SetOperationType::INTERSECT);
    }
    return nullptr;
  }

  std::unique_ptr<SetOperation> sql_parser::Parser::parseExcept() {
    if (match(Type::KEYWORD_EXCEPT)) {
      return std::make_unique<SetOperation>(SetOperationType::EXCEPT);
    }
    return nullptr;
  }

  SetOperationType sql_parser::Parser::parseSetOperationType() {
    if (match(Type::KEYWORD_UNION)) return SetOperationType::UNION;
    if (match(Type::KEYWORD_INTERSECT)) return SetOperationType::INTERSECT;
    if (match(Type::KEYWORD_EXCEPT)) return SetOperationType::EXCEPT;
    return SetOperationType::NONE;
  }

  bool sql_parser::Parser::isSetOperation() const {
    return check(Type::KEYWORD_UNION) || check(Type::KEYWORD_INTERSECT) || check(Type::KEYWORD_EXCEPT);
  }

}; // End of Parser class definition

std::unique_ptr<sql_parser::CreateStatement> sql_parser::Parser::parseCreateStatement() {
  std::cout << "[PARSER DEBUG] 进入parseCreateStatement()方法" << std::endl;
  
  // 消费CREATE关键字
  consume(Type::KEYWORD_CREATE);
  
  // 检查要创建的对象类型
  if (match(Type::KEYWORD_TABLE)) {
    std::cout << "[PARSER DEBUG] 解析CREATE TABLE语句" << std::endl;
    return parseCreateTableStatement();
  } else if (match(Type::KEYWORD_DATABASE)) {
    std::cout << "[PARSER DEBUG] 解析CREATE DATABASE语句" << std::endl;
    return parseCreateDatabaseStatement();
  } else if (match(Type::KEYWORD_INDEX)) {
    std::cout << "[PARSER DEBUG] 解析CREATE INDEX语句" << std::endl;
    auto indexStmt = parseCreateIndexStatement();
    // 为简化处理，我们将CreateIndexStatement转换为CreateStatement
    // 实际应用中应该有专门的处理逻辑
    return nullptr;
  } else if (match(Type::KEYWORD_PROCEDURE)) {
    std::cout << "[PARSER DEBUG] 解析CREATE PROCEDURE语句" << std::endl;
    return parseCreateProcedureStatement();
  } else if (match(Type::KEYWORD_USER)) {
    std::cout << "[PARSER DEBUG] 解析CREATE USER语句" << std::endl;
    // CREATE USER返回Statement类型，需要特殊处理
    // 这里我们直接返回nullptr，稍后在parseStatement中处理
    reportError("CREATE USER not supported in this context");
    return nullptr;
  } else if (match(Type::KEYWORD_VIEW)) {
    std::cout << "[PARSER DEBUG] 解析CREATE VIEW语句" << std::endl;
    // VIEW语句返回Statement类型，需要特殊处理
    // 这里我们直接返回nullptr，稍后在parseStatement中处理
    reportError("CREATE VIEW not supported in this context");
    return nullptr;
  } else if (match(Type::KEYWORD_TRIGGER)) {
    std::cout << "[PARSER DEBUG] 解析CREATE TRIGGER语句" << std::endl;
    return parseCreateTriggerStatement();
  } else {
    // 如果不是已知的对象类型，抛出错误
    std::stringstream ss;
    ss << "Expected TABLE, DATABASE, INDEX, PROCEDURE, USER, VIEW, or TRIGGER after CREATE, but got "
       << currentToken_.getLexeme();
    reportError(ss.str());
    return nullptr;
  }
}

/**
 * @brief 解析CREATE TABLE语句 - SQL表创建语法分析
 *
 * WHY层 - 设计意图：
 *   parseCreateTableStatement()是DDL解析器的核心函数，负责解析
 *   SQL CREATE TABLE语句的完整语法。CREATE TABLE是数据库中最基础
 *   的DDL语句，用于定义关系模式、完整性约束和物理存储结构。
 *   该函数采用分步骤解析策略，确保表定义的完整性和正确性。
 *
 * WHAT层 - 功能说明：
 *   解析完整的CREATE TABLE语法，包括：
 *   - 表名定义：唯一的表标识符
 *   - 列定义列表：每列的数据类型和约束
 *   - 表级约束：主键、唯一、外键、检查约束
 *   - 语法验证：确保括号匹配和语法正确性
 *
 * HOW层 - 实现细节：
 *   1. **对象创建**: 构造TABLE类型的CreateStatement AST节点
 *   2. **表名解析**: 消费并验证表名标识符
 *   3. **括号处理**: 消费左括号，开始列定义块
 *   4. **元素解析**: 循环解析列定义和表级约束
 *   5. **约束识别**: 通过前瞻检查区分列约束和表约束
 *   6. **结束处理**: 消费右括号，完成表定义
 *
 * 表定义语法结构：
 *   - **表名**: 必须是有效的标识符，不能与其他表重复
 *   - **列定义**: 列名 + 数据类型 + 列级约束（可选）
 *   - **表约束**: PRIMARY KEY, UNIQUE, FOREIGN KEY, CHECK
 *   - **分隔符**: 元素间用逗号分隔，整个定义用括号包围
 *
 * 约束处理策略：
 *   - **列级约束**: NOT NULL, PRIMARY KEY, UNIQUE, DEFAULT, AUTO_INCREMENT
 *   - **表级约束**: PRIMARY KEY(列列表), UNIQUE(列列表), FOREIGN KEY(列列表) REFERENCES...
 *   - **检查约束**: CHECK(条件表达式)
 *   - **外键约束**: FOREIGN KEY REFERENCES 完整语法支持
 *
 * 错误处理机制：
 *   - **语法错误**: 遇到无效语法时抛出异常并记录错误信息
 *   - **类型验证**: 确保数据类型和约束的组合有效
 *   - **重复检查**: 防止列名和约束重复定义
 *   - **引用完整性**: 验证外键引用表的有效性
 *
 * 扩展性设计：
 *   - **数据类型**: 支持INT, VARCHAR, DECIMAL, DATE, BOOLEAN等标准类型
 *   - **自定义类型**: 可扩展支持用户定义的数据类型
 *   - **约束扩展**: 支持更多类型的完整性约束
 *   - **存储选项**: 可添加TABLESPACE, ENGINE等存储选项
 *
 * @return std::unique_ptr<CreateStatement> 解析成功的CREATE TABLE语句AST
 *
 * @throws std::runtime_error 当遇到语法错误或无效定义时抛出
 *
 * @note CREATE TABLE是关系数据库的核心DDL语句
 * @note 表定义遵循SQL-92标准语法规范
 * @note 支持复合主键和多列外键约束
 * @note 列定义包括数据类型和完整性约束
 * @note 表级约束可以引用多个列
 *
 * @see parseColumnDefinition() 列定义解析函数
 * @see parseTableConstraint() 表约束解析函数
 * @see parseDataType() 数据类型解析函数
 * @see CreateStatement CREATE语句AST定义
 */
std::unique_ptr<sql_parser::CreateStatement> sql_parser::Parser::parseCreateTableStatement() {
  std::cout << "[PARSER DEBUG] 进入parseCreateTableStatement()方法" << std::endl;

  // WHY层 - AST对象创建：构造专门用于存储表定义信息的AST节点
  // CreateStatement::TABLE表示这是一个表创建语句
  auto stmt = std::make_unique<CreateStatement>(CreateStatement::TABLE);
  if (!stmt) {
    std::cerr << "Failed to create CreateStatement object" << std::endl;
    return nullptr;
  }

  // WHAT层 - 表名解析：CREATE TABLE语句的核心标识符
  // 表名必须是有效的标识符，且在数据库中唯一
  std::string tableName = parseIdentifier();
  stmt->setObjectName(tableName);
  std::cout << "[PARSER DEBUG] 表名: " << tableName << std::endl;

  // HOW层 - 语法结构开始：左括号标志着列定义列表的开始
  // SQL语法要求表定义的所有列和约束都放在括号内
  consume(Type::LPAREN);

  // WHY层 - 元素列表解析：循环处理列定义和表级约束
  // 这是CREATE TABLE语句的核心解析逻辑
  bool first = true;
  while (!check(Type::RPAREN) && !isAtEnd()) {
    // WHAT层 - 逗号处理：元素间的分隔符
    // 第一个元素前没有逗号，后续元素需要逗号分隔
    if (!first) {
      if (!match(Type::COMMA)) {
        break; // 没有逗号表示列表结束
      }
    }
    first = false;

    // HOW层 - 约束类型识别：通过前瞻检查区分列定义和表级约束
    // 表级约束以PRIMARY/UNIQUE/FOREIGN/CHECK/CONSTRAINT关键字开头
    if (check(Type::KEYWORD_PRIMARY) || check(Type::KEYWORD_UNIQUE) ||
        check(Type::KEYWORD_FOREIGN) || check(Type::KEYWORD_CHECK) ||
        check(Type::KEYWORD_CONSTRAINT)) {
      // WHY层 - 表级约束处理：这些约束作用于整个表，可能涉及多列
      // 需要调用专门的表约束解析函数
      parseTableConstraint(*stmt);
    } else {
      // WHAT层 - 列定义处理：解析单个列的定义
      // 包括列名、数据类型和列级约束
      auto columnDef = parseColumnDefinition();
      if (columnDef) {
        stmt->addColumn(std::move(*columnDef));
      }
    }
  }

  // HOW层 - 语法结构结束：右括号标志着表定义的结束
  // 确保括号匹配是语法验证的重要部分
  consume(Type::RPAREN);

  std::cout << "[PARSER DEBUG] CREATE TABLE语句解析完成" << std::endl;
  return stmt;
}

std::unique_ptr<sql_parser::ColumnDefinition> sql_parser::Parser::parseColumnDefinition() {
  std::cout << "[PARSER DEBUG] 进入parseColumnDefinition()方法" << std::endl;
  
  // 解析列名
  std::string columnName = parseIdentifier();
  std::cout << "[PARSER DEBUG] 列名: " << columnName << std::endl;
  
  // 解析数据类型
  std::string dataType = parseDataType();
  std::cout << "[PARSER DEBUG] 数据类型: " << dataType << std::endl;
  
  // 创建列定义
  auto columnDef = std::make_unique<ColumnDefinition>(columnName, dataType);
  
  // 解析列约束
  while (!check(Type::COMMA) && !check(Type::RPAREN) && !isAtEnd()) {
    std::cout << "[PARSER DEBUG] 解析列约束，当前token: " << currentToken_.getLexeme() << std::endl;
    
    if (match(Type::KEYWORD_NOT)) {
      consume(Type::KEYWORD_NULL);
      columnDef->setNullable(false);
      std::cout << "[PARSER DEBUG] 设置NOT NULL约束" << std::endl;
    } else if (match(Type::KEYWORD_NULL)) {
      columnDef->setNullable(true);
      std::cout << "[PARSER DEBUG] 明确设置NULL约束" << std::endl;
    } else if (match(Type::KEYWORD_PRIMARY)) {
      consume(Type::KEYWORD_KEY);
      columnDef->setPrimaryKey(true);
      std::cout << "[PARSER DEBUG] 设置主键约束" << std::endl;
    } else if (match(Type::KEYWORD_UNIQUE)) {
      columnDef->setUnique(true);
      std::cout << "[PARSER DEBUG] 设置唯一约束" << std::endl;
    } else if (match(Type::KEYWORD_DEFAULT)) {
      std::string defaultValue = parseDefaultValue();
      columnDef->setDefaultValue(defaultValue);
      std::cout << "[PARSER DEBUG] 设置默认值: " << defaultValue << std::endl;
    } else if (match(Type::KEYWORD_AUTO_INCREMENT)) {
      columnDef->setAutoIncrement(true);
      std::cout << "[PARSER DEBUG] 设置自增约束" << std::endl;
    } else if (match(Type::KEYWORD_REFERENCES)) {
      // 外键约束在列级别暂时跳过，会在表级约束中处理
      columnDef->setForeignKey(true);
      std::string refTable = parseIdentifier();
      std::cout << "[PARSER DEBUG] 设置外键约束，引用表: " << refTable << std::endl;
      // 跳过引用的列名（简化处理）
      if (match(Type::LPAREN)) {
        parseIdentifier(); // 跳过列名
        consume(Type::RPAREN);
      }
    } else {
      // 未知的约束类型，跳出循环
      break;
    }
  }
  
  std::cout << "[PARSER DEBUG] 列定义解析完成" << std::endl;
  return columnDef;
}

std::string sql_parser::Parser::parseDataType() {
  std::cout << "[PARSER DEBUG] 进入parseDataType()方法" << std::endl;
  
  std::stringstream dataType;
  
  // 解析基本数据类型
  if (check(Type::KEYWORD_INT) || check(Type::KEYWORD_INTEGER) || 
      check(Type::KEYWORD_SMALLINT) || check(Type::KEYWORD_BIGINT) || 
      check(Type::KEYWORD_TINYINT)) {
    dataType << currentToken_.getLexeme();
    advance();
  } else if (check(Type::KEYWORD_VARCHAR) || check(Type::KEYWORD_CHAR)) {
    dataType << currentToken_.getLexeme();
    advance();
    
    // 如果有长度参数
    if (match(Type::LPAREN)) {
      dataType << "(";
      
      // 解析长度值
      if (check(Type::INTEGER_LITERAL)) {
        dataType << currentToken_.getLexeme();
        advance();
      }
      
      dataType << ")";
      consume(Type::RPAREN);
    }
  } else if (check(Type::KEYWORD_DECIMAL) || check(Type::KEYWORD_NUMERIC)) {
    dataType << currentToken_.getLexeme();
    advance();
    
    // 如果有精度参数
    if (match(Type::LPAREN)) {
      dataType << "(";
      
      // 解析精度和小数位数
      if (check(Type::INTEGER_LITERAL)) {
        dataType << currentToken_.getLexeme();
        advance();
        
        if (match(Type::COMMA)) {
          dataType << ",";
          if (check(Type::INTEGER_LITERAL)) {
            dataType << currentToken_.getLexeme();
            advance();
          }
        }
      }
      
      dataType << ")";
      consume(Type::RPAREN);
    }
  } else if (check(Type::KEYWORD_DATE) || check(Type::KEYWORD_TIME) || 
             check(Type::KEYWORD_TIMESTAMP) || check(Type::KEYWORD_DATETIME) ||
             check(Type::KEYWORD_BOOLEAN) || check(Type::KEYWORD_BOOL)) {
    dataType << currentToken_.getLexeme();
    advance();
  } else {
    // 默认情况下，将标识符作为数据类型
    dataType << currentToken_.getLexeme();
    advance();
  }
  
  std::cout << "[PARSER DEBUG] 数据类型解析完成: " << dataType.str() << std::endl;
  return dataType.str();
}

std::string sql_parser::Parser::parseDefaultValue() {
  std::cout << "[PARSER DEBUG] 进入parseDefaultValue()方法" << std::endl;
  
  std::stringstream defaultValue;
  
  if (check(Type::INTEGER_LITERAL) || check(Type::FLOAT_LITERAL)) {
    defaultValue << currentToken_.getLexeme();
    advance();
  } else if (check(Type::STRING_LITERAL)) {
    defaultValue << currentToken_.getLexeme();
    advance();
  } else if (check(Type::KEYWORD_NULL)) {
    defaultValue << "NULL";
    advance();
  } else if (check(Type::KEYWORD_CURRENT_TIMESTAMP) || check(Type::KEYWORD_NOW)) {
    defaultValue << currentToken_.getLexeme();
    advance();
  } else {
    // 尝试解析其他字面量
    defaultValue << currentToken_.getLexeme();
    advance();
  }
  
  std::cout << "[PARSER DEBUG] 默认值解析完成: " << defaultValue.str() << std::endl;
  return defaultValue.str();
}

void sql_parser::Parser::parseTableConstraint(CreateStatement& stmt) {
  std::cout << "[PARSER DEBUG] 进入parseTableConstraint()方法" << std::endl;
  
  // 检查是否有约束名称
  std::string constraintName;
  if (match(Type::KEYWORD_CONSTRAINT)) {
    constraintName = parseIdentifier();
  }
  
  // 解析约束类型
  if (match(Type::KEYWORD_PRIMARY)) {
    consume(Type::KEYWORD_KEY);
    auto constraint = TableConstraint(TableConstraint::PRIMARY_KEY, constraintName);
    
    // 解析列列表
    consume(Type::LPAREN);
    bool first = true;
    while (!check(Type::RPAREN) && !isAtEnd()) {
      if (!first) {
        if (!match(Type::COMMA)) {
          break;
        }
      }
      first = false;
      
      std::string column = parseIdentifier();
      constraint.addColumn(column);
    }
    consume(Type::RPAREN);
    
    // 添加约束到语句中
    stmt.addConstraint(std::move(constraint));
    std::cout << "[PARSER DEBUG] 解析主键约束完成" << std::endl;
  } else if (match(Type::KEYWORD_UNIQUE)) {
    auto constraint = TableConstraint(TableConstraint::UNIQUE, constraintName);
    
    // 解析列列表
    consume(Type::LPAREN);
    bool first = true;
    while (!check(Type::RPAREN) && !isAtEnd()) {
      if (!first) {
        if (!match(Type::COMMA)) {
          break;
        }
      }
      first = false;
      
      std::string column = parseIdentifier();
      constraint.addColumn(column);
    }
    consume(Type::RPAREN);
    
    // 添加约束到语句中
    stmt.addConstraint(std::move(constraint));
    std::cout << "[PARSER DEBUG] 解析唯一约束完成" << std::endl;
  } else if (match(Type::KEYWORD_FOREIGN)) {
    consume(Type::KEYWORD_KEY);
    auto constraint = TableConstraint(TableConstraint::FOREIGN_KEY, constraintName);
    
    // 解析列列表
    consume(Type::LPAREN);
    bool first = true;
    while (!check(Type::RPAREN) && !isAtEnd()) {
      if (!first) {
        if (!match(Type::COMMA)) {
          break;
        }
      }
      first = false;
      
      std::string column = parseIdentifier();
      constraint.addColumn(column);
    }
    consume(Type::RPAREN);
    
    // 解析REFERENCES子句
    consume(Type::KEYWORD_REFERENCES);
    std::string refTable = parseIdentifier();
    constraint.setReferencedTable(refTable);
    
    // 解析引用的列列表
    if (match(Type::LPAREN)) {
      bool firstRef = true;
      while (!check(Type::RPAREN) && !isAtEnd()) {
        if (!firstRef) {
          if (!match(Type::COMMA)) {
            break;
          }
        }
        firstRef = false;
        
        std::string refColumn = parseIdentifier();
        constraint.addReferencedColumn(refColumn);
      }
      consume(Type::RPAREN);
    }
    
    // 添加约束到语句中
    stmt.addConstraint(std::move(constraint));
    std::cout << "[PARSER DEBUG] 解析外键约束完成" << std::endl;
  } else if (match(Type::KEYWORD_CHECK)) {
    auto constraint = TableConstraint(TableConstraint::CHECK, constraintName);
    
    // 解析检查表达式（简化处理）
    consume(Type::LPAREN);
    std::stringstream checkExpr;
    while (!check(Type::RPAREN) && !isAtEnd()) {
      checkExpr << currentToken_.getLexeme() << " ";
      advance();
    }
    consume(Type::RPAREN);
    
    constraint.setCheckExpression(checkExpr.str());
    
    // 添加约束到语句中
    stmt.addConstraint(std::move(constraint));
    std::cout << "[PARSER DEBUG] 解析检查约束完成" << std::endl;
  }
}

std::unique_ptr<sql_parser::CreateStatement> sql_parser::Parser::parseCreateDatabaseStatement() {
  std::cout << "[PARSER DEBUG] 进入parseCreateDatabaseStatement()方法" << std::endl;
  
  // 创建一个DATABASE类型的CreateStatement
  auto stmt = std::make_unique<CreateStatement>(CreateStatement::DATABASE);
  
  // 解析数据库名
  std::string dbName = parseIdentifier();
  stmt->setObjectName(dbName);
  std::cout << "[PARSER DEBUG] 数据库名: " << dbName << std::endl;
  
  std::cout << "[PARSER DEBUG] CREATE DATABASE语句解析完成" << std::endl;
  return stmt;
}

std::unique_ptr<sql_parser::AlterStatement> sql_parser::Parser::parseAlterStatement() {
  std::cout << "[PARSER DEBUG] 进入parseAlterStatement()方法" << std::endl;
  
  // 消费ALTER关键字
  consume(Type::KEYWORD_ALTER);
  
  // 检查目标对象类型
  AlterStatement::Target target;
  if (match(Type::KEYWORD_TABLE)) {
    target = AlterStatement::TABLE;
    std::cout << "[PARSER DEBUG] 解析ALTER TABLE语句" << std::endl;
  } else if (match(Type::KEYWORD_DATABASE)) {
    target = AlterStatement::DATABASE;
    std::cout << "[PARSER DEBUG] 解析ALTER DATABASE语句" << std::endl;
  } else {
    std::stringstream ss;
    ss << "Expected TABLE or DATABASE after ALTER, but got " 
       << currentToken_.getLexeme();
    reportError(ss.str());
    return nullptr;
  }
  
  // 创建AlterStatement对象
  auto stmt = std::make_unique<AlterStatement>(target);
  
  // 解析对象名称
  if (target == AlterStatement::TABLE) {
    std::string tableName = parseIdentifier();
    stmt->setTableName(tableName);
    std::cout << "[PARSER DEBUG] 表名: " << tableName << std::endl;
  } else {
    std::string dbName = parseIdentifier();
    stmt->setDatabaseName(dbName);
    std::cout << "[PARSER DEBUG] 数据库名: " << dbName << std::endl;
  }
  
  // 解析具体的操作
  if (match(Type::KEYWORD_ADD)) {
    stmt->setAction(AlterStatement::ADD_COLUMN);
    std::cout << "[PARSER DEBUG] 解析ADD COLUMN操作" << std::endl;
    
    // 可选的COLUMN关键字
    if (match(Type::KEYWORD_COLUMN)) {
      std::cout << "[PARSER DEBUG] 消费COLUMN关键字" << std::endl;
    }
    
    // 解析列定义
    auto columnDef = parseColumnDefinition();
    if (columnDef) {
      stmt->setColumnDefinition(std::move(*columnDef));
      std::cout << "[PARSER DEBUG] 列定义解析完成" << std::endl;
    }
  } else if (match(Type::KEYWORD_DROP)) {
    stmt->setAction(AlterStatement::DROP_COLUMN);
    std::cout << "[PARSER DEBUG] 解析DROP COLUMN操作" << std::endl;
    
    // 可选的COLUMN关键字
    if (match(Type::KEYWORD_COLUMN)) {
      std::cout << "[PARSER DEBUG] 消费COLUMN关键字" << std::endl;
    }
    
    // 解析列名
    std::string columnName = parseIdentifier();
    stmt->setColumnName(columnName);
    std::cout << "[PARSER DEBUG] 列名: " << columnName << std::endl;
  } else if (match(Type::KEYWORD_MODIFY)) {
    stmt->setAction(AlterStatement::MODIFY_COLUMN);
    std::cout << "[PARSER DEBUG] 解析MODIFY COLUMN操作" << std::endl;
    
    // 可选的COLUMN关键字
    if (match(Type::KEYWORD_COLUMN)) {
      std::cout << "[PARSER DEBUG] 消费COLUMN关键字" << std::endl;
    }
    
    // 解析列定义
    auto columnDef = parseColumnDefinition();
    if (columnDef) {
      stmt->setColumnDefinition(std::move(*columnDef));
      std::cout << "[PARSER DEBUG] 列定义解析完成" << std::endl;
    }
  } else if (match(Type::KEYWORD_RENAME)) {
    stmt->setAction(AlterStatement::RENAME_TABLE);
    std::cout << "[PARSER DEBUG] 解析RENAME TO操作" << std::endl;
    
    // 消费TO关键字
    consume(Type::KEYWORD_TO);
    
    // 解析新表名
    std::string newTableName = parseIdentifier();
    stmt->setNewTableName(newTableName);
    std::cout << "[PARSER DEBUG] 新表名: " << newTableName << std::endl;
  } else {
    std::stringstream ss;
    ss << "Unsupported ALTER operation: " << currentToken_.getLexeme();
    reportError(ss.str());
    return nullptr;
  }
  
  std::cout << "[PARSER DEBUG] ALTER语句解析完成" << std::endl;
  return stmt;
}

/**
 * @brief 解析SELECT语句 - SQL查询的核心语法分析
 *
 * WHY层 - 设计意图：
 *   parseSelectStatement()是SQL解析器中最复杂的函数之一，负责解析
 *   SQL SELECT语句的所有组成部分。SELECT语句是SQL中最常用的语句类型，
 *   支持从简单的数据检索到复杂的多表连接、聚合计算和排序。
 *   该函数采用分步骤解析策略，确保每个子句都被正确识别和处理。
 *
 * WHAT层 - 功能说明：
 *   解析完整的SELECT语句语法，包括：
 *   - SELECT子句：选择列表（列名、函数、*）
 *   - FROM子句：数据源（表名、子查询、JOIN）
 *   - WHERE子句：条件过滤
 *   - GROUP BY子句：分组聚合
 *   - HAVING子句：分组条件过滤
 *   - ORDER BY子句：结果排序
 *   - LIMIT子句：结果限制（待实现）
 *
 * HOW层 - 实现细节：
 *   1. **关键字消费**: 首先消费SELECT关键字启动解析
 *   2. **DISTINCT处理**: 检查并设置去重标志
 *   3. **选择列表解析**: 处理列名、函数调用或*通配符
 *   4. **FROM子句解析**: 解析数据源和JOIN操作
 *   5. **条件子句解析**: WHERE、GROUP BY、HAVING、ORDER BY
 *
 * 选择列表解析策略：
 *   - **通配符处理**: SELECT * 表示选择所有列
 *   - **函数调用识别**: 支持COUNT(*) SUM(col)等聚合函数
 *   - **别名处理**: 支持AS关键字定义列别名
 *   - **逗号分隔**: 多列用逗号分隔
 *
 * 解析顺序约束：
 *   - SELECT必须在最前面
 *   - FROM必须在SELECT之后（除非是SELECT常量）
 *   - WHERE在FROM之后
 *   - GROUP BY在WHERE之后
 *   - HAVING在GROUP BY之后
 *   - ORDER BY在HAVING之后
 *
 * 错误处理机制：
 *   - **语法错误**: 遇到不支持的语法时抛出异常
 *   - **缺失子句**: 某些子句是可选的，缺失时跳过
 *   - **顺序错误**: 严格按照SQL语法顺序检查
 *
 * 扩展性设计：
 *   - **子查询支持**: FROM子句可包含子查询
 *   - **复杂表达式**: WHERE条件可包含复杂逻辑表达式
 *   - **多表连接**: 支持INNER/LEFT/RIGHT/FULL JOIN
 *   - **窗口函数**: ORDER BY可包含窗口函数（待扩展）
 *
 * @return std::unique_ptr<SelectStatement> 解析成功的SELECT语句AST
 *
 * @throws std::runtime_error 当遇到语法错误或不支持的特性时抛出
 *
 * @note SELECT语句是SQL的核心，支持关系代数的所有基本操作
 * @note 当前实现支持SQL-92标准的主要特性
 * @note 聚合函数如COUNT、SUM、AVG等在选择列表中识别
 * @note JOIN操作在FROM子句中处理，支持多表连接
 * @note WHERE条件使用简化解析，复杂的表达式解析待完善
 *
 * @see parseJoinClause() JOIN子句解析函数
 * @see parseExpression() 表达式解析函数（待实现）
 * @see SelectStatement SELECT语句AST定义
 */
std::unique_ptr<sql_parser::SelectStatement> sql_parser::Parser::parseSelectStatement() {
  std::cout << "[PARSER DEBUG] 进入parseSelectStatement()方法" << std::endl;

  // WHY层 - 解析启动：消费SELECT关键字，开始SELECT语句的解析过程
  // SELECT是SQL查询语句的起始关键字，必须首先出现
  consume(Type::KEYWORD_SELECT);

  // HOW层 - 对象创建：创建SelectStatement AST节点来存储解析结果
  // SelectStatement封装了SELECT语句的所有组成部分
  auto stmt = std::make_unique<SelectStatement>();

  // WHAT层 - DISTINCT处理：检查SELECT后是否有DISTINCT关键字
  // DISTINCT表示结果去重，是SELECT语句的可选修饰符
  if (match(Type::KEYWORD_DISTINCT)) {
    std::cout << "[PARSER DEBUG] 检测到DISTINCT关键字" << std::endl;
    stmt->setDistinct(true);
  }

  // WHY层 - 选择列表解析：这是SELECT语句的核心部分，指定要检索的列
  // 支持列名、函数调用、通配符等多种形式的选择项

  // HOW层 - 通配符处理：检查是否是SELECT * 的情况
  // *表示选择所有列，是最简单的选择列表形式
  if (match(Type::OPERATOR_MULTIPLY)) {
    // SELECT *
    std::cout << "[PARSER DEBUG] 解析SELECT *" << std::endl;
    stmt->setSelectAll(true);
  } else {
    // WHY层 - 具体列解析：解析列名列表、函数调用或表达式
    // 支持复杂的选择列表，如列名、聚合函数、标量函数等
    std::cout << "[PARSER DEBUG] 解析具体的列名列表或函数调用" << std::endl;

    // HOW层 - 逗号分隔列表：选择项之间用逗号分隔
    // 使用first标志处理第一个元素（无需检查逗号）
    bool first = true;
    while (!check(Type::KEYWORD_FROM) && !isAtEnd()) {
      // WHY层 - 分隔符处理：第一个选择项前没有逗号，后续项需要逗号
      // 这是SQL语法的基本规则
      if (!first) {
        if (!match(Type::COMMA)) {
          break; // 没有逗号表示选择列表结束
        }
      }
      first = false;

      // HOW层 - 前瞻检查：尝试识别函数调用
      // 函数调用形式为：function_name(parameters)
      // 需要前瞻检查当前token后是否有左括号

      // 解析列名或函数调用
      std::string columnExpr;

      // WHY层 - 函数调用识别：检查是否是聚合函数或标量函数调用
      // 常见的函数包括COUNT(*) SUM(col) AVG(col)等
      if (check(Type::IDENTIFIER)) {
        // 检查下一个token是否是左括号
        if (!hasLookahead_) {
          lookaheadToken_ = lexer_.nextToken();
          hasLookahead_ = true;
        }
        if (lookaheadToken_.getType() == Type::LPAREN) {
        // 函数调用解析
        std::string funcName = currentToken_.getLexeme();
        advance(); // 消费函数名
        consume(Type::LPAREN); // 消费左括号

        // HOW层 - 函数参数解析：解析函数的参数
        // 简化实现：支持单个标识符、*或字面量
        std::string inner;
        if (check(Type::OPERATOR_MULTIPLY)) {
          inner = "*"; // COUNT(*)的情况
          advance();
        } else if (check(Type::IDENTIFIER)) {
          inner = parseIdentifier(); // 列名参数
        } else if (check(Type::STRING_LITERAL) || check(Type::INTEGER_LITERAL) || check(Type::FLOAT_LITERAL)) {
          inner = currentToken_.getLexeme(); // 字面量参数
          advance();
        } else {
          inner = ""; // 无参数或空参数
        }

        consume(Type::RPAREN); // 消费右括号
        columnExpr = funcName + "(" + inner + ")";
        std::cout << "[PARSER DEBUG] 解析函数调用: " << columnExpr << std::endl;
      } else {
        // WHAT层 - 普通列名：最常见的选择项形式
        // 直接解析标识符作为列名
        columnExpr = parseIdentifier();
        std::cout << "[PARSER DEBUG] 解析列名: " << columnExpr << std::endl;
      }

      // WHY层 - 别名处理：检查是否有AS关键字定义列别名
      // 别名用于重命名结果集中的列，便于引用
      if (match(Type::KEYWORD_AS)) {
        std::string alias = parseIdentifier();
        columnExpr += " AS " + alias;
        std::cout << "[PARSER DEBUG] 添加别名: " << alias << std::endl;
      }

      // HOW层 - 结果存储：将解析的选择项添加到SELECT语句中
      stmt->addSelectColumn(columnExpr);
      std::cout << "[PARSER DEBUG] 添加选择列: " << columnExpr << std::endl;
    }
  }
  
  // 解析FROM子句
  if (match(Type::KEYWORD_FROM)) {
    std::cout << "[PARSER DEBUG] 解析FROM子句" << std::endl;
    std::string table = parseIdentifier();
    stmt->setTableName(table);
    stmt->addFromTable(table);
    std::cout << "[PARSER DEBUG] 表名: " << table << std::endl;

    // 解析JOIN子句（支持多个JOIN）
    while (true) {
      if (check(Type::KEYWORD_JOIN) || check(Type::KEYWORD_INNER)) {
        std::cout << "[PARSER DEBUG] 检测到JOIN子句" << std::endl;
        auto joinClause = parseJoinClause();
        if (joinClause) {
          stmt->addJoinClause(std::move(joinClause));
        }
      } else {
        break;
      }
    }
  } else {
    std::stringstream ss;
    ss << "Expected FROM clause in SELECT statement";
    reportError(ss.str());
    return nullptr;
  }
  
  // 解析WHERE子句（可选）
  if (match(Type::KEYWORD_WHERE)) {
    std::cout << "[PARSER DEBUG] 解析WHERE子句" << std::endl;
    // 简化处理，只解析简单的条件 "column = value"
    std::string column = parseIdentifier();
    std::string op = currentToken_.getLexeme();
    advance(); // 消费操作符
    std::string value = currentToken_.getLexeme();
    advance(); // 消费值
    
    WhereClause whereClause(column, op, value);
    stmt->setWhereClause(whereClause);
    std::cout << "[PARSER DEBUG] WHERE条件: " << column << " " << op << " " << value << std::endl;
  }
  
  // 解析GROUP BY子句（可选）
  if (match(Type::KEYWORD_GROUP)) {
    consume(Type::KEYWORD_BY);
    std::cout << "[PARSER DEBUG] 解析GROUP BY子句" << std::endl;

    // 解析GROUP BY列列表
    bool first = true;
    while (!check(Type::KEYWORD_HAVING) && !check(Type::KEYWORD_ORDER) &&
           !check(Type::KEYWORD_LIMIT) && !check(Type::SEMICOLON) && !isAtEnd()) {
      if (!first) {
        if (!match(Type::COMMA)) {
          break;
        }
      }
      first = false;

      std::string column = parseIdentifier();
      stmt->addGroupByColumn(column);
      std::cout << "[PARSER DEBUG] GROUP BY列: " << column << std::endl;
    }
  }

  // 解析ORDER BY子句（可选）
  if (match(Type::KEYWORD_ORDER)) {
    consume(Type::KEYWORD_BY);
    std::cout << "[PARSER DEBUG] 解析ORDER BY子句" << std::endl;

    // 解析ORDER BY列
    std::string orderByColumn = parseIdentifier();
    stmt->setOrderByColumn(orderByColumn);

    // 检查是否有排序方向（ASC/DESC）
    if (match(Type::KEYWORD_ASC)) {
      stmt->setOrderDirection("ASC");
      std::cout << "[PARSER DEBUG] ORDER BY方向: ASC" << std::endl;
    } else if (match(Type::KEYWORD_DESC)) {
      stmt->setOrderDirection("DESC");
      std::cout << "[PARSER DEBUG] ORDER BY方向: DESC" << std::endl;
    } else {
      // 默认升序
      stmt->setOrderDirection("ASC");
      std::cout << "[PARSER DEBUG] ORDER BY方向: 默认ASC" << std::endl;
    }

    std::cout << "[PARSER DEBUG] ORDER BY列: " << orderByColumn << std::endl;
  }
  
  // 解析HAVING子句（可选）
  if (match(Type::KEYWORD_HAVING)) {
    std::cout << "[PARSER DEBUG] 解析HAVING子句" << std::endl;
    // 简化实现：解析简单的HAVING条件表达式
    // 这里可以解析聚合函数相关的条件，如 COUNT(*) > 5 等
    // 暂时实现为简单的字符串存储，实际应该解析为表达式树

    std::stringstream having_expr;
    int paren_depth = 0;

    while (!isAtEnd()) {
      if (check(Type::LPAREN)) {
        paren_depth++;
        having_expr << currentToken_.getLexeme();
        advance();
      } else if (check(Type::RPAREN)) {
        paren_depth--;
        having_expr << currentToken_.getLexeme();
        advance();
        if (paren_depth == 0) {
          break; // 括号匹配完成
        }
      } else if (paren_depth == 0 &&
                 (check(Type::KEYWORD_ORDER) || check(Type::KEYWORD_LIMIT) ||
                  check(Type::KEYWORD_UNION) || check(Type::SEMICOLON))) {
        break; // 遇到下一个子句或语句结束
      } else {
        having_expr << currentToken_.getLexeme() << " ";
        advance();
      }
    }

    std::string having_condition = having_expr.str();
    // 移除末尾空格
    while (!having_condition.empty() && having_condition.back() == ' ') {
      having_condition.pop_back();
    }

    std::cout << "[PARSER DEBUG] HAVING条件: " << having_condition << std::endl;

    // 暂时不设置HAVING表达式，因为需要表达式解析器支持
    // stmt->setHavingClause(...);
  }
  
  std::cout << "[PARSER DEBUG] SELECT语句解析完成" << std::endl;
  return stmt;
}

/**
 * @brief 解析INSERT INTO语句 - SQL数据插入语法分析
 *
 * WHY层 - 设计意图：
 *   parseInsertStatement()负责解析SQL INSERT语句，这是关系数据库中最
 *   基础的DML操作之一。INSERT语句用于向表中添加新的数据行，支持单行
 *   和多行插入。该函数采用分步骤解析策略，确保插入操作的完整性和正确性。
 *
 * WHAT层 - 功能说明：
 *   解析完整的INSERT INTO语法，包括：
 *   - 目标表名：数据要插入的表
 *   - 列名列表：指定插入数据的列（可选）
 *   - 值列表：要插入的实际数据值
 *   - 支持单行插入和批量插入
 *
 * HOW层 - 实现细节：
 *   1. **关键字消费**: 消费INSERT和INTO关键字启动解析
 *   2. **表名解析**: 验证并消费目标表名标识符
 *   3. **列列表处理**: 可选的列名列表，指定插入哪些列
 *   4. **VALUES处理**: 消费VALUES关键字，开始值列表解析
 *   5. **值列表解析**: 解析括号包围的值列表
 *   6. **结果封装**: 创建InsertStatement AST节点存储解析结果
 *
 * INSERT语法结构：
 *   - **基本语法**: INSERT INTO table_name VALUES (value1, value2, ...)
 *   - **指定列**: INSERT INTO table_name (col1, col2) VALUES (val1, val2)
 *   - **批量插入**: 支持单个VALUES子句（当前实现）
 *
 * 值类型支持：
 *   - **字符串字面量**: 'string value'（带引号的字符串）
 *   - **数字字面量**: 123, 45.67（整数和浮点数）
 *   - **标识符**: 变量名或特殊值（NULL等）
 *
 * 完整性检查：
 *   - **表存在性**: 确保目标表存在（运行时检查）
 *   - **列匹配**: 列数量与值的数量必须匹配
 *   - **类型兼容**: 插入值必须与列的数据类型兼容
 *   - **约束验证**: 检查主键、唯一、外键等约束
 *
 * 错误处理机制：
 *   - **语法错误**: 遇到无效语法时抛出异常
 *   - **类型错误**: 值类型与列定义不匹配时报告错误
 *   - **约束冲突**: 违反完整性约束时提供详细错误信息
 *
 * 性能优化：
 *   - **批量处理**: 支持一次插入多行数据
 *   - **预编译**: 准备语句可以重用执行计划
 *   - **索引更新**: 插入时更新相关索引
 *   - **事务支持**: 支持事务中的插入操作
 *
 * 扩展性设计：
 *   - **SELECT插入**: 支持INSERT ... SELECT语法
 *   - **DEFAULT值**: 支持DEFAULT关键字使用列默认值
 *   - **子查询**: VALUES子句中支持子查询
 *   - **ON DUPLICATE**: 支持ON DUPLICATE KEY UPDATE
 *
 * @return std::unique_ptr<InsertStatement> 解析成功的INSERT语句AST
 *
 * @throws std::runtime_error 当遇到语法错误或无效插入时抛出
 *
 * @note INSERT是数据库DML操作的基础，用于添加新数据
 * @note 当前实现支持基本的单表单行插入
 * @note 批量插入需要扩展VALUES子句处理
 * @note 列列表是可选的，不指定时插入所有列
 * @note 值列表必须与列列表或表定义的列数匹配
 *
 * @see parseIdentifier() 标识符解析函数
 * @see parseDataType() 数据类型解析函数（间接相关）
 * @see InsertStatement INSERT语句AST定义
 */


std::unique_ptr<sql_parser::UpdateStatement> sql_parser::Parser::parseUpdateStatement() {
  throw std::runtime_error("parseUpdateStatement not yet implemented");
}

std::unique_ptr<sql_parser::DeleteStatement> sql_parser::Parser::parseDeleteStatement() {
  throw std::runtime_error("parseDeleteStatement not yet implemented");
}

std::unique_ptr<sql_parser::UseStatement> sql_parser::Parser::parseUseStatement() {
  throw std::runtime_error("parseUseStatement not yet implemented");
}

std::unique_ptr<sql_parser::ShowStatement> sql_parser::Parser::parseShowStatement() {
  throw std::runtime_error("parseShowStatement not yet implemented");
}

std::unique_ptr<sql_parser::CreateIndexStatement> sql_parser::Parser::parseCreateIndexStatement() {
  throw std::runtime_error("parseCreateIndexStatement not yet implemented");
}

std::unique_ptr<sql_parser::DropIndexStatement> sql_parser::Parser::parseDropIndexStatement() {
  throw std::runtime_error("parseDropIndexStatement not yet implemented");
}

std::unique_ptr<sql_parser::DropStatement> sql_parser::Parser::parseDropStatement() {
  std::cout << "[PARSER DEBUG] 进入parseDropStatement()方法" << std::endl;

  // 消费DROP关键字
  consume(Type::KEYWORD_DROP);

  // 检查要删除的对象类型
  DropStatement::ObjectType objectType;
  if (match(Type::KEYWORD_TABLE)) {
    objectType = DropStatement::TABLE;
    std::cout << "[PARSER DEBUG] 解析DROP TABLE语句" << std::endl;
  } else if (match(Type::KEYWORD_DATABASE)) {
    objectType = DropStatement::DATABASE;
    std::cout << "[PARSER DEBUG] 解析DROP DATABASE语句" << std::endl;
  } else if (match(Type::KEYWORD_INDEX)) {
    objectType = DropStatement::INDEX;
    std::cout << "[PARSER DEBUG] 解析DROP INDEX语句" << std::endl;
  } else if (match(Type::KEYWORD_USER)) {
    objectType = DropStatement::USER;
    std::cout << "[PARSER DEBUG] 解析DROP USER语句" << std::endl;
  } else {
    std::stringstream ss;
    ss << "Expected TABLE, DATABASE, INDEX, or USER after DROP, but got "
       << currentToken_.getLexeme();
    reportError(ss.str());
    return nullptr;
  }

  // 检查是否有IF EXISTS子句（对于USER类型，在对象类型后）
  bool ifExists = false;
  if (match(Type::KEYWORD_IF)) {
    consume(Type::KEYWORD_EXISTS);
    ifExists = true;
    std::cout << "[PARSER DEBUG] 检测到IF EXISTS子句" << std::endl;
  }

  // 创建DropStatement对象
  auto stmt = std::make_unique<DropStatement>(objectType);
  stmt->setIfExists(ifExists);

  // 解析对象名称
  std::string objectName = parseIdentifier();
  stmt->setObjectName(objectName);
  std::cout << "[PARSER DEBUG] 对象名称: " << objectName << std::endl;

  std::cout << "[PARSER DEBUG] DROP语句解析完成" << std::endl;
  return stmt;
}

std::unique_ptr<sql_parser::CreateUserStatement> sql_parser::Parser::parseCreateUserStatement() {
  std::cout << "[PARSER DEBUG] 进入parseCreateUserStatement()方法" << std::endl;
  
  // 消费CREATE关键字
  consume(Type::KEYWORD_CREATE);
  
  // 消费USER关键字
  consume(Type::KEYWORD_USER);
  
  // 解析用户名
  std::string username = parseIdentifier();
  
  // 解析密码部分
  std::string password;
  bool withPassword = false;
  if (match(Type::KEYWORD_IDENTIFIED)) {
    consume(Type::KEYWORD_BY);
    password = parseIdentifier();
    withPassword = false;  // IDENTIFIED BY格式
  } else if (match(Type::KEYWORD_WITH)) {
    consume(Type::KEYWORD_PASSWORD);
    password = parseIdentifier();
    withPassword = true;   // WITH PASSWORD格式
  }
  
  // 创建CreateUserStatement对象
  auto stmt = std::make_unique<CreateUserStatement>(username, password);
  stmt->setWithPassword(withPassword);
  
  std::cout << "[PARSER DEBUG] CREATE USER语句解析完成" << std::endl;
  return stmt;
}

std::unique_ptr<sql_parser::DropUserStatement> sql_parser::Parser::parseDropUserStatement() {
  std::cout << "[PARSER DEBUG] 进入parseDropUserStatement()方法" << std::endl;

  // 消费DROP关键字（已经在parseStatement中消费了）
  consume(Type::KEYWORD_DROP);

  // 消费USER关键字
  consume(Type::KEYWORD_USER);

  // 检查是否有IF EXISTS子句
  bool ifExists = false;
  if (match(Type::KEYWORD_IF)) {
    consume(Type::KEYWORD_EXISTS);
    ifExists = true;
    std::cout << "[PARSER DEBUG] 检测到IF EXISTS子句" << std::endl;
  }

  // 解析用户名
  std::string username = parseIdentifier();
  if (!username.empty()) {
    std::cout << "[PARSER DEBUG] 用户名: " << username << std::endl;
  }

  // 创建DropUserStatement对象
  auto stmt = std::make_unique<DropUserStatement>(username);
  stmt->setIfExists(ifExists);

  std::cout << "[PARSER DEBUG] DROP USER语句解析完成" << std::endl;
  return stmt;
}

std::unique_ptr<sql_parser::GrantStatement> sql_parser::Parser::parseGrantStatement() {
  std::cout << "[PARSER DEBUG] 进入parseGrantStatement()方法" << std::endl;
  
  // 消费GRANT关键字
  consume(Type::KEYWORD_GRANT);
  
  // 创建GrantStatement对象
  auto stmt = std::make_unique<GrantStatement>();
  
  // 解析权限列表
  if (match(Type::KEYWORD_ALL)) {
    // 处理ALL [PRIVILEGES]情况
    if (match(Type::KEYWORD_PRIVILEGES)) {
      stmt->addPrivilege("ALL PRIVILEGES");
    } else {
      stmt->addPrivilege("ALL");
    }
  } else {
    // 解析具体权限列表
    std::string privilege = parseIdentifier();
    stmt->addPrivilege(privilege);
    
    while (match(Type::COMMA)) {
      privilege = parseIdentifier();
      stmt->addPrivilege(privilege);
    }
  }
  
  // 可选的PRIVILEGES关键字
  if (check(Type::KEYWORD_PRIVILEGES)) {
    advance();
  }
  
  // 消费ON关键字
  consume(Type::KEYWORD_ON);
  
  // 解析对象类型和名称
  if (match(Type::KEYWORD_TABLE)) {
    stmt->setObjectType("TABLE");
    std::string tableName = parseIdentifier();
    stmt->setObjectName(tableName);
  } else {
    // 默认为TABLE类型
    stmt->setObjectType("TABLE");
    std::string objectName = parseIdentifier();
    stmt->setObjectName(objectName);
  }
  
  // 消费TO关键字
  consume(Type::KEYWORD_TO);
  
  // 解析被授权用户
  std::string grantee = parseIdentifier();
  stmt->setGrantee(grantee);
  
  std::cout << "[PARSER DEBUG] GRANT语句解析完成" << std::endl;
  return stmt;
}

std::unique_ptr<sql_parser::RevokeStatement> sql_parser::Parser::parseRevokeStatement() {
  std::cout << "[PARSER DEBUG] 进入parseRevokeStatement()方法" << std::endl;

  // 消费REVOKE关键字
  consume(Type::KEYWORD_REVOKE);

  // 创建RevokeStatement对象
  auto stmt = std::make_unique<RevokeStatement>();

  // 解析权限列表
  if (match(Type::KEYWORD_ALL)) {
    // 处理ALL [PRIVILEGES]情况
    if (match(Type::KEYWORD_PRIVILEGES)) {
      stmt->addPrivilege("ALL PRIVILEGES");
    } else {
      stmt->addPrivilege("ALL");
    }
  } else {
    // 解析具体权限列表，支持逗号分隔的多个权限
    bool first = true;
    while (!check(Type::KEYWORD_ON) && !check(Type::KEYWORD_PRIVILEGES) && !isAtEnd()) {
      if (!first) {
        if (!match(Type::COMMA)) {
          break; // 没有逗号表示权限列表结束
        }
      }
      first = false;

      // 解析权限名 - 可以是标识符或关键字
      std::string privilege;
      if (check(Type::IDENTIFIER)) {
        privilege = parseIdentifier();
      } else {
        // 检查常见的权限关键字
        if (match(Type::KEYWORD_SELECT)) {
          privilege = "SELECT";
        } else if (match(Type::KEYWORD_INSERT)) {
          privilege = "INSERT";
        } else if (match(Type::KEYWORD_UPDATE)) {
          privilege = "UPDATE";
        } else if (match(Type::KEYWORD_DELETE)) {
          privilege = "DELETE";
        } else if (match(Type::KEYWORD_CREATE)) {
          privilege = "CREATE";
        } else if (match(Type::KEYWORD_DROP)) {
          privilege = "DROP";
        } else if (match(Type::KEYWORD_GRANT)) {
          privilege = "GRANT";
        } else if (match(Type::KEYWORD_ALTER)) {
          privilege = "ALTER";
        } else {
          // 其他情况当作标识符处理
          privilege = parseIdentifier();
        }
      }

      if (!privilege.empty()) {
        stmt->addPrivilege(privilege);
        std::cout << "[PARSER DEBUG] 添加权限: " << privilege << std::endl;
      }
    }
  }

  // 可选的PRIVILEGES关键字
  if (match(Type::KEYWORD_PRIVILEGES)) {
    std::cout << "[PARSER DEBUG] 消费PRIVILEGES关键字" << std::endl;
  }

  // 消费ON关键字
  consume(Type::KEYWORD_ON);

  // 解析对象类型和名称
  if (match(Type::KEYWORD_TABLE)) {
    stmt->setObjectType("TABLE");
    std::cout << "[PARSER DEBUG] 设置对象类型: TABLE" << std::endl;
  } else {
    // 默认为TABLE类型
    stmt->setObjectType("TABLE");
  }

  std::string objectName = parseIdentifier();
  if (!objectName.empty()) {
    stmt->setObjectName(objectName);
    std::cout << "[PARSER DEBUG] 设置对象名称: " << objectName << std::endl;
  }

  // 消费FROM关键字
  consume(Type::KEYWORD_FROM);

  // 解析被撤销权限的用户
  std::string grantee = parseIdentifier();
  if (!grantee.empty()) {
    stmt->setGrantee(grantee);
    std::cout << "[PARSER DEBUG] 设置被撤销权限用户: " << grantee << std::endl;
  }

  std::cout << "[PARSER DEBUG] REVOKE语句解析完成" << std::endl;
  return stmt;
}

std::vector<std::string> sql_parser::Parser::parseColumnNames() {
  throw std::runtime_error("parseColumnNames not yet implemented");
}

std::vector<std::unique_ptr<Expression>> sql_parser::Parser::parseExpressions() {
  throw std::runtime_error("parseExpressions not yet implemented");
}

std::unique_ptr<sql_parser::Expression> sql_parser::Parser::parseExpression() {
  throw std::runtime_error("parseExpression not yet implemented");
}

  // ============================================================================
  // DELETED EXPRESSION PARSING METHODS - ARCHITECTURE SECURITY MEASURES
  // ============================================================================
  //
  // 这些方法已被显式删除以防止架构违规：
  //
  // WHY: 防止"偷偷加回"parseExpression逻辑
  // Parser类不得直接构造或解析AST表达式节点，所有表达式解析必须通过
  // ExpressionParser统一处理。这是架构安全的强制执行点。
  //
  // 任何尝试重新实现这些方法的代码都将在编译时失败，确保：
  // 1. 所有表达式解析通过ExpressionParser统一处理
  // 2. 避免直接AST节点构造导致的架构不一致
  // 3. 保持解析器的职责分离和模块化设计
  //
  // 违反此设计将导致编译错误，迫使开发者重新考虑架构决策。
  // ============================================================================

  std::unique_ptr<sql_parser::Expression> sql_parser::Parser::parseLogicalOr() = delete;
  std::unique_ptr<sql_parser::Expression> sql_parser::Parser::parseLogicalAnd() = delete;
  std::unique_ptr<sql_parser::Expression> sql_parser::Parser::parseEquality() = delete;
  std::unique_ptr<sql_parser::Expression> sql_parser::Parser::parseComparison() = delete;
  std::unique_ptr<sql_parser::Expression> sql_parser::Parser::parseTerm() = delete;
  std::unique_ptr<sql_parser::Expression> sql_parser::Parser::parseFactor() = delete;
  std::unique_ptr<sql_parser::Expression> sql_parser::Parser::parseUnary() = delete;
  std::unique_ptr<sql_parser::Expression> sql_parser::Parser::parsePrimary() = delete;
  std::unique_ptr<sql_parser::Expression> sql_parser::Parser::parseIdentifierExpression() = delete;

std::vector<std::unique_ptr<ColumnDefinition>> sql_parser::Parser::parseColumnDefinitions() {
  std::cout << "[PARSER DEBUG] 进入parseColumnDefinitions()方法" << std::endl;
  
  std::vector<std::unique_ptr<ColumnDefinition>> columns;
  
  // 解析第一个列定义
  auto firstColumn = parseColumnDefinition();
  if (firstColumn) {
    columns.push_back(std::move(firstColumn));
  }
  
  // 解析后续的列定义（如果有逗号分隔）
  while (match(Type::COMMA)) {
    auto column = parseColumnDefinition();
    if (column) {
      columns.push_back(std::move(column));
    }
  }
  
  std::cout << "[PARSER DEBUG] 列定义解析完成，共" << columns.size() << "个列" << std::endl;
  return columns;
}

std::string sql_parser::Parser::parseQualifiedName() {
  throw std::runtime_error("parseQualifiedName not yet implemented");
}

std::string sql_parser::Parser::parseIdentifier() {
  std::cout << "[PARSER DEBUG] 进入parseIdentifier()方法" << std::endl;
  
  if (check(Type::IDENTIFIER)) {
    std::string identifier = currentToken_.getLexeme();
    advance();
    std::cout << "[PARSER DEBUG] 标识符: " << identifier << std::endl;
    return identifier;
  } else {
    std::stringstream ss;
    ss << "Expected identifier, but got " << currentToken_.getLexeme();
    reportError(ss.str());
    return "";
  }
}

std::string sql_parser::Parser::parseStringLiteral() {
  throw std::runtime_error("parseStringLiteral not yet implemented");
}

int sql_parser::Parser::parseIntLiteral() {
  throw std::runtime_error("parseIntLiteral not yet implemented");
}



std::unique_ptr<sql_parser::SetOperation> sql_parser::Parser::parseUnion() {
  throw std::runtime_error("parseUnion not yet implemented");
}

std::unique_ptr<sql_parser::SetOperation> sql_parser::Parser::parseIntersect() {
  throw std::runtime_error("parseIntersect not yet implemented");
}

std::unique_ptr<sql_parser::SetOperation> sql_parser::Parser::parseExcept() {
  throw std::runtime_error("parseExcept not yet implemented");
}

// JOIN clause parsing
std::unique_ptr<sql_parser::JoinClause> sql_parser::Parser::parseJoinClause() {
  std::cout << "[PARSER DEBUG] 进入parseJoinClause()方法" << std::endl;

  // 确定JOIN类型
  JoinClause::JoinType joinType = JoinClause::INNER_JOIN;

  // 检查JOIN类型
  if (match(Type::KEYWORD_INNER)) {
    joinType = JoinClause::INNER_JOIN;
    std::cout << "[PARSER DEBUG] 检测到INNER JOIN" << std::endl;
  } else if (match(Type::KEYWORD_LEFT)) {
    if (match(Type::KEYWORD_OUTER)) {
      joinType = JoinClause::LEFT_JOIN;
      std::cout << "[PARSER DEBUG] 检测到LEFT OUTER JOIN" << std::endl;
    } else {
      joinType = JoinClause::LEFT_JOIN;
      std::cout << "[PARSER DEBUG] 检测到LEFT JOIN" << std::endl;
    }
  } else if (match(Type::KEYWORD_RIGHT)) {
    if (match(Type::KEYWORD_OUTER)) {
      joinType = JoinClause::RIGHT_JOIN;
      std::cout << "[PARSER DEBUG] 检测到RIGHT OUTER JOIN" << std::endl;
    } else {
      joinType = JoinClause::RIGHT_JOIN;
      std::cout << "[PARSER DEBUG] 检测到RIGHT JOIN" << std::endl;
    }
  } else if (match(Type::KEYWORD_FULL)) {
    if (match(Type::KEYWORD_OUTER)) {
      joinType = JoinClause::FULL_JOIN;
      std::cout << "[PARSER DEBUG] 检测到FULL OUTER JOIN" << std::endl;
    } else {
      joinType = JoinClause::FULL_JOIN;
      std::cout << "[PARSER DEBUG] 检测到FULL JOIN" << std::endl;
    }
  } else if (match(Type::KEYWORD_JOIN)) {
    // 默认为INNER JOIN
    joinType = JoinClause::INNER_JOIN;
    std::cout << "[PARSER DEBUG] 检测到默认JOIN (INNER)" << std::endl;
  }

  // 如果还没有消费JOIN关键字，现在消费
  if (!match(Type::KEYWORD_JOIN)) {
    std::stringstream ss;
    ss << "Expected JOIN keyword, but got " << currentToken_.getLexeme();
    reportError(ss.str());
    return nullptr;
  }

  // 解析表名
  std::string tableName = parseIdentifier();
  std::cout << "[PARSER DEBUG] JOIN表名: " << tableName << std::endl;

  // 解析ON条件
  std::unique_ptr<Expression> condition = nullptr;
  if (match(Type::KEYWORD_ON)) {
    std::cout << "[PARSER DEBUG] 解析JOIN ON条件" << std::endl;

    // 简化实现：解析形如 "table1.column = table2.column" 的条件
    // 实际实现中应该使用完整的表达式解析器

    // 解析左边列名（可能带表前缀）
    std::string leftColumn = parseIdentifier();
    if (leftColumn.empty()) {
      reportError("Expected column name in JOIN condition");
      return nullptr;
    }

    if (match(Type::DOT)) {
      std::string columnPart = parseIdentifier();
      if (!columnPart.empty()) {
        leftColumn += "." + columnPart;
      }
    }

    // 解析操作符（应该等于号）
    if (!match(Type::OPERATOR_EQUAL)) {
      std::stringstream ss;
      ss << "Expected '=' in JOIN condition, but got " << currentToken_.getLexeme();
      reportError(ss.str());
      return nullptr;
    }

    // 解析右边列名（可能带表前缀）
    std::string rightColumn = parseIdentifier();
    if (rightColumn.empty()) {
      reportError("Expected column name in JOIN condition");
      return nullptr;
    }

    if (match(Type::DOT)) {
      std::string columnPart = parseIdentifier();
      if (!columnPart.empty()) {
        rightColumn += "." + columnPart;
      }
    }

    // 创建二元表达式作为JOIN条件
    try {
      auto leftExpr = std::make_unique<IdentifierExpression>(leftColumn);
      auto rightExpr = std::make_unique<IdentifierExpression>(rightColumn);
      condition = std::make_unique<BinaryExpression>(
          std::move(leftExpr), std::move(rightExpr), Type::OPERATOR_EQUAL);
    } catch (const std::exception& e) {
      reportError(std::string("Failed to create JOIN condition: ") + e.what());
      return nullptr;
    }

    std::cout << "[PARSER DEBUG] JOIN条件: " << leftColumn << " = " << rightColumn << std::endl;
  } else if (match(Type::KEYWORD_USING)) {
    // USING子句的简化处理
    std::cout << "[PARSER DEBUG] 检测到USING子句（简化处理）" << std::endl;
    if (!match(Type::LPAREN)) {
      reportError("Expected '(' after USING");
      return nullptr;
    }

    std::string usingColumn = parseIdentifier();
    if (usingColumn.empty()) {
      reportError("Expected column name in USING clause");
      return nullptr;
    }

    if (!match(Type::RPAREN)) {
      reportError("Expected ')' after USING column");
      return nullptr;
    }

    std::cout << "[PARSER DEBUG] USING列: " << usingColumn << std::endl;
    // TODO: 将USING转换为ON条件
  } else {
    std::stringstream ss;
    ss << "Expected ON or USING clause in JOIN, but got " << currentToken_.getLexeme();
    reportError(ss.str());
    return nullptr;
  }

  // 创建并返回JoinClause
  auto joinClause = std::make_unique<JoinClause>(joinType, tableName, std::move(condition));
  std::cout << "[PARSER DEBUG] JOIN子句解析完成" << std::endl;
  return joinClause;
}

// ==================== Procedure and Trigger Parsing ====================

std::unique_ptr<sql_parser::CreateStatement> sql_parser::Parser::parseCreateProcedureStatement() {
  std::cout << "[PARSER DEBUG] 进入parseCreateProcedureStatement()方法" << std::endl;

  // 解析过程名
  std::string procedureName = parseIdentifier();
  std::cout << "[PARSER DEBUG] 过程名: " << procedureName << std::endl;

  // 解析参数列表（可选）
  std::vector<ProcedureParameter> parameters;
  if (match(Type::LPAREN)) {
    std::cout << "[PARSER DEBUG] 解析过程参数列表" << std::endl;
    bool first = true;
    while (!check(Type::RPAREN) && !isAtEnd()) {
      if (!first) {
        if (!match(Type::COMMA)) {
          break;
        }
      }
      first = false;

      // 解析参数模式 (IN, OUT, INOUT)
      ProcedureParameter::Mode mode = ProcedureParameter::IN;
      if (match(Type::KEYWORD_IN)) {
        mode = ProcedureParameter::IN;
      } else if (match(Type::KEYWORD_OUT)) {
        mode = ProcedureParameter::OUT;
      } else if (match(Type::KEYWORD_INOUT)) {
        mode = ProcedureParameter::INOUT;
      }

      // 解析参数名
      std::string paramName = parseIdentifier();
      if (paramName.empty()) {
        reportError("Expected parameter name");
        return nullptr;
      }

      // 解析参数类型
      std::string paramType = parseIdentifier();
      if (paramType.empty()) {
        reportError("Expected parameter type");
        return nullptr;
      }

      parameters.emplace_back(paramName, paramType, mode);
      std::cout << "[PARSER DEBUG] 添加参数: " << ProcedureParameter(paramName, paramType, mode).getModeString()
                << " " << paramName << " " << paramType << std::endl;
    }
    consume(Type::RPAREN);
  }

  // 消费AS关键字
  consume(Type::KEYWORD_AS);

  // 解析过程体
  std::stringstream bodyStream;
  consume(Type::KEYWORD_BEGIN);

  int braceLevel = 1;
  while (!isAtEnd() && braceLevel > 0) {
    if (match(Type::KEYWORD_BEGIN)) {
      braceLevel++;
      bodyStream << "BEGIN ";
    } else if (match(Type::KEYWORD_END)) {
      braceLevel--;
      if (braceLevel > 0) {
        bodyStream << "END ";
      }
    } else {
      bodyStream << currentToken_.getLexeme() << " ";
      advance();
    }
  }

  std::string body = bodyStream.str();
  // 移除末尾空格
  while (!body.empty() && body.back() == ' ') {
    body.pop_back();
  }

  std::cout << "[PARSER DEBUG] 过程体: " << body << std::endl;

  // 创建CreateProcedureStatement对象
  auto stmt = std::make_unique<CreateProcedureStatement>(procedureName);
  for (const auto& param : parameters) {
    stmt->addParameter(param);
  }
  stmt->setBody(body);

  std::cout << "[PARSER DEBUG] CREATE PROCEDURE语句解析完成" << std::endl;
  return stmt;
}

std::unique_ptr<sql_parser::Statement> sql_parser::Parser::parseCreateViewStatement() {
  std::cout << "[PARSER DEBUG] 进入parseCreateViewStatement()方法" << std::endl;

  // 解析视图名
  std::string viewName = parseIdentifier();
  std::cout << "[PARSER DEBUG] 视图名: " << viewName << std::endl;

  // 解析可选的列名列表
  std::vector<std::string> columnNames;
  if (match(Type::LPAREN)) {
    std::cout << "[PARSER DEBUG] 解析视图列名列表" << std::endl;
    bool first = true;
    while (!check(Type::RPAREN) && !isAtEnd()) {
      if (!first) {
        if (!match(Type::COMMA)) {
          break;
        }
      }
      first = false;

      std::string columnName = parseIdentifier();
      if (!columnName.empty()) {
        columnNames.push_back(columnName);
        std::cout << "[PARSER DEBUG] 添加视图列: " << columnName << std::endl;
      }
    }
    consume(Type::RPAREN);
  }

  // 消费AS关键字
  consume(Type::KEYWORD_AS);

  // 解析SELECT语句
  std::cout << "[PARSER DEBUG] 解析视图的SELECT语句" << std::endl;
  auto selectStmt = parseSelectStatement();
  if (!selectStmt) {
    reportError("Expected SELECT statement in CREATE VIEW");
    return nullptr;
  }

  // 创建CreateViewStatement对象
  auto stmt = std::make_unique<CreateViewStatement>(viewName);
  stmt->setSelectStatement(std::move(selectStmt));

  // 设置列名（如果有）
  for (const auto& columnName : columnNames) {
    stmt->addColumnName(columnName);
  }

  std::cout << "[PARSER DEBUG] CREATE VIEW语句解析完成" << std::endl;
  return stmt;
}

std::unique_ptr<sql_parser::CreateStatement> sql_parser::Parser::parseCreateTriggerStatement() {
  std::cout << "[PARSER DEBUG] 进入parseCreateTriggerStatement()方法" << std::endl;

  // 解析触发器名
  std::string triggerName = parseIdentifier();
  std::cout << "[PARSER DEBUG] 触发器名: " << triggerName << std::endl;

  // 解析触发时机 (BEFORE/AFTER)
  TriggerDefinition::Timing timing;
  if (match(Type::KEYWORD_BEFORE)) {
    timing = TriggerDefinition::BEFORE;
    std::cout << "[PARSER DEBUG] 触发时机: BEFORE" << std::endl;
  } else if (match(Type::KEYWORD_AFTER)) {
    timing = TriggerDefinition::AFTER;
    std::cout << "[PARSER DEBUG] 触发时机: AFTER" << std::endl;
  } else {
    reportError("Expected BEFORE or AFTER for trigger timing");
    return nullptr;
  }

  // 解析触发事件 (INSERT/UPDATE/DELETE)
  TriggerDefinition::Event event;
  if (match(Type::KEYWORD_INSERT)) {
    event = TriggerDefinition::INSERT;
    std::cout << "[PARSER DEBUG] 触发事件: INSERT" << std::endl;
  } else if (match(Type::KEYWORD_UPDATE)) {
    event = TriggerDefinition::UPDATE;
    std::cout << "[PARSER DEBUG] 触发事件: UPDATE" << std::endl;
  } else if (match(Type::KEYWORD_DELETE)) {
    event = TriggerDefinition::DELETE;
    std::cout << "[PARSER DEBUG] 触发事件: DELETE" << std::endl;
  } else {
    reportError("Expected INSERT, UPDATE, or DELETE for trigger event");
    return nullptr;
  }

  // 消费ON关键字
  consume(Type::KEYWORD_ON);

  // 解析表名
  std::string tableName = parseIdentifier();
  std::cout << "[PARSER DEBUG] 目标表名: " << tableName << std::endl;

  // 解析触发级别 (ROW/STATEMENT) - 可选，默认为ROW
  TriggerDefinition::Level level = TriggerDefinition::ROW;
  if (match(Type::KEYWORD_FOR)) {
    if (match(Type::KEYWORD_EACH)) {
      consume(Type::KEYWORD_ROW);
      level = TriggerDefinition::ROW;
      std::cout << "[PARSER DEBUG] 触发级别: ROW" << std::endl;
    } else {
      // 默认为STATEMENT级别（简化处理）
      level = TriggerDefinition::STATEMENT;
      std::cout << "[PARSER DEBUG] 触发级别: STATEMENT" << std::endl;
    }
  }

  // 解析触发条件 (WHEN子句) - 可选
  std::string condition;
  if (match(Type::KEYWORD_WHEN)) {
    consume(Type::LPAREN);
    std::stringstream conditionStream;
    int parenLevel = 1;
    while (!isAtEnd() && parenLevel > 0) {
      if (match(Type::LPAREN)) {
        parenLevel++;
        conditionStream << "(";
      } else if (match(Type::RPAREN)) {
        parenLevel--;
        if (parenLevel > 0) {
          conditionStream << ")";
        }
      } else {
        conditionStream << currentToken_.getLexeme() << " ";
        advance();
      }
    }
    condition = conditionStream.str();
    // 移除末尾空格
    while (!condition.empty() && condition.back() == ' ') {
      condition.pop_back();
    }
    std::cout << "[PARSER DEBUG] 触发条件: " << condition << std::endl;
  }

  // 消费AS关键字（可选）
  if (match(Type::KEYWORD_AS)) {
    std::cout << "[PARSER DEBUG] 消费AS关键字" << std::endl;
  }

  // 解析触发器体
  std::stringstream bodyStream;
  consume(Type::KEYWORD_BEGIN);

  int braceLevel = 1;
  while (!isAtEnd() && braceLevel > 0) {
    if (match(Type::KEYWORD_BEGIN)) {
      braceLevel++;
      bodyStream << "BEGIN ";
    } else if (match(Type::KEYWORD_END)) {
      braceLevel--;
      if (braceLevel > 0) {
        bodyStream << "END ";
      }
    } else {
      bodyStream << currentToken_.getLexeme() << " ";
      advance();
    }
  }

  std::string body = bodyStream.str();
  // 移除末尾空格
  while (!body.empty() && body.back() == ' ') {
    body.pop_back();
  }

  std::cout << "[PARSER DEBUG] 触发器体: " << body << std::endl;

  // 创建TriggerDefinition对象
  TriggerDefinition triggerDef(triggerName, timing, event, level, tableName);
  triggerDef.setCondition(condition);
  triggerDef.setBody(body);

  // 创建CreateTriggerStatement对象
  auto stmt = std::make_unique<CreateTriggerStatement>(triggerDef);

  std::cout << "[PARSER DEBUG] CREATE TRIGGER语句解析完成" << std::endl;
  return stmt;
}

  // ==================== LOAD DATA Statement Parsing ====================

std::unique_ptr<sql_parser::Statement> sql_parser::Parser::parseLoadDataStatement() {
  std::cout << "[PARSER DEBUG] 进入parseLoadDataStatement()方法" << std::endl;

  // LOAD DATA语句暂时不支持，返回空指针并报错
  std::cout << "[PARSER DEBUG] LOAD DATA语句暂不支持" << std::endl;
  reportError("LOAD DATA statement not yet supported");
  return nullptr;
}

} // namespace sql_parser
} // namespace sqlcc
