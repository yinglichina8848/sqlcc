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
