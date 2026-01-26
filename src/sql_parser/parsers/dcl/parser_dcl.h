/**
 * ParserDCL - DCL语句解析器头文件
 * 
 * 包含数据控制语言（DCL）语句的解析方法，包括：
 * - GRANT语句：权限授予
 * - REVOKE语句：权限撤销
 * 
 * 设计原则：
 * - 单一职责：专门处理DCL语句解析
 * - 模块化：每个DCL语句类型有独立的解析方法
 * - 可扩展：易于添加新的DCL语句支持
 */

#ifndef SQLCC_SQL_PARSER_PARSERS_DCL_PARSER_DCL_H
#define SQLCC_SQL_PARSER_PARSERS_DCL_PARSER_DCL_H

#include "../parser_core.h"
#include "../../ast/ast_node.h"

namespace sqlcc {
namespace sql_parser {

class ParserDCL : public ParserCore {
public:
    ParserDCL(TokenStream& tokens);
    
    // DCL语句解析方法
    std::unique_ptr<ASTNode> parseGrantStatement();
    std::unique_ptr<ASTNode> parseRevokeStatement();
    
    // DCL辅助解析方法
    std::vector<std::string> parsePrivilegeList();
    std::string parseObjectType();
    std::string parseObjectName();
    std::vector<std::string> parseUserNames();
    
    // 权限验证方法
    bool isValidPrivilege(const std::string& privilege);
    bool isValidObjectType(const std::string& object_type);
};

} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_PARSERS_DCL_PARSER_DCL_H