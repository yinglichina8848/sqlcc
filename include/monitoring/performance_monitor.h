#ifndef SQLCC_MONITORING_PERFORMANCE_MONITOR_H
#define SQLCC_MONITORING_PERFORMANCE_MONITOR_H

#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <atomic>
#include <chrono>
#include <mutex>

namespace sqlcc {

struct SystemMetrics {
    double cpu_usage_percent = 0.0;      // CPU使用率
    double memory_usage_mb = 0.0;        // 内存使用量(MB)
    double disk_io_mb_per_sec = 0.0;     // 磁盘I/O速率(MB/s)
    double network_io_mb_per_sec = 0.0;  // 网络I/O速率(MB/s)
    size_t active_connections = 0;       // 活跃连接数
    size_t total_connections = 0;        // 总连接数
    std::chrono::system_clock::time_point timestamp;
};

struct DatabaseMetrics {
    size_t active_transactions = 0;      // 活跃事务数
    size_t committed_transactions = 0;   // 已提交事务数
    size_t rolled_back_transactions = 0; // 已回滚事务数
    double avg_query_time_ms = 0.0;      // 平均查询时间(ms)
    double queries_per_second = 0.0;     // 每秒查询数(QPS)
    size_t cache_hit_rate_percent = 0;   // 缓存命中率(%)
    size_t buffer_pool_hit_rate_percent = 0; // 缓冲池命中率(%)
    std::chrono::system_clock::time_point timestamp;
};

struct QueryMetrics {
    std::string query_id;
    std::string query_text;
    double execution_time_ms = 0.0;
    size_t rows_affected = 0;
    size_t rows_returned = 0;
    bool is_cached = false;
    std::string execution_plan;
    std::chrono::system_clock::time_point start_time;
    std::chrono::system_clock::time_point end_time;
};

struct AlertConfig {
    std::string metric_name;
    std::string condition;  // ">", "<", ">=", "<=", "=="
    double threshold;
    std::string severity;   // "low", "medium", "high", "critical"
    bool enabled = true;
};

struct Alert {
    std::string alert_id;
    std::string metric_name;
    std::string message;
    std::string severity;
    double current_value;
    double threshold;
    std::chrono::system_clock::time_point timestamp;
    bool acknowledged = false;
};

class PerformanceMonitor {
public:
    PerformanceMonitor();
    ~PerformanceMonitor() = default;

    // 核心监控接口
    void start_monitoring();
    void stop_monitoring();
    bool is_monitoring() const { return monitoring_active_.load(); }

    // 指标收集
    SystemMetrics collect_system_metrics();
    DatabaseMetrics collect_database_metrics();
    void record_query_metrics(const QueryMetrics& metrics);

    // 告警管理
    void add_alert_config(const AlertConfig& config);
    void remove_alert_config(const std::string& metric_name);
    std::vector<Alert> get_active_alerts() const;
    void acknowledge_alert(const std::string& alert_id);

    // 历史数据查询
    std::vector<SystemMetrics> get_system_metrics_history(
        std::chrono::system_clock::time_point start,
        std::chrono::system_clock::time_point end) const;

    std::vector<DatabaseMetrics> get_database_metrics_history(
        std::chrono::system_clock::time_point start,
        std::chrono::system_clock::time_point end) const;

    std::vector<QueryMetrics> get_slow_queries(
        size_t limit = 100,
        double min_execution_time_ms = 1000.0) const;

    // 配置管理
    void set_collection_interval_ms(size_t interval) {
        collection_interval_ms_ = interval;
    }

    void set_max_history_size(size_t size) {
        max_history_size_ = size;
    }

    void set_slow_query_threshold_ms(double threshold) {
        slow_query_threshold_ms_ = threshold;
    }

private:
    // 监控状态
    std::atomic<bool> monitoring_active_{false};
    std::unique_ptr<std::thread> monitoring_thread_;

    // 配置参数
    size_t collection_interval_ms_ = 5000;  // 5秒收集间隔
    size_t max_history_size_ = 10000;      // 最大历史记录数
    double slow_query_threshold_ms_ = 1000.0; // 慢查询阈值

    // 数据存储
    mutable std::mutex data_mutex_;
    std::vector<SystemMetrics> system_metrics_history_;
    std::vector<DatabaseMetrics> database_metrics_history_;
    std::vector<QueryMetrics> query_metrics_history_;
    std::vector<Alert> active_alerts_;

    // 告警配置
    std::unordered_map<std::string, AlertConfig> alert_configs_;

    // 内部方法
    void monitoring_loop();
    void check_alerts(const SystemMetrics& sys_metrics,
                     const DatabaseMetrics& db_metrics);
    void cleanup_old_data();
    bool should_trigger_alert(const std::string& metric_name,
                            double current_value,
                            const AlertConfig& config) const;
    std::string generate_alert_id() const;
};

} // namespace sqlcc

#endif // SQLCC_MONITORING_PERFORMANCE_MONITOR_H
