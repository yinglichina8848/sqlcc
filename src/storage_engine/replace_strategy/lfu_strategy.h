/**
 * @file lfu_strategy.h
 * @brief LFU（最不经常使用）替换策略类定义
 *
 * Why: 需要实现LFU页面替换算法，将访问频率最低的页面替换出去
 * What: LFUReplaceStrategy类继承AbstractReplaceStrategy，实现LFU替换算法
 * How: 维护页面访问频率计数，选择访问次数最少的页面进行替换
 */

#pragma once

#include "src/storage_engine/replace_strategy/abstract_strategy.h"
#include <list>
#include <unordered_map>

namespace sqlcc {

/**
 * @brief LFU（最不经常使用）替换策略
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
