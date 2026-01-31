/**
 * @file config_test.cpp
 * @brief Config模块完整单元测试
 *
 * 测试覆盖：
 * 1. ConfigManager单例模式
 * 2. 配置值的设置和获取（String/Int/Bool/Double）
 * 3. 配置文件的加载和保存
 * 4. 配置重新加载
 * 5. 批量配置操作
 * 6. 线程安全性
 * 7. 配置默认值处理
 * 8. 配置键检查
 */

#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <fstream>
#include <chrono>
#include <cstdio>

// 引入配置管理器头文件
#include "src/utils/config_manager.h"
#include "src/utils/config_lifecycle.h"

namespace sqlcc {
namespace test {

// ==================== ConfigManager Basic Tests ====================

class ConfigManagerBasicTest : public ::testing::Test {
protected:
    void SetUp() override {
        manager = &ConfigManager::GetInstance();
        manager->ClearAll();
    }

    void TearDown() override {
        // 清理测试数据
        manager->ClearAll();
        // 删除临时文件
        std::remove(temp_config_file.c_str());
    }

    ConfigManager* manager;
    std::string temp_config_file = "/tmp/sqlcc_test_config_" + 
        std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) + ".conf";
};

// 测试单例模式
TEST_F(ConfigManagerBasicTest, SingletonPattern) {
    ConfigManager& manager1 = ConfigManager::GetInstance();
    ConfigManager& manager2 = ConfigManager::GetInstance();
    EXPECT_EQ(&manager1, &manager2);
}

// 测试设置和获取字符串配置
TEST_F(ConfigManagerBasicTest, SetAndGetString) {
    EXPECT_TRUE(manager->SetValue("server.host", "localhost"));
    EXPECT_EQ(manager->GetString("server.host"), "localhost");

    EXPECT_TRUE(manager->SetValue("server.host", "127.0.0.1"));
    EXPECT_EQ(manager->GetString("server.host"), "127.0.0.1");
}

// 测试设置和获取整数配置
TEST_F(ConfigManagerBasicTest, SetAndGetInt) {
    EXPECT_TRUE(manager->SetValue("server.port", 3306));
    EXPECT_EQ(manager->GetInt("server.port"), 3306);

    EXPECT_TRUE(manager->SetValue("buffer.size", 1024));
    EXPECT_EQ(manager->GetInt("buffer.size"), 1024);
}

// 测试设置和获取布尔配置
TEST_F(ConfigManagerBasicTest, SetAndGetBool) {
    EXPECT_TRUE(manager->SetValue("debug.enabled", true));
    EXPECT_TRUE(manager->GetBool("debug.enabled"));

    EXPECT_TRUE(manager->SetValue("debug.enabled", false));
    EXPECT_FALSE(manager->GetBool("debug.enabled"));
}

// 测试设置和获取双精度浮点配置
TEST_F(ConfigManagerBasicTest, SetAndGetDouble) {
    EXPECT_TRUE(manager->SetValue("cache.hit_rate", 0.95));
    EXPECT_DOUBLE_EQ(manager->GetDouble("cache.hit_rate"), 0.95);

    EXPECT_TRUE(manager->SetValue("timeout.value", 3.14159));
    EXPECT_NEAR(manager->GetDouble("timeout.value"), 3.14159, 0.00001);
}

// 测试检查键是否存在
TEST_F(ConfigManagerBasicTest, HasKey) {
    EXPECT_FALSE(manager->HasKey("test.key"));

    manager->SetValue("test.key", "value");
    EXPECT_TRUE(manager->HasKey("test.key"));

    EXPECT_FALSE(manager->HasKey("nonexistent.key"));
}

// 测试获取默认值
TEST_F(ConfigManagerBasicTest, GetDefaultValue) {
    // 不存在的键返回默认值
    EXPECT_EQ(manager->GetString("nonexistent", "default"), "default");
    EXPECT_EQ(manager->GetInt("nonexistent", 42), 42);
    EXPECT_TRUE(manager->GetBool("nonexistent", true));
    EXPECT_DOUBLE_EQ(manager->GetDouble("nonexistent", 1.5), 1.5);

    // 存在的键忽略默认值
    manager->SetValue("existing", "value");
    EXPECT_EQ(manager->GetString("existing", "default"), "value");
}

// ==================== ConfigManager File Operations Tests ====================

class ConfigManagerFileTest : public ::testing::Test {
protected:
    void SetUp() override {
        manager = &ConfigManager::GetInstance();
        manager->ClearAll();
        temp_config_file = "/tmp/sqlcc_test_config_" + 
            std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) + ".conf";
    }

    void TearDown() override {
        manager->ClearAll();
        std::remove(temp_config_file.c_str());
    }

    void CreateTestConfigFile(const std::string& content) {
        std::ofstream file(temp_config_file);
        file << content;
        file.close();
    }

    ConfigManager* manager;
    std::string temp_config_file;
};

// 测试加载配置文件
TEST_F(ConfigManagerFileTest, LoadConfigFile) {
    CreateTestConfigFile(
        "server.host=localhost\n"
        "server.port=3306\n"
        "debug.enabled=true\n"
        "cache.size=1024\n"
    );

    EXPECT_TRUE(manager->LoadConfig(temp_config_file));
    EXPECT_EQ(manager->GetString("server.host"), "localhost");
    EXPECT_EQ(manager->GetInt("server.port"), 3306);
    EXPECT_TRUE(manager->GetBool("debug.enabled"));
    EXPECT_EQ(manager->GetInt("cache.size"), 1024);
}

// 测试保存配置文件
TEST_F(ConfigManagerFileTest, SaveToFile) {
    manager->SetValue("test.key1", "value1");
    manager->SetValue("test.key2", 42);
    manager->SetValue("test.key3", true);

    EXPECT_TRUE(manager->SaveToFile(temp_config_file));

    // 验证文件存在
    std::ifstream file(temp_config_file);
    EXPECT_TRUE(file.is_open());

    // 验证文件内容（INI格式：section.key=value）
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    // 保存格式为INI: [section]\nkey=value
    EXPECT_TRUE(content.find("[test]") != std::string::npos);
    EXPECT_TRUE(content.find("key1=value1") != std::string::npos);
    EXPECT_TRUE(content.find("key2=42") != std::string::npos);
    EXPECT_TRUE(content.find("key3=true") != std::string::npos);
}

// 测试加载不存在的文件
TEST_F(ConfigManagerFileTest, LoadNonExistentFile) {
    // 加载不存在的文件时，会加载默认配置并返回true
    EXPECT_TRUE(manager->LoadConfig("/tmp/nonexistent_config_file_12345.conf"));
    // 验证默认配置已加载
    EXPECT_TRUE(manager->HasKey("buffer_pool.read_lock_timeout_ms"));
}

// 测试重新加载配置
TEST_F(ConfigManagerFileTest, ReloadConfig) {
    CreateTestConfigFile(
        "server.host=localhost\n"
        "server.port=3306\n"
    );

    EXPECT_TRUE(manager->LoadConfig(temp_config_file));
    EXPECT_EQ(manager->GetString("server.host"), "localhost");
    EXPECT_EQ(manager->GetInt("server.port"), 3306);

    // 修改配置文件
    CreateTestConfigFile(
        "server.host=127.0.0.1\n"
        "server.port=5432\n"
    );

    // 重新加载
    EXPECT_TRUE(manager->ReloadConfig());
    EXPECT_EQ(manager->GetString("server.host"), "127.0.0.1");
    EXPECT_EQ(manager->GetInt("server.port"), 5432);
}

// 测试加载新的配置文件
TEST_F(ConfigManagerFileTest, ReloadConfigWithNewFile) {
    std::string file1 = temp_config_file + ".1";
    std::string file2 = temp_config_file + ".2";

    // 创建第一个配置文件
    std::ofstream f1(file1);
    f1 << "key1=value1\n";
    f1.close();

    // 加载第一个文件
    EXPECT_TRUE(manager->LoadConfig(file1));
    EXPECT_EQ(manager->GetString("key1"), "value1");

    // 创建第二个配置文件
    std::ofstream f2(file2);
    f2 << "key2=value2\n";
    f2.close();

    // 加载第二个文件（完整替换）
    EXPECT_TRUE(manager->ReloadConfig(file2));
    EXPECT_FALSE(manager->HasKey("key1"));
    EXPECT_EQ(manager->GetString("key2"), "value2");

    // 清理
    std::remove(file1.c_str());
    std::remove(file2.c_str());
}

// 测试配置文件格式错误
TEST_F(ConfigManagerFileTest, InvalidConfigFormat) {
    CreateTestConfigFile(
        "invalid line without equals\n"
        "valid.key=value\n"
        "another invalid\n"
    );

    // 仍然可以加载，但会跳过无效行
    EXPECT_TRUE(manager->LoadConfig(temp_config_file));
    EXPECT_EQ(manager->GetString("valid.key"), "value");
}

// ==================== ConfigManager Batch Operations Tests ====================

class ConfigManagerBatchTest : public ::testing::Test {
protected:
    void SetUp() override {
        manager = &ConfigManager::GetInstance();
        manager->ClearAll();
    }

    void TearDown() override {
        manager->ClearAll();
    }

    ConfigManager* manager;
};

// 测试获取所有键
TEST_F(ConfigManagerBatchTest, GetAllKeys) {
    manager->SetValue("key1", "value1");
    manager->SetValue("key2", "value2");
    manager->SetValue("key3", "value3");

    auto keys = manager->GetAllKeys();
    EXPECT_EQ(keys.size(), 3);
    EXPECT_TRUE(std::find(keys.begin(), keys.end(), "key1") != keys.end());
    EXPECT_TRUE(std::find(keys.begin(), keys.end(), "key2") != keys.end());
    EXPECT_TRUE(std::find(keys.begin(), keys.end(), "key3") != keys.end());
}

// 测试获取前缀匹配的键
TEST_F(ConfigManagerBatchTest, GetKeysWithPrefix) {
    manager->SetValue("server.host", "localhost");
    manager->SetValue("server.port", 3306);
    manager->SetValue("server.timeout", 30);
    manager->SetValue("database.name", "testdb");

    auto server_keys = manager->GetKeysWithPrefix("server");
    EXPECT_EQ(server_keys.size(), 3);
    EXPECT_TRUE(std::find(server_keys.begin(), server_keys.end(), "server.host") != server_keys.end());
    EXPECT_TRUE(std::find(server_keys.begin(), server_keys.end(), "server.port") != server_keys.end());
    EXPECT_TRUE(std::find(server_keys.begin(), server_keys.end(), "server.timeout") != server_keys.end());

    auto db_keys = manager->GetKeysWithPrefix("database");
    EXPECT_EQ(db_keys.size(), 1);
    EXPECT_TRUE(std::find(db_keys.begin(), db_keys.end(), "database.name") != db_keys.end());
}

// 测试空配置时获取键
TEST_F(ConfigManagerBatchTest, GetKeysFromEmptyConfig) {
    auto all_keys = manager->GetAllKeys();
    EXPECT_TRUE(all_keys.empty());

    auto prefix_keys = manager->GetKeysWithPrefix("prefix");
    EXPECT_TRUE(prefix_keys.empty());
}

// ==================== ConfigManager Thread Safety Tests ====================

class ConfigManagerThreadSafetyTest : public ::testing::Test {
protected:
    void SetUp() override {
        manager = &ConfigManager::GetInstance();
        manager->ClearAll();
    }

    void TearDown() override {
        manager->ClearAll();
    }

    ConfigManager* manager;
};

// 测试多线程并发设置配置
TEST_F(ConfigManagerThreadSafetyTest, ConcurrentSetValue) {
    const int num_threads = 10;
    const int operations_per_thread = 100;
    std::vector<std::thread> threads;

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&, i]() {
            for (int j = 0; j < operations_per_thread; ++j) {
                std::string key = "thread_" + std::to_string(i) + "_key_" + std::to_string(j);
                manager->SetValue(key, i * 1000 + j);
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    // 验证所有配置都已设置
    auto all_keys = manager->GetAllKeys();
    EXPECT_EQ(all_keys.size(), num_threads * operations_per_thread);
}

// 测试多线程并发读取配置
TEST_F(ConfigManagerThreadSafetyTest, ConcurrentGetValue) {
    manager->SetValue("test.key", 42);

    const int num_threads = 10;
    const int reads_per_thread = 1000;
    std::vector<std::thread> threads;
    std::atomic<int> total_reads(0);

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < reads_per_thread; ++j) {
                int value = manager->GetInt("test.key");
                EXPECT_EQ(value, 42);
                total_reads.fetch_add(1);
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_EQ(total_reads.load(), num_threads * reads_per_thread);
}

// 测试多线程并发读写
TEST_F(ConfigManagerThreadSafetyTest, ConcurrentReadWrite) {
    const int num_threads = 10;
    std::vector<std::thread> threads;

    // 一半线程写，一半线程读
    for (int i = 0; i < num_threads; ++i) {
        if (i % 2 == 0) {
            // 写线程
            threads.emplace_back([&, i]() {
                for (int j = 0; j < 100; ++j) {
                    std::string key = "rw_key_" + std::to_string(i);
                    manager->SetValue(key, j);
                }
            });
        } else {
            // 读线程
            threads.emplace_back([&]() {
                for (int j = 0; j < 100; ++j) {
                    if (manager->HasKey("rw_key_0")) {
                        manager->GetInt("rw_key_0");
                    }
                }
            });
        }
    }

    for (auto& thread : threads) {
        thread.join();
    }

    // 不应该崩溃或死锁
    SUCCEED();
}

// ==================== ConfigManager Edge Cases Tests ====================

class ConfigManagerEdgeCasesTest : public ::testing::Test {
protected:
    void SetUp() override {
        manager = &ConfigManager::GetInstance();
        manager->ClearAll();
    }

    void TearDown() override {
        manager->ClearAll();
    }

    ConfigManager* manager;
};

// 测试空键
TEST_F(ConfigManagerEdgeCasesTest, EmptyKey) {
    // 空键可能不被支持，或者返回默认值
    std::string value = manager->GetString("", "default");
    // 验证行为
    EXPECT_EQ(value, "default");
}

// 测试空值
TEST_F(ConfigManagerEdgeCasesTest, EmptyValue) {
    manager->SetValue("empty.key", "");
    EXPECT_EQ(manager->GetString("empty.key"), "");
}

// 测试特殊字符键
TEST_F(ConfigManagerEdgeCasesTest, SpecialCharactersInKey) {
    manager->SetValue("key.with.dots", "value1");
    manager->SetValue("key-with-dashes", "value2");
    manager->SetValue("key_with_underscores", "value3");

    EXPECT_EQ(manager->GetString("key.with.dots"), "value1");
    EXPECT_EQ(manager->GetString("key-with-dashes"), "value2");
    EXPECT_EQ(manager->GetString("key_with_underscores"), "value3");
}

// 测试特殊字符值
TEST_F(ConfigManagerEdgeCasesTest, SpecialCharactersInValue) {
    manager->SetValue("special.key", "value with spaces and 特殊字符!@#$%");
    EXPECT_EQ(manager->GetString("special.key"), "value with spaces and 特殊字符!@#$%");
}

// 测试数值边界
TEST_F(ConfigManagerEdgeCasesTest, NumericBoundaries) {
    manager->SetValue("int.max", 2147483647);
    manager->SetValue("int.min", -2147483647);
    manager->SetValue("double.large", 1.79769e+308);
    manager->SetValue("double.small", 2.22507e-308);

    EXPECT_EQ(manager->GetInt("int.max"), 2147483647);
    EXPECT_EQ(manager->GetInt("int.min"), -2147483647);
    EXPECT_NEAR(manager->GetDouble("double.large"), 1.79769e+308, 1e303);
    EXPECT_NEAR(manager->GetDouble("double.small"), 2.22507e-308, 1e-313);
}

// ==================== ConfigManager Integration Tests ====================

class ConfigManagerIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        manager = &ConfigManager::GetInstance();
        manager->ClearAll();
        temp_config_file = "/tmp/sqlcc_test_config_integration_" + 
            std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) + ".conf";
    }

    void TearDown() override {
        manager->ClearAll();
        std::remove(temp_config_file.c_str());
    }

    ConfigManager* manager;
    std::string temp_config_file;
};

// 测试完整的配置生命周期
TEST_F(ConfigManagerIntegrationTest, ConfigLifecycle) {
    // 1. 创建配置文件
    std::ofstream file(temp_config_file);
    file << "app.name=SQLCC\n";
    file << "app.version=1.0.0\n";
    file << "debug.enabled=false\n";
    file.close();

    // 2. 加载配置
    EXPECT_TRUE(manager->LoadConfig(temp_config_file));
    EXPECT_EQ(manager->GetString("app.name"), "SQLCC");
    EXPECT_EQ(manager->GetString("app.version"), "1.0.0");
    EXPECT_FALSE(manager->GetBool("debug.enabled"));

    // 3. 修改配置
    manager->SetValue("debug.enabled", true);
    manager->SetValue("app.version", "1.1.0");

    // 4. 保存配置
    std::string saved_file = temp_config_file + ".saved";
    EXPECT_TRUE(manager->SaveToFile(saved_file));

    // 5. 验证保存的配置（使用单例实例）
    ConfigManager& new_manager = ConfigManager::GetInstance();
    new_manager.LoadConfig(saved_file);
    EXPECT_TRUE(new_manager.GetBool("debug.enabled"));
    EXPECT_EQ(new_manager.GetString("app.version"), "1.1.0");

    // 清理
    std::remove(saved_file.c_str());
}

// 测试配置管理器的清理
TEST_F(ConfigManagerIntegrationTest, ClearAll) {
    manager->SetValue("key1", "value1");
    manager->SetValue("key2", "value2");
    manager->SetValue("key3", "value3");

    EXPECT_EQ(manager->GetAllKeys().size(), 3);

    manager->ClearAll();
    EXPECT_TRUE(manager->GetAllKeys().empty());
    EXPECT_FALSE(manager->HasKey("key1"));
}

// 测试配置重置功能
TEST_F(ConfigManagerIntegrationTest, ResetForTest) {
    manager->SetValue("test.key", "test_value");
    EXPECT_TRUE(manager->HasKey("test.key"));

    manager->ResetForTest();
    EXPECT_FALSE(manager->HasKey("test.key"));
}

// ==================== ConfigManager Operation Timeout Tests ====================

class ConfigManagerTimeoutTest : public ::testing::Test {
protected:
    void SetUp() override {
        manager = &ConfigManager::GetInstance();
    }

    void TearDown() override {
        manager->ResetForTest();
    }

    ConfigManager* manager;
};

// 测试设置操作超时
TEST_F(ConfigManagerTimeoutTest, SetOperationTimeout) {
    manager->SetOperationTimeout(5000);
    EXPECT_EQ(manager->GetOperationTimeout(), 5000);

    manager->SetOperationTimeout(10000);
    EXPECT_EQ(manager->GetOperationTimeout(), 10000);
}

// 测试默认超时值
TEST_F(ConfigManagerTimeoutTest, DefaultTimeout) {
    EXPECT_EQ(manager->GetOperationTimeout(), kDefaultOperationTimeoutMs);
}

// ==================== ConfigLifecycle Tests ====================

class ConfigLifecycleTest : public ::testing::Test {
protected:
    void TearDown() override {
    }
};

// 测试 FormatConfigValue 函数 - 布尔值
TEST_F(ConfigLifecycleTest, FormatConfigValueBool) {
    ConfigValue bool_true = true;
    ConfigValue bool_false = false;
    EXPECT_EQ(FormatConfigValue(bool_true), "true");
    EXPECT_EQ(FormatConfigValue(bool_false), "false");
}

// 测试 FormatConfigValue 函数 - 整数
TEST_F(ConfigLifecycleTest, FormatConfigValueInt) {
    ConfigValue int_val = 42;
    EXPECT_EQ(FormatConfigValue(int_val), "42");
    
    ConfigValue int_neg = -100;
    EXPECT_EQ(FormatConfigValue(int_neg), "-100");
    
    ConfigValue int_zero = 0;
    EXPECT_EQ(FormatConfigValue(int_zero), "0");
}

// 测试 FormatConfigValue 函数 - 浮点数
TEST_F(ConfigLifecycleTest, FormatConfigValueDouble) {
    ConfigValue double_val = 3.14159;
    EXPECT_EQ(FormatConfigValue(double_val), "3.141590");
    
    ConfigValue double_zero = 0.0;
    EXPECT_EQ(FormatConfigValue(double_zero), "0.000000");
}

// 测试 FormatConfigValue 函数 - 字符串
TEST_F(ConfigLifecycleTest, FormatConfigValueString) {
    ConfigValue str_val = std::string("hello");
    EXPECT_EQ(FormatConfigValue(str_val), "\"hello\"");
    
    ConfigValue empty_str = std::string("");
    EXPECT_EQ(FormatConfigValue(empty_str), "\"\"");
}

// 测试 ParseConfigValue 函数 - 布尔值
TEST_F(ConfigLifecycleTest, ParseConfigValueBool) {
    ConfigValue result;
    
    EXPECT_TRUE(ParseConfigValue("true", result));
    EXPECT_EQ(std::get<bool>(result), true);
    
    EXPECT_TRUE(ParseConfigValue("TRUE", result));
    EXPECT_EQ(std::get<bool>(result), true);
    
    EXPECT_TRUE(ParseConfigValue("True", result));
    EXPECT_EQ(std::get<bool>(result), true);
    
    EXPECT_TRUE(ParseConfigValue("false", result));
    EXPECT_EQ(std::get<bool>(result), false);
    
    EXPECT_TRUE(ParseConfigValue("FALSE", result));
    EXPECT_EQ(std::get<bool>(result), false);
    
    EXPECT_TRUE(ParseConfigValue("False", result));
    EXPECT_EQ(std::get<bool>(result), false);
}

// 测试 ParseConfigValue 函数 - 整数
TEST_F(ConfigLifecycleTest, ParseConfigValueInt) {
    ConfigValue result;
    
    EXPECT_TRUE(ParseConfigValue("42", result));
    EXPECT_EQ(std::get<int>(result), 42);
    
    EXPECT_TRUE(ParseConfigValue("-100", result));
    EXPECT_EQ(std::get<int>(result), -100);
    
    EXPECT_TRUE(ParseConfigValue("0", result));
    EXPECT_EQ(std::get<int>(result), 0);
    
    EXPECT_TRUE(ParseConfigValue("2147483647", result));
    EXPECT_EQ(std::get<int>(result), 2147483647);
}

// 测试 ParseConfigValue 函数 - 浮点数
TEST_F(ConfigLifecycleTest, ParseConfigValueDouble) {
    ConfigValue result;
    
    EXPECT_TRUE(ParseConfigValue("3.14159", result));
    EXPECT_NEAR(std::get<double>(result), 3.14159, 0.0001);
    
    EXPECT_TRUE(ParseConfigValue("-100.5", result));
    EXPECT_NEAR(std::get<double>(result), -100.5, 0.0001);
    
    EXPECT_TRUE(ParseConfigValue("0.0", result));
    EXPECT_NEAR(std::get<double>(result), 0.0, 0.0001);
}

// 测试 ParseConfigValue 函数 - 字符串
TEST_F(ConfigLifecycleTest, ParseConfigValueString) {
    ConfigValue result;
    
    EXPECT_TRUE(ParseConfigValue("hello", result));
    EXPECT_EQ(std::get<std::string>(result), "hello");
    
    EXPECT_TRUE(ParseConfigValue("hello world", result));
    EXPECT_EQ(std::get<std::string>(result), "hello world");
    
    EXPECT_TRUE(ParseConfigValue("123abc", result));
    EXPECT_EQ(std::get<std::string>(result), "123abc");
}

// 测试 ParseConfigValue 函数 - 空白字符处理
TEST_F(ConfigLifecycleTest, ParseConfigValueWhitespace) {
    ConfigValue result;
    
    EXPECT_TRUE(ParseConfigValue("  42  ", result));
    EXPECT_EQ(std::get<int>(result), 42);
    
    EXPECT_TRUE(ParseConfigValue("  true  ", result));
    EXPECT_EQ(std::get<bool>(result), true);
    
    EXPECT_TRUE(ParseConfigValue("  hello  ", result));
    EXPECT_EQ(std::get<std::string>(result), "hello");
}

// ==================== ConfigSnapshot Tests ====================

class ConfigSnapshotTest : public ::testing::Test {
protected:
    void SetUp() override {
        config_data_["server.host"] = std::string("localhost");
        config_data_["server.port"] = 3306;
        config_data_["debug.enabled"] = true;
        config_data_["cache.size"] = 1024;
    }
    
    std::unordered_map<std::string, ConfigValue> config_data_;
};

// 测试 ConfigSnapshot 创建
TEST_F(ConfigSnapshotTest, CreateSnapshot) {
    auto snapshot = ConfigSnapshotFactory::CreateSnapshot(
        config_data_, "v1.0", "Test snapshot");
    
    EXPECT_NE(snapshot, nullptr);
    EXPECT_EQ(snapshot->GetConfigCount(), 4);
    
    ConfigValue value;
    EXPECT_TRUE(snapshot->GetValue("server.host", value));
    EXPECT_EQ(std::get<std::string>(value), "localhost");
    
    EXPECT_TRUE(snapshot->GetValue("server.port", value));
    EXPECT_EQ(std::get<int>(value), 3306);
    
    EXPECT_TRUE(snapshot->GetValue("debug.enabled", value));
    EXPECT_EQ(std::get<bool>(value), true);
    
    EXPECT_TRUE(snapshot->GetValue("cache.size", value));
    EXPECT_EQ(std::get<int>(value), 1024);
}

// 测试 ConfigSnapshot HasKey
TEST_F(ConfigSnapshotTest, HasKey) {
    auto snapshot = ConfigSnapshotFactory::CreateSnapshot(
        config_data_, "v1.0", "Test snapshot");
    
    EXPECT_TRUE(snapshot->HasKey("server.host"));
    EXPECT_TRUE(snapshot->HasKey("server.port"));
    EXPECT_TRUE(snapshot->HasKey("debug.enabled"));
    EXPECT_TRUE(snapshot->HasKey("cache.size"));
    
    EXPECT_FALSE(snapshot->HasKey("nonexistent.key"));
}

// 测试 ConfigSnapshot GetAllKeys
TEST_F(ConfigSnapshotTest, GetAllKeys) {
    auto snapshot = ConfigSnapshotFactory::CreateSnapshot(
        config_data_, "v1.0", "Test snapshot");
    
    auto keys = snapshot->GetAllKeys();
    EXPECT_EQ(keys.size(), 4);
    
    EXPECT_TRUE(std::find(keys.begin(), keys.end(), "server.host") != keys.end());
    EXPECT_TRUE(std::find(keys.begin(), keys.end(), "server.port") != keys.end());
    EXPECT_TRUE(std::find(keys.begin(), keys.end(), "debug.enabled") != keys.end());
    EXPECT_TRUE(std::find(keys.begin(), keys.end(), "cache.size") != keys.end());
}

// 测试 ConfigSnapshot GetKeysWithPrefix
TEST_F(ConfigSnapshotTest, GetKeysWithPrefix) {
    auto snapshot = ConfigSnapshotFactory::CreateSnapshot(
        config_data_, "v1.0", "Test snapshot");
    
    auto server_keys = snapshot->GetKeysWithPrefix("server.");
    EXPECT_EQ(server_keys.size(), 2);
    EXPECT_TRUE(std::find(server_keys.begin(), server_keys.end(), "server.host") != server_keys.end());
    EXPECT_TRUE(std::find(server_keys.begin(), server_keys.end(), "server.port") != server_keys.end());
    
    auto debug_keys = snapshot->GetKeysWithPrefix("debug.");
    EXPECT_EQ(debug_keys.size(), 1);
    EXPECT_TRUE(std::find(debug_keys.begin(), debug_keys.end(), "debug.enabled") != debug_keys.end());
}

// 测试 ConfigSnapshot 校验和
TEST_F(ConfigSnapshotTest, Checksum) {
    auto snapshot = ConfigSnapshotFactory::CreateSnapshot(
        config_data_, "v1.0", "Test snapshot");
    
    std::string checksum = snapshot->CalculateChecksum();
    EXPECT_FALSE(checksum.empty());
    
    EXPECT_TRUE(snapshot->ValidateIntegrity());
}

// 测试 ConfigSnapshot Clone
TEST_F(ConfigSnapshotTest, Clone) {
    auto snapshot = ConfigSnapshotFactory::CreateSnapshot(
        config_data_, "v1.0", "Test snapshot");
    
    auto cloned = snapshot->Clone();
    
    EXPECT_NE(cloned, nullptr);
    EXPECT_EQ(cloned->GetConfigCount(), 4);
    
    ConfigValue value;
    EXPECT_TRUE(cloned->GetValue("server.host", value));
    EXPECT_EQ(std::get<std::string>(value), "localhost");
}

// 测试 ConfigSnapshot Equals
TEST_F(ConfigSnapshotTest, Equals) {
    auto snapshot1 = ConfigSnapshotFactory::CreateSnapshot(
        config_data_, "v1.0", "Test snapshot");
    
    auto snapshot2 = ConfigSnapshotFactory::CreateSnapshot(
        config_data_, "v1.0", "Test snapshot");
    
    auto snapshot3 = ConfigSnapshotFactory::CreateSnapshot(
        config_data_, "v2.0", "Different snapshot");
    
    EXPECT_TRUE(snapshot1->Equals(*snapshot2));
    EXPECT_FALSE(snapshot1->Equals(*snapshot3));
}

// 测试 ConfigSnapshotFactory CreateEmptySnapshot
TEST_F(ConfigSnapshotTest, CreateEmptySnapshot) {
    auto empty_snapshot = ConfigSnapshotFactory::CreateEmptySnapshot(
        "v0.0", "Empty snapshot");
    
    EXPECT_NE(empty_snapshot, nullptr);
    EXPECT_EQ(empty_snapshot->GetConfigCount(), 0);
    EXPECT_TRUE(empty_snapshot->GetAllKeys().empty());
}

// 测试 ConfigSnapshotFactory MergeSnapshots
TEST_F(ConfigSnapshotTest, MergeSnapshots) {
    std::unordered_map<std::string, ConfigValue> base_data;
    base_data["base.key1"] = 100;
    base_data["base.key2"] = std::string("base_value");
    
    std::unordered_map<std::string, ConfigValue> override_data;
    override_data["override.key1"] = 200;
    override_data["override.key3"] = true;
    
    auto base_snapshot = ConfigSnapshotFactory::CreateSnapshot(base_data, "base", "Base config");
    auto override_snapshot = ConfigSnapshotFactory::CreateSnapshot(override_data, "override", "Override config");
    
    auto merged = ConfigSnapshotFactory::MergeSnapshots(
        base_snapshot, override_snapshot, "merged", "Merged config");
    
    EXPECT_NE(merged, nullptr);
    EXPECT_EQ(merged->GetConfigCount(), 4);  // base.key1, base.key2, override.key1, override.key3
    
    ConfigValue value;
    EXPECT_TRUE(merged->GetValue("base.key1", value));
    EXPECT_EQ(std::get<int>(value), 100);  // 保留base的值，override.key1是不同的键
    
    EXPECT_TRUE(merged->GetValue("base.key2", value));
    EXPECT_EQ(std::get<std::string>(value), "base_value");
    
    EXPECT_TRUE(merged->GetValue("override.key1", value));
    EXPECT_EQ(std::get<int>(value), 200);
    
    EXPECT_TRUE(merged->GetValue("override.key3", value));
    EXPECT_EQ(std::get<bool>(value), true);
}

// 测试 ConfigSnapshotManager
class ConfigSnapshotManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        config_data_["key1"] = 100;
        config_data_["key2"] = std::string("value");
    }
    
    std::unordered_map<std::string, ConfigValue> config_data_;
};

// 测试 ConfigSnapshotManager 添加和获取快照
TEST_F(ConfigSnapshotManagerTest, AddAndGetSnapshot) {
    ConfigSnapshotManager manager;
    
    auto snapshot = ConfigSnapshotFactory::CreateSnapshot(
        config_data_, "v1.0", "Test snapshot");
    
    EXPECT_TRUE(manager.AddSnapshot(snapshot));
    
    auto retrieved = manager.GetSnapshot("v1.0");
    EXPECT_NE(retrieved, nullptr);
    EXPECT_EQ(retrieved->GetConfigCount(), 2);
}

// 测试 ConfigSnapshotManager 获取当前快照
TEST_F(ConfigSnapshotManagerTest, GetCurrentSnapshot) {
    ConfigSnapshotManager manager;
    
    auto snapshot1 = ConfigSnapshotFactory::CreateSnapshot(
        config_data_, "v1.0", "First snapshot");
    auto snapshot2 = ConfigSnapshotFactory::CreateSnapshot(
        config_data_, "v2.0", "Second snapshot");
    
    manager.AddSnapshot(snapshot1);
    EXPECT_EQ(manager.GetCurrentVersionId(), "v1.0");
    
    manager.AddSnapshot(snapshot2);
    EXPECT_EQ(manager.GetCurrentVersionId(), "v2.0");
}

// 测试 ConfigSnapshotManager 获取所有版本ID
TEST_F(ConfigSnapshotManagerTest, GetAllVersionIds) {
    ConfigSnapshotManager manager;
    
    auto snapshot1 = ConfigSnapshotFactory::CreateSnapshot(
        config_data_, "v1.0", "First snapshot");
    auto snapshot2 = ConfigSnapshotFactory::CreateSnapshot(
        config_data_, "v2.0", "Second snapshot");
    
    manager.AddSnapshot(snapshot1);
    manager.AddSnapshot(snapshot2);
    
    auto version_ids = manager.GetAllVersionIds();
    EXPECT_EQ(version_ids.size(), 2);
    EXPECT_TRUE(std::find(version_ids.begin(), version_ids.end(), "v1.0") != version_ids.end());
    EXPECT_TRUE(std::find(version_ids.begin(), version_ids.end(), "v2.0") != version_ids.end());
}

// 测试 ConfigSnapshotManager 回滚
TEST_F(ConfigSnapshotManagerTest, Rollback) {
    ConfigSnapshotManager manager;
    
    auto snapshot1 = ConfigSnapshotFactory::CreateSnapshot(
        config_data_, "v1.0", "First snapshot");
    auto snapshot2 = ConfigSnapshotFactory::CreateSnapshot(
        config_data_, "v2.0", "Second snapshot");
    
    manager.AddSnapshot(snapshot1);
    manager.AddSnapshot(snapshot2);
    
    EXPECT_TRUE(manager.RollbackToVersion("v1.0"));
    EXPECT_EQ(manager.GetCurrentVersionId(), "v1.0");
    
    EXPECT_FALSE(manager.RollbackToVersion("nonexistent"));
}

// 测试 ConfigSnapshotManager 清理快照
TEST_F(ConfigSnapshotManagerTest, CleanupSnapshots) {
    ConfigSnapshotManager manager;
    
    for (int i = 0; i < 15; i++) {
        auto snapshot = ConfigSnapshotFactory::CreateSnapshot(
            config_data_, "v" + std::to_string(i) + ".0", "Snapshot " + std::to_string(i));
        manager.AddSnapshot(snapshot);
    }
    
    EXPECT_EQ(manager.GetSnapshotCount(), 15);
    
    size_t removed = manager.CleanupSnapshots(10);
    EXPECT_EQ(removed, 5);
    EXPECT_EQ(manager.GetSnapshotCount(), 10);
}

// 测试 ConfigSnapshotManager 删除快照
TEST_F(ConfigSnapshotManagerTest, RemoveSnapshot) {
    ConfigSnapshotManager manager;
    
    auto snapshot1 = ConfigSnapshotFactory::CreateSnapshot(
        config_data_, "v1.0", "First snapshot");
    auto snapshot2 = ConfigSnapshotFactory::CreateSnapshot(
        config_data_, "v2.0", "Second snapshot");
    
    manager.AddSnapshot(snapshot1);
    manager.AddSnapshot(snapshot2);
    
    EXPECT_FALSE(manager.RemoveSnapshot("v2.0"));  // 不能删除当前版本
    EXPECT_TRUE(manager.RemoveSnapshot("v1.0"));
    
    EXPECT_EQ(manager.GetSnapshotCount(), 1);
    EXPECT_EQ(manager.GetSnapshot("v1.0"), nullptr);
}

// 测试 ConfigSnapshotManager 获取版本历史
TEST_F(ConfigSnapshotManagerTest, GetVersionHistory) {
    ConfigSnapshotManager manager;
    
    auto snapshot1 = ConfigSnapshotFactory::CreateSnapshot(
        config_data_, "v1.0", "First snapshot");
    auto snapshot2 = ConfigSnapshotFactory::CreateSnapshot(
        config_data_, "v2.0", "Second snapshot");
    
    manager.AddSnapshot(snapshot1);
    manager.AddSnapshot(snapshot2);
    manager.RollbackToVersion("v1.0");
    
    auto history = manager.GetVersionHistory();
    EXPECT_EQ(history.size(), 3);
}

// ==================== GenerateVersionId Tests ====================

TEST(GenerateVersionIdTest, Basic) {
    std::string id1 = GenerateVersionId("v");
    EXPECT_FALSE(id1.empty());
    EXPECT_EQ(id1[0], 'v');
    
    std::string id2 = GenerateVersionId("version");
    EXPECT_FALSE(id2.empty());
    EXPECT_TRUE(id2.find("version") == 0);
    
    std::string id3 = GenerateVersionId("");
    EXPECT_FALSE(id3.empty());
}

} // namespace test
} // namespace sqlcc

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}