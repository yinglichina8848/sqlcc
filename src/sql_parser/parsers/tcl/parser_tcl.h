/**
 * @file parser_tcl.h
 * @brief SQLCC TCL解析器 - 事务控制语言解析引擎
 *
 * ParserTCL 负责解析SQL中的事务控制语言（Transaction Control Language），
 * 用于管理数据库事务的边界、状态和持久性。
 *
 * 📚 配套教材参考：
 * - [第8章：OLTP事务处理](../../textbook/《数据库系统原理与开发实践》.md#第八章oltp事务处理)
 * - [8.1 事务的定义与ACID属性](../../textbook/《数据库系统原理与开发实践》.md#81-事务的定义与acid属性)
 * - [8.2 事务控制语句](../../textbook/《数据库系统原理与开发实践》.md#82-事务控制语句)
 *
 * WHY层 - 设计意图：
 *   事务是关系数据库的核心抽象，用于保证数据的一致性和完整性。
 *   通过独立的TCL解析器，可以精确处理事务的开始、提交、回滚以及保存点管理，
 *   支持复杂的隔离级别设置和嵌套事务控制。
 *
 * WHAT层 - 功能说明：
 *   - 事务开始：BEGIN [TRANSACTION] [READ ONLY | READ WRITE]
 *   - 事务提交：COMMIT [WORK]
 *   - 事务回滚：ROLLBACK [WORK] [TO SAVEPOINT name]
 *   - 保存点：SAVEPOINT name (通常包含在ROLLBACK逻辑中或独立处理)
 *   - 隔离级别：SET TRANSACTION ISOLATION LEVEL ...
 *
 * HOW层 - 实现机制：
 *   - 状态机辅助：解析过程需要识别当前上下文是否允许特定的事务语句
 *   - 选项解析：处理READ ONLY, ISOLATION LEVEL等事务属性
 *
 * @author SQLCC技术委员会
 * @version 1.1.0
 * @date 2026-02-02
 */

#ifndef SQLCC_SQL_PARSER_PARSERS_TCL_PARSER_TCL_H
#define SQLCC_SQL_PARSER_PARSERS_TCL_PARSER_TCL_H

#include "../parser_core.h"
#include "../../ast/ast_node.h"

namespace sqlcc {
namespace sql_parser {

/**
 * @brief TCL语句解析器类
 *
 * WHY层 - 设计意图：
 *   集中处理所有与事务生命周期控制相关的语法分析。
 *
 * WHAT层 - 核心职责：
 *   - 解析事务边界指令（BEGIN/COMMIT/ROLLBACK）
 *   - 解析事务属性配置（隔离级别、读写模式）
 */
class ParserTCL : public ParserCore {
public:
    /**
     * @brief 构造函数
     * @param tokens 词法分析器生成的Token流
     */
    ParserTCL(TokenStream& tokens);
    
    /**
     * @brief 解析COMMIT语句
     *
     * WHY: 标志事务的成功结束，将所有修改永久化。
     * WHAT: 解析 "COMMIT [WORK]"。
     *
     * @return CommitStatement的AST节点指针
     */
    std::unique_ptr<ASTNode> parseCommitStatement();

    /**
     * @brief 解析ROLLBACK语句
     *
     * WHY: 标志事务的失败或取消，撤销未提交的修改。
     * WHAT: 解析 "ROLLBACK [WORK] [TO SAVEPOINT name]"。
     * HOW: 区分是回滚整个事务还是回滚到特定保存点。
     *
     * @return RollbackStatement的AST节点指针
     */
    std::unique_ptr<ASTNode> parseRollbackStatement();

    /**
     * @brief 解析BEGIN语句
     *
     * WHY: 显式启动一个新的事务上下文。
     * WHAT: 解析 "BEGIN [TRANSACTION] [mode]"。
     *
     * @return BeginStatement的AST节点指针
     */
    std::unique_ptr<ASTNode> parseBeginStatement();
    
    // ==========================================
    // 事务选项解析辅助方法
    // ==========================================

    /**
     * @brief 解析事务模式
     *
     * @return 事务模式字符串（如 READ ONLY, READ WRITE）
     */
    std::string parseTransactionMode();

    /**
     * @brief 解析隔离级别
     *
     * @return 隔离级别字符串（如 SERIALIZABLE, REPEATABLE READ）
     */
    std::string parseIsolationLevel();
};

} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_PARSERS_TCL_PARSER_TCL_H