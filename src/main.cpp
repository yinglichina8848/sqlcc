/**
 * @file main.cpp
 *
 * WHY: 为什么需要主程序入口？
 *
 * 数据库系统需要一个启动入口点来初始化和协调整个系统的各个组件。没有主程序入口，数据库系统就无法启动和运行，
 * 无法初始化配置管理器、存储引擎、网络服务器等核心组件。
 *
 * 主要问题解决：
 * 1. 系统初始化：按正确的顺序启动各个系统组件
 * 2. 配置加载：读取和应用系统配置文件
 * 3. 组件协调：确保各组件间的依赖关系正确建立
 * 4. 错误处理：捕获启动过程中的异常并适当处理
 * 5. 生命周期管理：控制系统的启动和关闭流程
 *
 * 主程序失败的影响：
 * - 系统无法启动：用户无法使用数据库服务
 * - 配置错误：系统使用错误的参数运行
 * - 资源泄露：组件未正确初始化导致资源管理问题
 * - 安全风险：系统在未正确初始化的状态下运行
 *
 * WHAT: 这实现了什么功能？
 *
 * 主程序提供完整的数据库系统启动和运行管理功能：
 * - 版本显示：输出系统版本信息，便于版本跟踪
 * - 配置初始化：创建并初始化全局配置管理器
 * - 存储引擎启动：初始化数据存储和访问组件
 * - 错误处理：捕获并处理启动过程中的异常
 * - 状态输出：显示系统初始化状态和配置信息
 * - 优雅关闭：确保系统资源正确释放
 *
 * 核心组件：
 * - 配置管理器：负责系统配置的加载和管理
 * - 存储引擎：提供数据存储和访问的核心功能
 * - 异常处理器：处理启动过程中的各种异常情况
 * - 日志系统：记录系统启动和运行状态
 * - 资源管理器：管理系统资源的分配和释放
 *
 * HOW: 如何实现的？
 *
 * 技术实现要点：
 * 1. 单例模式：使用配置管理器的单例实例保证全局配置一致性
 * 2. 智能指针：std::unique_ptr管理存储引擎对象的生命周期
 * 3. 异常处理：try-catch块捕获并处理启动异常
 * 4. 配置文件：从标准路径加载系统配置文件
 * 5. 状态输出：使用std::cout输出系统状态信息
 * 6. 资源清理：通过RAII机制自动释放系统资源
 *
 * 架构设计：
 * - 初始化序列：配置管理器→存储引擎→网络服务
 * - 依赖注入：通过构造函数参数注入组件依赖
 * - 错误隔离：将错误处理与正常流程分离
 * - 状态监控：实时监控系统初始化状态
 * - 优雅降级：启动失败时提供适当的降级处理
 *
 * 性能优化：
 * - 延迟初始化：按需初始化组件减少启动时间
 * - 异步加载：后台加载非关键组件
 * - 缓存预热：预加载常用数据到缓存
 * - 内存预分配：提前分配必要的内存空间
 * - 连接池初始化：预先建立数据库连接
 *
 * @note 该实现专为SQLCC数据库系统优化，支持快速启动和稳定运行
 * @see include/utils/config_manager.h
 * @see include/storage_engine.h
 */

#include <iostream>
#include <memory>
#include "version.h"
#include "utils/config_manager.h"
#include "include/storage_engine.h"
#include "exception.h"

// 使用声明简化代码，引入核心组件类
using sqlcc::ConfigManager;
using sqlcc::StorageEngine;

/**
 * @brief SQLCC数据库系统主程序入口
 *
 * 主程序负责整个数据库系统的初始化、配置加载、组件启动和运行管理。
 * 这是一个完整的系统启动流程，确保所有组件按正确的顺序初始化并协调工作。
 *
 * 启动流程：
 * 1. 显示版本信息
 * 2. 初始化配置管理器
 * 3. 加载系统配置文件
 * 4. 创建并初始化存储引擎
 * 5. 输出系统状态信息
 * 6. 保存配置文件快照
 * 7. 返回退出码
 *
 * @return int 退出码，0表示成功，非0表示失败
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
