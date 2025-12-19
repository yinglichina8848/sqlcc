/**
 * @file test_smart_config_manager.cpp
 * @brief 智能配置管理器测试文件
 * 
 * Why: 测试智能配置管理器的内存安全、RAII模式和异常安全配置功能
 * What: 提供全面的测试用例，验证智能指针管理、RAII生命周期和异常安全
 * How: 使用Google Test框架或自定义测试框架进行单元测试
 */

#include "utils/smart_config_manager.h"
#include <iostream>
#include <cassert>
#include <thread>
#include <vector>
#include <chrono>

namespace sqlcc {
namespace test {

/**
 * @brief 简单的测试框架
 */
class TestFramework {
private:
    int test_count_ = 0;
    int passed_count_ = 0;
    int failed_count_ = 0;
    
public:
    void RunTest(const std::string& test_name, std::function<void()> test_func) {
        std::cout << "Running test: " << test_name << "... ";
        test_count_++;
        
        try {
            test_func();
            passed_count_++;
            std::cout << "PASSED" << std::endl;
        } catch (const std::exception& e) {
            failed_count_++;
            std::cout << "FAILED: " << e.what() << std::endl;
        } catch (...) {
            failed_count_++;
            std::cout << "FAILED: Unknown exception" << std::endl;
        }
    }
    
    void PrintSummary() {
        std::cout << "\nTest Summary:" << std::endl;
        std::cout << "  Total: " << test_count_ << std::endl;
        std::cout << "  Passed: " << passed_count_ << std::endl;
        std::cout << "  Failed: " << failed_count_ << std::endl;
        std::cout << "  Success Rate: " << (test_count_ > 0 ? (passed_count_ * 100.0 / test_count_) : 0) << "%" << std::endl;
    }
    
    bool AllPassed() const {
        return failed_count_ == 0;
    }
};

/**
 * @brief 测试智能配置管理器的基本功能
 */
void TestBasicFunctionality() {
    // 销毁之前的实例（如果有）
    SmartConfigManager::DestroyInstance();
    
    // 获取单例实例
    auto manager = SmartConfigManager::GetInstance();
    assert(manager != nullptr);
    
    // 初始化
    bool init_result = manager->Initialize();
    assert(init_result == true);
    
    // 测试基本配置访问
    std::string host = manager->GetStringConfig("database.host", "localhost");
    assert(host == "localhost");
    
    int port = manager->GetIntConfig("database.port", 5432);
    assert(port == 5432);
    
    bool ssl = manager->GetBoolConfig("database.ssl", true);
    assert(ssl == true);
    
    double timeout = manager->GetDoubleConfig("timeout", 30.0);
    assert(timeout == 30.0);
}

/**
 * @brief 测试智能指针和内存安全
 */
void TestMemorySafety() {
    // 销毁之前的实例
    SmartConfigManager::DestroyInstance();
    
    auto manager = SmartConfigManager::GetInstance();
    manager->Initialize();
    
    // 测试多个智能指针引用
    std::vector<std::shared_ptr<SmartConfigManager>> managers;
    for (int i = 0; i < 10; ++i) {
        managers.push_back(SmartConfigManager::GetInstance());
    }
    
    // 验证所有指针指向同一个实例
    for (size_t i = 1; i < managers.size(); ++i) {
        assert(managers[i] == managers[0]);
    }
    
    // 清理引用
    managers.clear();
    
    // 验证单例仍然存在
    auto manager2 = SmartConfigManager::GetInstance();
    assert(manager2 != nullptr);
    assert(manager2 == manager);
}

/**
 * @brief 测试RAII生命周期管理
 */
void TestRAIILifecycle() {
    // 销毁之前的实例
    SmartConfigManager::DestroyInstance();
    
    {
        auto manager = SmartConfigManager::GetInstance();
        manager->Initialize();
        
        // 在作用域内测试配置访问
        std::string value = manager->GetStringConfig("test.key", "default");
        assert(value == "default");
    }
    
    // 验证RAII清理后仍然可以获取实例
    auto manager2 = SmartConfigManager::GetInstance();
    assert(manager2 != nullptr);
}

/**
 * @brief 测试异常安全配置
 */
void TestExceptionSafety() {
    // 销毁之前的实例
    SmartConfigManager::DestroyInstance();
    
    auto manager = SmartConfigManager::GetInstance();
    
    // 测试未初始化时的访问（应该返回默认值）
    std::string value = manager->GetStringConfig("test.key", "default");
    assert(value == "default");
    
    // 初始化
    manager->Initialize();
    
    // 测试异常配置访问
    try {
        // 模拟异常情况
        std::string invalid_key = "";
        std::string result = manager->GetStringConfig(invalid_key, "fallback");
        assert(result == "fallback");
    } catch (const std::exception& e) {
        // 不应该抛出异常
        assert(false);
    }
}

/**
 * @brief 测试线程安全
 */
void TestThreadSafety() {
    // 销毁之前的实例
    SmartConfigManager::DestroyInstance();
    
    auto manager = SmartConfigManager::GetInstance();
    manager->Initialize();
    
    const int num_threads = 10;
    const int operations_per_thread = 100;
    std::vector<std::thread> threads;
    std::atomic<int> success_count{0};
    std::atomic<int> error_count{0};
    
    // 启动多个线程并发访问配置
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([manager, i, operations_per_thread, &success_count, &error_count]() {
            try {
                for (int j = 0; j < operations_per_thread; ++j) {
                    std::string key = "thread_" + std::to_string(i) + "_key_" + std::to_string(j);
                    std::string expected_value = "value_" + std::to_string(i) + "_" + std::to_string(j);
                    
                    // 更新配置
                    auto future = manager->UpdateConfigAsync(key, expected_value);
                    bool update_result = future.get();
                    
                    if (update_result) {
                        // 读取配置
                        std::string actual_value = manager->GetStringConfig(key, "default");
                        if (actual_value == expected_value) {
                            success_count++;
                        } else {
                            error_count++;
                        }
                    } else {
                        error_count++;
                    }
                }
            } catch (const std::exception& e) {
                error_count++;
            }
        });
    }
    
    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }
    
    // 验证结果
    assert(success_count > 0);  // 至少有一些操作应该成功
    std::cout << "Thread safety test: " << success_count << " successes, " 
              << error_count << " errors" << std::endl;
}

/**
 * @brief 测试热更新功能
 */
void TestHotReload() {
    // 销毁之前的实例
    SmartConfigManager::DestroyInstance();
    
    auto manager = SmartConfigManager::GetInstance();
    manager->Initialize();
    
    // 启用热更新
    bool hot_reload_result = manager->EnableHotReload(std::chrono::milliseconds(100));
    assert(hot_reload_result == true);
    
    // 更新配置
    std::string test_key = "hot_reload_test";
    std::string test_value = "initial_value";
    
    auto future = manager->UpdateConfigAsync(test_key, test_value);
    bool update_result = future.get();
    assert(update_result == true);
    
    // 验证初始值
    std::string initial_value = manager->GetStringConfig(test_key, "default");
    assert(initial_value == test_value);
    
    // 等待热更新周期
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // 停止热更新
    bool stop_result = manager->StopHotReload();
    assert(stop_result == true);
}

/**
 * @brief 测试版本管理
 */
void TestVersionManagement() {
    // 销毁之前的实例
    SmartConfigManager::DestroyInstance();
    
    auto manager = SmartConfigManager::GetInstance();
    manager->Initialize();
    
    // 获取初始版本
    std::string initial_version = manager->GetCurrentVersionId();
    assert(!initial_version.empty());
    
    // 更新配置
    std::string test_key = "version_test";
    std::string test_value = "version_value";
    
    auto future = manager->UpdateConfigAsync(test_key, test_value);
    bool update_result = future.get();
    assert(update_result == true);
    
    // 获取新版本
    std::string new_version = manager->GetCurrentVersionId();
    assert(!new_version.empty());
    assert(new_version != initial_version);
    
    std::cout << "Version management test: initial=" << initial_version 
              << ", new=" << new_version << std::endl;
}

/**
 * @brief 测试批量更新
 */
void TestBatchUpdate() {
    // 销毁之前的实例
    SmartConfigManager::DestroyInstance();
    
    auto manager = SmartConfigManager::GetInstance();
    manager->Initialize();
    
    // 准备批量配置
    std::unordered_map<std::string, ConfigValue> batch_configs;
    for (int i = 0; i < 10; ++i) {
        std::string key = "batch_key_" + std::to_string(i);
        std::string value = "batch_value_" + std::to_string(i);
        batch_configs[key] = value;
    }
    
    // 执行批量更新
    auto future = manager->BatchUpdateConfigsAsync(batch_configs);
    bool batch_result = future.get();
    assert(batch_result == true);
    
    // 验证批量更新结果
    for (int i = 0; i < 10; ++i) {
        std::string key = "batch_key_" + std::to_string(i);
        std::string expected_value = "batch_value_" + std::to_string(i);
        std::string actual_value = manager->GetStringConfig(key, "default");
        assert(actual_value == expected_value);
    }
}

/**
 * @brief 测试统计信息
 */
void TestStatistics() {
    // 销毁之前的实例
    SmartConfigManager::DestroyInstance();
    
    auto manager = SmartConfigManager::GetInstance();
    manager->Initialize();
    
    // 执行一些操作
    for (int i = 0; i < 5; ++i) {
        manager->GetStringConfig("stat_test_" + std::to_string(i), "default");
    }
    
    for (int i = 0; i < 3; ++i) {
        auto future = manager->UpdateConfigAsync("stat_update_" + std::to_string(i), "value");
        future.get();
    }
    
    // 获取统计信息
    std::string stats = manager->GetStatistics();
    assert(!stats.empty());
    
    std::cout << "Statistics test output:" << std::endl;
    std::cout << stats << std::endl;
}

/**
 * @brief 测试加密支持
 */
void TestEncryptionSupport() {
    // 销毁之前的实例
    SmartConfigManager::DestroyInstance();
    
    auto manager = SmartConfigManager::GetInstance();
    
    // 设置加密密钥
    std::string encryption_key = "test_encryption_key_12345";
    manager->SetEncryptionKey(encryption_key);
    
    // 验证加密密钥
    std::string retrieved_key = manager->GetEncryptionKey();
    assert(retrieved_key == encryption_key);
    
    // 初始化
    manager->Initialize();
    
    // 验证加密功能在运行时也有效
    std::string runtime_key = manager->GetEncryptionKey();
    assert(runtime_key == encryption_key);
}

/**
 * @brief 运行所有测试
 */
void RunAllTests() {
    TestFramework framework;
    
    std::cout << "=== Smart Config Manager Test Suite ===" << std::endl;
    
    // 基本功能测试
    framework.RunTest("Basic Functionality", TestBasicFunctionality);
    framework.RunTest("Memory Safety", TestMemorySafety);
    framework.RunTest("RAII Lifecycle", TestRAIILifecycle);
    framework.RunTest("Exception Safety", TestExceptionSafety);
    
    // 高级功能测试
    framework.RunTest("Thread Safety", TestThreadSafety);
    framework.RunTest("Hot Reload", TestHotReload);
    framework.RunTest("Version Management", TestVersionManagement);
    framework.RunTest("Batch Update", TestBatchUpdate);
    
    // 管理功能测试
    framework.RunTest("Statistics", TestStatistics);
    framework.RunTest("Encryption Support", TestEncryptionSupport);
    
    // 打印测试总结
    framework.PrintSummary();
    
    // 清理
    SmartConfigManager::DestroyInstance();
    
    if (framework.AllPassed()) {
        std::cout << "\n✅ All tests passed! Smart Config Manager is working correctly." << std::endl;
    } else {
        std::cout << "\n❌ Some tests failed. Please check the implementation." << std::endl;
    }
}

}  // namespace test
}  // namespace sqlcc

/**
 * @brief 主函数 - 运行测试
 */
int main() {
    try {
        sqlcc::test::RunAllTests();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test suite failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Test suite failed with unknown exception" << std::endl;
        return 1;
    }
}