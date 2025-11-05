#pragma once

#include "page.h"
#include "disk_manager.h"
#include <unordered_map>
#include <list>
#include <memory>

namespace sqlcc {

// 缓冲池默认大小
static constexpr size_t DEFAULT_BUFFER_POOL_SIZE = 64;

/**
 * @brief 缓冲池管理器类，实现LRU页面替换算法
 */
class BufferPool {
public:
    /**
     * @brief 构造函数
     * @param disk_manager 磁盘管理器指针
     * @param pool_size 缓冲池大小（页面数量）
     */
    BufferPool(DiskManager* disk_manager, size_t pool_size = DEFAULT_BUFFER_POOL_SIZE);

    /**
     * @brief 析构函数
     */
    ~BufferPool();

    /**
     * @brief 禁止拷贝构造
     */
    BufferPool(const BufferPool&) = delete;

    /**
     * @brief 禁止赋值操作
     */
    BufferPool& operator=(const BufferPool&) = delete;

    /**
     * @brief 获取页面
     * @param page_id 页面ID
     * @return 页面指针，如果页面不存在返回nullptr
     */
    Page* FetchPage(int32_t page_id);

    /**
     * @brief 取消固定页面（减少引用计数）
     * @param page_id 页面ID
     * @param is_dirty 页面是否被修改
     * @return 操作成功返回true，否则返回false
     */
    bool UnpinPage(int32_t page_id, bool is_dirty);

    /**
     * @brief 刷新页面到磁盘
     * @param page_id 页面ID
     * @return 操作成功返回true，否则返回false
     */
    bool FlushPage(int32_t page_id);

    /**
     * @brief 分配新页面
     * @param[out] page_id 新分配的页面ID
     * @return 新页面指针，分配失败返回nullptr
     */
    Page* NewPage(int32_t* page_id);

    /**
     * @brief 释放页面
     * @param page_id 页面ID
     * @return 操作成功返回true，否则返回false
     */
    bool DeletePage(int32_t page_id);

    /**
     * @brief 刷新所有页面到磁盘
     */
    void FlushAllPages();

private:
    /**
     * @brief 替换页面（LRU算法）
     * @return 被替换的页面ID，如果无法替换返回-1
     */
    int32_t ReplacePage();

    /**
     * @brief 将页面移到LRU链表头部
     * @param page_id 页面ID
     */
    void MoveToHead(int32_t page_id);

    // 磁盘管理器指针
    DiskManager* disk_manager_;

    // 缓冲池大小
    size_t pool_size_;

    // 页面映射表：page_id -> page指针
    std::unordered_map<int32_t, std::unique_ptr<Page>> page_table_;

    // 脏页标记：page_id -> 是否脏页
    std::unordered_map<int32_t, bool> dirty_pages_;

    // 页面引用计数：page_id -> 引用计数
    std::unordered_map<int32_t, int32_t> page_refs_;

    // LRU链表，存储页面ID
    std::list<int32_t> lru_list_;

    // LRU映射：page_id -> lru_list中的迭代器
    std::unordered_map<int32_t, std::list<int32_t>::iterator> lru_map_;
};

}  // namespace sqlcc