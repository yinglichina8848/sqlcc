#include "storage/concurrency_control.h"
#include <algorithm>
#include <random>

namespace sqlcc {

// ==================== DeadlockDetector Implementation ====================

DeadlockDetector::DeadlockDetector(ConfigManager &config) : config_(config) {
  // 初始化配置参数
  detection_interval_ = std::chrono::milliseconds(
      config_.GetInt("buffer.deadlock.detection_interval_ms", 100));
  max_transaction_age_ =
      config_.GetInt("buffer.deadlock.max_transaction_age_ms", 5000);
  enable_detection_ = config_.GetBool("buffer.deadlock.enable_detection", true);

  // 初始化统计信息
  stats_ = DetectionStats{};
}

DeadlockDetector::~DeadlockDetector() {
  // 清理资源
  std::lock_guard<std::mutex> lock(graph_mutex_);
  wait_graph_.clear();
}

void DeadlockDetector::AddWaitRelation(int32_t waiter, int32_t holder,
                                       int32_t page_id) {
  if (!enable_detection_)
    return;

  std::lock_guard<std::mutex> lock(graph_mutex_);

  // 确保等待者和持有者节点存在
  if (wait_graph_.find(waiter) == wait_graph_.end()) {
    wait_graph_[waiter] = std::make_unique<DeadlockNode>(waiter);
  }

  if (wait_graph_.find(holder) == wait_graph_.end()) {
    wait_graph_[holder] = std::make_unique<DeadlockNode>(holder);
  }

  // 添加等待关系
  wait_graph_[waiter]->wait_for.insert(holder);
  wait_graph_[waiter]->wait_pages.insert(page_id);
  wait_graph_[holder]->held_pages.insert(page_id);
}

void DeadlockDetector::RemoveWaitRelation(int32_t waiter, int32_t holder,
                                          int32_t page_id) {
  if (!enable_detection_)
    return;

  std::lock_guard<std::mutex> lock(graph_mutex_);

  auto waiter_it = wait_graph_.find(waiter);
  if (waiter_it != wait_graph_.end()) {
    waiter_it->second->wait_for.erase(holder);
    waiter_it->second->wait_pages.erase(page_id);
  }

  auto holder_it = wait_graph_.find(holder);
  if (holder_it != wait_graph_.end()) {
    holder_it->second->held_pages.erase(page_id);
  }
}

void DeadlockDetector::AddHeldPage(int32_t transaction_id, int32_t page_id,
                                   bool is_exclusive) {
  if (!enable_detection_)
    return;

  std::lock_guard<std::mutex> lock(graph_mutex_);

  // 确保事务节点存在
  if (wait_graph_.find(transaction_id) == wait_graph_.end()) {
    wait_graph_[transaction_id] =
        std::make_unique<DeadlockNode>(transaction_id);
  }

  // 添加持有页面信息
  wait_graph_[transaction_id]->held_pages.insert(page_id);
}

void DeadlockDetector::RemoveHeldPage(int32_t transaction_id, int32_t page_id) {
  if (!enable_detection_)
    return;

  std::lock_guard<std::mutex> lock(graph_mutex_);

  auto it = wait_graph_.find(transaction_id);
  if (it != wait_graph_.end()) {
    it->second->held_pages.erase(page_id);
  }
}

DeadlockResult DeadlockDetector::DetectDeadlock() {
  DeadlockResult result;

  if (!enable_detection_) {
    return result;
  }

  auto start_time = std::chrono::steady_clock::now();

  std::lock_guard<std::mutex> lock(graph_mutex_);

  // 清理已结束的事务
  CleanupFinishedTransactions();

  // 使用DFS检测环
  std::unordered_map<int32_t, int>
      visit_state; // 0: 未访问, 1: 访问中, 2: 已访问
  std::vector<int32_t> path;

  for (const auto &pair : wait_graph_) {
    int32_t node_id = pair.first;
    if (visit_state[node_id] == 0) {
      if (DFS(node_id, visit_state, path)) {
        // 找到死锁环
        result.has_deadlock = true;
        result.cycle = path;
        result.victim_transaction = SelectVictim(path);

        // 更新统计信息
        std::lock_guard<std::mutex> stats_lock(stats_mutex_);
        stats_.deadlocks_found++;

        return result;
      }
    }
  }

  // 更新统计信息
  auto end_time = std::chrono::steady_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
      end_time - start_time);

  std::lock_guard<std::mutex> stats_lock(stats_mutex_);
  stats_.total_detections++;

  // 计算平均检测时间
  if (stats_.total_detections == 1) {
    stats_.avg_detection_time = duration;
  } else {
    auto total_time =
        stats_.avg_detection_time * (stats_.total_detections - 1) + duration;
    stats_.avg_detection_time = total_time / stats_.total_detections;
  }

  return result;
}

bool DeadlockDetector::DFS(int32_t node_id,
                           std::unordered_map<int32_t, int> &visit_state,
                           std::vector<int32_t> &path) {
  visit_state[node_id] = 1; // 标记为访问中
  path.push_back(node_id);

  auto it = wait_graph_.find(node_id);
  if (it != wait_graph_.end()) {
    for (int32_t neighbor : it->second->wait_for) {
      if (visit_state[neighbor] == 0) {
        if (DFS(neighbor, visit_state, path)) {
          return true;
        }
      } else if (visit_state[neighbor] == 1) {
        // 找到环，构建环路径
        std::vector<int32_t> cycle;
        auto cycle_start = std::find(path.begin(), path.end(), neighbor);
        cycle.insert(cycle.end(), cycle_start, path.end());
        cycle.push_back(neighbor); // 添加起始节点以闭合环

        path = cycle;
        return true;
      }
    }
  }

  visit_state[node_id] = 2; // 标记为已访问
  path.pop_back();
  return false;
}

int32_t DeadlockDetector::SelectVictim(const std::vector<int32_t> &cycle) {
  // 选择持有页面最少的事务作为牺牲者
  int32_t victim = cycle[0];
  size_t min_held_pages = wait_graph_[victim]->held_pages.size();

  for (size_t i = 1; i < cycle.size(); ++i) {
    int32_t transaction_id = cycle[i];
    size_t held_pages = wait_graph_[transaction_id]->held_pages.size();

    if (held_pages < min_held_pages) {
      min_held_pages = held_pages;
      victim = transaction_id;
    }
  }

  return victim;
}

void DeadlockDetector::CleanupFinishedTransactions() {
  // 在实际实现中，这里应该检查事务是否已经结束
  // 目前简单实现为清理没有等待关系和持有页面的节点
  auto it = wait_graph_.begin();
  while (it != wait_graph_.end()) {
    if (it->second->wait_for.empty() && it->second->held_pages.empty()) {
      it = wait_graph_.erase(it);
    } else {
      ++it;
    }
  }
}

DeadlockDetector::DetectionStats DeadlockDetector::GetStats() const {
  std::lock_guard<std::mutex> lock(stats_mutex_);
  return stats_;
}

void DeadlockDetector::ResetStats() {
  std::lock_guard<std::mutex> lock(stats_mutex_);
  stats_ = DetectionStats{};
}

// ==================== HierarchicalLockManager Implementation
// ====================

HierarchicalLockManager::HierarchicalLockManager(ConfigManager &config)
    : config_(config), shutdown_(false) {
  // 初始化配置参数
  default_timeout_ = std::chrono::milliseconds(
      config_.GetInt("buffer.lock.default_timeout_ms", 5000));
  max_wait_queue_size_ = config_.GetInt("buffer.lock.max_wait_queue_size", 100);
  enable_deadlock_detection_ =
      config_.GetBool("buffer.lock.enable_deadlock_detection", true);

  // 初始化统计信息
  stats_ = LockManagerStats{};

  // 创建死锁检测器
  if (enable_deadlock_detection_) {
    deadlock_detector_ = std::make_shared<DeadlockDetector>(config_);
  }

  // 启动超时处理线程
  timeout_thread_ = std::thread(&HierarchicalLockManager::HandleTimeouts, this);
}

HierarchicalLockManager::~HierarchicalLockManager() {
  // 通知超时处理线程退出
  {
    std::lock_guard<std::mutex> lock(shutdown_mutex_);
    shutdown_ = true;
    shutdown_cv_.notify_all();
  }

  // 等待超时处理线程结束
  if (timeout_thread_.joinable()) {
    timeout_thread_.join();
  }

  // 清理资源
  std::unique_lock<std::shared_mutex> lock(locks_mutex_);
  page_locks_.clear();
}

bool HierarchicalLockManager::AcquirePageLock(
    int32_t transaction_id, int32_t page_id, bool is_exclusive,
    std::chrono::milliseconds timeout, std::function<void(bool)> callback) {
  // 检查参数有效性
  if (transaction_id < 0 || page_id < 0) {
    return false;
  }

  // 使用默认超时值
  if (timeout == std::chrono::milliseconds(0)) {
    timeout = default_timeout_;
  }

  std::unique_lock<std::shared_mutex> lock(locks_mutex_);

  // 获取或创建页面锁信息
  if (page_locks_.find(page_id) == page_locks_.end()) {
    page_locks_[page_id] = std::make_unique<PageLockInfo>();
  }

  auto &lock_info = page_locks_[page_id];

  // 检查锁兼容性
  if (IsLockCompatible(*lock_info, is_exclusive, transaction_id)) {
    // 可以直接获取锁
    if (is_exclusive) {
      lock_info->exclusive_holder = transaction_id;
    } else {
      lock_info->shared_holders.insert(transaction_id);
    }

    // 更新统计信息
    {
      std::lock_guard<std::mutex> stats_lock(stats_mutex_);
      if (is_exclusive) {
        stats_.exclusive_locks++;
      } else {
        stats_.shared_locks++;
      }

      if (lock_info->shared_holders.size() +
              (lock_info->exclusive_holder != -1 ? 1 : 0) ==
          1) {
        stats_.total_pages_locked++;
      }
    }

    // 通知死锁检测器
    if (deadlock_detector_) {
      deadlock_detector_->AddHeldPage(transaction_id, page_id, is_exclusive);
    }

    if (callback) {
      callback(true);
    }

    return true;
  }

  // 需要等待
  if (lock_info->wait_queue.size() >= max_wait_queue_size_) {
    // 等待队列已满，拒绝请求
    if (callback) {
      callback(false);
    }
    return false;
  }

  // 创建锁请求
  auto request = std::make_shared<LockRequest>(transaction_id, page_id,
                                               is_exclusive, timeout, callback);

  // 添加到等待队列
  lock_info->wait_queue.push(request);

  // 更新统计信息
  {
    std::lock_guard<std::mutex> stats_lock(stats_mutex_);
    stats_.waiting_requests++;
  }

  // 通知死锁检测器
  if (deadlock_detector_) {
    // 确定当前持有锁的事务
    int32_t holder = -1;
    if (is_exclusive) {
      holder = lock_info->exclusive_holder;
    } else if (!lock_info->shared_holders.empty()) {
      holder = *lock_info->shared_holders.begin(); // 任选一个共享锁持有者
    }

    if (holder != -1) {
      deadlock_detector_->AddWaitRelation(transaction_id, holder, page_id);
    }
  }

  // 释放锁，等待回调
  lock.unlock();

  // 异步处理等待队列
  ProcessWaitQueue(page_id);

  return true;
}

bool HierarchicalLockManager::ReleasePageLock(int32_t transaction_id,
                                              int32_t page_id) {
  std::unique_lock<std::shared_mutex> lock(locks_mutex_);

  auto it = page_locks_.find(page_id);
  if (it == page_locks_.end()) {
    return false;
  }

  auto &lock_info = it->second;
  bool released = false;

  // 检查是否持有排他锁
  if (lock_info->exclusive_holder == transaction_id) {
    lock_info->exclusive_holder = -1;
    released = true;
  } else {
    // 检查是否持有共享锁
    auto shared_it = lock_info->shared_holders.find(transaction_id);
    if (shared_it != lock_info->shared_holders.end()) {
      lock_info->shared_holders.erase(shared_it);
      released = true;
    }
  }

  if (released) {
    // 通知死锁检测器
    if (deadlock_detector_) {
      deadlock_detector_->RemoveHeldPage(transaction_id, page_id);
    }

    // 更新统计信息
    {
      std::lock_guard<std::mutex> stats_lock(stats_mutex_);
      if (lock_info->shared_holders.empty() &&
          lock_info->exclusive_holder == -1) {
        stats_.total_pages_locked--;
      }
    }

    // 处理等待队列
    lock.unlock();
    ProcessWaitQueue(page_id);
  }

  return released;
}

bool HierarchicalLockManager::HoldsPageLock(int32_t transaction_id,
                                            int32_t page_id,
                                            bool is_exclusive) const {
  std::shared_lock<std::shared_mutex> lock(locks_mutex_);

  auto it = page_locks_.find(page_id);
  if (it == page_locks_.end()) {
    return false;
  }

  const auto &lock_info = it->second;

  if (is_exclusive) {
    return lock_info->exclusive_holder == transaction_id;
  } else {
    return lock_info->shared_holders.find(transaction_id) !=
           lock_info->shared_holders.end();
  }
}

size_t HierarchicalLockManager::GetWaitQueueSize(int32_t page_id) const {
  std::shared_lock<std::shared_mutex> lock(locks_mutex_);

  auto it = page_locks_.find(page_id);
  if (it == page_locks_.end()) {
    return 0;
  }

  return it->second->wait_queue.size();
}

HierarchicalLockManager::LockManagerStats
HierarchicalLockManager::GetStats() const {
  std::lock_guard<std::mutex> lock(stats_mutex_);
  return stats_;
}

void HierarchicalLockManager::ResetStats() {
  std::lock_guard<std::mutex> lock(stats_mutex_);
  stats_ = LockManagerStats{};
}

void HierarchicalLockManager::SetDeadlockDetector(
    std::shared_ptr<DeadlockDetector> detector) {
  deadlock_detector_ = detector;
}

void HierarchicalLockManager::ReleaseAllLocks(int32_t transaction_id) {
  std::unique_lock<std::shared_mutex> lock(locks_mutex_);

  // 收集需要处理的页面ID
  std::vector<int32_t> pages_to_process;

  for (const auto &pair : page_locks_) {
    int32_t page_id = pair.first;
    const auto &lock_info = pair.second;

    if (lock_info->exclusive_holder == transaction_id ||
        lock_info->shared_holders.find(transaction_id) !=
            lock_info->shared_holders.end()) {
      pages_to_process.push_back(page_id);
    }
  }

  // 释放锁
  for (int32_t page_id : pages_to_process) {
    ReleasePageLock(transaction_id, page_id);
  }
}

void HierarchicalLockManager::ProcessWaitQueue(int32_t page_id) {
  std::unique_lock<std::shared_mutex> lock(locks_mutex_);

  auto it = page_locks_.find(page_id);
  if (it == page_locks_.end()) {
    return;
  }

  auto &lock_info = it->second;
  auto &wait_queue = lock_info->wait_queue;

  // 处理等待队列中的请求
  std::queue<std::shared_ptr<LockRequest>> remaining_queue;

  while (!wait_queue.empty()) {
    auto request = wait_queue.front();
    wait_queue.pop();

    // 检查请求是否已超时
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - request->request_time);

    if (elapsed >= request->timeout) {
      // 请求超时
      {
        std::lock_guard<std::mutex> stats_lock(stats_mutex_);
        stats_.timeouts++;
      }

      if (request->callback) {
        request->callback(false);
      }

      // 通知死锁检测器
      if (deadlock_detector_) {
        // 移除等待关系
        int32_t holder = -1;
        if (request->is_exclusive) {
          holder = lock_info->exclusive_holder;
        } else if (!lock_info->shared_holders.empty()) {
          holder = *lock_info->shared_holders.begin();
        }

        if (holder != -1) {
          deadlock_detector_->RemoveWaitRelation(request->transaction_id,
                                                 holder, page_id);
        }
      }

      continue;
    }

    // 检查锁兼容性
    if (IsLockCompatible(*lock_info, request->is_exclusive,
                         request->transaction_id)) {
      // 可以获取锁
      if (request->is_exclusive) {
        lock_info->exclusive_holder = request->transaction_id;
      } else {
        lock_info->shared_holders.insert(request->transaction_id);
      }

      // 更新统计信息
      {
        std::lock_guard<std::mutex> stats_lock(stats_mutex_);
        if (request->is_exclusive) {
          stats_.exclusive_locks++;
        } else {
          stats_.shared_locks++;
        }

        if (lock_info->shared_holders.size() +
                (lock_info->exclusive_holder != -1 ? 1 : 0) ==
            1) {
          stats_.total_pages_locked++;
        }
      }

      // 通知死锁检测器
      if (deadlock_detector_) {
        deadlock_detector_->AddHeldPage(request->transaction_id, page_id,
                                        request->is_exclusive);

        // 移除等待关系
        int32_t holder = -1;
        if (request->is_exclusive) {
          holder = lock_info->exclusive_holder;
        } else if (!lock_info->shared_holders.empty()) {
          holder = *lock_info->shared_holders.begin();
        }

        if (holder != -1) {
          deadlock_detector_->RemoveWaitRelation(request->transaction_id,
                                                 holder, page_id);
        }
      }

      // 调用回调
      if (request->callback) {
        request->callback(true);
      }
    } else {
      // 仍需等待，放回队列
      remaining_queue.push(request);
    }
  }

  // 更新等待队列
  wait_queue = remaining_queue;

  // 更新统计信息
  {
    std::lock_guard<std::mutex> stats_lock(stats_mutex_);
    stats_.waiting_requests = wait_queue.size();
  }
}

bool HierarchicalLockManager::IsLockCompatible(const PageLockInfo &lock_info,
                                               bool is_exclusive,
                                               int32_t transaction_id) const {
  if (is_exclusive) {
    // 排他锁：不能与任何锁共存
    return lock_info.exclusive_holder == -1 && lock_info.shared_holders.empty();
  } else {
    // 共享锁：可以与其他共享锁共存，但不能与排他锁共存
    return lock_info.exclusive_holder == -1 ||
           lock_info.exclusive_holder == transaction_id;
  }
}

void HierarchicalLockManager::AddToWaitQueue(
    int32_t page_id, std::shared_ptr<LockRequest> request) {
  std::unique_lock<std::shared_mutex> lock(locks_mutex_);

  if (page_locks_.find(page_id) == page_locks_.end()) {
    page_locks_[page_id] = std::make_unique<PageLockInfo>();
  }

  page_locks_[page_id]->wait_queue.push(request);
}

void HierarchicalLockManager::RemoveFromWaitQueue(int32_t page_id,
                                                  int32_t transaction_id) {
  std::unique_lock<std::shared_mutex> lock(locks_mutex_);

  auto it = page_locks_.find(page_id);
  if (it == page_locks_.end()) {
    return;
  }

  auto &wait_queue = it->second->wait_queue;
  std::queue<std::shared_ptr<LockRequest>> new_queue;

  while (!wait_queue.empty()) {
    auto request = wait_queue.front();
    wait_queue.pop();

    if (request->transaction_id != transaction_id) {
      new_queue.push(request);
    }
  }

  wait_queue = new_queue;
}

void HierarchicalLockManager::HandleTimeouts() {
  while (true) {
    // 检查是否需要退出
    {
      std::unique_lock<std::mutex> lock(shutdown_mutex_);
      if (shutdown_) {
        break;
      }
    }

    // 定期检查死锁
    if (deadlock_detector_) {
      auto result = deadlock_detector_->DetectDeadlock();

      if (result.has_deadlock) {
        // 处理死锁
        {
          std::lock_guard<std::mutex> stats_lock(stats_mutex_);
          stats_.deadlocks++;
        }

        // 中止牺牲者事务的所有锁请求
        ReleaseAllLocks(result.victim_transaction);
      }
    }

    // 等待一段时间再检查
    std::unique_lock<std::mutex> lock(shutdown_mutex_);
    shutdown_cv_.wait_for(lock, std::chrono::milliseconds(100),
                          [this] { return shutdown_.load(); });
  }
}

// ==================== Prefetcher Implementation ====================

Prefetcher::Prefetcher(ConfigManager &config, DiskManager &disk_manager)
    : config_(config), disk_manager_(disk_manager), shutdown_(false) {
  // 初始化配置参数
  enabled_ = config_.GetBool("buffer.prefetch.enabled", true);
  max_prefetch_pages_ = config_.GetInt("buffer.prefetch.max_pages", 10);
  history_size_ = config_.GetInt("buffer.prefetch.history_size", 100);
  prefetch_expiry_ = std::chrono::milliseconds(
      config_.GetInt("buffer.prefetch.expiry_ms", 5000));
  prefetch_interval_ = std::chrono::milliseconds(
      config_.GetInt("buffer.prefetch.interval_ms", 100));

  // 初始化统计信息
  stats_ = PrefetcherStats{};

  // 启动预取工作线程
  if (enabled_) {
    prefetch_thread_ = std::thread(&Prefetcher::PrefetchWorker, this);
  }
}

Prefetcher::~Prefetcher() {
  // 通知预取工作线程退出
  {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    shutdown_ = true;
    queue_cv_.notify_all();
  }

  // 等待预取工作线程结束
  if (prefetch_thread_.joinable()) {
    prefetch_thread_.join();
  }

  // 清理资源
  {
    std::unique_lock<std::shared_mutex> lock(prefetch_mutex_);
    prefetch_cache_.clear();
  }

  {
    std::lock_guard<std::mutex> lock(history_mutex_);
    access_history_.clear();
  }
}

void Prefetcher::NotifyPageAccess(int32_t transaction_id, int32_t page_id) {
  if (!enabled_)
    return;

  // 更新访问历史
  {
    std::lock_guard<std::mutex> lock(history_mutex_);

    // 确保事务历史存在
    if (access_history_.find(transaction_id) == access_history_.end()) {
      access_history_[transaction_id] = std::make_unique<AccessHistory>();
    }

    auto &history = access_history_[transaction_id];

    // 更新转换计数
    if (!history->recent_pages.empty()) {
      int32_t prev_page = history->recent_pages.back();
      history->transitions[prev_page][page_id]++;
    }

    // 添加到最近访问页面
    history->recent_pages.push_back(page_id);

    // 限制历史大小
    while (history->recent_pages.size() > history_size_) {
      int32_t old_page = history->recent_pages.front();
      history->recent_pages.pop_front();
    }
  }

  // 预测下一访问页面并预取
  auto next_pages = PredictNextPages(transaction_id, page_id);
  for (int32_t next_page : next_pages) {
    PrefetchPage(next_page);
  }
}

void Prefetcher::PrefetchPage(int32_t page_id) {
  if (!enabled_)
    return;

  // 检查是否已经在缓存中
  {
    std::shared_lock<std::shared_mutex> lock(prefetch_mutex_);
    if (prefetch_cache_.find(page_id) != prefetch_cache_.end()) {
      return;
    }
  }

  // 添加到预取队列
  {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    prefetch_queue_.push(page_id);
    queue_cv_.notify_one();
  }
}

bool Prefetcher::GetPrefetchedPage(int32_t page_id, Page &page) {
  if (!enabled_)
    return false;

  std::unique_lock<std::shared_mutex> lock(prefetch_mutex_);

  auto it = prefetch_cache_.find(page_id);
  if (it == prefetch_cache_.end()) {
    return false;
  }

  // 检查是否已过期
  auto now = std::chrono::steady_clock::now();
  auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
      now - it->second->prefetch_time);

  if (elapsed >= it->second->expiry_time) {
    // 已过期，移除
    prefetch_cache_.erase(it);
    return false;
  }

  // 返回预取的页面
  page = it->second->page;
  it->second->used = true;

  // 移除缓存（页面已被使用）
  prefetch_cache_.erase(it);

  // 更新统计信息
  {
    std::lock_guard<std::mutex> stats_lock(stats_mutex_);
    stats_.successful_prefetches++;
  }

  return true;
}

Prefetcher::PrefetcherStats Prefetcher::GetStats() const {
  std::lock_guard<std::mutex> lock(stats_mutex_);
  return stats_;
}

void Prefetcher::ResetStats() {
  std::lock_guard<std::mutex> lock(stats_mutex_);
  stats_ = PrefetcherStats{};
}

void Prefetcher::SetEnabled(bool enabled) { enabled_ = enabled; }

void Prefetcher::PrefetchWorker() {
  while (true) {
    // 检查是否需要退出
    {
      std::lock_guard<std::mutex> lock(queue_mutex_);
      if (shutdown_) {
        break;
      }

      // 等待预取任务
      if (prefetch_queue_.empty()) {
        std::unique_lock<std::mutex> unique_lock(queue_mutex_);
        queue_cv_.wait(unique_lock, [this] {
          return !prefetch_queue_.empty() || shutdown_;
        });
        if (shutdown_)
          break;
      }
    }

    // 处理预取队列
    std::queue<int32_t> local_queue;
    {
      std::lock_guard<std::mutex> lock(queue_mutex_);
      local_queue = prefetch_queue_;
      while (!prefetch_queue_.empty()) {
        prefetch_queue_.pop();
      }
    }

    // 执行预取
    while (!local_queue.empty()) {
      int32_t page_id = local_queue.front();
      local_queue.pop();

      // 检查缓存大小限制
      {
        std::shared_lock<std::shared_mutex> lock(prefetch_mutex_);
        if (prefetch_cache_.size() >= max_prefetch_pages_) {
          break;
        }
      }

      // 从磁盘读取页面
      auto start_time = std::chrono::steady_clock::now();
      Page page;
      bool success =
          disk_manager_.ReadPage(page_id, reinterpret_cast<char *>(&page));
      auto end_time = std::chrono::steady_clock::now();

      if (success) {
        // 添加到预取缓存
        std::unique_lock<std::shared_mutex> lock(prefetch_mutex_);
        prefetch_cache_[page_id] =
            std::make_unique<PrefetchEntry>(page, prefetch_expiry_);

        // 更新统计信息
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            end_time - start_time);

        std::lock_guard<std::mutex> stats_lock(stats_mutex_);
        stats_.total_prefetches++;

        // 计算平均预取时间
        if (stats_.total_prefetches == 1) {
          stats_.avg_prefetch_time = duration;
        } else {
          auto total_time =
              stats_.avg_prefetch_time * (stats_.total_prefetches - 1) +
              duration;
          stats_.avg_prefetch_time = total_time / stats_.total_prefetches;
        }
      }
    }

    // 清理过期的预取页面
    CleanupExpiredPrefetches();

    // 等待一段时间再处理下一批
    std::this_thread::sleep_for(prefetch_interval_);
  }
}

std::vector<int32_t> Prefetcher::PredictNextPages(int32_t transaction_id,
                                                  int32_t current_page) {
  std::vector<int32_t> predictions;

  std::lock_guard<std::mutex> lock(history_mutex_);

  auto it = access_history_.find(transaction_id);
  if (it == access_history_.end() ||
      it->second->transitions.find(current_page) ==
          it->second->transitions.end()) {
    return predictions;
  }

  // 获取从当前页面的转换计数
  const auto &transitions = it->second->transitions.at(current_page);

  // 按转换次数排序
  std::vector<std::pair<int32_t, int>> sorted_transitions(transitions.begin(),
                                                          transitions.end());

  std::sort(sorted_transitions.begin(), sorted_transitions.end(),
            [](const auto &a, const auto &b) { return a.second > b.second; });

  // 返回前几个预测页面
  size_t max_predictions =
      std::min(static_cast<size_t>(3), sorted_transitions.size());
  for (size_t i = 0; i < max_predictions; ++i) {
    predictions.push_back(sorted_transitions[i].first);
  }

  return predictions;
}

void Prefetcher::CleanupExpiredPrefetches() {
  std::unique_lock<std::shared_mutex> lock(prefetch_mutex_);

  auto now = std::chrono::steady_clock::now();
  auto it = prefetch_cache_.begin();

  while (it != prefetch_cache_.end()) {
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - it->second->prefetch_time);

    if (elapsed >= it->second->expiry_time) {
      // 过期，检查是否被使用过
      if (!it->second->used) {
        std::lock_guard<std::mutex> stats_lock(stats_mutex_);
        stats_.unused_prefetches++;
      }

      it = prefetch_cache_.erase(it);
    } else {
      ++it;
    }
  }

  // 更新预取准确率
  {
    std::lock_guard<std::mutex> stats_lock(stats_mutex_);
    if (stats_.total_prefetches > 0) {
      stats_.prefetch_accuracy =
          static_cast<double>(stats_.successful_prefetches) /
          stats_.total_prefetches;
    }
  }
}

} // namespace sqlcc