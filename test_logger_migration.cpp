/**
 * SQLCC Logger Migration Test - Stage 2 Validation
 * Test program to verify Logger module migration to Clang 18 + C++20
 */

#include "include/utils/logger.h"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    std::cout << "=== SQLCC Logger Migration Test ===\n";

    // Test basic logging functionality
    std::cout << "Testing basic logging functionality...\n";

    auto& logger = sqlcc::Logger::GetInstance();

    // Test different log levels
    logger.Info("Logger migration test started");
    logger.Debug("This is a debug message (may not appear if level > DEBUG)");
    logger.Warn("This is a warning message");
    logger.Error("This is an error message");

    // Test move semantics
    logger.Info(std::string("Testing move semantics with string"));

    // Test file logging
    std::cout << "Testing file logging...\n";
    logger.SetLogFile("test_logger_output.log");
    logger.Info("This message should be written to file");

    // Test log level filtering
    std::cout << "Testing log level filtering...\n";
    logger.SetLogLevel(sqlcc::LogLevel::ERROR);
    logger.Info("This INFO message should be filtered out");
    logger.Warn("This WARN message should be filtered out");
    logger.Error("This ERROR message should appear");

    // Reset to INFO level
    logger.SetLogLevel(sqlcc::LogLevel::INFO);
    logger.Info("Log level reset to INFO");

    // Test with macros (backward compatibility)
    std::cout << "Testing backward compatibility macros...\n";
    SQLCC_LOG_INFO("Using SQLCC_LOG_INFO macro");
    SQLCC_LOG_WARN("Using SQLCC_LOG_WARN macro");
    SQLCC_LOG_ERROR("Using SQLCC_LOG_ERROR macro");

    std::cout << "=== Test completed successfully ===\n";
    std::cout << "Check test_logger_output.log for file logging results\n";

    return 0;
}
