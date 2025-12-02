#ifndef SQLCC_SQL_EXECUTOR_H
#define SQLCC_SQL_EXECUTOR_H

#include <memory>
#include <string>
#include "database_manager.h"
#include "user_manager.h"

namespace sqlcc {

class SqlExecutor {
public:
    SqlExecutor();
    // 新增：接受DatabaseManager的构造函数，用于共享数据库实例
    SqlExecutor(std::shared_ptr<DatabaseManager> db_manager);
    ~SqlExecutor();
    
    std::string Execute(const std::string& sql);
    std::string ExecuteFile(const std::string& file_path);
    std::string GetLastError() const;

private:
    std::shared_ptr<DatabaseManager> db_manager_;
    std::shared_ptr<UserManager> user_manager_;
    std::string last_error_;
    
    void SetError(const std::string& error);
    std::string ExecuteSelect(void* stmt);
    std::string ExecuteInsert(void* stmt);
    std::string ExecuteUpdate(void* stmt);
    std::string ExecuteDelete(void* stmt);
    std::string ExecuteCreate(void* stmt);
    std::string ExecuteDrop(void* stmt);
    void TrimString(std::string& str);
};

} // namespace sqlcc

#endif // SQLCC_SQL_EXECUTOR_H