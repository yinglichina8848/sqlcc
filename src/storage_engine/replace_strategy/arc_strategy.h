/**
 * @file arc_strategy.h
 * @brief ARC（自适应替换缓存）替换策略类定义
 *
 * Why: 需要实现ARC算法，自适应地在LRU和LFU之间切换以获得更好的性能
 * What: ARCReplaceStrategy类继承AbstractReplaceStrategy，实现ARC替换算法
 * How: 结合LRU和LFU的优点，使用T1、T2、B1、B2四个列表进行自适应替换
 */

#pragma once

#include "src/storage_engine/replace_strategy/abstract_strategy.h"
#include <list>

namespace sqlcc {

/**
 * @brief ARC（自适应替换缓存）替换策略
 *
 * 结合了LRU和LFU的优点，自适应地在两种策略之间切换。
 */
class ARCReplaceStrategy : public AbstractReplaceStrategy {
public:
    /**
     * @brief 构造函数
     * @param p 初始T1和T2的大小参数（默认：总缓存大小的1/32）
     * @param total_size 总缓存大小
     */
    ARCReplaceStrategy(size_t p, size_t total_size);

    /**
     * @brief 析构函数
     */
    ~ARCReplaceStrategy() override = default;

    void RecordAccess(int32_t page_id, bool is_hit, bool is_write) override;
    int32_t SelectVictim() override;

private:
    /**
     * @brief T1列表：最近访问的页面
     */
    std::list<int32_t> t1_list_;

    /**
     * @brief T2列表：经常访问的页面
     */
    std::list<int32_t> t2_list_;

    /**
     * @brief B1列表：最近被替换出T1的页面
     */
    std::list<int32_t> b1_list_;

    /**
     * @brief B2列表：最近被替换出T2的页面
     */
    std::list<int32_t> b2_list_;

    /**
     * @brief P值：T1的目标大小
     */
    size_t p_;

    /**
     * @brief 总缓存大小
     */
    const size_t total_size_;

    /**
     * @brief 获取迭代器的工具函数
     */
    std::list<int32_t>::iterator GetIterator(const std::string& list_name, int32_t page_id);
};

} // namespace sqlcc
