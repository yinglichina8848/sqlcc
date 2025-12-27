/**
 * SQLCC Logger Implementation - Stage 2 Migration Ready
 * C++20 Module implementation of Logger class - Optimized for Clang 18
 * Migration Phase: Traditional implementation with module preparation
 *
 * This file implements the Logger singleton class with modern C++ features.
 * Thread-safe, efficient, and ready for future module conversion.
 */

#include "../../include/utils/logger.h"  // Include our optimized header

// Required standard library includes for implementation
// These are separated from header to minimize compilation dependencies
#include <iostream>     // std::cout, std::cerr
#include <fstream>      // std::ofstream
#include <chrono>       // Time utilities
#include <iomanip>      // std::put_time, std::setfill, std::setw
#include <sstream>      // std::ostringstream
#include <memory>       // std::unique_ptr for RAII
#include <mutex>        // Thread safety (future enhancement)

// Future C++20 Modules implementation
// TODO: When modules are enabled, this becomes:
// module sqlcc.utils.logger;
// import <iostream>;
// import <fstream>;
// etc.

namespace sqlcc {

// Static member definitions
Logger& Logger::GetInstance() {
    // Thread-safe singleton using C++11 magic statics
    static Logger instance;
    return instance;
}

void Logger::SetLogLevel(LogLevel level) noexcept {
    log_level_ = level;
}

void Logger::SetLogFile(const std::string& filename) {
    // Create new file stream outside mutex to avoid holding lock during file I/O
    auto new_log_file = std::make_unique<std::ofstream>(filename, std::ios::out | std::ios::app);
    bool is_open = new_log_file->is_open();

    // Thread-safe update of member variables
    { 
        std::lock_guard<std::mutex> lock(mutex_);
        log_file_ = std::move(new_log_file);
        use_file_ = is_open;
    }

    // 不自动记录日志，避免干扰测试
    // 测试中可以显式记录日志进行验证
}

// L-value reference overloads (efficient)
void Logger::Debug(const std::string& message) {
    Log(LogLevel::DEBUG, message);
}

void Logger::Info(const std::string& message) {
    Log(LogLevel::INFO, message);
}

void Logger::Warn(const std::string& message) {
    Log(LogLevel::WARN, message);
}

void Logger::Error(const std::string& message) {
    Log(LogLevel::ERROR, message);
}

// R-value reference overloads (more efficient for temporaries)
void Logger::Debug(std::string&& message) {
    Log(LogLevel::DEBUG, std::move(message));
}

void Logger::Info(std::string&& message) {
    Log(LogLevel::INFO, std::move(message));
}

void Logger::Warn(std::string&& message) {
    Log(LogLevel::WARN, std::move(message));
}

void Logger::Error(std::string&& message) {
    Log(LogLevel::ERROR, std::move(message));
}

// Private constructor - singleton pattern
Logger::Logger()
    : log_level_(LogLevel::INFO)
    , log_file_(nullptr)
    , use_file_(false)
    , mutex_()
    , last_error_() {
    // Initialization complete - could add startup logging here
}

// Core logging implementation - now thread-safe
void Logger::Log(LogLevel level, const std::string& message) {
    // Early exit for filtered messages - performance optimization
    if (level < log_level_) {
        return;
    }

    // Get current time with high precision
    const auto now = std::chrono::system_clock::now();
    const auto time_t_now = std::chrono::system_clock::to_time_t(now);
    const auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;

    // Format timestamp with milliseconds
    std::ostringstream timestamp_stream;
    timestamp_stream << std::put_time(std::localtime(&time_t_now), "%Y-%m-%d %H:%M:%S");
    timestamp_stream << '.' << std::setfill('0') << std::setw(3) << milliseconds.count();

    // Convert log level to string
    const char* level_str = [level]() noexcept -> const char* {
        switch (level) {
            case LogLevel::DEBUG: return "DEBUG";
            case LogLevel::INFO:  return "INFO";
            case LogLevel::WARN:  return "WARN";
            case LogLevel::ERROR: return "ERROR";
            default:              return "UNKNOWN";
        }
    }();

    // Construct final log message
    std::ostringstream log_stream;
    log_stream << "[" << timestamp_stream.str() << "] [" << level_str << "] " << message;

    const std::string final_message = log_stream.str();

    // Thread-safe output to appropriate destination(s)
    std::lock_guard<std::mutex> lock(mutex_);
    if (use_file_ && log_file_ && log_file_->is_open()) {
        // File output (with flush for immediate visibility)
        *log_file_ << final_message << std::endl;
        log_file_->flush();

        // Also output ERROR level to console for immediate attention
        if (level >= LogLevel::ERROR) {
            std::cerr << final_message << std::endl;
        }
    } else {
        // Console output only
        if (level >= LogLevel::ERROR) {
            std::cerr << final_message << std::endl;
        } else {
            std::cout << final_message << std::endl;
        }
    }
}

// Future enhancements (commented for now):
/*
// Thread-safe logging with mutex (for future multi-threaded enhancements)
void Logger::Log(LogLevel level, const std::string& message) {
    std::lock_guard<std::mutex> lock(log_mutex_);
    // ... existing implementation
}

// Asynchronous logging implementation
void Logger::AsyncLog(LogLevel level, std::string message) {
    // Queue message for background processing
    // ... implementation for high-throughput logging
}
*/

} // namespace sqlcc

// Module implementation preparation
// When modules are enabled, this file will become:
// module sqlcc.utils.logger;
// with implementation details hidden from interface