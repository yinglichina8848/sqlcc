// Test program for C++20 Logger Module
import sqlcc.utils.logger;

int main() {
    // Test the logger module
    auto& logger = sqlcc::Logger::GetInstance();

    logger.Info("Logger module test started");
    logger.Debug("This is a debug message");
    logger.Warn("This is a warning message");
    logger.Error("This is an error message");

    // Test log level setting
    logger.SetLogLevel(sqlcc::LogLevel::WARN);
    logger.Debug("This debug message should not appear");
    logger.Info("This info message should not appear");
    logger.Warn("This warning should appear");
    logger.Error("This error should appear");

    // Test file logging
    logger.SetLogFile("test_log.txt");
    logger.Info("This message goes to file");

    logger.Info("Logger module test completed");
    return 0;
}
