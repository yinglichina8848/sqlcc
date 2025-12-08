#include <iostream>
#include <memory>
#include <vector>
#include <utility>
#include "database_manager.h"

using namespace sqlcc;

int main() {
    std::cout << "=== CREATE TABLE功能简化测试 ===" << std::endl;
    
    try {
        // 创建DatabaseManager
        auto db_manager = std::make_shared<DatabaseManager>("./test_data");
        
        // 创建测试数据库
        if (!db_manager->CreateDatabase("test_db")) {
            std::cout << "创建数据库失败" << std::endl;
            return 1;
        }
        
        // 使用数据库
        if (!db_manager->UseDatabase("test_db")) {
            std::cout << "使用数据库失败" << std::endl;
            return 1;
        }
        
        std::cout << "✅ 数据库创建成功" << std::endl;
        
        // 准备列定义（使用DatabaseManager期望的格式）
        std::vector<std::pair<std::string, std::string>> columns = {
            {"id", "INT"},
            {"name", "VARCHAR(50)"},
            {"age", "INT"}
        };
        
        std::cout << "列定义准备完成" << std::endl;
        
        // 直接调用DatabaseManager的CreateTable方法
        bool success = db_manager->CreateTable("test_table", columns);
        
        if (success) {
            std::cout << "✅ CREATE TABLE执行成功" << std::endl;
            
            // 验证表是否真的创建了
            if (db_manager->TableExists("test_table")) {
                std::cout << "✅ 表验证成功: test_table确实存在" << std::endl;
            } else {
                std::cout << "❌ 表验证失败: test_table不存在" << std::endl;
            }
            
            // 列出所有表
            auto tables = db_manager->ListTables();
            std::cout << "数据库中的表: " << std::endl;
            for (const auto& table : tables) {
                std::cout << "  - " << table << std::endl;
            }
            
        } else {
            std::cout << "❌ CREATE TABLE执行失败" << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cout << "❌ 发生异常: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "=== 测试完成 ===" << std::endl;
    return 0;
}