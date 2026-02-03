/**
 * @file parser_dml.h
 * @brief SQLCC DML解析器 - 数据操作语言解析引擎
 *
 * ParserDML 负责解析SQL中最常用的数据操作语言（Data Manipulation Language），
 * 包括数据的查询（SELECT）、插入（INSERT）、更新（UPDATE）和删除（DELETE）。
 * 本模块采用模块化设计，将DML解析逻辑从主解析器中分离，降低系统的复杂度。
 *
 * 📚 配套教材参考：
 * - [第4章：SQL数据操作](../../textbook/《数据库系统原理与开发实践》.md#第四章sql数据操作)
 * - [4.1 数据查询(SELECT)](../../textbook/《数据库系统原理与开发实践》.md#41-数据查询select)
 * - [4.2 数据插入(INSERT)](../../textbook/《数据库系统原理与开发实践》.md#42-数据插入insert)
 * - [4.3 数据更新与删除](../../textbook/《数据库系统原理与开发实践》.md#43-数据更新与删除)
 *
 * WHY层 - 设计意图：
 *   DML语句是数据库交互中最频繁使用的部分，其语法结构复杂且变化多端（特别是SELECT语句）。
 *   将DML解析逻辑独立封装，不仅符合单一职责原则，还有利于针对性地进行性能优化
 *   和功能扩展，同时降低了主解析器的代码体积和维护成本。
 *
 * WHAT层 - 功能说明：
 *   - 查询解析：支持简单查询、连接查询、子查询、聚合查询等
 *   - 变更解析：支持INSERT/UPDATE/DELETE语句的完整语法
 *   - 子句解析：统一处理WHERE、JOIN、ORDER BY等通用子句
 *   - 集合操作：支持UNION、INTERSECT、EXCEPT等集合运算
 *
 * HOW层 - 实现机制：
 *   - 递归下降：采用标准的递归下降算法分析语法结构
 *   - PIMPL模式：使用"指针指向实现"模式隐藏复杂的内部依赖（如SelectParser）
 *   - 复用机制：复用ExpressionParser处理所有表达式逻辑
 *   - 异常安全：解析过程中的异常捕捉与资源自动释放
 *
 * @author SQLCC技术委员会
 * @version 1.1.0
 * @date 2026-02-02
 */

#ifndef SQLCC_SQL_PARSER_PARSERS_DML_PARSER_DML_H
#define SQLCC_SQL_PARSER_PARSERS_DML_PARSER_DML_H

#include "../parser_core.h"
#include "../../ast/ast_node.h"
#include "../../ast/dml/ast_dml_nodes.h"
#include "../../ast/expression.h"
#include "../../set_operation.h"
#include "../../select_parser_fwd.h"

// 前向声明
class SelectParser;

namespace sqlcc {
namespace sql_parser {

/**
 * @brief DML语句解析器类
 *
 * WHY层 - 设计意图：
 *   作为一个专门的解析器组件，ParserDML封装了所有与数据操作相关的语法分析逻辑。
 *   它通过PIMPL模式隐藏了复杂的SelectParser依赖，减少了头文件的编译依赖。
 *
 * WHAT层 - 核心职责：
 *   - 识别并解析各种DML语句关键字
 *   - 构建对应的AST节点（如SelectStatement, InsertStatement）
 *   - 处理语句中的子句依赖关系
 *
 * HOW层 - 架构协作：
 *   ParserDML继承自ParserCore，共享Token流和基础解析工具。
 *   对于复杂的SELECT语句，它会内部委托给专门的SelectParser（在Impl中实现）。
 */
class ParserDML : public ParserCore {
public:
    /**
     * @brief 构造函数
     * @param tokens 词法分析器生成的Token流
     */
    ParserDML(TokenStream& tokens);
    
    /**
     * @brief 析构函数
     * 必须在cpp文件中定义，以支持PIMPL模式下的unique_ptr
     */
    ~ParserDML();
    
    /**
     * @brief 解析SELECT语句
     *
     * WHY: SELECT是SQL中最复杂的语句，包含投影、筛选、连接、分组、排序等多个部分。
     * WHAT: 解析完整的SELECT查询，包括可能的子查询和集合操作。
     * HOW: 识别SELECT关键字后，委托给内部的SelectParser处理详细逻辑。
     *
     * @return SelectStatement的智能指针
     */
    std::unique_ptr<SelectStatement> parseSelectStatement();

    /**
     * @brief 解析INSERT语句
     *
     * WHY: 数据录入是数据库的基础功能。
     * WHAT: 解析 "INSERT INTO table (cols) VALUES ..." 或 "INSERT INTO ... SELECT ..."。
     * HOW: 
     *   1. 解析目标表名
     *   2. 解析可选的列名列表
     *   3. 分支判断：VALUES子句 vs SELECT子句
     *
     * @return InsertStatement的AST节点指针
     */
    std::unique_ptr<ASTNode> parseInsertStatement();

    /**
     * @brief 解析UPDATE语句
     *
     * WHY: 支持数据的修改操作。
     * WHAT: 解析 "UPDATE table SET col=val WHERE ..."。
     * HOW: 解析SET子句中的赋值列表，并可选地解析WHERE子句。
     *
     * @return UpdateStatement的AST节点指针
     */
    std::unique_ptr<ASTNode> parseUpdateStatement();

    /**
     * @brief 解析DELETE语句
     *
     * WHY: 支持数据的删除操作。
     * WHAT: 解析 "DELETE FROM table WHERE ..."。
     * HOW: 解析目标表和可选的WHERE子句。
     *
     * @return DeleteStatement的AST节点指针
     */
    std::unique_ptr<ASTNode> parseDeleteStatement();

    /**
     * @brief 解析复合SELECT语句（带集合操作）
     *
     * WHY: 支持 UNION, INTERSECT, EXCEPT 等高级查询操作。
     * WHAT: 解析通过集合运算符连接的多个SELECT语句。
     *
     * @return 可能是SelectStatement或SetOperation的节点
     */
    std::unique_ptr<ASTNode> parseCompositeSelectStatement();
    
    // DML辅助解析方法
    std::unique_ptr<WhereClause> parseWhereClause();
    std::unique_ptr<JoinClause> parseJoinClause();
    std::unique_ptr<SetOperation> parseSetOperation();
    SetOperationType parseSetOperationType();
    
    // INSERT语句辅助方法
    std::vector<std::string> parseInsertColumnNames();
    std::vector<std::vector<std::unique_ptr<Expression>>> parseInsertValues();
    std::unique_ptr<SelectStatement> parseInsertSelect();
    
    // UPDATE语句辅助方法
    std::vector<std::pair<std::string, std::unique_ptr<Expression>>> parseUpdateAssignments();
    
    // DELETE语句辅助方法
    std::vector<std::string> parseDeleteTableNames();

private:
    class Impl;  // 使用PIMPL模式隐藏实现细节
    std::unique_ptr<Impl> impl_;  ///< 实现类指针
};

} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_PARSERS_DML_PARSER_DML_H