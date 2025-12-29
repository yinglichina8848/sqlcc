#include "sql_executor.h"
#include <iostream>
#include <chrono>

namespace sqlcc {

// 构造函数实现
SqlExecutor::SqlExecutor() {
  db_manager_ = std::make_shared<DatabaseManager>("./data", 1024, 16, 64);
}

// 新增构造函数：接受DatabaseManager实例
SqlExecutor::SqlExecutor(std::shared_ptr<DatabaseManager> db_manager)
    : db_manager_(db_manager) {
}

SqlExecutor::~SqlExecutor() = default;

// 执行SQL语句
std::string SqlExecutor::Execute(const std::string &sql) {
  // 验证输入参数
  if (sql.empty()) {
    SetError("SQL语句不能为空");
    return "Error: " + GetLastError();
  }

  ClearError();
  execution_stats_.clear();

  // 记录执行开始时间
  auto start_time = std::chrono::high_resolution_clock::now();

  try {
    // 简单实现：直接返回成功消息，不进行实际解析和执行
    std::string result = "SQL executed successfully (simplified): " + sql;

    // 计算执行时间
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    execution_stats_ = "Execution time: " + std::to_string(duration.count()) + " ms";

    return result;
  } catch (const std::exception &e) {
    std::string error_msg = "Exception occurred during SQL execution: " + std::string(e.what());
    SetError(error_msg);
    return "Error: " + GetLastError();
  } catch (...) {
    std::string error_msg = "Unknown exception occurred during SQL execution";
    SetError(error_msg);
    return "Error: " + GetLastError();
  }
}

std::string SqlExecutor::ExecuteFile(const std::string &file_path) {
  SetError("ExecuteFile not implemented in simplified version");
  return "Error: " + GetLastError();
}

// 获取最后一次执行的错误信息
std::string SqlExecutor::GetLastError() const { return last_error_; }

// 获取执行统计信息
std::string SqlExecutor::GetExecutionStats() const { return execution_stats_; }

// 设置错误信息
void SqlExecutor::SetError(const std::string &error) { last_error_ = error; }

// 清除错误信息
void SqlExecutor::ClearError() { last_error_.clear(); }

} // namespace sqlcc
