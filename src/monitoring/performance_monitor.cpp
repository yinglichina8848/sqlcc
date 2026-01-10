#include "monitoring/performance_monitor.h"
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>
#include <algorithm>
#include <iostream>

namespace sqlcc {

PerformanceMonitor::PerformanceMonitor() {
    // 初始化默认告警配置
    alert_configs_["cpu_usage"] = {"cpu_usage", ">", 80.0, "medium", true};
    alert_configs_["memory_usage"] = {"memory_usage", ">", 90.0, "high", true};
    alert_configs_["active_connections"] = {"active_connections", ">", 1000, "medium", true};
    alert_configs_["avg_query_time"] = {"avg_query_time", ">", 2000.0, "high", true};
}

void PerformanceMonitor::start_monitoring() {
    if (monitoring_active_.load()) {
        return; // 已经在监控中
    }

    monitoring_active_.store(true);
    monitoring_thread_ = std::make_unique<std::thread>(&PerformanceMonitor::monitoring_loop, this);
}

void PerformanceMonitor::stop_monitoring() {
    monitoring_active_.store(false);
    if (monitoring_thread_ && monitoring_thread_->joinable()) {
        monitoring_thread_->join();
    }
}

SystemMetrics PerformanceMonitor::collect_system_metrics() {
    SystemMetrics metrics;
    metrics.timestamp = std::chrono::system_clock::now();

    // 简化实现 - 在实际系统中需要调用系统API
    // 这里提供模拟数据用于演示

    // CPU使用率 (0-100%)
    metrics.cpu_usage_percent = 45.2 + (rand() % 20); // 模拟45-65%

    // 内存使用量 (MB)
    metrics.memory_usage_mb = 1024.0 + (rand() % 512); // 模拟1-1.5GB

    // 磁盘I/O速率 (MB/s)
    metrics.disk_io_mb_per_sec = 50.0 + (rand() % 50); // 模拟50-100MB/s

    // 网络I/O速率 (MB/s)
    metrics.network_io_mb_per_sec = 10.0 + (rand() % 20); // 模拟10-30MB/s

    // 连接数
    metrics.active_connections = 150 + (rand() % 50); // 模拟150-200
    metrics.total_connections = metrics.active_connections + 50;

    return metrics;
}

DatabaseMetrics PerformanceMonitor::collect_database_metrics() {
    DatabaseMetrics metrics;
    metrics.timestamp = std::chrono::system_clock::now();

    // 简化实现 - 在实际系统中需要从数据库管理器获取
    // 这里提供模拟数据用于演示

    // 活跃事务数
    metrics.active_transactions = 25 + (rand() % 25); // 模拟25-50

    // 事务统计
    metrics.committed_transactions = 1250 + (rand() % 250); // 模拟1250-1500
    metrics.rolled_back_transactions = 15 + (rand() % 10); // 模拟15-25

    // 查询性能指标
    metrics.avg_query_time_ms = 45.0 + (rand() % 30); // 模拟45-75ms
    metrics.queries_per_second = 850 + (rand() % 200); // 模拟850-1050 QPS

    // 缓存命中率
    metrics.cache_hit_rate_percent = 85 + (rand() % 10); // 模拟85-95%
    metrics.buffer_pool_hit_rate_percent = 92 + (rand() % 5); // 模拟92-97%

    return metrics;
}

void PerformanceMonitor::record_query_metrics(const QueryMetrics& metrics) {
    std::lock_guard<std::mutex> lock(data_mutex_);

    // 记录查询指标
    query_metrics_history_.push_back(metrics);

    // 清理旧数据
    cleanup_old_data();

    // 检查是否为慢查询
    if (metrics.execution_time_ms >= slow_query_threshold_ms_) {
        // 可以在这里触发慢查询告警或记录
    }
}

void PerformanceMonitor::add_alert_config(const AlertConfig& config) {
    std::lock_guard<std::mutex> lock(data_mutex_);
    alert_configs_[config.metric_name] = config;
}

void PerformanceMonitor::remove_alert_config(const std::string& metric_name) {
    std::lock_guard<std::mutex> lock(data_mutex_);
    alert_configs_.erase(metric_name);
}

std::vector<Alert> PerformanceMonitor::get_active_alerts() const {
    std::lock_guard<std::mutex> lock(data_mutex_);
    return active_alerts_;
}

void PerformanceMonitor::acknowledge_alert(const std::string& alert_id) {
    std::lock_guard<std::mutex> lock(data_mutex_);
    for (auto& alert : active_alerts_) {
        if (alert.alert_id == alert_id) {
            alert.acknowledged = true;
            break;
        }
    }
}

std::vector<SystemMetrics> PerformanceMonitor::get_system_metrics_history(
    std::chrono::system_clock::time_point start,
    std::chrono::system_clock::time_point end) const {

    std::lock_guard<std::mutex> lock(data_mutex_);
    std::vector<SystemMetrics> result;

    for (const auto& metrics : system_metrics_history_) {
        if (metrics.timestamp >= start && metrics.timestamp <= end) {
            result.push_back(metrics);
        }
    }

    return result;
}

std::vector<DatabaseMetrics> PerformanceMonitor::get_database_metrics_history(
    std::chrono::system_clock::time_point start,
    std::chrono::system_clock::time_point end) const {

    std::lock_guard<std::mutex> lock(data_mutex_);
    std::vector<DatabaseMetrics> result;

    for (const auto& metrics : database_metrics_history_) {
        if (metrics.timestamp >= start && metrics.timestamp <= end) {
            result.push_back(metrics);
        }
    }

    return result;
}

std::vector<QueryMetrics> PerformanceMonitor::get_slow_queries(
    size_t limit, double min_execution_time_ms) const {

    std::lock_guard<std::mutex> lock(data_mutex_);
    std::vector<QueryMetrics> result;

    for (const auto& query : query_metrics_history_) {
        if (query.execution_time_ms >= min_execution_time_ms) {
            result.push_back(query);
        }
    }

    // 按执行时间降序排序
    std::sort(result.begin(), result.end(),
              [](const QueryMetrics& a, const QueryMetrics& b) {
                  return a.execution_time_ms > b.execution_time_ms;
              });

    // 限制返回数量
    if (result.size() > limit) {
        result.resize(limit);
    }

    return result;
}

void PerformanceMonitor::monitoring_loop() {
    while (monitoring_active_.load()) {
        // 收集系统指标
        auto sys_metrics = collect_system_metrics();
        auto db_metrics = collect_database_metrics();

        {
            std::lock_guard<std::mutex> lock(data_mutex_);

            // 保存历史数据
            system_metrics_history_.push_back(sys_metrics);
            database_metrics_history_.push_back(db_metrics);

            // 检查告警
            check_alerts(sys_metrics, db_metrics);

            // 清理旧数据
            cleanup_old_data();
        }

        // 等待下一个收集周期
        std::this_thread::sleep_for(std::chrono::milliseconds(collection_interval_ms_));
    }
}

void PerformanceMonitor::check_alerts(const SystemMetrics& sys_metrics,
                                    const DatabaseMetrics& db_metrics) {
    // 检查CPU使用率告警
    check_metric_alert("cpu_usage", sys_metrics.cpu_usage_percent);

    // 检查内存使用率告警
    check_metric_alert("memory_usage", sys_metrics.memory_usage_mb);

    // 检查活跃连接数告警
    check_metric_alert("active_connections", static_cast<double>(sys_metrics.active_connections));

    // 检查平均查询时间告警
    check_metric_alert("avg_query_time", db_metrics.avg_query_time_ms);

    // 清理已解决的告警（简化实现）
    active_alerts_.erase(
        std::remove_if(active_alerts_.begin(), active_alerts_.end(),
                       [](const Alert& alert) {
                           // 简化：1小时后自动清理已确认的告警
                           auto now = std::chrono::system_clock::now();
                           auto duration = now - alert.timestamp;
                           return alert.acknowledged &&
                                  duration > std::chrono::hours(1);
                       }),
        active_alerts_.end());
}

void PerformanceMonitor::check_metric_alert(const std::string& metric_name, double current_value) {
    auto it = alert_configs_.find(metric_name);
    if (it == alert_configs_.end() || !it->second.enabled) {
        return;
    }

    const auto& config = it->second;
    if (should_trigger_alert(metric_name, current_value, config)) {
        Alert alert;
        alert.alert_id = generate_alert_id();
        alert.metric_name = metric_name;
        alert.message = "Metric " + metric_name + " exceeded threshold";
        alert.severity = config.severity;
        alert.current_value = current_value;
        alert.threshold = config.threshold;
        alert.timestamp = std::chrono::system_clock::now();

        active_alerts_.push_back(alert);

        // 在实际系统中，这里可以发送通知或触发其他操作
        std::cout << "ALERT: " << alert.message << " (Value: " << current_value
                  << ", Threshold: " << config.threshold << ")" << std::endl;
    }
}

void PerformanceMonitor::cleanup_old_data() {
    auto now = std::chrono::system_clock::now();
    auto cutoff = now - std::chrono::hours(24); // 保留24小时的数据

    // 清理系统指标历史
    system_metrics_history_.erase(
        std::remove_if(system_metrics_history_.begin(), system_metrics_history_.end(),
                       [cutoff](const SystemMetrics& m) { return m.timestamp < cutoff; }),
        system_metrics_history_.end());

    // 清理数据库指标历史
    database_metrics_history_.erase(
        std::remove_if(database_metrics_history_.begin(), database_metrics_history_.end(),
                       [cutoff](const DatabaseMetrics& m) { return m.timestamp < cutoff; }),
        database_metrics_history_.end());

    // 清理查询指标历史（保留最近的10000条）
    if (query_metrics_history_.size() > max_history_size_) {
        query_metrics_history_.erase(
            query_metrics_history_.begin(),
            query_metrics_history_.begin() + (query_metrics_history_.size() - max_history_size_));
    }
}

bool PerformanceMonitor::should_trigger_alert(const std::string& metric_name,
                                           double current_value,
                                           const AlertConfig& config) const {
    if (!config.enabled) {
        return false;
    }

    if (config.condition == ">") {
        return current_value > config.threshold;
    } else if (config.condition == "<") {
        return current_value < config.threshold;
    } else if (config.condition == ">=") {
        return current_value >= config.threshold;
    } else if (config.condition == "<=") {
        return current_value <= config.threshold;
    } else if (config.condition == "==") {
        return current_value == config.threshold;
    }

    return false;
}

std::string PerformanceMonitor::generate_alert_id() const {
    static size_t counter = 0;
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();

    return "alert_" + std::to_string(timestamp) + "_" + std::to_string(++counter);
}

} // namespace sqlcc
