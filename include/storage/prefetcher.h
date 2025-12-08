#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <unordered_map>

#include "disk_manager.h"
#include "page.h"
#include "utils/config_manager.h"

namespace sqlcc {

/**
 * @brief 预取请求结构
 */
struct PrefetchRequest {
  int32_t page_id;                                    // 页面ID
  int32_t transaction_id;                             // 事务ID
  std::chrono::steady_clock::time_point request_time; // 请求时间
  int priority;                                       // 优先级

  PrefetchRequest(int32_t pid, int32_t tid, int p = 0)
      : page_id(pid), transaction_id(tid), priority(p),
        request_time(std::chrono::steady_clock::now()) {}

  // 比较操作符，用于优先级队列
  bool operator<(const PrefetchRequest &other) const {
    // 优先级高的排在前面
    if (priority != other.priority) {
      return priority < other.priority;
    }
    // 优先级相同，请求时间早的排在前面
    return request_time > other.request_time;
  }
};

/**
 * @brief 预取器类
 *
 * 负责预测和预加载可能需要的页面，减少磁盘I/O延迟
 */
class Prefetcher {
public:
  /**
   * @brief 预取器统计信息
   */
  struct PrefetcherStats {
    std::atomic<size_t> total_prefetches{0};        // 总预取次数
    std::atomic<size_t> successful_prefetches{0};   // 成功预取次数
    std::atomic<size_t> hit_prefetches{0};          // 预取命中次数
    std::atomic<size_t> wasted_prefetches{0};       // 浪费的预取次数
    std::chrono::microseconds avg_prefetch_time{0}; // 平均预取时间

    // 默认构造函数
    PrefetcherStats() = default;

    // 拷贝构造函数（需要处理原子变量的不可拷贝性）
    PrefetcherStats(const PrefetcherStats &other) {
      total_prefetches.store(other.total_prefetches.load());
      successful_prefetches.store(other.successful_prefetches.load());
      hit_prefetches.store(other.hit_prefetches.load());
      wasted_prefetches.store(other.wasted_prefetches.load());
      avg_prefetch_time = other.avg_prefetch_time;
    }

    // 移动构造函数
    PrefetcherStats(PrefetcherStats &&other) noexcept {
      total_prefetches.store(other.total_prefetches.load());
      successful_prefetches.store(other.successful_prefetches.load());
      hit_prefetches.store(other.hit_prefetches.load());
      wasted_prefetches.store(other.wasted_prefetches.load());
      avg_prefetch_time = std::move(other.avg_prefetch_time);
    }

    // 拷贝赋值操作符
    PrefetcherStats &operator=(const PrefetcherStats &other) {
      if (this != &other) {
        total_prefetches.store(other.total_prefetches.load());
        successful_prefetches.store(other.successful_prefetches.load());
        hit_prefetches.store(other.hit_prefetches.load());
        wasted_prefetches.store(other.wasted_prefetches.load());
        avg_prefetch_time = other.avg_prefetch_time;
      }
      return *this;
    }

    // 移动赋值操作符
    PrefetcherStats &operator=(PrefetcherStats &&other) noexcept {
      if (this != &other) {
        total_prefetches.store(other.total_prefetches.load());
        successful_prefetches.store(other.successful_prefetches.load());
        hit_prefetches.store(other.hit_prefetches.load());
        wasted_prefetches.store(other.wasted_prefetches.load());
        avg_prefetch_time = std::move(other.avg_prefetch_time);
      }
      return *this;
    }

    double hit_rate() const {
      size_t total = successful_prefetches.load();
      return total > 0
                 ? (static_cast<double>(hit_prefetches.load()) * 100.0) / total
                 : 0.0;
    }

    double waste_rate() const {
      size_t total = successful_prefetches.load();
      return total > 0
                 ? (static_cast<double>(wasted_prefetches.load()) * 100.0) /
                       total
                 : 0.0;
    }
  };

  /**
   * @brief 构造函数
   * @param config_manager 配置管理器引用
   * @param disk_manager 磁盘管理器引用
   */
  explicit Prefetcher(ConfigManager &config_manager, DiskManager &disk_manager);

  /**
   * @brief 析构函数
   */
  ~Prefetcher();

  /**
   * @brief 启动预取器
   */
  void Start();

  /**
   * @brief 停止预取器
   */
  void Stop();

  /**
   * @brief 通知页面访问
   * @param transaction_id 事务ID
   * @param page_id 页面ID
   */
  void NotifyPageAccess(int32_t transaction_id, int32_t page_id);

  /**
   * @brief 请求预取页面
   * @param page_id 页面ID
   * @param transaction_id 事务ID
   * @param priority 优先级
   */
  void RequestPrefetch(int32_t page_id, int32_t transaction_id = -1,
                       int priority = 0);

  /**
   * @brief 获取预取的页面
   * @param page_id 页面ID
   * @param page 输出页面对象
   * @return 是否成功获取
   */
  bool GetPrefetchedPage(int32_t page_id, Page &page);

  /**
   * @brief 获取预取器统计信息
   * @return 统计信息
   */
  PrefetcherStats GetStats() const;

  /**
   * @brief 重置统计信息
   */
  void ResetStats();

  /**
   * @brief 设置是否启用
   * @param enabled 是否启用
   */
  void SetEnabled(bool enabled);

  /**
   * @brief 检查是否启用
   * @return 是否启用
   */
  bool IsEnabled() const;

private:
  /**
   * @brief 预取工作线程函数
   */
  void PrefetchWorker();

  /**
   * @brief 分析访问模式并预测下一页面
   * @param transaction_id 事务ID
   * @param current_page_id 当前页面ID
   * @return 预测的下一页面ID列表
   */
  std::vector<int32_t> PredictNextPages(int32_t transaction_id,
                                        int32_t current_page_id);

  /**
   * @brief 更新访问历史
   * @param transaction_id 事务ID
   * @param page_id 页面ID
   */
  void UpdateAccessHistory(int32_t transaction_id, int32_t page_id);

  /**
   * @brief 执行预取
   * @param page_id 页面ID
   * @return 是否成功
   */
  bool DoPrefetch(int32_t page_id);

  /**
   * @brief 清理过期的预取页面
   */
  void CleanupExpiredPages();

  /**
   * @brief 更新配置参数
   */
  void UpdateConfig();

  // 配置和依赖
  ConfigManager &config_manager_;
  DiskManager &disk_manager_;

  // 预取缓存
  std::unordered_map<int32_t, Page> prefetch_cache_;
  mutable std::mutex cache_mutex_;

  // 预取请求队列
  std::priority_queue<PrefetchRequest> prefetch_queue_;
  mutable std::mutex queue_mutex_;
  std::condition_variable queue_cv_;

  // 访问历史
  std::unordered_map<int32_t, std::vector<int32_t>> access_history_;
  mutable std::mutex history_mutex_;

  // 工作线程
  std::thread worker_thread_;
  std::atomic<bool> running_;
  std::atomic<bool> enabled_;

  // 统计信息
  mutable std::mutex stats_mutex_;
  PrefetcherStats stats_;

  // 配置参数
  size_t max_cache_size_;
  std::chrono::milliseconds prefetch_timeout_;
  std::chrono::milliseconds cache_expire_time_;
  int max_prefetch_distance_;
  double prefetch_threshold_;
};

} // namespace sqlcc