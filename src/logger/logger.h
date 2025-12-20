// SQLCC Logger Module Interface
// C++20 Module version of logger.h
// NOTE: Using hybrid approach for Clang 18 compatibility
// This file serves as both traditional header and module interface

#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>

// Module declaration for future use
// export module sqlcc.utils.logger;

namespace sqlcc {

    // Log levels
    enum class LogLevel {
        DEBUG,
        INFO,
        WARN,
        ERROR
    };

    // Logger class
    class Logger {
    public:
        static Logger& GetInstance();

        void SetLogLevel(LogLevel level);
        void SetLogFile(const std::string& filename);

        void Debug(const std::string& message);
        void Info(const std::string& message);
        void Warn(const std::string& message);
        void Error(const std::string& message);

    private:
        Logger();
        Logger(const Logger&) = delete;
        Logger& operator=(const Logger&) = delete;

        void Log(LogLevel level, const std::string& message);

        LogLevel log_level_;
        std::ofstream log_file_;
        bool use_file_;
    };

} // namespace sqlcc

// Convenience macros (keeping for backward compatibility)
// Note: These are not exported as they are preprocessor macros
#define SQLCC_LOGGER ::sqlcc::Logger::GetInstance()
#define SQLCC_LOG_DEBUG(msg) SQLCC_LOGGER.Debug(msg)
#define SQLCC_LOG_INFO(msg) SQLCC_LOGGER.Info(msg)
#define SQLCC_LOG_WARN(msg) SQLCC_LOGGER.Warn(msg)
#define SQLCC_LOG_ERROR(msg) SQLCC_LOGGER.Error(msg)
