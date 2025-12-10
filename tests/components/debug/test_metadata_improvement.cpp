#include "../../../include/core/system_database.h"
#include "../../../include/database_manager.h"
#include "../../../include/sql_parser/ast_nodes.h"
#include <iostream>
#include <memory>

int main() {
    using namespace sqlcc;
    
    try {
        // 创建数据库管理器
        auto db_manager = std::make_shared<DatabaseManager>("./test_data", 1024, 16, 64);
        
        // 初始化数据库管理器
        if (!db_manager->Initialize()) {
            std::cout << "数据库管理器初始化失败" << std::endl;
            return 1;
        }
        
        std::cout << "数据库管理器初始化成功" << std::endl;
        
        // 测试CREATE DATABASE操作
        std::cout << "\n=== 测试CREATE DATABASE操作 ===" << std::endl;
        if (db_manager->CreateDatabase("test_db")) {
            std::cout << "创建数据库test_db成功" << std::endl;
        } else {
            std::cout << "创建数据库test_db失败" << std::endl;
        }
        
        // 测试USE DATABASE操作
        std::cout << "\n=== 测试USE DATABASE操作 ===" << std::endl;
        if (db_manager->UseDatabase("test_db")) {
            std::cout << "切换到数据库test_db成功" << std::endl;
        } else {
            std::cout << "切换到数据库test_db失败" << std::endl;
        }
        
        // 测试CREATE TABLE操作
        std::cout << "\n=== 测试CREATE TABLE操作 ===" << std::endl;
        std::vector<std::pair<std::string, std::string>> columns = {
            {"id", "INT PRIMARY KEY"},
            {"name", "VARCHAR(255) NOT NULL"},
            {"age", "INT"},
            {"salary", "DECIMAL(10,2)"}
        };
        
        if (db_manager->CreateTable("test_table", columns)) {
            std::cout << "创建表test_table成功" << std::endl;
        } else {
            std::cout << "创建表test_table失败" << std::endl;
        }
        
        // 测试系统数据库初始化
        std::cout << "\n=== 测试系统数据库初始化 ===" << std::endl;
        auto system_db = std::make_shared<SystemDatabase>(db_manager);
        if (system_db->Initialize()) {
            std::cout << "系统数据库初始化成功" << std::endl;
        } else {
            std::cout << "系统数据库初始化失败: " << system_db->GetLastError() << std::endl;
        }
        
        // 测试元数据注册
        std::cout << "\n=== 测试元数据注册 ===" << std::endl;
        if (system_db->CreateDatabaseRecord("test_db", "root", "测试数据库")) {
            std::cout << "注册数据库元数据成功" << std::endl;
        } else {
            std::cout << "注册数据库元数据失败: " << system_db->GetLastError() << std::endl;
        }
        
        // 测试表元数据注册
        auto db_record = system_db->GetDatabaseRecord("test_db");
        if (system_db->CreateTableRecord(db_record.db_id, "test_db", "test_table", "root")) {
            std::cout << "注册表元数据成功" << std::endl;
        } else {
            std::cout << "注册表元数据失败: " << system_db->GetLastError() << std::endl;
        }
        
        // 测试列元数据注册
        auto table_record = system_db->GetTableRecord("test_db", "test_table");
        bool columns_registered = true;
        for (size_t i = 0; i < columns.size(); ++i) {
            if (!system_db->CreateColumnRecord(table_record.table_id, columns[i].first, 
                                            columns[i].second, true, "", i + 1)) {
                columns_registered = false;
                std::cout << "注册列元数据失败: " << system_db->GetLastError() << std::endl;
                break;
            }
        }
        
        if (columns_registered) {
            std::cout << "注册列元数据成功" << std::endl;
        }
        
        // 测试DROP TABLE操作
        std::cout << "\n=== 测试DROP TABLE操作 ===" << std::endl;
        if (db_manager->DropTable("test_table")) {
            std::cout << "删除表test_table成功" << std::endl;
        } else {
            std::cout << "删除表test_table失败" << std::endl;
        }
        
        // 测试元数据清理
        std::cout << "\n=== 测试元数据清理 ===" << std::endl;
        if (system_db->DropTableRecord("test_db", "test_table")) {
            std::cout << "清理表元数据成功" << std::endl;
        } else {
            std::cout << "清理表元数据失败: " << system_db->GetLastError() << std::endl;
        }
        
        // 测试DROP DATABASE操作
        std::cout << "\n=== 测试DROP DATABASE操作 ===" << std::endl;
        if (db_manager->DropDatabase("test_db")) {
            std::cout << "删除数据库test_db成功" << std::endl;
        } else {
            std::cout << "删除数据库test_db失败" << std::endl;
        }
        
        // 测试元数据清理
        std::cout << "\n=== 测试数据库元数据清理 ===" << std::endl;
        if (system_db->DropDatabaseRecord("test_db")) {
            std::cout << "清理数据库元数据成功" << std::endl;
        } else {
            std::cout << "清理数据库元数据失败: " << system_db->GetLastError() << std::endl;
        }
        
        std::cout << "\n所有测试完成！" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "异常: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}