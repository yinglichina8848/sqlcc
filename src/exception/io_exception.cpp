/**
 * IOException实现文件 - 数据库I/O异常处理的具体实现
 *
 * 这个文件实现了IOException类的所有成员函数，包括构造函数、析构函数、
 * 赋值操作符等。IOException专门用于处理数据库系统中的I/O相关异常情况。
 */

#include "include/exception/io_exception.h"

namespace sqlcc {

// 构造函数实现
IOException::IOException(const std::string& message)
    : Exception("I/O Error: " + message) {
    // 构造函数实现委托给基类Exception
    // 基类构造函数已经处理了消息的前缀添加和初始化
}

}  // namespace sqlcc
