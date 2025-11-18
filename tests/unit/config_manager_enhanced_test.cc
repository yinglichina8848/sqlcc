/**
 * @file config_manager_enhanced_test.cc
 * @brief 配置管理器增强测试实现文件
 * 
 * Why: 需要提高config_manager.cc的代码覆盖率，特别是未覆盖的核心方法
 * What: 实现ConfigManagerEnhancedTest类，提供更全面的测试用例
 * How: 使用Google Test框架编写测试用例，覆盖配置管理器的所有公共接口和错误处理路径
 */

#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>
#include <thread>
#include <chrono>
#include <algorithm>

#include "config_manager.h"

namespace sqlcc {
namespace test {

/**
 * @brief 配置管理器增强测试类
 * 
 * Why: 需要创建增强测试类来提高配置管理器的代码覆盖率
 * What: ConfigManagerEnhancedTest类继承自testing::Test，提供更全面的测试环境
 * How: 实现SetUp和TearDown方法，创建更复杂的测试场景
 */
class ConfigManagerEnhancedTest : public ::testing::Test {
protected:
    /**
     * @brief 测试环境设置
     * 
     * Why: 需要在每个测试前设置测试环境，包括创建多个测试配置文件
     * What: SetUp方法创建临时目录和多种类型的配置文件
     * How: 使用文件系统库创建临时目录和各种配置文件
     */
    void SetUp() override {
        // 创建临时目录
        temp_dir_ = std::filesystem::temp_directory_path() / "config_manager_enhanced_test";
        std::filesystem::create_directories(temp_dir_);
        
        // 创建基本测试配置文件
        basic_config_file_ = temp_dir_ / "basic_config.conf";
        CreateBasicConfigFile();
        
        // 创建复杂测试配置文件（包含节）
        complex_config_file_ = temp_dir_ / "complex_config.conf";
        CreateComplexConfigFile();
        
        // 创建无效配置文件
        invalid_config_file_ = temp_dir_ / "invalid_config.conf";
        CreateInvalidConfigFile();
        
        // 创建环境特定配置文件
        env_config_file_ = temp_dir_ / "basic_config.test.conf";
        CreateEnvConfigFile();
    }
    
    /**
     * @brief 测试环境清理
     * 
     * Why: 需要在每个测试后清理测试环境，删除临时文件和目录
     * What: TearDown方法删除所有测试配置文件和临时目录
     * How: 使用文件系统库删除临时文件和目录
     */
    void TearDown() override {
        // 清理所有临时文件和目录
        if (std::filesystem::exists(basic_config_file_)) {
            std::filesystem::remove(basic_config_file_);
        }
        
        if (std::filesystem::exists(complex_config_file_)) {
            std::filesystem::remove(complex_config_file_);
        }
        
        if (std::filesystem::exists(invalid_config_file_)) {
            std::filesystem::remove(invalid_config_file_);
        }
        
        if (std::filesystem::exists(env_config_file_)) {
            std::filesystem::remove(env_config_file_);
        }
        
        if (std::filesystem::exists(temp_dir_)) {
            std::filesystem::remove_all(temp_dir_);
        }
    }
    
    /**
     * @brief 创建基本测试配置文件
     * 
     * Why: 需要创建基本配置文件，用于测试基本配置加载功能
     * What: CreateBasicConfigFile方法创建包含基本配置项的测试文件
     * How: 使用ofstream写入配置项到文件
     */
    void CreateBasicConfigFile() {
        std::ofstream config_file(basic_config_file_);
        config_file << "# Basic Configuration File\n";
        config_file << "database.page_size = 4096\n";
        config_file << "database.buffer_pool_size = 1024\n";
        config_file << "database.enable_logging = true\n";
        config_file << "performance.max_threads = 8\n";
        config_file << "performance.query_timeout = 30.5\n";
        config_file << "system.log_level = INFO\n";
        config_file.close();
    }
    
    /**
     * @brief 创建复杂测试配置文件（包含节）
     * 
     * Why: 需要创建包含节的配置文件，测试配置解析功能
     * What: CreateComplexConfigFile方法创建包含多个节的配置文件
     * How: 使用ofstream写入节和配置项到文件
     */
    void CreateComplexConfigFile() {
        std::ofstream config_file(complex_config_file_);
        config_file << "# Complex Configuration File\n\n";
        
        config_file << "[database]\n";
        config_file << "page_size = 8192\n";
        config_file << "buffer_pool_size = 2048\n";
        config_file << "enable_logging = false\n\n";
        
        config_file << "[performance]\n";
        config_file << "max_threads = 16\n";
        config_file << "query_timeout = 60.0\n\n";
        
        config_file << "[system]\n";
        config_file << "log_level = DEBUG\n";
        config_file << "log_file = /var/log/sqlcc.log\n";
        
        config_file.close();
    }
    
    /**
     * @brief 创建无效配置文件
     * 
     * Why: 需要创建无效配置文件，测试错误处理功能
     * What: CreateInvalidConfigFile方法创建包含无效配置项的文件
     * How: 使用ofstream写入无效配置项到文件
     */
    void CreateInvalidConfigFile() {
        std::ofstream config_file(invalid_config_file_);
        config_file << "# Invalid Configuration File\n";
        config_file << "invalid_line_without_equals\n";
        config_file << "another_invalid_line\n";
        config_file << "database.page_size = 4096\n";  // 有效行
        config_file << "[section_without_closing_bracket\n";  // 无效节
        config_file << "system.log_level = INFO\n";  // 有效行
        config_file.close();
    }
    
    /**
     * @brief 创建环境特定配置文件
     * 
     * Why: 需要创建环境特定配置文件，测试环境特定配置加载功能
     * What: CreateEnvConfigFile方法创建包含环境特定配置的测试文件
     * How: 使用ofstream写入环境特定配置项到文件
     */
    void CreateEnvConfigFile() {
        std::ofstream config_file(env_config_file_);
        config_file << "# Environment-specific Configuration for test\n";
        config_file << "database.buffer_pool_size = 4096\n";
        config_file << "system.log_level = DEBUG\n";
        config_file.close();
    }
    
    // 测试文件路径
    std::filesystem::path temp_dir_;
    std::filesystem::path basic_config_file_;
    std::filesystem::path complex_config_file_;
    std::filesystem::path invalid_config_file_;
    std::filesystem::path env_config_file_;
};

/**
 * @brief 测试加载默认配置
 * 
 * Why: 需要验证加载默认配置功能是否正确工作
 * What: 测试LoadDefaultConfig方法能否正确设置默认配置值
 * How: 创建新的ConfigManager实例，验证默认配置值是否正确设置
 */
TEST_F(ConfigManagerEnhancedTest, LoadDefaultConfig) {
    ConfigManager& config = ConfigManager::GetInstance();
    
    // 先加载默认配置，确保ConfigManager处于初始状态
    config.LoadDefaultConfig();
    
    // 不加载任何配置文件，直接检查默认配置值
    EXPECT_EQ(config.GetString("database.db_file_path"), "./data/sqlcc.db");
    EXPECT_EQ(config.GetInt("database.db_file_size_limit"), 1024);
    EXPECT_EQ(config.GetInt("database.page_size"), 8192);
    
    EXPECT_EQ(config.GetInt("buffer_pool.pool_size"), 64);
    EXPECT_EQ(config.GetString("buffer_pool.replacement_policy"), "LRU");
    EXPECT_EQ(config.GetString("buffer_pool.prefetch_strategy"), "SEQUENTIAL");
    EXPECT_EQ(config.GetInt("buffer_pool.prefetch_window"), 4);
    EXPECT_EQ(config.GetInt("buffer_pool.flush_interval"), 30);
    EXPECT_DOUBLE_EQ(config.GetDouble("buffer_pool.dirty_page_threshold"), 0.75);
    
    EXPECT_EQ(config.GetInt("disk_manager.io_thread_pool_size"), 4);
    EXPECT_EQ(config.GetInt("disk_manager.batch_read_size"), 8);
    EXPECT_EQ(config.GetInt("disk_manager.batch_write_size"), 8);
    EXPECT_TRUE(config.GetBool("disk_manager.async_io"));
    EXPECT_FALSE(config.GetBool("disk_manager.direct_io"));
    EXPECT_EQ(config.GetString("disk_manager.io_scheduler"), "FIFO");
    
    EXPECT_EQ(config.GetString("storage_engine.concurrency_control"), "PESSIMISTIC");
    EXPECT_EQ(config.GetInt("storage_engine.lock_timeout"), 5000);
    EXPECT_EQ(config.GetInt("storage_engine.deadlock_detection_interval"), 1000);
    EXPECT_EQ(config.GetString("storage_engine.isolation_level"), "READ_COMMITTED");
    EXPECT_EQ(config.GetInt("storage_engine.checkpoint_interval"), 60);
    
    EXPECT_EQ(config.GetString("logging.log_level"), "INFO");
    EXPECT_EQ(config.GetString("logging.log_file_path"), "./logs/sqlcc.log");
    EXPECT_EQ(config.GetInt("logging.log_file_size_limit"), 100);
    EXPECT_EQ(config.GetInt("logging.log_file_backup_count"), 5);
    EXPECT_TRUE(config.GetBool("logging.log_to_console"));
    
    EXPECT_FALSE(config.GetBool("performance.enable_monitoring"));
    EXPECT_EQ(config.GetInt("performance.stats_interval"), 10);
    EXPECT_EQ(config.GetString("performance.stats_output_path"), "./stats/");
    EXPECT_FALSE(config.GetBool("performance.enable_profiling"));
    
    EXPECT_FALSE(config.GetBool("testing.test_mode"));
    EXPECT_EQ(config.GetString("testing.test_data_dir"), "./test_data/");
    EXPECT_EQ(config.GetString("testing.test_output_dir"), "./test_results/");
    EXPECT_FALSE(config.GetBool("testing.verbose_test_log"));
}

/**
 * @brief 测试配置文件解析
 * 
 * Why: 需要验证配置文件解析功能是否正确工作
 * What: 测试ParseConfigFile和ParseConfigLine方法能否正确解析配置文件
 * How: 创建复杂配置文件，加载并验证配置值是否正确解析
 */
TEST_F(ConfigManagerEnhancedTest, ParseConfigFile) {
    ConfigManager& config = ConfigManager::GetInstance();
    
    // 加载复杂配置文件
    bool result = config.LoadConfig(complex_config_file_.string());
    EXPECT_TRUE(result);
    
    // 验证配置值是否正确解析
    EXPECT_EQ(config.GetInt("database.page_size"), 8192);
    EXPECT_EQ(config.GetInt("database.buffer_pool_size"), 2048);
    EXPECT_FALSE(config.GetBool("database.enable_logging"));
    
    EXPECT_EQ(config.GetInt("performance.max_threads"), 16);
    EXPECT_DOUBLE_EQ(config.GetDouble("performance.query_timeout"), 60.0);
    
    EXPECT_EQ(config.GetString("system.log_level"), "DEBUG");
    EXPECT_EQ(config.GetString("system.log_file"), "/var/log/sqlcc.log");
}

/**
 * @brief 测试无效配置文件处理
 * 
 * Why: 需要验证无效配置文件处理功能是否正确工作
 * What: 测试ParseConfigFile方法能否正确处理无效配置文件
 * How: 创建无效配置文件，加载并验证是否能正确处理
 */
TEST_F(ConfigManagerEnhancedTest, ParseInvalidConfigFile) {
    ConfigManager& config = ConfigManager::GetInstance();
    
    // 加载无效配置文件
    bool result = config.LoadConfig(invalid_config_file_.string());
    EXPECT_TRUE(result);  // 即使有无效行，也应该成功解析有效行
    
    // 验证有效配置值是否正确解析
    EXPECT_EQ(config.GetInt("database.page_size"), 4096);
    EXPECT_EQ(config.GetString("system.log_level"), "INFO");
}

/**
 * @brief 测试配置重新加载
 * 
 * Why: 需要验证配置重新加载功能是否正确工作
 * What: 测试ReloadConfig方法能否正确重新加载配置
 * How: 修改配置文件，调用ReloadConfig方法，验证配置值是否更新
 */
TEST_F(ConfigManagerEnhancedTest, ReloadConfig) {
    ConfigManager& config = ConfigManager::GetInstance();
    
    // 加载基本配置文件
    config.LoadConfig(basic_config_file_.string());
    
    // 验证初始配置值
    EXPECT_EQ(config.GetInt("database.page_size"), 4096);
    EXPECT_EQ(config.GetString("system.log_level"), "INFO");
    
    // 修改配置文件
    std::ofstream config_file(basic_config_file_, std::ios::app);
    config_file << "database.page_size = 8192\n";
    config_file << "system.log_level = DEBUG\n";
    config_file.close();
    
    // 重新加载配置
    bool reload_result = config.ReloadConfig();
    EXPECT_TRUE(reload_result);
    
    // 验证配置值是否更新
    EXPECT_EQ(config.GetInt("database.page_size"), 8192);
    EXPECT_EQ(config.GetString("system.log_level"), "DEBUG");
}

/**
 * @brief 测试配置保存
 * 
 * Why: 需要验证配置保存功能是否正确工作
 * What: 测试SaveToFile方法能否正确保存配置到文件
 * How: 修改配置值，保存到文件，然后重新加载验证
 */
TEST_F(ConfigManagerEnhancedTest, SaveToFile) {
    ConfigManager& config = ConfigManager::GetInstance();
    
    // 加载基本配置文件
    config.LoadConfig(basic_config_file_.string());
    
    // 修改配置值
    config.SetValue("database.page_size", 8192);
    config.SetValue("test.new.key", "new_value");
    
    // 保存到新文件
    std::filesystem::path save_file = temp_dir_ / "saved_config.conf";
    bool save_result = config.SaveToFile(save_file.string());
    EXPECT_TRUE(save_result);
    
    // 验证保存的文件内容
    std::ifstream file(save_file.string());
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    file.close();
    
    // 检查文件内容是否包含配置项
    // SaveToFile方法会按节组织输出
    // database.page_size会被写入[database]节下，显示为"page_size = 8192"
    // test.new.key会被写入[test]节下，显示为"new.key = new_value"
    EXPECT_NE(content.find("[database]"), std::string::npos);
    EXPECT_NE(content.find("page_size = 8192"), std::string::npos);
    EXPECT_NE(content.find("[test]"), std::string::npos);
    EXPECT_NE(content.find("new.key = new_value"), std::string::npos);
    
    // 重新加载保存的配置文件
    ConfigManager& new_config = ConfigManager::GetInstance();
    bool load_result = new_config.LoadConfig(save_file.string());
    EXPECT_TRUE(load_result);
    
    // 验证保存的配置值
    EXPECT_EQ(new_config.GetInt("database.page_size"), 8192);
    EXPECT_EQ(new_config.GetString("test.new.key"), "new_value");
}

/**
 * @brief 测试获取所有配置键
 * 
 * Why: 需要验证获取所有配置键功能是否正确工作
 * What: 测试GetAllKeys方法能否正确返回所有配置键
 * How: 加载配置文件，添加新配置，调用GetAllKeys验证返回的键
 */
TEST_F(ConfigManagerEnhancedTest, GetAllKeys) {
    ConfigManager& config = ConfigManager::GetInstance();
    
    // 加载基本配置文件
    config.LoadConfig(basic_config_file_.string());
    
    // 添加新配置
    config.SetValue("test.key1", "value1");
    config.SetValue("test.key2", "value2");
    
    // 获取所有键
    std::vector<std::string> all_keys = config.GetAllKeys();
    
    // 验证键的数量和内容
    EXPECT_GE(all_keys.size(), 8); // 至少包含原始配置的6个键和新增的2个键
    
    // 验证特定键是否存在
    EXPECT_TRUE(std::find(all_keys.begin(), all_keys.end(), "database.page_size") != all_keys.end());
    EXPECT_TRUE(std::find(all_keys.begin(), all_keys.end(), "system.log_level") != all_keys.end());
    EXPECT_TRUE(std::find(all_keys.begin(), all_keys.end(), "test.key1") != all_keys.end());
    EXPECT_TRUE(std::find(all_keys.begin(), all_keys.end(), "test.key2") != all_keys.end());
}

/**
 * @brief 测试获取指定前缀的配置键
 * 
 * Why: 需要验证获取指定前缀的配置键功能是否正确工作
 * What: 测试GetKeysWithPrefix方法能否正确返回指定前缀的配置键
 * How: 加载配置文件，添加新配置，调用GetKeysWithPrefix验证返回的键
 */
TEST_F(ConfigManagerEnhancedTest, GetKeysWithPrefix) {
    ConfigManager& config = ConfigManager::GetInstance();
    
    // 加载基本配置文件
    config.LoadConfig(basic_config_file_.string());
    
    // 添加新配置
    config.SetValue("test.key1", "value1");
    config.SetValue("test.key2", "value2");
    config.SetValue("other.key", "value");
    
    // 获取database前缀的键
    std::vector<std::string> db_keys = config.GetKeysWithPrefix("database.");
    // database相关的键可能包括默认配置中的其他键
    EXPECT_GE(db_keys.size(), 3); // 至少包含database.page_size, database.buffer_pool_size, database.enable_logging
    
    // 验证键的内容
    EXPECT_TRUE(std::find(db_keys.begin(), db_keys.end(), "database.page_size") != db_keys.end());
    EXPECT_TRUE(std::find(db_keys.begin(), db_keys.end(), "database.buffer_pool_size") != db_keys.end());
    EXPECT_TRUE(std::find(db_keys.begin(), db_keys.end(), "database.enable_logging") != db_keys.end());
    
    // 获取test前缀的键
    std::vector<std::string> test_keys = config.GetKeysWithPrefix("test.");
    EXPECT_EQ(test_keys.size(), 2);
    
    // 验证键的内容
    EXPECT_TRUE(std::find(test_keys.begin(), test_keys.end(), "test.key1") != test_keys.end());
    EXPECT_TRUE(std::find(test_keys.begin(), test_keys.end(), "test.key2") != test_keys.end());
    
    // 获取不存在的prefix
    std::vector<std::string> empty_keys = config.GetKeysWithPrefix("nonexistent.");
    EXPECT_TRUE(empty_keys.empty());
}

/**
 * @brief 测试配置变更通知
 * 
 * Why: 需要验证配置变更通知功能是否正确工作
 * What: 测试NotifyConfigChange方法能否正确通知注册的回调函数
 * How: 注册多个回调函数，修改配置值，验证所有回调是否被调用
 */
// 配置变更通知功能已被移除，相关测试用例已删除
// TEST_F(ConfigManagerEnhancedTest, NotifyConfigChange) {
//     // 此测试用例依赖于已移除的回调功能
// }

/**
 * @brief 测试配置变更回调异常处理
 * 
 * Why: 需要验证配置变更回调异常处理功能是否正确工作
 * What: 测试NotifyConfigChange方法能否正确处理回调函数中的异常
 * How: 注册会抛出异常的回调函数，修改配置值，验证异常是否被正确处理
 */
// 配置变更回调异常处理功能已被移除，相关测试用例已删除
// TEST_F(ConfigManagerEnhancedTest, CallbackExceptionHandling) {
//     // 此测试用例依赖于已移除的回调功能
// }

/**
 * @brief 测试配置文件不存在的情况
 * 
 * Why: 需要验证配置文件不存在时的处理
 * What: 测试LoadConfig方法能否正确处理不存在的配置文件
 * How: 尝试加载不存在的配置文件，验证返回值
 */
TEST_F(ConfigManagerEnhancedTest, LoadNonExistentFile) {
    ConfigManager& config = ConfigManager::GetInstance();
    
    // 尝试加载不存在的配置文件
    std::string non_existent_file = (temp_dir_ / "non_existent.conf").string();
    bool result = config.LoadConfig(non_existent_file);
    
    // 验证返回值
    EXPECT_FALSE(result);
}

/**
 * @brief 测试保存配置到不存在的目录
 * 
 * Why: 需要验证保存配置到不存在目录时的处理
 * What: 测试SaveToFile方法能否正确处理不存在的目录
 * How: 尝试保存到不存在的目录，验证返回值
 */
TEST_F(ConfigManagerEnhancedTest, SaveToNonExistentDirectory) {
    ConfigManager& config = ConfigManager::GetInstance();
    
    // 加载基本配置文件
    config.LoadConfig(basic_config_file_.string());
    
    // 尝试保存到不存在的目录
    std::string non_existent_dir = (temp_dir_ / "non_existent_dir" / "config.conf").string();
    bool result = config.SaveToFile(non_existent_dir);
    
    // 验证返回值
    EXPECT_FALSE(result);
}

}  // namespace test
}  // namespace sqlcc