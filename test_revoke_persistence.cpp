#include "include/user_manager.h"
#include "include/system_database.h"
#include "include/database_manager.h"
#include "include/sql_executor.h"
#include <iostream>
#include <memory>
#include <filesystem>

using namespace sqlcc;
namespace fs = std::filesystem;

int main() {
    std::cout << "=== REVOKE 持久化测试 ===" << std::endl;
    
    std::string test_dir = "./test_revoke_data";
    
    // 清理旧测试数据
    if (fs::exists(test_dir)) {
        fs::remove_all(test_dir);
    }
    
    // ========== 阶段1: 创建用户并授权 ==========
    std::cout << "\n【阶段1】初始化并授权..." << std::endl;
    {
        auto db_manager = std::make_shared<DatabaseManager>(test_dir);
        auto sys_db = std::make_shared<SystemDatabase>(db_manager);
        
        if (!sys_db->Initialize()) {
            std::cerr << "❌ Failed to initialize SystemDatabase" << std::endl;
            return 1;
        }
        std::cout << "✓ SystemDatabase initialized" << std::endl;
        
        UserManager user_mgr(test_dir);
        user_mgr.SetSystemDatabase(sys_db.get());
        
        // 创建测试用户
        if (!user_mgr.CreateUser("alice", "pass123", "USER")) {
            std::cerr << "❌ Failed to create user alice" << std::endl;
            return 1;
        }
        std::cout << "✓ User 'alice' created" << std::endl;
        
        // 授予3个权限
        if (!user_mgr.GrantPrivilege("alice", "testdb", "users", "SELECT")) {
            std::cerr << "❌ Failed to grant SELECT" << std::endl;
            return 1;
        }
        std::cout << "✓ Granted: SELECT on testdb.users" << std::endl;
        
        if (!user_mgr.GrantPrivilege("alice", "testdb", "users", "INSERT")) {
            std::cerr << "❌ Failed to grant INSERT" << std::endl;
            return 1;
        }
        std::cout << "✓ Granted: INSERT on testdb.users" << std::endl;
        
        if (!user_mgr.GrantPrivilege("alice", "testdb", "orders", "UPDATE")) {
            std::cerr << "❌ Failed to grant UPDATE" << std::endl;
            return 1;
        }
        std::cout << "✓ Granted: UPDATE on testdb.orders" << std::endl;
        
        // 验证权限已写入（通过SQL查询）
        SqlExecutor sql_exec(db_manager);
        std::string result = sql_exec.Execute("SELECT * FROM sys_privileges WHERE grantee_name = 'alice'");
        std::cout << "SQL查询结果:\n" << result << std::endl;
        
        // UserManager和SystemDatabase析构时会保存数据
    }
    
    // ========== 阶段2: 重新加载并撤销1个权限 ==========
    std::cout << "\n【阶段2】重新加载并撤销权限..." << std::endl;
    {
        auto db_manager = std::make_shared<DatabaseManager>(test_dir);
        auto sys_db = std::make_shared<SystemDatabase>(db_manager);
        
        if (!sys_db->Initialize()) {
            std::cerr << "❌ Failed to re-initialize SystemDatabase" << std::endl;
            return 1;
        }
        
        UserManager user_mgr(test_dir);
        user_mgr.SetSystemDatabase(sys_db.get());
        
        // 验证权限已持久化（仍然是3个）
        SqlExecutor sql_exec(db_manager);
        std::string result_before = sql_exec.Execute("SELECT * FROM sys_privileges WHERE grantee_name = 'alice'");
        std::cout << "\n撤销前的权限列表:\n" << result_before << std::endl;
        
        // 撤销INSERT权限
        std::cout << "\n执行REVOKE..." << std::endl;
        if (!user_mgr.RevokePrivilege("alice", "testdb", "users", "INSERT")) {
            std::cerr << "❌ Failed to revoke INSERT privilege" << std::endl;
            return 1;
        }
        std::cout << "✓ Revoked: INSERT on testdb.users" << std::endl;
        
        // 验证内存中权限已减少
        std::string result_after = sql_exec.Execute("SELECT * FROM sys_privileges WHERE grantee_name = 'alice'");
        std::cout << "\n撤销后的权限列表:\n" << result_after << std::endl;
        
        // UserManager和SystemDatabase析构时会保存数据
    }
    
    // ========== 阶段3: 再次重新加载验证持久化 ==========
    std::cout << "\n【阶段3】验证REVOKE是否持久化..." << std::endl;
    {
        auto db_manager = std::make_shared<DatabaseManager>(test_dir);
        auto sys_db = std::make_shared<SystemDatabase>(db_manager);
        
        if (!sys_db->Initialize()) {
            std::cerr << "❌ Failed to re-initialize SystemDatabase" << std::endl;
            return 1;
        }
        
        UserManager user_mgr(test_dir);
        user_mgr.SetSystemDatabase(sys_db.get());
        
        // 验证REVOKE已持久化
        SqlExecutor sql_exec(db_manager);
        std::string final_result = sql_exec.Execute("SELECT * FROM sys_privileges WHERE grantee_name = 'alice'");
        std::cout << "\n最终权限列表:\n" << final_result << std::endl;
        
        // 简单验证：检查输出中是否包含预期的权限
        // 如果查询成功，结果中应该包含SELECT和UPDATE，但不应该包含INSERT
        bool query_success = final_result.find("executed successfully") == std::string::npos;
        bool has_select = final_result.find("SELECT") != std::string::npos;
        bool has_update = final_result.find("UPDATE") != std::string::npos;
        bool has_insert = final_result.find("INSERT") != std::string::npos;
        
        if (!query_success) {
            std::cerr << "❌ 查询权限表失败" << std::endl;
            return 1;
        }
        
        if (!has_select) {
            std::cerr << "❌ Missing SELECT privilege!" << std::endl;
            return 1;
        }
        if (!has_update) {
            std::cerr << "❌ Missing UPDATE privilege!" << std::endl;
            return 1;
        }
        if (has_insert) {
            std::cerr << "❌ INSERT privilege should have been revoked!" << std::endl;
            return 1;
        }
        
        std::cout << "\n✓ SELECT权限存在" << std::endl;
        std::cout << "✓ UPDATE权限存在" << std::endl;
        std::cout << "✓ INSERT权限已被正确撤销" << std::endl;
    }
    
    std::cout << "\n=== ✅ 所有测试通过！REVOKE功能正常工作并已持久化 ===" << std::endl;
    
    // 清理测试数据
    if (fs::exists(test_dir)) {
        fs::remove_all(test_dir);
        std::cout << "\n测试数据已清理" << std::endl;
    }
    
    return 0;
}
