#pragma once

#include "page.h"
#include "disk_manager.h"
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <deque>
#include <list>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <thread>

#include "disk_manager.h"
#include "page.h"
#include "config_manager.h"
#include "exception.h"

/**
 * =================================================================================================
 * 商业数据库核心技术：缓冲池(Buffer Pool)设计思想深度剖析
 * =================================================================================================
 *
 * WHY (为什么缓冲池是数据库的核心创新):
 * --------------------------------------------------------------------------------------
 *   商用数据库面临的核心挑战：
 *   1. 内存-磁盘性能差距(DRAM vs HDD：10^6倍差距)
 *   2. I/O密集型工作负载(OLTP/OLAP混合场景)
 *   3. 高并发访问控制(成千上万并发连接)
 *   4. 可预测性性能要求(高SLA承诺)
 *
 *   缓冲池的设计哲学：
 *   - 内存即缓存：将随机磁盘访问转换为内存访问
 *   - 智能预取：预测性加载减少等待时间
 *   - 自适应策略：动态调整适应工作负载变化
 *   - 故障容错：WAL+Checkpoint保证数据一致性
 *
 * WHAT (缓冲池如何重新定义数据库性能):
 * --------------------------------------------------------------------------------------
 *   核心机制：
 *   1. 页面缓存：将磁盘页面映射到内存缓冲区
 *   2. LRU替换：最近最少使用算法选择淘汰页面
 *   3. 脏页管理：延迟写入+批量刷新优化I/O
 *   4. 并发保护：细粒度锁+读写分离设计
 *
 *   商业价值实现：
 *   - 查询加速：内存命中率>99%的常态化要求
 *   - 吞吐量提升：每秒数万TPS的并发处理能力
 *   - 延迟控制：毫秒级响应确保用户体验
 *   - 资源优化：有限内存发挥最大价值
 *
 * HOW (缓冲池在商业数据库中的科技创新):
 * --------------------------------------------------------------------------------------
 *   1. 多层缓冲架构：
 *      - L1缓存：热点页面常驻内存
 *      - L2扩展：SSD缓存补充DRAM
 *      - 预取机制：智能预测未来访问
 *      - 压缩存储：T储更多数据页面
 *
 *   2. 预取策略优化：
 *      - 空间局部性：连续页面批量预取
 *      - 时间局部性：热点页面优先缓存
 *      - 用户行为预测：基于访问模式的预取
 *      - 自适应调整：实时学习工作负载特征
 *
 *   3. 并发控制创新：
 *      - 分页锁机制：页面粒度的读写锁
 *      - 乐观并发：版本戳控制并发冲突
 *      - 无锁预取：异步预取避免阻塞
 *      - 锁升级策略：动态平衡粒度与性能
 *
 *   4. 资源管理策略：
 *      - 动态调整：根据负载实时调整缓存大小
 *      - 多租户隔离：不同应用间的资源分配
 *      - QoS保证：确保高优先级查询的性能
 *      - 故障恢复：快速故障转移保证可用性
 *
 * WHY 缓冲池是数据库性能的决定性因素：
 * --------------------------------------------------------------------------------------
 *   1. I/O性能决定论：
 *      - 随便访问磁盘：5-10ms延迟随机I/O
 *      - 缓存池命中：纳秒级内存访问
 *      - 吞吐量差距：高达10^5倍的性能提升
 *
 *   2. 可扩展性保障：
 *      - 分层存储：DRAM+SSD+HDD异构架构
 *      - 智能路由：热点数据优先内存存储
 *      - 自适应伸缩：自动化容量调整
 *      - 云原生支持：Elastic扩容能力
 *
 *   3. 并发处理极限：
 *      - 无锁预取：前台查询不阻塞预取
 *      - 批处理优化：批量I/O减少系统调用开销
 *      - 异步刷新：后台写入不影响前端响应
 *      - 读写分离：分离读写操作提高并发度
 *
 *   4. 稳定性核心：
 *      - Checkpoint机制：定期持久化一致状态
 *      - 脏页延迟：减少同步写入的阻塞时间
 *      - Buffer分配控制：防止大查询占用过多缓存
 *      - 死锁检测：自动检测并解决死锁问题
 *
 * =================================================================================================
 * 缓冲池性能指标的商业级要求
 * =================================================================================================
 *   1. 命中率目标：
 *      - OLTP场景：>99.5%的命中率
 *      - OLAP场景：根据数据热点特点优化
 *      - 实时监控：秒级监控命中率波动
 *      - 告警机制：低命中率实时告警
 *
 *   2. 延迟控制：
 *      - p95延迟：<10ms (数据在缓存中)
 *      - p99延迟：<50ms (允许极少数磁盘访问)
 *      - 预取优化：预测性预取减少冷启动时间
 *
 *   3. 吞吐量目标：
 *      - 内存优化：数万TPS的稳定吞吐
 *      - I/O优化：磁盘访问时的瓶颈优化
 *      - 批量处理：减少上下文切换开销
 *
 *   4. 资源效率：
 *      - 内存利用：最大化有限内存的价值
 *      - I/O带宽：平衡读写操作的资源分配
 *      - CPU开销：最小化缓冲池的管理开销
 * =================================================================================================
 */
namespace sqlcc {

// 缓冲池类，负责管理内存中的页面缓存
// Why: 数据库系统需要高效的内存管理机制，缓冲池作为内存和磁盘之间的桥梁，减少磁盘I/O操作
// What: BufferPool类实现了基于LRU算法的页面缓存系统，管理页面的加载、替换和写入
// How: 使用哈希表快速查找页面，双向链表实现LRU算法，通过引用计数和脏页标记管理页面生命周期
class BufferPool {
public:
    // 构造函数，初始化缓冲池
    // Why: 需要创建缓冲池实例，设置缓冲池大小和关联的磁盘管理器
    // What: 构造函数接收磁盘管理器指针、缓冲池大小和配置管理器引用，初始化缓冲池状态
    // How: 设置成员变量，初始化页面表和LRU列表，创建互斥锁，条件注册配置回调
    explicit BufferPool(DiskManager* disk_manager, size_t pool_size, ConfigManager& config_manager);
    
    // 删除拷贝构造函数和赋值运算符，防止意外拷贝
    BufferPool(const BufferPool&) = delete;
    BufferPool& operator=(const BufferPool&) = delete;

    // 析构函数，清理资源
    // Why: 需要释放缓冲池占用的资源，确保所有脏页被写入磁盘
    // What: 析构函数负责刷新所有脏页到磁盘，释放页面资源
    // How: 调用FlushAllPages方法确保数据持久化，释放所有页面对象
    ~BufferPool();

    // 获取页面，如果页面不在缓冲池中则从磁盘加载
    // Why: 数据库操作需要访问页面数据，需要从磁盘加载到内存
    // What: FetchPage方法根据页面ID获取页面对象，如果页面不在内存中则从磁盘加载
    // How: 首先在页面表中查找页面，如果找到则更新LRU列表并返回；如果未找到则从磁盘加载
    Page* FetchPage(int32_t page_id);

    // 批量获取页面，优化多个页面的加载性能
    // Why: 某些操作需要同时访问多个页面，批量加载可以提高性能
    // What: BatchFetchPages方法根据页面ID列表获取多个页面对象
    // How: 对每个页面调用FetchPage方法，或者实现更高效的批量加载策略
    std::vector<Page*> BatchFetchPages(const std::vector<int32_t>& page_ids);

    // 创建新页面
    // Why: 数据库需要新的存储空间来存储数据，例如插入新记录或创建索引
    // What: NewPage方法在缓冲池中分配一个新的页面
    // How: 从空闲页面列表中获取页面，或者替换一个现有页面，初始化页面数据
    Page* NewPage(int32_t* page_id);

    // 取消固定页面，减少页面的固定计数
    // Why: 当页面使用完毕后，需要通知缓冲池该页面可以被替换
    // What: UnpinPage方法减少页面的固定计数，如果页面被修改则标记为脏页
    // How: 减少页面的引用计数，如果引用计数为0且页面被修改，则标记为脏页
    bool UnpinPage(int32_t page_id, bool is_dirty);

    // 刷新页面到磁盘
    // Why: 需要将修改后的页面数据持久化到磁盘，保证数据的持久性和一致性
    // What: FlushPage方法将指定页面的数据写入磁盘文件
    // How: 调用磁盘管理器的WritePage方法，将页面数据写入磁盘
    bool FlushPage(int32_t page_id);

    // 刷新所有页面到磁盘
    // Why: 在系统关闭或检查点时，需要将所有修改过的页面写入磁盘，保证数据持久性
    // What: FlushAllPages方法将缓冲池中所有脏页写入磁盘
    // How: 遍历所有页面，对脏页调用FlushPage方法
    void FlushAllPages();

    // 删除页面
    // Why: 当数据不再需要时，需要释放页面空间，例如删除记录或索引
    // What: DeletePage方法从缓冲池中删除指定页面
    // How: 从页面表中移除页面，释放页面对象
    bool DeletePage(int32_t page_id);

    // 预取页面到缓冲池
    // Why: 预取可以提前加载可能需要的页面，减少未来的磁盘I/O延迟
    // What: PrefetchPage方法将指定页面预加载到缓冲池
    // How: 类似FetchPage，但不增加页面的固定计数
    bool PrefetchPage(int32_t page_id);

    // 批量预取页面到缓冲池
    // Why: 批量预取可以一次性加载多个页面，提高预取效率
    // What: BatchPrefetchPages方法将多个页面预加载到缓冲池
    // How: 对每个页面调用PrefetchPage方法，或者实现更高效的批量预取策略
    bool BatchPrefetchPages(const std::vector<int32_t>& page_ids);

    // 获取缓冲池使用统计信息
    // Why: 监控缓冲池的使用情况有助于性能调优和问题诊断
    // What: GetStats方法返回缓冲池的统计信息，如命中率等
    // How: 收集并返回各种统计指标
    std::unordered_map<std::string, double> GetStats() const;

    // 获取缓冲池大小
    // Why: 需要了解缓冲池的容量信息，用于监控和调试
    // What: GetPoolSize方法返回缓冲池的页面容量
    // How: 直接返回pool_size_成员变量
    size_t GetPoolSize() const;

    // 获取已使用页面数
    // Why: 需要了解缓冲池的使用情况，用于监控和调试
    // What: GetUsedPages方法返回当前已使用的页面数量
    // How: 返回页面表的大小
    size_t GetUsedPages() const;

    // 检查页面是否在缓冲池中
    // Why: 需要快速判断页面是否已加载到内存中，避免不必要的磁盘I/O
    // What: IsPageInBuffer方法检查指定页面ID是否存在于缓冲池中
    // How: 检查页面表是否包含该页面ID
    bool IsPageInBuffer(int32_t page_id) const;

    // 设置是否模拟刷新失败（仅用于测试）
    // Why: 需要测试缓冲池在磁盘写入失败时的错误处理逻辑
    // What: SetSimulateFlushFailure方法设置是否模拟刷新失败
    // How: 设置simulate_flush_failure_成员变量，影响FlushPage方法的行为
    void SetSimulateFlushFailure(bool simulate) { simulate_flush_failure_ = simulate; }
    
    // 设置是否启用配置回调（仅用于测试）
    // Why: 需要在测试中禁用配置回调，避免异步线程导致死锁或死循环
    // What: SetEnableConfigCallback方法控制是否启用配置变更回调
    // How: 设置enable_config_callback_成员变量，影响构造函数的回调注册行为// 注意：配置回调功能已禁用，此方法保留以保持接口兼容性
    void SetEnableConfigCallback(bool /*enable*/) {}

private:
    // 查找可替换的页面
    // Why: 当缓冲池满时，需要选择一个页面进行替换
    // What: FindVictimPage方法根据LRU算法找到可替换的页面
    // How: 从LRU列表的尾部开始查找，找到引用计数为0的页面
    int32_t FindVictimPage();

    // 替换页面
    // Why: 当需要加载新页面但缓冲池已满时，需要替换一个现有页面
    // What: ReplacePage方法替换指定页面，将旧页面写回磁盘（如果脏）
    // How: 将旧页面写回磁盘，更新页面数据，重新插入LRU列表
    bool ReplacePage(int32_t victim_page_id, int32_t new_page_id);

    // 更新LRU列表
    // Why: LRU算法需要维护页面的访问顺序
    // What: UpdateLRUList方法将页面移动到LRU列表的头部
    // How: 从LRU列表中移除页面，然后插入到头部
    void UpdateLRUList(int32_t page_id);

    // 移动页面到LRU链表头部
    // Why: 当页面被访问时，需要将其移动到LRU链表头部，表示最近被访问
    // What: MoveToHead方法将指定页面移动到LRU链表头部，更新LRU映射
    // How: 从LRU链表中删除页面，然后将其添加到头部，并更新LRU映射中的迭代器
    void MoveToHead(int32_t page_id);

    // 从LRU列表中移除页面
    // Why: 当页面被替换或删除时，需要从LRU列表中移除
    // What: RemoveFromLRUList方法从LRU列表中移除指定页面
    // How: 在LRU列表中查找页面并移除
    void RemoveFromLRUList(int32_t page_id);

    // 替换页面（无锁版本）
    // Why: 需要在已有锁的情况下替换页面，避免死锁
    // What: ReplacePageInternal方法在持锁状态下替换页面
    // How: 直接操作LRU列表和页面表，不重新获取锁
    int32_t ReplacePageInternal();

    // 替换页面
    // Why: 当缓冲池已满且需要加载新页面时，必须选择一个现有页面进行替换
    // What: ReplacePage方法使用LRU算法选择一个引用计数为0的页面进行替换
    // How: 从LRU链表尾部开始查找，找到第一个引用计数为0的页面
    int32_t ReplacePage();

    // 配置变更回调处理
    // Why: 需要响应配置变更，动态调整缓冲池行为
    // What: OnConfigChange方法处理配置变更事件
    // How: 根据变更的配置项调整相应的缓冲池参数
    void OnConfigChange(const std::string& key, const ConfigValue& value);

    // 调整缓冲池大小
    // Why: 需要动态调整缓冲池的大小以适应不同的负载需求
    // What: AdjustBufferPoolSize方法调整缓冲池大小
    // How: 在持锁状态下移除多余的页面或标记容量变更
    void AdjustBufferPoolSize(size_t new_pool_size);

    // 安全调整缓冲池大小（无锁版本）
    // Why: 需要在不需要获取锁的情况下调整缓冲池大小
    // What: AdjustBufferPoolSizeNoLock方法调整缓冲池大小
    // How: 通过发送消息到队列的方式触发异步调整，不直接获取锁
    void AdjustBufferPoolSizeNoLock(size_t new_pool_size);

    // 磁盘管理器指针，负责磁盘I/O操作
    // Why: 缓冲池需要与磁盘管理器交互，进行页面的读写操作
    // What: disk_manager_指向DiskManager对象，提供磁盘I/O接口
    // How: 通过构造函数初始化，在需要读写页面时调用相应方法
    DiskManager* disk_manager_;

    // 配置管理器引用，用于获取配置参数
    // Why: 缓冲池需要从配置管理器获取配置参数，如预取策略等
    // What: config_manager_引用ConfigManager对象，提供配置访问接口
    // How: 通过构造函数初始化，在需要获取配置时调用相应方法
    ConfigManager& config_manager_;

    // 缓冲池大小，表示可以缓存的页面数量
    // Why: 需要限制缓冲池的容量，避免内存溢出
    // What: pool_size_表示缓冲池可以容纳的最大页面数
    // How: 通过构造函数初始化，在创建新页面时检查是否超出容量
    size_t pool_size_;

    // 页面表，存储页面ID到页面对象的映射
    // Why: 需要快速查找页面，避免遍历整个缓冲池
    // What: page_table_是哈希表，键为页面ID，值为页面对象指针
    // How: 使用unordered_map实现，提供O(1)的平均查找时间
    std::unordered_map<int32_t, Page*> page_table_;

    // LRU列表，存储页面的访问顺序
    // Why: 需要根据页面的访问时间选择替换页面
    // What: lru_list_是双向链表，存储页面ID，最近访问的页面在头部
    // How: 使用list实现，每次访问页面时将其移动到头部
    std::list<int32_t> lru_list_;

    // LRU列表迭代器映射，快速定位页面在LRU列表中的位置
    // Why: 需要快速找到页面在LRU列表中的位置，避免遍历整个列表
    // What: lru_map_是哈希表，键为页面ID，值为页面在LRU列表中的迭代器
    // How: 使用unordered_map实现，提供O(1)的平均查找时间
    std::unordered_map<int32_t, std::list<int32_t>::iterator> lru_map_;

    // 互斥锁，保护缓冲池的并发访问
    // Why: 缓冲池可能被多个线程同时访问，需要同步机制保证数据一致性
    // What: latch_是定时互斥锁，保护所有共享数据的访问，并支持超时机制避免死锁
    // How: 在访问共享数据前加锁，访问完成后解锁
    mutable std::timed_mutex latch_;

    // 统计信息，记录缓冲池的使用情况
    // Why: 监控缓冲池的使用情况有助于性能调优和问题诊断
    // What: stats_记录各种统计指标，如访问次数、命中次数等
    // How: 在相应操作时更新统计信息
    struct Stats {
        size_t total_accesses = 0;     // 总访问次数
        size_t total_hits = 0;          // 命中次数
        size_t total_misses = 0;        // 未命中次数
        size_t total_evictions = 0;     // 替换次数
        size_t total_prefetches = 0;    // 预取次数
        size_t prefetch_hits = 0;       // 预取命中次数
    } stats_;

    // 页面引用计数表，存储页面ID到引用计数的映射
    // Why: 需要跟踪每个页面被多少个操作引用，防止正在使用的页面被替换
    // What: page_refs_是哈希表，键为页面ID，值为引用计数
    // How: 使用unordered_map实现，提供O(1)的平均查找时间
    std::unordered_map<int32_t, int> page_refs_;

    // 脏页标记表，存储页面ID到脏页标记的映射
    // Why: 需要跟踪哪些页面被修改过，以便在替换时写回磁盘
    // What: dirty_pages_是哈希表，键为页面ID，值为布尔值，表示是否为脏页
    // How: 使用unordered_map实现，提供O(1)的平均查找时间
    std::unordered_map<int32_t, bool> dirty_pages_;

    // 预取队列，存储待预取的页面ID
    // Why: 需要维护一个预取队列，用于异步预取可能需要的页面
    // What: prefetch_queue_是双端队列，存储待预取的页面ID
    // How: 使用deque实现，支持在队头和队尾的高效插入和删除
    std::deque<int32_t> prefetch_queue_;

    // 页面访问统计，用于预测性预取
    // Why: 需要跟踪页面的访问模式，以便进行智能预取
    // What: access_stats_是哈希表，键为页面ID，值为访问次数
    // How: 使用unordered_map实现，提供O(1)的平均查找时间
    std::unordered_map<int32_t, int> access_stats_;

    // 批量操作缓冲区，用于批量读写操作
    // Why: 批量操作可以提高I/O性能，需要预分配缓冲区空间
    // What: batch_buffer_是向量，存储批量操作的数据缓冲区
    // How: 使用vector实现，在构造函数中预分配空间
    std::vector<char*> batch_buffer_;

    // 模拟刷新失败标志，用于测试
    // Why: 需要测试缓冲池在磁盘写入失败时的错误处理逻辑
    // What: simulate_flush_failure_是布尔值，控制是否模拟刷新失败
    // How: 当设置为true时，FlushPage方法会模拟写入失败
    bool simulate_flush_failure_;
    
    // 锁超时时间（毫秒）
    // Why: 需要限制锁获取的等待时间，避免死锁导致的长时间阻塞
    // What: 不同类型操作的锁超时时间
    // How: 从配置管理器获取或使用默认值，在获取锁时使用try_lock_for方法
    size_t read_lock_timeout_ms_;    // 读取操作的锁超时时间
    size_t write_lock_timeout_ms_;   // 写入和修改操作的锁超时时间
    size_t lock_timeout_ms_;         // 默认锁超时时间（用于其他操作）
};

}  // namespace sqlcc
