/**
 * @file buffer_pool_sharded.h
 * @brief SQLCC分段缓冲池管理器 - 高并发内存管理中枢
 *
 * BufferPoolSharded 是数据库系统的“心脏缓冲区”，负责管理磁盘页面在内存中的缓存。
 * 它通过分段（Sharding）设计，极大地缓解了多线程环境下的锁竞争压力，是系统支持
 * 高吞吐量并发访问的关键。
 *
 * 📚 配套教材参考：
 * - [第7章：缓冲区管理](../../textbook/《数据库系统原理与开发实践》.md#第七章缓冲区管理)
 * - [7.1 缓冲区管理机制](../../textbook/《数据库系统原理与开发实践》.md#71-缓冲区管理机制)
 * - [7.2 置换算法（LRU, Clock）](../../textbook/《数据库系统原理与开发实践》.md#72-置换算法lruclock)
 * - [7.3 并发访问与锁优化](../../textbook/《数据库系统原理与开发实践》.md#73-并发访问与锁优化)
 *
 * WHY层 - 设计意图：
 *   1. **消除I/O瓶颈**：通过内存缓存减少物理磁盘读写。
 *   2. **高并发支持**：传统的单锁缓冲池在多核CPU下会成为瓶颈，分段设计将锁粒度降至 1/N。
 *   3. **数据一致性**：作为 Page 的托管中心，负责协调脏页刷盘与日志同步（WAL）。
 *   4. **资源公平性**：通过 LRU 算法确保热点数据常驻内存，冷数据及时淘汰。
 *
 * WHAT层 - 功能说明：
 *   - 页面生命周期：FetchPage (从磁盘加载/内存获取), UnpinPage (释放使用权), NewPage (空间分配)。
 *   - 分段架构：通过哈希 page_id 路由到不同的 Shard，支持 2^n 个独立锁分区。
 *   - 置换策略：实现标准 LRU (Least Recently Used) 算法进行内存淘汰。
 *   - 统计监控：实时跟踪缓存命中率 (Hit Rate)、淘汰次数 (Evictions) 等关键性能指标。
 *
 * HOW层 - 实现机制：
 *   - 路由：`shard_index = page_id & (num_shards - 1)`，采用位运算代替模运算优化。
 *   - 引用计数：每个 PageWrapper 维护 ref_count，确保被“固定”的页面不会被淘汰。
 *   - 脏页管理：Unpin 时标记 is_dirty，后台线程或淘汰时触发物理刷盘。
 *   - 智能替换：当 Shard 满载时，扫描 LRU 链表，选择 ref_count=0 的页面进行牺牲。
 *
 * ⚡ 架构协作说明：
 *   - **与 WAL 协作**：在刷新脏页到磁盘前，必须确保对应的重做日志（Redo Log）已持久化（Write-Ahead Logging）。
 *   - **与 Executor 协作**：执行器通过获取独占锁（exclusive=true）来保证页面修改的原子性。
 */

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

#include "../../storage/disk_manager.h"
#include "../../page/page.h"
#include "../../utils/config_manager.h"
#include "../../exception/exception.h"
#include "buffer_pool_interface.h"

namespace sqlcc {

/**
 * @class BufferPoolSharded
 * @brief 分片缓冲池 - 现代多核数据库的标配实现
 * 
 * 实现 IBufferPool 接口，提供缓冲区管理功能
 */
class BufferPoolSharded : public IBufferPool {
public:
    /**
     * @brief 构造函数
     * @param disk_manager 存储引擎底层的磁盘读写组件
     * @param config_manager 系统配置管理器
     * @param pool_size 总内存容量（页面数）
     * @param num_shards 分片数量，必须是2的幂（如 8, 16, 32）
     */
    BufferPoolSharded(std::shared_ptr<DiskManager> disk_manager, ConfigManager& config_manager, 
                     size_t pool_size, size_t num_shards = 16);

    ~BufferPoolSharded();

    /**
     * @brief 获取页面 - 缓冲池最频繁的操作
     * 
     * WHY: 它是所有数据访问的入口。
     * HOW:
     * 1. 哈希路由到特定 Shard。
     * 2. 加锁查询哈希表。
     * 3. 命中则移动 LRU 到头部，返回。
     * 4. 未命中则申请牺牲页，从磁盘 ReadPage。
     * 
     * @param page_id 目标页面ID
     * @param exclusive 是否需要获取独占权限（用于写操作）
     * @return std::unique_ptr<Page> 页面包装后的指针
     */
    std::unique_ptr<Page> FetchPage(int32_t page_id, bool exclusive = false);

    /**
     * @brief 分配并初始化新页面
     * 
     * WHAT: 在磁盘分配 page_id 的同时，在内存中为其预留空间并固定（Pin）。
     * @param page_id [out] 输出新分配的页面ID
     */
    std::unique_ptr<Page> NewPage(int32_t* page_id);

    /**
     * @brief 强制将指定脏页刷盘
     */
    bool FlushPage(int32_t page_id);

    /**
     * @brief 关机或检查点时刷新所有内存数据
     */
    void FlushAllPages();

    /**
     * @brief 销毁页面资源
     */
    bool DeletePage(int32_t page_id);

    /**
     * @brief 释放页面使用权（解除 Pin）
     * 
     * WHY: 页面使用完必须释放，否则缓冲池会因“全固定”而无法进行新页面换入（Deadlock）。
     * WHAT: 减少引用计数，标记脏页位，并可能将其放入 LRU 淘汰序列。
     * 
     * @param page_id 目标页面ID
     * @param is_dirty 本次使用是否修改了页面内容
     */
    bool UnpinPage(int32_t page_id, bool is_dirty);

    /**
     * @brief 获取系统统计信息
     * 返回 Hit Rate, Misses, Load Factor 等。
     */
    std::unordered_map<std::string, double> GetStats() const;

    size_t GetPoolSize() const { return pool_size_; }
    size_t GetCurrentPageCount() const;

private:
    /**
     * @struct PageWrapper
     * @brief 页面容器，维护 Page 的元数据状态
     */
    struct PageWrapper {
        std::unique_ptr<Page> page;           ///< 物理页面数据
        int ref_count;                       ///< 引用计数（固定计数）
        bool is_dirty;                       ///< 是否被修改（待刷盘）
        std::list<int32_t>::iterator lru_iter; ///< 在 LRU 链表中的位置
        bool is_in_lru;                      ///< 标记是否处于可置换状态

        PageWrapper(std::unique_ptr<Page> page_ptr = nullptr)
            : page(std::move(page_ptr)), ref_count(0), is_dirty(false), is_in_lru(false) {}
    };

    /**
     * @struct Shard
     * @brief 独立的分片分区，拥有自己的锁和管理结构
     */
    struct Shard {
        std::mutex mutex;                                ///< 分片级互斥锁
        std::unordered_map<int32_t, std::shared_ptr<PageWrapper>> page_table; ///< 逻辑ID到包装器的映射
        std::list<int32_t> lru_list;                      ///< LRU 置换链表
        std::unordered_map<int32_t, std::list<int32_t>::iterator> lru_map; ///< 加速 LRU 定位
        size_t current_size;                             ///< 本分片承载的页面数
        size_t max_size;                                 ///< 本分片额定容量

        Shard(size_t max_size = 0) : current_size(0), max_size(max_size) {}
    };

    /**
     * @brief 快速哈希路由
     * 要求 num_shards_ 必须是 2 的幂，利用 & 位运算代替 %。
     */
    inline size_t GetShardIndex(int32_t page_id) const {
        return (static_cast<size_t>(page_id) & (num_shards_ - 1));
    }

    /**
     * @brief 页面置换算法核心
     * 从 Shard 中挑选一个牺牲者并移除。
     */
    int32_t ReplacePage(Shard& shard);

    void MoveToHead(Shard& shard, int32_t page_id);
    void RemoveFromLRU(Shard& shard, int32_t page_id);

    std::shared_ptr<DiskManager> disk_manager_;
    ConfigManager& config_manager_;
    size_t pool_size_;
    size_t num_shards_;
    std::vector<std::unique_ptr<Shard>> shards_;

    /**
     * @struct Stats
     * @brief 原子性能统计指标
     */
    struct Stats {
        std::atomic<size_t> total_accesses{0};
        std::atomic<size_t> total_hits{0};
        std::atomic<size_t> total_misses{0};
        std::atomic<size_t> total_evictions{0};
    } stats_;

    std::unordered_set<int32_t> allocated_pages_;
    mutable std::mutex allocated_pages_mutex_;
    std::atomic<int32_t> next_page_id_;
};

}  // namespace sqlcc

#endif  // SQLCC_BUFFER_POOL_SHARDED_H

