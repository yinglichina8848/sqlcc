#include "include/user_manager.h"
#include "include/system_database.h"
#include "include/database_manager.h"
#include <iostream>
#include <memory>

using namespace sqlcc;

int main() {
    std::cout << "=== GRANT/REVOKE 完整功能测试 ===" << std::endl;
    
    // 1. 初始化DatabaseManager和SystemDatabase
    std::cout << "\n1. 初始化系统..." << std::endl;
    auto db_manager = std::make_shared<DatabaseManager>("./test_grant_revoke_data");
    auto sys_db = std::make_shared<SystemDatabase>(db_manager);
    
    if (!sys_db->Initialize()) {
        std::cerr << "Failed to initialize system database: " << sys_db->GetLastError() << std::endl;
        return 1;
    }
    std::cout << "   ✓ SystemDatabase initialized" << std::endl;
    
    // 2. 创建UserManager并设置SystemDatabase引用
    std::cout << "\n2. 初始化UserManager..." << std::endl;
    UserManager user_mgr("./test_grant_revoke_data");
    user_mgr.SetSystemDatabase(sys_db.get());
    std::cout << "   ✓ UserManager initialized with SystemDatabase" << std::endl;
    
    // 3. 创建测试用户
    std::cout << "\n3. 创建测试用户..." << std::endl;
    if (!user_mgr.CreateUser("testuser", "password123", "USER")) {
        std::cerr << "Failed to create user: " << user_mgr.GetLastError() << std::endl;
        return 1;
    }
    std::cout << "   ✓ User 'testuser' created" << std::endl;
    
    // 4. 授予权限（应该同时写入内存文件和SystemDatabase）
    std::cout << "\n4. 授予权限..." << std::endl;
    if (!user_mgr.GrantPrivilege("testuser", "mydb", "users", "SELECT")) {
        std::cerr << "Failed to grant privilege: " << user_mgr.GetLastError() << std::endl;
        return 1;
    }
    std::cout << "   ✓ Granted SELECT on mydb.users to testuser" << std::endl;
    
    if (!user_mgr.GrantPrivilege("testuser", "mydb", "orders", "INSERT")) {
        std::cerr << "Failed to grant privilege: " << user_mgr.GetLastError() << std::endl;
        return 1;
    }
    std::cout << "   ✓ Granted INSERT on mydb.orders to testuser" << std::endl;
    
    // 5. 从SystemDatabase查询权限
    std::cout << "\n5. 从SystemDatabase查询权限..." << std::endl;
    auto privileges = sys_db->GetUserPrivileges("testuser");
    std::cout << "   Found " << privileges.size() << " privilege(s):" << std::endl;
    for (const auto& priv : privileges) {
        std::cout << "   - " << priv.privilege << " on " 
                  << priv.db_name << "." << priv.table_name 
                  << " (grantor: " << priv.grantor << ")" << std::endl;
    }
    
    if (privileges.size() != 2) {
        std::cerr << "Expected 2 privileges, got " << privileges.size() << std::endl;
        return 1;
    }
    
    // 6. 撤销一个权限
    std::cout << "\n6. 撤销权限..." << std::endl;
    if (!user_mgr.RevokePrivilege("testuser", "mydb", "users", "SELECT")) {
        std::cerr << "Failed to revoke privilege: " << user_mgr.GetLastError() << std::endl;
        return 1;
    }
    std::cout << "   ✓ Revoked SELECT on mydb.users from testuser" << std::endl;
    
    // 7. 再次查询权限
    std::cout << "\n7. 再次查询权限（应该只剩1个）..." << std::endl;
    privileges = sys_db->GetUserPrivileges("testuser");
    std::cout << "   Found " << privileges.size() << " privilege(s):" << std::endl;
    for (const auto& priv : privileges) {
        std::cout << "   - " << priv.privilege << " on " 
                  << priv.db_name << "." << priv.table_name 
                  << " (grantor: " << priv.grantor << ")" << std::endl;
    }
    
    if (privileges.size() != 1) {
        std::cerr << "Expected 1 privilege after revoke, got " << privileges.size() << std::endl;
        return 1;
    }
    
    std::cout << "\n=== 所有测试通过! ===" << std::endl;
    return 0;
}
