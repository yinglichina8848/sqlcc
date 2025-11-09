#pragma once

#include "page.h"
#include "disk_manager.h"
#include <unordered_map>
#include <list>
#include <memory>
#include <unordered_set>
#include <vector>
#include <queue>
#include <deque>

namespace sqlcc {

// 缓冲池默认大小
// Why: 设置一个合理的默认值，平衡内存使用和性能
// What: 默认缓冲池可以容纳64个页面，每个页面8KB，总共约512KB内存
// How: 使用constexpr定义编译时常量，提高性能
static constexpr size_t DEFAULT_BUFFER_POOL_SIZE = 64;

/**
 * @brief 缓冲池管理器类，实现LRU页面替换算法
 * 
 * Why: 数据库系统需要频繁访问磁盘上的数据，但磁盘I/O速度远慢于内存访问。
 *      为了减少磁盘I/O次数，提高数据库性能，需要在内存中缓存经常访问的数据页面。
 *      缓冲池是数据库系统的核心组件，负责管理内存中的数据页面与磁盘之间的数据交换。
 * 
 * What: BufferPool类实现了一个基于LRU(Least Recently Used)算法的缓冲池管理器。
 *       它维护一个固定大小的内存区域，用于存储从磁盘读取的数据页面。
 *       当缓冲池已满且需要加载新页面时，会选择最近最少使用的页面进行替换。
 * 
 * How: 使用哈希表(page_table_)快速查找页面，使用双向链表(lru_list_)实现LRU算法。
 *      通过引用计数(page_refs_)跟踪页面的使用情况，通过脏页标记(dirty_pages_)
 *      跟踪哪些页面被修改过，需要在替换时写回磁盘。
 */
class BufferPool {
public:
    /**
     * @brief 构造函数
     * 
     * Why: 需要初始化缓冲池的基本状态，包括设置磁盘管理器、分配内存空间等。
     * 
     * @param disk_manager 磁盘管理器指针，用于读写磁盘页面
     *                    What: 磁盘管理器负责处理底层的文件I/O操作
     *                    How: 通过调用磁盘管理器的ReadPage和WritePage方法实现页面读写
     * 
     * @param pool_size 缓冲池大小（页面数量）
     *                  What: 指定缓冲池可以容纳的页面数量
     *                  How: 默认值为DEFAULT_BUFFER_POOL_SIZE(64)，可根据系统内存大小调整
     */
    BufferPool(DiskManager* disk_manager, size_t pool_size = DEFAULT_BUFFER_POOL_SIZE);

    /**
     * @brief 析构函数
     * 
     * Why: 在对象销毁时需要释放资源，确保所有脏页被写回磁盘，避免数据丢失。
     * 
     * What: 析构函数会调用FlushAllPages()方法，将所有脏页写回磁盘。
     * 
     * How: 遍历所有页面，检查脏页标记，将脏页写回磁盘，然后释放内存。
     */
    ~BufferPool();

    /**
     * @brief 禁止拷贝构造
     * 
     * Why: BufferPool管理着重要的系统资源，允许拷贝可能导致资源重复释放或内存泄漏。
     * 
     * What: 通过将拷贝构造函数标记为delete，防止编译器自动生成拷贝构造函数。
     * 
     * How: 使用= delete语法显式禁用拷贝构造函数。
     */
    BufferPool(const BufferPool&) = delete;

    /**
     * @brief 禁止赋值操作
     * 
     * Why: 同拷贝构造函数，赋值操作也可能导致资源管理问题。
     * 
     * What: 禁用赋值操作符，防止对象间的赋值。
     * 
     * How: 使用= delete语法显式禁用赋值操作符。
     */
    BufferPool& operator=(const BufferPool&) = delete;

    /**
     * @brief 获取页面
     * 
     * Why: 数据库操作需要访问特定页面ID的数据，这是缓冲池最核心的功能。
     *      如果页面不在内存中，需要从磁盘加载；如果已在内存中，直接返回。
     * 
     * What: FetchPage方法根据页面ID获取对应的页面指针。
     *       如果页面不在缓冲池中，会从磁盘加载；如果缓冲池已满，
     *       会使用LRU算法替换一个页面。
     * 
     * How: 1. 首先检查页面是否在缓冲池中(page_table_)
     *      2. 如果在，增加引用计数，移到LRU链表头部，返回页面指针
     *      3. 如果不在，检查缓冲池是否已满
     *      4. 如果已满，调用ReplacePage()替换一个页面
     *      5. 从磁盘加载新页面，加入缓冲池，返回页面指针
     * 
     * @param page_id 页面ID，唯一标识一个数据页面
     *                What: 页面ID是数据库中页面的唯一标识符
     *                How: 通常是一个整数，由磁盘管理器分配和管理
     * 
     * @return 页面指针，如果页面不存在返回nullptr
     *         What: 返回指向页面数据的指针，可以直接读写页面内容
     *         How: 如果页面加载失败或内存不足，返回nullptr
     */
    Page* FetchPage(int32_t page_id);
    
    /**
     * @brief 批量从缓冲池获取多个页面，优化连续访问性能
     * @param page_ids 页面ID数组
     * @return 页面对象指针数组
     * 
     * Why: 批量操作可以减少锁竞争和函数调用开销，提高连续页面访问的性能
     * What: BatchFetchPages方法一次性获取多个页面，对连续页面进行优化排序
     * How: 按页面ID排序以优化磁盘访问，批量处理不在缓冲池中的页面
     */
    std::vector<Page*> BatchFetchPages(const std::vector<int32_t>& page_ids);
    
    /**
     * @brief 预取指定页面到缓冲池，但不增加引用计数
     * @param page_id 页面ID
     * 
     * Why: 预取可以提前加载可能需要的页面，减少未来的I/O等待时间
     * What: PrefetchPage方法将页面加载到缓冲池中，但不增加引用计数
     * How: 类似FetchPage但不增加引用计数，适用于预测性加载
     */
    void PrefetchPage(int32_t page_id);
    
    /**
     * @brief 批量预取多个页面到缓冲池
     * @param page_ids 页面ID数组
     * 
     * Why: 批量预取可以进一步减少I/O开销，特别适用于顺序访问模式
     * What: BatchPrefetchPages方法一次性预取多个页面，优化磁盘访问模式
     * How: 按页面ID排序以优化磁盘访问，批量处理不在缓冲池中的页面
     */
    void BatchPrefetchPages(const std::vector<int32_t>& page_ids);

    /**
     * @brief 取消固定页面（减少引用计数）
     * 
     * Why: 当一个页面使用完毕后，需要通知缓冲池不再使用该页面，
     *      以便缓冲池可以正确管理页面的生命周期和LRU顺序。
     * 
     * What: UnpinPage方法减少页面的引用计数，并标记页面是否被修改。
     *       当引用计数降为0时，页面可以被LRU算法替换。
     * 
     * How: 1. 检查页面是否在缓冲池中
     *      2. 减少页面的引用计数
     *      3. 如果is_dirty为true，标记页面为脏页
     *      4. 如果引用计数降为0，将页面移到LRU链表尾部
     * 
     * @param page_id 页面ID，要取消固定的页面
     * @param is_dirty 页面是否被修改，true表示页面内容被修改过
     *                What: 脏页标记表示页面内容与磁盘上的版本不一致
     *                How: 脏页需要在被替换前写回磁盘，以保证数据持久性
     * 
     * @return 操作成功返回true，否则返回false
     *         What: 表示操作是否成功执行
     *         How: 如果页面不存在或引用计数已经为0，返回false
     */
    bool UnpinPage(int32_t page_id, bool is_dirty);

    /**
     * @brief 刷新页面到磁盘
     * 
     * Why: 为了确保持久性，被修改过的页面(脏页)需要及时写回磁盘。
     *      这在检查点操作或系统关闭时尤为重要。
     * 
     * What: FlushPage方法将指定页面写回磁盘，无论其引用计数是多少。
     *       写入成功后，清除脏页标记。
     * 
     * How: 1. 检查页面是否在缓冲池中
     *      2. 如果是脏页，调用磁盘管理器的WritePage方法写回磁盘
     *      3. 清除脏页标记
     *      4. 返回操作结果
     * 
     * @param page_id 页面ID，要刷新的页面
     * 
     * @return 操作成功返回true，否则返回false
     *         What: 表示页面是否成功写回磁盘
     *         How: 如果页面不存在或写入失败，返回false
     */
    bool FlushPage(int32_t page_id);

    /**
     * @brief 分配新页面
     * 
     * Why: 当数据库需要存储新数据时，需要分配一个新的页面来存储这些数据。
     *      这是数据库扩展存储空间的基础操作。
     * 
     * What: NewPage方法分配一个新的页面，并返回其页面ID和页面指针。
     *       新页面会被初始化为全零，并加入缓冲池管理。
     * 
     * How: 1. 调用磁盘管理器分配一个新的页面ID
     *      2. 检查缓冲池是否已满
     *      3. 如果已满，调用ReplacePage()替换一个页面
     *      4. 创建新页面对象，初始化为全零
     *      5. 将新页面加入缓冲池，设置引用计数为1
     *      6. 返回页面ID和页面指针
     * 
     * @param[out] page_id 新分配的页面ID，通过输出参数返回
     *                   What: 新页面的唯一标识符
     *                   How: 由磁盘管理器分配，确保唯一性
     * 
     * @return 新页面指针，分配失败返回nullptr
     *         What: 指向新分配页面的指针，可以直接读写页面内容
     *         How: 如果内存不足或磁盘空间不足，返回nullptr
     */
    Page* NewPage(int32_t* page_id);

    /**
     * @brief 释放页面
     * 
     * Why: 当数据不再需要时，应释放占用的页面空间，以便其他数据可以使用。
     *      这是数据库回收存储空间的重要操作。
     * 
     * What: DeletePage方法从缓冲池中移除指定页面，并通知磁盘管理器回收该页面。
     *       如果页面是脏页，会先写回磁盘再释放。
     * 
     * How: 1. 检查页面是否在缓冲池中
     *      2. 如果是脏页，先写回磁盘
     *      3. 从缓冲池中移除页面
     *      4. 通知磁盘管理器回收页面ID
     *      5. 返回操作结果
     * 
     * @param page_id 页面ID，要释放的页面
     * 
     * @return 操作成功返回true，否则返回false
     *         What: 表示页面是否成功释放
     *         How: 如果页面不存在或引用计数不为0，返回false
     */
    bool DeletePage(int32_t page_id);

    /**
     * @brief 刷新所有页面到磁盘
     * 
     * Why: 在系统关闭或检查点操作时，需要确保所有内存中的修改都被持久化到磁盘。
     *      这是保证数据库ACID特性中持久性的重要操作。
     * 
     * What: FlushAllPages方法遍历缓冲池中的所有页面，将脏页写回磁盘。
     *       这个操作通常在系统关闭或定期检查点时调用。
     * 
     * How: 1. 遍历page_table_中的所有页面
     *      2. 对于每个脏页，调用磁盘管理器的WritePage方法写回磁盘
     *      3. 清除所有脏页标记
     *      4. 返回操作结果
     */
    void FlushAllPages();

private:
    /**
     * @brief 替换页面（LRU算法）
     * 
     * Why: 当缓冲池已满且需要加载新页面时，必须选择一个现有页面进行替换。
     *      LRU算法选择最近最少使用的页面，这是基于局部性原理的有效策略。
     * 
     * What: ReplacePage方法实现LRU页面替换算法，选择一个可替换的页面。
     *       优先选择引用计数为0且不在LRU链表头部的页面。
     * 
     * How: 1. 从LRU链表尾部开始查找
     *      2. 找到第一个引用计数为0的页面
     *      3. 如果该页面是脏页，先写回磁盘
     *      4. 从缓冲池中移除该页面
     *      5. 返回被替换的页面ID
     *      6. 如果没有可替换的页面，返回-1
     * 
     * @return 被替换的页面ID，如果无法替换返回-1
     *         What: 被替换页面的唯一标识符
     *         How: 如果所有页面都在使用中(引用计数>0)，返回-1表示无法替换
     */
    int32_t ReplacePage();

    /**
     * @brief 将页面移到LRU链表头部
     * 
     * Why: LRU算法需要维护页面的访问顺序，最近访问的页面应该放在链表头部。
     *      这样当需要替换页面时，链表尾部的页面就是最近最少使用的。
     * 
     * What: MoveToHead方法将指定页面移到LRU链表的头部，表示最近被访问过。
     *       这是LRU算法的核心操作之一。
     * 
     * How: 1. 从LRU链表中移除指定页面
     *      2. 将该页面添加到链表头部
     *      3. 更新lru_map_中该页面的迭代器
     * 
     * @param page_id 页面ID，要移动的页面
     *                What: 要移动到LRU链表头部的页面ID
     *                How: 页面必须在缓冲池中，否则操作无效
     */
    void MoveToHead(int32_t page_id);

    // 磁盘管理器指针
    // Why: 缓冲池需要与磁盘交互，读写页面数据
    // What: disk_manager_指向负责磁盘I/O的DiskManager对象
    // How: 通过调用disk_manager_->ReadPage()和disk_manager_->WritePage()实现页面读写
    DiskManager* disk_manager_;

    // 缓冲池大小
    // Why: 需要知道缓冲池可以容纳多少页面，以便在满时进行替换
    // What: pool_size_表示缓冲池可以容纳的页面数量
    // How: 在构造函数中初始化，运行时不变
    size_t pool_size_;

    // 页面映射表：page_id -> page指针
    // Why: 需要快速根据页面ID查找对应的页面对象
    // What: page_table_是一个哈希表，键是页面ID，值是页面对象的智能指针
    // How: 使用std::unordered_map实现O(1)时间复杂度的查找
    std::unordered_map<int32_t, std::unique_ptr<Page>> page_table_;

    // 脏页标记：page_id -> 是否脏页
    // Why: 需要跟踪哪些页面被修改过，以便在替换时写回磁盘
    // What: dirty_pages_是一个哈希表，记录哪些页面是脏页
    // How: 当页面被修改时设置标记，当页面写回磁盘时清除标记
    std::unordered_map<int32_t, bool> dirty_pages_;

    // 页面引用计数：page_id -> 引用计数
    // Why: 需要跟踪页面被多少个操作引用，防止正在使用的页面被替换
    // What: page_refs_是一个哈希表，记录每个页面的引用计数
    // How: 当获取页面时增加计数，当取消固定页面时减少计数
    std::unordered_map<int32_t, int32_t> page_refs_;

    // LRU链表，存储页面ID
    // Why: LRU算法需要一个数据结构来维护页面的访问顺序
    // What: lru_list_是一个双向链表，头部是最近访问的页面，尾部是最久未访问的页面
    // How: 使用std::list实现，支持O(1)时间复杂度的插入和删除
    std::list<int32_t> lru_list_;

    // LRU映射：page_id -> lru_list中的迭代器
    // Why: 需要快速在LRU链表中找到指定页面的位置
    // What: lru_map_是一个哈希表，键是页面ID，值是对应页面在LRU链表中的迭代器
    // How: 使用std::unordered_map实现O(1)时间复杂度的查找
    std::unordered_map<int32_t, std::list<int32_t>::iterator> lru_map_;
    
    // 性能优化：页面访问统计，用于预测性预取
    // Why: 跟踪页面访问模式可以帮助预测未来可能访问的页面
    // What: access_stats_成员变量记录页面访问频率和顺序
    // How: 使用哈希表存储页面ID到访问统计信息的映射
    std::unordered_map<int32_t, int> access_stats_;
    
    // 性能优化：预取队列，存储待预取的页面ID
    // Why: 使用队列管理预取请求，避免预取操作影响主要操作的性能
    // What: prefetch_queue_成员变量是待预取页面ID的队列
    // How: 使用std::deque实现，支持双端操作，便于检查重复项
    std::deque<int32_t> prefetch_queue_;
    
    // 性能优化：批量操作缓冲区，用于批量读取页面
    // Why: 批量读取可以减少I/O操作次数，提高磁盘访问效率
    // What: batch_buffer_成员变量是临时存储批量读取页面的缓冲区
    // How: 使用vector存储批量读取的页面对象
    std::vector<Page*> batch_buffer_;
};

}  // namespace sqlcc