/**
 * @file config_lifecycle.h
 * @brief RAII配置生命周期管理器头文件
 * 
 * Why: 需要实现RAII模式的配置生命周期管理，确保资源自动清理
 * What: 提供ConfigLifecycleManager类和相关的RAII配置访问功能
 * How: 使用RAII模式管理配置资源的获取和释放
 */

#ifndef SQLCC_CONFIG_LIFECYCLE_H_
#define SQLCC_CONFIG_LIFECYCLE_H_

#include "utils/config_snapshot.h"
#include <memory>
#include <functional>
#include <exception>
#include <chrono>

namespace sqlcc {

/**
 * @brief 配置生命周期状态枚举
 */
enum class ConfigLifecycleState {
    UNINITIALIZED,    // 未初始化
    INITIALIZING,     // 初始化中
    READY,           // 就绪状态
    UPDATING,        // 更新中
    SHUTTING_DOWN,   // 关闭中
    SHUTDOWN         // 已关闭
};

/**
 * @brief 配置生命周期异常类
 */
class ConfigLifecycleException : public std::exception {
private:
    std::string message_;
    ConfigLifecycleState state_;
    
public:
    ConfigLifecycleException(const std::string& message, ConfigLifecycleState state)
        : message_(message), state_(state) {}
    
    const char* what() const noexcept override {
        return message_.c_str();
    }
    
    ConfigLifecycleState GetState() const {
        return state_;
    }
};

/**
 * @brief RAII配置生命周期管理器
 * 
 * 提供配置资源的自动获取和释放，确保异常安全
 */
class ConfigLifecycleManager {
private:
    mutable std::shared_mutex state_mutex_;
    std::atomic<ConfigLifecycleState> state_{ConfigLifecycleState::UNINITIALIZED};
    ConfigSnapshotManager snapshot_manager_;
    std::string current_config_path_;
    
    // 生命周期回调函数
    std::function<void()> on_initialize_;
    std::function<void()> on_shutdown_;
    std::function<void(const std::string&)> on_config_change_;
    
    // 性能监控
    std::chrono::steady_clock::time_point init_time_;
    std::chrono::steady_clock::time_point last_update_time_;
    std::atomic<size_t> update_count_{0};
    std::atomic<size_t> error_count_{0};

public:
    /**
     * @brief 构造函数
     * @param config_path 配置文件路径
     */
    explicit ConfigLifecycleManager(const std::string& config_path = "")
        : current_config_path_(config_path) {
        init_time_ = std::chrono::steady_clock::now();
    }
    
    /**
     * @brief 析构函数
     */
    ~ConfigLifecycleManager() {
        Shutdown();
    }
    
    // 禁止拷贝构造和赋值
    ConfigLifecycleManager(const ConfigLifecycleManager&) = delete;
    ConfigLifecycleManager& operator=(const ConfigLifecycleManager&) = delete;
    
    // 允许移动构造和赋值
    ConfigLifecycleManager(ConfigLifecycleManager&&) noexcept = default;
    ConfigLifecycleManager& operator=(ConfigLifecycleManager&&) noexcept = default;

    /**
     * @brief 初始化配置管理器
     * @param config_path 配置文件路径
     * @return bool 是否成功
     */
    bool Initialize(const std::string& config_path = "") {
        std::unique_lock<std::shared_mutex> lock(state_mutex_);
        
        if (state_.load() != ConfigLifecycleState::UNINITIALIZED) {
            return false;
        }
        
        try {
            state_.store(ConfigLifecycleState::INITIALIZING);
            
            if (!config_path.empty()) {
                current_config_path_ = config_path;
            }
            
            // 执行初始化回调
            if (on_initialize_) {
                on_initialize_();
            }
            
            // 创建默认空配置快照
            auto default_snapshot = ConfigSnapshotFactory::CreateEmptySnapshot(
                GenerateVersionId("init_"), "Default configuration snapshot");
            
            if (!snapshot_manager_.AddSnapshot(default_snapshot)) {
                throw ConfigLifecycleException("Failed to add default snapshot", ConfigLifecycleState::INITIALIZING);
            }
            
            state_.store(ConfigLifecycleState::READY);
            return true;
            
        } catch (const std::exception& e) {
            state_.store(ConfigLifecycleState::UNINITIALIZED);
            ++error_count_;
            throw ConfigLifecycleException(
                std::string("Initialization failed: ") + e.what(), 
                ConfigLifecycleState::UNINITIALIZED);
        }
    }

    /**
     * @brief 关闭配置管理器
     * @return bool 是否成功
     */
    bool Shutdown() {
        std::unique_lock<std::shared_mutex> lock(state_mutex_);
        
        if (state_.load() == ConfigLifecycleState::SHUTDOWN || 
            state_.load() == ConfigLifecycleState::SHUTTING_DOWN) {
            return true;
        }
        
        try {
            state_.store(ConfigLifecycleState::SHUTTING_DOWN);
            
            // 执行关闭回调
            if (on_shutdown_) {
                on_shutdown_();
            }
            
            // 清理快照管理器
            auto version_ids = snapshot_manager_.GetAllVersionIds();
            for (const auto& version_id : version_ids) {
                snapshot_manager_.RemoveSnapshot(version_id);
            }
            
            state_.store(ConfigLifecycleState::SHUTDOWN);
            return true;
            
        } catch (const std::exception& e) {
            state_.store(ConfigLifecycleState::SHUTDOWN);
            ++error_count_;
            throw ConfigLifecycleException(
                std::string("Shutdown failed: ") + e.what(), 
                ConfigLifecycleState::SHUTDOWN);
        }
    }

    /**
     * @brief 获取当前状态
     * @return ConfigLifecycleState 当前状态
     */
    ConfigLifecycleState GetState() const {
        return state_.load();
    }

    /**
     * @brief 检查是否就绪
     * @return bool 是否就绪
     */
    bool IsReady() const {
        return state_.load() == ConfigLifecycleState::READY;
    }

    /**
     * @brief 获取当前配置快照（RAII安全）
     * @return ConfigSnapshot::SnapshotPtr 快照智能指针
     */
    ConfigSnapshot::SnapshotPtr GetCurrentSnapshot() const {
        if (!IsReady()) {
            throw ConfigLifecycleException("Config manager not ready", state_.load());
        }
        
        std::shared_lock<std::shared_mutex> lock(state_mutex_);
        return snapshot_manager_.GetCurrentSnapshot();
    }

    /**
     * @brief 更新配置快照（RAII安全）
     * @param snapshot 新快照
     * @return bool 是否成功
     */
    bool UpdateSnapshot(const ConfigSnapshot::SnapshotPtr& snapshot) {
        if (!IsReady()) {
            return false;
        }
        
        std::unique_lock<std::shared_mutex> lock(state_mutex_);
        
        try {
            state_.store(ConfigLifecycleState::UPDATING);
            
            if (!snapshot) {
                throw ConfigLifecycleException("Invalid snapshot", ConfigLifecycleState::UPDATING);
            }
            
            if (!snapshot_manager_.AddSnapshot(snapshot)) {
                throw ConfigLifecycleException("Failed to add snapshot", ConfigLifecycleState::UPDATING);
            }
            
            last_update_time_ = std::chrono::steady_clock::now();
            ++update_count_;
            
            // 执行配置变更回调
            if (on_config_change_) {
                on_config_change_(snapshot->GetMetadata().version_id);
            }
            
            state_.store(ConfigLifecycleState::READY);
            return true;
            
        } catch (const std::exception& e) {
            state_.store(ConfigLifecycleState::READY);  // 恢复到就绪状态
            ++error_count_;
            throw ConfigLifecycleException(
                std::string("Update failed: ") + e.what(), 
                ConfigLifecycleState::READY);
        }
    }

    /**
     * @brief 回滚到指定版本（RAII安全）
     * @param version_id 目标版本ID
     * @return bool 是否成功
     */
    bool RollbackToVersion(const std::string& version_id) {
        if (!IsReady()) {
            return false;
        }
        
        std::unique_lock<std::shared_mutex> lock(state_mutex_);
        
        try {
            state_.store(ConfigLifecycleState::UPDATING);
            
            if (!snapshot_manager_.RollbackToVersion(version_id)) {
                throw ConfigLifecycleException("Rollback failed", ConfigLifecycleState::UPDATING);
            }
            
            last_update_time_ = std::chrono::steady_clock::now();
            ++update_count_;
            
            // 执行配置变更回调
            if (on_config_change_) {
                on_config_change_(version_id);
            }
            
            state_.store(ConfigLifecycleState::READY);
            return true;
            
        } catch (const std::exception& e) {
            state_.store(ConfigLifecycleState::READY);
            ++error_count_;
            throw ConfigLifecycleException(
                std::string("Rollback failed: ") + e.what(), 
                ConfigLifecycleState::READY);
        }
    }

    /**
     * @brief 获取快照管理器引用
     * @return ConfigSnapshotManager& 快照管理器引用
     */
    ConfigSnapshotManager& GetSnapshotManager() {
        return snapshot_manager_;
    }

    /**
     * @brief 获取快照管理器常量引用
     * @return const ConfigSnapshotManager& 快照管理器常量引用
     */
    const ConfigSnapshotManager& GetSnapshotManager() const {
        return snapshot_manager_;
    }

    /**
     * @brief 设置初始化回调
     * @param callback 回调函数
     */
    void SetInitializeCallback(std::function<void()> callback) {
        std::unique_lock<std::shared_mutex> lock(state_mutex_);
        on_initialize_ = std::move(callback);
    }

    /**
     * @brief 设置关闭回调
     * @param callback 回调函数
     */
    void SetShutdownCallback(std::function<void()> callback) {
        std::unique_lock<std::shared_mutex> lock(state_mutex_);
        on_shutdown_ = std::move(callback);
    }

    /**
     * @brief 设置配置变更回调
     * @param callback 回调函数
     */
    void SetConfigChangeCallback(std::function<void(const std::string&)> callback) {
        std::unique_lock<std::shared_mutex> lock(state_mutex_);
        on_config_change_ = std::move(callback);
    }

    /**
     * @brief 获取统计信息
     * @return std::string 统计信息字符串
     */
    std::string GetStatistics() const {
        std::shared_lock<std::shared_mutex> lock(state_mutex_);
        
        auto now = std::chrono::steady_clock::now();
        auto uptime = std::chrono::duration_cast<std::chrono::seconds>(now - init_time_).count();
        auto last_update_seconds = std::chrono::duration_cast<std::chrono::seconds>(
            now - last_update_time_).count();
        
        std::stringstream ss;
        ss << "Config Lifecycle Statistics:" << std::endl;
        ss << "  State: " << static_cast<int>(state_.load()) << std::endl;
        ss << "  Uptime: " << uptime << " seconds" << std::endl;
        ss << "  Update Count: " << update_count_.load() << std::endl;
        ss << "  Error Count: " << error_count_.load() << std::endl;
        ss << "  Last Update: " << last_update_seconds << " seconds ago" << std::endl;
        ss << "  Snapshot Count: " << snapshot_manager_.GetSnapshotCount() << std::endl;
        ss << "  Current Version: " << snapshot_manager_.GetCurrentVersionId() << std::endl;
        
        return ss.str();
    }
};

/**
 * @brief RAII配置访问器
 * 
 * 提供异常安全的配置访问，自动管理配置快照的生命周期
 */
class ConfigRAIIAccessor {
private:
    ConfigLifecycleManager* lifecycle_manager_;
    ConfigSnapshot::SnapshotPtr snapshot_;
    std::string accessor_id_;
    std::chrono::steady_clock::time_point access_time_;
    bool is_valid_;

public:
    /**
     * @brief 构造函数
     * @param lifecycle_manager 生命周期管理器
     * @param accessor_id 访问器ID（用于调试和监控）
     */
    explicit ConfigRAIIAccessor(
        ConfigLifecycleManager* lifecycle_manager,
        const std::string& accessor_id = "")
        : lifecycle_manager_(lifecycle_manager),
          accessor_id_(accessor_id.empty() ? "default_accessor" : accessor_id),
          access_time_(std::chrono::steady_clock::now()),
          is_valid_(false) {
        
        if (!lifecycle_manager_) {
            throw ConfigLifecycleException("Invalid lifecycle manager", ConfigLifecycleState::UNINITIALIZED);
        }
        
        try {
            snapshot_ = lifecycle_manager_->GetCurrentSnapshot();
            if (!snapshot_) {
                throw ConfigLifecycleException("Failed to get current snapshot", 
                    lifecycle_manager_->GetState());
            }
            
            is_valid_ = true;
            
        } catch (const std::exception& e) {
            is_valid_ = false;
            throw ConfigLifecycleException(
                std::string("Accessor initialization failed: ") + e.what(),
                lifecycle_manager_->GetState());
        }
    }
    
    /**
     * @brief 析构函数
     */
    ~ConfigRAIIAccessor() {
        is_valid_ = false;
        snapshot_.reset();  // 释放快照引用
    }

    // 禁止拷贝构造和赋值
    ConfigRAIIAccessor(const ConfigRAIIAccessor&) = delete;
    ConfigRAIIAccessor& operator=(const ConfigRAIIAccessor&) = delete;

    // 允许移动构造和赋值
    ConfigRAIIAccessor(ConfigRAIIAccessor&& other) noexcept
        : lifecycle_manager_(other.lifecycle_manager_),
          snapshot_(std::move(other.snapshot_)),
          accessor_id_(std::move(other.accessor_id_)),
          access_time_(other.access_time_),
          is_valid_(other.is_valid_) {
        other.is_valid_ = false;
    }
    
    ConfigRAIIAccessor& operator=(ConfigRAIIAccessor&& other) noexcept {
        if (this != &other) {
            lifecycle_manager_ = other.lifecycle_manager_;
            snapshot_ = std::move(other.snapshot_);
            accessor_id_ = std::move(other.accessor_id_);
            access_time_ = other.access_time_;
            is_valid_ = other.is_valid_;
            other.is_valid_ = false;
        }
        return *this;
    }

    /**
     * @brief 获取配置值
     * @param key 配置键
     * @param value 输出参数，配置值
     * @return bool 是否成功
     */
    bool GetValue(const std::string& key, ConfigValue& value) const {
        if (!is_valid_ || !snapshot_) {
            return false;
        }
        
        return snapshot_->GetValue(key, value);
    }

    /**
     * @brief 获取配置值（带默认值）
     * @param key 配置键
     * @param default_value 默认值
     * @return ConfigValue 配置值或默认值
     */
    ConfigValue GetValue(const std::string& key, const ConfigValue& default_value) const {
        ConfigValue value;
        if (GetValue(key, value)) {
            return value;
        }
        return default_value;
    }

    /**
     * @brief 检查配置键是否存在
     * @param key 配置键
     * @return bool 是否存在
     */
    bool HasKey(const std::string& key) const {
        if (!is_valid_ || !snapshot_) {
            return false;
        }
        
        return snapshot_->HasKey(key);
    }

    /**
     * @brief 获取所有配置键
     * @return std::vector<std::string> 配置键列表
     */
    std::vector<std::string> GetAllKeys() const {
        if (!is_valid_ || !snapshot_) {
            return {};
        }
        
        return snapshot_->GetAllKeys();
    }

    /**
     * @brief 获取访问器ID
     * @return std::string 访问器ID
     */
    const std::string& GetAccessorId() const {
        return accessor_id_;
    }

    /**
     * @brief 获取访问时间
     * @return std::chrono::steady_clock::time_point 访问时间
     */
    std::chrono::steady_clock::time_point GetAccessTime() const {
        return access_time_;
    }

    /**
     * @brief 检查是否有效
     * @return bool 是否有效
     */
    bool IsValid() const {
        return is_valid_ && snapshot_ != nullptr;
    }

    /**
     * @brief 获取当前快照版本
     * @return std::string 当前版本ID
     */
    std::string GetCurrentVersionId() const {
        if (!is_valid_ || !snapshot_) {
            return "";
        }
        
        return snapshot_->GetMetadata().version_id;
    }
};

/**
 * @brief 异常安全配置访问模板函数
 * 
 * 提供类型安全的配置访问，自动处理类型转换和异常
 */
template<typename T>
class SafeConfigAccessor {
public:
    /**
     * @brief 安全获取配置值
     * @param accessor RAII访问器
     * @param key 配置键
     * @param default_value 默认值
     * @return T 配置值或默认值
     */
    static T GetValue(const ConfigRAIIAccessor& accessor, const std::string& key, const T& default_value = T{}) {
        try {
            ConfigValue config_value;
            if (!accessor.GetValue(key, config_value)) {
                return default_value;
            }
            
            return ConvertConfigValue<T>(config_value, default_value);
            
        } catch (const std::exception& e) {
            // 记录错误日志（在实际实现中可以添加日志系统）
            return default_value;
        }
    }
    
    /**
     * @brief 安全设置配置值（验证和转换）
     * @param value 要设置的值
     * @return ConfigValue 转换后的配置值
     */
    static ConfigValue SetValue(const T& value) {
        return ConvertToConfigValue(value);
    }

private:
    /**
     * @brief 配置值类型转换模板函数
     */
    template<typename U>
    static U ConvertConfigValue(const ConfigValue& value, const U& default_value) {
        try {
            if constexpr (std::is_same_v<U, bool>) {
                if (std::holds_alternative<bool>(value)) {
                    return std::get<bool>(value);
                } else if (std::holds_alternative<std::string>(value)) {
                    const auto& str = std::get<std::string>(value);
                    return (str == "true" || str == "TRUE" || str == "True" || str == "1");
                }
            } else if constexpr (std::is_integral_v<U> && !std::is_same_v<U, bool>) {
                if (std::holds_alternative<int>(value)) {
                    return static_cast<U>(std::get<int>(value));
                } else if (std::holds_alternative<double>(value)) {
                    return static_cast<U>(std::get<double>(value));
                }
            } else if constexpr (std::is_floating_point_v<U>) {
                if (std::holds_alternative<double>(value)) {
                    return static_cast<U>(std::get<double>(value));
                } else if (std::holds_alternative<int>(value)) {
                    return static_cast<U>(std::get<int>(value));
                }
            } else if constexpr (std::is_same_v<U, std::string>) {
                if (std::holds_alternative<std::string>(value)) {
                    return std::get<std::string>(value);
                } else if (std::holds_alternative<bool>(value)) {
                    return std::get<bool>(value) ? "true" : "false";
                } else if (std::holds_alternative<int>(value)) {
                    return std::to_string(std::get<int>(value));
                } else if (std::holds_alternative<double>(value)) {
                    return std::to_string(std::get<double>(value));
                }
            }
        } catch (const std::exception& e) {
            // 转换失败，返回默认值
        }
        
        return default_value;
    }
    
    /**
     * @brief 转换为配置值
     */
    template<typename U>
    static ConfigValue ConvertToConfigValue(const U& value) {
        if constexpr (std::is_same_v<U, bool>) {
            return value;
        } else if constexpr (std::is_integral_v<U> && !std::is_same_v<U, bool>) {
            return static_cast<int>(value);
        } else if constexpr (std::is_floating_point_v<U>) {
            return static_cast<double>(value);
        } else if constexpr (std::is_same_v<U, std::string>) {
            return value;
        } else {
            // 默认转换为字符串
            return std::to_string(value);
        }
    }
};



}  // namespace sqlcc

#endif  // SQLCC_CONFIG_LIFECYCLE_H_
