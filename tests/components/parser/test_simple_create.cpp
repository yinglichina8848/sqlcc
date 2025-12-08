#include <iostream>
#include <memory>
#include <vector>
#include <utility>
#include <filesystem>
#include "database_manager.h"

using namespace sqlcc;
namespace fs = std::filesystem;

int main() {
    std::cout << "=== 简单CREATE TABLE测试 ===" << std::endl;
    
    try {
        // 创建独立的测试目录
        std::string test_dir = "./test_create_table_data";
        if (fs::exists(test_dir)) {
            fs::remove_all(test_dir);
        }
        fs::create_directories(test_dir);
        
        std::cout << "测试目录创建: " << test_dir << std::endl;
        
        // 创建DatabaseManager实例
        auto db_manager = std::make_shared<DatabaseManager>(test_dir);
        
        std::cout << "DatabaseManager创建成功" << std::endl;
        
        // 测试CREATE DATABASE
        std::cout << "\n执行CREATE DATABASE test_db..." << std::endl;
        bool db_created = db_manager->CreateDatabase("test_db");
        
        if (db_created) {
            std::cout << "✅ CREATE DATABASE成功" << std::endl;
        } else {
            std::cout << "❌ CREATE DATABASE失败" << std::endl;
            return 1;
        }
        
        // 测试USE DATABASE
        std::cout << "执行USE test_db..." << std::endl;
        bool db_used = db_manager->UseDatabase("test_db");
        
        if (db_used) {
            std::cout << "✅ USE DATABASE成功" << std::endl;
        } else {
            std::cout << "❌ USE DATABASE失败" << std::endl;
            return 1;
        }
        
        // 测试CREATE TABLE - 这是我们修复的关键
        std::cout << "\n执行CREATE TABLE test_table (id INT, name VARCHAR(50), age INT)..." << std::endl;
        
        // 准备列定义（使用DatabaseManager期望的格式）
        std::vector<std::pair<std::string, std::string>> columns = {
            {"id", "INT"},
            {"name", "VARCHAR(50)"},
            {"age", "INT"}
        };
        
        bool table_created = db_manager->CreateTable("test_table", columns);
        
        if (table_created) {
            std::cout << "✅ CREATE TABLE成功" << std::endl;
            
            // 验证表是否真的创建了
            std::cout << "验证表是否存在..." << std::endl;
            bool table_exists = db_manager->TableExists("test_table");
            
            if (table_exists) {
                std::cout << "✅ 表验证成功: test_table确实存在" << std::endl;
                
                // 列出所有表
                auto tables = db_manager->ListTables();
                std::cout << "数据库中的表: " << std::endl;
                for (const auto& table : tables) {
                    std::cout << "  - " << table << std::endl;
                }
                
            } else {
                std::cout << "❌ 表验证失败: test_table不存在" << std::endl;
            }
            
        } else {
            std::cout << "❌ CREATE TABLE失败" << std::endl;
            return 1;
        }
        
        std::cout << "\n=== 测试完成 ===" << std::endl;
        
        // 清理测试目录
        if (fs::exists(test_dir)) {
            fs::remove_all(test_dir);
        }
        
    } catch (const std::exception& e) {
        std::cout << "❌ 发生异常: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}