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

    // 验证文件内容
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    EXPECT_TRUE(content.find("test.key1") != std::string::npos);
    EXPECT_TRUE(content.find("value1") != std::string::npos);
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

// 测试加载不存在的文件
TEST_F(ConfigManagerFileTest, LoadNonExistentFile) {
    EXPECT_FALSE(manager->LoadConfig("/tmp/nonexistent_config_file_12345.conf"));
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

} // namespace test
} // namespace sqlcc

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}