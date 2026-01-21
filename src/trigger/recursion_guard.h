/**
 * @file recursion_guard.h
 * @brief 递归防护机制类定义
 *
 * Why: 防止触发器递归调用导致的死循环
 * What: RecursionGuard类跟踪触发器调用栈和递归深度
 * How: 维护调用栈和深度计数，提供递归检测功能
 */

#pragma once

#include <string>
#include <vector>
#include <unordered_map>

namespace sqlcc {
namespace trigger {

/**
 * @brief 递归防护机制
 *
 * 防止触发器递归调用导致的死循环和栈溢出
 */
class RecursionGuard {
public:
    /**
     * @brief 构造函数
     */
    RecursionGuard();

    /**
     * @brief 析构函数
     */
    ~RecursionGuard();

    /**
     * @brief 进入触发器执行上下文
     * @param trigger_name 触发器名称
     * @return 如果允许执行返回true，否则返回false (防止递归)
     */
    bool enterTrigger(const std::string& trigger_name);

    /**
     * @brief 离开触发器执行上下文
     * @param trigger_name 触发器名称
     */
    void exitTrigger(const std::string& trigger_name);

    /**
     * @brief 检查是否在递归调用中
     * @param trigger_name 触发器名称
     * @return 如果在递归中返回true
     */
    bool isRecursive(const std::string& trigger_name) const;

    /**
     * @brief 获取当前调用深度
     */
    size_t getDepth() const;

    /**
     * @brief 重置防护状态
     */
    void reset();

private:
    std::vector<std::string> call_stack_;  ///< 调用栈
    std::unordered_map<std::string, int> trigger_depth_;  ///< 触发器深度计数

    static const size_t MAX_RECURSION_DEPTH = 10;  ///< 最大递归深度
};

} // namespace trigger
} // namespace sqlcc
