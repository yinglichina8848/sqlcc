/**
 * @file performance_monitor.cpp
 *
 * WHY: 为什么需要性能监控？
 *
 * 数据库系统运行在复杂多变的负载环境下，需要实时了解系统性能状态。
 * 性能监控提供量化指标，帮助识别性能瓶颈、优化系统配置、保障服务质量。
 * 多维度指标体系支持从宏观统计到微观细节的全方位性能洞察。
 * 实时监控与历史趋势分析相结合，为系统调优提供科学依据。
 *
 * 设计目标：
 * - 实时性：低延迟的性能数据收集，<1ms的监控开销
 * - 多维度：覆盖CPU、内存、I/O、锁等关键性能指标
 * - 可扩展：支持自定义指标，动态注册新监控项
 * - 准确性：原子操作保证多线程环境下的数据一致性
 *
 * WHAT: 这实现了什么功能？
 *
 * 性能监控模块提供完整的指标收集和分析服务：
 * - 多种指标类型：计数器、仪表盘、直方图、计时器
 * - 指标生命周期管理：注册、更新、重置、导出
 * - 存储引擎指标：专门针对SQLCC存储引擎的性能监控
 * - 导出格式支持：JSON和纯文本格式的数据导出
 *
 * 核心组件：
 * - Metric基类：统一的指标接口定义
 * - 具体指标类：CounterMetric、GaugeMetric、HistogramMetric、TimerMetric
 * - PerformanceMonitor：指标管理中心
 * - StorageEngineMetrics：存储引擎专用指标集合
 *
 * HOW: 如何实现的？
 *
 * 技术实现要点：
 * 1. 指标类型设计：基于统计学原理的四种基础指标类型
 * 2. 线程安全：std::atomic保证多线程环境下的数据一致性
 * 3. 内存管理：智能指针管理指标生命周期
 * 4. 性能优化：延迟初始化和缓存机制减少开销
 * 5. 数据导出：支持多种格式的序列化输出
 * 6. RAII模式：ScopedTimer自动管理计时器的生命周期
 *
 * 性能优化：
 * - 原子操作：无锁的指标更新
 * - 批量导出：减少序列化开销
 * - 懒加载：按需初始化指标实例
 * - 内存复用：对象池模式复用指标实例
 *
 * @note 该实现专为SQLCC数据库优化，支持高并发监控场景
 * @see docs/design/monitoring/performance_monitor_design.md
 */

#include "performance_monitor.h"
#include <iostream>
#include <limits>

namespace sqlcc {

// Metric implementation
Metric::Metric(const std::string &name, const std::string &description,
               MetricType type, MetricUnit unit)
    : name_(name), description_(description), type_(type), unit_(unit) {}

// CounterMetric implementation
CounterMetric::CounterMetric(const std::string &name, const std::string &description)
    : Metric(name, description, MetricType::COUNTER, MetricUnit::COUNT) {}

std::string CounterMetric::ToString() const {
    return name_ + ": " + std::to_string(value_.load());
}

// GaugeMetric implementation
GaugeMetric::GaugeMetric(const std::string &name, const std::string &description)
    : Metric(name, description, MetricType::GAUGE, MetricUnit::COUNT) {}

std::string GaugeMetric::ToString() const {
    return name_ + ": " + std::to_string(value_.load());
}

// HistogramMetric implementation
HistogramMetric::HistogramMetric(const std::string &name, const std::string &description,
                                 const std::vector<double> &bucket_bounds)
    : Metric(name, description, MetricType::HISTOGRAM, MetricUnit::MICROSECONDS) {
    for (double bound : bucket_bounds) {
        buckets_.emplace_back(bound);
    }
}

void HistogramMetric::Observe(double value) {
    count_.fetch_add(1);
    sum_.fetch_add(value);

    for (auto &bucket : buckets_) {
        if (value <= bucket.upper_bound) {
            bucket.count.fetch_add(1);
        }
    }
}

void HistogramMetric::Reset() {
    count_.store(0);
    sum_.store(0.0);
    for (auto &bucket : buckets_) {
        bucket.count.store(0);
    }
}

std::string HistogramMetric::ToString() const {
    std::string result = name_ + " (count: " + std::to_string(count_.load()) +
                        ", sum: " + std::to_string(sum_.load()) + "):\n";
    for (const auto &bucket : buckets_) {
        result += "  <= " + std::to_string(bucket.upper_bound) + ": " +
                 std::to_string(bucket.count.load()) + "\n";
    }
    return result;
}

// TimerMetric implementation
TimerMetric::TimerMetric(const std::string &name, const std::string &description)
    : Metric(name, description, MetricType::TIMER, MetricUnit::MICROSECONDS) {}

void TimerMetric::Record(std::chrono::microseconds duration) {
    uint64_t us = duration.count();
    count_.fetch_add(1);
    sum_.fetch_add(us);

    uint64_t current_min = min_.load();
    while (us < current_min && !min_.compare_exchange_weak(current_min, us)) {}

    uint64_t current_max = max_.load();
    while (us > current_max && !max_.compare_exchange_weak(current_max, us)) {}
}

std::chrono::microseconds TimerMetric::GetAverage() const {
    uint64_t total = count_.load();
    if (total == 0) return std::chrono::microseconds(0);
    return std::chrono::microseconds(sum_.load() / total);
}

void TimerMetric::Reset() {
    count_.store(0);
    sum_.store(0);
    min_.store(std::numeric_limits<uint64_t>::max());
    max_.store(0);
}

std::string TimerMetric::ToString() const {
    uint64_t cnt = count_.load();
    if (cnt == 0) {
        return name_ + ": no samples";
    }

    uint64_t avg = sum_.load() / cnt;
    return name_ + " (count: " + std::to_string(cnt) +
           ", avg: " + std::to_string(avg) + "us, min: " + std::to_string(min_.load()) +
           "us, max: " + std::to_string(max_.load()) + "us)";
}

// PerformanceMonitor implementation
PerformanceMonitor &PerformanceMonitor::GetInstance() {
    static PerformanceMonitor instance;
    return instance;
}

void PerformanceMonitor::RegisterMetric(std::shared_ptr<Metric> metric) {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    metrics_[metric->GetName()] = metric;
}

std::shared_ptr<Metric> PerformanceMonitor::GetMetric(const std::string &name) const {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    auto it = metrics_.find(name);
    return it != metrics_.end() ? it->second : nullptr;
}

std::vector<std::shared_ptr<Metric>> PerformanceMonitor::GetAllMetrics() const {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    std::vector<std::shared_ptr<Metric>> result;
    for (const auto &pair : metrics_) {
        result.push_back(pair.second);
    }
    return result;
}

void PerformanceMonitor::ResetAllMetrics() {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    for (auto &pair : metrics_) {
        pair.second->Reset();
    }
}

std::string PerformanceMonitor::ExportMetrics(const std::string &format) const {
    auto metrics = GetAllMetrics();
    std::string result;

    if (format == "json") {
        result = "{\n";
        for (size_t i = 0; i < metrics.size(); ++i) {
            result += "  \"" + metrics[i]->GetName() + "\": \"" +
                     metrics[i]->ToString() + "\"";
            if (i < metrics.size() - 1) result += ",";
            result += "\n";
        }
        result += "}\n";
    } else {
        // Plain text format
        for (const auto &metric : metrics) {
            result += metric->ToString() + "\n";
        }
    }

    return result;
}

std::shared_ptr<CounterMetric> PerformanceMonitor::CreateCounter(const std::string &name,
                                                                 const std::string &description) {
    auto metric = std::make_shared<CounterMetric>(name, description);
    RegisterMetric(metric);
    return metric;
}

std::shared_ptr<GaugeMetric> PerformanceMonitor::CreateGauge(const std::string &name,
                                                             const std::string &description) {
    auto metric = std::make_shared<GaugeMetric>(name, description);
    RegisterMetric(metric);
    return metric;
}

std::shared_ptr<HistogramMetric> PerformanceMonitor::CreateHistogram(const std::string &name,
                                                                     const std::string &description,
                                                                     const std::vector<double> &bucket_bounds) {
    auto metric = std::make_shared<HistogramMetric>(name, description, bucket_bounds);
    RegisterMetric(metric);
    return metric;
}

std::shared_ptr<TimerMetric> PerformanceMonitor::CreateTimer(const std::string &name,
                                                             const std::string &description) {
    auto metric = std::make_shared<TimerMetric>(name, description);
    RegisterMetric(metric);
    return metric;
}

std::unique_ptr<PerformanceMonitor::ScopedTimer>
PerformanceMonitor::CreateScopedTimer(std::shared_ptr<TimerMetric> timer) {
    return std::unique_ptr<ScopedTimer>(new ScopedTimer(timer));
}

// ScopedTimer implementation
PerformanceMonitor::ScopedTimer::ScopedTimer(std::shared_ptr<TimerMetric> timer)
    : timer_(timer), start_time_(std::chrono::steady_clock::now()) {}

PerformanceMonitor::ScopedTimer::~ScopedTimer() {
    auto end_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
        end_time - start_time_);
    timer_->Record(duration);
}

// StorageEngineMetrics implementation
StorageEngineMetrics &StorageEngineMetrics::GetInstance() {
    static StorageEngineMetrics instance;
    if (!instance.initialized_) {
        instance.Initialize();
    }
    return instance;
}

void StorageEngineMetrics::Initialize() {
    // BufferPool metrics
    buffer_pool_hits = PerformanceMonitor::GetInstance().CreateCounter(
        "buffer_pool_hits", "Number of buffer pool cache hits");
    buffer_pool_misses = PerformanceMonitor::GetInstance().CreateCounter(
        "buffer_pool_misses", "Number of buffer pool cache misses");
    buffer_pool_evictions = PerformanceMonitor::GetInstance().CreateCounter(
        "buffer_pool_evictions", "Number of buffer pool evictions");
    buffer_pool_size = PerformanceMonitor::GetInstance().CreateGauge(
        "buffer_pool_size", "Current buffer pool size");
    buffer_pool_allocated_pages = PerformanceMonitor::GetInstance().CreateGauge(
        "buffer_pool_allocated_pages", "Number of allocated pages");
    buffer_pool_dirty_pages = PerformanceMonitor::GetInstance().CreateGauge(
        "buffer_pool_dirty_pages", "Number of dirty pages");

    buffer_pool_fetch_time = PerformanceMonitor::GetInstance().CreateTimer(
        "buffer_pool_fetch_time", "Time spent fetching pages from buffer pool");
    buffer_pool_flush_time = PerformanceMonitor::GetInstance().CreateTimer(
        "buffer_pool_flush_time", "Time spent flushing pages to disk");

    // Add more metrics initialization as needed...

    initialized_ = true;
}

} // namespace sqlcc
