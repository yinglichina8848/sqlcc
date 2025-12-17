/**
 * @file advanced_lock_manager.cpp
 * @brief 高级锁管理器实现 - 改进的页面锁定机制
 *
 * 该文件实现了高级锁管理器的核心功能，包括：
 * - 多种锁模式的并发控制
 * - 死锁检测和预防
 * - 锁升级和降级机制
 * - 异步锁操作
 * - 意向锁支持
 * - 锁超时管理
 */

#include "storage/advanced_lock_manager.h"
#include "exception.h"
#include "utils/logger.h"
#include <algorithm>
#include <chrono>
#include <thread>
#include <random>

namespace sqlcc {

// 锁兼容性矩阵实现
LockCompatibilityMatrix::LockCompatibilityMatrix() {
    // 初始化锁兼容性矩阵
    // 矩阵格式：[existing][requested]
    // 锁模式：SHARED, EXCLUSIVE, INTENTION_SHARED, INTENTION_EXCLUSIVE, SHARED_INTENTION_EXCLUSIVE

    // 初始化为false
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 5; ++j) {
            compatibility_matrix_[i][j] = false;
        }
    }

    // 设置兼容性规则
    // SHARED (S) 兼容性
    compatibility_matrix_[SHARED][SHARED] = true;                          // S + S
    compatibility_matrix_[SHARED][INTENTION_SHARED] = true;               // S + IS
    compatibility_matrix_[SHARED][INTENTION_EXCLUSIVE] = false;           // S + IX (不兼容)
    compatibility_matrix_[SHARED][SHARED_INTENTION_EXCLUSIVE] = false;    // S + SIX (不兼容)
    compatibility_matrix_[SHARED][EXCLUSIVE] = false;                     // S + X (不兼容)

    // EXCLUSIVE (X) 兼容性 - X锁与任何锁都不兼容
    compatibility_matrix_[EXCLUSIVE][SHARED] = false;
    compatibility_matrix_[EXCLUSIVE][EXCLUSIVE] = false;
    compatibility_matrix_[EXCLUSIVE][INTENTION_SHARED] = false;
    compatibility_matrix_[EXCLUSIVE][INTENTION_EXCLUSIVE] = false;
    compatibility_matrix_[EXCLUSIVE][SHARED_INTENTION_EXCLUSIVE] = false;

    // INTENTION_SHARED (IS) 兼容性
    compatibility_matrix_[INTENTION_SHARED][SHARED] = true;               // IS + S
    compatibility_matrix_[INTENTION_SHARED][EXCLUSIVE] = false;           // IS + X
    compatibility_matrix_[INTENTION_SHARED][INTENTION_SHARED] = true;     // IS + IS
    compatibility_matrix_[INTENTION_SHARED][INTENTION_EXCLUSIVE] = true;  // IS + IX
    compatibility_matrix_[INTENTION_SHARED][SHARED_INTENTION_EXCLUSIVE] = true; // IS + SIX

    // INTENTION_EXCLUSIVE (IX) 兼容性
    compatibility_matrix_[INTENTION_EXCLUSIVE][SHARED] = false;           // IX + S (不兼容)
    compatibility_matrix_[INTENTION_EXCLUSIVE][EXCLUSIVE] = false;        // IX + X (不兼容)
    compatibility_matrix_[INTENTION_EXCLUSIVE][INTENTION_SHARED] = true;  // IX + IS
    compatibility_matrix_[INTENTION_EXCLUSIVE][INTENTION_EXCLUSIVE] = true; // IX + IX
    compatibility_matrix_[INTENTION_EXCLUSIVE][SHARED_INTENTION_EXCLUSIVE] = false; // IX + SIX (不兼容)

    // SHARED_INTENTION_EXCLUSIVE (SIX) 兼容性
    compatibility_matrix_[SHARED_INTENTION_EXCLUSIVE][SHARED] = false;    // SIX + S (不兼容)
    compatibility_matrix_[SHARED_INTENTION_EXCLUSIVE][EXCLUSIVE] = false; // SIX + X (不兼容)
    compatibility_matrix_[SHARED_INTENTION_EXCLUSIVE][INTENTION_SHARED] = true;  // SIX + IS
    compatibility_matrix_[SHARED_INTENTION_EXCLUSIVE][INTENTION_EXCLUSIVE] = false; // SIX + IX (不兼容)
    compatibility_matrix_[SHARED_INTENTION_EXCLUSIVE][SHARED_INTENTION_EXCLUSIVE] = false; // SIX + SIX (不兼容)

    // 对称矩阵 - 如果A兼容B，那么B也兼容A
    for (int i = 0; i < 5; ++i) {
        for (int j = i + 1; j < 5; ++j) {
            if (compatibility_matrix_[i][j]) {
                compatibility_matrix_[j][i] = true;
            }
        }
    }
}

bool LockCompatibilityMatrix::IsCompatible(LockMode existing, LockMode requested) const {
    return compatibility_matrix_[existing][requested];
}

bool LockCompatibilityMatrix::CanGrantImmediately(const PageLockState& state, LockMode requested) const {
    // 检查请求的锁是否与所有现有锁兼容
    for (const auto& holder : state.holders) {
        if (!IsCompatible(holder.mode, requested)) {
            return false;
        }
    }
    return true;
}

// 死锁检测器实现
DeadlockDetector::DeadlockDetector() = default;

bool DeadlockDetector::DetectDeadlock(const std::unordered_map<int32_t, PageLockState>& lock_table,
                                     int32_t transaction_id, std::vector<int32_t>& deadlock_chain) {
    std::unique_lock<std::shared_mutex> lock(graph_mutex_);

    // 构建等待图的快照
    std::unordered_map<int32_t, std::unordered_set<int32_t>> current_graph = wait_graph_;

    // 从当前事务开始深度优先搜索
    std::unordered_set<int32_t> visited;
    std::unordered_set<int32_t> recursion_stack;
    std::vector<int32_t> path;

    return HasCycleDFS(transaction_id, visited, recursion_stack, path);
}

void DeadlockDetector::AddWaitFor(int32_t waiter, int32_t holder) {
    std::unique_lock<std::shared_mutex> lock(graph_mutex_);
    wait_graph_[waiter].insert(holder);
}

void DeadlockDetector::RemoveWaitFor(int32_t waiter) {
    std::unique_lock<std::shared_mutex> lock(graph_mutex_);
    wait_graph_.erase(waiter);
}

const std::unordered_map<int32_t, std::unordered_set<int32_t>>& DeadlockDetector::GetWaitGraph() const {
    return wait_graph_;
}

bool DeadlockDetector::HasCycleDFS(int32_t current, std::unordered_set<int32_t>& visited,
                                  std::unordered_set<int32_t>& recursion_stack,
                                  std::vector<int32_t>& path) const {
    visited.insert(current);
    recursion_stack.insert(current);
    path.push_back(current);

    auto it = wait_graph_.find(current);
    if (it != wait_graph_.end()) {
        for (int32_t neighbor : it->second) {
            if (recursion_stack.find(neighbor) != recursion_stack.end()) {
                // 发现环
                path.push_back(neighbor);
                return true;
            }

            if (visited.find(neighbor) == visited.end()) {
                if (HasCycleDFS(neighbor, visited, recursion_stack, path)) {
                    return true;
                }
            }
        }
    }

    recursion_stack.erase(current);
    path.pop_back();
    return false;
}

// 辅助函数：更新锁计数器
inline void UpdateLockCounts(PageLockState& state, LockMode mode, int delta) {
    switch (mode) {
        case SHARED:
            state.shared_count += delta;
            break;
        case EXCLUSIVE:
            state.exclusive_count += delta;
            break;
        case INTENTION_SHARED:
            state.intention_shared_count += delta;
            break;
        case INTENTION_EXCLUSIVE:
            state.intention_exclusive_count += delta;
            break;
        case SHARED_INTENTION_EXCLUSIVE:
            state.shared_intention_exclusive_count += delta;
            break;
    }
}

// 锁升级/降级管理器实现
LockUpgradeManager::LockUpgradeManager() {
    // 初始化锁升级兼容性矩阵
    // 矩阵格式：[current][requested]
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 5; ++j) {
            upgrade_matrix_[i][j] = false;
        }
    }

    // 设置升级规则
    // SHARED 可以升级到其他锁
    upgrade_matrix_[SHARED][EXCLUSIVE] = true;
    upgrade_matrix_[SHARED][INTENTION_EXCLUSIVE] = true;
    upgrade_matrix_[SHARED][SHARED_INTENTION_EXCLUSIVE] = true;

    // INTENTION_SHARED 可以升级到更强的意向锁
    upgrade_matrix_[INTENTION_SHARED][INTENTION_EXCLUSIVE] = true;
    upgrade_matrix_[INTENTION_SHARED][SHARED_INTENTION_EXCLUSIVE] = true;
    upgrade_matrix_[INTENTION_SHARED][EXCLUSIVE] = true;

    // INTENTION_EXCLUSIVE 可以升级到排他锁
    upgrade_matrix_[INTENTION_EXCLUSIVE][EXCLUSIVE] = true;

    // SHARED_INTENTION_EXCLUSIVE 可以升级到排他锁
    upgrade_matrix_[SHARED_INTENTION_EXCLUSIVE][EXCLUSIVE] = true;

    // EXCLUSIVE 不能升级（已经是最高级别）
    upgrade_matrix_[EXCLUSIVE][EXCLUSIVE] = false;
}

LockUpgradeManager::UpgradeStrategy LockUpgradeManager::CanUpgrade(LockMode current, LockMode requested) const {
    if (upgrade_matrix_[current][requested]) {
        // 检查是否可以立即升级
        if (current == SHARED && (requested == INTENTION_EXCLUSIVE || requested == SHARED_INTENTION_EXCLUSIVE)) {
            return IMMEDIATE_UPGRADE;
        } else {
            return DEFERRED_UPGRADE; // 需要等待其他锁释放
        }
    }
    return NO_UPGRADE;
}

bool LockUpgradeManager::PerformUpgrade(PageLockState& state, int32_t transaction_id, LockMode new_mode) {
    // 查找事务当前的锁
    auto it = std::find_if(state.holders.begin(), state.holders.end(),
                          [transaction_id](const LockHolder& holder) {
                              return holder.transaction_id == transaction_id;
                          });

    if (it == state.holders.end()) {
        return false; // 事务没有持有锁
    }

    // 检查升级是否允许
    if (CanUpgrade(it->mode, new_mode) == NO_UPGRADE) {
        return false;
    }

    // 执行升级
    LockMode old_mode = it->mode;
    it->mode = new_mode;
    it->acquire_time = std::chrono::steady_clock::now();

    // 更新计数器
    UpdateLockCounts(state, old_mode, -1);
    UpdateLockCounts(state, new_mode, 1);

    // 更新事务锁映射
    state.transaction_locks[transaction_id] = new_mode;

    return true;
}

bool LockUpgradeManager::CanDowngrade(LockMode current, LockMode requested) const {
    // 降级规则：任何锁都可以降级到更弱的锁
    if (current == EXCLUSIVE) {
        return requested == SHARED || requested == INTENTION_SHARED ||
               requested == INTENTION_EXCLUSIVE || requested == SHARED_INTENTION_EXCLUSIVE;
    } else if (current == SHARED_INTENTION_EXCLUSIVE) {
        return requested == SHARED || requested == INTENTION_SHARED || requested == INTENTION_EXCLUSIVE;
    } else if (current == INTENTION_EXCLUSIVE) {
        return requested == SHARED || requested == INTENTION_SHARED;
    } else if (current == SHARED) {
        return requested == INTENTION_SHARED;
    }

    return false;
}

bool LockUpgradeManager::PerformDowngrade(PageLockState& state, int32_t transaction_id, LockMode new_mode) {
    // 查找事务当前的锁
    auto it = std::find_if(state.holders.begin(), state.holders.end(),
                          [transaction_id](const LockHolder& holder) {
                              return holder.transaction_id == transaction_id;
                          });

    if (it == state.holders.end()) {
        return false; // 事务没有持有锁
    }

    // 检查降级是否允许
    if (!CanDowngrade(it->mode, new_mode)) {
        return false;
    }

    // 执行降级
    LockMode old_mode = it->mode;
    it->mode = new_mode;
    it->acquire_time = std::chrono::steady_clock::now();

    // 更新计数器
    UpdateLockCounts(state, old_mode, -1);
    UpdateLockCounts(state, new_mode, 1);

    // 更新事务锁映射
    state.transaction_locks[transaction_id] = new_mode;

    return true;
}

// 高级锁管理器实现
AdvancedLockManager::AdvancedLockManager(size_t max_locks, std::chrono::milliseconds default_timeout)
    : max_locks_(max_locks),
      default_timeout_(default_timeout),
      deadlock_detection_enabled_(true),
      deadlock_check_interval_(std::chrono::milliseconds(100)),
      async_worker_running_(false) {

    // 设置默认死锁处理策略（选择最年轻的事务作为受害者）
    deadlock_resolution_strategy_ = [](const std::vector<int32_t>& chain) -> int32_t {
        return chain.back(); // 返回链中的最后一个事务（通常是最新的）
    };

    StartAsyncWorker();

    SQLCC_LOG_INFO("AdvancedLockManager initialized with max_locks=" +
                   std::to_string(max_locks_) + ", timeout=" +
                   std::to_string(default_timeout_.count()) + "ms");
}

AdvancedLockManager::~AdvancedLockManager() {
    StopAsyncWorker();
}

LockRequestStatus AdvancedLockManager::AcquireLock(int32_t page_id, LockMode mode,
                                                  int32_t transaction_id,
                                                  std::chrono::milliseconds timeout) {
    auto start_time = std::chrono::steady_clock::now();

    // 检查锁表大小限制
    {
        std::shared_lock<std::shared_mutex> lock(lock_table_mutex_);
        if (lock_table_.size() >= max_locks_) {
            std::lock_guard<std::mutex> stats_lock(stats_mutex_);
            stats_.lock_timeouts++;
            return TIMED_OUT;
        }
    }

    // 尝试获取锁
    LockRequestStatus result = TryAcquireLock(page_id, mode, transaction_id);

    if (result == WAITING) {
        // 需要等待，添加到等待队列
        PageLockState& state = GetOrCreatePageLockState(page_id);
        LockRequest request(transaction_id, mode, page_id, timeout);
        state.waiting_queue.push(request);

        // 等待锁或超时
        std::unique_lock<std::mutex> wait_lock(async_mutex_);
        auto wait_result = async_cv_.wait_for(wait_lock, timeout);

        if (wait_result == std::cv_status::timeout) {
            // 超时，从等待队列中移除
            {
                std::unique_lock<std::shared_mutex> lock(lock_table_mutex_);
                std::queue<LockRequest> new_queue;
                while (!state.waiting_queue.empty()) {
                    LockRequest req = state.waiting_queue.front();
                    state.waiting_queue.pop();
                    if (req.transaction_id != transaction_id) {
                        new_queue.push(req);
                    }
                }
                state.waiting_queue = std::move(new_queue);
            }

            std::lock_guard<std::mutex> stats_lock(stats_mutex_);
            stats_.lock_timeouts++;
            return TIMED_OUT;
        }

        // 检查最终结果
        result = request.status;
    }

    // 更新等待时间统计
    auto wait_time = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start_time);
    UpdateLockWaitTime(wait_time);

    return result;
}

LockRequestStatus AdvancedLockManager::ReleaseLock(int32_t page_id, int32_t transaction_id) {
    std::unique_lock<std::shared_mutex> lock(lock_table_mutex_);

    auto it = lock_table_.find(page_id);
    if (it == lock_table_.end()) {
        return ABORTED; // 页面没有锁
    }

    PageLockState& state = it->second;

    // 查找并移除锁
    auto holder_it = std::find_if(state.holders.begin(), state.holders.end(),
                                 [transaction_id](const LockHolder& holder) {
                                     return holder.transaction_id == transaction_id;
                                 });

    if (holder_it == state.holders.end()) {
        return ABORTED; // 事务没有持有锁
    }

    LockMode released_mode = holder_it->mode;
    state.holders.erase(holder_it);

    // 更新计数器
    UpdateLockCounts(state, released_mode, -1);
    state.transaction_locks.erase(transaction_id);

    // 从死锁检测器中移除等待关系
    deadlock_detector_.RemoveWaitFor(transaction_id);

    // 检查是否有等待的请求可以被授予
    GrantWaitingRequests(page_id);

    std::lock_guard<std::mutex> stats_lock(stats_mutex_);
    stats_.total_locks--;

    return GRANTED;
}

std::vector<LockRequestStatus> AdvancedLockManager::AcquireLocks(
    const std::vector<std::pair<int32_t, LockMode>>& requests,
    int32_t transaction_id, std::chrono::milliseconds timeout) {

    std::vector<LockRequestStatus> results;

    // 简单的两阶段锁定：先检查所有锁是否可以获取，然后再获取
    // 在实际实现中，这可能需要更复杂的算法来避免死锁

    for (const auto& request : requests) {
        LockRequestStatus status = AcquireLock(request.first, request.second,
                                              transaction_id, timeout);
        results.push_back(status);

        // 如果任何一个锁获取失败，回滚之前获取的锁
        if (status != GRANTED) {
            // 回滚已获取的锁
            for (size_t i = 0; i < results.size() - 1; ++i) {
                if (results[i] == GRANTED) {
                    ReleaseLock(requests[i].first, transaction_id);
                }
            }
            break;
        }
    }

    return results;
}

size_t AdvancedLockManager::ReleaseAllLocks(int32_t transaction_id) {
    std::unique_lock<std::shared_mutex> lock(lock_table_mutex_);

    size_t released_count = 0;

    // 遍历所有页面，释放事务持有的锁
    for (auto& pair : lock_table_) {
        PageLockState& state = pair.second;

        auto holder_it = std::find_if(state.holders.begin(), state.holders.end(),
                                     [transaction_id](const LockHolder& holder) {
                                         return holder.transaction_id == transaction_id;
                                     });

        if (holder_it != state.holders.end()) {
            LockMode released_mode = holder_it->mode;
            state.holders.erase(holder_it);
            UpdateLockCounts(state, released_mode, -1);
            state.transaction_locks.erase(transaction_id);
            released_count++;

            // 检查等待队列
            GrantWaitingRequests(pair.first);
        }
    }

    // 从死锁检测器中移除等待关系
    deadlock_detector_.RemoveWaitFor(transaction_id);

    std::lock_guard<std::mutex> stats_lock(stats_mutex_);
    stats_.total_locks -= released_count;

    return released_count;
}

bool AdvancedLockManager::AcquireLockAsync(int32_t page_id, LockMode mode, int32_t transaction_id,
                                          std::function<void(LockRequestStatus)> callback,
                                          std::chrono::milliseconds timeout) {
    std::lock_guard<std::mutex> lock(async_mutex_);

    if (!async_worker_running_.load()) {
        return false;
    }

    LockRequest request(transaction_id, mode, page_id, timeout);
    request.callback = std::move(callback);
    async_requests_.push(request);
    async_cv_.notify_one();

    return true;
}

bool AdvancedLockManager::HasLock(int32_t page_id, int32_t transaction_id, LockMode min_mode) const {
    std::shared_lock<std::shared_mutex> lock(lock_table_mutex_);

    auto it = lock_table_.find(page_id);
    if (it == lock_table_.end()) {
        return false;
    }

    const PageLockState& state = it->second;

    auto lock_it = state.transaction_locks.find(transaction_id);
    if (lock_it == state.transaction_locks.end()) {
        return false;
    }

    // 检查锁模式是否满足最小要求
    LockMode current_mode = lock_it->second;
    return current_mode >= min_mode;
}

LockMode AdvancedLockManager::GetLockMode(int32_t page_id, int32_t transaction_id) const {
    std::shared_lock<std::shared_mutex> lock(lock_table_mutex_);

    auto it = lock_table_.find(page_id);
    if (it == lock_table_.end()) {
        return SHARED; // 默认值，表示没有锁
    }

    const PageLockState& state = it->second;
    auto lock_it = state.transaction_locks.find(transaction_id);

    return lock_it != state.transaction_locks.end() ? lock_it->second : SHARED;
}

std::vector<int32_t> AdvancedLockManager::GetLockedPages(int32_t transaction_id) const {
    std::shared_lock<std::shared_mutex> lock(lock_table_mutex_);

    std::vector<int32_t> locked_pages;

    for (const auto& pair : lock_table_) {
        const PageLockState& state = pair.second;
        if (state.transaction_locks.find(transaction_id) != state.transaction_locks.end()) {
            locked_pages.push_back(pair.first);
        }
    }

    return locked_pages;
}

std::vector<int32_t> AdvancedLockManager::GetLockHolders(int32_t page_id) const {
    std::shared_lock<std::shared_mutex> lock(lock_table_mutex_);

    auto it = lock_table_.find(page_id);
    if (it == lock_table_.end()) {
        return {};
    }

    const PageLockState& state = it->second;
    std::vector<int32_t> holders;

    for (const auto& holder : state.holders) {
        holders.push_back(holder.transaction_id);
    }

    return holders;
}

LockRequestStatus AdvancedLockManager::UpgradeLock(int32_t page_id, int32_t transaction_id, LockMode new_mode) {
    std::unique_lock<std::shared_mutex> lock(lock_table_mutex_);

    PageLockState& state = GetOrCreatePageLockState(page_id);

    if (upgrade_manager_.PerformUpgrade(state, transaction_id, new_mode)) {
        std::lock_guard<std::mutex> stats_lock(stats_mutex_);
        stats_.lock_upgrades++;
        return GRANTED;
    }

    return ABORTED;
}

LockRequestStatus AdvancedLockManager::DowngradeLock(int32_t page_id, int32_t transaction_id, LockMode new_mode) {
    std::unique_lock<std::shared_mutex> lock(lock_table_mutex_);

    PageLockState& state = GetOrCreatePageLockState(page_id);

    if (upgrade_manager_.PerformDowngrade(state, transaction_id, new_mode)) {
        std::lock_guard<std::mutex> stats_lock(stats_mutex_);
        stats_.lock_downgrades++;
        return GRANTED;
    }

    return ABORTED;
}

bool AdvancedLockManager::DetectAndResolveDeadlock(std::vector<int32_t>& victims) {
    std::shared_lock<std::shared_mutex> lock(lock_table_mutex_);

    std::vector<int32_t> deadlock_chain;
    bool has_deadlock = false;

    // 检查所有等待的事务
    for (const auto& pair : lock_table_) {
        const PageLockState& state = pair.second;

        // 复制队列内容到临时vector进行遍历
        std::queue<LockRequest> temp_queue = state.waiting_queue;
        while (!temp_queue.empty()) {
            const LockRequest& request = temp_queue.front();
            temp_queue.pop();
            
            std::vector<int32_t> chain;
            if (deadlock_detector_.DetectDeadlock(lock_table_, request.transaction_id, chain)) {
                has_deadlock = true;
                victims.insert(victims.end(), chain.begin(), chain.end());
            }
        }
    }

    // 去重受害者列表
    std::sort(victims.begin(), victims.end());
    auto last = std::unique(victims.begin(), victims.end());
    victims.erase(last, victims.end());

    if (has_deadlock && !victims.empty() && deadlock_resolution_strategy_) {
        // 应用死锁解决策略
        int32_t victim = deadlock_resolution_strategy_(victims);

        // 中止受害者事务
        for (int32_t victim_id : victims) {
            if (victim_id == victim) {
                ReleaseAllLocks(victim_id);
                SQLCC_LOG_WARN("Deadlock detected, aborted transaction: " + std::to_string(victim_id));
                break;
            }
        }

        std::lock_guard<std::mutex> stats_lock(stats_mutex_);
        stats_.deadlocks_detected++;
        stats_.deadlocks_resolved++;
    }

    return has_deadlock;
}

void AdvancedLockManager::SetDeadlockResolutionStrategy(std::function<int32_t(const std::vector<int32_t>&)> strategy) {
    deadlock_resolution_strategy_ = std::move(strategy);
}

void AdvancedLockManager::CleanupExpiredLocks() {
    std::unique_lock<std::shared_mutex> lock(lock_table_mutex_);

    auto now = std::chrono::steady_clock::now();
    size_t cleaned_count = 0;

    for (auto& pair : lock_table_) {
        PageLockState& state = pair.second;

        // 清理过期的等待请求
        std::queue<LockRequest> new_queue;
        while (!state.waiting_queue.empty()) {
            LockRequest request = state.waiting_queue.front();
            state.waiting_queue.pop();

            auto elapsed = now - request.request_time;
            if (elapsed < request.timeout) {
                new_queue.push(request);
            } else {
                // 超时请求
                if (request.callback) {
                    request.callback(TIMED_OUT);
                }
                std::lock_guard<std::mutex> stats_lock(stats_mutex_);
                stats_.lock_timeouts++;
                cleaned_count++;
            }
        }
        state.waiting_queue = std::move(new_queue);
    }

    if (cleaned_count > 0) {
        SQLCC_LOG_DEBUG("Cleaned up " + std::to_string(cleaned_count) + " expired lock requests");
    }
}

void AdvancedLockManager::SetLockTimeout(std::chrono::milliseconds timeout) {
    default_timeout_ = timeout;
}

AdvancedLockManager::LockManagerStats AdvancedLockManager::GetStats() const {
    std::lock_guard<std::mutex> stats_lock(stats_mutex_);
    return stats_;
}

void AdvancedLockManager::SetMaxLocks(size_t max_locks) {
    std::unique_lock<std::shared_mutex> lock(lock_table_mutex_);
    max_locks_ = max_locks;
}

void AdvancedLockManager::EnableDeadlockDetection(bool enable) {
    deadlock_detection_enabled_ = enable;
}

void AdvancedLockManager::SetDeadlockCheckInterval(std::chrono::milliseconds interval) {
    deadlock_check_interval_ = interval;
}

// 私有辅助方法实现
PageLockState& AdvancedLockManager::GetOrCreatePageLockState(int32_t page_id) {
    return lock_table_[page_id]; // 自动创建默认构造的PageLockState
}

LockRequestStatus AdvancedLockManager::TryAcquireLock(int32_t page_id, LockMode mode, int32_t transaction_id) {
    PageLockState& state = GetOrCreatePageLockState(page_id);

    // 检查事务是否已经有锁
    auto existing_lock = state.transaction_locks.find(transaction_id);
    if (existing_lock != state.transaction_locks.end()) {
        // 检查是否是锁升级
        if (upgrade_manager_.CanUpgrade(existing_lock->second, mode) != LockUpgradeManager::NO_UPGRADE) {
            return UpgradeLock(page_id, transaction_id, mode);
        }
        // 已经有兼容的锁，直接成功
        return GRANTED;
    }

    // 检查锁兼容性
    if (!IsLockCompatible(state, mode)) {
        // 添加到死锁检测器的等待图
        for (const auto& holder : state.holders) {
            deadlock_detector_.AddWaitFor(transaction_id, holder.transaction_id);
        }
        return WAITING;
    }

    // 授予锁
    state.holders.emplace_back(transaction_id, mode);
    UpdateLockCounts(state, mode, 1);
    state.transaction_locks[transaction_id] = mode;

    std::lock_guard<std::mutex> stats_lock(stats_mutex_);
    stats_.total_locks++;

    return GRANTED;
}

void AdvancedLockManager::GrantWaitingRequests(int32_t page_id) {
    PageLockState& state = GetOrCreatePageLockState(page_id);

    // 检查是否有等待的请求现在可以被授予
    while (!state.waiting_queue.empty()) {
        LockRequest request = state.waiting_queue.front();

        if (IsLockCompatible(state, request.mode)) {
            // 可以授予锁
            state.waiting_queue.pop();
            state.holders.emplace_back(request.transaction_id, request.mode);
            UpdateLockCounts(state, request.mode, 1);
            state.transaction_locks[request.transaction_id] = request.mode;

            // 移除等待关系
            deadlock_detector_.RemoveWaitFor(request.transaction_id);

            // 通知回调
            if (request.callback) {
                request.callback(GRANTED);
            }

            std::lock_guard<std::mutex> stats_lock(stats_mutex_);
            stats_.total_locks++;
            stats_.waiting_requests--;
        } else {
            // 不能授予，保持在队列中
            break;
        }
    }
}

void AdvancedLockManager::UpdateLockWaitTime(std::chrono::milliseconds wait_time) {
    lock_wait_times_.push_back(wait_time.count());

    // 保持最近1000个记录
    if (lock_wait_times_.size() > 1000) {
        lock_wait_times_.erase(lock_wait_times_.begin());
    }

    // 计算平均等待时间
    if (!lock_wait_times_.empty()) {
        double sum = 0.0;
        for (double time : lock_wait_times_) {
            sum += time;
        }
        stats_.average_lock_wait_time_ms = sum / lock_wait_times_.size();
    }
}

void AdvancedLockManager::ProcessAsyncRequests() {
    while (async_worker_running_.load()) {
        std::unique_lock<std::mutex> lock(async_mutex_);

        // 等待请求或停止信号
        async_cv_.wait_for(lock, std::chrono::milliseconds(10));

        // 处理所有待处理的请求
        while (!async_requests_.empty() && async_worker_running_.load()) {
            LockRequest request = async_requests_.front();
            async_requests_.pop();

            lock.unlock();

            // 处理请求
            LockRequestStatus status = AcquireLock(request.page_id, request.mode,
                                                   request.transaction_id, request.timeout);

            // 调用回调
            if (request.callback) {
                request.callback(status);
            }

            lock.lock();
        }
    }
}

void AdvancedLockManager::StartAsyncWorker() {
    if (!async_worker_running_.load()) {
        async_worker_running_.store(true);
        async_worker_thread_ = std::thread(&AdvancedLockManager::ProcessAsyncRequests, this);
    }
}

void AdvancedLockManager::StopAsyncWorker() {
    if (async_worker_running_.load()) {
        async_worker_running_.store(false);
        async_cv_.notify_one();

        if (async_worker_thread_.joinable()) {
            async_worker_thread_.join();
        }
    }
}

bool AdvancedLockManager::IsLockCompatible(const PageLockState& state, LockMode requested) const {
    return compatibility_matrix_.CanGrantImmediately(state, requested);
}

bool AdvancedLockManager::CanUpgradeLock(const PageLockState& state, int32_t transaction_id, LockMode new_mode) const {
    // 检查事务当前是否持有锁
    auto it = state.transaction_locks.find(transaction_id);
    if (it == state.transaction_locks.end()) {
        return false;
    }

    return upgrade_manager_.CanUpgrade(it->second, new_mode) != LockUpgradeManager::NO_UPGRADE;
}

// 锁管理器工厂实现
std::shared_ptr<AdvancedLockManager> LockManagerFactory::CreateBasicLockManager(
    std::chrono::milliseconds default_timeout) {

    return std::make_shared<AdvancedLockManager>(10000, default_timeout);
}

std::shared_ptr<AdvancedLockManager> LockManagerFactory::CreateHighConcurrencyLockManager(
    size_t max_locks, std::chrono::milliseconds default_timeout) {

    auto manager = std::make_shared<AdvancedLockManager>(max_locks, default_timeout);
    manager->SetDeadlockCheckInterval(std::chrono::milliseconds(50)); // 更频繁的死锁检查
    return manager;
}

std::shared_ptr<AdvancedLockManager> LockManagerFactory::CreateStrictLockManager(
    std::chrono::milliseconds default_timeout) {

    auto manager = std::make_shared<AdvancedLockManager>(5000, default_timeout);
    manager->SetDeadlockCheckInterval(std::chrono::milliseconds(10)); // 最严格的死锁检查
    return manager;
}

} // namespace sqlcc
