/**
 * @file lru_strategy.h
 * @brief LRU（最近最少使用）替换策略类定义
 *
 * Why: 需要实现LRU页面替换算法，将最近最少使用的页面替换出去
 * What: LRUReplaceStrategy类继承AbstractReplaceStrategy，实现LRU替换算法
 * How: 使用双向链表维护页面访问顺序，最近使用的页面在链表头部
 */

#pragma once

#include "abstract_strategy.h"
#include <list>
#include <unordered_map>

namespace sqlcc {

/**
 * @brief LRU（最近最少使用）替换策略
 *
 * WHY层 - 设计意图：
 *   LRU 基于“最近被访问过的数据在未来更有可能被再次访问”的假设。
 *   它能有效应对具有时间局部性（Temporal Locality）的工作负载，
 *   是数据库缓冲池最常用、最高效的通用平衡算法。
 *
 * WHAT层 - 功能说明：
 *   实现基于顺序的时间戳置换逻辑。
 *   维护一个活跃页面的排序队列，头部为最常访问，尾部为最久未访问。
 *
 * HOW层 - 实现机制：
 *   1. 核心结构：使用 std::list 记录顺序，使用 std::unordered_map 存储迭代器实现 O(1) 定位。
 *   2. 命中更新：每次 RecordAccess，若命正则将该 page_id 移动到 list 头部。
 *   3. 牺牲者选择：SelectVictim 从 list 尾部向前扫描，返回首个 pin_count 为 0 的页面。
 */
class LRUReplaceStrategy : public AbstractReplaceStrategy {
public:
    /**
     * @brief 构造函数
     */
    LRUReplaceStrategy();

    /**
     * @brief 析构函数
     */
    ~LRUReplaceStrategy() override = default;

    void RecordAccess(int32_t page_id, bool is_hit, bool is_write) override;
    int32_t SelectVictim() override;

private:
    /**
     * @brief LRU链表：最近使用的页面在头部
     */
    std::list<int32_t> lru_list_;

    /**
     * @brief 页面ID到LRU链表迭代器的映射
     */
    std::unordered_map<int32_t, std::list<int32_t>::iterator> lru_map_;

    /**
     * @brief 更新LRU列表
     * @param page_id 页面ID
     */
    void UpdateLRU(int32_t page_id);
};

} // namespace sqlcc
