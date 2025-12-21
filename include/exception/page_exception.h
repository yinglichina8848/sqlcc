/**
 * @file page_exception.h
 * @brief 页面异常类定义
 *
 * Why: 需要一个专门的异常类来处理页面操作中可能出现的错误
 * What: PageException类继承自Exception，用于处理页面相关的异常
 * How: 继承Exception类，在构造函数中添加"Page Error: "前缀
 */

#pragma once

#include "base_exception.h"
#include <string>

namespace sqlcc {

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

}  // namespace sqlcc
