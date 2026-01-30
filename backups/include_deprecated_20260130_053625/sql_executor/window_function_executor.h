#ifndef SQLCC_SQL_EXECUTOR_WINDOW_FUNCTION_EXECUTOR_H
#define SQLCC_SQL_EXECUTOR_WINDOW_FUNCTION_EXECUTOR_H

#include "sql_parser/window_function.h"
#include "database_manager.h"
#include <memory>
#include <vector>
#include <string>

namespace sqlcc {

/**
 * @brief 窗口函数执行结果
 */
struct WindowFunctionResult {
    std::vector<std::string> values;
    bool success;
    std::string error_message;
};

/**
 * @brief 窗口函数执行器
 * 
 * 负责执行各种窗口函数，如ROW_NUMBER、RANK、DENSE_RANK等
 */
class WindowFunctionExecutor {
public:
    WindowFunctionExecutor(std::shared_ptr<DatabaseManager> db_manager);
    ~WindowFunctionExecutor() = default;

    /**
     * @brief 执行窗口函数
     * @param window_function 窗口函数节点
     * @param table_name 表名
     * @param data 数据行
     * @return 执行结果
     */
    WindowFunctionResult executeWindowFunction(
        const sql_parser::WindowFunction& window_function,
        const std::string& table_name,
        const std::vector<std::vector<std::string>>& data);

private:
    /**
     * @brief 执行ROW_NUMBER函数
     */
    WindowFunctionResult executeRowNumber(
        const sql_parser::WindowFunction& window_function,
        const std::vector<std::vector<std::string>>& data);

    /**
     * @brief 执行RANK函数
     */
    WindowFunctionResult executeRank(
        const sql_parser::WindowFunction& window_function,
        const std::vector<std::vector<std::string>>& data);

    /**
     * @brief 执行DENSE_RANK函数
     */
    WindowFunctionResult executeDenseRank(
        const sql_parser::WindowFunction& window_function,
        const std::vector<std::vector<std::string>>& data);

    /**
     * @brief 根据分区和排序列对数据进行排序
     */
    std::vector<size_t> sortData(
        const std::vector<std::string>& partition_columns,
        const std::vector<std::string>& order_columns,
        const std::string& order_direction,
        const std::vector<std::vector<std::string>>& data);

private:
    std::shared_ptr<DatabaseManager> db_manager_;
};

} // namespace sqlcc

#endif // SQLCC_SQL_EXECUTOR_WINDOW_FUNCTION_EXECUTOR_H