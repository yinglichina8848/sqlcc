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
        // Why: 记录当前设置的日志级别，用于后续日志过滤
        // What: 将传入的日志级别赋值给成员变量log_level_
        // How: 使用赋值运算符将参数赋值给成员变量
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
        // Why: 打开日志文件，将日志输出到文件
        // What: 使用std::ofstream打开文件，设置输出和追加模式
        // How: 调用log_file_.open方法，传入文件名和打开模式
        log_file_.open(filename, std::ios::out | std::ios::app);
        // Why: 检查文件是否成功打开，设置use_file_标志
        // What: 根据文件是否成功打开设置use_file_标志
        // How: 调用log_file_.is_open()方法检查文件状态
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
        // Why: 记录调试级别的日志消息
        // What: 调用Log方法，传入DEBUG级别和消息内容
        // How: 直接调用Log方法，传入LogLevel::DEBUG和message参数
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
        // Why: 记录信息级别的日志消息
        // What: 调用Log方法，传入INFO级别和消息内容
        // How: 直接调用Log方法，传入LogLevel::INFO和message参数
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
        // Why: 记录警告级别的日志消息
        // What: 调用Log方法，传入WARN级别和消息内容
        // How: 直接调用Log方法，传入LogLevel::WARN和message参数
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
        // Why: 记录错误级别的日志消息
        // What: 调用Log方法，传入ERROR级别和消息内容
        // How: 直接调用Log方法，传入LogLevel::ERROR和message参数
        Log(LogLevel::ERROR, message);
    }

private:
    /**
     * @brief 私有构造函数
     * 
     * Why: 防止外部创建Logger实例，确保单例模式的正确实现
     * What: 私有构造函数初始化日志级别和文件输出标志
     * How: 使用成员初始化列表初始化log_level_为INFO，use_file_为false
     */
    Logger() : log_level_(LogLevel::INFO), use_file_(false) {}

    /**
     * @brief 禁止拷贝构造
     * 
     * Why: 防止通过拷贝构造创建新的Logger实例，确保单例模式的正确实现
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
     * @param message 日志消息
     */
    void Log(LogLevel level, const std::string& message) {
        // 检查日志级别
        // Why: 过滤掉低于当前日志级别的消息，减少日志输出量
        // What: 比较传入的日志级别和当前设置的日志级别
        // How: 使用比较运算符，如果传入级别低于当前级别则直接返回
        if (level < log_level_) {
            return;
        }

        // 获取当前时间戳
        // Why: 需要记录日志发生的时间，便于问题排查和性能分析
        // What: 获取当前系统时间，包括日期、时间和毫秒
        // How: 使用std::chrono获取当前时间点，转换为time_t和毫秒
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;

        // 格式化时间戳
        // Why: 将时间戳转换为易读的格式，便于人类阅读
        // What: 将时间戳格式化为"YYYY-MM-DD HH:MM:SS.mmm"格式
        // How: 使用std::ostringstream和std::put_time格式化时间
        std::ostringstream timestamp;
        timestamp << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        timestamp << '.' << std::setfill('0') << std::setw(3) << ms.count();

        // 获取日志级别字符串
        // Why: 将枚举类型的日志级别转换为字符串，便于输出
        // What: 根据日志级别枚举值返回对应的字符串表示
        // How: 使用switch语句根据枚举值返回对应字符串
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
        // Why: 将时间戳、级别和消息组合成完整的日志格式
        // What: 构造"[时间戳] [级别] 消息"格式的日志字符串
        // How: 使用std::ostringstream将各部分组合成字符串
        std::ostringstream log_msg;
        log_msg << "[" << timestamp.str() << "] [" << level_str << "] " << message;

        // 输出日志
        // Why: 将日志消息输出到指定位置（文件或控制台）
        // What: 根据设置将日志输出到文件或控制台，错误级别输出到cerr
        // How: 检查use_file_标志，选择输出流并输出日志
        if (use_file_ && log_file_.is_open()) {
            // Why: 将日志写入文件，便于长期保存和后续分析
            // What: 将日志消息写入文件并刷新缓冲区
            // How: 使用log_file_输出日志并调用flush方法
            log_file_ << log_msg.str() << std::endl;
            log_file_.flush();
        } else {
            // Why: 将日志输出到控制台，便于实时查看
            // What: 根据日志级别选择输出流（cout或cerr）
            // How: 使用if语句判断日志级别，选择合适的输出流
            if (level == LogLevel::ERROR) {
                // Why: 错误日志输出到标准错误流，与普通信息区分
                // What: 使用std::cerr输出错误日志
                // How: 使用流插入运算符输出日志
                std::cerr << log_msg.str() << std::endl;
            } else {
                // Why: 普通日志输出到标准输出流
                // What: 使用std::cout输出普通日志
                // How: 使用流插入运算符输出日志
                std::cout << log_msg.str() << std::endl;
            }
        }
    }

    /// 当前日志级别
    // Why: 记录当前设置的日志级别，用于过滤日志消息
    // What: 存储当前日志级别，只有等于或高于此级别的日志才会被输出
    // How: 使用LogLevel枚举类型定义成员变量
    LogLevel log_level_;

    /// 日志文件流
    // Why: 需要一个文件流对象来写入日志文件
    // What: 存储日志文件的输出流对象
    // How: 使用std::ofstream类型定义成员变量
    std::ofstream log_file_;

    /// 是否使用文件记录日志
    // Why: 需要一个标志来控制日志输出位置（文件或控制台）
    // What: 存储是否使用文件记录日志的标志
    // How: 使用bool类型定义成员变量
    bool use_file_;
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

}  // namespace sqlcc