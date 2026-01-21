/**
 * @file clock_strategy.h
 * @brief CLOCK（时钟）替换策略类定义
 *
 * Why: 需要实现CLOCK页面替换算法，提供比LRU更高效的近似算法
 * What: ClockReplaceStrategy类继承AbstractReplaceStrategy，实现CLOCK替换算法
 * How: 使用引用位和时钟指针来近似LRU行为，减少LRU的链表维护开销
 */

#pragma once

#include "abstract_strategy.h"
#include <list>
#include <unordered_map>

namespace sqlcc {

/**
 * @brief CLOCK（时钟）替换策略
 */
class ClockReplaceStrategy : public AbstractReplaceStrategy {
public:
    /**
     * @brief 构造函数
     */
    ClockReplaceStrategy();

    /**
     * @brief 析构函数
     */
    ~ClockReplaceStrategy() override = default;

    void RecordAccess(int32_t page_id, bool is_hit, bool is_write) override;
    int32_t SelectVictim() override;

private:
    /**
     * @brief 页面引用位
     */
    std::unordered_map<int32_t, bool> reference_bits_;

    /**
     * @brief 时钟指针
     */
    std::list<int32_t>::iterator clock_hand_;
    std::list<int32_t> clock_list_;
};

} // namespace sqlcc
