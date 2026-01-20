#pragma once

#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace sqlcc {

/**
 * 性能监控指标系统
 * 提供对存储引擎各个组件的性能监控和统计功能
 */

// 性能指标类型
enum class MetricType {
  COUNTER,   // 计数器
  GAUGE,     // 仪表盘（当前值）
  HISTOGRAM, // 直方图（分布统计）
  TIMER      // 计时器（时间统计）
};

// 性能指标单位
enum class MetricUnit {
  COUNT,        // 个数
  BYTES,        // 字节
  MICROSECONDS, // 微秒
  MILLISECONDS, // 毫秒
  SECONDS,      // 秒
  PERCENT,      // 百分比
  NONE          // 无单位
};

// 直方图桶配置
struct HistogramBucket {
  double upper_bound;
  std::atomic<uint64_t> count;

  HistogramBucket(double bound) : upper_bound(bound), count(0) {}

  // 拷贝构造函数（需要处理原子变量的不可拷贝性）
  HistogramBucket(const HistogramBucket &other)
      : upper_bound(other.upper_bound), count(other.count.load()) {}

  // 移动构造函数
  HistogramBucket(HistogramBucket &&other) noexcept
      : upper_bound(other.upper_bound), count(other.count.load()) {}

  // 拷贝赋值操作符
  HistogramBucket &operator=(const HistogramBucket &other) {
    if (this != &other) {
      upper_bound = other.upper_bound;
      count.store(other.count.load());
    }
    return *this;
  }

  // 移动赋值操作符
  HistogramBucket &operator=(HistogramBucket &&other) noexcept {
    if (this != &other) {
      upper_bound = other.upper_bound;
      count.store(other.count.load());
    }
    return *this;
  }
};

// 性能指标基类
class Metric {
public:
  Metric(const std::string &name, const std::string &description,
         MetricType type, MetricUnit unit);
  virtual ~Metric() = default;

  const std::string &GetName() const { return name_; }
  const std::string &GetDescription() const { return description_; }
  MetricType GetType() const { return type_; }
  MetricUnit GetUnit() const { return unit_; }

  virtual void Reset() = 0;
  virtual std::string ToString() const = 0;

protected:
  std::string name_;
  std::string description_;
  MetricType type_;
  MetricUnit unit_;
};

// 计数器指标
class CounterMetric : public Metric {
public:
  CounterMetric(const std::string &name, const std::string &description);

  void Increment() { value_.fetch_add(1); }
  void Increment(uint64_t delta) { value_.fetch_add(delta); }
  uint64_t GetValue() const { return value_.load(); }

  void Reset() override { value_.store(0); }
  std::string ToString() const override;

private:
  std::atomic<uint64_t> value_;
};

// 仪表盘指标
class GaugeMetric : public Metric {
public:
  GaugeMetric(const std::string &name, const std::string &description);

  void Set(uint64_t value) { value_.store(value); }
  uint64_t GetValue() const { return value_.load(); }

  void Reset() override { value_.store(0); }
  std::string ToString() const override;

private:
  std::atomic<uint64_t> value_;
};

// 直方图指标
class HistogramMetric : public Metric {
public:
  HistogramMetric(const std::string &name, const std::string &description,
                  const std::vector<double> &bucket_bounds);

  void Observe(double value);
  uint64_t GetCount() const { return count_.load(); }
  double GetSum() const { return sum_.load(); }

  const std::vector<HistogramBucket> &GetBuckets() const { return buckets_; }

  void Reset() override;
  std::string ToString() const override;

private:
  std::vector<HistogramBucket> buckets_;
  std::atomic<uint64_t> count_;
  std::atomic<double> sum_;
};

// 计时器指标
class TimerMetric : public Metric {
public:
  TimerMetric(const std::string &name, const std::string &description);

  void Record(std::chrono::microseconds duration);
  std::chrono::microseconds GetAverage() const;
  uint64_t GetMin() const { return min_.load(); }
  uint64_t GetMax() const { return max_.load(); }
  uint64_t GetCount() const { return count_.load(); }

  void Reset() override;
  std::string ToString() const override;

private:
  std::atomic<uint64_t> count_;
  std::atomic<uint64_t> sum_; // 微秒
  std::atomic<uint64_t> min_; // 微秒
  std::atomic<uint64_t> max_; // 微秒
};

// 性能监控器
class PerformanceMonitor {
public:
  static PerformanceMonitor &GetInstance();

  // 注册指标
  void RegisterMetric(std::shared_ptr<Metric> metric);

  // 获取指标
  std::shared_ptr<Metric> GetMetric(const std::string &name) const;

  // 获取所有指标
  std::vector<std::shared_ptr<Metric>> GetAllMetrics() const;

  // 重置所有指标
  void ResetAllMetrics();

  // 导出指标数据
  std::string ExportMetrics(const std::string &format = "json") const;

  // 创建便捷方法
  std::shared_ptr<CounterMetric> CreateCounter(const std::string &name,
                                               const std::string &description);
  std::shared_ptr<GaugeMetric> CreateGauge(const std::string &name,
                                           const std::string &description);
  std::shared_ptr<HistogramMetric>
  CreateHistogram(const std::string &name, const std::string &description,
                  const std::vector<double> &bucket_bounds);
  std::shared_ptr<TimerMetric> CreateTimer(const std::string &name,
                                           const std::string &description);

  // RAII计时器辅助类
  class ScopedTimer {
  public:
    ScopedTimer(std::shared_ptr<TimerMetric> timer);
    ~ScopedTimer();

  private:
    std::shared_ptr<TimerMetric> timer_;
    std::chrono::steady_clock::time_point start_time_;
  };

  // 创建作用域计时器
  static std::unique_ptr<ScopedTimer>
  CreateScopedTimer(std::shared_ptr<TimerMetric> timer);

private:
  PerformanceMonitor() = default;
  ~PerformanceMonitor() = default;

  mutable std::mutex metrics_mutex_;
  std::unordered_map<std::string, std::shared_ptr<Metric>> metrics_;
};

// 存储引擎专用指标
class StorageEngineMetrics {
public:
  static StorageEngineMetrics &GetInstance();

  // 初始化指标
  void Initialize();

  // BufferPool指标
  std::shared_ptr<CounterMetric> buffer_pool_hits;
  std::shared_ptr<CounterMetric> buffer_pool_misses;
  std::shared_ptr<CounterMetric> buffer_pool_evictions;
  std::shared_ptr<GaugeMetric> buffer_pool_size;
  std::shared_ptr<GaugeMetric> buffer_pool_allocated_pages;
  std::shared_ptr<GaugeMetric> buffer_pool_dirty_pages;
  std::shared_ptr<TimerMetric> buffer_pool_fetch_time;
  std::shared_ptr<TimerMetric> buffer_pool_flush_time;

  // 替换策略指标
  std::shared_ptr<CounterMetric> replacement_strategy_accesses;
  std::shared_ptr<HistogramMetric> replacement_strategy_access_frequency;

  // 锁管理器指标
  std::shared_ptr<CounterMetric> lock_acquisitions;
  std::shared_ptr<CounterMetric> lock_contentions;
  std::shared_ptr<CounterMetric> lock_timeouts;
  std::shared_ptr<TimerMetric> lock_wait_time;
  std::shared_ptr<GaugeMetric> active_locks;

  // 死锁检测指标
  std::shared_ptr<CounterMetric> deadlock_detections;
  std::shared_ptr<CounterMetric> deadlock_resolutions;
  std::shared_ptr<TimerMetric> deadlock_detection_time;

  // 预取器指标
  std::shared_ptr<CounterMetric> prefetch_requests;
  std::shared_ptr<CounterMetric> prefetch_hits;
  std::shared_ptr<CounterMetric> prefetch_misses;
  std::shared_ptr<GaugeMetric> prefetch_queue_size;

  // 磁盘I/O指标
  std::shared_ptr<CounterMetric> disk_reads;
  std::shared_ptr<CounterMetric> disk_writes;
  std::shared_ptr<TimerMetric> disk_read_time;
  std::shared_ptr<TimerMetric> disk_write_time;
  std::shared_ptr<HistogramMetric> disk_io_size;

  // 事务指标
  std::shared_ptr<CounterMetric> transaction_commits;
  std::shared_ptr<CounterMetric> transaction_rollbacks;
  std::shared_ptr<TimerMetric> transaction_duration;
  std::shared_ptr<GaugeMetric> active_transactions;

  // 查询指标
  std::shared_ptr<CounterMetric> query_executions;
  std::shared_ptr<TimerMetric> query_execution_time;
  std::shared_ptr<HistogramMetric> query_result_size;

private:
  StorageEngineMetrics() = default;
  ~StorageEngineMetrics() = default;

  bool initialized_ = false;
};

// 性能监控宏定义
#define PERF_COUNTER(name, description)                                        \
  sqlcc::PerformanceMonitor::GetInstance().CreateCounter(name, description)

#define PERF_GAUGE(name, description)                                          \
  sqlcc::PerformanceMonitor::GetInstance().CreateGauge(name, description)

#define PERF_HISTOGRAM(name, description, ...)                                 \
  sqlcc::PerformanceMonitor::GetInstance().CreateHistogram(name, description,  \
                                                           {__VA_ARGS__})

#define PERF_TIMER(name, description)                                          \
  sqlcc::PerformanceMonitor::GetInstance().CreateTimer(name, description)

#define PERF_SCOPED_TIMER(timer)                                               \
  auto scoped_timer = sqlcc::PerformanceMonitor::CreateScopedTimer(timer)

} // namespace sqlcc