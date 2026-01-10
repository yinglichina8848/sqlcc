#ifndef SQLCC_EXECUTION_COST_ESTIMATOR_H
#define SQLCC_EXECUTION_COST_ESTIMATOR_H

#include <memory>
#include <vector>
#include <unordered_map>
#include <string>

namespace sqlcc {

struct QueryCost {
    double cpu_cost = 0.0;        // CPU代价
    double io_cost = 0.0;         // I/O代价
    double network_cost = 0.0;    // 网络代价
    double memory_cost = 0.0;     // 内存代价
    double total_cost = 0.0;      // 总代价

    QueryCost& operator+=(const QueryCost& other) {
        cpu_cost += other.cpu_cost;
        io_cost += other.io_cost;
        network_cost += other.network_cost;
        memory_cost += other.memory_cost;
        total_cost += other.total_cost;
        return *this;
    }

    QueryCost operator+(const QueryCost& other) const {
        QueryCost result = *this;
        result += other;
        return result;
    }
};

struct TableStatistics {
    std::string table_name;
    size_t row_count = 0;         // 表行数
    size_t page_count = 0;        // 表页数
    size_t avg_row_size = 0;      // 平均行大小
    std::unordered_map<std::string, size_t> column_cardinalities;  // 列基数

    // 直方图数据 (简化版)
    std::unordered_map<std::string, std::vector<double>> column_histograms;
};

struct IndexStatistics {
    std::string index_name;
    std::string table_name;
    size_t levels = 0;            // 索引层数
    size_t pages = 0;             // 索引页数
    double selectivity = 1.0;     // 选择度
    bool is_unique = false;       // 是否唯一索引
};

class CostEstimator {
public:
    CostEstimator();
    ~CostEstimator() = default;

    // 主要接口
    QueryCost estimate_query_cost(const class ExecutionPlan* plan);

    // 代价计算方法
    QueryCost estimate_scan_cost(const class TableScan* scan);
    QueryCost estimate_index_scan_cost(const class IndexScan* index_scan);
    QueryCost estimate_join_cost(const class Join* join);
    QueryCost estimate_sort_cost(const class Sort* sort);
    QueryCost estimate_aggregate_cost(const class Aggregate* aggregate);
    QueryCost estimate_filter_cost(const class Filter* filter);

    // 统计信息管理
    void update_table_statistics(const std::string& table_name,
                               const TableStatistics& stats);
    void update_index_statistics(const std::string& index_name,
                               const IndexStatistics& stats);

    const TableStatistics* get_table_statistics(const std::string& table_name) const;
    const IndexStatistics* get_index_statistics(const std::string& index_name) const;

    // 代价参数设置
    void set_cpu_cost_per_row(double cost) { cpu_cost_per_row_ = cost; }
    void set_io_cost_per_page(double cost) { io_cost_per_page_ = cost; }
    void set_memory_cost_per_byte(double cost) { memory_cost_per_byte_ = cost; }

private:
    // 基础代价参数
    double cpu_cost_per_row_ = 0.01;      // 每行CPU代价
    double io_cost_per_page_ = 1.0;       // 每页I/O代价
    double memory_cost_per_byte_ = 0.0001; // 每字节内存代价
    double network_cost_per_kb_ = 0.1;    // 每KB网络代价

    // 统计信息存储
    std::unordered_map<std::string, TableStatistics> table_stats_;
    std::unordered_map<std::string, IndexStatistics> index_stats_;

    // 内部辅助方法
    double calculate_selectivity(const class Expression* condition,
                               const TableStatistics* table_stats) const;
    size_t estimate_result_rows(const class ExecutionPlan* plan) const;
    double calculate_index_selectivity(const std::string& column_name,
                                     const std::string& operator_type,
                                     const std::string& value,
                                     const IndexStatistics* index_stats) const;
};

} // namespace sqlcc

#endif // SQLCC_EXECUTION_COST_ESTIMATOR_H
