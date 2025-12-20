/**
 * SQLCC Logger Module - C++20 Modules Interface
 * True C++20 Modules implementation with dual-mode support
 * Migration Phase: Modules + Traditional Header Compatibility
 */

// Module declaration - C++20 Modules interface
export module sqlcc.utils.logger;

// Standard library imports for the module
import <string>;
import <memory>;
import <vector>;
import <unordered_map>;
import <mutex>;
import <chrono>;
import <iomanip>;
import <sstream>;
import <iostream>;
import <fstream>;

// Export the Logger class and related types
export namespace sqlcc {

    /**
     * @brief Log level enumeration for message classification
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

// Module implementation will be in a separate file
// This file only contains the interface declaration
