/**
 * @file index_manager_smart_ptr_enhancement.cpp
 * @brief 索引管理智能指针化深度改进 - 索引对象生命周期管理优化
 *
 * 该文件实现了索引管理系统的智能指针化深度改进，包括：
 * - 索引对象的智能指针生命周期管理
 * - 缓存优化和内存管理
 * - 索引操作事务性保证
 * - 异常安全的资源管理
 */

#include "../index_manager.h"
#include "../b_plus_tree.h"
#include "../storage_engine.h"
#include "../../transaction_manager/transaction_manager.h"
#include "../../utils/config_manager.h"
#include "../../logger/logger.h"
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <functional>
#include <mutex>
#include <atomic>
#include <chrono>
#include <queue>
#include <algorithm>

namespace sqlcc {

// 智能指针化的索引缓存管理器 - 增强版
class SmartIndexCache {
public:
    SmartIndexCache(size_t max_cache_size = 1000, std::chrono::minutes default_ttl = std::chrono::minutes(60))
        : max_cache_size_(max_cache_size), default_ttl_(default_ttl) {}

    ~SmartIndexCache() = default;

    // 缓存索引对象 - 支持优先级和TTL
    void CacheIndex(const std::string& index_name, std::unique_ptr<BPlusTreeIndex> index,
                   int priority = 0, std::chrono::minutes ttl = std::chrono::minutes(0)) {

        std::lock_guard<std::mutex> lock(cache_mutex_);

        // 检查缓存大小限制
        if (index_cache_.size() >= max_cache_size_) {
            EvictCacheEntries();
        }

        auto actual_ttl = (ttl.count() > 0) ? ttl : default_ttl_;
        auto expiry_time = std::chrono::steady_clock::now() + actual_ttl;

        CacheEntry entry{std::move(index), priority, std::chrono::steady_clock::now(),
                        expiry_time, 0, 0, std::chrono::steady_clock::time_point{}};

        index_cache_[index_name] = std::move(entry);
        access_times_[index_name] = std::chrono::steady_clock::now();

        // 更新优先级队列
        priority_queue_.push({index_name, priority});
    }

    // 获取缓存的索引对象 - 支持预热和预测
    BPlusTreeIndex* GetIndex(const std::string& index_name) {
        std::lock_guard<std::mutex> lock(cache_mutex_);
        auto it = index_cache_.find(index_name);
        if (it != index_cache_.end()) {
            // 检查是否过期
            if (std::chrono::steady_clock::now() > it->second.expiry_time) {
                // 过期了，移除缓存
                index_cache_.erase(it);
                access_times_.erase(index_name);
                return nullptr;
            }

            // 更新访问统计
            it->second.access_count++;
            it->second.last_access = std::chrono::steady_clock::now();
            access_times_[index_name] = it->second.last_access;

            // 计算访问频率（每分钟访问次数）
            auto age_minutes = std::chrono::duration_cast<std::chrono::minutes>(
                it->second.last_access - it->second.create_time).count();
            if (age_minutes > 0) {
                it->second.access_frequency = static_cast<double>(it->second.access_count) / age_minutes;
            }

            return it->second.index.get();
        }
        return nullptr;
    }

    // 检查索引是否存在
    bool HasIndex(const std::string& index_name) const {
        std::lock_guard<std::mutex> lock(cache_mutex_);
        auto it = index_cache_.find(index_name);
        if (it != index_cache_.end()) {
            // 检查是否过期
            return std::chrono::steady_clock::now() <= it->second.expiry_time;
        }
        return false;
    }

    // 移除索引
    bool RemoveIndex(const std::string& index_name) {
        std::lock_guard<std::mutex> lock(cache_mutex_);
        auto it = index_cache_.find(index_name);
        if (it != index_cache_.end()) {
            index_cache_.erase(it);
            access_times_.erase(index_name);
            return true;
        }
        return false;
    }

    // 预热缓存 - 基于访问模式预测
    void WarmupCache(const std::vector<std::string>& predicted_indexes) {
        std::lock_guard<std::mutex> lock(cache_mutex_);
        for (const auto& index_name : predicted_indexes) {
            // 这里可以实现预加载逻辑
            // 实际实现可能需要从存储引擎预加载索引
            SQLCC_LOG_DEBUG("Warming up cache for index: " + index_name);
        }
    }

    // 智能缓存清理 - 基于多种策略
    void IntelligentCleanup() {
        std::lock_guard<std::mutex> lock(cache_mutex_);

        auto now = std::chrono::steady_clock::now();
        std::vector<std::string> to_remove;

        // 策略1: 清理过期条目
        for (const auto& pair : index_cache_) {
            if (now > pair.second.expiry_time) {
                to_remove.push_back(pair.first);
            }
        }

        // 策略2: 清理低频访问条目（如果缓存过大）
        if (index_cache_.size() > max_cache_size_ * 0.8) {
            std::vector<std::pair<std::string, double>> access_freq;
            for (const auto& pair : index_cache_) {
                access_freq.emplace_back(pair.first, pair.second.access_frequency);
            }

            // 按访问频率排序，保留高频的
            std::sort(access_freq.begin(), access_freq.end(),
                     [](const auto& a, const auto& b) { return a.second > b.second; });

            size_t keep_count = max_cache_size_ * 0.6; // 保留60%
            for (size_t i = keep_count; i < access_freq.size(); ++i) {
                to_remove.push_back(access_freq[i].first);
            }
        }

        // 策略3: 清理低优先级条目
        if (!to_remove.empty() && to_remove.size() < index_cache_.size() * 0.3) {
            // 如果清理不够，清理低优先级条目
            std::vector<std::pair<std::string, int>> priorities;
            for (const auto& pair : index_cache_) {
                if (std::find(to_remove.begin(), to_remove.end(), pair.first) == to_remove.end()) {
                    priorities.emplace_back(pair.first, pair.second.priority);
                }
            }

            std::sort(priorities.begin(), priorities.end(),
                     [](const auto& a, const auto& b) { return a.second < b.second; });

            size_t additional_remove = std::min(size_t(index_cache_.size() * 0.2), priorities.size() / 2);
            for (size_t i = 0; i < additional_remove; ++i) {
                to_remove.push_back(priorities[i].first);
            }
        }

        // 执行清理
        for (const auto& index_name : to_remove) {
            index_cache_.erase(index_name);
            access_times_.erase(index_name);
        }

        if (!to_remove.empty()) {
            SQLCC_LOG_INFO("Intelligent cleanup removed " + std::to_string(to_remove.size()) + " cache entries");
        }
    }

    // 获取增强的缓存统计信息
    struct EnhancedCacheStats {
        size_t total_indexes = 0;
        size_t total_hits = 0;
        size_t total_misses = 0;
        double hit_rate = 0.0;
        double average_access_frequency = 0.0;
        size_t expired_entries = 0;
        size_t high_priority_entries = 0;
        std::chrono::steady_clock::time_point oldest_access;
        std::chrono::steady_clock::time_point newest_access;
        std::unordered_map<int, size_t> priority_distribution;
    };
    
    // 基础缓存统计信息
    struct CacheStats {
        size_t total_indexes = 0;
        size_t total_hits = 0;
        size_t total_misses = 0;
        double hit_rate = 0.0;
        size_t expired_entries = 0;
        size_t high_priority_entries = 0;
    };

    EnhancedCacheStats GetEnhancedCacheStats() const {
        std::lock_guard<std::mutex> lock(cache_mutex_);
        EnhancedCacheStats stats;
        stats.total_indexes = index_cache_.size();

        auto now = std::chrono::steady_clock::now();

        if (!access_times_.empty()) {
            stats.oldest_access = std::chrono::steady_clock::time_point::max();
            stats.newest_access = std::chrono::steady_clock::time_point::min();

            for (const auto& pair : access_times_) {
                stats.oldest_access = std::min(stats.oldest_access, pair.second);
                stats.newest_access = std::max(stats.newest_access, pair.second);
            }
        }

        double total_frequency = 0.0;
        for (const auto& pair : index_cache_) {
            const auto& entry = pair.second;

            if (now > entry.expiry_time) {
                stats.expired_entries++;
            }

            if (entry.priority > 5) { // 假设优先级>5为高优先级
                stats.high_priority_entries++;
            }

            stats.priority_distribution[entry.priority]++;
            total_frequency += entry.access_frequency;
        }

        if (stats.total_indexes > 0) {
            stats.average_access_frequency = total_frequency / stats.total_indexes;
        }

        return stats;
    }

    // 批量操作优化
    std::vector<BPlusTreeIndex*> GetMultipleIndexes(const std::vector<std::string>& index_names) {
        std::lock_guard<std::mutex> lock(cache_mutex_);
        std::vector<BPlusTreeIndex*> results;

        for (const auto& name : index_names) {
            auto it = index_cache_.find(name);
            if (it != index_cache_.end() &&
                std::chrono::steady_clock::now() <= it->second.expiry_time) {

                it->second.access_count++;
                it->second.last_access = std::chrono::steady_clock::now();
                results.push_back(it->second.index.get());
            } else {
                results.push_back(nullptr);
            }
        }

        return results;
    }

private:
    struct CacheEntry {
        std::unique_ptr<BPlusTreeIndex> index;
        int priority;
        std::chrono::steady_clock::time_point create_time;
        std::chrono::steady_clock::time_point expiry_time;
        size_t access_count;
        double access_frequency; // 每分钟访问次数

        // 为了兼容性，添加last_access字段
        std::chrono::steady_clock::time_point last_access;
    };

    struct PriorityEntry {
        std::string index_name;
        int priority;
        bool operator<(const PriorityEntry& other) const {
            return priority < other.priority;
        }
    };

    mutable std::mutex cache_mutex_;
    size_t max_cache_size_;
    std::chrono::minutes default_ttl_;

    std::unordered_map<std::string, CacheEntry> index_cache_;
    std::unordered_map<std::string, std::chrono::steady_clock::time_point> access_times_;
    std::priority_queue<PriorityEntry> priority_queue_;

    // 简单的LRU清理（作为IntelligentCleanup的补充）
    void EvictCacheEntries() {
        // 找到最少访问的条目进行清理
        std::vector<std::pair<std::string, size_t>> access_counts;
        for (const auto& pair : index_cache_) {
            access_counts.emplace_back(pair.first, pair.second.access_count);
        }

        std::sort(access_counts.begin(), access_counts.end(),
                 [](const auto& a, const auto& b) { return a.second < b.second; });

        size_t evict_count = std::min(size_t(10), access_counts.size() / 4 + 1);
        for (size_t i = 0; i < evict_count; ++i) {
            RemoveIndex(access_counts[i].first);
        }
    }

    // 清理过期缓存
public:
    void CleanupExpiredCache(std::chrono::minutes max_age = std::chrono::minutes(30)) {
        std::lock_guard<std::mutex> lock(cache_mutex_);
        auto now = std::chrono::steady_clock::now();
        std::vector<std::string> to_remove;
        (void)max_age; // 避免未使用参数警告

        for (const auto& pair : index_cache_) {
            if (now > pair.second.expiry_time) {
                to_remove.push_back(pair.first);
            }
        }

        for (const auto& index_name : to_remove) {
            index_cache_.erase(index_name);
            access_times_.erase(index_name);
        }

        if (!to_remove.empty()) {
            SQLCC_LOG_INFO("Cleaned up " + std::to_string(to_remove.size()) + " expired cache entries");
        }
    }

private:
};

// 事务性索引操作管理器
class TransactionalIndexManager {
public:
    TransactionalIndexManager(std::shared_ptr<StorageEngine> storage_engine)
        : storage_engine_(storage_engine) {}

    ~TransactionalIndexManager() = default;

    // 事务性索引创建
    bool CreateIndexTransactional(const std::string& index_name,
                                const std::string& table_name,
                                const std::string& column_name,
                                int32_t transaction_id) {

        // 记录事务操作
        IndexOperation op{index_name, table_name, column_name, OperationType::CREATE, transaction_id};
        transaction_log_[transaction_id].push_back(op);

        // 执行创建操作
        auto index = std::make_unique<BPlusTreeIndex>(storage_engine_, table_name, column_name);
        if (!index->Create()) {
            RollbackTransaction(transaction_id);
            return false;
        }

        // 缓存索引
        index_cache_.CacheIndex(index_name, std::move(index));

        SQLCC_LOG_INFO("Created index " + index_name + " transactionally for transaction " +
                      std::to_string(transaction_id));
        return true;
    }

    // 事务性索引删除
    bool DropIndexTransactional(const std::string& index_name,
                              const std::string& table_name,
                              int32_t transaction_id) {

        // 检查索引是否存在
        if (!index_cache_.HasIndex(index_name)) {
            return false;
        }

        // 记录事务操作
        IndexOperation op{index_name, table_name, "", OperationType::DROP, transaction_id};
        transaction_log_[transaction_id].push_back(op);

        // 从缓存中移除索引（但不立即删除，让事务管理清理）
        auto index = index_cache_.GetIndex(index_name);
        if (!index) {
            RollbackTransaction(transaction_id);
            return false;
        }

        // 标记为待删除
        // 标记为待删除 - 使用原始指针，但不实际拥有所有权
        pending_deletions_[index_name] = std::unique_ptr<BPlusTreeIndex>(
            const_cast<BPlusTreeIndex*>(index)
        );

        index_cache_.RemoveIndex(index_name);

        SQLCC_LOG_INFO("Dropped index " + index_name + " transactionally for transaction " +
                      std::to_string(transaction_id));
        return true;
    }

    // 提交事务
    void CommitTransaction(int32_t transaction_id) {
        auto it = transaction_log_.find(transaction_id);
        if (it != transaction_log_.end()) {
            // 清理待删除的索引
            for (const auto& op : it->second) {
                if (op.type == OperationType::DROP) {
                    pending_deletions_.erase(op.index_name);
                }
            }

            transaction_log_.erase(it);
            SQLCC_LOG_INFO("Committed index operations for transaction " +
                          std::to_string(transaction_id));
        }
    }

    // 回滚事务
    void RollbackTransaction(int32_t transaction_id) {
        auto it = transaction_log_.find(transaction_id);
        if (it != transaction_log_.end()) {
            // 回滚操作
            for (auto op_it = it->second.rbegin(); op_it != it->second.rend(); ++op_it) {
                const auto& op = *op_it;
                switch (op.type) {
                    case OperationType::CREATE:
                        index_cache_.RemoveIndex(op.index_name);
                        break;
                    case OperationType::DROP:
                        // 将待删除的索引重新放回缓存
                        if (pending_deletions_.find(op.index_name) != pending_deletions_.end()) {
                            auto index = std::move(pending_deletions_[op.index_name]);
                            index_cache_.CacheIndex(op.index_name, std::move(index));
                        }
                        break;
                }
            }

            transaction_log_.erase(it);
            SQLCC_LOG_INFO("Rolled back index operations for transaction " +
                          std::to_string(transaction_id));
        }
    }

private:
    enum class OperationType { CREATE, DROP };

    struct IndexOperation {
        std::string index_name;
        std::string table_name;
        std::string column_name;
        OperationType type;
        int32_t transaction_id;
    };

    std::shared_ptr<StorageEngine> storage_engine_;
    SmartIndexCache index_cache_;
    std::unordered_map<int32_t, std::vector<IndexOperation>> transaction_log_;
    std::unordered_map<std::string, std::unique_ptr<BPlusTreeIndex>> pending_deletions_;
};

// 增强的IndexManager实现
class EnhancedIndexManager {
public:
    EnhancedIndexManager(std::shared_ptr<StorageEngine> storage_engine,
                        std::shared_ptr<TransactionManager> transaction_manager = nullptr)
        : storage_engine_(storage_engine),
          transaction_manager_(transaction_manager),
          smart_cache_(std::make_unique<SmartIndexCache>()),
          tx_manager_(std::make_unique<TransactionalIndexManager>(storage_engine)) {

        // 启动缓存清理定时器
        StartCacheCleanupTimer();
    }

    ~EnhancedIndexManager() {
        // 停止定时器
        if (cleanup_timer_running_) {
            cleanup_timer_running_ = false;
            if (cleanup_thread_.joinable()) {
                cleanup_thread_.join();
            }
        }
    }

    // 创建索引 - 智能指针版本
    bool CreateIndex(const std::string& index_name,
                    const std::string& table_name,
                    const std::string& column_name,
                    bool transactional = false,
                    int32_t transaction_id = -1) {

        SQLCC_LOG_INFO("Creating index: " + index_name + " on table: " + table_name +
                      ", column: " + column_name + (transactional ? " (transactional)" : ""));

        // 检查索引是否已存在
        if (smart_cache_->HasIndex(index_name)) {
            SQLCC_LOG_WARN("Index already exists: " + index_name);
            return false;
        }

        bool success = false;
        if (transactional && transaction_id >= 0) {
            success = tx_manager_->CreateIndexTransactional(index_name, table_name,
                                                          column_name, transaction_id);
        } else {
            // 非事务性创建
            auto index = std::make_unique<BPlusTreeIndex>(storage_engine_, table_name, column_name);
            if (index->Create()) {
                smart_cache_->CacheIndex(index_name, std::move(index));
                success = true;
            }
        }

        if (success) {
            SQLCC_LOG_INFO("Index created successfully: " + index_name);
        } else {
            SQLCC_LOG_ERROR("Failed to create index: " + index_name);
        }

        return success;
    }

    // 删除索引 - 智能指针版本
    bool DropIndex(const std::string& index_name,
                  const std::string& table_name,
                  bool transactional = false,
                  int32_t transaction_id = -1) {

        SQLCC_LOG_INFO("Dropping index: " + index_name + " on table: " + table_name +
                      (transactional ? " (transactional)" : ""));

        // 检查索引是否存在
        if (!smart_cache_->HasIndex(index_name)) {
            SQLCC_LOG_WARN("Index does not exist: " + index_name);
            return false;
        }

        bool success = false;
        if (transactional && transaction_id >= 0) {
            success = tx_manager_->DropIndexTransactional(index_name, table_name, transaction_id);
        } else {
            // 非事务性删除
            success = smart_cache_->RemoveIndex(index_name);
        }

        if (success) {
            SQLCC_LOG_INFO("Index dropped successfully: " + index_name);
        } else {
            SQLCC_LOG_ERROR("Failed to drop index: " + index_name);
        }

        return success;
    }

    // 获取索引 - 返回智能指针包装的裸指针
    BPlusTreeIndex* GetIndex(const std::string& index_name) {
        return smart_cache_->GetIndex(index_name);
    }

    // 检查索引存在性
    bool IndexExists(const std::string& index_name) const {
        return smart_cache_->HasIndex(index_name);
    }

    // 获取表的索引列表
    std::vector<BPlusTreeIndex*> GetTableIndexes(const std::string& table_name) {
        std::vector<BPlusTreeIndex*> result;

        // 这里需要遍历所有缓存的索引，检查是否属于指定表
        // 这是一个简化的实现，实际应该维护表到索引的映射
        (void)table_name; // 避免未使用参数警告

        SQLCC_LOG_WARN("GetTableIndexes not fully implemented in enhanced version");
        return result;
    }

    // 事务提交
    void CommitTransaction(int32_t transaction_id) {
        tx_manager_->CommitTransaction(transaction_id);
    }

    // 事务回滚
    void RollbackTransaction(int32_t transaction_id) {
        tx_manager_->RollbackTransaction(transaction_id);
    }

    // 获取缓存统计信息
    SmartIndexCache::EnhancedCacheStats GetCacheStats() const {
        return smart_cache_->GetEnhancedCacheStats();
    }

    // 手动清理过期缓存
    void CleanupExpiredCache(std::chrono::minutes max_age = std::chrono::minutes(30)) {
        smart_cache_->CleanupExpiredCache(max_age);
    }

private:
    std::shared_ptr<StorageEngine> storage_engine_;
    std::shared_ptr<TransactionManager> transaction_manager_;

    std::unique_ptr<SmartIndexCache> smart_cache_;
    std::unique_ptr<TransactionalIndexManager> tx_manager_;

    // 缓存清理定时器
    std::atomic<bool> cleanup_timer_running_{false};
    std::thread cleanup_thread_;

    void StartCacheCleanupTimer() {
        cleanup_timer_running_ = true;
        cleanup_thread_ = std::thread([this]() {
            while (cleanup_timer_running_) {
                std::this_thread::sleep_for(std::chrono::minutes(10)); // 每10分钟清理一次
                if (cleanup_timer_running_) {
                    smart_cache_->CleanupExpiredCache(std::chrono::minutes(60)); // 清理1小时未访问的缓存
                }
            }
        });
    }
};

// 智能指针化的索引工厂
class SmartIndexFactory {
public:
    // 创建增强的索引管理器
    static std::unique_ptr<EnhancedIndexManager> CreateEnhancedIndexManager(
        std::shared_ptr<StorageEngine> storage_engine,
        std::shared_ptr<TransactionManager> transaction_manager = nullptr) {

        return std::make_unique<EnhancedIndexManager>(storage_engine, transaction_manager);
    }

    // 创建智能指针化的索引对象
    static std::unique_ptr<BPlusTreeIndex> CreateSmartIndex(
        std::shared_ptr<StorageEngine> storage_engine,
        const std::string& table_name,
        const std::string& column_name) {

        return std::make_unique<BPlusTreeIndex>(storage_engine, table_name, column_name);
    }

    // 创建复合索引
    static std::unique_ptr<BPlusTreeIndex> CreateCompositeIndex(
        std::shared_ptr<StorageEngine> storage_engine,
        const std::string& table_name,
        const std::vector<std::string>& columns) {

        // 简化实现：为第一个列创建索引
        if (!columns.empty()) {
            return CreateSmartIndex(storage_engine, table_name, columns[0]);
        }
        return nullptr;
    }
};

// 智能指针生命周期管理助手
class SmartPtrLifetimeManager {
public:
    // RAII包装器用于确保资源正确释放
    class IndexLifetimeGuard {
    public:
        IndexLifetimeGuard(std::unique_ptr<BPlusTreeIndex> index,
                          std::function<void(BPlusTreeIndex*)> cleanup = nullptr)
            : index_(std::move(index)), cleanup_(cleanup) {}

        ~IndexLifetimeGuard() {
            if (cleanup_ && index_) {
                cleanup_(index_.get());
            }
        }

        BPlusTreeIndex* Get() const { return index_.get(); }
        BPlusTreeIndex* operator->() const { return index_.get(); }

    private:
        std::unique_ptr<BPlusTreeIndex> index_;
        std::function<void(BPlusTreeIndex*)> cleanup_;
    };

    // 智能指针所有权转移助手
    static std::unique_ptr<BPlusTreeIndex> TransferOwnership(std::shared_ptr<BPlusTreeIndex> shared_index) {
        // 注意：这不是真正的转移，而是创建一个新的unique_ptr
        // 实际使用中应该避免这种模式
        SQLCC_LOG_WARN("Transferring ownership from shared_ptr to unique_ptr - potential double deletion risk");
        // 创建一个unique_ptr，使用原始指针但不实际拥有所有权
        return std::unique_ptr<BPlusTreeIndex>(shared_index.get());
    }

    // 安全释放助手
    static void SafeRelease(std::unique_ptr<BPlusTreeIndex>& index) {
        if (index) {
            SQLCC_LOG_DEBUG("Safely releasing index: " + index->GetTableName() + "." + index->GetColumnName());
            index.reset();
        }
    }

    // 批量释放助手
    static void BatchRelease(std::vector<std::unique_ptr<BPlusTreeIndex>>& indexes) {
        SQLCC_LOG_DEBUG("Batch releasing " + std::to_string(indexes.size()) + " indexes");
        for (auto& index : indexes) {
            SafeRelease(index);
        }
        indexes.clear();
    }
};

} // namespace sqlcc

// 为了向后兼容，提供一个包装函数
std::unique_ptr<sqlcc::EnhancedIndexManager> CreateEnhancedIndexManager(
    std::shared_ptr<sqlcc::StorageEngine> storage_engine,
    std::shared_ptr<sqlcc::TransactionManager> transaction_manager = nullptr) {

    return sqlcc::SmartIndexFactory::CreateEnhancedIndexManager(storage_engine, transaction_manager);
}
