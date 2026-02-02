/**
 * @file parser_dcl.h
 * @brief SQLCC DCL解析器 - 数据控制语言解析引擎
 *
 * ParserDCL 负责解析SQL中的数据控制语言（Data Control Language），
 * 主要用于定义数据库的安全策略，包括用户权限的授予（GRANT）和撤销（REVOKE）。
 * 它是实现基于角色的访问控制（RBAC）模型的基础组件。
 *
 * 📚 配套教材参考：
 * - [第10章：数据库安全性](../../textbook/《数据库系统原理与开发实践》.md#第十章数据库安全性)
 * - [10.1 存取控制技术](../../textbook/《数据库系统原理与开发实践》.md#101-存取控制技术)
 * - [10.2 自主存取控制(DAC)](../../textbook/《数据库系统原理与开发实践》.md#102-自主存取控制dac)
 * - [10.3 角色授权](../../textbook/《数据库系统原理与开发实践》.md#103-角色授权)
 *
 * WHY层 - 设计意图：
 *   安全性是企业级数据库的关键特性。通过专门的DCL解析器，可以集中处理
 *   复杂的权限验证语法，确保授权指令被正确解析，防止权限提升或越权访问
 *   等安全隐患。
 *
 * WHAT层 - 功能说明：
 *   - 权限授予：GRANT <privileges> ON <object> TO <users> [WITH GRANT OPTION]
 *   - 权限撤销：REVOKE <privileges> ON <object> FROM <users> [CASCADE/RESTRICT]
 *   - 权限验证：验证权限名称和对象类型的合法性
 *
 * HOW层 - 实现机制：
 *   - 语法分析：识别标准SQL授权语法结构
 *   - 权限集解析：处理逗号分隔的权限列表或ALL PRIVILEGES
 *   - 对象识别：区分表、视图、数据库等不同粒度的授权对象
 *
 * @author SQLCC技术委员会
 * @version 1.1.0
 * @date 2026-02-02
 */

#ifndef SQLCC_SQL_PARSER_PARSERS_DCL_PARSER_DCL_H
#define SQLCC_SQL_PARSER_PARSERS_DCL_PARSER_DCL_H

#include "../parser_core.h"
#include "../../ast/ast_node.h"
#include <vector>
#include <string>

namespace sqlcc {
namespace sql_parser {

/**
 * @brief DCL语句解析器类
 *
 * WHY层 - 设计意图：
 *   将安全相关的语法解析独立出来，便于进行严格的代码审计和安全性审查。
 *
 * WHAT层 - 核心职责：
 *   - 解析GRANT语句构建GrantStatement
 *   - 解析REVOKE语句构建RevokeStatement
 *   - 提供辅助方法解析权限列表和用户列表
 */
class ParserDCL : public ParserCore {
public:
    /**
     * @brief 构造函数
     * @param tokens 词法分析器生成的Token流
     */
    ParserDCL(TokenStream& tokens);
    
    /**
     * @brief 解析GRANT语句
     *
     * WHY: 允许数据库管理员向用户或角色授予特定的操作权限。
     * WHAT: 解析 "GRANT priv1, priv2 ON object_type object_name TO user1, user2 [WITH GRANT OPTION]"。
     * HOW: 
     *   1. 解析权限列表
     *   2. 解析ON子句（对象类型和名称）
     *   3. 解析TO子句（用户列表）
     *   4. 检查是否包含WITH GRANT OPTION
     *
     * @return GrantStatement的AST节点指针
     */
    std::unique_ptr<ASTNode> parseGrantStatement();

    /**
     * @brief 解析REVOKE语句
     *
     * WHY: 允许数据库管理员收回之前授予的权限。
     * WHAT: 解析 "REVOKE priv1, priv2 ON object_type object_name FROM user1, user2 [CASCADE/RESTRICT]"。
     * HOW:
     *   1. 解析权限列表
     *   2. 解析ON子句
     *   3. 解析FROM子句
     *   4. 处理级联撤销选项（CASCADE/RESTRICT）
     *
     * @return RevokeStatement的AST节点指针
     */
    std::unique_ptr<ASTNode> parseRevokeStatement();
    
    // ==========================================
    // DCL辅助解析方法
    // ==========================================

    /**
     * @brief 解析权限列表
     *
     * @return 权限名称字符串列表（如 SELECT, INSERT）
     */
    std::vector<std::string> parsePrivilegeList();

    /**
     * @brief 解析对象类型
     *
     * @return 对象类型字符串（如 TABLE, DATABASE, VIEW）
     */
    std::string parseObjectType();

    /**
     * @brief 解析对象名称
     *
     * @return 对象标识符
     */
    std::string parseObjectName();

    /**
     * @brief 解析用户列表
     *
     * @return 用户名字符串列表
     */
    std::vector<std::string> parseUserNames();
    
    // 权限验证方法
    bool isValidPrivilege(const std::string& privilege);
    bool isValidObjectType(const std::string& object_type);
};

} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_PARSERS_DCL_PARSER_DCL_H