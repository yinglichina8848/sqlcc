/**
 * @file base_exception.h
 * @brief SQLCC异常基类定义
 *
 * Why: 需要一个统一的异常基类来处理所有数据库系统中的异常
 * What: Exception类继承自std::runtime_error，作为所有自定义异常的基类
 * How: 使用public继承方式继承std::runtime_error，提供构造函数接收异常消息
 */

#pragma once

#include <stdexcept>
#include <string>

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

}  // namespace sqlcc
