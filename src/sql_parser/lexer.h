/**
 * WHY: 为什么需要词法分析器头文件？
 *
 * 词法分析器是SQL解析器的第一阶段，负责将SQL字符串分解为有意义的词法单元。
 * 词法分析器头文件定义了词法分析器的接口和状态管理，提供了：
 * - Lexer类的公共接口定义
 * - 词法分析状态枚举
 * - 词法分析器配置和使用方法
 * - 错误处理和调试支持
 *
 * 词法分析器头文件的重要性：
 * 1. 接口定义：为词法分析器提供清晰的外部接口
 * 2. 状态管理：定义词法分析过程中的各种状态
 * 3. 类型安全：确保词法分析器的正确使用
 * 4. 错误处理：提供词法错误的统一处理机制
 * 5. 性能优化：支持词法分析器的性能监控和调优
 *
 * 词法分析器设计的核心价值：
 * - 模块化：词法分析逻辑与语法分析逻辑完全分离
 * - 可扩展：支持新的Token类型和词法规则
 * - 高效性：提供快速的词法分析算法
 * - 可靠性：确保词法分析的准确性和稳定性
 * - 可维护：清晰的接口和实现分离
 *
 * WHAT: 这定义了什么功能？
 *
 * 词法分析器头文件定义了完整的SQL词法分析功能：
 * - Lexer类：词法分析器的主类，管理分析过程
 * - LexerState枚举：词法分析器的状态机状态
 * - Token生成：将输入字符串转换为Token序列
 * - 错误处理：词法错误的检测和报告
 * - 位置跟踪：Token在源代码中的位置信息
 * - 字符处理：支持多字节字符和Unicode
 * - 注释处理：跳过SQL注释
 * - 字符串处理：处理带引号的字符串字面量
 * - 数字处理：识别整数和浮点数字面量
 *
 * 核心组件：
 * - Lexer：词法分析器主类
 * - LexerState：词法分析状态枚举
 * - Token：词法单元封装类
 * - ErrorHandler：词法错误处理机制
 * - CharacterClassifier：字符分类器
 * - StateTransitionTable：状态转换表
 *
 * HOW: 如何实现的？
 *
 * 技术实现要点：
 * 1. 有限自动机：使用DFA实现词法识别算法
 * 2. 状态机模式：状态驱动的词法分析流程
 * 3. 工厂模式：根据Token类型创建相应Token对象
 * 4. 迭代器模式：提供Token流的顺序访问
 * 5. 策略模式：可插拔的字符处理策略
 * 6. 单例模式：共享SQL关键字集合
 *
 * 架构设计：
 * - 接口分离：头文件定义接口，实现文件提供实现
 * - 依赖注入：通过构造函数注入输入字符串
 * - 状态管理：内部状态机管理词法分析状态
 * - 错误传播：通过异常向上传播词法错误
 * - 资源管理：自动管理词法分析资源
 *
 * 性能优化策略：
 * - 预编译状态表：避免运行时状态表构造
 * - 缓存机制：缓存常用Token和关键字映射
 * - 批量处理：减少函数调用开销
 * - 内存池：复用Token对象的内存分配
 * - 延迟初始化：按需初始化状态转换表
 *
 * 错误处理机制：
 * - 词法错误：检测无效字符和词法错误
 * - 位置信息：提供准确的错误位置信息
 * - 错误恢复：提供基本的错误恢复机制
 * - 异常规范：定义可能抛出的异常类型
 * - 日志记录：详细记录词法分析过程和错误
 *
 * 扩展性设计：
 * - 新Token类型：通过枚举扩展支持新Token
 * - 自定义词法规则：支持自定义词法分析规则
 * - 多语言支持：支持不同编程语言的词法分析
 * - 配置化：词法分析器行为的配置化管理
 * - 插件架构：支持第三方词法分析插件
 *
 * 调试和诊断：
 * - 状态跟踪：详细记录状态机转换过程
 * - Token输出：提供Token序列的可视化输出
 * - 性能分析：分析词法分析的性能瓶颈
 * - 测试支持：提供词法分析的单元测试接口
 * - 诊断工具：支持词法分析过程的诊断和调试
 */

#ifndef SQLCC_SQL_PARSER_LEXER_H
#define SQLCC_SQL_PARSER_LEXER_H

#include <string>
#include <unordered_map>
#include <unordered_set>

namespace sqlcc {
namespace sql_parser {

class Token;

/**
 * @brief 词法分析器状态枚举
 *
 * 定义词法分析器在分析过程中的各种状态。
 * 这些状态构成了有限自动机的基础。
 */
enum class LexerState {
  START,                    // 开始状态
  IDENTIFIER,               // 标识符状态
  NUMBER,                   // 数字状态
  NUMBER_DECIMAL,           // 十进制数字状态
  NUMBER_EXPONENT,          // 指数数字状态
  STRING_SINGLE,            // 单引号字符串状态
  STRING_DOUBLE,            // 双引号字符串状态
  STRING_ESCAPE,            // 字符串转义状态
  OPERATOR,                 // 操作符状态
  PUNCTUATION,              // 标点符号状态
  COMMENT_LINE,             // 行注释状态
  COMMENT_BLOCK,            // 块注释状态
  COMMENT_BLOCK_STAR,       // 块注释星号状态
  END_OF_INPUT              // 输入结束状态
};

/**
 * @brief SQL词法分析器类
 *
 * 负责将SQL字符串分解为词法单元(Token)的词法分析器。
 * 使用有限自动机实现词法识别算法。
 */
class Lexer {
public:
  /**
   * @brief 构造函数
   *
   * @param input 要分析的SQL字符串
   */
  explicit Lexer(const std::string &input);

  /**
   * @brief 获取下一个Token
   *
   * 从当前位置开始分析，生成并返回下一个Token。
   * 如果到达输入末尾，返回END_OF_INPUT类型的Token。
   *
   * @return 下一个Token
   * @throws std::runtime_error 当遇到词法错误时
   */
  Token nextToken();

  /**
   * @brief 检查是否到达输入末尾
   *
   * @return 如果到达输入末尾返回true，否则返回false
   */
  bool isAtEnd() const;

  /**
   * @brief 获取当前行号
   *
   * @return 当前行号（从1开始）
   */
  size_t getCurrentLine() const { return line_; }

  /**
   * @brief 获取当前列号
   *
   * @return 当前列号（从1开始）
   */
  size_t getCurrentColumn() const { return column_; }

private:
  /**
   * @brief 前进到下一个字符
   *
   * @return 当前字符，然后移动到下一个位置
   */
  char advance();

  /**
   * @brief 查看当前字符（不前进）
   *
   * @return 当前字符，如果到达末尾返回'\0'
   */
  char peek() const;

  /**
   * @brief 查看下一个字符（不前进）
   *
   * @return 下一个字符，如果到达末尾返回'\0'
   */
  char peekNext() const;

  /**
   * @brief 设置状态转换表
   *
   * 初始化有限自动机的状态转换表。
   */
  void setupTransitionTable();

  /**
   * @brief 创建Token对象
   *
   * 根据当前状态和词素创建相应的Token对象。
   *
   * @param state 当前状态
   * @param lexeme 词素字符串
   * @param line 行号
   * @param column 列号
   * @return 创建的Token对象
   */
  Token createToken(LexerState state, const std::string &lexeme, int line, int column);

  /**
   * @brief 创建标识符Token
   *
   * 处理标识符和关键字的创建。
   *
   * @param lexeme 词素字符串
   * @param line 行号
   * @param column 列号
   * @return 创建的Token对象
   */
  Token createIdentifierToken(const std::string &lexeme, int line, int column);

  /**
   * @brief 创建关键字Token
   *
   * 将关键字字符串映射到相应的Token类型。
   *
   * @param lexeme 关键字字符串
   * @return 创建的Token对象
   */
  Token createKeywordToken(const std::string &lexeme);

  /**
   * @brief 创建数字Token
   *
   * 区分整数和浮点数创建相应的Token。
   *
   * @param lexeme 数字字符串
   * @param line 行号
   * @param column 列号
   * @return 创建的Token对象
   */
  Token createNumberToken(const std::string &lexeme, int line, int column);

  /**
   * @brief 创建字符串Token
   *
   * @param lexeme 字符串字面量
   * @param line 行号
   * @param column 列号
   * @return 创建的Token对象
   */
  Token createStringToken(const std::string &lexeme, int line, int column);

  /**
   * @brief 创建操作符Token
   *
   * 处理各种操作符的创建，包括多字符操作符。
   *
   * @param lexeme 操作符字符串
   * @param line 行号
   * @param column 列号
   * @return 创建的Token对象
   */
  Token createOperatorToken(const std::string &lexeme, int line, int column);

  /**
   * @brief 创建标点符号Token
   *
   * @param lexeme 标点符号字符串
   * @param line 行号
   * @param column 列号
   * @return 创建的Token对象
   */
  Token createPunctuationToken(const std::string &lexeme, int line, int column);

  /**
   * @brief 处理行注释
   *
   * 跳过行注释内容直到行末。
   */
  void handleLineComment();

  /**
   * @brief 处理块注释
   *
   * 跳过块注释内容直到注释结束。
   */
  void handleBlockComment();

  /**
   * @brief 报告词法错误
   *
   * @param message 错误消息
   * @throws std::runtime_error 总是抛出异常
   */
  void reportError(const std::string &message);

  std::string input_;                           // 输入字符串
  size_t position_;                             // 当前位置
  size_t line_;                                 // 当前行号
  size_t column_;                               // 当前列号
  LexerState current_state_;                    // 当前状态
  std::unordered_map<LexerState, std::unordered_map<char, LexerState>> transitions_; // 状态转换表
};

} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_LEXER_H
