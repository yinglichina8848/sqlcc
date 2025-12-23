#ifndef SQLCC_CORE_SQL_EXECUTOR_INTERFACE_H
#define SQLCC_CORE_SQL_EXECUTOR_INTERFACE_H

#include <string>
#include <memory>
#include <vector>
#include <optional>

namespace sqlcc {

namespace sql_parser {
    class Statement;
    class SelectStatement;
    class InsertStatement;
    class UpdateStatement;
    class DeleteStatement;
}

class QueryPlan;
class ExecutionResult;

/**
 * @brief SQL执行器接口 - 核心服务层组件
 *
 * 定义了SQL执行器的完整功能接口，支持各种SQL操作，
 * 提供统一的执行入口和结果处理机制。
 */
class SqlExecutorInterface {
public:
    virtual ~SqlExecutorInterface() = default;

    // ========== 核心执行方法 ==========

    /**
     * @brief 执行SQL语句
     * @param sql SQL语句字符串
     * @return 执行结果消息
     */
    virtual std::string Execute(const std::string& sql) = 0;

    /**
     * @brief 执行预解析的SQL语句
     * @param stmt 解析后的语句对象
     * @return 执行结果
     */
    virtual std::shared_ptr<ExecutionResult> Execute(std::shared_ptr<sql_parser::Statement> stmt) = 0;

    /**
     * @brief 执行文件中的SQL语句
     * @param file_path SQL文件路径
     * @return 执行结果消息
     */
    virtual std::string ExecuteFile(const std::string& file_path) = 0;

    // ========== 查询执行方法 ==========

    /**
     * @brief 执行SELECT查询
     * @param select_stmt SELECT语句对象
     * @return 查询结果
     */
    virtual std::shared_ptr<ExecutionResult> ExecuteQuery(std::shared_ptr<sql_parser::SelectStatement> select_stmt) = 0;

    /**
     * @brief 执行SELECT查询（字符串形式）
     * @param sql SELECT语句字符串
     * @return 查询结果
     */
    virtual std::shared_ptr<ExecutionResult> ExecuteQuery(const std::string& sql) = 0;

    // ========== 修改操作方法 ==========

    /**
     * @brief 执行INSERT语句
     * @param insert_stmt INSERT语句对象
     * @return 影响的行数
     */
    virtual size_t ExecuteInsert(std::shared_ptr<sql_parser::InsertStatement> insert_stmt) = 0;

    /**
     * @brief 执行UPDATE语句
     * @param update_stmt UPDATE语句对象
     * @return 影响的行数
     */
    virtual size_t ExecuteUpdate(std::shared_ptr<sql_parser::UpdateStatement> update_stmt) = 0;

    /**
     * @brief 执行DELETE语句
     * @param delete_stmt DELETE语句对象
     * @return 影响的行数
     */
    virtual size_t ExecuteDelete(std::shared_ptr<sql_parser::DeleteStatement> delete_stmt) = 0;

    // ========== 事务管理方法 ==========

    /**
     * @brief 开始事务
     * @param isolation_level 隔离级别
     * @return 事务ID
     */
    virtual int64_t BeginTransaction(std::optional<std::string> isolation_level = std::nullopt) = 0;

    /**
     * @brief 提交事务
     * @param transaction_id 事务ID
     * @return 是否成功
     */
    virtual bool CommitTransaction(int64_t transaction_id) = 0;

    /**
     * @brief 回滚事务
     * @param transaction_id 事务ID
     * @return 是否成功
     */
    virtual bool RollbackTransaction(int64_t transaction_id) = 0;

    // ========== 预编译语句方法 ==========

    /**
     * @brief 准备预编译语句
     * @param sql SQL模板
     * @return 预编译语句ID
     */
    virtual int64_t PrepareStatement(const std::string& sql) = 0;

    /**
     * @brief 执行预编译语句
     * @param stmt_id 预编译语句ID
     * @param parameters 参数值列表
     * @return 执行结果
     */
    virtual std::shared_ptr<ExecutionResult> ExecutePrepared(int64_t stmt_id,
                                                            const std::vector<std::string>& parameters) = 0;

    /**
     * @brief 释放预编译语句
     * @param stmt_id 预编译语句ID
     */
    virtual void ReleasePrepared(int64_t stmt_id) = 0;

    // ========== 元数据查询方法 ==========

    /**
     * @brief 获取表结构信息
     * @param table_name 表名
     * @return 表结构描述
     */
    virtual std::string GetTableSchema(const std::string& table_name) = 0;

    /**
     * @brief 获取数据库中的所有表名
     * @return 表名列表
     */
    virtual std::vector<std::string> GetTableNames() = 0;

    /**
     * @brief 检查表是否存在
     * @param table_name 表名
     * @return 是否存在
     */
    virtual bool TableExists(const std::string& table_name) = 0;

    // ========== 错误处理方法 ==========

    /**
     * @brief 获取最后一次执行的错误信息
     * @return 错误信息
     */
    virtual std::string GetLastError() const = 0;

    /**
     * @brief 获取执行统计信息
     * @return 统计信息
     */
    virtual std::string GetExecutionStats() const = 0;

    /**
     * @brief 清除错误状态
     */
    virtual void ClearError() = 0;

    // ========== 配置和初始化方法 ==========

    /**
     * @brief 初始化执行器
     * @return 是否成功
     */
    virtual bool Initialize() = 0;

    /**
     * @brief 检查是否已初始化
     * @return 初始化状态
     */
    virtual bool IsInitialized() const = 0;

    /**
     * @brief 设置执行超时时间
     * @param timeout_ms 超时时间（毫秒）
     */
    virtual void SetTimeout(int64_t timeout_ms) = 0;

    /**
     * @brief 获取当前数据库名
     * @return 数据库名
     */
    virtual std::string GetCurrentDatabase() const = 0;

    /**
     * @brief 切换数据库
     * @param db_name 数据库名
     * @return 是否成功
     */
    virtual bool UseDatabase(const std::string& db_name) = 0;
};

} // namespace sqlcc

#endif // SQLCC_CORE_SQL_EXECUTOR_INTERFACE_H
