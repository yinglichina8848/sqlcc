#pragma once

#include "page.h"
#include "disk_manager.h"
#include <unordered_map>
#include <mutex>
#include <memory>
#include <chrono>
#include <list>
#include <cstring>

#include "disk_manager.h"
#include "page.h"
#include "config_manager.h"
#include "exception.h"

namespace sqlcc {

// Simplified BufferPool class following production-ready design principles
class BufferPool {
public:
    // Core functionality kept simple and clean
    explicit BufferPool(DiskManager* disk_manager, size_t pool_size, ConfigManager& config_manager);
    ~BufferPool();

    // Core page management operations
    Page* FetchPage(int32_t page_id);
    bool UnpinPage(int32_t page_id, bool is_dirty);
    Page* NewPage(int32_t* page_id);
    bool FlushPage(int32_t page_id);
    bool DeletePage(int32_t page_id);

    // Dynamic buffer pool resizing
    bool Resize(size_t new_pool_size);

    // Performance monitoring interface
    struct Metrics {
        size_t total_requests = 0;
        size_t cache_hits = 0;
        size_t evictions = 0;
        double hit_rate() const {
            return total_requests > 0 ? (static_cast<double>(cache_hits) * 100.0) / total_requests : 0.0;
        }
    };
    Metrics GetMetrics() const;

    // Utility methods
    size_t GetPoolSize() const { return pool_size_; }
    size_t GetUsedPages() const;
    bool IsPageInBuffer(int32_t page_id) const;

private:
    // Internal implementation details
    int32_t FindVictimPage();
    bool ReplacePage(int32_t victim_page_id, int32_t new_page_id);
    void UpdateLRU(int32_t page_id);

    // Member variables
    DiskManager* disk_manager_;
    ConfigManager& config_manager_;
    size_t pool_size_;

    // Data structures - simplified design
    std::unordered_map<int32_t, Page*> page_table_;           // page_id -> Page*
    std::unordered_map<int32_t, int> page_refs_;              // Reference counts
    std::unordered_map<int32_t, bool> dirty_pages_;           // Dirty page tracking

    // Simplified LRU using std::list with iterators
    std::list<int32_t> lru_list_;
    std::unordered_map<int32_t, std::list<int32_t>::iterator> lru_map_;

    // Concurrency control - unified with hierarchical locking
    mutable std::timed_mutex latch_;

    // Performance metrics
    mutable Metrics metrics_;
    size_t lock_timeout_ms_;  // Unified timeout

    // Prevent copying
    BufferPool(const BufferPool&) = delete;
    BufferPool& operator=(const BufferPool&) = delete;
};

} // namespace sqlcc
