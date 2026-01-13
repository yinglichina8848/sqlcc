/**
 * @file connection_state_machine.cpp
 * @brief 连接状态机实现 - 严格验证连接状态转换安全性
 *
 * 该文件实现了连接状态机的核心逻辑，确保网络连接状态的严格管理和安全转换。
 * 防止状态机被不当使用导致的安全漏洞。
 */

#include <stdexcept>
#include <unordered_map>

namespace sqlcc {
namespace network {

// 连接状态机实现
ConnectionStateMachine::ConnectionStateMachine()
    : state_(DISCONNECTED),
      state_change_callback_(nullptr),
      connect_timeout_(std::chrono::seconds(30)),
      reconnect_delay_(std::chrono::seconds(5)),
      keep_alive_interval_(std::chrono::seconds(60)),
      connection_start_time_(std::chrono::steady_clock::now()),
      last_activity_time_(std::chrono::steady_clock::now()),
      connection_timer_active_(false),
      auto_reconnect_enabled_(false),
      keep_alive_enabled_(false),
      max_retry_attempts_(3),
      current_retry_count_(0) {
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
        {CONNECTION_ERROR, "CONNECTION_ERROR"}
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
    return state_ != CONNECTION_ERROR;
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
    state_ = CONNECTION_ERROR;

    // 触发状态变更回调
    if (state_change_callback_ && old_state != CONNECTION_ERROR) {
        try {
            state_change_callback_(old_state, CONNECTION_ERROR);
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
            return to == CONNECTING || to == CONNECTION_ERROR;

        case CONNECTING:
            return to == CONNECTED || to == DISCONNECTED || to == CONNECTION_ERROR;

        case CONNECTED:
            return to == AUTHENTICATING || to == CLOSING || to == CONNECTION_ERROR;

        case AUTHENTICATING:
            return to == AUTHENTICATED || to == CONNECTED || to == CLOSING || to == CONNECTION_ERROR;

        case AUTHENTICATED:
            return to == KEY_EXCHANGING || to == CLOSING || to == CONNECTION_ERROR;

        case KEY_EXCHANGING:
            return to == ENCRYPTED || to == AUTHENTICATED || to == CLOSING || to == CONNECTION_ERROR;

        case ENCRYPTED:
            return to == CLOSING || to == CONNECTION_ERROR;

        case CLOSING:
            return to == CLOSED || to == CONNECTION_ERROR;

        case CLOSED:
            // CLOSED是终止状态，不能转换到其他状态
            return to == CLOSED;

        case CONNECTION_ERROR:
            // CONNECTION_ERROR是终止状态，只能重置
            return to == DISCONNECTED;

        default:
            return false;
    }
}

// 超时和重连机制实现
void ConnectionStateMachine::SetTimeouts(std::chrono::milliseconds connect_timeout,
                                       std::chrono::milliseconds reconnect_delay,
                                       std::chrono::milliseconds keep_alive_interval) {
    std::lock_guard<std::mutex> lock(mutex_);
    connect_timeout_ = connect_timeout;
    reconnect_delay_ = reconnect_delay;
    keep_alive_interval_ = keep_alive_interval;
}

void ConnectionStateMachine::StartConnectionTimer() {
    std::lock_guard<std::mutex> lock(mutex_);
    connection_start_time_ = std::chrono::steady_clock::now();
    connection_timer_active_ = true;
}

void ConnectionStateMachine::StopConnectionTimer() {
    std::lock_guard<std::mutex> lock(mutex_);
    connection_timer_active_ = false;
}

bool ConnectionStateMachine::IsConnectionTimedOut() const {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!connection_timer_active_) {
        return false;
    }

    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - connection_start_time_);
    return elapsed >= connect_timeout_;
}

void ConnectionStateMachine::ResetConnectionTimer() {
    std::lock_guard<std::mutex> lock(mutex_);
    connection_start_time_ = std::chrono::steady_clock::now();
    connection_timer_active_ = true;
}

// 重连机制实现
void ConnectionStateMachine::EnableAutoReconnect(bool enabled, int max_retry_attempts) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto_reconnect_enabled_ = enabled;
    max_retry_attempts_ = max_retry_attempts;
    if (!enabled) {
        current_retry_count_ = 0;
    }
}

bool ConnectionStateMachine::IsAutoReconnectEnabled() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return auto_reconnect_enabled_;
}

int ConnectionStateMachine::GetMaxRetryAttempts() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return max_retry_attempts_;
}

int ConnectionStateMachine::GetCurrentRetryCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return current_retry_count_;
}

void ConnectionStateMachine::IncrementRetryCount() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (auto_reconnect_enabled_) {
        ++current_retry_count_;
    }
}

void ConnectionStateMachine::ResetRetryCount() {
    std::lock_guard<std::mutex> lock(mutex_);
    current_retry_count_ = 0;
}

bool ConnectionStateMachine::ShouldAttemptReconnect() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return auto_reconnect_enabled_ &&
           current_retry_count_ < max_retry_attempts_ &&
           state_ == CONNECTION_ERROR;
}

std::chrono::milliseconds ConnectionStateMachine::GetReconnectDelay() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return reconnect_delay_;
}

// 保活机制实现
void ConnectionStateMachine::EnableKeepAlive(bool enabled) {
    std::lock_guard<std::mutex> lock(mutex_);
    keep_alive_enabled_ = enabled;
    if (enabled) {
        UpdateLastActivity();
    }
}

bool ConnectionStateMachine::IsKeepAliveEnabled() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return keep_alive_enabled_;
}

void ConnectionStateMachine::UpdateLastActivity() {
    std::lock_guard<std::mutex> lock(mutex_);
    last_activity_time_ = std::chrono::steady_clock::now();
}

bool ConnectionStateMachine::IsKeepAliveTimeout() const {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!keep_alive_enabled_) {
        return false;
    }

    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - last_activity_time_);
    return elapsed >= keep_alive_interval_;
}

} // namespace network
} // namespace sqlcc