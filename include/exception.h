/**
 * @file exception.h
 * @brief 数据库系统异常处理类定义
 *
 * Why: 需要一个统一的异常处理机制来处理数据库系统中可能出现的各种错误情况
 * What: 定义了数据库系统的异常类层次结构，包括基类Exception和多个派生类
 * How: 使用C++标准库的std::runtime_error作为基类，通过继承实现不同类型的异常
 *
 * Note: 异常类已重构为独立文件，每个类一个文件。保持此头文件用于向后兼容性。
 */

// Why: 防止头文件被多次包含，避免编译错误
// What: 使用#pragma once指令确保头文件只被编译一次
// How: 在文件开头添加#pragma once预处理指令
#pragma once

// Why: 包含所有分离的异常类头文件，保持向后兼容性
// What: 包含所有异常类定义，保持API不变
// How: 使用#include预处理指令包含分离的异常头文件
#include "exception/base_exception.h"
#include "exception/io_exception.h"
#include "exception/buffer_exception.h"
#include "exception/page_exception.h"
#include "exception/disk_exception.h"
#include "exception/lock_exception.h"
#include "exception/feature_exception.h"
#include "exception/argument_exception.h"

// Why: 将所有异常类放在命名空间中，避免命名冲突
// What: 定义sqlcc命名空间，包含所有数据库系统相关的异常类
// How: 使用namespace关键字定义命名空间，并使用using声明导出所有异常类
namespace sqlcc {

// 导出所有异常类到sqlcc命名空间，保持向后兼容性
using sqlcc::Exception;
using sqlcc::IOException;
using sqlcc::BufferPoolException;
using sqlcc::PageException;
using sqlcc::DiskManagerException;
using sqlcc::LockTimeoutException;
using sqlcc::NotImplementedException;
using sqlcc::IllegalArgumentException;

}  // namespace sqlcc
