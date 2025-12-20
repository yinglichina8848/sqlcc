/**
 * SQLCC Logger Module Interface - Dual-Mode Support
 * C++20 Modules + Traditional Header with Conditional Compilation
 * Migration Phase: Modules + Traditional Header Compatibility
 *
 * This file supports both:
 * 1. Traditional #include mode for backward compatibility
 * 2. C++20 Modules mode for modern builds
 *
 * All includes are optimized for minimal compilation dependencies.
 * Thread-safe singleton implementation with modern C++ features.
 *
 * Key improvements:
 * - Smart pointers for memory safety
 * - Forward declarations to reduce compilation dependencies
 * - Thread-safe operations with mutex protection
 * - Modern C++ data structures and algorithms
 * - Dual-mode support (modules + traditional)
 *
 * Conditional compilation:
 * - SQLCC_LOGGER_MODULES: Enable modules mode
 * - Default: Traditional header mode
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
