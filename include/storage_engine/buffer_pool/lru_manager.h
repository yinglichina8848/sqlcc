/**
 * @file lru_manager.h
 * @brief LRU缓存管理器类定义
 *
 * Why: 需要一个专门的LRU管理器来处理页面缓存的替换策略
 * What: LRUManager类提供LRU算法的实现，管理页面的访问顺序
 * How: 使用双向链表和哈希映射来实现高效的LRU操作
 */

#pragma once

#include <list>
#include <unordered_map>
#include <cstdint>
#include <mutex>

namespace sqlcc {
namespace storage {

/**
 * @brief LRU缓存管理器
 *
 * 提供LRU（Least Recently Used）缓存替换策略的实现。
 * 维护页面的访问顺序，支持快速查找和更新操作。
 */
class LRUManager {
public:
    /**
     * @brief 构造函数
     */
    LRUManager();

    /**
     * @brief 析构函数
     */
    ~LRUManager();

    /**
     * @brief 访问页面，将页面移到LRU链表头部
     * @param page_id 页面ID
     */
    void Access(int32_t page_id);

    /**
     * @brief 添加新页面到LRU管理器
     * @param page_id 页面ID
     */
    void Add(int32_t page_id);

    /**
     * @brief 从LRU管理器中移除页面
     * @param page_id 页面ID
     */
    void Remove(int32_t page_id);

    /**
     * @brief 获取最少使用的页面（LRU链表尾部）
     * @return 最少使用的页面ID，如果没有页面则返回-1
     */
    int32_t GetLeastRecentlyUsed() const;

    /**
     * @brief 检查页面是否在LRU管理器中
     * @param page_id 页面ID
     * @return 如果页面存在返回true，否则返回false
     */
    bool Contains(int32_t page_id) const;

    /**
     * @brief 清空LRU管理器
     */
    void Clear();

    /**
     * @brief 获取当前管理的页面数量
     * @return 页面数量
     */
    size_t Size() const;

private:
    mutable std::mutex mutex_;  ///< 互斥锁，保护并发访问

    std::list<int32_t> lru_list_;  ///< LRU双向链表，头部是最常使用的页面
    std::unordered_map<int32_t, std::list<int32_t>::iterator> lru_map_;  ///< 页面ID到链表迭代器的映射
};

} // namespace storage
} // namespace sqlcc
