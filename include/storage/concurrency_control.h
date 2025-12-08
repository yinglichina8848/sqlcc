#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <shared_mutex>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "disk_manager.h"
#include "page.h"
#include "utils/config_manager.h"

namespace sqlcc {

/**
 * @brief 死锁检测节点信息
 */
struct DeadlockNode {
  int32_t transaction_id;
  std::unordered_set<int32_t> wait_for;   // 等待的事务ID集合
  std::unordered_set<int32_t> held_pages; // 持有的页面ID集合
  std::unordered_set<int32_t> wait_pages; // 等待的页面ID集合

  DeadlockNode(int32_t tid) : transaction_id(tid) {}
};

/**
 * @brief 死锁检测结果
 */
struct DeadlockResult {
  bool has_deadlock;
  std::vector<int32_t> cycle; // 死锁环中的事务ID列表
  int32_t victim_transaction; // 被选为牺牲者的事务ID

  DeadlockResult() : has_deadlock(false), victim_transaction(-1) {}
};

/**
 * @brief 锁请求信息
 */
struct LockRequest {
  int32_t transaction_id;
  int32_t page_id;
  bool is_exclusive; // true: 排他锁, false: 共享锁
  std::chrono::steady_clock::time_point request_time;
  std::chrono::milliseconds timeout;
  std::function<void(bool)> callback; // 回调函数，参数表示是否成功获取锁

  LockRequest(int32_t tid, int32_t pid, bool excl, std::chrono::milliseconds to,
              std::function<void(bool)> cb)
      : transaction_id(tid), page_id(pid), is_exclusive(excl),
        request_time(std::chrono::steady_clock::now()), timeout(to),
        callback(cb) {}
};

/**
 * @brief 页面锁信息
 */
struct PageLockInfo {
  std::shared_mutex page_mutex;                        // 页面级别的读写锁
  std::unordered_set<int32_t> shared_holders;          // 持有共享锁的事务ID
  int32_t exclusive_holder;                            // 持有排他锁的事务ID
  std::queue<std::shared_ptr<LockRequest>> wait_queue; // 等待队列

  PageLockInfo() : exclusive_holder(-1) {}
};

/**
 * @brief 死锁检测器
 */
class DeadlockDetector {
public:
  explicit DeadlockDetector(ConfigManager &config);
  ~DeadlockDetector();

  // 添加锁等待关系
  void AddWaitRelation(int32_t waiter, int32_t holder, int32_t page_id);

  // 移除锁等待关系
  void RemoveWaitRelation(int32_t waiter, int32_t holder, int32_t page_id);

  // 添加事务持有页面信息
  void AddHeldPage(int32_t transaction_id, int32_t page_id, bool is_exclusive);

  // 移除事务持有页面信息
  void RemoveHeldPage(int32_t transaction_id, int32_t page_id);

  // 执行死锁检测
  DeadlockResult DetectDeadlock();

  // 获取检测统计信息
  struct DetectionStats {
    size_t total_detections;
    size_t deadlocks_found;
    std::chrono::milliseconds avg_detection_time;
    size_t max_wait_queue_size;
  };

  DetectionStats GetStats() const;

  // 重置统计信息
  void ResetStats();

private:
  // 深度优先搜索检测死锁环
  bool DFS(int32_t node_id, std::unordered_map<int32_t, int> &visit_state,
           std::vector<int32_t> &path);

  // 选择牺牲者事务
  int32_t SelectVictim(const std::vector<int32_t> &cycle);

  // 清理已结束的事务
  void CleanupFinishedTransactions();

  ConfigManager &config_;
  std::unordered_map<int32_t, std::unique_ptr<DeadlockNode>> wait_graph_;
  mutable std::mutex graph_mutex_;

  // 统计信息
  mutable std::mutex stats_mutex_;
  DetectionStats stats_;

  // 检测配置
  std::chrono::milliseconds detection_interval_;
  size_t max_transaction_age_;
  bool enable_detection_;
};

/**
 * @brief 分层锁管理器
 */
class HierarchicalLockManager {
public:
  explicit HierarchicalLockManager(ConfigManager &config);
  ~HierarchicalLockManager();

  // 请求页面锁
  bool AcquirePageLock(
      int32_t transaction_id, int32_t page_id, bool is_exclusive,
      std::chrono::milliseconds timeout = std::chrono::milliseconds(5000),
      std::function<void(bool)> callback = nullptr);

  // 释放页面锁
  bool ReleasePageLock(int32_t transaction_id, int32_t page_id);

  // 检查事务是否持有页面锁
  bool HoldsPageLock(int32_t transaction_id, int32_t page_id,
                     bool is_exclusive) const;

  // 获取锁等待队列长度
  size_t GetWaitQueueSize(int32_t page_id) const;

  // 获取锁管理器统计信息
  struct LockManagerStats {
    size_t total_pages_locked;
    size_t exclusive_locks;
    size_t shared_locks;
    size_t waiting_requests;
    size_t timeouts;
    size_t deadlocks;
  };

  LockManagerStats GetStats() const;

  // 重置统计信息
  void ResetStats();

  // 设置死锁检测器
  void SetDeadlockDetector(std::shared_ptr<DeadlockDetector> detector);

  // 强制释放事务的所有锁（事务中止时使用）
  void ReleaseAllLocks(int32_t transaction_id);

private:
  // 处理锁等待队列
  void ProcessWaitQueue(int32_t page_id);

  // 检查锁兼容性
  bool IsLockCompatible(const PageLockInfo &lock_info, bool is_exclusive,
                        int32_t transaction_id) const;

  // 添加锁请求到等待队列
  void AddToWaitQueue(int32_t page_id, std::shared_ptr<LockRequest> request);

  // 从等待队列移除请求
  void RemoveFromWaitQueue(int32_t page_id, int32_t transaction_id);

  // 处理锁超时
  void HandleTimeouts();

  ConfigManager &config_;
  std::unordered_map<int32_t, std::unique_ptr<PageLockInfo>> page_locks_;
  mutable std::shared_mutex locks_mutex_;

  // 死锁检测器
  std::shared_ptr<DeadlockDetector> deadlock_detector_;

  // 统计信息
  mutable std::mutex stats_mutex_;
  LockManagerStats stats_;

  // 超时处理线程
  std::thread timeout_thread_;
  std::atomic<bool> shutdown_;
  std::condition_variable shutdown_cv_;
  std::mutex shutdown_mutex_;

  // 配置参数
  std::chrono::milliseconds default_timeout_;
  size_t max_wait_queue_size_;
  bool enable_deadlock_detection_;
};

/**
 * @brief 预取器
 */
class Prefetcher {
public:
  explicit Prefetcher(ConfigManager &config, DiskManager &disk_manager);
  ~Prefetcher();

  // 通知页面访问（用于预测下一访问页面）
  void NotifyPageAccess(int32_t transaction_id, int32_t page_id);

  // 预取页面
  void PrefetchPage(int32_t page_id);

  // 获取预取的页面（如果存在）
  bool GetPrefetchedPage(int32_t page_id, Page &page);

  // 获取预取器统计信息
  struct PrefetcherStats {
    size_t total_prefetches;
    size_t successful_prefetches;
    size_t unused_prefetches;
    double prefetch_accuracy;
    std::chrono::milliseconds avg_prefetch_time;
  };

  PrefetcherStats GetStats() const;

  // 重置统计信息
  void ResetStats();

  // 启用/禁用预取器
  void SetEnabled(bool enabled);

private:
  // 预取工作线程
  void PrefetchWorker();

  // 预测下一访问页面
  std::vector<int32_t> PredictNextPages(int32_t transaction_id,
                                        int32_t current_page);

  // 清理过期的预取页面
  void CleanupExpiredPrefetches();

  ConfigManager &config_;
  DiskManager &disk_manager_;

  // 预取页面缓存
  struct PrefetchEntry {
    Page page;
    std::chrono::steady_clock::time_point prefetch_time;
    std::chrono::milliseconds expiry_time;
    bool used;

    PrefetchEntry(const Page &p, std::chrono::milliseconds expiry)
        : page(p), prefetch_time(std::chrono::steady_clock::now()),
          expiry_time(expiry), used(false) {}
  };

  std::unordered_map<int32_t, std::unique_ptr<PrefetchEntry>> prefetch_cache_;
  mutable std::shared_mutex prefetch_mutex_;

  // 访问历史（用于预测）
  struct AccessHistory {
    std::deque<int32_t> recent_pages; // 最近访问的页面
    std::unordered_map<int32_t, std::unordered_map<int32_t, int>>
        transitions; // 页面转换计数
  };

  std::unordered_map<int32_t, std::unique_ptr<AccessHistory>> access_history_;
  mutable std::mutex history_mutex_;

  // 预取工作线程
  std::thread prefetch_thread_;
  std::queue<int32_t> prefetch_queue_;
  std::mutex queue_mutex_;
  std::condition_variable queue_cv_;
  std::atomic<bool> shutdown_;

  // 统计信息
  mutable std::mutex stats_mutex_;
  PrefetcherStats stats_;

  // 配置参数
  bool enabled_;
  size_t max_prefetch_pages_;
  size_t history_size_;
  std::chrono::milliseconds prefetch_expiry_;
  std::chrono::milliseconds prefetch_interval_;
};

} // namespace sqlcc