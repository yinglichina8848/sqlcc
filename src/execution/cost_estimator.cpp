#include "execution/cost_estimator.h"
#include <algorithm>
#include <cmath>

namespace sqlcc {

CostEstimator::CostEstimator() {
    // 初始化默认统计信息（示例数据）
    // 在实际使用中，这些数据应该从系统目录或统计信息表中获取
}

QueryCost CostEstimator::estimate_query_cost(const class ExecutionPlan* plan) {
    if (!plan) {
        return QueryCost{};
    }

    QueryCost total_cost;

    // 递归计算执行计划的代价
    // 这里需要根据实际的执行计划节点类型进行代价计算
    // 暂时返回基础代价

    total_cost.total_cost = total_cost.cpu_cost + total_cost.io_cost +
                           total_cost.network_cost + total_cost.memory_cost;

    return total_cost;
}

QueryCost CostEstimator::estimate_scan_cost(const class TableScan* scan) {
    QueryCost cost;

    if (!scan) return cost;

    // 获取表统计信息
    const auto* table_stats = get_table_statistics(scan->get_table_name());
    if (!table_stats) {
        // 使用默认估计
        cost.io_cost = 100.0;  // 假设100页
        cost.cpu_cost = 50.0;  // 假设50行处理
    } else {
        // 基于实际统计信息计算
        size_t estimated_pages = (table_stats->row_count * table_stats->avg_row_size) / 4096 + 1;
        cost.io_cost = estimated_pages * io_cost_per_page_;
        cost.cpu_cost = table_stats->row_count * cpu_cost_per_row_;
    }

    cost.total_cost = cost.cpu_cost + cost.io_cost;
    return cost;
}

QueryCost CostEstimator::estimate_index_scan_cost(const class IndexScan* index_scan) {
    QueryCost cost;

    if (!index_scan) return cost;

    const auto* index_stats = get_index_statistics(index_scan->get_index_name());
    if (!index_stats) {
        // 默认索引扫描代价
        cost.io_cost = 10.0;  // 索引查找 + 少量数据页
        cost.cpu_cost = 5.0;  // 索引查找代价
    } else {
        // 基于索引统计信息计算
        double selectivity = calculate_index_selectivity(
            index_scan->get_column_name(),
            index_scan->get_operator(),
            index_scan->get_value(),
            index_stats
        );

        // 索引层级查找代价
        cost.io_cost = index_stats->levels * io_cost_per_page_;
        // 数据页访问代价（基于选择度）
        cost.io_cost += selectivity * index_stats->pages * io_cost_per_page_;
        cost.cpu_cost = selectivity * 1000 * cpu_cost_per_row_;  // 估算CPU处理
    }

    cost.total_cost = cost.cpu_cost + cost.io_cost;
    return cost;
}

QueryCost CostEstimator::estimate_join_cost(const class Join* join) {
    QueryCost cost;

    if (!join) return cost;

    // 简化版JOIN代价计算
    // 实际应该考虑不同的JOIN算法（Nested Loop, Hash Join, Sort-Merge Join）

    auto left_cost = estimate_query_cost(join->get_left_child());
    auto right_cost = estimate_query_cost(join->get_right_child());

    size_t left_rows = estimate_result_rows(join->get_left_child());
    size_t right_rows = estimate_result_rows(join->get_right_child());

    // Nested Loop Join代价估计
    cost.cpu_cost = left_rows * right_rows * cpu_cost_per_row_;
    cost.io_cost = left_cost.io_cost + right_cost.io_cost +
                   (left_rows * right_rows) / 1000.0 * io_cost_per_page_;  // 估算随机I/O

    cost.total_cost = cost.cpu_cost + cost.io_cost;
    return cost;
}

QueryCost CostEstimator::estimate_sort_cost(const class Sort* sort) {
    QueryCost cost;

    if (!sort) return cost;

    size_t input_rows = estimate_result_rows(sort->get_child());

    // 排序代价基于比较次数
    // 假设使用快速排序，比较次数约为 n*log(n)
    double comparisons = input_rows * std::log2(std::max(size_t(1), input_rows));
    cost.cpu_cost = comparisons * cpu_cost_per_row_;

    // 内存使用代价
    cost.memory_cost = input_rows * 8 * memory_cost_per_byte_;  // 假设8字节指针

    cost.total_cost = cost.cpu_cost + cost.memory_cost;
    return cost;
}

QueryCost CostEstimator::estimate_aggregate_cost(const class Aggregate* aggregate) {
    QueryCost cost;

    if (!aggregate) return cost;

    size_t input_rows = estimate_result_rows(aggregate->get_child());

    // 聚合操作代价
    cost.cpu_cost = input_rows * cpu_cost_per_row_ * 2.0;  // 聚合需要额外计算

    // 分组操作可能需要哈希表
    if (aggregate->has_group_by()) {
        cost.memory_cost = input_rows * 16 * memory_cost_per_byte_;  // 哈希表开销
    }

    cost.total_cost = cost.cpu_cost + cost.memory_cost;
    return cost;
}

QueryCost CostEstimator::estimate_filter_cost(const class Filter* filter) {
    QueryCost cost;

    if (!filter) return cost;

    auto child_cost = estimate_query_cost(filter->get_child());
    size_t input_rows = estimate_result_rows(filter->get_child());

    // 过滤条件评估代价
    double selectivity = calculate_selectivity(filter->get_condition(),
                                              get_table_statistics(""));  // 需要实际表名

    cost.cpu_cost = input_rows * cpu_cost_per_row_;  // 条件评估
    cost.io_cost = child_cost.io_cost * selectivity;  // 只访问满足条件的页

    cost.total_cost = cost.cpu_cost + cost.io_cost;
    return cost;
}

void CostEstimator::update_table_statistics(const std::string& table_name,
                                          const TableStatistics& stats) {
    table_stats_[table_name] = stats;
}

void CostEstimator::update_index_statistics(const std::string& index_name,
                                          const IndexStatistics& stats) {
    index_stats_[index_name] = stats;
}

const TableStatistics* CostEstimator::get_table_statistics(const std::string& table_name) const {
    auto it = table_stats_.find(table_name);
    return it != table_stats_.end() ? &it->second : nullptr;
}

const IndexStatistics* CostEstimator::get_index_statistics(const std::string& index_name) const {
    auto it = index_stats_.find(index_name);
    return it != index_stats_.end() ? &it->second : nullptr;
}

double CostEstimator::calculate_selectivity(const class Expression* condition,
                                          const TableStatistics* table_stats) const {
    if (!condition) return 1.0;

    // 简化版选择度计算
    // 实际应该解析条件表达式并基于统计信息计算

    // 默认假设10%的选择度
    return 0.1;
}

size_t CostEstimator::estimate_result_rows(const class ExecutionPlan* plan) const {
    if (!plan) return 0;

    // 简化版结果行数估计
    // 实际应该基于统计信息和操作符类型进行精确计算

    // 默认假设返回1000行
    return 1000;
}

double CostEstimator::calculate_index_selectivity(const std::string& column_name,
                                                const std::string& operator_type,
                                                const std::string& value,
                                                const IndexStatistics* index_stats) const {
    if (!index_stats) return 0.1;  // 默认选择度

    // 基于操作符类型估算选择度
    if (operator_type == "=") {
        return 1.0 / std::max(size_t(1), index_stats->pages);  // 唯一值假设
    } else if (operator_type == ">" || operator_type == "<" ||
               operator_type == ">=" || operator_type == "<=") {
        return 0.3;  // 范围查询假设30%选择度
    } else if (operator_type == "LIKE") {
        return 0.1;  // 模糊匹配假设10%选择度
    }

    return index_stats->selectivity;  // 使用索引默认选择度
}

} // namespace sqlcc
