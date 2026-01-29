/**
 * @file parser_dcl.cpp
 * @brief DCL语句解析器实现
 * 
 * 专门处理数据控制语言（DCL）语句的解析，包括：
 * - GRANT语句（权限授予）
 * - REVOKE语句（权限撤销）
 */

#include "parser_dcl.h"
#include "../../token.h"
#include "../../ast/ast_nodes.h"
#include <iostream>
#include <stdexcept>

namespace sqlcc {
namespace sql_parser {

ParserDCL::ParserDCL(TokenStream& tokens) : ParserCore(tokens) {}

// ==================== GRANT语句解析 ====================

std::unique_ptr<ASTNode> ParserDCL::parseGrantStatement() {
    std::cout << "[PARSER DEBUG] parseGrantStatement() called" << std::endl;
    
    consume(Type::KEYWORD_GRANT);
    
    // 解析权限列表
    std::vector<std::string> privileges;
    bool first = true;
    
    while (!check(Type::KEYWORD_ON) && !isAtEnd()) {
        if (!first) {
            consume(Type::COMMA);
        }
        first = false;
        
        std::string privilege = parseIdentifier();
        privileges.push_back(privilege);
    }
    
    consume(Type::KEYWORD_ON);
    
    // 解析对象类型和名称
    std::string objectType = parseIdentifier();
    std::string objectName = parseQualifiedName();
    
    consume(Type::KEYWORD_TO);
    
    // 解析用户名
    std::string userName = parseIdentifier();
    
    auto stmt = std::make_unique<GrantStatement>();
    for (const auto& privilege : privileges) {
        stmt->addPrivilege(privilege);
    }
    stmt->setObjectType(objectType);
    stmt->setObjectName(objectName);
    stmt->setGrantee(userName);
    
    return stmt;
}

// ==================== REVOKE语句解析 ====================

std::unique_ptr<ASTNode> ParserDCL::parseRevokeStatement() {
    std::cout << "[PARSER DEBUG] parseRevokeStatement() called" << std::endl;
    
    consume(Type::KEYWORD_REVOKE);
    
    // 解析权限列表
    std::vector<std::string> privileges;
    bool first = true;
    
    while (!check(Type::KEYWORD_ON) && !isAtEnd()) {
        if (!first) {
            consume(Type::COMMA);
        }
        first = false;
        
        std::string privilege = parseIdentifier();
        privileges.push_back(privilege);
    }
    
    consume(Type::KEYWORD_ON);
    
    // 解析对象类型和名称
    std::string objectType = parseIdentifier();
    std::string objectName = parseQualifiedName();
    
    consume(Type::KEYWORD_FROM);
    
    // 解析用户名
    std::string userName = parseIdentifier();
    
    auto stmt = std::make_unique<RevokeStatement>();
    for (const auto& privilege : privileges) {
        stmt->addPrivilege(privilege);
    }
    stmt->setObjectType(objectType);
    stmt->setObjectName(objectName);
    stmt->setGrantee(userName);
    
    return stmt;
}

} // namespace sql_parser
} // namespace sqlcc