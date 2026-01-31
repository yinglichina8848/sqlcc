/**
 * @file strategy_factory.h
 * @brief 替换策略工厂类定义
 *
 * Why: 需要提供统一的工厂接口来创建不同类型的替换策略实例
 * What: ReplaceStrategyFactory类提供静态工厂方法创建各种替换策略
 * How: 使用枚举类型标识策略类型，提供创建和名称转换方法
 */

#pragma once

#include "abstract_strategy.h"
#include <memory>
#include <string>

namespace sqlcc {

/**
 * @brief 替换策略工厂
 *
 * WHY层 - 设计意图：
 *   系统需要支持多种缓存置换算法以应对不同的部署环境（如嵌入式 vs 云端高并发）。
 *   ReplaceStrategyFactory 隔离了各种具体策略类的实例化逻辑，
 *   使 BufferPool 能够根据配置文件动态加载所需的算法，实现了真正的“算法插件化”。
 *
 * WHAT层 - 功能说明：
 *   根据 StrategyType 或 字符串名称 创建对应的具体策略子类实例（std::unique_ptr）。
 *   提供策略类型与人类可读名称之间的转换支持。
 *
 * HOW层 - 实现机制：
 *   1. 简单工厂模式：CreateStrategy 方法内部使用 switch-case 结构分发实例化。
 *   2. 配置映射：GetStrategyType 实现了不区分大小写的字符串匹配逻辑（如 "lru" -> StrategyType::LRU）。
 *   3. 零状态接口：所有方法均为 static，作为纯工具类提供服务。
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
