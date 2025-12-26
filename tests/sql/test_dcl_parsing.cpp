#include <sql_parser/parser.h>
#include <sql_parser/ast_nodes.h>
#include <iostream>
#include <memory>

using namespace sqlcc::sql_parser;

int main() {
    std::cout << "=== 测试DCL语句解析 ===" << std::endl;
    
    // 测试GRANT语句
    std::string grantSql = "GRANT ALL PRIVILEGES ON TABLE users TO testuser;";
    std::cout << "\n测试SQL: " << grantSql << std::endl;
    
    try {
        Parser grantParser(grantSql);
        auto grantStatements = grantParser.parse();
        
        if (!grantStatements.empty()) {
            auto stmt = dynamic_cast<GrantStatement*>(grantStatements[0].get());
            if (stmt) {
                std::cout << "✅ GRANT语句解析成功" << std::endl;
                std::cout << "  权限数量: " << stmt->getPrivileges().size() << std::endl;
                for (const auto& priv : stmt->getPrivileges()) {
                    std::cout << "  权限: " << priv << std::endl;
                }
                std::cout << "  对象类型: " << stmt->getObjectType() << std::endl;
                std::cout << "  对象名称: " << stmt->getObjectName() << std::endl;
                std::cout << "  被授权用户: " << stmt->getGrantee() << std::endl;
            } else {
                std::cout << "❌ 解析结果不是GrantStatement类型" << std::endl;
            }
        } else {
            std::cout << "❌ 未能解析出语句" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << "❌ GRANT语句解析失败: " << e.what() << std::endl;
    }
    
    // 测试REVOKE语句
    std::string revokeSql = "REVOKE SELECT, INSERT ON TABLE products FROM admin;";
    std::cout << "\n测试SQL: " << revokeSql << std::endl;
    
    try {
        Parser revokeParser(revokeSql);
        auto revokeStatements = revokeParser.parse();
        
        if (!revokeStatements.empty()) {
            auto stmt = dynamic_cast<RevokeStatement*>(revokeStatements[0].get());
            if (stmt) {
                std::cout << "✅ REVOKE语句解析成功" << std::endl;
                std::cout << "  权限数量: " << stmt->getPrivileges().size() << std::endl;
                for (const auto& priv : stmt->getPrivileges()) {
                    std::cout << "  权限: " << priv << std::endl;
                }
                std::cout << "  对象类型: " << stmt->getObjectType() << std::endl;
                std::cout << "  对象名称: " << stmt->getObjectName() << std::endl;
                std::cout << "  被撤销权限用户: " << stmt->getGrantee() << std::endl;
            } else {
                std::cout << "❌ 解析结果不是RevokeStatement类型" << std::endl;
            }
        } else {
            std::cout << "❌ 未能解析出语句" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << "❌ REVOKE语句解析失败: " << e.what() << std::endl;
    }
    
    // 测试CREATE USER语句
    std::string createUserSql = "CREATE USER testuser IDENTIFIED BY 'password123';";
    std::cout << "\n测试SQL: " << createUserSql << std::endl;
    
    try {
        Parser createUserParser(createUserSql);
        auto createUserStatements = createUserParser.parse();
        
        if (!createUserStatements.empty()) {
            auto stmt = dynamic_cast<CreateUserStatement*>(createUserStatements[0].get());
            if (stmt) {
                std::cout << "✅ CREATE USER语句解析成功" << std::endl;
                std::cout << "  用户名: " << stmt->getUsername() << std::endl;
                std::cout << "  密码: " << stmt->getPassword() << std::endl;
                std::cout << "  是否为WITH PASSWORD格式: " << (stmt->isWithPassword() ? "是" : "否") << std::endl;
            } else {
                std::cout << "❌ 解析结果不是CreateUserStatement类型" << std::endl;
            }
        } else {
            std::cout << "❌ 未能解析出语句" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << "❌ CREATE USER语句解析失败: " << e.what() << std::endl;
    }
    
    // 测试DROP USER语句
    std::string dropUserSql = "DROP USER IF EXISTS olduser;";
    std::cout << "\n测试SQL: " << dropUserSql << std::endl;
    
    try {
        Parser dropUserParser(dropUserSql);
        auto dropUserStatements = dropUserParser.parse();
        
        if (!dropUserStatements.empty()) {
            auto stmt = dynamic_cast<DropUserStatement*>(dropUserStatements[0].get());
            if (stmt) {
                std::cout << "✅ DROP USER语句解析成功" << std::endl;
                std::cout << "  用户名: " << stmt->getUsername() << std::endl;
                std::cout << "  是否有IF EXISTS: " << (stmt->isIfExists() ? "是" : "否") << std::endl;
            } else {
                std::cout << "❌ 解析结果不是DropUserStatement类型" << std::endl;
            }
        } else {
            std::cout << "❌ 未能解析出语句" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << "❌ DROP USER语句解析失败: " << e.what() << std::endl;
    }
    
    std::cout << "\n=== DCL语句解析测试完成 ===" << std::endl;
    return 0;
}
