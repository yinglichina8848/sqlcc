#pragma once

#include "page.h"
#include "disk_manager.h"
#include <unordered_map>
#include <memory>
#include <mutex>
#include <atomic>
#include <chrono>

namespace sqlcc {

// 重构后的缓冲池，简化和优化现有BufferPool的设计
class BufferPoolV2 {
public:
    // 构造函数和析构函数
    explicit BufferPoolV2(DiskManager* disk_manager, size_t pool_size);
    BufferPoolV2(const BufferPoolV2&) = delete;
    BufferPoolV2& operator=(const BufferPoolV2&) = delete;
    ~BufferPoolV2();

    // 核心页面操作
    Page* FetchPage(int32_t page_id);
    bool UnpinPage(int32_t page_id, bool is_dirty);
    Page* NewPage(int32_t* page_id);

    // 页面持久化
    bool FlushPage(int32_t page_id);
    bool DeletePage(int32_t page_id);
    void FlushAllPages();

    // 性能监控接口
    struct Metrics {
        size_t total_requests = 0;
        size_t cache_hits = 0;
        size_t evictions = 0;
        double hit_rate = 0.0;
    };
    Metrics GetMetrics() const;

    // 动态调整接口
    bool Resize(size_t new_pool_size);

    // 获取池大小
    size_t GetPoolSize() const { return pool_size_; }

private:
    // 私有辅助方法
    void PutPageToCache(int32_t page_id, std::shared_ptr<Page> page);
    void RemovePageFromCache(int32_t page_id);
    bool EvictOnePage();

    // LRU缓存实现（在实现文件中定义）
    class LRUCache;
    std::unique_ptr<LRUCache> lru_cache_;

    // 磁盘管理器指针
    DiskManager* disk_manager_;

    // 缓冲池大小
    size_t pool_size_;

    // 页面引用计数
    std::unordered_map<int32_t, int> page_refs_;

    // 脏页标记 table
    std::unordered_map<int32_t, bool> dirty_pages_;

    // 性能指标
    mutable std::mutex metrics_mutex_;
    Metrics metrics_;
};

}  // namespace sqlcc
