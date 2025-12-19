/**
 * @file config_lifecycle.cpp
 * @brief RAII配置生命周期管理器实现文件
 * 
 * Why: 实现RAII模式的配置生命周期管理，确保资源自动清理
 * What: 提供ConfigLifecycleManager类和相关的RAII配置访问功能的实现
 * How: 使用RAII模式管理配置资源的获取和释放
 */

#include "utils/config_lifecycle.h"
#include <sstream>

namespace sqlcc {

// ConfigLifecycleManager 实现

bool ConfigLifecycleManager::Initialize(const std::string& config_path) {
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

bool ConfigLifecycleManager::Shutdown() {
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

ConfigLifecycleState ConfigLifecycleManager::GetState() const {
    return state_.load();
}

bool ConfigLifecycleManager::IsReady() const {
    return state_.load() == ConfigLifecycleState::READY;
}

ConfigSnapshot::SnapshotPtr ConfigLifecycleManager::GetCurrentSnapshot() const {
    if (!IsReady()) {
        throw ConfigLifecycleException("Config manager not ready", state_.load());
    }
    
    std::shared_lock<std::shared_mutex> lock(state_mutex_);
    return snapshot_manager_.GetCurrentSnapshot();
}

bool ConfigLifecycleManager::UpdateSnapshot(const ConfigSnapshot::SnapshotPtr& snapshot) {
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

bool ConfigLifecycleManager::RollbackToVersion(const std::string& version_id) {
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

ConfigSnapshotManager& ConfigLifecycleManager::GetSnapshotManager() {
    return snapshot_manager_;
}

const ConfigSnapshotManager& ConfigLifecycleManager::GetSnapshotManager() const {
    return snapshot_manager_;
}

void ConfigLifecycleManager::SetInitializeCallback(std::function<void()> callback) {
    std::unique_lock<std::shared_mutex> lock(state_mutex_);
    on_initialize_ = std::move(callback);
}

void ConfigLifecycleManager::SetShutdownCallback(std::function<void()> callback) {
    std::unique_lock<std::shared_mutex> lock(state_mutex_);
    on_shutdown_ = std::move(callback);
}

void ConfigLifecycleManager::SetConfigChangeCallback(std::function<void(const std::string&)> callback) {
    std::unique_lock<std::shared_mutex> lock(state_mutex_);
    on_config_change_ = std::move(callback);
}

std::string ConfigLifecycleManager::GetStatistics() const {
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

// 工具函数实现

std::string GenerateVersionId(const std::string& prefix) {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << prefix << std::put_time(std::localtime(&time_t), "%Y%m%d_%H%M%S") 
       << "_" << std::setfill('0') << std::setw(3) << ms.count();
    
    return ss.str();
}

std::string FormatConfigValue(const ConfigValue& value) {
    return std::visit([](const auto& val) -> std::string {
        if constexpr (std::is_same_v<std::decay_t<decltype(val)>, bool>) {
            return val ? "true" : "false";
        } else if constexpr (std::is_same_v<std::decay_t<decltype(val)>, std::string>) {
            return "\"" + val + "\"";
        } else {
            return std::to_string(val);
        }
    }, value);
}

bool ParseConfigValue(const std::string& str, ConfigValue& value) {
    std::string trimmed_str = str;
    
    // 去除前后空格
    trimmed_str.erase(0, trimmed_str.find_first_not_of(" \t\r\n"));
    trimmed_str.erase(trimmed_str.find_last_not_of(" \t\r\n") + 1);
    
    // 处理布尔值
    if (trimmed_str == "true" || trimmed_str == "TRUE" || trimmed_str == "True") {
        value = true;
        return true;
    } else if (trimmed_str == "false" || trimmed_str == "FALSE" || trimmed_str == "False") {
        value = false;
        return true;
    }
    
    // 处理整数
    try {
        size_t pos;
        int int_val = std::stoi(trimmed_str, &pos);
        if (pos == trimmed_str.length()) {
            value = int_val;
            return true;
        }
    } catch (...) {
        // 不是整数，继续尝试其他类型
    }
    
    // 处理浮点数
    try {
        size_t pos;
        double double_val = std::stod(trimmed_str, &pos);
        if (pos == trimmed_str.length()) {
            value = double_val;
            return true;
        }
    } catch (...) {
        // 不是浮点数，作为字符串处理
    }
    
    // 作为字符串处理
    value = trimmed_str;
    return true;
}

}  // namespace sqlcc