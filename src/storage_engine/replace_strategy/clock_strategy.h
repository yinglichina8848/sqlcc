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
 *
 * WHY层 - 设计意图：
 *   LRU 算法在每次页面访问时都需要移动链表节点（Lock & Pointer assignments），在大并发下存在锁竞争开销。
 *   CLOCK 算法（又称 Second Chance 算法）作为 LRU 的一种高效近似实现，
 *   通过“转动时钟拨针”来寻找未被访问过的页面，避免了高频的链表操作。
 *
 * WHAT层 - 功能说明：
 *   为每个页面维护一个“引用位 (Reference Bit)”。
 *   通过一个循环扫描的指针（Clock Hand）周期性地重置引用位并寻找牺牲页。
 *
 * HOW层 - 实现机制：
 *   1. 引用追踪：RecordAccess 时将对应页面的引用位置为 true。
 *   2. 拨针扫描：SelectVictim 开始从当前 clock_hand_ 指向的位置循环：
 *      - 若引用位为 true：将其重置为 false，并给一次“生存机会”，指针移动到下一个。
 *      - 若引用位为 false：该页即为牺牲者，将其移除并返回。
 *   3. 同步机制：利用 clock_list_ 维护环形遍历逻辑。
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
