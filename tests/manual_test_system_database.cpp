#include "system_database.h"
#include "database_manager.h"
#include <iostream>
#include <memory>

using namespace sqlcc;

int main() {
    std::cout << "=== SystemDatabase Manual Test ===" << std::endl;
    
    // 创建DatabaseManager
    auto db_manager = std::make_shared<DatabaseManager>("./test_manual_system_db", 1024, 4, 4);
    std::cout << "[INFO] DatabaseManager created" << std::endl;
    
    // 创建SystemDatabase
    auto sys_db = std::make_shared<SystemDatabase>(db_manager);
    std::cout << "[INFO] SystemDatabase created" << std::endl;
    
    // 初始化系统数据库
    bool init_result = sys_db->Initialize();
    std::cout << "[TEST] Initialize(): " << (init_result ? "SUCCESS" : "FAILED") << std::endl;
    if (!init_result) {
        std::cout << "[ERROR] " << sys_db->GetLastError() << std::endl;
        return 1;
    }
    
    // 测试创建数据库记录
    std::cout << "\n--- Testing CreateDatabaseRecord ---" << std::endl;
    bool create_db = sys_db->CreateDatabaseRecord("test_db", "root", "Test database");
    std::cout << "[TEST] CreateDatabaseRecord('test_db'): " << (create_db ? "SUCCESS" : "FAILED") << std::endl;
    if (!create_db) {
        std::cout << "[ERROR] " << sys_db->GetLastError() << std::endl;
    }
    
    // 测试数据库是否存在
    std::cout << "\n--- Testing DatabaseExists ---" << std::endl;
    bool db_exists = sys_db->DatabaseExists("test_db");
    std::cout << "[TEST] DatabaseExists('test_db'): " << (db_exists ? "true" : "false") << std::endl;
    if (!db_exists) {
        std::cout << "[ERROR] " << sys_db->GetLastError() << std::endl;
    }
    
    // 测试创建用户记录
    std::cout << "\n--- Testing CreateUserRecord ---" << std::endl;
    bool create_user = sys_db->CreateUserRecord("alice", "password_hash_123", "admin");
    std::cout << "[TEST] CreateUserRecord('alice'): " << (create_user ? "SUCCESS" : "FAILED") << std::endl;
    if (!create_user) {
        std::cout << "[ERROR] " << sys_db->GetLastError() << std::endl;
    }
    
    // 测试用户是否存在
    std::cout << "\n--- Testing UserExists ---" << std::endl;
    bool user_exists = sys_db->UserExists("alice");
    std::cout << "[TEST] UserExists('alice'): " << (user_exists ? "true" : "false") << std::endl;
    if (!user_exists) {
        std::cout << "[ERROR] " << sys_db->GetLastError() << std::endl;
    }
    
    // 测试创建角色记录
    std::cout << "\n--- Testing CreateRoleRecord ---" << std::endl;
    bool create_role = sys_db->CreateRoleRecord("admin_role");
    std::cout << "[TEST] CreateRoleRecord('admin_role'): " << (create_role ? "SUCCESS" : "FAILED") << std::endl;
    if (!create_role) {
        std::cout << "[ERROR] " << sys_db->GetLastError() << std::endl;
    }
    
    // 测试角色是否存在
    std::cout << "\n--- Testing RoleExists ---" << std::endl;
    bool role_exists = sys_db->RoleExists("admin_role");
    std::cout << "[TEST] RoleExists('admin_role'): " << (role_exists ? "true" : "false") << std::endl;
    if (!role_exists) {
        std::cout << "[ERROR] " << sys_db->GetLastError() << std::endl;
    }
    
    // 测试创建表记录
    std::cout << "\n--- Testing CreateTableRecord ---" << std::endl;
    int64_t db_id = 1001;
    bool create_table = sys_db->CreateTableRecord(db_id, "public", "users", "root", "BASE TABLE");
    std::cout << "[TEST] CreateTableRecord('users'): " << (create_table ? "SUCCESS" : "FAILED") << std::endl;
    if (!create_table) {
        std::cout << "[ERROR] " << sys_db->GetLastError() << std::endl;
    }
    
    // 测试表是否存在
    std::cout << "\n--- Testing TableExists ---" << std::endl;
    bool table_exists = sys_db->TableExists("public", "users");
    std::cout << "[TEST] TableExists('public', 'users'): " << (table_exists ? "true" : "false") << std::endl;
    if (!table_exists) {
        std::cout << "[ERROR] " << sys_db->GetLastError() << std::endl;
    }
    
    // 测试创建列记录
    std::cout << "\n--- Testing CreateColumnRecord ---" << std::endl;
    int64_t table_id = 2001;
    bool create_column = sys_db->CreateColumnRecord(table_id, "id", "INT", false, "", 1);
    std::cout << "[TEST] CreateColumnRecord('id'): " << (create_column ? "SUCCESS" : "FAILED") << std::endl;
    if (!create_column) {
        std::cout << "[ERROR] " << sys_db->GetLastError() << std::endl;
    }
    
    // 测试创建索引记录
    std::cout << "\n--- Testing CreateIndexRecord ---" << std::endl;
    bool create_index = sys_db->CreateIndexRecord(table_id, "idx_id", "id", false, "BTREE");
    std::cout << "[TEST] CreateIndexRecord('idx_id'): " << (create_index ? "SUCCESS" : "FAILED") << std::endl;
    if (!create_index) {
        std::cout << "[ERROR] " << sys_db->GetLastError() << std::endl;
    }
    
    // 测试创建约束记录
    std::cout << "\n--- Testing CreateConstraintRecord ---" << std::endl;
    bool create_constraint = sys_db->CreateConstraintRecord(table_id, "pk_id", "PRIMARY KEY", "id", "", "", "");
    std::cout << "[TEST] CreateConstraintRecord('pk_id'): " << (create_constraint ? "SUCCESS" : "FAILED") << std::endl;
    if (!create_constraint) {
        std::cout << "[ERROR] " << sys_db->GetLastError() << std::endl;
    }
    
    // 测试授予权限记录
    std::cout << "\n--- Testing GrantPrivilegeRecord ---" << std::endl;
    bool grant_priv = sys_db->GrantPrivilegeRecord("USER", "alice", "test_db", "users", "SELECT", "root");
    std::cout << "[TEST] GrantPrivilegeRecord: " << (grant_priv ? "SUCCESS" : "FAILED") << std::endl;
    if (!grant_priv) {
        std::cout << "[ERROR] " << sys_db->GetLastError() << std::endl;
    }
    
    std::cout << "\n=== All Manual Tests Completed ===" << std::endl;
    
    // 清理
    db_manager->Close();
    
    return 0;
}
