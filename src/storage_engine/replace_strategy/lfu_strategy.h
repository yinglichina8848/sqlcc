/**
 * @file lfu_strategy.h
 * @brief LFU（最不经常使用）替换策略类定义
 *
 * Why: 需要实现LFU页面替换算法，将访问频率最低的页面替换出去
 * What: LFUReplaceStrategy类继承AbstractReplaceStrategy，实现LFU替换算法
 * How: 维护页面访问频率计数，选择访问次数最少的页面进行替换
 */

#pragma once

#include "abstract_strategy.h"
#include <list>
#include <unordered_map>

namespace sqlcc {

/**
 * @brief LFU（最不经常使用）替换策略
 *
 * WHY层 - 设计意图：
 *   LRU 可能因为一次性的全表扫描（Scans）而污染整个缓冲池。
 *   LFU 基于“被访问次数越多的数据在未来更有用”的假设，
 *   它能有效应对存在热点数据且局部性相对稳定的工作负载。
 *
 * WHAT层 - 功能说明：
 *   维护所有页面的全局访问计数。
 *   当需要替换时，优先淘汰访问计数最小的页面。
 *
 * HOW层 - 实现机制：
 *   1. 频率管理：继承基类的 access_count 自动统计。
 *   2. 顺序组织：使用 std::list 维护一个按频率从小到大排列的队列。
 *   3. 命中提升：RecordAccess 会导致 page_id 在 list 中向后（高频区）移动。
 *   4. 牺牲者选择：SelectVictim 直接从 list 头部（低频区）寻找非 Pin 的候选者。
 */
class LFUReplaceStrategy : public AbstractReplaceStrategy {
public:
    /**
     * @brief 构造函数
     */
    LFUReplaceStrategy();

    /**
     * @brief 析构函数
     */
    ~LFUReplaceStrategy() override = default;

    void RecordAccess(int32_t page_id, bool is_hit, bool is_write) override;
    int32_t SelectVictim() override;

private:
    /**
     * @brief 访问频率列表：频率低的页面在头部
     */
    std::list<int32_t> frequency_list_;

    /**
     * @brief 页面ID到频率列表迭代器的映射
     */
    std::unordered_map<int32_t, std::list<int32_t>::iterator> frequency_map_;
};

} // namespace sqlcc
