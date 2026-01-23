#ifndef SQLCC_SQL_PARSER_TOKEN_STREAM_H
#define SQLCC_SQL_PARSER_TOKEN_STREAM_H

#include "token.h"
#include "lexer.h"
#include <string>

namespace sqlcc {
namespace sql_parser {

/**
 * @brief Token流管理类 - 负责词法分析器的token流管理
 *
 * WHY层 - 设计意图：
 *   TokenStream类封装了token流的管理逻辑，从Parser中分离出来，
 *   实现了单一职责原则。Token流管理是递归下降解析器的基础设施，
 *   通过精心设计的缓存机制和前瞻功能，提高了解析效率。
 *
 * WHAT层 - 功能说明：
 *   1. 管理词法分析器产生的token流
 *   2. 提供前瞻(lookahead)功能，支持LL(k)文法
 *   3. 实现token前进机制
 *   4. 提供类型检查和强制消费功能
 *
 * HOW层 - 实现细节：
 *   **组合模式**: 组合Lexer对象，协同工作
 *   **缓存策略**: 使用hasLookahead_和lookaheadToken_缓存前瞻token
 *   **状态管理**: 维护currentToken_作为当前解析位置
 *   **错误处理**: 提供详细的token相关错误信息
 *
 * 编译原理中的对应概念：
 *   - **输入指针**: TokenStream管理语法分析器的输入指针
 *   - **前瞻符号**: 实现LL(1)文法的1个前瞻符号
 *   - **词法接口**: 作为语法分析器与词法分析器的桥梁
 *   - **状态机转换**: 实现token流的状态转换
 *
 * 前瞻机制的工作原理：
 *   ```
 *   假设输入流: SELECT name FROM users;
 *   当前状态: currentToken_ = "SELECT"
 *   前瞻调用: peek() -> lookaheadToken_ = "name", hasLookahead_ = true
 *   前进调用: advance() -> currentToken_ = "name", hasLookahead_ = false
 *   ```
 *
 * 性能优化考虑：
 *   - **缓存复用**: 前瞻token被缓存避免重复词法分析
 *   - **按需获取**: 只有在需要时才调用lexer_.nextToken()
 *   - **内存效率**: 只维护必要的token状态信息
 *   - **调用开销**: 最小化lexer_.nextToken()的调用频率
 *
 * @note 该类不进行任何语法验证，只负责token流管理
 * @note 前瞻机制是递归下降法避免回溯的关键技术
 * @note 与Parser配合使用实现完整的语法分析功能
 *
 * @see Lexer 词法分析器类
 * @see Token token数据结构
 * @see Parser 语法分析器类
 */
class TokenStream {
public:
    /**
     * @brief 构造函数 - 初始化token流管理器
     * @param lexer 词法分析器引用，用于获取token
     */
    explicit TokenStream(Lexer& lexer);

    /**
     * @brief 获取当前token
     * @return 当前正在处理的token
     */
    const Token& current() const;

    /**
     * @brief 前瞻下一个token（不消费）
     * @return 下一个token，如果没有则抛出异常
     */
    const Token& peek();

    /**
     * @brief 前进到下一个token
     * 使用缓存的前瞻token或从词法分析器获取新token
     */
    void advance();

    /**
     * @brief 检查当前token是否为指定类型
     * @param type 要检查的token类型
     * @return 如果匹配返回true，否则返回false
     */
    bool check(Type type) const;

    /**
     * @brief 强制消费指定类型的token
     * @param type 期望的token类型
     * @param message 错误消息（当类型不匹配时使用）
     * @throws std::runtime_error 当token类型不匹配时抛出
     */
    void expect(Type type, const std::string& message = "");

    /**
     * @brief 检查是否到达输入末尾
     * @return 如果当前token是END_OF_INPUT则返回true
     */
    bool isAtEnd() const;

private:
    Lexer& lexer_;              ///< 词法分析器引用
    Token currentToken_;        ///< 当前正在处理的token
    Token lookaheadToken_;      ///< 缓存的前瞻token
    bool hasLookahead_;         ///< 是否有缓存的前瞻token
};

} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_TOKEN_STREAM_H
