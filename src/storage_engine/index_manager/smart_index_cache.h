#pragma once

#include <memory>
#include <unordered_map>
#include <mutex>
#include <string>
#include <vector>
#include <chrono>

namespace sqlcc {

class BPlusTreeIndex;

namespace storage_engine {

namespace index_manager {

struct CacheEntry {
    std::unique_ptr<BPlusTreeIndex> index;
    int priority;
    std::chrono::steady_clock::time_point create_time;
    std::chrono::steady_clock::time_point expiry_time;
    size_t access_count;
    double access_frequency;
    std::chrono::steady_clock::time_point last_access;
};

struct EnhancedCacheStats {
    size_t total_indexes = 0;
    size_t expired_entries = 0;
    size_t high_priority_entries = 0;
    double average_access_frequency = 0.0;
    std::chrono::steady_clock::time_point oldest_access;
    std::chrono::steady_clock::time_point newest_access;
    std::unordered_map<int, size_t> priority_distribution;
};

class SmartIndexCache {
public:
    SmartIndexCache(size_t max_cache_size = 100, std::chrono::minutes default_ttl = std::chrono::minutes(30));
    ~SmartIndexCache();

    void CacheIndex(const std::string& index_name, std::unique_ptr<BPlusTreeIndex> index,
                    int priority = 5, std::chrono::minutes ttl = std::chrono::minutes(0));

    BPlusTreeIndex* GetIndex(const std::string& index_name);
    bool HasIndex(const std::string& index_name) const;
    bool RemoveIndex(const std::string& index_name);

    void WarmupCache(const std::vector<std::string>& predicted_indexes);
    void IntelligentCleanup();
    EnhancedCacheStats GetEnhancedCacheStats() const;
    std::vector<BPlusTreeIndex*> GetMultipleIndexes(const std::vector<std::string>& index_names);
    void CleanupExpiredCache(std::chrono::minutes max_age = std::chrono::minutes(60));

    size_t get_size() const { return index_cache_.size(); }

private:
    void EvictCacheEntries();

    mutable std::mutex cache_mutex_;
    size_t max_cache_size_;
    std::chrono::minutes default_ttl_;

    std::unordered_map<std::string, CacheEntry> index_cache_;
    std::unordered_map<std::string, std::chrono::steady_clock::time_point> access_times_;
};

} // namespace index_manager
} // namespace storage_engine
} // namespace sqlcc
