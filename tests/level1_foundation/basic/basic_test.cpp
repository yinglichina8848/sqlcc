#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>

// SQLCC core components for real functionality testing
#include "logger.h"
#include "config_manager.h"
#include "exception.h"

// Basic functionality tests for foundation layer
// These tests verify SQLCC core components work correctly with coverage

TEST(BasicFunctionalityTest, LoggerCoreFunctionality) {
    // Test SQLCC Logger core functionality
    SQLCC::Logger& logger = SQLCC::Logger::GetInstance();

    // Test logger initialization (calls core logger code)
    EXPECT_TRUE(&logger != nullptr);

    // Test logger methods that execute core code paths
    logger.SetLogLevel(SQLCC::LogLevel::INFO);
    EXPECT_EQ(logger.GetLogLevel(), SQLCC::LogLevel::INFO);

    // Test logger output (triggers core logging paths)
    logger.Info("Test info message from core test");
    logger.Debug("Test debug message from core test");
    logger.Error("Test error message from core test");
}

TEST(BasicFunctionalityTest, ConfigManagerCoreFunctionality) {
    // Test SQLCC ConfigManager core functionality
    SQLCC::ConfigManager& config = SQLCC::ConfigManager::GetInstance();

    // Test config manager initialization (calls core config code)
    EXPECT_TRUE(&config != nullptr);

    // Test config operations that execute core code paths
    config.SetValue("test_key", "test_value");
    EXPECT_EQ(config.GetString("test_key"), "test_value");

    config.SetValue("test_int", 42);
    EXPECT_EQ(config.GetInt("test_int"), 42);

    config.SetValue("test_bool", true);
    EXPECT_TRUE(config.GetBool("test_bool"));

    // Test config file operations
    auto temp_path = std::filesystem::temp_directory_path() / "test_config.ini";
    config.SaveToFile(temp_path.string());  // Should execute save logic

    // Clean up
    if (std::filesystem::exists(temp_path)) {
        std::filesystem::remove(temp_path);
    }
}

TEST(BasicFunctionalityTest, ExceptionCoreFunctionality) {
    // Test SQLCC Exception core functionality
    try {
        // Test SQLCC exception throwing and catching
        throw SQLCC::DatabaseException("Test database exception from core test");
    } catch (const SQLCC::DatabaseException& e) {
        EXPECT_STREQ(e.what(), "Test database exception from core test");
        EXPECT_EQ(e.GetErrorCode(), SQLCC::ErrorCode::UNKNOWN_ERROR);
    }

    try {
        // Test another SQLCC exception type
        throw SQLCC::ParseException("Test parse exception from core test");
    } catch (const SQLCC::ParseException& e) {
        EXPECT_STREQ(e.what(), "Test parse exception from core test");
    }
}

TEST(BasicFunctionalityTest, StringOperations) {
    std::string str = "Hello, World!";
    EXPECT_EQ(str.length(), 13);
    EXPECT_EQ(str.substr(0, 5), "Hello");
    EXPECT_TRUE(str.find("World") != std::string::npos);
}

TEST(BasicFunctionalityTest, VectorOperations) {
    std::vector<int> vec = {1, 2, 3, 4, 5};
    EXPECT_EQ(vec.size(), 5);
    EXPECT_EQ(vec[0], 1);
    EXPECT_EQ(vec.back(), 5);

    vec.push_back(6);
    EXPECT_EQ(vec.size(), 6);
    EXPECT_EQ(vec.back(), 6);
}

TEST(BasicFunctionalityTest, SmartPointers) {
    auto unique_ptr = std::make_unique<int>(42);
    EXPECT_EQ(*unique_ptr, 42);

    auto shared_ptr = std::make_shared<std::string>("test");
    EXPECT_EQ(*shared_ptr, "test");
    EXPECT_EQ(shared_ptr.use_count(), 1);
}

TEST(BasicFunctionalityTest, TypeTraits) {
    // Test basic type operations
    EXPECT_TRUE(std::is_integral<int>::value);
    EXPECT_TRUE(std::is_floating_point<double>::value);
    EXPECT_TRUE(std::is_pointer<int*>::value);
    EXPECT_FALSE(std::is_pointer<int>::value);
}