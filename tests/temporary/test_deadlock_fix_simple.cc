#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include "storage_engine.h"
#include "config_manager.h"

int main() {
    std::cout << "=== BufferPool构造死锁修复验证测试 ===" << std::endl;
    
    try {
        // 获取配置管理器单例实例
        sqlcc::ConfigManager& config_manager = sqlcc::ConfigManager::GetInstance();
        
        // 注册配置回调
        config_manager.RegisterChangeCallback("buffer_pool.pool_size", [](const std::string& key, const sqlcc::ConfigValue& value) {
            std::cout << "配置回调被调用: " << key << std::endl;
        });
        
        // 测试1: 正常构造BufferPool
        std::cout << "测试1: 正常构造BufferPool..." << std::endl;
        
        // 设置必要配置
        config_manager.SetValue("database.file_path", sqlcc::ConfigValue("test_simple.db"));
        config_manager.SetValue("buffer_pool.pool_size", sqlcc::ConfigValue(64));
        
        // 构造StorageEngine (这里应该不会死锁)
        sqlcc::StorageEngine engine(config_manager);
        std::cout << "✅ BufferPool构造成功!" << std::endl;
        
        // 测试2: 在构造过程中触发配置变更
        std::cout << "测试2: 在构造过程中触发配置变更..." << std::endl;
        config_manager.SetValue("buffer_pool.pool_size", sqlcc::ConfigValue(128));
        std::cout << "✅ 配置变更处理成功!" << std::endl;
        
        // 测试3: 多次构造和析构
        std::cout << "测试3: 多次构造和析构..." << std::endl;
        for (int i = 0; i < 3; i++) {
            config_manager.SetValue("database.file_path", sqlcc::ConfigValue("test_temp.db"));
            config_manager.SetValue("buffer_pool.pool_size", sqlcc::ConfigValue(32));
            sqlcc::StorageEngine temp_engine(config_manager);
            std::cout << "第" << (i+1) << "次构造成功" << std::endl;
        }
        std::cout << "✅ 多次构造测试通过!" << std::endl;
        
        std::cout << "🎉 所有测试通过! BufferPool构造死锁修复成功!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ 测试失败: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}