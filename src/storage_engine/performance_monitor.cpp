#include "storage/performance_monitor.h"
#include <algorithm>
#include <atomic>
#include <cmath>
#include <iomanip>
#include <sstream>

namespace sqlcc {

// Metric基类实现
Metric::Metric(const std::string &name, const std::string &description,
               MetricType type, MetricUnit unit)
    : name_(name), description_(description), type_(type), unit_(unit) {}

// CounterMetric实现
CounterMetric::CounterMetric(const std::string &name,
                             const std::string &description)
    : Metric(name, description, MetricType::COUNTER, MetricUnit::COUNT),
      value_(0) {}

std::string CounterMetric::ToString() const {
  std::ostringstream oss;
  oss << name_ << "=" << value_.load();
  return oss.str();
}

// GaugeMetric实现
GaugeMetric::GaugeMetric(const std::string &name,
                         const std::string &description)
    : Metric(name, description, MetricType::GAUGE, MetricUnit::NONE),
      value_(0) {}

std::string GaugeMetric::ToString() const {
  std::ostringstream oss;
  oss << name_ << "=" << value_.load();
  return oss.str();
}

// HistogramMetric实现
HistogramMetric::HistogramMetric(const std::string &name,
                                 const std::string &description,
                                 const std::vector<double> &bucket_bounds)
    : Metric(name, description, MetricType::HISTOGRAM, MetricUnit::NONE),
      count_(0), sum_(0.0) {

  // 创建桶，确保最后一个桶的上界是+Inf
  for (double bound : bucket_bounds) {
    buckets_.push_back(HistogramBucket(bound));
  }
  // 添加+Inf桶
  buckets_.push_back(HistogramBucket(std::numeric_limits<double>::infinity()));
}

void HistogramMetric::Observe(double value) {
  // 增加计数和总和
  count_.fetch_add(1);

  // 使用compare_exchange_weak来实现原子加法
  double current_sum = sum_.load();
  while (!sum_.compare_exchange_weak(current_sum, current_sum + value)) {
    // 重试直到成功
  }

  // 找到合适的桶
  for (auto &bucket : buckets_) {
    if (value <= bucket.upper_bound) {
      bucket.count.fetch_add(1);
      break;
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
  std::ostringstream oss;
  oss << name_ << "{count=" << count_.load() << ", sum=" << std::fixed
      << std::setprecision(2) << sum_.load() << ", buckets=[";

  bool first = true;
  for (const auto &bucket : buckets_) {
    if (!first) {
      oss << ", ";
    }
    if (std::isinf(bucket.upper_bound)) {
      oss << "+Inf";
    } else {
      oss << std::fixed << std::setprecision(2) << bucket.upper_bound;
    }
    oss << ":" << bucket.count.load();
    first = false;
  }

  oss << "]}";
  return oss.str();
}

// TimerMetric实现
TimerMetric::TimerMetric(const std::string &name,
                         const std::string &description)
    : Metric(name, description, MetricType::TIMER, MetricUnit::MICROSECONDS),
      count_(0), sum_(0), min_(std::numeric_limits<uint64_t>::max()), max_(0) {}

void TimerMetric::Record(std::chrono::microseconds duration) {
  uint64_t micros = duration.count();

  // 更新统计信息
  count_.fetch_add(1);
  sum_.fetch_add(micros);

  // 更新最小值
  uint64_t current_min = min_.load();
  while (micros < current_min &&
         !min_.compare_exchange_weak(current_min, micros)) {
    // 重试直到成功
  }

  // 更新最大值
  uint64_t current_max = max_.load();
  while (micros > current_max &&
         !max_.compare_exchange_weak(current_max, micros)) {
    // 重试直到成功
  }
}

std::chrono::microseconds TimerMetric::GetAverage() const {
  uint64_t count_val = count_.load();
  if (count_val == 0) {
    return std::chrono::microseconds(0);
  }
  return std::chrono::microseconds(sum_.load() / count_val);
}

void TimerMetric::Reset() {
  count_.store(0);
  sum_.store(0);
  min_.store(std::numeric_limits<uint64_t>::max());
  max_.store(0);
}

std::string TimerMetric::ToString() const {
  std::ostringstream oss;
  oss << name_ << "{count=" << count_.load() << ", avg=" << GetAverage().count()
      << "μs"
      << ", min=" << GetMin() << "μs"
      << ", max=" << GetMax() << "μs"
      << ", sum=" << sum_.load() << "μs}";
  return oss.str();
}

// PerformanceMonitor实现
PerformanceMonitor &PerformanceMonitor::GetInstance() {
  static PerformanceMonitor instance;
  return instance;
}

void PerformanceMonitor::RegisterMetric(std::shared_ptr<Metric> metric) {
  std::lock_guard<std::mutex> lock(metrics_mutex_);
  if (metric) {
    metrics_[metric->GetName()] = metric;
  }
}

std::shared_ptr<Metric>
PerformanceMonitor::GetMetric(const std::string &name) const {
  std::lock_guard<std::mutex> lock(metrics_mutex_);
  auto it = metrics_.find(name);
  if (it != metrics_.end()) {
    return it->second;
  }
  return nullptr;
}

std::vector<std::shared_ptr<Metric>> PerformanceMonitor::GetAllMetrics() const {
  std::lock_guard<std::mutex> lock(metrics_mutex_);
  std::vector<std::shared_ptr<Metric>> result;
  result.reserve(metrics_.size());

  for (const auto &pair : metrics_) {
    result.push_back(pair.second);
  }

  return result;
}

void PerformanceMonitor::ResetAllMetrics() {
  std::lock_guard<std::mutex> lock(metrics_mutex_);
  for (auto &pair : metrics_) {
    if (pair.second) {
      pair.second->Reset();
    }
  }
}

std::string PerformanceMonitor::ExportMetrics(const std::string &format) const {
  std::lock_guard<std::mutex> lock(metrics_mutex_);

  if (format == "json") {
    std::ostringstream oss;
    oss << "{\n";

    bool first = true;
    for (const auto &pair : metrics_) {
      if (!first) {
        oss << ",\n";
      }

      const auto &metric = pair.second;
      if (!metric) continue;
      oss << "  \"" << metric->GetName() << "\": {";
      oss << "\"type\":\"";

      switch (metric->GetType()) {
      case MetricType::COUNTER:
        oss << "counter";
        break;
      case MetricType::GAUGE:
        oss << "gauge";
        break;
      case MetricType::HISTOGRAM:
        oss << "histogram";
        break;
      case MetricType::TIMER:
        oss << "timer";
        break;
      }

      oss << "\", \"description\":\"" << metric->GetDescription() << "\"";

      // 根据类型添加特定值
      switch (metric->GetType()) {
      case MetricType::COUNTER: {
        auto counter = std::static_pointer_cast<CounterMetric>(metric);
        oss << ", \"value\":" << counter->GetValue();
        break;
      }
      case MetricType::GAUGE: {
        auto gauge = std::static_pointer_cast<GaugeMetric>(metric);
        oss << ", \"value\":" << gauge->GetValue();
        break;
      }
      case MetricType::HISTOGRAM: {
        auto histogram = std::static_pointer_cast<HistogramMetric>(metric);
        oss << ", \"count\":" << histogram->GetCount();
        oss << ", \"sum\":" << histogram->GetSum();
        oss << ", \"buckets\":[";

        bool first_bucket = true;
        for (const auto &bucket : histogram->GetBuckets()) {
          if (!first_bucket) {
            oss << ", ";
          }
          oss << "{\"upper_bound\":";
          if (std::isinf(bucket.upper_bound)) {
            oss << "\"+Inf\"";
          } else {
            oss << std::fixed << std::setprecision(2) << bucket.upper_bound;
          }
          oss << ", \"count\":" << bucket.count.load() << "}";
          first_bucket = false;
        }

        oss << "]";
        break;
      }
      case MetricType::TIMER: {
        auto timer = std::static_pointer_cast<TimerMetric>(metric);
        oss << ", \"count\":" << timer->GetCount();
        oss << ", \"avg\":" << timer->GetAverage().count();
        oss << ", \"min\":" << timer->GetMin();
        oss << ", \"max\":" << timer->GetMax();
        oss << ", \"sum\":" << timer->GetCount() * timer->GetAverage().count();
        break;
      }
      }

      oss << "}";
      first = false;
    }

    oss << "\n}";
    return oss.str();
  } else if (format == "prometheus") {
    // Prometheus格式导出
    std::ostringstream oss;

    for (const auto &pair : metrics_) {
      const auto &metric = pair.second;

      // 添加HELP和TYPE行
      oss << "# HELP " << metric->GetName() << " " << metric->GetDescription()
          << "\n";
      oss << "# TYPE " << metric->GetName() << " ";

      switch (metric->GetType()) {
      case MetricType::COUNTER:
        oss << "counter";
        break;
      case MetricType::GAUGE:
        oss << "gauge";
        break;
      case MetricType::HISTOGRAM:
        oss << "histogram";
        break;
      case MetricType::TIMER:
        oss << "summary";
        break;
      }

      oss << "\n";

      // 添加指标值
      switch (metric->GetType()) {
      case MetricType::COUNTER: {
        auto counter = std::static_pointer_cast<CounterMetric>(metric);
        oss << metric->GetName() << " " << counter->GetValue() << "\n";
        break;
      }
      case MetricType::GAUGE: {
        auto gauge = std::static_pointer_cast<GaugeMetric>(metric);
        oss << metric->GetName() << " " << gauge->GetValue() << "\n";
        break;
      }
      case MetricType::HISTOGRAM: {
        auto histogram = std::static_pointer_cast<HistogramMetric>(metric);
        oss << metric->GetName() << "_count " << histogram->GetCount() << "\n";
        oss << metric->GetName() << "_sum " << histogram->GetSum() << "\n";

        for (const auto &bucket : histogram->GetBuckets()) {
          oss << metric->GetName() << "_bucket{le=\"";
          if (std::isinf(bucket.upper_bound)) {
            oss << "+Inf";
          } else {
            oss << std::fixed << std::setprecision(2) << bucket.upper_bound;
          }
          oss << "\"} " << bucket.count.load() << "\n";
        }
        break;
      }
      case MetricType::TIMER: {
        auto timer = std::static_pointer_cast<TimerMetric>(metric);
        oss << metric->GetName() << "_count " << timer->GetCount() << "\n";
        oss << metric->GetName() << "_sum "
            << timer->GetCount() * timer->GetAverage().count() << "\n";
        oss << metric->GetName() << "{quantile=\"0.5\"} "
            << timer->GetAverage().count() << "\n";
        oss << metric->GetName() << "{quantile=\"0.9\"} "
            << timer->GetAverage().count() * 0.9 << "\n";
        oss << metric->GetName() << "{quantile=\"0.99\"} " << timer->GetMax()
            << "\n";
        break;
      }
      }
    }

    return oss.str();
  } else {
    // 默认文本格式
    std::ostringstream oss;

    for (const auto &pair : metrics_) {
      oss << pair.second->ToString() << "\n";
    }

    return oss.str();
  }
}

std::shared_ptr<CounterMetric>
PerformanceMonitor::CreateCounter(const std::string &name,
                                  const std::string &description) {
  auto metric = std::make_shared<CounterMetric>(name, description);
  RegisterMetric(metric);
  return metric;
}

std::shared_ptr<GaugeMetric>
PerformanceMonitor::CreateGauge(const std::string &name,
                                const std::string &description) {
  auto metric = std::make_shared<GaugeMetric>(name, description);
  RegisterMetric(metric);
  return metric;
}

std::shared_ptr<HistogramMetric>
PerformanceMonitor::CreateHistogram(const std::string &name,
                                    const std::string &description,
                                    const std::vector<double> &bucket_bounds) {
  auto metric =
      std::make_shared<HistogramMetric>(name, description, bucket_bounds);
  RegisterMetric(metric);
  return metric;
}

std::shared_ptr<TimerMetric>
PerformanceMonitor::CreateTimer(const std::string &name,
                                const std::string &description) {
  auto metric = std::make_shared<TimerMetric>(name, description);
  RegisterMetric(metric);
  return metric;
}

// ScopedTimer实现
PerformanceMonitor::ScopedTimer::ScopedTimer(std::shared_ptr<TimerMetric> timer)
    : timer_(timer), start_time_(std::chrono::steady_clock::now()) {}

PerformanceMonitor::ScopedTimer::~ScopedTimer() {
  auto end_time = std::chrono::steady_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
      end_time - start_time_);
  timer_->Record(duration);
}

std::unique_ptr<PerformanceMonitor::ScopedTimer>
PerformanceMonitor::CreateScopedTimer(std::shared_ptr<TimerMetric> timer) {
  return std::make_unique<ScopedTimer>(timer);
}

// StorageEngineMetrics实现
StorageEngineMetrics &StorageEngineMetrics::GetInstance() {
  static StorageEngineMetrics instance;
  return instance;
}

void StorageEngineMetrics::Initialize() {
  if (initialized_) {
    return;
  }

  // BufferPool指标
  buffer_pool_hits =
      PERF_COUNTER("buffer_pool_hits", "Number of buffer pool cache hits");
  buffer_pool_misses =
      PERF_COUNTER("buffer_pool_misses", "Number of buffer pool cache misses");
  buffer_pool_evictions = PERF_COUNTER(
      "buffer_pool_evictions", "Number of pages evicted from buffer pool");
  buffer_pool_size =
      PERF_GAUGE("buffer_pool_size", "Current size of buffer pool");
  buffer_pool_allocated_pages =
      PERF_GAUGE("buffer_pool_allocated_pages",
                 "Number of allocated pages in buffer pool");
  buffer_pool_dirty_pages = PERF_GAUGE("buffer_pool_dirty_pages",
                                       "Number of dirty pages in buffer pool");
  buffer_pool_fetch_time = PERF_TIMER("buffer_pool_fetch_time",
                                      "Time to fetch a page from buffer pool");
  buffer_pool_flush_time =
      PERF_TIMER("buffer_pool_flush_time", "Time to flush a page to disk");

  // 替换策略指标
  replacement_strategy_accesses =
      PERF_COUNTER("replacement_strategy_accesses",
                   "Number of accesses to replacement strategy");
  replacement_strategy_access_frequency =
      PERF_HISTOGRAM("replacement_strategy_access_frequency",
                     "Distribution of page access frequencies",
                     {1, 5, 10, 50, 100, 500, 1000});

  // 锁管理器指标
  lock_acquisitions =
      PERF_COUNTER("lock_acquisitions", "Number of lock acquisitions");
  lock_contentions =
      PERF_COUNTER("lock_contentions", "Number of lock contentions");
  lock_timeouts = PERF_COUNTER("lock_timeouts", "Number of lock timeouts");
  lock_wait_time = PERF_TIMER("lock_wait_time", "Time waiting for locks");
  active_locks = PERF_GAUGE("active_locks", "Number of currently active locks");

  // 死锁检测指标
  deadlock_detections =
      PERF_COUNTER("deadlock_detections", "Number of deadlock detections");
  deadlock_resolutions =
      PERF_COUNTER("deadlock_resolutions", "Number of deadlock resolutions");
  deadlock_detection_time = PERF_TIMER("deadlock_detection_time",
                                       "Time to detect and resolve deadlocks");

  // 预取器指标
  prefetch_requests =
      PERF_COUNTER("prefetch_requests", "Number of prefetch requests");
  prefetch_hits =
      PERF_COUNTER("prefetch_hits", "Number of successful prefetches");
  prefetch_misses =
      PERF_COUNTER("prefetch_misses", "Number of unsuccessful prefetches");
  prefetch_queue_size =
      PERF_GAUGE("prefetch_queue_size", "Current size of prefetch queue");

  // 磁盘I/O指标
  disk_reads = PERF_COUNTER("disk_reads", "Number of disk reads");
  disk_writes = PERF_COUNTER("disk_writes", "Number of disk writes");
  disk_read_time = PERF_TIMER("disk_read_time", "Time spent reading from disk");
  disk_write_time = PERF_TIMER("disk_write_time", "Time spent writing to disk");
  disk_io_size =
      PERF_HISTOGRAM("disk_io_size", "Size of disk I/O operations in bytes",
                     {1024, 4096, 16384, 65536, 262144, 1048576, 4194304});

  // 事务指标
  transaction_commits =
      PERF_COUNTER("transaction_commits", "Number of transaction commits");
  transaction_rollbacks =
      PERF_COUNTER("transaction_rollbacks", "Number of transaction rollbacks");
  transaction_duration =
      PERF_TIMER("transaction_duration", "Duration of transactions");
  active_transactions = PERF_GAUGE("active_transactions",
                                   "Number of currently active transactions");

  // 查询指标
  query_executions =
      PERF_COUNTER("query_executions", "Number of query executions");
  query_execution_time =
      PERF_TIMER("query_execution_time", "Time to execute queries");
  query_result_size =
      PERF_HISTOGRAM("query_result_size", "Size of query results in rows",
                     {1, 10, 100, 1000, 10000, 100000, 1000000});

  initialized_ = true;
}

} // namespace sqlcc
