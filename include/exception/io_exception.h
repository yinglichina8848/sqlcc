/**
 * @file io_exception.h
 * @brief 文件I/O异常类定义
 *
 * Why: 需要一个专门的异常类来处理文件读写操作中可能出现的错误
 * What: IOException类继承自Exception，用于处理文件I/O相关的异常
 * How: 继承Exception类，在构造函数中添加"IO Error: "前缀
 */

#pragma once

#include "base_exception.h"
#include <string>

namespace sqlcc {

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

}  // namespace sqlcc
