/**
 * @file feature_exception.h
 * @brief 未实现功能异常类定义
 *
 * Why: 需要一个专门的异常类来处理尚未实现的功能
 * What: NotImplementedException类继承自Exception，用于处理未实现功能相关的异常
 * How: 继承Exception类，在构造函数中添加"Not Implemented Error: "前缀
 */

#pragma once

#include "base_exception.h"
#include <string>

namespace sqlcc {

/**
 * @brief 未实现功能异常类
 *
 * Why: 需要一个专门的异常类来处理尚未实现的功能
 * What: NotImplementedException类继承自Exception，用于处理未实现功能相关的异常
 * How: 继承Exception类，在构造函数中添加"Not Implemented Error: "前缀
 */
class NotImplementedException : public Exception {
public:
    /**
     * @brief 构造函数
     *
     * Why: 需要创建未实现功能异常对象并初始化异常消息
     * What: 构造函数接收异常消息字符串，添加"Not Implemented Error: "前缀后传递给基类
     * How: 使用成员初始化列表调用基类构造函数，添加前缀标识异常类型
     *
     * @param message 异常消息字符串
     */
    explicit NotImplementedException(const std::string& message) : Exception("Not Implemented Error: " + message) {}
};

}  // namespace sqlcc
