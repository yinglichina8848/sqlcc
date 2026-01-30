/**
 * @file io_exception.h
 *
 * IOException类定义 - I/O异常处理
 *
 * IOException专门用于处理数据库系统中的I/O相关异常情况，
 * 如文件读写错误、网络通信错误等。
 *
 * 继承关系：
 * IOException -> Exception -> std::exception
 *
 * @author SQLCC Team
 * @version 1.0
 * @date 2024-12-01
 */

#pragma once

#include "src/exception/exception.h"

namespace sqlcc {

class IOException : public Exception {
public:
    explicit IOException(const std::string& message) : Exception(message) {}
    virtual ~IOException() noexcept = default;

    const char* what() const noexcept override {
        static std::string full_message;
        full_message = std::string("I/O Error: ") + Exception::what();
        return full_message.c_str();
    }

    const char* type() const noexcept {
        return "IOException";
    }
};

} // namespace sqlcc
