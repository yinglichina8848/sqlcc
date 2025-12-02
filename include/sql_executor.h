#ifndef SQLCC_SQL_EXECUTOR_H
#define SQLCC_SQL_EXECUTOR_H

#include <memory>
#include <string>
#include "database_manager.h"
#include "user_manager.h"
#include "system_database.h"
#include "unified_query_plan.h"
#include "permission_validator.h"
#include "sql_parser/parser.h"

namespace sqlcc {

/**
 * @brief SQL执行器类 - 重构版本
 * 
 * 使用统一查询计划架构，整合DDL/DML/DCL执行器公共逻辑
 * 解决执行器分离过度、缺少统一查询计划、错误处理不一致的问题
 */
class SqlExecutor {
public:
    SqlExecutor();
    // 新增：接受DatabaseManager的构造函数，用于共享数据库实例
    SqlExecutor(std::shared_ptr<DatabaseManager> db_manager);
    ~SqlExecutor();
    
    /**
     * @brief 执行SQL语句
     * @param sql SQL语句字符串
     * @return 执行结果消息
     */
    std::string Execute(const std::string& sql);
    
    /**
     * @brief 执行SQL文件
     * @param file_path 文件路径
     * @return 执行结果消息
     */
    std::string ExecuteFile(const std::string& file_path);
    
    /**
     * @brief 获取最后一次执行的错误信息
     * @return 错误信息
     */
    std::string GetLastError() const;
    
    /**
     * @brief 获取执行统计信息
     * @return 统计信息字符串
     */
    std::string GetExecutionStats() const;

private:
    std::shared_ptr<DatabaseManager> db_manager_;
    std::shared_ptr<UserManager> user_manager_;
    std::shared_ptr<SystemDatabase> system_db_;
    std::unique_ptr<PermissionValidator> permission_validator_;
    std::string last_error_;
    std::string execution_stats_;
    std::string current_user_;
    std::string current_database_;
    
    /**
     * @brief 设置错误信息
     * @param error 错误信息
     */
    void SetError(const std::string& error);
    
    /**
     * @brief 清除错误信息
     */
    void ClearError();
    
    /**
     * @brief 初始化系统数据库
     */
    bool InitializeSystemDatabase();
    
    /**
     * @brief 解析SQL语句
     * @param sql SQL语句
     * @return 解析后的语句对象
     */
    std::unique_ptr<sql_parser::Statement> ParseSQL(const std::string& sql);
    
    /**
     * @brief 创建统一查询计划
     * @param stmt 解析后的语句
     * @return 查询计划对象
     */
    std::unique_ptr<UnifiedQueryPlan> CreateQueryPlan(std::unique_ptr<sql_parser::Statement> stmt);
    
    /**
     * @brief 初始化权限验证器
     */
    bool InitializePermissionValidator();
    
    /**
     * @brief 更新当前数据库
     * @param sql SQL语句
     */
    void UpdateCurrentDatabase(const std::string& sql);
    
    /**
     * @brief 去除字符串两端的空白字符
     * @param str 要处理的字符串
     */
    void TrimString(std::string& str);
};

} // namespace sqlcc

#endif // SQLCC_SQL_EXECUTOR_H