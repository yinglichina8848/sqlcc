#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include "../include/core/core_database_manager.h"

using namespace sqlcc;

int main() {
    std::cout << "开始 CRUD 性能测试..." << std::endl;

    try {
        // 创建数据库管理器
        auto db_manager = std::make_unique<DatabaseManager>("/tmp/test_db");

        // 测试数据库操作
        std::cout << "1. 创建数据库..." << std::endl;
        bool result = db_manager->CreateDatabase("testdb");
        std::cout << "   创建数据库结果: " << (result ? "成功" : "失败") << std::endl;

        // 测试使用数据库
        std::cout << "2. 使用数据库..." << std::endl;
        result = db_manager->UseDatabase("testdb");
        std::cout << "   使用数据库结果: " << (result ? "成功" : "失败") << std::endl;

        // 测试表操作
        std::cout << "3. 创建表..." << std::endl;
        std::vector<std::pair<std::string, std::string>> columns = {
            {"id", "INT PRIMARY KEY"},
            {"name", "VARCHAR(100)"},
            {"age", "INT"}
        };
        result = db_manager->CreateTable("testdb", "users", columns);
        std::cout << "   创建表结果: " << (result ? "成功" : "失败") << std::endl;

        // 测试表存在性检查
        std::cout << "4. 检查表存在性..." << std::endl;
        result = db_manager->TableExists("users");
        std::cout << "   表存在性检查结果: " << (result ? "存在" : "不存在") << std::endl;

        // 测试列出表
        std::cout << "5. 列出表..." << std::endl;
        auto tables = db_manager->ListTables();
        std::cout << "   表数量: " << tables.size() << std::endl;
        for (const auto& table : tables) {
            std::cout << "   - " << table << std::endl;
        }

        // 测试列出数据库
        std::cout << "6. 列出数据库..." << std::endl;
        auto databases = db_manager->ListDatabases();
        std::cout << "   数据库数量: " << databases.size() << std::endl;
        for (const auto& db : databases) {
            std::cout << "   - " << db << std::endl;
        }

        // 测试事务操作
        std::cout << "7. 测试事务..." << std::endl;
        TransactionId txn_id = db_manager->BeginTransaction();
        std::cout << "   事务ID: " << txn_id << std::endl;

        result = db_manager->CommitTransaction(txn_id);
        std::cout << "   提交事务结果: " << (result ? "成功" : "失败") << std::endl;

        // 测试删除表
        std::cout << "8. 删除表..." << std::endl;
        result = db_manager->DropTable("users");
        std::cout << "   删除表结果: " << (result ? "成功" : "失败") << std::endl;

        // 测试删除数据库
        std::cout << "9. 删除数据库..." << std::endl;
        result = db_manager->DropDatabase("testdb");
        std::cout << "   删除数据库结果: " << (result ? "成功" : "失败") << std::endl;

        // 关闭数据库管理器
        std::cout << "10. 关闭数据库管理器..." << std::endl;
        result = db_manager->Close();
        std::cout << "    关闭结果: " << (result ? "成功" : "失败") << std::endl;

        std::cout << "\nCRUD 性能测试完成!" << std::endl;
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "测试失败: " << e.what() << std::endl;
        return 1;
    }
}
