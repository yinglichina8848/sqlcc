#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace sqlcc {

/**
 * @brief 日志级别枚举
 */
enum class LogLevel {
    DEBUG,
    INFO,
    WARN,
    ERROR
};

/**
 * @brief 简单日志记录器类
 */
class Logger {
public:
    /**
     * @brief 获取单例实例
     * @return Logger实例引用
     */
    static Logger& GetInstance() {
        static Logger instance;
        return instance;
    }

    /**
     * @brief 设置日志级别
     * @param level 日志级别
     */
    void SetLogLevel(LogLevel level) {
        log_level_ = level;
    }

    /**
     * @brief 设置日志文件
     * @param filename 日志文件名
     */
    void SetLogFile(const std::string& filename) {
        log_file_.open(filename, std::ios::out | std::ios::app);
        use_file_ = log_file_.is_open();
    }

    /**
     * @brief 记录调试日志
     * @param message 日志消息
     */
    void Debug(const std::string& message) {
        Log(LogLevel::DEBUG, message);
    }

    /**
     * @brief 记录信息日志
     * @param message 日志消息
     */
    void Info(const std::string& message) {
        Log(LogLevel::INFO, message);
    }

    /**
     * @brief 记录警告日志
     * @param message 日志消息
     */
    void Warn(const std::string& message) {
        Log(LogLevel::WARN, message);
    }

    /**
     * @brief 记录错误日志
     * @param message 日志消息
     */
    void Error(const std::string& message) {
        Log(LogLevel::ERROR, message);
    }

private:
    /**
     * @brief 私有构造函数
     */
    Logger() : log_level_(LogLevel::INFO), use_file_(false) {}

    /**
     * @brief 禁止拷贝构造
     */
    Logger(const Logger&) = delete;

    /**
     * @brief 禁止赋值操作
     */
    Logger& operator=(const Logger&) = delete;

    /**
     * @brief 记录日志的实际实现
     * @param level 日志级别
     * @param message 日志消息
     */
    void Log(LogLevel level, const std::string& message) {
        // 检查日志级别
        if (level < log_level_) {
            return;
        }

        // 获取当前时间戳
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;

        // 格式化时间戳
        std::ostringstream timestamp;
        timestamp << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        timestamp << '.' << std::setfill('0') << std::setw(3) << ms.count();

        // 获取日志级别字符串
        std::string level_str;
        switch (level) {
            case LogLevel::DEBUG:
                level_str = "DEBUG";
                break;
            case LogLevel::INFO:
                level_str = "INFO";
                break;
            case LogLevel::WARN:
                level_str = "WARN";
                break;
            case LogLevel::ERROR:
                level_str = "ERROR";
                break;
        }

        // 构造完整日志消息
        std::ostringstream log_msg;
        log_msg << "[" << timestamp.str() << "] [" << level_str << "] " << message;

        // 输出日志
        if (use_file_ && log_file_.is_open()) {
            log_file_ << log_msg.str() << std::endl;
            log_file_.flush();
        } else {
            // 根据日志级别选择输出流
            if (level == LogLevel::ERROR) {
                std::cerr << log_msg.str() << std::endl;
            } else {
                std::cout << log_msg.str() << std::endl;
            }
        }
    }

    /// 当前日志级别
    LogLevel log_level_;

    /// 日志文件流
    std::ofstream log_file_;

    /// 是否使用文件记录日志
    bool use_file_;
};

/**
 * @brief 获取日志记录器实例的宏定义
 */
#define SQLCC_LOGGER ::sqlcc::Logger::GetInstance()

/**
 * @brief 记录调试日志的宏定义
 */
#define SQLCC_LOG_DEBUG(msg) SQLCC_LOGGER.Debug(msg)

/**
 * @brief 记录信息日志的宏定义
 */
#define SQLCC_LOG_INFO(msg) SQLCC_LOGGER.Info(msg)

/**
 * @brief 记录警告日志的宏定义
 */
#define SQLCC_LOG_WARN(msg) SQLCC_LOGGER.Warn(msg)

/**
 * @brief 记录错误日志的宏定义
 */
#define SQLCC_LOG_ERROR(msg) SQLCC_LOGGER.Error(msg)

}  // namespace sqlcc