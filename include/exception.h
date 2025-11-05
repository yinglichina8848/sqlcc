#pragma once

#include <stdexcept>
#include <string>

namespace sqlcc {

/**
 * @brief SQLCC异常基类
 */
class Exception : public std::runtime_error {
public:
    explicit Exception(const std::string& message) : std::runtime_error(message) {}
};

/**
 * @brief 文件I/O异常类
 */
class IOException : public Exception {
public:
    explicit IOException(const std::string& message) : Exception("IO Error: " + message) {}
};

/**
 * @brief 缓冲池异常类
 */
class BufferPoolException : public Exception {
public:
    explicit BufferPoolException(const std::string& message) : Exception("Buffer Pool Error: " + message) {}
};

/**
 * @brief 页面异常类
 */
class PageException : public Exception {
public:
    explicit PageException(const std::string& message) : Exception("Page Error: " + message) {}
};

/**
 * @brief 磁盘管理器异常类
 */
class DiskManagerException : public Exception {
public:
    explicit DiskManagerException(const std::string& message) : Exception("Disk Manager Error: " + message) {}
};

}  // namespace sqlcc