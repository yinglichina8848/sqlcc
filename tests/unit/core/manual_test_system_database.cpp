#include "core/system_database.h"
#include "core/core_database_manager.h"
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
    
    // 测试IsInitialized方法
    bool is_initialized = sys_db->IsInitialized();
    std::cout << "[TEST] IsInitialized() (before init): " << (is_initialized ? "true" : "false") << std::endl;
    
    // 初始化系统数据库
    bool init_result = sys_db->Initialize();
    std::cout << "[TEST] Initialize(): " << (init_result ? "SUCCESS" : "FAILED") << std::endl;
    if (!init_result) {
        std::cout << "[ERROR] " << sys_db->GetLastError() << std::endl;
        return 1;
    }
    
    // 再次测试IsInitialized方法
    is_initialized = sys_db->IsInitialized();
    std::cout << "[TEST] IsInitialized() (after init): " << (is_initialized ? "true" : "false") << std::endl;
    
    // 测试GetDatabaseManager方法
    auto retrieved_db_manager = sys_db->GetDatabaseManager();
    std::cout << "[TEST] GetDatabaseManager(): " << (retrieved_db_manager ? "SUCCESS" : "FAILED") << std::endl;
    
    // 测试GetLastError方法
    std::string last_error = sys_db->GetLastError();
    std::cout << "[TEST] GetLastError(): " << (last_error.empty() ? "EMPTY" : last_error) << std::endl;
    
    std::cout << "\n=== All Manual Tests Completed ===" << std::endl;
    
    // 清理
    db_manager->Close();
    
    return 0;
}
