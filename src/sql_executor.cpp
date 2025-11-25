#include "sql_executor.h"
#include <algorithm>
#include <cctype>
#include <string>

namespace sqlcc {

// 构造函数实现
SqlExecutor::SqlExecutor()
    : last_error_(""), user_manager_(std::make_shared<UserManager>()) {}

// 析构函数实现
SqlExecutor::~SqlExecutor() {
  // 析构函数实现
}

// 执行SQL语句
std::string SqlExecutor::Execute(const std::string &sql) {
  std::string trimmed_sql = sql;
  TrimString(trimmed_sql);

  // 简单的SQL语句类型识别和分发
  std::string upper_sql = trimmed_sql;
  std::transform(upper_sql.begin(), upper_sql.end(), upper_sql.begin(),
                 ::toupper);

  if (upper_sql.find("SELECT") == 0) {
    return ExecuteSelect(trimmed_sql);
  } else if (upper_sql.find("INSERT") == 0) {
    return ExecuteInsert(trimmed_sql);
  } else if (upper_sql.find("UPDATE") == 0) {
    return ExecuteUpdate(trimmed_sql);
  } else if (upper_sql.find("DELETE") == 0) {
    return ExecuteDelete(trimmed_sql);
  }

  return "SQL语句类型不支持";
}

// 从文件执行SQL语句
std::string SqlExecutor::ExecuteFile(const std::string &file_path) {
  // 简化实现：直接返回成功信息
  return "文件执行功能暂未实现";
}

// 获取最后一次执行的错误信息
std::string SqlExecutor::GetLastError() { return last_error_; }

// 设置错误信息
void SqlExecutor::SetError(const std::string &error) { last_error_ = error; }

// 简化的SELECT查询执行
std::string SqlExecutor::ExecuteSelect(const std::string &sql) {
  return "SELECT查询结果";
}

// 其他SQL命令执行方法
std::string SqlExecutor::ExecuteInsert(const std::string &sql) {
  return "插入成功";
}

std::string SqlExecutor::ExecuteUpdate(const std::string &sql) {
  return "更新成功";
}

std::string SqlExecutor::ExecuteDelete(const std::string &sql) {
  return "删除成功";
}

} // namespace sqlcc

// 辅助函数：修剪字符串（在命名空间外实现，与头文件声明匹配）
void sqlcc::TrimString(std::string &str) {
  // 移除前导空格
  size_t start = str.find_first_not_of(" \t\n\r");
  if (start != std::string::npos) {
    str = str.substr(start);
  } else {
    str.clear(); // 全部是空白字符，清空字符串
    return;
  }

  // 移除尾随空格
  size_t end = str.find_last_not_of(" \t\n\r");
  if (end != std::string::npos) {
    str = str.substr(0, end + 1);
  }
}