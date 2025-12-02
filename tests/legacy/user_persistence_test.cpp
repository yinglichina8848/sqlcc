#include "database_manager.h"
#include "user_manager.h"
#include <iostream>
#include <filesystem>

using namespace sqlcc;
namespace fs = std::filesystem;

int main() {
    try {
        std::cout << "=== User and Permission Persistence Test ===" << std::endl;
        
        // 清理之前的测试数据
        fs::remove_all("./user_test_data");
        fs::create_directories("./user_test_data");
        
        std::cout << "Part 1: Creating users and permissions..." << std::endl;
        {
            UserManager user_manager("./user_test_data");
            
            // 创建角色
            std::cout << "Creating role 'developer'..." << std::endl;
            if (!user_manager.CreateRole("developer")) {
                std::cerr << "Failed to create role 'developer': " << user_manager.GetLastError() << std::endl;
                return 1;
            }
            std::cout << "Role 'developer' created successfully!" << std::endl;
            
            // 创建用户
            std::cout << "Creating user 'alice' with password 'password123'..." << std::endl;
            if (!user_manager.CreateUser("alice", "password123", "developer")) {
                std::cerr << "Failed to create user 'alice': " << user_manager.GetLastError() << std::endl;
                return 1;
            }
            std::cout << "User 'alice' created successfully!" << std::endl;
            
            // 授予权限
            std::cout << "Granting SELECT permission on database 'testdb' to user 'alice'..." << std::endl;
            if (!user_manager.GrantPrivilege("alice", "testdb", "*", "SELECT")) {
                std::cerr << "Failed to grant SELECT permission to user 'alice': " << user_manager.GetLastError() << std::endl;
                return 1;
            }
            std::cout << "SELECT permission granted to user 'alice' successfully!" << std::endl;
            
            // 授予权限给角色
            std::cout << "Granting INSERT permission on database 'testdb' to role 'developer'..." << std::endl;
            if (!user_manager.GrantPrivilege("developer", "testdb", "*", "INSERT")) {
                std::cerr << "Failed to grant INSERT permission to role 'developer': " << user_manager.GetLastError() << std::endl;
                return 1;
            }
            std::cout << "INSERT permission granted to role 'developer' successfully!" << std::endl;
            
            std::cout << "Users in system:" << std::endl;
            auto users = user_manager.ListUsers();
            for (const auto& user : users) {
                std::cout << "  - " << user.username << " (" << user.role << ")" << std::endl;
            }
            
            std::cout << "Roles in system:" << std::endl;
            auto roles = user_manager.ListRoles();
            for (const auto& role : roles) {
                std::cout << "  - " << role.role_name << std::endl;
            }
        } // UserManager在这里被销毁，应该触发自动保存
        
        std::cout << "\nPart 1 completed. UserManager destroyed.\n" << std::endl;
        
        std::cout << "Part 2: Verifying persistence after restart..." << std::endl;
        {
            UserManager user_manager("./user_test_data");
            
            std::cout << "Users after restart:" << std::endl;
            auto users = user_manager.ListUsers();
            for (const auto& user : users) {
                std::cout << "  - " << user.username << " (" << user.role << ")" << std::endl;
            }
            
            std::cout << "Roles after restart:" << std::endl;
            auto roles = user_manager.ListRoles();
            for (const auto& role : roles) {
                std::cout << "  - " << role.role_name << std::endl;
            }
            
            // 验证用户存在
            std::cout << "Checking if user 'alice' exists..." << std::endl;
            users = user_manager.ListUsers();
            bool alice_exists = false;
            for (const auto& user : users) {
                if (user.username == "alice") {
                    alice_exists = true;
                    break;
                }
            }
            
            if (alice_exists) {
                std::cout << "User 'alice' exists after restart!" << std::endl;
            } else {
                std::cout << "User 'alice' does not exist after restart!" << std::endl;
                return 1;
            }
            
            // 验证角色存在
            std::cout << "Checking if role 'developer' exists..." << std::endl;
            roles = user_manager.ListRoles();
            bool developer_exists = false;
            for (const auto& role : roles) {
                if (role.role_name == "developer") {
                    developer_exists = true;
                    break;
                }
            }
            
            if (developer_exists) {
                std::cout << "Role 'developer' exists after restart!" << std::endl;
            } else {
                std::cout << "Role 'developer' does not exist after restart!" << std::endl;
                return 1;
            }
            
            // 验证权限
            std::cout << "Checking permissions for user 'alice'..." << std::endl;
            auto user_permissions = user_manager.ListUserPermissions("alice");
            std::cout << "User 'alice' has " << user_permissions.size() << " direct permissions." << std::endl;
            for (const auto& perm : user_permissions) {
                std::cout << "  - " << perm.privilege << " on " << perm.database << "." << perm.table << std::endl;
            }
            
            std::cout << "Checking permissions for role 'developer'..." << std::endl;
            auto role_permissions = user_manager.ListRolePermissions("developer");
            std::cout << "Role 'developer' has " << role_permissions.size() << " permissions." << std::endl;
            for (const auto& perm : role_permissions) {
                std::cout << "  - " << perm.privilege << " on " << perm.database << "." << perm.table << std::endl;
            }
        }
        
        std::cout << "\nPart 2 completed. Test finished successfully!" << std::endl;
        
        // 检查生成的文件
        std::cout << "\nGenerated files:" << std::endl;
        if (fs::exists("./user_test_data")) {
            for (const auto& entry : fs::directory_iterator("./user_test_data")) {
                std::cout << "  " << entry.path().filename().string() << std::endl;
            }
        }
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}