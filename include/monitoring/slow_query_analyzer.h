#ifndef SQLCC_MONITORING_SLOW_QUERY_ANALYZER_H
#define SQLCC_MONITORING_SLOW_QUERY_ANALYZER_H

#include "monitoring/performance_monitor.h"
#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <functional>

namespace sqlcc {

struct QueryPerformancePattern {
    std::string pattern_type;        // "table_scan", "full_join", "complex_subquery", etc.
    std::string description;         // 模式描述
    std::vector<std::string> recommendations; // 优化建议
    double impact_score;            // 影响分数 (0-100)
};

struct OptimizationSuggestion {
    std::string suggestion_id;
    std::string query_id;
    std::string suggestion_type;     // "index", "query_rewrite", "schema_change", etc.
    std::string description;
    std::vector<std::string> sql_commands; // 具体的SQL命令
    double estimated_improvement;   // 预估改进百分比
    std::string difficulty;         // "easy", "medium", "hard"
    std::chrono::system_clock::time_point created_at;

    // 建议状态
    bool applied = false;
    bool verified = false;
    std::chrono::system_clock::time_point applied_at;
    double actual_improvement = 0.0;
};

class SlowQueryAnalyzer {
public:
    SlowQueryAnalyzer();
    ~SlowQueryAnalyzer() = default;

    // 分析慢查询
    std::vector<OptimizationSuggestion> analyze_slow_query(const QueryMetrics& query_metrics);

    // 批量分析
    std::vector<OptimizationSuggestion> analyze_slow_queries(
        const std::vector<QueryMetrics>& queries);

    // 获取历史建议
    std::vector<OptimizationSuggestion> get_suggestions(
        size_t limit = 100,
        const std::string& suggestion_type = "") const;

    // 标记建议为已应用
    void mark_suggestion_applied(const std::string& suggestion_id,
                                double actual_improvement = 0.0);

    // 验证建议效果
    void verify_suggestion_effectiveness(const std::string& suggestion_id,
                                       const QueryMetrics& before_metrics,
                                       const QueryMetrics& after_metrics);

private:
    // 分析方法
    std::vector<QueryPerformancePattern> identify_performance_patterns(
        const QueryMetrics& query_metrics);

    OptimizationSuggestion generate_index_suggestion(
        const QueryPerformancePattern& pattern,
        const QueryMetrics& query_metrics);

    OptimizationSuggestion generate_query_rewrite_suggestion(
        const QueryPerformancePattern& pattern,
        const QueryMetrics& query_metrics);

    OptimizationSuggestion generate_schema_change_suggestion(
        const QueryPerformancePattern& pattern,
        const QueryMetrics& query_metrics);

    // 模式识别
    QueryPerformancePattern detect_table_scan_pattern(const QueryMetrics& query_metrics);
    QueryPerformancePattern detect_full_join_pattern(const QueryMetrics& query_metrics);
    QueryPerformancePattern detect_complex_subquery_pattern(const QueryMetrics& query_metrics);
    QueryPerformancePattern detect_missing_index_pattern(const QueryMetrics& query_metrics);
    QueryPerformancePattern detect_large_sort_pattern(const QueryMetrics& query_metrics);
    QueryPerformancePattern detect_cartesian_product_pattern(const QueryMetrics& query_metrics);

    // 辅助方法
    std::string extract_table_name(const std::string& query_text);
    std::vector<std::string> extract_columns(const std::string& query_text);
    std::vector<std::string> extract_join_conditions(const std::string& query_text);
    bool has_where_clause(const std::string& query_text);
    bool has_limit_clause(const std::string& query_text);

    double calculate_impact_score(const QueryPerformancePattern& pattern,
                                const QueryMetrics& query_metrics);

    std::string generate_suggestion_id();

    // 数据存储
    std::vector<OptimizationSuggestion> suggestions_history_;
    std::unordered_map<std::string, OptimizationSuggestion> suggestions_map_;

    // 配置参数
    double min_execution_time_threshold_ms_ = 1000.0;  // 慢查询阈值
    size_t max_suggestions_per_query_ = 3;             // 每个查询最多建议数
    size_t max_history_size_ = 10000;                  // 历史建议最大数量
};

} // namespace sqlcc

#endif // SQLCC_MONITORING_SLOW_QUERY_ANALYZER_H
