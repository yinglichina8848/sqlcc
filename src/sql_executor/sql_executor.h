#ifndef SQLCC_SQL_EXECUTOR_H
#define SQLCC_SQL_EXECUTOR_H

#include <string>
#include <vector>

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
    const std::string &GetLastError() const;
    
    // 设置错误信息
    void SetError(const std::string &error);

private:
    // 简化的SELECT查询执行
    std::string ExecuteSelect(const std::string &sql);
    
    // 其他私有成员和方法可以在这里添加
};

// 辅助函数：修剪字符串
void TrimString(std::string &str);

} // namespace sqlcc

#endif // SQLCC_SQL_EXECUTOR_H