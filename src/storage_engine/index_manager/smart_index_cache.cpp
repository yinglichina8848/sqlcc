#include "storage_engine/index_manager/smart_index_cache.h"
#include "storage/b_plus_tree.h"
#include "utils/logger.h"
#include <algorithm>
#include <chrono>

namespace sqlcc {
namespace storage_engine {
namespace index_manager {

SmartIndexCache::SmartIndexCache(size_t max_cache_size, std::chrono::minutes default_ttl)
    : max_cache_size_(max_cache_size), default_ttl_(default_ttl) {}

void SmartIndexCache::CacheIndex(const std::string& index_name,
                                std::unique_ptr<BPlusTreeIndex> index,
                                int priority,
                                std::chrono::minutes ttl) {
    std::lock_guard<std::mutex> lock(cache_mutex_);

    // 检查缓存大小限制
    if (index_cache_.size() >= max_cache_size_) {
        EvictCacheEntries();
    }

    auto actual_ttl = (ttl.count() > 0) ? ttl : default_ttl_;
    auto expiry_time = std::chrono::steady_clock::now() + actual_ttl;

    CacheEntry entry{std::move(index), priority, std::chrono::steady_clock::now(),
                    expiry_time, 0, 0.0, std::chrono::steady_clock::time_point{}};

    index_cache_[index_name] = std::move(entry);
    access_times_[index_name] = std::chrono::steady_clock::now();

    // 更新优先级队列
    priority_queue_.push({index_name, priority});
}

BPlusTreeIndex* SmartIndexCache::GetIndex(const std::string& index_name) {
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

bool SmartIndexCache::HasIndex(const std::string& index_name) const {
    std::lock_guard<std::mutex> lock(cache_mutex_);
    auto it = index_cache_.find(index_name);
    if (it != index_cache_.end()) {
        // 检查是否过期
        return std::chrono::steady_clock::now() <= it->second.expiry_time;
    }
    return false;
}

bool SmartIndexCache::RemoveIndex(const std::string& index_name) {
    std::lock_guard<std::mutex> lock(cache_mutex_);
    auto it = index_cache_.find(index_name);
    if (it != index_cache_.end()) {
        index_cache_.erase(it);
        access_times_.erase(index_name);
        return true;
    }
    return false;
}

void SmartIndexCache::WarmupCache(const std::vector<std::string>& predicted_indexes) {
    std::lock_guard<std::mutex> lock(cache_mutex_);
    for (const auto& index_name : predicted_indexes) {
        // 这里可以实现预加载逻辑
        // 实际实现可能需要从存储引擎预加载索引
        SQLCC_LOG_DEBUG("Warming up cache for index: " + index_name);
    }
}

void SmartIndexCache::IntelligentCleanup() {
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

SmartIndexCache::EnhancedCacheStats SmartIndexCache::GetEnhancedCacheStats() const {
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

std::vector<BPlusTreeIndex*> SmartIndexCache::GetMultipleIndexes(const std::vector<std::string>& index_names) {
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

void SmartIndexCache::CleanupExpiredCache(std::chrono::minutes max_age) {
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

void SmartIndexCache::EvictCacheEntries() {
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

} // namespace index_manager
} // namespace storage_engine
} // namespace sqlcc
