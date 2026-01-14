/**
 * @file logger_test.cpp
 * @brief Logger单元测试
 *
 * 测试Logger类的核心功能，包括：
 * - 单例模式
 * - 日志级别设置
 * - 日志文件设置
 * - 各种日志方法（Debug, Info, Warn, Error）
 * - 右值引用重载
 * - 日志级别过滤
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <filesystem>
#include <fstream>
#include <string>
#include <sstream>
#include <thread>

#include "../../include/utils/logger.h"

// 使用sqlcc命名空间
using namespace sqlcc;

/**
 * @brief Logger测试套件
 */
class LoggerTest : public ::testing::Test {
protected:
    /**
     * @brief 测试前设置
     */
    void SetUp() override {
        // 创建临时日志文件
        temp_log_path_ = std::filesystem::temp_directory_path() / "test_log.txt";
        // 确保文件不存在
        if (std::filesystem::exists(temp_log_path_)) {
            std::filesystem::remove(temp_log_path_);
        }
        // 重置Logger实例状态
        ResetLoggerState();
    }

    /**
     * @brief 测试后清理
     */
    void TearDown() override {
        // 清理临时文件
        if (std::filesystem::exists(temp_log_path_)) {
            std::filesystem::remove(temp_log_path_);
        }
        // 重置Logger状态
        ResetLoggerState();
    }

    /**
     * @brief 重置Logger实例状态
     */
    void ResetLoggerState() {
        Logger& logger = Logger::GetInstance();
        // 重置日志级别
        logger.SetLogLevel(LogLevel::INFO);
        // 关闭文件日志（如果有）
        // 注意：Logger没有提供直接关闭文件的方法，所以我们通过重新设置一个无效文件来模拟
        logger.SetLogFile("/dev/null");
    }

    std::filesystem::path temp_log_path_;
};

/**
 * @brief 测试单例模式
 */
TEST_F(LoggerTest, SingletonPattern) {
    // 获取两个Logger实例引用
    Logger& instance1 = Logger::GetInstance();
    Logger& instance2 = Logger::GetInstance();

    // 验证是同一个实例
    EXPECT_EQ(&instance1, &instance2);
}

/**
 * @brief 测试日志级别设置
 */
TEST_F(LoggerTest, SetLogLevel) {
    Logger& logger = Logger::GetInstance();

    // 设置不同的日志级别
    logger.SetLogLevel(LogLevel::DEBUG);
    logger.SetLogLevel(LogLevel::INFO);
    logger.SetLogLevel(LogLevel::WARN);
    logger.SetLogLevel(LogLevel::ERROR);

    // 注意：Logger没有提供GetLogLevel方法，所以我们通过验证日志过滤来间接测试
}

/**
 * @brief 测试日志文件设置
 */
TEST_F(LoggerTest, SetLogFile) {
    Logger& logger = Logger::GetInstance();

    // 设置日志文件
    logger.SetLogFile(temp_log_path_.string());

    // 验证文件是否被创建
    EXPECT_TRUE(std::filesystem::exists(temp_log_path_));

    // 写入一条日志
    logger.Info("Test log message");

    // 验证日志是否被写入文件
    std::ifstream log_file(temp_log_path_);
    std::string line;
    EXPECT_TRUE(std::getline(log_file, line));
    EXPECT_THAT(line, ::testing::HasSubstr("Test log message"));
    log_file.close();
}

/**
 * @brief 测试无效日志文件路径
 */
TEST_F(LoggerTest, SetLogFileInvalidPath) {
    Logger& logger = Logger::GetInstance();

    // 设置一个无效的日志文件路径
    // 注意：Logger应该能处理这种情况而不抛出异常
    EXPECT_NO_THROW({
        logger.SetLogFile("/nonexistent/path/that/cannot/exist.txt");
    });

    // 确保日志系统仍然可以工作（输出到控制台）
    EXPECT_NO_THROW({
        logger.Error("Test error message");
    });
}

/**
 * @brief 测试日志级别过滤
 */
TEST_F(LoggerTest, LogLevelFiltering) {
    Logger& logger = Logger::GetInstance();
    logger.SetLogFile(temp_log_path_.string());

    // 设置日志级别为WARN
    logger.SetLogLevel(LogLevel::WARN);

    // 写入不同级别的日志
    logger.Debug("This debug message should be filtered out");
    logger.Info("This info message should be filtered out");
    logger.Warn("This warn message should be logged");
    logger.Error("This error message should be logged");

    // 读取日志文件并验证
    std::ifstream log_file(temp_log_path_);
    std::string content((std::istreambuf_iterator<char>(log_file)), std::istreambuf_iterator<char>());
    log_file.close();

    // 验证过滤效果
    EXPECT_THAT(content, ::testing::Not(::testing::HasSubstr("This debug message")));
    EXPECT_THAT(content, ::testing::Not(::testing::HasSubstr("This info message")));
    EXPECT_THAT(content, ::testing::HasSubstr("This warn message"));
    EXPECT_THAT(content, ::testing::HasSubstr("This error message"));
}

/**
 * @brief 测试左值引用日志方法
 */
TEST_F(LoggerTest, LogMethodsLValue) {
    Logger& logger = Logger::GetInstance();
    logger.SetLogFile(temp_log_path_.string());
    logger.SetLogLevel(LogLevel::DEBUG);

    // 测试左值引用日志方法
    std::string debug_msg = "Debug message with lvalue";
    std::string info_msg = "Info message with lvalue";
    std::string warn_msg = "Warn message with lvalue";
    std::string error_msg = "Error message with lvalue";

    logger.Debug(debug_msg);
    logger.Info(info_msg);
    logger.Warn(warn_msg);
    logger.Error(error_msg);

    // 读取日志文件并验证
    std::ifstream log_file(temp_log_path_);
    std::string content((std::istreambuf_iterator<char>(log_file)), std::istreambuf_iterator<char>());
    log_file.close();

    // 验证所有日志都被写入
    EXPECT_THAT(content, ::testing::HasSubstr(debug_msg));
    EXPECT_THAT(content, ::testing::HasSubstr(info_msg));
    EXPECT_THAT(content, ::testing::HasSubstr(warn_msg));
    EXPECT_THAT(content, ::testing::HasSubstr(error_msg));
}

/**
 * @brief 测试右值引用日志方法
 */
TEST_F(LoggerTest, LogMethodsRValue) {
    Logger& logger = Logger::GetInstance();
    logger.SetLogFile(temp_log_path_.string());
    logger.SetLogLevel(LogLevel::DEBUG);

    // 测试右值引用日志方法
    logger.Debug(std::string("Debug message with rvalue"));
    logger.Info(std::string("Info message with rvalue"));
    logger.Warn(std::string("Warn message with rvalue"));
    logger.Error(std::string("Error message with rvalue"));

    // 读取日志文件并验证
    std::ifstream log_file(temp_log_path_);
    std::string content((std::istreambuf_iterator<char>(log_file)), std::istreambuf_iterator<char>());
    log_file.close();

    // 验证所有日志都被写入
    EXPECT_THAT(content, ::testing::HasSubstr("Debug message with rvalue"));
    EXPECT_THAT(content, ::testing::HasSubstr("Info message with rvalue"));
    EXPECT_THAT(content, ::testing::HasSubstr("Warn message with rvalue"));
    EXPECT_THAT(content, ::testing::HasSubstr("Error message with rvalue"));
}

/**
 * @brief 测试日志格式
 */
TEST_F(LoggerTest, LogFormat) {
    Logger& logger = Logger::GetInstance();
    logger.SetLogFile(temp_log_path_.string());
    logger.SetLogLevel(LogLevel::INFO);

    // 写入一条日志
    const std::string test_msg = "Test log format";
    logger.Info(test_msg);

    // 读取日志文件
    std::ifstream log_file(temp_log_path_);
    std::string line;
    std::getline(log_file, line);
    log_file.close();

    // 验证日志格式：[时间戳] [级别] 消息
    // 使用简单的字符串检查替代复杂的正则表达式
    EXPECT_THAT(line, ::testing::HasSubstr("[INFO]"));
    EXPECT_THAT(line, ::testing::HasSubstr("Test log format"));
    EXPECT_THAT(line, ::testing::HasSubstr("-")); // 年份和月份之间的分隔符
    EXPECT_THAT(line, ::testing::HasSubstr(":")); // 小时和分钟之间的分隔符
    EXPECT_THAT(line, ::testing::HasSubstr(".")); // 秒和毫秒之间的分隔符
}

/**
 * @brief 测试多线程环境下的日志记录
 */
TEST_F(LoggerTest, MultiThreadedLogging) {
    Logger& logger = Logger::GetInstance();
    logger.SetLogFile(temp_log_path_.string());
    logger.SetLogLevel(LogLevel::DEBUG);

    // 创建多个线程同时写入日志
    const int num_threads = 10;
    const int logs_per_thread = 10;
    std::vector<std::thread> threads;

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&logger, i, logs_per_thread]() {
            for (int j = 0; j < logs_per_thread; ++j) {
                logger.Debug("Thread " + std::to_string(i) + ", log " + std::to_string(j));
            }
        });
    }

    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }

    // 读取日志文件并验证
    std::ifstream log_file(temp_log_path_);
    int log_count = 0;
    std::string line;

    while (std::getline(log_file, line)) {
        log_count++;
        EXPECT_THAT(line, ::testing::HasSubstr("Thread"));
        EXPECT_THAT(line, ::testing::HasSubstr("log"));
    }
    log_file.close();

    // 验证所有日志都被写入
    EXPECT_EQ(log_count, num_threads * logs_per_thread);
}

/**
 * @brief 测试错误级别日志是否同时输出到控制台和文件
 */
TEST_F(LoggerTest, ErrorLogToBothConsoleAndFile) {
    Logger& logger = Logger::GetInstance();
    logger.SetLogFile(temp_log_path_.string());
    logger.SetLogLevel(LogLevel::ERROR);

    // 写入一条错误日志
    const std::string error_msg = "Test error message";
    logger.Error(error_msg);

    // 验证日志是否被写入文件
    std::ifstream log_file(temp_log_path_);
    std::string line;
    EXPECT_TRUE(std::getline(log_file, line));
    EXPECT_THAT(line, ::testing::HasSubstr(error_msg));
    log_file.close();

    // 注意：我们无法直接捕获控制台输出，所以这里只验证文件输出
}

/**
 * @brief 测试特殊字符处理
 */
TEST_F(LoggerTest, SpecialCharactersInLog) {
    Logger& logger = Logger::GetInstance();
    logger.SetLogFile(temp_log_path_.string());
    logger.SetLogLevel(LogLevel::DEBUG);

    // 测试包含特殊字符的日志
    std::string special_msg = "Special characters: !@#$%^&*()_+{}[]|\\:;'\"<>,.?/\nNew line";
    logger.Info(special_msg);

    // 读取日志文件并验证
    std::ifstream log_file(temp_log_path_);
    std::string content((std::istreambuf_iterator<char>(log_file)), std::istreambuf_iterator<char>());
    log_file.close();

    // 验证特殊字符处理（注意：换行符可能被处理）
    EXPECT_THAT(content, ::testing::HasSubstr("Special characters: !@#$%^&*()_+{}[]|\\:;'\"<>,.?/"));
    EXPECT_THAT(content, ::testing::HasSubstr("New line"));
}

/**
 * @brief 测试空日志消息
 */
TEST_F(LoggerTest, EmptyLogMessages) {
    Logger& logger = Logger::GetInstance();
    logger.SetLogFile(temp_log_path_.string());
    logger.SetLogLevel(LogLevel::DEBUG);

    // 测试空日志消息
    EXPECT_NO_THROW({
        logger.Debug("");
        logger.Info("");
        logger.Warn("");
        logger.Error("");
    });

    // 读取日志文件并验证
    std::ifstream log_file(temp_log_path_);
    int log_count = 0;
    std::string line;

    while (std::getline(log_file, line)) {
        log_count++;
    }
    log_file.close();

    // 验证所有空日志都被处理
    EXPECT_EQ(log_count, 4);
}

/**
 * @brief 测试大日志消息
 */
TEST_F(LoggerTest, LargeLogMessages) {
    Logger& logger = Logger::GetInstance();
    logger.SetLogFile(temp_log_path_.string());
    logger.SetLogLevel(LogLevel::DEBUG);

    // 创建一个大日志消息
    std::string large_msg(10000, 'X'); // 10000个'X'字符
    large_msg += " - End of large message";

    // 测试大日志消息
    EXPECT_NO_THROW({
        logger.Info(large_msg);
    });

    // 读取日志文件并验证
    std::ifstream log_file(temp_log_path_);
    std::string content((std::istreambuf_iterator<char>(log_file)), std::istreambuf_iterator<char>());
    log_file.close();

    // 验证大日志消息被正确处理
    EXPECT_THAT(content, ::testing::HasSubstr("End of large message"));
}