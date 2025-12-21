#pragma once

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <chrono>
#include <mutex>
#include <queue>
#include <functional>
#include <atomic>

namespace sqlcc {
namespace storage_engine {
namespace index_manager {

// Forward declarations
class BPlusTreeIndex;

/**
 * @brief 智能索引缓存管理器
 *
 * 提供高级的索引缓存功能，支持优先级、TTL、访问统计和智能清理策略。
 * 实现了LRU缓存、多策略清理和访问模式预测等高级特性。
 */
class SmartIndexCache {
public:
    /**
     * @brief 增强的缓存统计信息
     */
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

    /**
     * @brief 基础缓存统计信息
     */
    struct CacheStats {
        size_t total_indexes = 0;
        size_t total_hits = 0;
        size_t total_misses = 0;
        double hit_rate = 0.0;
        size_t expired_entries = 0;
        size_t high_priority_entries = 0;
    };

    /**
     * @brief 构造函数
     * @param max_cache_size 最大缓存大小
     * @param default_ttl 默认生存时间
     */
    SmartIndexCache(size_t max_cache_size = 1000,
                   std::chrono::minutes default_ttl = std::chrono::minutes(60));

    /**
     * @brief 析构函数
     */
    ~SmartIndexCache() = default;

    /**
     * @brief 缓存索引对象
     * @param index_name 索引名称
     * @param index 索引对象智能指针
     * @param priority 优先级
     * @param ttl 生存时间（可选）
     */
    void CacheIndex(const std::string& index_name,
                   std::unique_ptr<BPlusTreeIndex> index,
                   int priority = 0,
                   std::chrono::minutes ttl = std::chrono::minutes(0));

    /**
     * @brief 获取缓存的索引对象
     * @param index_name 索引名称
     * @return 索引对象指针（如果不存在返回nullptr）
     */
    BPlusTreeIndex* GetIndex(const std::string& index_name);

    /**
     * @brief 检查索引是否存在
     * @param index_name 索引名称
     * @return 是否存在
     */
    bool HasIndex(const std::string& index_name) const;

    /**
     * @brief 移除索引
     * @param index_name 索引名称
     * @return 是否成功移除
     */
    bool RemoveIndex(const std::string& index_name);

    /**
     * @brief 预热缓存
     * @param predicted_indexes 预测的索引列表
     */
    void WarmupCache(const std::vector<std::string>& predicted_indexes);

    /**
     * @brief 智能缓存清理
     */
    void IntelligentCleanup();

    /**
     * @brief 获取增强的缓存统计信息
     * @return 增强的缓存统计信息
     */
    EnhancedCacheStats GetEnhancedCacheStats() const;

    /**
     * @brief 批量获取多个索引
     * @param index_names 索引名称列表
     * @return 索引对象指针列表
     */
    std::vector<BPlusTreeIndex*> GetMultipleIndexes(const std::vector<std::string>& index_names);

    /**
     * @brief 手动清理过期缓存
     * @param max_age 最大年龄
     */
    void CleanupExpiredCache(std::chrono::minutes max_age = std::chrono::minutes(30));

private:
    /**
     * @brief 缓存条目结构
     */
    struct CacheEntry {
        std::unique_ptr<BPlusTreeIndex> index;
        int priority;
        std::chrono::steady_clock::time_point create_time;
        std::chrono::steady_clock::time_point expiry_time;
        size_t access_count;
        double access_frequency; // 每分钟访问次数
        std::chrono::steady_clock::time_point last_access;
    };

    /**
     * @brief 优先级条目结构
     */
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

    /**
     * @brief 简单的LRU清理（补充IntelligentCleanup）
     */
    void EvictCacheEntries();

    // 禁止拷贝和赋值
    SmartIndexCache(const SmartIndexCache&) = delete;
    SmartIndexCache& operator=(const SmartIndexCache&) = delete;
};

} // namespace index_manager
} // namespace storage_engine
} // namespace sqlcc
