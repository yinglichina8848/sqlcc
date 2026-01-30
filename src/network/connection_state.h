/**
 * @file connection_state.h
 * @brief 网络连接状态管理 - 深度安全改进版
 * 
 * 该文件定义了网络连接的状态机实现，
 * 提供严格的连接状态验证和转换安全性保证
 * 
 * v1.1.4 深度安全改进:
 * - 连接状态机严格验证
 * - 状态转换安全性保证  
 * - 超时和重连机制
 * - 并发访问安全保证
 */

#ifndef SQLCC_CONNECTION_STATE_H
#define SQLCC_CONNECTION_STATE_H

#include <memory>
#include <mutex>
#include <chrono>
#include <atomic>
#include <condition_variable>
#include <stdexcept>
#include <string>

namespace sqlcc {
namespace network {

// 连接状态枚举
enum class ConnectionState {
    DISCONNECTED = 0,    // 未连接
    CONNECTING = 1,      // 正在连接
    CONNECTED = 2,       // 已连接
    AUTHENTICATING = 3,  // 认证中
    AUTHENTICATED = 4,   // 已认证
    DISCONNECTING = 5,   // 正在断开
    ERROR = 6,           // 错误状态
    RECONNECTING = 7     // 重连中
};

// 连接状态转换事件
enum class ConnectionEvent {
    CONNECT_REQUEST,      // 连接请求
    CONNECT_SUCCESS,      // 连接成功
    CONNECT_FAILURE,      // 连接失败
    AUTHENTICATE_REQUEST, // 认证请求
    AUTHENTICATE_SUCCESS, // 认证成功
    AUTHENTICATE_FAILURE, // 认证失败
    DISCONNECT_REQUEST,   // 断开请求
    DISCONNECT_SUCCESS,   // 断开成功
    TIMEOUT,              // 超时
    ERROR,                // 错误
    RECONNECT             // 重连
};

// 连接状态机 - 深度安全改进
class ConnectionStateMachine {
public:
    // 构造函数 - 增强异常安全
    ConnectionStateMachine()
        : current_state_(ConnectionState::DISCONNECTED),
          target_state_(ConnectionState::DISCONNECTED),
          is_transitioning_(false),
          last_error_(""),
          last_state_change_time_(std::chrono::steady_clock::now()) {}
    
    // 析构函数 - 增强异常安全
    ~ConnectionStateMachine() = default;
    
    // 获取当前状态 - 线程安全
    ConnectionState get_current_state() const {
        std::lock_guard<std::mutex> lock(state_mutex_);
        return current_state_;
    }
    
    // 检查是否可以进行指定转换 - 增强验证
    bool can_transition(ConnectionEvent event) const {
        std::lock_guard<std::mutex> lock(state_mutex_);
        return is_valid_transition(current_state_, event);
    }
    
    // 执行状态转换 - 强保证异常安全
    bool transition(ConnectionEvent event) {
        std::lock_guard<std::mutex> lock(state_mutex_);
        
        if (is_transitioning_) {
            throw std::runtime_error("State transition already in progress");
        }
        
        ConnectionState new_state = get_next_state(current_state_, event);
        if (new_state == current_state_) {
            return false; // 无效转换
        }
        
        // 执行状态转换
        is_transitioning_ = true;
        ConnectionState old_state = current_state_;
        current_state_ = new_state;
        target_state_ = new_state;
        last_state_change_time_ = std::chrono::steady_clock::now();
        last_error_ = "";
        
        is_transitioning_ = false;
        
        // 通知状态变化
        state_changed_cv_.notify_all();
        
        return true;
    }
    
    // 设置目标状态 - 线程安全
    void set_target_state(ConnectionState target_state) {
        std::lock_guard<std::mutex> lock(state_mutex_);
        target_state_ = target_state;
    }
    
    // 获取目标状态 - 线程安全
    ConnectionState get_target_state() const {
        std::lock_guard<std::mutex> lock(state_mutex_);
        return target_state_;
    }
    
    // 等待状态变化 - 增强异常安全
    bool wait_for_state_change(ConnectionState expected_state, 
                               std::chrono::milliseconds timeout) {
        std::unique_lock<std::mutex> lock(state_mutex_);
        return state_changed_cv_.wait_for(lock, timeout, 
            [this, expected_state]() { 
                return current_state_ == expected_state && !is_transitioning_; 
            });
    }
    
    // 设置错误状态 - 线程安全
    void set_error(const std::string& error_message) {
        std::lock_guard<std::mutex> lock(state_mutex_);
        current_state_ = ConnectionState::ERROR;
        last_error_ = error_message;
        last_state_change_time_ = std::chrono::steady_clock::now();
        state_changed_cv_.notify_all();
    }
    
    // 获取最后错误 - 线程安全
    std::string get_last_error() const {
        std::lock_guard<std::mutex> lock(state_mutex_);
        return last_error_;
    }
    
    // 检查是否正在转换 - 线程安全
    bool is_transitioning() const {
        std::lock_guard<std::mutex> lock(state_mutex_);
        return is_transitioning_;
    }
    
    // 获取状态持续时间 - 线程安全
    std::chrono::milliseconds get_state_duration() const {
        std::lock_guard<std::mutex> lock(state_mutex_);
        auto now = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            now - last_state_change_time_);
    }
    
    // 重置状态机 - 增强异常安全
    void reset() {
        std::lock_guard<std::mutex> lock(state_mutex_);
        if (is_transitioning_) {
            throw std::runtime_error("Cannot reset during state transition");
        }
        current_state_ = ConnectionState::DISCONNECTED;
        target_state_ = ConnectionState::DISCONNECTED;
        last_error_ = "";
        last_state_change_time_ = std::chrono::steady_clock::now();
        state_changed_cv_.notify_all();
    }

private:
    // 状态转换验证 - 增强安全性
    bool is_valid_transition(ConnectionState current, ConnectionEvent event) const {
        switch (current) {
            case ConnectionState::DISCONNECTED:
                return event == ConnectionEvent::CONNECT_REQUEST ||
                       event == ConnectionEvent::ERROR;
                       
            case ConnectionState::CONNECTING:
                return event == ConnectionEvent::CONNECT_SUCCESS ||
                       event == ConnectionEvent::CONNECT_FAILURE ||
                       event == ConnectionEvent::TIMEOUT ||
                       event == ConnectionEvent::DISCONNECT_REQUEST ||
                       event == ConnectionEvent::ERROR;
                       
            case ConnectionState::CONNECTED:
                return event == ConnectionEvent::AUTHENTICATE_REQUEST ||
                       event == ConnectionEvent::DISCONNECT_REQUEST ||
                       event == ConnectionEvent::ERROR;
                       
            case ConnectionState::AUTHENTICATING:
                return event == ConnectionEvent::AUTHENTICATE_SUCCESS ||
                       event == ConnectionEvent::AUTHENTICATE_FAILURE ||
                       event == ConnectionEvent::TIMEOUT ||
                       event == ConnectionEvent::DISCONNECT_REQUEST ||
                       event == ConnectionEvent::ERROR;
                       
            case ConnectionState::AUTHENTICATED:
                return event == ConnectionEvent::DISCONNECT_REQUEST ||
                       event == ConnectionEvent::ERROR;
                       
            case ConnectionState::DISCONNECTING:
                return event == ConnectionEvent::DISCONNECT_SUCCESS ||
                       event == ConnectionEvent::ERROR;
                       
            case ConnectionState::ERROR:
                return event == ConnectionEvent::RECONNECT ||
                       event == ConnectionEvent::DISCONNECT_REQUEST;
                       
            case ConnectionState::RECONNECTING:
                return event == ConnectionEvent::CONNECT_SUCCESS ||
                       event == ConnectionEvent::CONNECT_FAILURE ||
                       event == ConnectionEvent::TIMEOUT ||
                       event == ConnectionEvent::ERROR;
                       
            default:
                return false;
        }
    }
    
    // 获取下一个状态 - 增强验证
    ConnectionState get_next_state(ConnectionState current, ConnectionEvent event) const {
        if (!is_valid_transition(current, event)) {
            throw std::invalid_argument("Invalid state transition: " + 
                                       std::to_string(static_cast<int>(current)) + 
                                       " -> " + std::to_string(static_cast<int>(event)));
        }
        
        switch (current) {
            case ConnectionState::DISCONNECTED:
                if (event == ConnectionEvent::CONNECT_REQUEST) {
                    return ConnectionState::CONNECTING;
                }
                break;
                
            case ConnectionState::CONNECTING:
                if (event == ConnectionEvent::CONNECT_SUCCESS) {
                    return ConnectionState::CONNECTED;
                } else if (event == ConnectionEvent::CONNECT_FAILURE) {
                    return ConnectionState::ERROR;
                } else if (event == ConnectionEvent::DISCONNECT_REQUEST) {
                    return ConnectionState::DISCONNECTED;
                }
                break;
                
            case ConnectionState::CONNECTED:
                if (event == ConnectionEvent::AUTHENTICATE_REQUEST) {
                    return ConnectionState::AUTHENTICATING;
                } else if (event == ConnectionEvent::DISCONNECT_REQUEST) {
                    return ConnectionState::DISCONNECTING;
                }
                break;
                
            case ConnectionState::AUTHENTICATING:
                if (event == ConnectionEvent::AUTHENTICATE_SUCCESS) {
                    return ConnectionState::AUTHENTICATED;
                } else if (event == ConnectionEvent::AUTHENTICATE_FAILURE) {
                    return ConnectionState::ERROR;
                } else if (event == ConnectionEvent::DISCONNECT_REQUEST) {
                    return ConnectionState::DISCONNECTING;
                }
                break;
                
            case ConnectionState::AUTHENTICATED:
                if (event == ConnectionEvent::DISCONNECT_REQUEST) {
                    return ConnectionState::DISCONNECTING;
                }
                break;
                
            case ConnectionState::DISCONNECTING:
                if (event == ConnectionEvent::DISCONNECT_SUCCESS) {
                    return ConnectionState::DISCONNECTED;
                }
                break;
                
            case ConnectionState::ERROR:
                if (event == ConnectionEvent::RECONNECT) {
                    return ConnectionState::RECONNECTING;
                } else if (event == ConnectionEvent::DISCONNECT_REQUEST) {
                    return ConnectionState::DISCONNECTED;
                }
                break;
                
            case ConnectionState::RECONNECTING:
                if (event == ConnectionEvent::CONNECT_SUCCESS) {
                    return ConnectionState::CONNECTED;
                } else if (event == ConnectionEvent::CONNECT_FAILURE) {
                    return ConnectionState::ERROR;
                }
                break;
        }
        
        // 如果没有匹配的转换，保持当前状态
        return current;
    }
    
    ConnectionState current_state_;
    ConnectionState target_state_;
    mutable std::mutex state_mutex_;
    std::condition_variable state_changed_cv_;
    std::atomic<bool> is_transitioning_;
    std::string last_error_;
    std::chrono::steady_clock::time_point last_state_change_time_;
};

// 连接超时管理器 - 深度安全改进
class ConnectionTimeoutManager {
public:
    // 构造函数 - 增强异常安全
    explicit ConnectionTimeoutManager(std::chrono::milliseconds connect_timeout = 
                                   std::chrono::seconds(30))
        : connect_timeout_(connect_timeout),
          authentication_timeout_(std::chrono::seconds(10)),
          last_activity_time_(std::chrono::steady_clock::now()),
          is_timeout_enabled_(true) {}
    
    // 析构函数 - 增强异常安全
    ~ConnectionTimeoutManager() = default;
    
    // 设置连接超时 - 线程安全
    void set_connect_timeout(std::chrono::milliseconds timeout) {
        if (timeout <= std::chrono::milliseconds::zero()) {
            throw std::invalid_argument("Connect timeout must be positive");
        }
        std::lock_guard<std::mutex> lock(timeout_mutex_);
        connect_timeout_ = timeout;
    }
    
    // 设置认证超时 - 线程安全
    void set_authentication_timeout(std::chrono::milliseconds timeout) {
        if (timeout <= std::chrono::milliseconds::zero()) {
            throw std::invalid_argument("Authentication timeout must be positive");
        }
        std::lock_guard<std::mutex> lock(timeout_mutex_);
        authentication_timeout_ = timeout;
    }
    
    // 检查是否超时 - 增强验证
    bool is_timeout(ConnectionState current_state) const {
        std::lock_guard<std::mutex> lock(timeout_mutex_);
        
        if (!is_timeout_enabled_) {
            return false;
        }
        
        auto now = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - last_activity_time_);
        
        switch (current_state) {
            case ConnectionState::CONNECTING:
                return duration > connect_timeout_;
            case ConnectionState::AUTHENTICATING:
                return duration > authentication_timeout_;
            default:
                return false;
        }
    }
    
    // 更新活动时间 - 线程安全
    void update_activity_time() {
        std::lock_guard<std::mutex> lock(timeout_mutex_);
        last_activity_time_ = std::chrono::steady_clock::now();
    }
    
    // 启用/禁用超时 - 线程安全
    void set_timeout_enabled(bool enabled) {
        std::lock_guard<std::mutex> lock(timeout_mutex_);
        is_timeout_enabled_ = enabled;
    }
    
    // 检查超时是否启用 - 线程安全
    bool is_timeout_enabled() const {
        std::lock_guard<std::mutex> lock(timeout_mutex_);
        return is_timeout_enabled_;
    }

private:
    std::chrono::milliseconds connect_timeout_;
    std::chrono::milliseconds authentication_timeout_;
    std::chrono::steady_clock::time_point last_activity_time_;
    std::atomic<bool> is_timeout_enabled_;
    mutable std::mutex timeout_mutex_;
};

} // namespace network
} // namespace sqlcc

#endif // SQLCC_CONNECTION_STATE_H
