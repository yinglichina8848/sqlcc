#include "trigger/recursion_guard.h"

namespace sqlcc {
namespace trigger {

RecursionGuard::RecursionGuard() : call_stack_(), trigger_depth_() {
    // 初始化为空
}

RecursionGuard::~RecursionGuard() {
    // 清理工作（如果需要）
    reset();
}

bool RecursionGuard::enterTrigger(const std::string& trigger_name) {
    // 检查递归深度
    if (call_stack_.size() >= MAX_RECURSION_DEPTH) {
        return false; // 超过最大递归深度，拒绝进入
    }

    // 检查该触发器是否已经在调用栈中（防止直接递归）
    if (std::find(call_stack_.begin(), call_stack_.end(), trigger_name) != call_stack_.end()) {
        return false; // 直接递归，拒绝进入
    }

    // 增加触发器深度计数
    trigger_depth_[trigger_name]++;

    // 添加到调用栈
    call_stack_.push_back(trigger_name);

    return true;
}

void RecursionGuard::exitTrigger(const std::string& trigger_name) {
    // 从调用栈中移除
    auto it = std::find(call_stack_.rbegin(), call_stack_.rend(), trigger_name);
    if (it != call_stack_.rend()) {
        // 转换反向迭代器为正向迭代器
        auto forward_it = it.base();
        --forward_it; // 反向迭代器指向的位置需要调整
        call_stack_.erase(forward_it);
    }

    // 减少触发器深度计数
    auto depth_it = trigger_depth_.find(trigger_name);
    if (depth_it != trigger_depth_.end()) {
        depth_it->second--;
        if (depth_it->second <= 0) {
            trigger_depth_.erase(depth_it);
        }
    }
}

bool RecursionGuard::isRecursive(const std::string& trigger_name) const {
    // 检查触发器是否在当前调用栈中
    return std::find(call_stack_.begin(), call_stack_.end(), trigger_name) != call_stack_.end();
}

size_t RecursionGuard::getDepth() const {
    return call_stack_.size();
}

void RecursionGuard::reset() {
    call_stack_.clear();
    trigger_depth_.clear();
}

} // namespace trigger
} // namespace sqlcc
