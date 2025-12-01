/**
 * @file sql_executor_stub.cpp
 * @brief SQL执行器存根 - 仅用于链接演示程序
 * 
 * 提供最小实现以支持编译和链接
 */

#include "sql_executor.h"
#include <string>

namespace sqlcc {

// SqlExecutor的最小实现
SqlExecutor::SqlExecutor() {
}

SqlExecutor::~SqlExecutor() {
}

std::string SqlExecutor::Execute(const std::string& sql) {
    // 返回空结果
    return "";
}

std::string SqlExecutor::ExecuteFile(const std::string& filename) {
    return "ERROR: File execution not supported";
}

std::string SqlExecutor::GetLastError() const {
    return "";
}

} // namespace sqlcc
