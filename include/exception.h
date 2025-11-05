/**
 * @file exception.h
 * @brief 数据库系统异常处理类定义
 * 
 * Why: 需要一个统一的异常处理机制来处理数据库系统中可能出现的各种错误情况
 * What: 定义了数据库系统的异常类层次结构，包括基类Exception和多个派生类
 * How: 使用C++标准库的std::runtime_error作为基类，通过继承实现不同类型的异常
 */

// Why: 防止头文件被多次包含，避免编译错误
// What: 使用#pragma once指令确保头文件只被编译一次
// How: 在文件开头添加#pragma once预处理指令
#pragma once

// Why: 需要使用标准库中的异常类来构建自定义异常类
// What: 包含std::runtime_error类，作为异常基类
// How: 使用#include预处理指令包含标准库头文件
#include <stdexcept>

// Why: 需要使用字符串类来存储异常消息
// What: 包含std::string类，用于处理异常消息
// How: 使用#include预处理指令包含标准库头文件
#include <string>

// Why: 将所有异常类放在命名空间中，避免命名冲突
// What: 定义sqlcc命名空间，包含所有数据库系统相关的异常类
// How: 使用namespace关键字定义命名空间
namespace sqlcc {

/**
 * @brief SQLCC异常基类
 * 
 * Why: 需要一个统一的异常基类来处理所有数据库系统中的异常
 * What: Exception类继承自std::runtime_error，作为所有自定义异常的基类
 * How: 使用public继承方式继承std::runtime_error，提供构造函数接收异常消息
 */
class Exception : public std::runtime_error {
public:
    /**
     * @brief 构造函数
     * 
     * Why: 需要创建异常对象并初始化异常消息
     * What: 构造函数接收异常消息字符串，传递给基类std::runtime_error
     * How: 使用成员初始化列表调用基类构造函数
     * 
     * @param message 异常消息字符串
     */
    explicit Exception(const std::string& message) : std::runtime_error(message) {}
};

/**
 * @brief 文件I/O异常类
 * 
 * Why: 需要一个专门的异常类来处理文件读写操作中可能出现的错误
 * What: IOException类继承自Exception，用于处理文件I/O相关的异常
 * How: 继承Exception类，在构造函数中添加"IO Error: "前缀
 */
class IOException : public Exception {
public:
    /**
     * @brief 构造函数
     * 
     * Why: 需要创建I/O异常对象并初始化异常消息
     * What: 构造函数接收异常消息字符串，添加"IO Error: "前缀后传递给基类
     * How: 使用成员初始化列表调用基类构造函数，添加前缀标识异常类型
     * 
     * @param message 异常消息字符串
     */
    explicit IOException(const std::string& message) : Exception("IO Error: " + message) {}
};

/**
 * @brief 缓冲池异常类
 * 
 * Why: 需要一个专门的异常类来处理缓冲池操作中可能出现的错误
 * What: BufferPoolException类继承自Exception，用于处理缓冲池相关的异常
 * How: 继承Exception类，在构造函数中添加"Buffer Pool Error: "前缀
 */
class BufferPoolException : public Exception {
public:
    /**
     * @brief 构造函数
     * 
     * Why: 需要创建缓冲池异常对象并初始化异常消息
     * What: 构造函数接收异常消息字符串，添加"Buffer Pool Error: "前缀后传递给基类
     * How: 使用成员初始化列表调用基类构造函数，添加前缀标识异常类型
     * 
     * @param message 异常消息字符串
     */
    explicit BufferPoolException(const std::string& message) : Exception("Buffer Pool Error: " + message) {}
};

/**
 * @brief 页面异常类
 * 
 * Why: 需要一个专门的异常类来处理页面操作中可能出现的错误
 * What: PageException类继承自Exception，用于处理页面相关的异常
 * How: 继承Exception类，在构造函数中添加"Page Error: "前缀
 */
class PageException : public Exception {
public:
    /**
     * @brief 构造函数
     * 
     * Why: 需要创建页面异常对象并初始化异常消息
     * What: 构造函数接收异常消息字符串，添加"Page Error: "前缀后传递给基类
     * How: 使用成员初始化列表调用基类构造函数，添加前缀标识异常类型
     * 
     * @param message 异常消息字符串
     */
    explicit PageException(const std::string& message) : Exception("Page Error: " + message) {}
};

/**
 * @brief 磁盘管理器异常类
 * 
 * Why: 需要一个专门的异常类来处理磁盘管理器操作中可能出现的错误
 * What: DiskManagerException类继承自Exception，用于处理磁盘管理器相关的异常
 * How: 继承Exception类，在构造函数中添加"Disk Manager Error: "前缀
 */
class DiskManagerException : public Exception {
public:
    /**
     * @brief 构造函数
     * 
     * Why: 需要创建磁盘管理器异常对象并初始化异常消息
     * What: 构造函数接收异常消息字符串，添加"Disk Manager Error: "前缀后传递给基类
     * How: 使用成员初始化列表调用基类构造函数，添加前缀标识异常类型
     * 
     * @param message 异常消息字符串
     */
    explicit DiskManagerException(const std::string& message) : Exception("Disk Manager Error: " + message) {}
};

}  // namespace sqlcc