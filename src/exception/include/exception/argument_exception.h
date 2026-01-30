/**
 * @file argument_exception.h
 * @brief 非法参数异常类定义
 *
 * Why: 需要一个专门的异常类来处理非法参数的情况
 * What: IllegalArgumentException类继承自Exception，用于处理非法参数相关的异常
 * How: 继承Exception类，在构造函数中添加"Illegal Argument Error: "前缀
 */

#pragma once

#include "src/exception/include/exception/base_exception.h"
#include <string>

namespace sqlcc {

/**
 * @brief 非法参数异常类
 *
 * Why: 需要一个专门的异常类来处理非法参数的情况
 * What: IllegalArgumentException类继承自Exception，用于处理非法参数相关的异常
 * How: 继承Exception类，在构造函数中添加"Illegal Argument Error: "前缀
 */
class IllegalArgumentException : public Exception {
public:
    /**
     * @brief 构造函数
     *
     * Why: 需要创建非法参数异常对象并初始化异常消息
     * What: 构造函数接收异常消息字符串，添加"Illegal Argument Error: "前缀后传递给基类
     * How: 使用成员初始化列表调用基类构造函数，添加前缀标识异常类型
     *
     * @param message 异常消息字符串
     */
    explicit IllegalArgumentException(const std::string& message) : Exception("Illegal Argument Error: " + message) {}
};

}  // namespace sqlcc
