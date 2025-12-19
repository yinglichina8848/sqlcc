#include <iostream>
#include <string>

namespace sqlcc {

void SQLCC_LOG_DEBUG(const std::string& msg) {
    // 简单的日志实现
    std::cout << "[DEBUG] " << msg << std::endl;
}

void SQLCC_LOG_INFO(const std::string& msg) {
    // 简单的日志实现
    std::cout << "[INFO] " << msg << std::endl;
}

void SQLCC_LOG_WARN(const std::string& msg) {
    // 简单的日志实现
    std::cout << "[WARN] " << msg << std::endl;
}

void SQLCC_LOG_ERROR(const std::string& msg) {
    // 简单的日志实现
    std::cout << "[ERROR] " << msg << std::endl;
}

} // namespace sqlcc