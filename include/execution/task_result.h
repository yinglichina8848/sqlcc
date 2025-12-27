#ifndef SQLCC_EXECUTION_TASK_RESULT_H
#define SQLCC_EXECUTION_TASK_RESULT_H

#include <string>
#include <memory>
#include <chrono>

namespace sqlcc {

/**
 * @brief 任务类型枚举
 */
enum class TaskType {
    UNKNOWN = 0,      // 未知类型
    DDL_EXECUTE,      // DDL执行
    DML_EXECUTE,      // DML执行
    QUERY_EXECUTE,    // 查询执行
    TRANSACTION,      // 事务处理
    PROCEDURE_CALL,   // 存储过程调用
    TRIGGER_EXECUTE,  // 触发器执行
    SQL_EXECUTE,      // 通用SQL执行
    SYSTEM_MAINTAIN   // 系统维护
};

/**
 * @brief 任务执行结果类
 *
 * 封装任务执行的结果信息，包括成功状态、错误信息、执行时间等
 */
class TaskResult {
public:
    /**
     * @brief 构造函数
     * @param success 执行是否成功
     * @param task_type 任务类型
     */
    TaskResult(bool success = false, TaskType task_type = TaskType::DDL_EXECUTE);

    /**
     * @brief 析构函数
     */
    ~TaskResult() = default;

    /**
     * @brief 设置执行成功状态
     * @param success 成功标志
     */
    void set_success(bool success);

    /**
     * @brief 获取执行成功状态
     * @return 成功标志
     */
    bool get_success() const;

    /**
     * @brief 设置错误信息
     * @param error_msg 错误消息
     */
    void set_error_message(const std::string& error_msg);

    /**
     * @brief 获取错误信息
     * @return 错误消息
     */
    std::string get_error_message() const;

    /**
     * @brief 设置任务类型
     * @param type 任务类型
     */
    void set_task_type(TaskType type);

    /**
     * @brief 获取任务类型
     * @return 任务类型
     */
    TaskType get_task_type() const;

    /**
     * @brief 设置执行开始时间
     */
    void set_start_time();

    /**
     * @brief 设置执行结束时间
     */
    void set_end_time();

    /**
     * @brief 获取执行持续时间（毫秒）
     * @return 执行时间
     */
    long long get_execution_time_ms() const;

    /**
     * @brief 设置影响的行数
     * @param rows 行数
     */
    void set_affected_rows(size_t rows);

    /**
     * @brief 获取影响的行数
     * @return 行数
     */
    size_t get_affected_rows() const;

    /**
     * @brief 设置结果消息
     * @param message 结果消息
     */
    void set_result_message(const std::string& message);

    /**
     * @brief 获取结果消息
     * @return 结果消息
     */
    std::string get_result_message() const;

    /**
     * @brief 检查是否有错误
     * @return 是否有错误
     */
    bool has_error() const;

    /**
     * @brief 获取结果摘要
     * @return 摘要字符串
     */
    std::string get_summary() const;

private:
    bool success_;                          // 执行成功标志
    TaskType task_type_;                    // 任务类型
    std::string error_message_;             // 错误信息
    std::string result_message_;            // 结果消息
    size_t affected_rows_;                  // 影响的行数

    std::chrono::steady_clock::time_point start_time_;    // 开始时间
    std::chrono::steady_clock::time_point end_time_;      // 结束时间
    bool time_recorded_;                    // 时间是否已记录
};

} // namespace sqlcc

#endif // SQLCC_EXECUTION_TASK_RESULT_H
