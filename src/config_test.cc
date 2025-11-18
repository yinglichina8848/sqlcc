/**
 * @file config_test.cc
 * @brief 配置管理器测试程序
 * 
 * Why: 需要一个测试程序来验证配置管理器的功能是否正常工作
 * What: 测试程序负责测试配置管理器的加载、获取、设置和保存功能
 * How: 创建配置管理器实例，加载配置文件，测试各种配置操作
 */

#include <iostream>
#include <string>
#include "config_manager.h"
#include "exception.h"

using sqlcc::ConfigValue;
using sqlcc::ConfigManager;

/**
 * @brief 测试配置管理器的基本功能
 * 
 * Why: 需要测试配置管理器的基本功能是否正常工作
 * What: 测试配置加载、获取、设置和保存功能
 * How: 创建配置管理器实例，执行各种配置操作，验证结果
 */
void TestConfigManager() {
    std::cout << "=== Testing ConfigManager ===" << std::endl;
    
    try {
        // 创建配置管理器实例
        ConfigManager& config = ConfigManager::GetInstance();
        
        // 测试加载配置文件
        std::cout << "1. Testing LoadConfig..." << std::endl;
        if (config.LoadConfig("./config/sqlcc.conf")) {
            std::cout << "   Config file loaded successfully!" << std::endl;
        } else {
            std::cout << "   Warning: Failed to load config file, using default settings" << std::endl;
        }
        
        // 测试获取配置值
        std::cout << "2. Testing Get methods..." << std::endl;
        
        // 测试字符串配置
        std::string db_path = config.GetString("database.db_file_path", "./default.db");
        std::cout << "   Database file path: " << db_path << std::endl;
        
        // 测试整数配置
        int pool_size = config.GetInt("buffer_pool.pool_size", 64);
        std::cout << "   Buffer pool size: " << pool_size << std::endl;
        
        // 测试布尔配置
        bool enable_prefetch = config.GetBool("buffer_pool.enable_prefetch", true);
        std::cout << "   Enable prefetch: " << (enable_prefetch ? "true" : "false") << std::endl;
        
        // 测试双精度配置
        double prefetch_threshold = config.GetDouble("buffer_pool.prefetch_threshold", 0.8);
        std::cout << "   Prefetch threshold: " << prefetch_threshold << std::endl;
        
        // 测试设置配置值
        std::cout << "3. Testing SetValue..." << std::endl;
        
        // 设置字符串配置
        config.SetValue("database.db_file_path", std::string("./test.db"));
        std::cout << "   Set database file path to: ./test.db" << std::endl;
        
        // 设置整数配置
        config.SetValue("buffer_pool.pool_size", 128);
        std::cout << "   Set buffer pool size to: 128" << std::endl;
        
        // 设置布尔配置
        config.SetValue("buffer_pool.enable_prefetch", false);
        std::cout << "   Set enable prefetch to: false" << std::endl;
        
        // 设置双精度配置
        config.SetValue("buffer_pool.prefetch_threshold", 0.9);
        std::cout << "   Set prefetch threshold to: 0.9" << std::endl;
        
        // 验证设置后的值
        std::cout << "4. Verifying set values..." << std::endl;
        
        std::string new_db_path = config.GetString("database.db_file_path", "./default.db");
        std::cout << "   New database file path: " << new_db_path << std::endl;
        
        int new_pool_size = config.GetInt("buffer_pool.pool_size", 64);
        std::cout << "   New buffer pool size: " << new_pool_size << std::endl;
        
        bool new_enable_prefetch = config.GetBool("buffer_pool.enable_prefetch", true);
        std::cout << "   New enable prefetch: " << (new_enable_prefetch ? "true" : "false") << std::endl;
        
        double new_prefetch_threshold = config.GetDouble("buffer_pool.prefetch_threshold", 0.8);
        std::cout << "   New prefetch threshold: " << new_prefetch_threshold << std::endl;
        
        // 测试保存配置文件
        std::cout << "5. Testing SaveToFile..." << std::endl;
        if (config.SaveToFile("./config/sqlcc_test.conf")) {
            std::cout << "   Config file saved successfully!" << std::endl;
        } else {
            std::cout << "   Failed to save config file!" << std::endl;
        }
        
        // 配置变更回调功能已被移除
        std::cout << "6. Config change callbacks feature has been removed" << std::endl;
        std::cout << "   ✓ Config change notification is no longer supported" << std::endl;
        
        std::cout << "=== ConfigManager test completed ===" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error in ConfigManager test: " << e.what() << std::endl;
    }
}

/**
 * @brief 主函数入口
 * 
 * Why: 需要一个程序入口点来运行配置管理器测试
 * What: 主函数负责调用测试函数
 * How: 直接调用TestConfigManager函数
 * 
 * @return int 程序退出状态码，0表示正常退出
 */
int main() {
    std::cout << "SqlCC ConfigManager Test Program" << std::endl;
    std::cout << "=================================" << std::endl;
    
    TestConfigManager();
    
    return 0;
}