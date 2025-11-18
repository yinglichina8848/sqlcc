#ifndef SQLCC_BUFFER_POOL_SHARDED_H
#define SQLCC_BUFFER_POOL_SHARDED_H

#include <memory>
#include <unordered_map>
#include <list>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <vector>
#include <string>
#include <unordered_set>
#include <bitset>

#include "disk_manager.h"
#include "page.h"
#include "config_manager.h"
#include "exception.h"

namespace sqlcc {

/**
 * 基于RocksDB风格的Sharded Buffer Pool实现
 * 特点：
 * 1. 按2^n分shard，使用page_id哈希取模定位shard
 * 2. 每个shard独立LRU + 独立mutex
 * 3. 支持高并发访问
 */
class BufferPoolSharded {
public:
    /**
     * 构造函数
     * @param disk_manager 磁盘管理器实例
     * @param config_manager 配置管理器实例
     * @param pool_size 缓冲池大小
     * @param num_shards shard数量（必须是2的幂）
     */
    BufferPoolSharded(DiskManager* disk_manager, ConfigManager& config_manager, 
                     size_t pool_size, size_t num_shards = 16);

    /**
     * 析构函数
     */
    ~BufferPoolSharded();

    /**
     * 获取页面
     * @param page_id 页面ID
     * @param exclusive 是否需要独占锁
     * @return 页面对象指针，失败时返回nullptr
     */
    Page* FetchPage(int32_t page_id, bool exclusive = false);

    /**
     * 刷新页面到磁盘
     * @param page_id 页面ID
     * @return 是否刷新成功
     */
    bool FlushPage(int32_t page_id);

    /**
     * 刷新所有页面到磁盘
     */
    void FlushAllPages();

    /**
     * 删除页面
     * @param page_id 页面ID
     * @return 是否删除成功
     */
    bool DeletePage(int32_t page_id);

    /**
     * 解除页面固定
     * @param page_id 页面ID
     * @param is_dirty 是否为脏页
     * @return 是否解除成功
     */
    bool UnpinPage(int32_t page_id, bool is_dirty);

    /**
     * 创建新页面
     * @param page_id 输出参数，页面ID
     * @return 页面对象指针，失败时返回nullptr
     */
    Page* NewPage(int32_t* page_id);

    /**
     * 获取缓冲池统计信息
     * @return 统计信息哈希表
     */
    std::unordered_map<std::string, double> GetStats() const;

    /**
     * 获取缓冲池大小
     * @return 缓冲池大小
     */
    size_t GetPoolSize() const { return pool_size_; }

    /**
     * 获取当前页面数量
     * @return 当前页面数量
     */
    size_t GetCurrentPageCount() const;

private:
    // 页面对象包装类
    struct PageWrapper {
        Page* page;                          // 页面对象指针
        int ref_count;                       // 引用计数
        bool is_dirty;                       // 脏页标记
        std::list<int32_t>::iterator lru_iter; // LRU链表迭代器
        bool is_in_lru;                      // 是否在LRU链表中

        PageWrapper(Page* page_ptr = nullptr)
            : page(page_ptr), ref_count(0), is_dirty(false), is_in_lru(false) {}

        ~PageWrapper() {
            delete page;
        }
    };

    // 单个Shard的实现
    struct Shard {
        std::mutex mutex;                                // 每个shard独立的互斥锁
        std::unordered_map<int32_t, std::shared_ptr<PageWrapper>> page_table; // 页面表
        std::list<int32_t> lru_list;                      // LRU列表
        std::unordered_map<int32_t, std::list<int32_t>::iterator> lru_map; // LRU映射
        size_t current_size;                             // 当前页面数量
        size_t max_size;                                 // 最大页面数量

        Shard(size_t max_size = 0) : current_size(0), max_size(max_size) {}
    };

    // 根据页面ID获取对应的shard索引
    inline size_t GetShardIndex(int32_t page_id) const {
        // 使用哈希分片，确保是2的幂时快速取模
        return (static_cast<size_t>(page_id) & (num_shards_ - 1));
    }

    // 在指定shard中替换页面
    int32_t ReplacePage(Shard& shard);

    // 将页面移动到LRU链表头部
    void MoveToHead(Shard& shard, int32_t page_id);

    // 从LRU链表中移除页面
    void RemoveFromLRU(Shard& shard, int32_t page_id);

    // 磁盘管理器指针
    DiskManager* disk_manager_;

    // 配置管理器引用
    ConfigManager& config_manager_;

    // 缓冲池大小
    size_t pool_size_;

    // shard数量（必须是2的幂）
    size_t num_shards_;

    // shard数组
    std::vector<std::unique_ptr<Shard>> shards_;

    // 统计信息
    struct Stats {
        std::atomic<size_t> total_accesses{0};
        std::atomic<size_t> total_hits{0};
        std::atomic<size_t> total_misses{0};
        std::atomic<size_t> total_evictions{0};
    } stats_;

    // 已分配的页面集合，用于快速检查页面ID是否有效
    std::unordered_set<int32_t> allocated_pages_;
    mutable std::mutex allocated_pages_mutex_;

    // 页面ID生成器
    std::atomic<int32_t> next_page_id_;
};

}  // namespace sqlcc

#endif  // SQLCC_BUFFER_POOL_SHARDED_H