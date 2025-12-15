/**
 * @file connection_state_machine.cpp
 * @brief 连接状态机实现 - 严格验证连接状态转换安全性
 *
 * 该文件实现了连接状态机的核心逻辑，确保网络连接状态的严格管理和安全转换。
 * 防止状态机被不当使用导致的安全漏洞。
 */

#include "network/network.h"
#include <stdexcept>
#include <unordered_map>

namespace sqlcc {
namespace network {

// 连接状态机实现
ConnectionStateMachine::ConnectionStateMachine()
    : state_(DISCONNECTED), state_change_callback_(nullptr) {
}

ConnectionState ConnectionStateMachine::GetCurrentState() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return state_;
}

bool ConnectionStateMachine::IsInState(ConnectionState state) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return state_ == state;
}

std::string ConnectionStateMachine::GetStateName(ConnectionState state) const {
    static const std::unordered_map<ConnectionState, std::string> state_names = {
        {DISCONNECTED, "DISCONNECTED"},
        {CONNECTING, "CONNECTING"},
        {CONNECTED, "CONNECTED"},
        {AUTHENTICATING, "AUTHENTICATING"},
        {AUTHENTICATED, "AUTHENTICATED"},
        {KEY_EXCHANGING, "KEY_EXCHANGING"},
        {ENCRYPTED, "ENCRYPTED"},
        {CLOSING, "CLOSING"},
        {CLOSED, "CLOSED"},
        {ERROR, "ERROR"}
    };

    auto it = state_names.find(state);
    return it != state_names.end() ? it->second : "UNKNOWN";
}

bool ConnectionStateMachine::TransitionTo(ConnectionState new_state) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!IsValidTransition(state_, new_state)) {
        // 记录无效转换尝试（在实际实现中应该记录到日志）
        return false;
    }

    ConnectionState old_state = state_;
    state_ = new_state;

    // 触发状态变更回调
    if (state_change_callback_) {
        try {
            state_change_callback_(old_state, new_state);
        } catch (...) {
            // 回调异常不应该影响状态转换
            // 在实际实现中应该记录错误
        }
    }

    return true;
}

bool ConnectionStateMachine::CanTransitionTo(ConnectionState new_state) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return IsValidTransition(state_, new_state);
}

bool ConnectionStateMachine::CanConnect() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return state_ == DISCONNECTED;
}

bool ConnectionStateMachine::CanAuthenticate() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return state_ == CONNECTED;
}

bool ConnectionStateMachine::CanSendQuery() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return state_ == AUTHENTICATED || state_ == ENCRYPTED;
}

bool ConnectionStateMachine::CanEncrypt() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return state_ == AUTHENTICATED;
}

bool ConnectionStateMachine::CanClose() const {
    std::lock_guard<std::mutex> lock(mutex_);
    // 除了错误状态，任何状态都可以关闭
    return state_ != ERROR;
}

void ConnectionStateMachine::Reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    ConnectionState old_state = state_;
    state_ = DISCONNECTED;

    // 触发状态变更回调
    if (state_change_callback_ && old_state != DISCONNECTED) {
        try {
            state_change_callback_(old_state, DISCONNECTED);
        } catch (...) {
            // 忽略回调异常
        }
    }
}

void ConnectionStateMachine::ForceError() {
    std::lock_guard<std::mutex> lock(mutex_);
    ConnectionState old_state = state_;
    state_ = ERROR;

    // 触发状态变更回调
    if (state_change_callback_ && old_state != ERROR) {
        try {
            state_change_callback_(old_state, ERROR);
        } catch (...) {
            // 忽略回调异常
        }
    }
}

void ConnectionStateMachine::SetStateChangeCallback(StateChangeCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    state_change_callback_ = std::move(callback);
}

bool ConnectionStateMachine::IsValidTransition(ConnectionState from, ConnectionState to) const {
    // 定义有效的状态转换规则
    switch (from) {
        case DISCONNECTED:
            return to == CONNECTING || to == ERROR;

        case CONNECTING:
            return to == CONNECTED || to == DISCONNECTED || to == ERROR;

        case CONNECTED:
            return to == AUTHENTICATING || to == CLOSING || to == ERROR;

        case AUTHENTICATING:
            return to == AUTHENTICATED || to == CONNECTED || to == CLOSING || to == ERROR;

        case AUTHENTICATED:
            return to == KEY_EXCHANGING || to == CLOSING || to == ERROR;

        case KEY_EXCHANGING:
            return to == ENCRYPTED || to == AUTHENTICATED || to == CLOSING || to == ERROR;

        case ENCRYPTED:
            return to == CLOSING || to == ERROR;

        case CLOSING:
            return to == CLOSED || to == ERROR;

        case CLOSED:
            // CLOSED是终止状态，不能转换到其他状态
            return to == CLOSED;

        case ERROR:
            // ERROR是终止状态，只能重置
            return to == DISCONNECTED;

        default:
            return false;
    }
}

} // namespace network
} // namespace sqlcc
