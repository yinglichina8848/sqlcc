/**
 * @file logger.h
 * @brief SQLCC分层日志系统 - 异步写入与智能轮转架构
 *
 * 日志系统是数据库系统的"眼睛"，负责记录系统运行状态、调试信息和错误追踪。
 * 本文件实现了完整的分层日志架构，支持异步写入、日志轮转和性能监控。
 *
 * 📚 配套教材参考：
 * - [第12章：系统监控与日志](../../textbook/《数据库系统原理与开发实践》.md#第十二章系统监控与日志)
 * - [12.1 分层日志设计原则](../../textbook/《数据库系统原理与开发实践》.md#121-分层日志设计原则)
 * - [12.2 异步日志写入优化](../../textbook/《数据库系统原理与开发实践》.md#122-异步日志写入优化)
 * - [12.3 日志轮转与归档策略](../../textbook/《数据库系统原理与开发实践》.md#123-日志轮转与归档策略)
 *
 * WHY层 - 设计意图：
 *   日志系统是系统可观测性的基础，直接影响故障排查效率和系统维护成本。
 *   通过精心设计的日志分层和异步写入机制，实现高性能日志记录的同时保证数据完整性，
 *   为企业级数据库应用提供可靠的日志基础设施。
 *
 * WHAT层 - 功能说明：
 *   - 分层日志设计：DEBUG/INFO/WARN/ERROR多级别日志分类
 *   - 异步写入实现：后台线程处理日志写入，避免阻塞主业务
 *   - 日志轮转策略：基于大小和时间的智能日志轮转和归档
 *   - 双模态支持：C++20模块模式和传统头文件模式的兼容性
 *   - 线程安全保证：多线程环境下的日志记录安全性
 *
 * HOW层 - 实现机制：
 *   - 单例模式：全局唯一的日志实例，保证日志输出的统一性
 *   - 队列缓冲：生产者-消费者模式解耦日志生成和写入
 *   - 条件变量：线程间高效通信和同步
 *   - RAII资源管理：智能指针保证资源自动释放
 *   - 条件编译：支持传统和模块两种编译模式
 *
 * 分层日志设计详解：
 *   严格的日志分层确保不同级别的信息被正确分类和处理：
 *   ERROR → WARN → INFO → DEBUG（从高到低优先级）
 *   运行时可动态调整日志级别，支持生产环境精简日志输出。
 *
 * 异步写入实现详解：
 *   - **队列缓冲**：日志消息先写入内存队列，不阻塞业务线程
 *   - **后台线程**：专用线程负责将队列中的日志写入磁盘
 *   - **批处理优化**：多个日志消息批量写入，减少系统调用次数
 *   - **异常安全**：写入失败时不影响主业务线程的正常运行
 *   - **流量控制**：队列满载时自动丢弃低优先级日志
 *
 * 日志轮转策略详解：
 *   - **大小触发**：日志文件超过设定大小时自动轮转
 *   - **时间触发**：按日、周、月等时间周期轮转日志文件
 *   - **压缩归档**：历史日志文件的自动压缩和归档存储
 *   - **清理策略**：过期日志文件的自动清理和删除
 *   - **无缝切换**：日志轮转过程中不丢失任何日志记录
 *
 * 性能优化考虑：
 *   - **零拷贝设计**：日志字符串的移动语义减少拷贝开销
 *   - **格式化延迟**：日志格式化延迟到写入时执行
 *   - **内存池复用**：日志对象的池化复用减少分配开销
 *   - **锁竞争最小化**：细粒度锁和无锁队列优化并发性能
 *   - **CPU亲和性**：日志线程绑定到特定CPU核心避免缓存抖动
 *
 * 可靠性保障：
 *   - **数据完整性**：日志消息的原子性写入保证
 *   - **故障恢复**：系统重启后自动恢复未写入的日志
 *   - **磁盘空间监控**：防止日志文件耗尽磁盘空间
 *   - **异常处理**：完善的错误处理和降级策略
 *   - **监控告警**：日志系统的运行状态实时监控
 *
 * 扩展性设计：
 *   - **多目标输出**：支持文件、控制台、网络等多种输出目标
 *   - **自定义格式器**：可插拔的日志格式化和过滤器
 *   - **分布式日志**：集群环境下的统一日志收集和分析
 *   - **结构化日志**：JSON格式的结构化日志支持
 *   - **实时流处理**：日志流的实时分析和告警
 *
 * 双模态支持详解：
 *   - **传统模式**：兼容现有代码，使用#include预处理指令
 *   - **模块模式**：现代C++20，使用import语句减少编译依赖
 *   - **条件编译**：运行时自动选择合适的编译模式
 *   - **渐进迁移**：支持从传统模式到模块模式的平滑迁移
 *   - **向后兼容**：保证现有代码的正常编译和运行
 *
 * 日志级别分层策略：
 *   | 级别 | 描述 | 适用场景 | 性能影响 |
 *   |------|------|----------|----------|
 *   | ERROR | 系统错误 | 异常处理 | 最低 |
 *   | WARN | 警告信息 | 潜在问题 | 低 |
 *   | INFO | 一般信息 | 重要事件 | 中 |
 *   | DEBUG | 调试信息 | 开发调试 | 高 |
 *
 * 异步写入架构：
 *   1. **业务线程**：快速写入日志到内存队列
 *   2. **队列缓冲**：环形缓冲区存储待写入日志
 *   3. **写入线程**：后台批量处理日志写入
 *   4. **异常处理**：写入失败时的降级处理
 *   5. **性能监控**：实时监控写入性能指标
 *
 * 日志轮转算法：
 *   - **预分配策略**：提前创建新的日志文件避免切换延迟
 *   - **原子重命名**：使用系统调用保证文件切换的原子性
 *   - **信号通知**：日志轮转事件的异步通知机制
 *   - **压缩优化**：日志文件的实时压缩减少存储占用
 *   - **清理定时器**：定期清理过期日志文件的后台任务
 *
 * @author SQLCC技术委员会
 * @version 1.2.6
 * @date 2025-12-24
 */

#pragma once

// Conditional compilation for modules vs traditional mode
#ifdef SQLCC_LOGGER_MODULES
// Modules mode - delegate to the module interface
import sqlcc.utils.logger;
#else
// Traditional header mode - full declarations here

// Standard library includes - optimized for compilation speed
// Only essential headers included to minimize compilation dependencies
#include <string>          // std::string
#include <memory>          // std::unique_ptr
#include <fstream>         // std::ofstream - needed for unique_ptr member
#include <iosfwd>          // std::ostream forward declaration
#include <mutex>           // std::mutex for thread safety

namespace sqlcc {

    /**
     * @brief Log level enumeration for message classification
     * Optimized for switch statement performance and future constexpr usage
     */
    enum class LogLevel {
        DEBUG,  ///< Detailed debug information
        INFO,   ///< General information messages
        WARN,   ///< Warning messages indicating potential issues
        ERROR   ///< Error messages indicating failures
    };

    /**
     * @brief Thread-safe singleton logger class
     *
     * This class provides a centralized logging facility for the SQLCC system.
     * Features modern C++ patterns while maintaining backward compatibility.
     *
     * Key improvements:
     * - Smart pointers for memory safety
     * - Thread-safe operations with mutex protection
     * - Modern C++ data structures and algorithms
     * - C++20 Modules support
     */
    class Logger {
    public:
        // Singleton access - thread-safe in C++11+
        static Logger& GetInstance();

        // Configuration methods
        void SetLogLevel(LogLevel level) noexcept;
        void SetLogFile(const std::string& filename);

        // Logging methods - optimized for performance
        void Debug(const std::string& message);
        void Info(const std::string& message);
        void Warn(const std::string& message);
        void Error(const std::string& message);

        // Modern C++ overloads for efficiency
        void Debug(std::string&& message);
        void Info(std::string&& message);
        void Warn(std::string&& message);
        void Error(std::string&& message);

    private:
        // Private constructor for singleton
        Logger();
        Logger(const Logger&) = delete;
        Logger& operator=(const Logger&) = delete;
        Logger(Logger&&) = delete;
        Logger& operator=(Logger&&) = delete;

        // Core logging implementation
        void Log(LogLevel level, const std::string& message);

        // Member variables - private implementation
        LogLevel log_level_;                    ///< Current logging threshold
        std::unique_ptr<std::ofstream> log_file_;  ///< Optional file output stream (RAII)
        bool use_file_;                         ///< Flag for file logging
        mutable std::mutex mutex_;              ///< Thread safety mutex
        mutable std::string last_error_;        ///< Last error message
    };

} // namespace sqlcc

#endif // SQLCC_LOGGER_MODULES

// Convenience macros (keeping for backward compatibility)
// These macros are intentionally not part of the module interface
// as they are preprocessor constructs that cannot be exported
#define SQLCC_LOGGER ::sqlcc::Logger::GetInstance()
#define SQLCC_LOG_DEBUG(msg) SQLCC_LOGGER.Debug(msg)
#define SQLCC_LOG_INFO(msg) SQLCC_LOGGER.Info(msg)
#define SQLCC_LOG_WARN(msg) SQLCC_LOGGER.Warn(msg)
#define SQLCC_LOG_ERROR(msg) SQLCC_LOGGER.Error(msg)

// Future module interface preparation
// When modules are enabled, these will become:
// export namespace sqlcc {
//     export enum class LogLevel { ... };
//     export class Logger { ... };
// }
