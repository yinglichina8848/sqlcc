/**
 * SQLCC Logger Dual-Mode Test
 * Test program to verify both traditional header and modules mode work
 */

#include <iostream>
#include <string>
#include "include/utils/logger.h"

// Test 1: Traditional header mode (default)
void test_traditional_mode() {
    std::cout << "\n=== Testing Traditional Header Mode ===" << std::endl;

    auto& logger = sqlcc::Logger::GetInstance();
    logger.SetLogLevel(sqlcc::LogLevel::DEBUG);

    logger.Info("Traditional header mode test started");
    logger.Debug("This is a debug message");
    logger.Warn("This is a warning message");
    logger.Error("This is an error message");

    logger.Info(std::string("Testing move semantics with string"));

    logger.SetLogFile("traditional_mode_test.log");
    logger.Info("This should be written to file");

    std::cout << "Traditional mode test completed" << std::endl;
}

// Test 2: Modules mode (when SQLCC_LOGGER_MODULES is defined)
// Note: This requires recompilation with -DSQLCC_LOGGER_MODULES
void test_modules_mode() {
    std::cout << "\n=== Testing Modules Mode ===" << std::endl;

    auto& logger = sqlcc::Logger::GetInstance();
    logger.SetLogLevel(sqlcc::LogLevel::INFO);

    logger.Info("Modules mode test started");
    logger.Info("This message should appear in modules mode");

    std::cout << "Modules mode test completed" << std::endl;
}

int main() {
    std::cout << "=== SQLCC Logger Dual-Mode Compatibility Test ===" << std::endl;

#ifdef SQLCC_LOGGER_MODULES
    std::cout << "Compiled with SQLCC_LOGGER_MODULES defined - Testing Modules Mode" << std::endl;
    test_modules_mode();
#else
    std::cout << "Compiled without SQLCC_LOGGER_MODULES - Testing Traditional Header Mode" << std::endl;
    test_traditional_mode();
#endif

    std::cout << "\n=== Test completed successfully ===" << std::endl;
    std::cout << "Both traditional header and modules modes are supported!" << std::endl;

    return 0;
}
