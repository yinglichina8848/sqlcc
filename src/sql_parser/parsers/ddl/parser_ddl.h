/**
 * @file parser_ddl.h
 * @brief SQLCC DDL解析器 - 数据定义语言解析引擎
 *
 * ParserDDL 负责解析SQL中的数据定义语言（Data Definition Language），
 * 用于定义和管理数据库对象，如数据库、表、索引、视图、用户、存储过程和触发器。
 * 它是数据库模式（Schema）演进和管理的入口。
 *
 * 📚 配套教材参考：
 * - [第3章：SQL数据定义](../../textbook/《数据库系统原理与开发实践》.md#第三章sql数据定义)
 * - [3.1 模式定义与管理](../../textbook/《数据库系统原理与开发实践》.md#31-模式定义与管理)
 * - [3.2 完整性约束定义](../../textbook/《数据库系统原理与开发实践》.md#32-完整性约束定义)
 * - [3.3 索引与视图管理](../../textbook/《数据库系统原理与开发实践》.md#33-索引与视图管理)
 *
 * WHY层 - 设计意图：
 *   DDL语句负责定义数据的结构（Schema），其正确性直接关系到数据存储的物理布局
 *   和逻辑约束。将DDL解析独立，有助于清晰地分离"结构定义"与"数据操作"（DML）
 *   的关注点，并便于支持复杂的约束定义和对象管理语法。
 *
 * WHAT层 - 功能说明：
 *   - 对象创建：CREATE DATABASE/TABLE/INDEX/VIEW/USER/PROCEDURE/TRIGGER
 *   - 对象删除：DROP DATABASE/TABLE/INDEX/VIEW/USER/PROCEDURE/TRIGGER
 *   - 对象修改：ALTER TABLE/DATABASE
 *   - 约束解析：主键、外键、唯一性、Check、非空等约束定义
 *
 * HOW层 - 实现机制：
 *   - 分发机制：根据CREATE/DROP/ALTER后的关键字（如TABLE, VIEW）分发到具体解析函数
 *   - 递归结构：表定义包含列定义和约束定义，采用递归方式解析
 *   - 事务性DDL：解析过程需考虑DDL操作的原子性准备（虽解析本身不涉及执行）
 *
 * @author SQLCC技术委员会
 * @version 1.1.0
 * @date 2026-02-02
 */

#ifndef SQLCC_SQL_PARSER_PARSERS_DDL_PARSER_DDL_H
#define SQLCC_SQL_PARSER_PARSERS_DDL_PARSER_DDL_H

#include "../parser_core.h"
#include "../../ast/ast_node.h"
#include "../../ast/ddl/ast_ddl_nodes.h"
#include <memory>
#include <vector>

namespace sqlcc {
namespace sql_parser {

/**
 * @brief DDL语句解析器类
 *
 * WHY层 - 设计意图：
 *   集中管理所有数据库对象的定义和变更语法。
 *   DDL语法通常比DML更具声明性（Declarative），涉及大量的属性配置和约束描述，
 *   需要专门的解析逻辑来处理这些结构化信息。
 *
 * WHAT层 - 核心职责：
 *   - 路由CREATE/DROP/ALTER语句到特定对象的解析器
 *   - 细粒度解析列定义（类型、长度、属性）
 *   - 细粒度解析表级和列级约束
 */
class ParserDDL : public ParserCore {
public:
    /**
     * @brief 构造函数
     * @param tokens 词法分析器生成的Token流
     */
    ParserDDL(TokenStream& tokens);
    
    // ==========================================
    // CREATE 语句族解析
    // ==========================================

    /**
     * @brief 解析通用CREATE语句
     *
     * WHY: 作为CREATE语句族的入口分发器。
     * WHAT: 识别CREATE后的关键字（TABLE, VIEW等），调用相应的具体解析方法。
     *
     * @return 具体的CreateXXXStatement节点
     */
    std::unique_ptr<ASTNode> parseCreateStatement();

    /**
     * @brief 解析CREATE TABLE语句
     *
     * WHY: 表是关系数据库的核心对象。
     * WHAT: 解析 "CREATE TABLE name (col defs, constraints) ..."。
     * HOW: 循环解析列定义和表级约束，处理IF NOT EXISTS等选项。
     */
    std::unique_ptr<ASTNode> parseCreateTableStatement();

    std::unique_ptr<ASTNode> parseCreateDatabaseStatement();
    std::unique_ptr<ASTNode> parseCreateIndexStatement();
    std::unique_ptr<ASTNode> parseCreateViewStatement();
    std::unique_ptr<ASTNode> parseCreateUserStatement();
    std::unique_ptr<ASTNode> parseCreateProcedureStatement();
    std::unique_ptr<ASTNode> parseCreateTriggerStatement();
    
    // ==========================================
    // DROP 语句族解析
    // ==========================================

    /**
     * @brief 解析通用DROP语句
     *
     * WHY: 作为DROP语句族的入口分发器。
     * WHAT: 识别DROP后的关键字，调用相应的具体解析方法。
     */
    std::unique_ptr<ASTNode> parseDropStatement();

    std::unique_ptr<ASTNode> parseDropTableStatement();
    std::unique_ptr<ASTNode> parseDropDatabaseStatement();
    std::unique_ptr<ASTNode> parseDropIndexStatement();
    std::unique_ptr<ASTNode> parseDropViewStatement();
    std::unique_ptr<ASTNode> parseDropUserStatement();
    std::unique_ptr<ASTNode> parseDropProcedureStatement();
    std::unique_ptr<ASTNode> parseDropTriggerStatement();
    
    // ==========================================
    // ALTER 语句族解析
    // ==========================================

    /**
     * @brief 解析通用ALTER语句
     *
     * WHY: 作为ALTER语句族的入口分发器。
     * WHAT: 识别ALTER后的关键字，目前主要支持TABLE和DATABASE。
     */
    std::unique_ptr<ASTNode> parseAlterStatement();

    /**
     * @brief 解析ALTER TABLE语句
     *
     * WHY: 支持表结构的变更（Schema Evolution）。
     * WHAT: 解析ADD COLUMN, DROP COLUMN, ALTER COLUMN, ADD CONSTRAINT等操作。
     */
    std::unique_ptr<ASTNode> parseAlterTableStatement();

    std::unique_ptr<ASTNode> parseAlterDatabaseStatement();
    
    // ==========================================
    // 辅助解析方法
    // ==========================================

    /**
     * @brief 解析列定义
     *
     * WHY: 列是表的组成单元，包含名称、类型、默认值和列级约束。
     * WHAT: 解析 "col_name type [DEFAULT val] [NOT NULL] ..."。
     *
     * @return ColumnDefinition对象
     */
    std::unique_ptr<ColumnDefinition> parseColumnDefinition();

    /**
     * @brief 解析表级约束
     *
     * WHY: 表级约束涉及多列或特定逻辑（如复合主键）。
     * WHAT: 解析 PRIMARY KEY (cols), FOREIGN KEY ... REFERENCES ... 等。
     *
     * @return TableConstraint对象
     */
    std::unique_ptr<TableConstraint> parseTableConstraint();

    std::vector<std::unique_ptr<ColumnDefinition>> parseColumnDefinitions();
    std::vector<std::unique_ptr<TableConstraint>> parseTableConstraints();
};

} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_PARSERS_DDL_PARSER_DDL_H