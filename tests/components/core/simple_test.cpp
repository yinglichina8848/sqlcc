#include <iostream>
#include <memory>
#include "database_manager.h"
#include "system_database.h"
#include "user_manager.h"

using namespace sqlcc;

int main() {
    try {
        // 创建DatabaseManager
        auto db_manager = std::make_shared<DatabaseManager>("./test_data");
        
        // 创建SystemDatabase
        auto sys_db = std::make_shared<SystemDatabase>(db_manager);
        
        // 初始化SystemDatabase
        if (!sys_db->Initialize()) {
            std::cout << "Failed to initialize SystemDatabase: " << sys_db->GetLastError() << std::endl;
            return 1;
        }
        
        std::cout << "SystemDatabase initialized successfully" << std::endl;
        
        // 创建UserManager
        UserManager user_mgr("./test_data");
        user_mgr.SetSystemDatabase(sys_db);
        
        // 测试创建用户
        if (user_mgr.CreateUser("testuser", "password123", "USER")) {
            std::cout << "User 'testuser' created successfully" << std::endl;
        } else {
            std::cout << "Failed to create user: " << user_mgr.GetLastError() << std::endl;
            return 1;
        }
        
        // 测试创建角色
        if (user_mgr.CreateRole("testrole")) {
            std::cout << "Role 'testrole' created successfully" << std::endl;
        } else {
            std::cout << "Failed to create role: " << user_mgr.GetLastError() << std::endl;
            return 1;
        }
        
        // 测试授予权限
        if (user_mgr.GrantPrivilege("testuser", "testdb", "testtable", "SELECT")) {
            std::cout << "Granted SELECT privilege to 'testuser'" << std::endl;
        } else {
            std::cout << "Failed to grant privilege: " << user_mgr.GetLastError() << std::endl;
            return 1;
        }
        
        // 测试撤销权限
        if (user_mgr.RevokePrivilege("testuser", "testdb", "testtable", "SELECT")) {
            std::cout << "Revoked SELECT privilege from 'testuser'" << std::endl;
        } else {
            std::cout << "Failed to revoke privilege: " << user_mgr.GetLastError() << std::endl;
            return 1;
        }
        
        // 测试删除角色
        if (user_mgr.DropRole("testrole")) {
            std::cout << "Role 'testrole' dropped successfully" << std::endl;
        } else {
            std::cout << "Failed to drop role: " << user_mgr.GetLastError() << std::endl;
            return 1;
        }
        
        // 测试删除用户
        if (user_mgr.DropUser("testuser")) {
            std::cout << "User 'testuser' dropped successfully" << std::endl;
        } else {
            std::cout << "Failed to drop user: " << user_mgr.GetLastError() << std::endl;
            return 1;
        }
        
        std::cout << "All tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cout << "Exception occurred: " << e.what() << std::endl;
        return 1;
    }
}