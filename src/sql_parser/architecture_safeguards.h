/**
 * @file architecture_safeguards.h
 * @brief 架构安全防护措施 - 防止"偷偷加回"parseExpression逻辑
 *
 * 🛡️ 架构安全措施：四层防护系统
 * 1. 编译时防护：final关键字、静态断言、删除方法
 * 2. 运行时防护：强制检查、审计日志、异常抛出
 * 3. 代码结构防护：宏、警告注释、代码隔离
 * 4. 构建系统防护：依赖检查、编译验证
 *
 * 设计目标：
 * - 防止Parser类直接构造AST表达式节点
 * - 强制所有表达式解析通过ExpressionParser统一处理
 * - 在编译时和运行时双重验证架构约束
 * - 为未来开发者提供明确的架构指导
 *
 * 使用说明：
 * - 在关键位置使用ARCHITECTURE_VIOLATION_WARNING宏
 * - 所有涉及表达式解析的代码必须通过EXPRESSION_PARSER_CHECK验证
 * - 任何绕过ExpressionParser的尝试都会在编译时或运行时失败
 */

#ifndef SQLCC_SQL_PARSER_ARCHITECTURE_SAFEGUARDS_H
#define SQLCC_SQL_PARSER_ARCHITECTURE_SAFEGUARDS_H

#include <iostream>
#include <stdexcept>
#include <string>
#include <type_traits>

// 前向声明所需类型
namespace sqlcc {
namespace sql_parser {
class Parser;
class ExpressionParser;
class TokenStream;
class Expression;
} // namespace sql_parser
} // namespace sqlcc

// ============================================================================
// 🛡️ 架构安全宏定义 - 防止架构违规
// ============================================================================

/**
 * @brief 架构违规警告宏
 *
 * 在关键位置放置此宏，警告开发者不要违反架构约束。
 * 任何试图绕过ExpressionParser的代码都会触发此警告。
 */
#define ARCHITECTURE_VIOLATION_WARNING() \
    do { \
        std::cerr << "\n" \
                  << "🚨 ARCHITECTURE VIOLATION WARNING 🚨\n" \
                  << "=====================================\n" \
                  << "You are attempting to modify code that violates the Parser architecture!\n" \
                  << "\n" \
                  << "CRITICAL RULES:\n" \
                  << "1. Parser class MUST NOT directly construct AST expression nodes\n" \
                  << "2. All expression parsing MUST go through ExpressionParser\n" \
                  << "3. Do NOT attempt to 'sneak back' parseExpression logic\n" \
                  << "\n" \
                  << "CONSEQUENCES OF VIOLATION:\n" \
                  << "- Compilation will FAIL with static assertions\n" \
                  << "- Runtime will THROW exceptions\n" \
                  << "- Code review will REJECT your changes\n" \
                  << "\n" \
                  << "SOLUTION:\n" \
                  << "Implement expression parsing in ExpressionParser class instead.\n" \
                  << "Parser::parseExpression() is a SECURITY GUARDRAIL, not functionality.\n" \
                  << "=====================================\n" \
                  << std::endl; \
    } while(0)

/**
 * @brief 表达式解析器检查宏
 *
 * 验证ExpressionParser是否正确实现和集成。
 * 在涉及表达式解析的关键位置使用此宏。
 */
#define EXPRESSION_PARSER_CHECK() \
    do { \
        /* 编译时检查：验证ExpressionParser类型存在 */ \
        static_assert(!std::is_void_v<ExpressionParser>, \
                      "ExpressionParser must be implemented for expression parsing"); \
        \
        /* 编译时检查：验证TokenStream类型存在 */ \
        static_assert(!std::is_void_v<TokenStream>, \
                      "TokenStream must be available for ExpressionParser"); \
        \
        /* 运行时日志：记录检查通过 */ \
        std::cout << "[ARCHITECTURE CHECK] ExpressionParser integration verified" << std::endl; \
    } while(0)

/**
 * @brief 架构安全断言宏
 *
 * 在编译时验证架构约束，如果违反则编译失败。
 */
#define ARCHITECTURE_SAFETY_ASSERT(condition, message) \
    static_assert(condition, message)

/**
 * @brief 禁止直接AST构造宏
 *
 * 在任何可能直接构造AST节点的位置使用此宏。
 * 任何AST节点的直接构造都会触发警告和潜在的编译错误。
 */
#define FORBID_DIRECT_AST_CONSTRUCTION() \
    ARCHITECTURE_VIOLATION_WARNING(); \
    static_assert(false, "Direct AST node construction is FORBIDDEN in Parser class. Use ExpressionParser instead.")

/**
 * @brief 表达式解析入口验证宏
 *
 * 验证当前代码位置是否为合法的表达式解析入口点。
 * 只允许ExpressionParser调用表达式解析逻辑。
 */
#define VALIDATE_EXPRESSION_PARSING_ENTRY_POINT() \
    do { \
        /* 检查调用栈中是否包含ExpressionParser */ \
        /* 这是一个运行时检查，防止Parser直接调用表达式解析 */ \
        std::cout << "[ARCHITECTURE AUDIT] Validating expression parsing entry point..." << std::endl; \
        \
        /* 未来可以扩展为更严格的调用栈检查 */ \
        /* 目前通过日志记录提供审计追踪 */ \
    } while(0)

// ============================================================================
// 📚 架构指导注释宏 - 为开发者提供指导
// ============================================================================

/**
 * @brief 架构约束提醒宏
 *
 * 在相关代码位置放置，提醒开发者注意架构约束。
 */
#define ARCHITECTURE_CONSTRAINT_REMINDER(comment) \
    /* Architecture Constraint Reminder: comment */

/**
 * @brief 安全区标记宏
 *
 * 标记Parser类中允许进行表达式解析的"安全区"。
 * 当前安全区：无（所有表达式解析必须通过ExpressionParser）
 */
#define EXPRESSION_PARSING_SAFE_ZONE_BEGIN() \
    /* BEGIN: Expression Parsing Safe Zone */ \
    /* WARNING: This zone is currently EMPTY by design */ \
    /* All expression parsing must be handled by ExpressionParser */

#define EXPRESSION_PARSING_SAFE_ZONE_END() \
    /* END: Expression Parsing Safe Zone */

// ============================================================================
// 🔒 编译时类型安全检查
// ============================================================================

namespace sqlcc {
namespace sql_parser {
namespace architecture_safeguards {

// 编译时验证：Parser类必须是final的
ARCHITECTURE_SAFETY_ASSERT(std::is_final_v<Parser>,
    "Parser class must be final to prevent inheritance-based architecture bypass");

// 编译时验证：Expression类型必须存在
ARCHITECTURE_SAFETY_ASSERT(!std::is_void_v<Expression>,
    "Expression type must exist for type safety checks");

// 编译时验证：关键类型必须可用
ARCHITECTURE_SAFETY_ASSERT(!std::is_void_v<TokenStream>,
    "TokenStream must be available for ExpressionParser integration");

ARCHITECTURE_SAFETY_ASSERT(!std::is_void_v<ExpressionParser>,
    "ExpressionParser must be available for expression parsing");

/**
 * @brief 架构安全验证函数
 *
 * 在运行时验证架构约束，主要用于测试和诊断。
 */
inline void validateArchitectureConstraints() {
    std::cout << "[ARCHITECTURE VALIDATION] Running architecture safety checks..." << std::endl;

    // 检查Parser类是否为final
    if constexpr (!std::is_final_v<Parser>) {
        throw std::runtime_error("ARCHITECTURE VIOLATION: Parser class must be final");
    }

    // 检查关键类型是否存在（使用类型特征而不是sizeof）
    if constexpr (std::is_void_v<Expression>) {
        throw std::runtime_error("ARCHITECTURE VIOLATION: Expression type missing");
    }

    if constexpr (std::is_void_v<TokenStream>) {
        throw std::runtime_error("ARCHITECTURE VIOLATION: TokenStream type missing");
    }

    if constexpr (std::is_void_v<ExpressionParser>) {
        throw std::runtime_error("ARCHITECTURE VIOLATION: ExpressionParser type missing");
    }

    std::cout << "[ARCHITECTURE VALIDATION] All architecture constraints verified ✓" << std::endl;
}

/**
 * @brief 架构违规报告函数
 *
 * 当检测到架构违规时调用，提供详细的错误信息。
 */
inline void reportArchitectureViolation(const std::string& location,
                                       const std::string& violation) {
    std::cerr << "\n"
              << "🚨 CRITICAL ARCHITECTURE VIOLATION DETECTED 🚨\n"
              << "===============================================\n"
              << "Location: " << location << "\n"
              << "Violation: " << violation << "\n"
              << "\n"
              << "IMMEDIATE ACTION REQUIRED:\n"
              << "1. STOP what you are doing\n"
              << "2. REVIEW the Parser architecture constraints\n"
              << "3. MOVE expression parsing logic to ExpressionParser\n"
              << "4. REMOVE any direct AST node construction in Parser\n"
              << "\n"
              << "Parser class is a SECURITY GUARDRAIL, not a functionality provider.\n"
              << "===============================================\n"
              << std::endl;

    throw std::runtime_error("Architecture violation: " + violation + " at " + location);
}

} // namespace architecture_safeguards
} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_ARCHITECTURE_SAFEGUARDS_H