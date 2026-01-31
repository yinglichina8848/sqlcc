#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <future>
#include <thread>
#include <atomic>

#include "src/utils/smart_config_manager.h"
#include "src/utils/config_lifecycle.h"
#include "src/utils/config_snapshot.h"

namespace sqlcc {
namespace test {

// ==================== SmartConfigManager Tests ====================

class SmartConfigManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 确保单例是干净的
        if (SmartConfigManager::GetInstance()) {
            SmartConfigManager::DestroyInstance();
        }
    }

    void TearDown() override {
        // 清理单例
        SmartConfigManager::DestroyInstance();
    }
};

// 测试单例模式
TEST_F(SmartConfigManagerTest, SingletonPattern) {
    SmartConfigManager* instance1 = SmartConfigManager::GetInstance();
    SmartConfigManager* instance2 = SmartConfigManager::GetInstance();
    
    EXPECT_EQ(instance1, instance2);
    
    // 清理
    SmartConfigManager::DestroyInstance();
}

// 测试初始化和关闭
TEST_F(SmartConfigManagerTest, InitializeAndShutdown) {
    SmartConfigManager* manager = SmartConfigManager::GetInstance();
    
    // 初始化
    bool init_result = manager->Initialize("");
    EXPECT_TRUE(init_result);
    
    // 关闭
    bool shutdown_result = manager->Shutdown();
    EXPECT_TRUE(shutdown_result);
    
    SmartConfigManager::DestroyInstance();
}

// 测试重复初始化
TEST_F(SmartConfigManagerTest, DoubleInitialize) {
    SmartConfigManager* manager = SmartConfigManager::GetInstance();
    
    bool first_init = manager->Initialize("");
    EXPECT_TRUE(first_init);
    
    // 重复初始化应该返回 false
    bool second_init = manager->Initialize("");
    EXPECT_FALSE(second_init);
    
    manager->Shutdown();
    SmartConfigManager::DestroyInstance();
}

// 测试获取版本ID（空配置）
TEST_F(SmartConfigManagerTest, GetCurrentVersionId) {
    SmartConfigManager* manager = SmartConfigManager::GetInstance();
    manager->Initialize("");
    
    std::string version_id = manager->GetCurrentVersionId();
    // 初始化后会创建默认快照
    EXPECT_FALSE(version_id.empty());
    
    manager->Shutdown();
    SmartConfigManager::DestroyInstance();
}

// 测试获取统计信息
TEST_F(SmartConfigManagerTest, GetStatistics) {
    SmartConfigManager* manager = SmartConfigManager::GetInstance();
    manager->Initialize("");
    
    std::string stats = manager->GetStatistics();
    
    // 统计信息应该包含关键指标
    EXPECT_TRUE(stats.find("SmartConfigManager Statistics:") != std::string::npos);
    EXPECT_TRUE(stats.find("Initialized:") != std::string::npos);
    EXPECT_TRUE(stats.find("Config Reads:") != std::string::npos);
    
    manager->Shutdown();
    SmartConfigManager::DestroyInstance();
}

// 测试加密密钥设置和获取
TEST_F(SmartConfigManagerTest, EncryptionKey) {
    SmartConfigManager* manager = SmartConfigManager::GetInstance();
    manager->Initialize("");
    
    std::string test_key = "test_encryption_key_12345";
    
    manager->SetEncryptionKey(test_key);
    std::string retrieved_key = manager->GetEncryptionKey();
    
    EXPECT_EQ(retrieved_key, test_key);
    
    manager->Shutdown();
    SmartConfigManager::DestroyInstance();
}

// 测试热更新启用和停止
TEST_F(SmartConfigManagerTest, HotReloadEnableDisable) {
    SmartConfigManager* manager = SmartConfigManager::GetInstance();
    manager->Initialize("");
    
    // 启用热更新
    bool enable_result = manager->EnableHotReload(std::chrono::milliseconds(1000));
    EXPECT_TRUE(enable_result);
    
    // 停止热更新
    bool disable_result = manager->StopHotReload();
    EXPECT_TRUE(disable_result);
    
    manager->Shutdown();
    SmartConfigManager::DestroyInstance();
}

// 测试重复启用热更新
TEST_F(SmartConfigManagerTest, DoubleEnableHotReload) {
    SmartConfigManager* manager = SmartConfigManager::GetInstance();
    manager->Initialize("");
    
    bool first_enable = manager->EnableHotReload(std::chrono::milliseconds(1000));
    EXPECT_TRUE(first_enable);
    
    // 重复启用应该返回 false
    bool second_enable = manager->EnableHotReload(std::chrono::milliseconds(2000));
    EXPECT_FALSE(second_enable);
    
    manager->StopHotReload();
    manager->Shutdown();
    SmartConfigManager::DestroyInstance();
}

// 测试异步配置更新
TEST_F(SmartConfigManagerTest, AsyncBatchUpdate) {
    SmartConfigManager* manager = SmartConfigManager::GetInstance();
    manager->Initialize("");
    
    std::unordered_map<std::string, ConfigValue> configs;
    configs["test.key1"] = std::string("value1");
    configs["test.key2"] = 42;
    configs["test.key3"] = true;
    
    auto future = manager->BatchUpdateConfigsAsync(configs);
    
    // 等待异步操作完成
    bool result = future.get();
    EXPECT_TRUE(result);
    
    manager->Shutdown();
    SmartConfigManager::DestroyInstance();
}

// 测试并发访问
TEST_F(SmartConfigManagerTest, ConcurrentAccess) {
    SmartConfigManager* manager = SmartConfigManager::GetInstance();
    manager->Initialize("");
    
    std::atomic<bool> done{false};
    std::vector<std::thread> threads;
    
    // 多个线程同时获取统计信息
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back([manager, &done]() {
            while (!done.load()) {
                manager->GetStatistics();
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        });
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    done.store(true);
    
    for (auto& t : threads) {
        t.join();
    }
    
    // 不应该崩溃
    SUCCEED();
    
    manager->Shutdown();
    SmartConfigManager::DestroyInstance();
}

// 测试空配置获取
TEST_F(SmartConfigManagerTest, GetConfigWithDefaults) {
    SmartConfigManager* manager = SmartConfigManager::GetInstance();
    manager->Initialize("");
    
    // 获取不存在的配置应该返回默认值
    std::string str_val = manager->GetStringConfig("nonexistent", "default");
    EXPECT_EQ(str_val, "default");
    
    int int_val = manager->GetIntConfig("nonexistent", 123);
    EXPECT_EQ(int_val, 123);
    
    bool bool_val = manager->GetBoolConfig("nonexistent", true);
    EXPECT_TRUE(bool_val);
    
    double double_val = manager->GetDoubleConfig("nonexistent", 3.14);
    EXPECT_DOUBLE_EQ(double_val, 3.14);
    
    manager->Shutdown();
    SmartConfigManager::DestroyInstance();
}

// ==================== ConfigLifecycleManager Tests ====================

class ConfigLifecycleManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
    }

    void TearDown() override {
    }
};

TEST_F(ConfigLifecycleManagerTest, InitializeAndShutdown) {
    ConfigLifecycleManager manager;
    
    bool init_result = manager.Initialize("");
    EXPECT_TRUE(init_result);
    
    EXPECT_TRUE(manager.IsReady());
    EXPECT_EQ(manager.GetState(), ConfigLifecycleState::READY);
    
    bool shutdown_result = manager.Shutdown();
    EXPECT_TRUE(shutdown_result);
    
    EXPECT_EQ(manager.GetState(), ConfigLifecycleState::SHUTDOWN);
}

TEST_F(ConfigLifecycleManagerTest, GetCurrentSnapshot) {
    ConfigLifecycleManager manager;
    manager.Initialize("");
    
    auto snapshot = manager.GetCurrentSnapshot();
    EXPECT_NE(snapshot, nullptr);
    
    manager.Shutdown();
}

TEST_F(ConfigLifecycleManagerTest, GetStatistics) {
    ConfigLifecycleManager manager;
    manager.Initialize("");
    
    std::string stats = manager.GetStatistics();
    
    EXPECT_TRUE(stats.find("Config Lifecycle Statistics:") != std::string::npos);
    
    manager.Shutdown();
}

TEST_F(ConfigLifecycleManagerTest, Callbacks) {
    ConfigLifecycleManager manager;
    
    bool init_called = false;
    bool shutdown_called = false;
    bool config_change_called = false;
    
    manager.SetInitializeCallback([&init_called]() {
        init_called = true;
    });
    
    manager.SetShutdownCallback([&shutdown_called]() {
        shutdown_called = true;
    });
    
    manager.SetConfigChangeCallback([&config_change_called](const std::string&) {
        config_change_called = true;
    });
    
    manager.Initialize("");
    EXPECT_TRUE(init_called);
    
    manager.Shutdown();
    EXPECT_TRUE(shutdown_called);
}

} // namespace test
} // namespace sqlcc
