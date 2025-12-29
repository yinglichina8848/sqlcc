#pragma once

#include <memory>
#include <vector>
#include <string>
#include "sql_parser/window_function.h"
#include "core/core_database_manager.h"
#include "core/execution_context.h"
#include "core/execution_result.h"

namespace sqlcc {

class WindowFunctionExecutor {
public:
    explicit WindowFunctionExecutor(std::shared_ptr<DatabaseManager> db_manager);
    ~WindowFunctionExecutor();

    // 执行单个窗口函数
    ExecutionResult execute(const sql_parser::WindowFunction& stmt, ExecutionContext& context);

    // 执行多个窗口函数（基于基础查询结果）
    ExecutionResult executeWindowFunctions(
        const std::vector<std::unique_ptr<sql_parser::WindowFunction>>& window_funcs,
        const ExecutionResult& base_result,
        ExecutionContext& context);

private:
    std::shared_ptr<DatabaseManager> db_manager_;

    // 数据分区和排序
    std::vector<std::vector<Row>> partitionAndSortData(
        const ExecutionResult& data,
        const std::vector<std::string>& partitions,
        const std::vector<std::string>& order_cols,
        const std::vector<bool>& order_asc);

    void sortPartitionData(std::vector<Row>& partition_data,
                          const std::vector<std::string>& order_cols,
                          const std::vector<bool>& order_asc,
                          const std::vector<ColumnMeta>& column_metadata);

    // 窗口函数计算
    void calculateWindowFunction(
        const sql_parser::WindowFunction& window_func,
        const std::vector<std::vector<Row>>& partitions,
        std::vector<std::string>& results);

    // 具体窗口函数实现
    void calculateRowNumber(const std::vector<Row>& rows, std::vector<std::string>& results);
    void calculateRank(const std::vector<std::vector<Row>>& partitions, std::vector<std::string>& results);
    void calculateDenseRank(const std::vector<std::vector<Row>>& partitions, std::vector<std::string>& results);
    void calculateAggregateWindowFunction(
        const sql_parser::WindowFunction& window_func,
        const std::vector<std::vector<Row>>& partitions,
        std::vector<std::string>& results);

    // 聚合函数辅助方法
    std::string calculateSum(const std::vector<Row>& rows);
    std::string calculateAvg(const std::vector<Row>& rows);
    std::string calculateMin(const std::vector<Row>& rows);
    std::string calculateMax(const std::vector<Row>& rows);

    // 辅助函数
    size_t findColumnIndex(const std::vector<ColumnMeta>& columns, const std::string& column_name);
    int compareRows(const Row& a, const Row& b);
};

} // namespace sqlcc
