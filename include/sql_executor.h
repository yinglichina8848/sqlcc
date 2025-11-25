#ifndef SQLCC_SQL_EXECUTOR_H
#define SQLCC_SQL_EXECUTOR_H

#include <string>
#include <vector>
#include <memory>
#include "user_manager.h"

namespace sqlcc {

class SqlExecutor {
public:
    SqlExecutor();
    ~SqlExecutor();

    // 执行SQL语句
    std::string Execute(const std::string &sql);
    
    // 从文件执行SQL语句
    std::string ExecuteFile(const std::string &file_path);
    
    // 获取最后一次执行的错误信息
    std::string GetLastError();
    
    // 设置错误信息
    void SetError(const std::string &error);

private:
    // 简化的SELECT查询执行
    std::string ExecuteSelect(const std::string &sql);
    
    // 其他SQL命令执行方法
    std::string ExecuteInsert(const std::string &sql);
    std::string ExecuteUpdate(const std::string &sql);
    std::string ExecuteDelete(const std::string &sql);
    
    // 成员变量
    std::string last_error_;
    std::shared_ptr<UserManager> user_manager_;
};

// 辅助函数：修剪字符串
void TrimString(std::string &str);

} // namespace sqlcc

#endif // SQLCC_SQL_EXECUTOR_H