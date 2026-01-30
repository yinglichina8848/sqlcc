/**
 * @file strategy_factory.h
 * @brief 替换策略工厂类定义
 *
 * Why: 需要提供统一的工厂接口来创建不同类型的替换策略实例
 * What: ReplaceStrategyFactory类提供静态工厂方法创建各种替换策略
 * How: 使用枚举类型标识策略类型，提供创建和名称转换方法
 */

#pragma once

#include "src/storage_engine/replace_strategy/abstract_strategy.h"
#include <memory>
#include <string>

namespace sqlcc {

/**
 * @brief 替换策略工厂
 */
class ReplaceStrategyFactory {
public:
    /**
     * @brief 策略类型枚举
     */
    enum class StrategyType {
        LRU,     // 最近最少使用
        LFU,     // 最不经常使用
        CLOCK,   // 时钟算法
        ARC,     // 自适应替换缓存
        FIFO     // 先进先出
    };

    /**
     * @brief 创建替换策略实例
     * @param type 策略类型
     * @return 策略实例
     */
    static std::unique_ptr<AbstractReplaceStrategy> CreateStrategy(StrategyType type);

    /**
     * @brief 获取策略类型对应的名称
     * @param type 策略类型
     * @return 策略名称
     */
    static std::string GetStrategyName(StrategyType type);

    /**
     * @brief 根据名称获取策略类型
     * @param name 策略名称
     * @return 策略类型
     */
    static StrategyType GetStrategyType(const std::string& name);
};

} // namespace sqlcc
