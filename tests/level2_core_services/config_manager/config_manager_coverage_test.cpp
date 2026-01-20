#include <gtest/gtest.h>
#include <fstream>
#include <cstdio>
#include <filesystem>
#include "utils/config_manager.h"

using sqlcc::ConfigManager;
using namespace std::filesystem;

namespace fs = std::filesystem;

class ConfigManagerCoverageTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Reset singleton state for each test
        ConfigManager::GetInstance().ResetForTest();

        // Create a temporary config file for testing
        temp_config_file = "test_config.ini";
        CleanupTempFile();

        // Create test config content
        std::ofstream config_file(temp_config_file);
        config_file << "# Test configuration file\n";
        config_file << "[database]\n";
        config_file << "port=5432\n";
        config_file << "host=localhost\n";
        config_file << "timeout=30000\n";
        config_file << "debug=true\n";
        config_file << "max_connections=100\n";
        config_file << "\n";
        config_file << "[storage]\n";
        config_file << "buffer_size=1048576\n";
        config_file << "compression_level=6\n";
        config_file << "enable_logging=false\n";
        config_file.close();
    }

    void TearDown() override {
        ConfigManager::GetInstance().ResetForTest();
        CleanupTempFile();
    }

    void CleanupTempFile() {
        if (fs::exists(temp_config_file)) {
            fs::remove(temp_config_file);
        }
    }

    std::string temp_config_file;
};

// Test loading configuration from file
TEST_F(ConfigManagerCoverageTest, LoadConfigFromFile) {
    auto& config = ConfigManager::GetInstance();

    // Test loading config file
    bool result = config.LoadConfig(temp_config_file, "test");
    EXPECT_TRUE(result);

    // Verify loaded values
    EXPECT_EQ(config.GetInt("database.port"), 5432);
    EXPECT_EQ(config.GetString("database.host"), "localhost");
    EXPECT_EQ(config.GetInt("database.timeout"), 30000);
    EXPECT_TRUE(config.GetBool("database.debug"));
    EXPECT_EQ(config.GetInt("database.max_connections"), 100);
}

// Test configuration value type conversions
TEST_F(ConfigManagerCoverageTest, TypeConversions) {
    auto& config = ConfigManager::GetInstance();

    // Set various types of values
    config.SetValue("test.int", 42);
    config.SetValue("test.double", 3.14159);
    config.SetValue("test.bool_true", true);
    config.SetValue("test.bool_false", false);
    config.SetValue("test.string", "hello world");

    // Test int conversions
    EXPECT_EQ(config.GetInt("test.int"), 42);
    EXPECT_EQ(config.GetInt("test.double"), 3);  // double to int
    EXPECT_EQ(config.GetInt("test.bool_true"), 1);
    EXPECT_EQ(config.GetInt("test.bool_false"), 0);
    EXPECT_EQ(config.GetInt("test.string"), 0);  // invalid conversion, should return default
    EXPECT_EQ(config.GetInt("nonexistent", 999), 999);  // default value

    // Test double conversions
    EXPECT_DOUBLE_EQ(config.GetDouble("test.double"), 3.14159);
    EXPECT_DOUBLE_EQ(config.GetDouble("test.int"), 42.0);
    EXPECT_DOUBLE_EQ(config.GetDouble("test.string"), 0.0);  // invalid conversion
    EXPECT_DOUBLE_EQ(config.GetDouble("nonexistent", 123.45), 123.45);

    // Test bool conversions
    EXPECT_TRUE(config.GetBool("test.bool_true"));
    EXPECT_FALSE(config.GetBool("test.bool_false"));
    EXPECT_TRUE(config.GetBool("test.int"));  // non-zero int -> true
    EXPECT_FALSE(config.GetBool("test.string"));  // invalid string -> false

    // Test string conversions
    EXPECT_EQ(config.GetString("test.string"), "hello world");
    EXPECT_EQ(config.GetString("test.int"), "42");
    EXPECT_EQ(config.GetString("test.double"), "3.14159");  // Note: precision formatting
    EXPECT_EQ(config.GetString("test.bool_true"), "1");
    EXPECT_EQ(config.GetString("test.bool_false"), "0");
}

// Test configuration parsing with sections
TEST_F(ConfigManagerCoverageTest, ParseConfigWithSections) {
    auto& config = ConfigManager::GetInstance();

    // Create config with sections
    std::ofstream config_file("section_test.ini");
    config_file << "[section1]\n";
    config_file << "key1=value1\n";
    config_file << "key2=123\n";
    config_file << "\n";
    config_file << "[section2]\n";
    config_file << "key1=value2\n";
    config_file << "key3=true\n";
    config_file.close();

    config.LoadConfig("section_test.ini", "test");

    // Test section parsing
    EXPECT_EQ(config.GetString("section1.key1"), "value1");
    EXPECT_EQ(config.GetInt("section1.key2"), 123);
    EXPECT_EQ(config.GetString("section2.key1"), "value2");
    EXPECT_TRUE(config.GetBool("section2.key3"));

    // Cleanup
    fs::remove("section_test.ini");
}

// Test configuration reloading
TEST_F(ConfigManagerCoverageTest, ReloadConfig) {
    auto& config = ConfigManager::GetInstance();

    // Load initial config
    bool result = config.LoadConfig(temp_config_file, "test");
    EXPECT_TRUE(result);

    // Reload config - verify the method can be called and doesn't crash
    bool reload_result = config.ReloadConfig();
    EXPECT_TRUE(reload_result);

    // Verify config is still loaded after reload
    EXPECT_EQ(config.GetInt("database.port"), 5432);
}

// Test configuration reloading with new file (complete replacement)
TEST_F(ConfigManagerCoverageTest, ReloadConfigWithNewFile) {
    auto& config = ConfigManager::GetInstance();

    // Load initial config
    config.LoadConfig(temp_config_file, "test");
    EXPECT_EQ(config.GetInt("database.port"), 5432);
    EXPECT_EQ(config.GetString("database.host"), "localhost");
    EXPECT_TRUE(config.HasKey("database.host"));

    // Create new config file with different values
    std::string new_config_file = "new_test_config.ini";
    std::ofstream new_config(new_config_file);
    new_config << "[database]\n";
    new_config << "port=8080\n";
    new_config << "new.key=999\n";
    new_config << "\n";
    new_config << "[newsection]\n";
    new_config << "newvalue=test\n";
    new_config.close();

    // Reload with new config file - should completely replace configuration
    bool reload_result = config.ReloadConfig(new_config_file);
    EXPECT_TRUE(reload_result);

    // Verify new values are present
    EXPECT_EQ(config.GetInt("database.port"), 8080);
    EXPECT_EQ(config.GetInt("database.new.key"), 999);  // Note: this becomes database.new.key due to section
    EXPECT_EQ(config.GetString("newsection.newvalue"), "test");

    // Verify old values are completely gone (full replacement)
    EXPECT_FALSE(config.HasKey("database.host"));  // Old key should be gone
    EXPECT_FALSE(config.HasKey("database.debug"));  // Old key should be gone
    EXPECT_FALSE(config.HasKey("storage.buffer_size"));  // Old key should be gone

    // Cleanup
    fs::remove(new_config_file);
}

// Test configuration saving
TEST_F(ConfigManagerCoverageTest, SaveConfigToFile) {
    auto& config = ConfigManager::GetInstance();

    // Set some values
    config.SetValue("save.test.key1", "value1");
    config.SetValue("save.test.key2", 42);
    config.SetValue("save.test.key3", true);
    config.SetValue("save.test.key4", 3.14);

    // Save to file
    std::string save_file = "save_test.ini";
    bool save_result = config.SaveToFile(save_file);
    EXPECT_TRUE(save_result);

    // Load into same instance to verify (since it's a singleton)
    ConfigManager::GetInstance().ClearAll();
    ConfigManager::GetInstance().LoadConfig(save_file, "test");

    EXPECT_EQ(ConfigManager::GetInstance().GetString("save.test.key1"), "value1");
    EXPECT_EQ(ConfigManager::GetInstance().GetInt("save.test.key2"), 42);
    EXPECT_TRUE(ConfigManager::GetInstance().GetBool("save.test.key3"));
    EXPECT_DOUBLE_EQ(ConfigManager::GetInstance().GetDouble("save.test.key4"), 3.14);

    // Cleanup
    fs::remove(save_file);
}

// Test error handling for non-existent files
TEST_F(ConfigManagerCoverageTest, LoadNonExistentFile) {
    auto& config = ConfigManager::GetInstance();

    // Try to load non-existent file - should not crash and load defaults
    bool result = config.LoadConfig("nonexistent_file.ini", "test");
    EXPECT_TRUE(result);  // Should succeed and load defaults

    // Check that default values are loaded
    EXPECT_EQ(config.GetInt("buffer_pool.read_lock_timeout_ms"), 2000);
    EXPECT_EQ(config.GetInt("buffer_pool.write_lock_timeout_ms"), 5000);
}

// Test HasKey functionality
TEST_F(ConfigManagerCoverageTest, HasKey) {
    auto& config = ConfigManager::GetInstance();

    config.SetValue("test.exists", "value");
    EXPECT_TRUE(config.HasKey("test.exists"));
    EXPECT_FALSE(config.HasKey("test.nonexistent"));
}

// Test GetAllKeys and GetKeysWithPrefix
TEST_F(ConfigManagerCoverageTest, GetKeys) {
    auto& config = ConfigManager::GetInstance();

    config.SetValue("app.database.host", "localhost");
    config.SetValue("app.database.port", 5432);
    config.SetValue("app.cache.enabled", true);
    config.SetValue("system.memory.limit", 1024);

    auto all_keys = config.GetAllKeys();
    EXPECT_GE(all_keys.size(), 4);  // At least our test keys

    auto db_keys = config.GetKeysWithPrefix("app.database");
    EXPECT_EQ(db_keys.size(), 2);
    EXPECT_TRUE(std::find(db_keys.begin(), db_keys.end(), "app.database.host") != db_keys.end());
    EXPECT_TRUE(std::find(db_keys.begin(), db_keys.end(), "app.database.port") != db_keys.end());
}

// Test operation timeout functionality
TEST_F(ConfigManagerCoverageTest, OperationTimeout) {
    auto& config = ConfigManager::GetInstance();

    // Test default timeout
    EXPECT_EQ(config.GetOperationTimeout(), 30000);  // kDefaultOperationTimeoutMs

    // Set custom timeout
    config.SetOperationTimeout(5000);
    EXPECT_EQ(config.GetOperationTimeout(), 5000);
}

// Test singleton pattern
TEST_F(ConfigManagerCoverageTest, SingletonPattern) {
    auto& instance1 = ConfigManager::GetInstance();
    auto& instance2 = ConfigManager::GetInstance();

    // Should be the same instance
    EXPECT_EQ(&instance1, &instance2);

    // Modify through one reference, check through other
    instance1.SetValue("singleton.test", "value");
    EXPECT_EQ(instance2.GetString("singleton.test"), "value");
}

// Test that we cannot create new instances (constructor is private)
// This test verifies the singleton pattern by ensuring compilation would fail
// if we tried to create a new instance directly
TEST_F(ConfigManagerCoverageTest, SingletonEnforcement) {
    // This test passes by compilation - we cannot create new instances
    // The singleton pattern is enforced by making the constructor private
    auto& instance1 = ConfigManager::GetInstance();
    auto& instance2 = ConfigManager::GetInstance();

    // Both should point to the same instance
    EXPECT_EQ(&instance1, &instance2);
}

// Test parsing edge cases
TEST_F(ConfigManagerCoverageTest, ParseEdgeCases) {
    auto& config = ConfigManager::GetInstance();

    // Test file with comments and empty lines
    std::ofstream edge_config("edge_test.ini");
    edge_config << "# This is a comment\n";
    edge_config << "\n";  // Empty line
    edge_config << "; Another comment\n";
    edge_config << "key1=value1\n";
    edge_config << "\n";  // Another empty line
    edge_config << "key2=value2\n";
    edge_config.close();

    config.LoadConfig("edge_test.ini", "test");
    EXPECT_EQ(config.GetString("key1"), "value1");
    EXPECT_EQ(config.GetString("key2"), "value2");

    // Cleanup
    fs::remove("edge_test.ini");
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
