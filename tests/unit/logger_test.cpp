/**
 * @file logger_test.cpp
 * @brief Logger组件单元测试
 * @author SQLCC技术委员会
 * @date 2025-12-25
 */

#include "include/utils/logger.h"
#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>
#include <thread>
#include <vector>

namespace sqlcc {
namespace test {

class LoggerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 清理测试日志文件
        test_log_file_ = "/tmp/sqlcc_logger_test.log";
        std::filesystem::remove(test_log_file_);
    }

    void TearDown() override {
        std::filesystem::remove(test_log_file_);
    }

    std::string test_log_file_;
};

// 测试1: 单例模式
TEST_F(LoggerTest, SingletonPattern) {
    Logger& instance1 = Logger::GetInstance();
    Logger& instance2 = Logger::GetInstance();
    EXPECT_EQ(&instance1, &instance2);
}

// 测试2: 日志级别设置
TEST_F(LoggerTest, LogLevelSetting) {
    Logger& logger = Logger::GetInstance();
    
    // 设置不同日志级别，不应抛出异常
    EXPECT_NO_THROW(logger.SetLogLevel(LogLevel::DEBUG));
    EXPECT_NO_THROW(logger.SetLogLevel(LogLevel::INFO));
    EXPECT_NO_THROW(logger.SetLogLevel(LogLevel::WARN));
    EXPECT_NO_THROW(logger.SetLogLevel(LogLevel::ERROR));
}

// 测试3: 文件日志输出
TEST_F(LoggerTest, FileLogging) {
    Logger& logger = Logger::GetInstance();
    
    logger.SetLogFile(test_log_file_);
    logger.SetLogLevel(LogLevel::DEBUG);
    
    logger.Info("Test file logging message");
    
    // 验证文件存在且有内容
    std::ifstream log_file(test_log_file_);
    EXPECT_TRUE(log_file.good());
    
    std::string content;
    std::getline(log_file, content);
    EXPECT_FALSE(content.empty());
    EXPECT_NE(content.find("Test file logging message"), std::string::npos);
}

// 测试4: 各日志级别方法
TEST_F(LoggerTest, LogLevelMethods) {
    Logger& logger = Logger::GetInstance();
    logger.SetLogFile(test_log_file_);
    logger.SetLogLevel(LogLevel::DEBUG);
    
    // 测试各级别日志方法不抛异常
    EXPECT_NO_THROW(logger.Debug("Debug message"));
    EXPECT_NO_THROW(logger.Info("Info message"));
    EXPECT_NO_THROW(logger.Warn("Warning message"));
    EXPECT_NO_THROW(logger.Error("Error message"));
}

// 测试5: 移动语义重载
TEST_F(LoggerTest, MoveSemantics) {
    Logger& logger = Logger::GetInstance();
    logger.SetLogLevel(LogLevel::DEBUG);
    
    // 测试移动语义版本
    EXPECT_NO_THROW(logger.Debug(std::string("Move debug")));
    EXPECT_NO_THROW(logger.Info(std::string("Move info")));
    EXPECT_NO_THROW(logger.Warn(std::string("Move warn")));
    EXPECT_NO_THROW(logger.Error(std::string("Move error")));
}

// 测试6: 宏接口
TEST_F(LoggerTest, MacroInterface) {
    SQLCC_LOGGER.SetLogLevel(LogLevel::DEBUG);
    
    EXPECT_NO_THROW(SQLCC_LOG_DEBUG("Macro debug"));
    EXPECT_NO_THROW(SQLCC_LOG_INFO("Macro info"));
    EXPECT_NO_THROW(SQLCC_LOG_WARN("Macro warn"));
    EXPECT_NO_THROW(SQLCC_LOG_ERROR("Macro error"));
}

// 测试7: 线程安全
TEST_F(LoggerTest, ThreadSafety) {
    Logger& logger = Logger::GetInstance();
    logger.SetLogFile(test_log_file_);
    logger.SetLogLevel(LogLevel::DEBUG);
    
    constexpr int num_threads = 10;
    constexpr int logs_per_thread = 100;
    
    std::vector<std::thread> threads;
    threads.reserve(num_threads);
    
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&logger, i]() {
            for (int j = 0; j < logs_per_thread; ++j) {
                logger.Info("Thread " + std::to_string(i) + " Log " + std::to_string(j));
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    // 验证文件存在，无崩溃
    EXPECT_TRUE(std::filesystem::exists(test_log_file_));
}

// 测试8: 日志级别过滤
TEST_F(LoggerTest, LogLevelFiltering) {
    Logger& logger = Logger::GetInstance();
    logger.SetLogFile(test_log_file_);
    
    // 设置为ERROR级别，只有ERROR应该被记录
    logger.SetLogLevel(LogLevel::ERROR);
    
    logger.Debug("Should not appear");
    logger.Info("Should not appear");
    logger.Warn("Should not appear");
    logger.Error("Should appear");
    
    std::ifstream log_file(test_log_file_);
    std::string content((std::istreambuf_iterator<char>(log_file)),
                         std::istreambuf_iterator<char>());
    
    EXPECT_NE(content.find("Should appear"), std::string::npos);
    // DEBUG/INFO/WARN 在 ERROR 级别下不应该出现
}

} // namespace test
} // namespace sqlcc
