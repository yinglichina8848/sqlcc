// Test program for logger header
#include "include/utils/logger.h"

int main() {
    auto& logger = sqlcc::Logger::GetInstance();
    logger.SetLogLevel(sqlcc::LogLevel::INFO);
    logger.Info("Logger header test successful");

    // Test macros
    SQLCC_LOG_INFO("Macro test successful");

    return 0;
}
