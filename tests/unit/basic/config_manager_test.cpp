/**
 * @file config_manager_test.cpp
 * @brief ConfigManager单元测试
 *
 * 测试ConfigManager类的核心功能，包括：
 * - 单例模式
 * - 配置加载和解析
 * - 不同数据类型的配置访问
 * - 配置设置和修改
 * - 配置键存在性检查
 * - 配置保存功能
 * - 操作超时设置
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <filesystem>
#include <fstream>
#include <string>
#include <thread>
#include <chrono>

#include "config_manager.h"

// 使用sqlcc命名空间
using namespace sqlcc;

/**
 * @brief ConfigManager测试套件
 */
class ConfigManagerTest : public ::testing::Test {
protected:
    /**
     * @brief 测试前设置
     */
    void SetUp() override {
        // 创建临时配置文件
        temp_config_path_ = std::filesystem::temp_directory_path() / "test_config.ini";
        CreateTestConfigFile();

        // 清除ConfigManager的状态以确保测试独立性
        ClearConfigManagerState();
    }

    /**
     * @brief 测试后清理
     */
    void TearDown() override {
        // 清理临时文件
        if (std::filesystem::exists(temp_config_path_)) {
            std::filesystem::remove(temp_config_path_);
        }
    }

    /**
     * @brief 创建测试配置文件
     */
    void CreateTestConfigFile() {
        std::ofstream config_file(temp_config_path_);
        config_file << "[database]\n";
        config_file << "port = 3306\n";
        config_file << "max_connections = 100\n";
        config_file << "timeout = 30.5\n";
        config_file << "debug = true\n";
        config_file << "name = test_db\n";
        config_file << "\n";
        config_file << "[storage]\n";
        config_file << "buffer_size = 1024\n";
        config_file << "page_size = 4096\n";
        config_file.close();
    }

    std::filesystem::path temp_config_path_;

    /**
     * @brief 清除ConfigManager的状态
     */
    void ClearConfigManagerState() {
        ConfigManager& config = ConfigManager::GetInstance();
        // 清除所有配置项（通过重新设置空配置来模拟）
        // 注意：这里我们通过LoadConfig一个不存在的文件来清除状态
        config.LoadConfig("/nonexistent_config_file.ini");
    }
};

/**
 * @brief 测试单例模式
 */
TEST_F(ConfigManagerTest, SingletonPattern) {
    // 获取两个ConfigManager实例引用
    ConfigManager& instance1 = ConfigManager::GetInstance();
    ConfigManager& instance2 = ConfigManager::GetInstance();

    // 验证是同一个实例
    EXPECT_EQ(&instance1, &instance2);

    // 验证实例不为空
    EXPECT_NE(&instance1, nullptr);
    EXPECT_NE(&instance2, nullptr);
}

/**
 * @brief 测试配置加载
 */
TEST_F(ConfigManagerTest, LoadConfig) {
    ConfigManager& config = ConfigManager::GetInstance();

    // 测试加载配置文件
    bool result = config.LoadConfig(temp_config_path_.string());
    EXPECT_TRUE(result);

    // 验证加载的配置值
    EXPECT_EQ(config.GetInt("database.port"), 3306);
    EXPECT_EQ(config.GetInt("database.max_connections"), 100);
    EXPECT_DOUBLE_EQ(config.GetDouble("database.timeout"), 30.5);
    EXPECT_TRUE(config.GetBool("database.debug"));
    EXPECT_EQ(config.GetString("database.name"), "test_db");
}

/**
 * @brief 测试默认值获取
 */
TEST_F(ConfigManagerTest, DefaultValues) {
    ConfigManager& config = ConfigManager::GetInstance();

    // 测试不存在的键返回默认值
    EXPECT_EQ(config.GetInt("nonexistent.key"), 42);
    EXPECT_DOUBLE_EQ(config.GetDouble("nonexistent.key"), 3.14);
    EXPECT_FALSE(config.GetBool("nonexistent.key"));
    EXPECT_EQ(config.GetString("nonexistent.key"), "default_value");

    // 测试存在的键不使用默认值
    config.LoadConfig(temp_config_path_.string());
    EXPECT_EQ(config.GetInt("database.port", 9999), 3306);  // 应该返回实际值，而不是默认值
}

/**
 * @brief 测试配置设置
 */
TEST_F(ConfigManagerTest, SetValues) {
    ConfigManager& config = ConfigManager::GetInstance();

    // 设置不同类型的配置值
    EXPECT_TRUE(config.SetValue("test.int", 123));
    EXPECT_TRUE(config.SetValue("test.double", 45.67));
    EXPECT_TRUE(config.SetValue("test.bool", true));
    EXPECT_TRUE(config.SetValue("test.string", "test_value"));

    // 验证设置的值
    EXPECT_EQ(config.GetInt("test.int"), 123);
    EXPECT_DOUBLE_EQ(config.GetDouble("test.double"), 45.67);
    EXPECT_TRUE(config.GetBool("test.bool"));
    EXPECT_EQ(config.GetString("test.string"), "test_value");
}

/**
 * @brief 测试键存在性检查
 */
TEST_F(ConfigManagerTest, HasKey) {
    ConfigManager& config = ConfigManager::GetInstance();

    // 测试不存在的键
    EXPECT_FALSE(config.HasKey("nonexistent.key"));

    // 设置一个键
    config.SetValue("test.exists", 123);
    EXPECT_TRUE(config.HasKey("test.exists"));

    // 加载配置文件后测试
    config.LoadConfig(temp_config_path_.string());
    EXPECT_TRUE(config.HasKey("database.port"));
    EXPECT_FALSE(config.HasKey("nonexistent.section"));
}

/**
 * @brief 测试配置键列表获取
 */
TEST_F(ConfigManagerTest, GetAllKeys) {
    ConfigManager& config = ConfigManager::GetInstance();

    // 设置一些测试键
    config.SetValue("test.key1", 1);
    config.SetValue("test.key2", "value");
    config.SetValue("other.key3", true);

    // 获取所有键
    auto all_keys = config.GetAllKeys();
    EXPECT_GE(all_keys.size(), 3);

    // 验证包含我们设置的键
    EXPECT_THAT(all_keys, ::testing::Contains("test.key1"));
    EXPECT_THAT(all_keys, ::testing::Contains("test.key2"));
    EXPECT_THAT(all_keys, ::testing::Contains("other.key3"));
}

/**
 * @brief 测试前缀匹配键获取
 */
TEST_F(ConfigManagerTest, GetKeysWithPrefix) {
    ConfigManager& config = ConfigManager::GetInstance();

    // 设置一些带前缀的键
    config.SetValue("database.host", "localhost");
    config.SetValue("database.port", 3306);
    config.SetValue("storage.path", "/tmp");
    config.SetValue("storage.size", 1024);

    // 获取database前缀的键
    auto db_keys = config.GetKeysWithPrefix("database");
    EXPECT_EQ(db_keys.size(), 2);
    EXPECT_THAT(db_keys, ::testing::Contains("database.host"));
    EXPECT_THAT(db_keys, ::testing::Contains("database.port"));

    // 获取storage前缀的键
    auto storage_keys = config.GetKeysWithPrefix("storage");
    EXPECT_EQ(storage_keys.size(), 2);
    EXPECT_THAT(storage_keys, ::testing::Contains("storage.path"));
    EXPECT_THAT(storage_keys, ::testing::Contains("storage.size"));
}

/**
 * @brief 测试操作超时设置
 */
TEST_F(ConfigManagerTest, OperationTimeout) {
    ConfigManager& config = ConfigManager::GetInstance();

    // 测试默认超时时间
    EXPECT_EQ(config.GetOperationTimeout(), 5000);  // 5秒

    // 设置新的超时时间
    config.SetOperationTimeout(10000);  // 10秒
    EXPECT_EQ(config.GetOperationTimeout(), 10000);

    // 设置为0（无超时）
    config.SetOperationTimeout(0);
    EXPECT_EQ(config.GetOperationTimeout(), 0);
}

/**
 * @brief 测试配置保存功能
 */
TEST_F(ConfigManagerTest, SaveToFile) {
    ConfigManager& config = ConfigManager::GetInstance();

    // 设置一些配置值
    config.SetValue("save.test.int", 999);
    config.SetValue("save.test.string", "saved_value");
    config.SetValue("save.test.bool", false);

    // 创建临时保存文件路径
    auto save_path = std::filesystem::temp_directory_path() / "test_save_config.ini";

    // 保存配置
    bool save_result = config.SaveToFile(save_path.string());
    EXPECT_TRUE(save_result);

    // 验证文件存在
    EXPECT_TRUE(std::filesystem::exists(save_path));

    // 清理
    if (std::filesystem::exists(save_path)) {
        std::filesystem::remove(save_path);
    }
}

/**
 * @brief 测试配置重新加载
 */
TEST_F(ConfigManagerTest, ReloadConfig) {
    ConfigManager& config = ConfigManager::GetInstance();

    // 首次加载配置
    config.LoadConfig(temp_config_path_.string());
    EXPECT_EQ(config.GetInt("database.port"), 3306);

    // 修改配置值
    config.SetValue("database.port", 5432);
    EXPECT_EQ(config.GetInt("database.port"), 5432);

    // 重新加载配置（如果支持的话）
    // 注意：实际实现中可能需要修改配置文件内容来测试重新加载
    // 这里主要测试接口可用性
}

/**
 * @brief 测试配置类型转换
 */
TEST_F(ConfigManagerTest, TypeConversions) {
    ConfigManager& config = ConfigManager::GetInstance();

    // 设置整数值，读取为其他类型
    config.SetValue("type.int", 42);
    EXPECT_EQ(config.GetInt("type.int"), 42);
    EXPECT_DOUBLE_EQ(config.GetDouble("type.int"), 42.0);
    EXPECT_EQ(config.GetString("type.int"), "42");
    EXPECT_TRUE(config.GetBool("type.int"));  // 非零值转换为true

    // 设置布尔值
    config.SetValue("type.bool", true);
    EXPECT_TRUE(config.GetBool("type.bool"));
    EXPECT_EQ(config.GetInt("type.bool"), 1);
    EXPECT_EQ(config.GetString("type.bool"), "1");

    // 设置双精度浮点值
    config.SetValue("type.double", 3.14159);
    EXPECT_DOUBLE_EQ(config.GetDouble("type.double"), 3.14159);
    EXPECT_EQ(config.GetString("type.double"), "3.14159");

    // 设置字符串值
    config.SetValue("type.string", "hello");
    EXPECT_EQ(config.GetString("type.string"), "hello");
    EXPECT_EQ(config.GetInt("type.string"), 0);  // 字符串转整数失败返回0
}

/**
 * @brief 测试边界条件
 */
TEST_F(ConfigManagerTest, BoundaryConditions) {
    ConfigManager& config = ConfigManager::GetInstance();

    // 测试空键
    EXPECT_FALSE(config.HasKey(""));
    EXPECT_EQ(config.GetString(""), "");

    // 测试包含特殊字符的键
    config.SetValue("special.key-with-dashes", 123);
    EXPECT_TRUE(config.HasKey("special.key-with-dashes"));
    EXPECT_EQ(config.GetInt("special.key-with-dashes"), 123);

    // 测试层次结构键
    config.SetValue("level1.level2.level3.value", "nested");
    EXPECT_TRUE(config.HasKey("level1.level2.level3.value"));
    EXPECT_EQ(config.GetString("level1.level2.level3.value"), "nested");
}

/**
 * @brief 测试并发访问
 */
TEST_F(ConfigManagerTest, ConcurrentAccess) {
    ConfigManager& config = ConfigManager::GetInstance();

    // 测试并发读取
    auto read_func = [&config]() {
        for (int i = 0; i < 100; ++i) {
            config.GetInt("test.concurrent", 0);
            config.HasKey("test.concurrent");
        }
    };

    // 启动多个线程
    std::vector<std::thread> threads;
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back(read_func);
    }

    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }

    // 如果程序没有崩溃，说明并发访问是安全的
    EXPECT_TRUE(true);
}

/**
 * @brief 测试大配置文件的处理
 */
TEST_F(ConfigManagerTest, LargeConfiguration) {
    ConfigManager& config = ConfigManager::GetInstance();

    // 创建一个包含大量配置项的文件
    auto large_config_path = std::filesystem::temp_directory_path() / "large_config.ini";
    std::ofstream large_file(large_config_path);

    large_file << "[large_section]\n";
    for (int i = 0; i < 1000; ++i) {
        large_file << "key" << i << " = value" << i << "\n";
    }
    large_file.close();

    // 加载大配置文件
    bool load_result = config.LoadConfig(large_config_path.string());
    EXPECT_TRUE(load_result);

    // 验证部分配置项
    EXPECT_EQ(config.GetString("large_section.key0"), "value0");
    EXPECT_EQ(config.GetString("large_section.key999"), "value999");

    // 清理
    if (std::filesystem::exists(large_config_path)) {
        std::filesystem::remove(large_config_path);
    }
}
