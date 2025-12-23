#include "core/core_database_manager.h"
#include "core/user_manager.h"
#include "core/permission_validator.h"
#include <iostream>
#include <memory>

int main() {
    // 初始化数据库管理器
    auto db_manager = std::make_shared<sqlcc::DatabaseManager>("./test_drop_permission.db", 1024, 4, 2);
    auto user_manager = std::make_shared<sqlcc::UserManager>();
    
    // 初始化权限验证器
    auto permission_validator = std::make_shared<sqlcc::PermissionValidator>(user_manager, db_manager);
    
    // 测试表删除权限 - 表不存在的情况
    std::cout << "Testing DROP permission for non-existent table 'test_table'" << std::endl;
    sqlcc::PermissionResult result = permission_validator->validate(
        sqlcc::PermissionOperation::DROP_TABLE, "test_table", "admin", "test_db");
    
    std::cout << "Permission allowed: " << (result.allowed ? "YES" : "NO") << std::endl;
    if (!result.allowed && result.error) {
        std::cout << "Error message: " << result.error->getMessage() << std::endl;
    }
    
    // 创建数据库和表，然后再次测试
    std::cout << "\nCreating database and table..." << std::endl;
    db_manager->CreateDatabase("test_db");
    db_manager->UseDatabase("test_db");
    
    std::vector<std::pair<std::string, std::string>> columns = {
        {"id", "INTEGER"}, {"name", "VARCHAR"}};
    db_manager->CreateTable("test_table", columns);
    
    // 测试表删除权限 - 表存在的情况
    std::cout << "Testing DROP permission for existing table 'test_table'" << std::endl;
    result = permission_validator->validate(
        sqlcc::PermissionOperation::DROP_TABLE, "test_table", "admin", "test_db");
    
    std::cout << "Permission allowed: " << (result.allowed ? "YES" : "NO") << std::endl;
    if (!result.allowed && result.error) {
        std::cout << "Error message: " << result.error->getMessage() << std::endl;
    }
    
    // 清理测试文件
    system("rm -rf ./test_drop_permission.db");
    
    return 0;
}