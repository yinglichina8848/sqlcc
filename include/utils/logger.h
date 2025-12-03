/**
 * @file logger.h
 * @brief 数据库系统日志记录器定义
 * 
 * Why: 需要一个统一的日志记录机制来记录系统运行状态、调试信息和错误信息
 * What: 定义了日志记录器类和相关宏，支持不同级别的日志记录和文件输出
 * How: 使用单例模式实现Logger类，提供多种日志级别和输出方式
 */

// Why: 防止头文件被多次包含，避免编译错误
// What: 使用#pragma once指令确保头文件只被编译一次
// How: 在文件开头添加#pragma once预处理指令
#pragma once

// Why: 需要使用标准输入输出流来输出日志信息
// What: 包含iostream头文件，提供std::cout和std::cerr功能
// How: 使用#include预处理指令包含标准库头文件
#include <iostream>

// Why: 需要使用文件流来将日志写入文件
// What: 包含fstream头文件，提供std::ofstream功能
// How: 使用#include预处理指令包含标准库头文件
#include <fstream>

// Why: 需要使用字符串类来存储日志消息
// What: 包含string头文件，提供std::string功能
// How: 使用#include预处理指令包含标准库头文件
#include <string>

// Why: 需要使用时间相关功能来记录日志时间戳
// What: 包含chrono头文件，提供时间点、时长等功能
// How: 使用#include预处理指令包含标准库头文件
#include <chrono>

// Why: 需要使用格式化功能来格式化时间戳
// What: 包含iomanip头文件，提供std::put_time、std::setfill等功能
// How: 使用#include预处理指令包含标准库头文件
#include <iomanip>

// Why: 需要使用字符串流来构造日志消息
// What: 包含sstream头文件，提供std::ostringstream功能
// How: 使用#include预处理指令包含标准库头文件
#include <sstream>

// Why: 将所有日志相关类放在命名空间中，避免命名冲突
// What: 定义sqlcc命名空间，包含所有数据库系统相关的日志类
// How: 使用namespace关键字定义命名空间
namespace sqlcc {

/**
 * @brief 日志级别枚举
 * 
 * Why: 需要区分不同重要程度的日志消息，便于过滤和查找
 * What: 定义了DEBUG、INFO、WARN、ERROR四种日志级别，按重要性递增
 * How: 使用enum class定义强类型枚举，提高类型安全性
 */
enum class LogLevel {
    DEBUG,  // 调试级别：详细的调试信息，主要用于开发和测试
    INFO,   // 信息级别：一般信息，记录系统正常运行状态
    WARN,   // 警告级别：潜在问题，不会影响系统运行但需要注意
    ERROR   // 错误级别：严重错误，可能影响系统正常运行
};

/**
 * @brief 简单日志记录器类
 * 
 * Why: 需要一个统一的日志记录接口来记录系统运行状态、调试信息和错误信息
 * What: Logger类提供单例模式的日志记录功能，支持多种日志级别和输出方式
 * How: 使用单例模式确保全局只有一个日志记录器实例，提供多种日志记录方法
 */
class Logger {
public:
    /**
     * @brief 获取单例实例
     * 
     * Why: 需要确保全局只有一个日志记录器实例，避免多个实例造成的资源浪费和日志混乱
     * What: GetInstance方法返回Logger类的唯一实例引用
     * How: 使用静态局部变量实现线程安全的单例模式
     * 
     * @return Logger实例引用
     */
    static Logger& GetInstance() {
        // Why: 使用静态局部变量实现线程安全的懒加载单例
        // What: 定义静态局部变量instance，在第一次调用时初始化
        // How: 使用static关键字定义局部变量，C++11保证线程安全
        static Logger instance;
        return instance;
    }

    /**
     * @brief 设置日志级别
     * 
     * Why: 需要控制日志输出的详细程度，过滤掉不重要的日志信息
     * What: SetLogLevel方法设置当前日志级别，只有等于或高于此级别的日志才会被输出
     * How: 将传入的日志级别赋值给成员变量log_level_
     * 
     * @param level 日志级别
     */
    void SetLogLevel(LogLevel level) {
        log_level_ = level;
    }

    /**
     * @brief 设置日志文件
     * 
     * Why: 需要将日志输出到文件，便于长期保存和后续分析
     * What: SetLogFile方法打开指定的日志文件，将日志输出到文件而不是控制台
     * How: 使用std::ofstream打开文件，设置输出和追加模式
     * 
     * @param filename 日志文件名
     */
    void SetLogFile(const std::string& filename) {
        log_file_.open(filename, std::ios::out | std::ios::app);
        use_file_ = log_file_.is_open();
    }

    /**
     * @brief 记录调试日志
     * 
     * Why: 需要记录详细的调试信息，帮助开发者了解系统内部状态
     * What: Debug方法记录DEBUG级别的日志消息
     * How: 调用Log方法，传入DEBUG级别和消息内容
     * 
     * @param message 日志消息
     */
    void Debug(const std::string& message) {
        Log(LogLevel::DEBUG, message);
    }

    /**
     * @brief 记录信息日志
     * 
     * Why: 需要记录系统正常运行状态的信息，帮助了解系统运行情况
     * What: Info方法记录INFO级别的日志消息
     * How: 调用Log方法，传入INFO级别和消息内容
     * 
     * @param message 日志消息
     */
    void Info(const std::string& message) {
        Log(LogLevel::INFO, message);
    }

    /**
     * @brief 记录警告日志
     * 
     * Why: 需要记录潜在问题，提醒开发者注意可能的问题
     * What: Warn方法记录WARN级别的日志消息
     * How: 调用Log方法，传入WARN级别和消息内容
     * 
     * @param message 日志消息
     */
    void Warn(const std::string& message) {
        Log(LogLevel::WARN, message);
    }

    /**
     * @brief 记录错误日志
     * 
     * Why: 需要记录严重错误，帮助诊断和解决系统问题
     * What: Error方法记录ERROR级别的日志消息
     * How: 调用Log方法，传入ERROR级别和消息内容
     * 
     * @param message 日志消息
     */
    void Error(const std::string& message) {
        Log(LogLevel::ERROR, message);
    }

private:
    /**
     * @brief 私有构造函数
     * 
     * Why: 防止外部直接创建Logger实例，确保单例模式的正确实现
     * What: Logger类的私有构造函数
     * How: 将构造函数声明为private，只允许GetInstance方法访问
     */
    Logger() : log_level_(LogLevel::INFO), use_file_(false) {}

    /**
     * @brief 禁止拷贝构造函数
     * 
     * Why: 防止通过拷贝构造函数创建新的Logger实例，确保单例模式的正确实现
     * What: 将拷贝构造函数声明为delete，禁止编译器自动生成
     * How: 使用= delete语法显式删除拷贝构造函数
     */
    Logger(const Logger&) = delete;

    /**
     * @brief 禁止赋值操作
     * 
     * Why: 防止通过赋值操作创建新的Logger实例，确保单例模式的正确实现
     * What: 将赋值运算符声明为delete，禁止编译器自动生成
     * How: 使用= delete语法显式删除赋值运算符
     */
    Logger& operator=(const Logger&) = delete;

    /**
     * @brief 记录日志的实际实现
     * 
     * Why: 需要一个统一的日志记录实现，处理时间戳、级别过滤和输出
     * What: Log方法根据日志级别过滤消息，添加时间戳和级别信息，输出到控制台或文件
     * How: 检查日志级别，获取当前时间，格式化日志消息，选择输出流并输出
     * 
     * @param level 日志级别
     * @param message 日志消息内容
     */
    void Log(LogLevel level, const std::string& message) {
        if (level < log_level_) {
            return;
        }

        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;

        std::ostringstream timestamp;
        timestamp << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        timestamp << '.' << std::setfill('0') << std::setw(3) << ms.count();

        std::string level_str;
        switch (level) {
            case LogLevel::DEBUG: level_str = "DEBUG"; break;
            case LogLevel::INFO:  level_str = "INFO";  break;
            case LogLevel::WARN:  level_str = "WARN";  break;
            case LogLevel::ERROR: level_str = "ERROR"; break;
        }

        std::ostringstream log_msg;
        log_msg << "[" << timestamp.str() << "] [" << level_str << "] " << message;

        if (use_file_ && log_file_.is_open()) {
            log_file_ << log_msg.str() << std::endl;
            log_file_.flush();
        } else {
            if (level == LogLevel::ERROR) {
                std::cerr << log_msg.str() << std::endl;
            } else {
                std::cout << log_msg.str() << std::endl;
            }
        }
    }

    LogLevel log_level_;      ///< 当前日志级别
    std::ofstream log_file_;  ///< 日志文件流
    bool use_file_;           ///< 是否使用文件输出
};

/**
 * @brief 获取日志记录器实例的宏定义
 * 
 * Why: 提供一个简化的方式获取日志记录器实例，减少代码重复
 * What: SQLCC_LOGGER宏展开为Logger::GetInstance()调用
 * How: 使用#define预处理指令定义宏
 */
#define SQLCC_LOGGER ::sqlcc::Logger::GetInstance()

/**
 * @brief 记录调试日志的宏定义
 * 
 * Why: 提供一个简化的方式记录调试日志，减少代码重复
 * What: SQLCC_LOG_DEBUG宏展开为SQLCC_LOGGER.Debug(msg)调用
 * How: 使用#define预处理指令定义带参数的宏
 */
#define SQLCC_LOG_DEBUG(msg) SQLCC_LOGGER.Debug(msg)

/**
 * @brief 记录信息日志的宏定义
 * 
 * Why: 提供一个简化的方式记录信息日志，减少代码重复
 * What: SQLCC_LOG_INFO宏展开为SQLCC_LOGGER.Info(msg)调用
 * How: 使用#define预处理指令定义带参数的宏
 */
#define SQLCC_LOG_INFO(msg) SQLCC_LOGGER.Info(msg)

/**
 * @brief 记录警告日志的宏定义
 * 
 * Why: 提供一个简化的方式记录警告日志，减少代码重复
 * What: SQLCC_LOG_WARN宏展开为SQLCC_LOGGER.Warn(msg)调用
 * How: 使用#define预处理指令定义带参数的宏
 */
#define SQLCC_LOG_WARN(msg) SQLCC_LOGGER.Warn(msg)

/**
 * @brief 记录错误日志的宏定义
 * 
 * Why: 提供一个简化的方式记录错误日志，减少代码重复
 * What: SQLCC_LOG_ERROR宏展开为SQLCC_LOGGER.Error(msg)调用
 * How: 使用#define预处理指令定义带参数的宏
 */
#define SQLCC_LOG_ERROR(msg) SQLCC_LOGGER.Error(msg)

} // namespace sqlcc