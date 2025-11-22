/**
 * @file main.cc
 * @brief 数据库系统主程序入口
 * 
 * Why: 需要一个程序入口点来启动数据库系统，初始化配置管理器和存储引擎
 * What: 主程序负责初始化配置管理器，加载配置文件，创建存储引擎实例，然后正常退出
 * How: 包含必要的头文件，创建配置管理器实例，加载配置文件，创建存储引擎，然后返回0
 */

// Why: 需要使用标准输入输出流来显示版本信息
// What: 包含iostream头文件，提供std::cout功能
// How: 使用#include预处理指令包含标准库头文件
#include <iostream>
#include <memory>
#include "version.h"
#include "config_manager.h"
#include "storage_engine.h"
#include "exception.h"

// Why: 需要使用sqlcc命名空间中的类
// What: 使用using声明引入sqlcc命名空间中的ConfigManager和StorageEngine类
// How: 使用using声明简化代码
using sqlcc::ConfigManager;
using sqlcc::StorageEngine;

/**
 * @brief 主函数入口
 * 
 * 当前版本初始化配置管理器和存储引擎，后续将添加更多功能。
 */
int main() {
    try {
        // 输出版本信息
        std::cout << "SqlCC " << SQLCC_VERSION << " startup!" << std::endl;
        
        // 创建配置管理器实例
        ConfigManager& config_manager = ConfigManager::GetInstance();
        
        // 加载配置文件
        if (!config_manager.LoadConfig("./config/sqlcc.conf")) {
            std::cerr << "Warning: Failed to load config file, using default settings" << std::endl;
        }
        
        // 创建存储引擎实例
        std::unique_ptr<StorageEngine> storage_engine = std::make_unique<StorageEngine>(config_manager);
        
        // 输出配置信息
        std::string db_path = config_manager.GetString("database.db_file_path");
        int pool_size = config_manager.GetInt("buffer_pool.pool_size");
        std::cout << "Database file: " << db_path << std::endl;
        std::cout << "Buffer pool size: " << pool_size << " pages" << std::endl;
        
        // 保存配置文件
        if (!config_manager.SaveToFile("./config/sqlcc.conf")) {
            std::cerr << "Warning: Failed to save config file" << std::endl;
        }
        
        std::cout << "SqlCC initialized successfully!" << std::endl;
    }
    catch (const std::exception& e) {
        // 捕获并处理异常
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}

