// Logger Module Implementation
// This is the implementation part of the logger header
// Note: Using traditional approach for Clang 18 compatibility

#include "logger.h"

#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace sqlcc {

// Logger implementation
Logger& Logger::GetInstance() {
    static Logger instance;
    return instance;
}

void Logger::SetLogLevel(LogLevel level) {
    log_level_ = level;
}

void Logger::SetLogFile(const std::string& filename) {
    log_file_.open(filename, std::ios::out | std::ios::app);
    use_file_ = log_file_.is_open();
}

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

Logger::Logger() : log_level_(LogLevel::INFO), use_file_(false) {}

void Logger::Log(LogLevel level, const std::string& message) {
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

} // namespace sqlcc
