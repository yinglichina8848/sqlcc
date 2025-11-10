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
<<<<<<< Updated upstream

// Why: 需要获取数据库系统的版本信息
// What: 包含version.h头文件，提供SQLCC_VERSION宏定义
// How: 使用#include预处理指令包含项目头文件
=======
#include <memory>
>>>>>>> Stashed changes
#include "version.h"
#include "config_manager.h"
#include "storage_engine.h"
#include "exception.h"

using sqlcc::ConfigManager;
using sqlcc::StorageEngine;

// Why: 需要使用配置管理器来加载和管理配置
// What: 包含config_manager.h头文件，提供ConfigManager类
// How: 使用#include预处理指令包含项目头文件
#include "config_manager.h"

// Why: 需要使用存储引擎来管理数据库操作
// What: 包含storage_engine.h头文件，提供StorageEngine类
// How: 使用#include预处理指令包含项目头文件
#include "storage_engine.h"

// Why: 需要使用异常处理来捕获和处理错误
// What: 包含exception.h头文件，提供异常类
// How: 使用#include预处理指令包含项目头文件
#include "exception.h"

// Why: 需要使用sqlcc命名空间中的类
// What: 使用using声明引入sqlcc命名空间中的ConfigManager和StorageEngine类
// How: 使用using声明简化代码
using sqlcc::ConfigManager;
using sqlcc::StorageEngine;

/**
 * @brief 主函数入口
 * 
<<<<<<< Updated upstream
 * Why: 需要一个程序入口点来启动数据库系统
 * What: 主函数负责初始化配置管理器，加载配置文件，创建存储引擎实例，然后正常退出
 * How: 创建配置管理器实例，加载配置文件，创建存储引擎，捕获并处理可能的异常
 * 
 * @return int 程序退出状态码，0表示正常退出
 */
int main() {
    try {
        // Why: 需要显示数据库系统的版本信息，让用户了解当前运行的版本
        // What: 使用std::cout输出数据库系统名称和版本号
        // How: 使用流插入运算符<<输出字符串和SQLCC_VERSION宏
        std::cout << "SqlCC " << SQLCC_VERSION << " startup!" << std::endl;
        
        // 创建配置管理器实例
        // Why: 需要一个配置管理器实例来加载和管理配置
        // What: 创建ConfigManager单例实例
        // How: 使用ConfigManager::GetInstance()方法获取单例实例
        ConfigManager& config_manager = ConfigManager::GetInstance();
        
        // 加载配置文件
        // Why: 需要从配置文件加载系统配置参数
        // What: 调用LoadConfig方法加载配置文件
        // How: 传入配置文件路径，处理可能的加载错误
=======
 * 当前版本初始化配置管理器和存储引擎，后续将添加更多功能。
 */
int main() {
    try {
        // 输出版本信息
        std::cout << "SqlCC " << SQLCC_VERSION << " startup!" << std::endl;
        
        // 创建配置管理器实例
        ConfigManager& config_manager = ConfigManager::GetInstance();
        
        // 加载配置文件
>>>>>>> Stashed changes
        if (!config_manager.LoadConfig("./config/sqlcc.conf")) {
            std::cerr << "Warning: Failed to load config file, using default settings" << std::endl;
        }
        
        // 创建存储引擎实例
<<<<<<< Updated upstream
        // Why: 需要一个存储引擎实例来管理数据库操作
        // What: 创建StorageEngine实例，传入配置管理器引用
        // How: 使用new关键字创建StorageEngine实例
        std::unique_ptr<StorageEngine> storage_engine = std::make_unique<StorageEngine>(config_manager);
        
        // 输出配置信息
        // Why: 需要显示当前配置信息，便于调试和监控
        // What: 输出数据库文件路径和缓冲池大小
        // How: 使用配置管理器的GetString和GetInt方法获取配置值
=======
        std::unique_ptr<StorageEngine> storage_engine = std::make_unique<StorageEngine>(config_manager);
        
        // 输出配置信息
>>>>>>> Stashed changes
        std::string db_path = config_manager.GetString("database.db_file_path");
        int pool_size = config_manager.GetInt("buffer_pool.pool_size");
        std::cout << "Database file: " << db_path << std::endl;
        std::cout << "Buffer pool size: " << pool_size << " pages" << std::endl;
        
        // 保存配置文件
<<<<<<< Updated upstream
        // Why: 需要保存当前配置，以便下次启动时使用
        // What: 调用SaveToFile方法保存配置文件
        // How: 传入配置文件路径，处理可能的保存错误
=======
>>>>>>> Stashed changes
        if (!config_manager.SaveToFile("./config/sqlcc.conf")) {
            std::cerr << "Warning: Failed to save config file" << std::endl;
        }
        
        std::cout << "SqlCC initialized successfully!" << std::endl;
    }
    catch (const std::exception& e) {
        // 捕获并处理异常
<<<<<<< Updated upstream
        // Why: 需要捕获并处理可能发生的异常，提供友好的错误信息
        // What: 捕获标准异常，输出错误信息
        // How: 使用catch块捕获异常，使用std::cerr输出错误信息
=======
>>>>>>> Stashed changes
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    // Why: 需要返回一个状态码表示程序正常结束
    // What: 返回0表示程序成功执行完毕
    // How: 使用return语句返回0
    return 0;
}

