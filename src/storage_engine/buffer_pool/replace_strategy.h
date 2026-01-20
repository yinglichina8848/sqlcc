/**
 * @file replace_strategy.h
 * @brief 页面替换策略头文件
 *
 * Why: 需要一个统一的接口来定义各种页面替换策略
 * What: 定义了页面替换策略的抽象接口和各种具体实现
 * How: 提供抽象基类和具体策略类，支持多种替换算法
 *
 * Note: 替换策略类已重构为独立文件，每个类一个文件。保持此头文件用于向后兼容性。
 */

// Why: 防止头文件被多次包含，避免编译错误
// What: 使用#pragma once指令确保头文件只被编译一次
// How: 在文件开头添加#pragma once预处理指令
#pragma once

// Why: 包含所有分离的替换策略头文件，保持向后兼容性
// What: 包含所有替换策略类定义，保持API不变
// How: 使用#include预处理指令包含分离的策略头文件
#include "replace_strategy/abstract_strategy.h"
#include "replace_strategy/lru_strategy.h"
#include "replace_strategy/lfu_strategy.h"
#include "replace_strategy/clock_strategy.h"
#include "replace_strategy/arc_strategy.h"
#include "replace_strategy/strategy_factory.h"

// Why: 将所有替换策略类放在命名空间中，避免命名冲突
// What: 定义sqlcc命名空间，包含所有存储相关的替换策略类
// How: 使用namespace关键字定义命名空间，并使用using声明导出所有策略类
namespace sqlcc {

// 导出所有替换策略类到sqlcc命名空间，保持向后兼容性
using sqlcc::AbstractReplaceStrategy;
using sqlcc::LRUReplaceStrategy;
using sqlcc::LFUReplaceStrategy;
using sqlcc::ClockReplaceStrategy;
using sqlcc::ARCReplaceStrategy;
using sqlcc::ReplaceStrategyFactory;

} // namespace sqlcc
