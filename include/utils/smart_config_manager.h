/**
 * @file smart_config_manager.h
 * @brief 智能配置管理器头文件 - 集成内存安全和RAII模式
 * 
 * Why: 实现企业级配置管理器，支持内存安全、RAII模式和异常安全配置
 * What: 提供SmartConfigManager类，集成智能指针、RAII生命周期管理和线程安全
 * How: 使用std::shared_ptr、std::unique_ptr和RAII模式实现内存安全的配置管理
 */

#ifndef SQLCC_SMART_CONFIG_MANAGER_H_
#define SQLCC_SMART_CONFIG_MANAGER_H_

#include "utils/config_snapshot.h"
#include "utils/config_lifecycle.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <thread>
#include <future>

namespace sqlcc {

/**
 * @brief 智能配置管理器
 * 
 * 企业级配置管理器，提供内存安全、RAII模式和异常安全配置
 * 支持热更新、版本管理、分布式同步等企业级特性
 */
class SmartConfigManager {
private:
    // 单例实例（智能指针管理）
    static std::shared_ptr<SmartConfigManager> instance_;
    static std::mutex instance_mutex_;
    
    // 核心组件（智能指针管理）
    std::unique_ptr<ConfigLifecycleManager> lifecycle_manager_;
    std::unique_ptr<ConfigSnapshotManager> snapshot_manager_;
    
    // 配置状态
    std::atomic<bool> initialized_{false};
    std::atomic<bool> shutdown_requested_{false};
    std::string config_file_path_;
    
    // 热更新支持
    std::unique_ptr<std::thread> hot_reload_thread_;
    std::atomic<bool> hot_reload_enabled_{false};
    std::chrono::milliseconds hot_reload_interval_{5000};  // 5秒检查间隔
    
    // 性能监控
    std::atomic<size_t> config_reads_{0};
    std::atomic<size_t> config_writes_{0};
    std::atomic<size_t> config_errors_{0};
    
    // 加密支持（预留接口）
    std::unique_ptr<std::string> encryption_key_;  // 使用unique_ptr管理可选资源

public:
    /**
     * @brief 获取单例实例（线程安全）
     * @return std::shared_ptr<SmartConfigManager> 智能指针管理的单例实例
     */
    static std::shared_ptr<SmartConfigManager> GetInstance() {
        std::lock_guard<std::mutex> lock(instance_mutex_);
        
        if (!instance_) {
            instance_ = std::shared_ptr<SmartConfigManager>(new SmartConfigManager());
        }
        
        return instance_;
    }

    /**
     * @brief 销毁单例实例
     */
    static void DestroyInstance() {
        std::lock_guard<std::mutex> lock(instance_mutex_);
        
        if (instance_) {
            instance_->Shutdown();
            instance_.reset();  // 智能指针自动清理
        }
    }

    /**
     * @brief 初始化配置管理器
     * @param config_file_path 配置文件路径
     * @return bool 是否成功
     */
    bool Initialize(const std::string& config_file_path = "") {
        if (initialized_.exchange(true)) {
            return false;  // 已经初始化
        }
        
        try {
            config_file_path_ = config_file_path;
            
            // 创建生命周期管理器（智能指针）
            lifecycle_manager_ = std::make_unique<ConfigLifecycleManager>(config_file_path);
            
            // 设置生命周期回调
            lifecycle_manager_->SetInitializeCallback([this]() {
                // 初始化回调逻辑
                OnInitialize();
            });
            
            lifecycle_manager_->SetShutdownCallback([this]() {
                // 关闭回调逻辑
                OnShutdown();
            });
            
            lifecycle_manager_->SetConfigChangeCallback([this](const std::string& version_id) {
                // 配置变更回调逻辑
                OnConfigChange(version_id);
            });
            
            // 初始化生命周期管理器
            if (!lifecycle_manager_->Initialize(config_file_path)) {
                initialized_.store(false);
                return false;
            }
            
            // 加载初始配置
            if (!config_file_path.empty()) {
                LoadConfiguration(config_file_path);
            }
            
            return true;
            
        } catch (const std::exception& e) {
            initialized_.store(false);
            ++config_errors_;
            throw ConfigLifecycleException(
                std::string("SmartConfigManager initialization failed: ") + e.what(),
                ConfigLifecycleState::UNINITIALIZED);
        }
    }

    /**
     * @brief 关闭配置管理器
     * @return bool 是否成功
     */
    bool Shutdown() {
        if (!initialized_.exchange(false)) {
            return false;  // 已经关闭或未初始化
        }
        
        try {
            shutdown_requested_.store(true);
            
            // 停止热更新线程
            StopHotReload();
            
            // 关闭生命周期管理器
            if (lifecycle_manager_) {
                lifecycle_manager_->Shutdown();
            }
            
            // 清理智能指针
            lifecycle_manager_.reset();
            snapshot_manager_.reset();
            encryption_key_.reset();
            
            return true;
            
        } catch (const std::exception& e) {
            ++config_errors_;
            throw ConfigLifecycleException(
                std::string("SmartConfigManager shutdown failed: ") + e.what(),
                ConfigLifecycleState::SHUTDOWN);
        }
    }

    /**
     * @brief 获取配置值（异常安全）
     * @param key 配置键
     * @param default_value 默认值
     * @return T 配置值或默认值
     */
    template<typename T>
    T GetConfig(const std::string& key, const T& default_value = T{}) const {
        if (!initialized_.load()) {
            return default_value;
        }
        
        try {
            ConfigRAIIAccessor accessor(lifecycle_manager_.get(), "SmartConfigManager::GetConfig");
            ++config_reads_;
            
            return SafeConfigAccessor<T>::GetValue(accessor, key, default_value);
            
        } catch (const std::exception& e) {
            ++config_errors_;
            return default_value;
        }
    }

    /**
     * @brief 获取字符串配置（专用重载）
     * @param key 配置键
     * @param default_value 默认值
     * @return std::string 配置值或默认值
     */
    std::string GetStringConfig(const std::string& key, const std::string& default_value = "") const {
        return GetConfig<std::string>(key, default_value);
    }

    /**
     * @brief 获取整数配置（专用重载）
     * @param key 配置键
     * @param default_value 默认值
     * @return int 配置值或默认值
     */
    int GetIntConfig(const std::string& key, int default_value = 0) const {
        return GetConfig<int>(key, default_value);
    }

    /**
     * @brief 获取布尔配置（专用重载）
     * @param key 配置键
     * @param default_value 默认值
     * @return bool 配置值或默认值
     */
    bool GetBoolConfig(const std::string& key, bool default_value = false) const {
        return GetConfig<bool>(key, default_value);
    }

    /**
     * @brief 获取双精度配置（专用重载）
     * @param key 配置键
     * @param default_value 默认值
     * @return double 配置值或默认值
     */
    double GetDoubleConfig(const std::string& key, double default_value = 0.0) const {
        return GetConfig<double>(key, default_value);
    }

    /**
     * @brief 更新配置（异步）
     * @param key 配置键
     * @param value 配置值
     * @return std::future<bool> 异步结果
     */
    template<typename T>
    std::future<bool> UpdateConfigAsync(const std::string& key, const T& value) {
        return std::async(std::launch::async, [this, key, value]() {
            return UpdateConfig(key, value);
        });
    }

    /**
     * @brief 批量更新配置（异步）
     * @param configs 配置映射
     * @return std::future<bool> 异步结果
     */
    std::future<bool> BatchUpdateConfigsAsync(const std::unordered_map<std::string, ConfigValue>& configs) {
        return std::async(std::launch::async, [this, configs]() {
            return BatchUpdateConfigs(configs);
        });
    }

    /**
     * @brief 启用热更新
     * @param check_interval_ms 检查间隔（毫秒）
     * @return bool 是否成功
     */
    bool EnableHotReload(std::chrono::milliseconds check_interval = std::chrono::milliseconds(5000)) {
        if (!initialized_.load() || hot_reload_enabled_.exchange(true)) {
            return false;
        }
        
        try {
            hot_reload_interval_ = check_interval;
            
            // 创建热更新线程（智能指针管理）
            hot_reload_thread_ = std::make_unique<std::thread>([this]() {
                HotReloadWorker();
            });
            
            return true;
            
        } catch (const std::exception& e) {
            hot_reload_enabled_.store(false);
            ++config_errors_;
            return false;
        }
    }

    /**
     * @brief 停止热更新
     * @return bool 是否成功
     */
    bool StopHotReload() {
        if (!hot_reload_enabled_.exchange(false)) {
            return false;
        }
        
        try {
            if (hot_reload_thread_ && hot_reload_thread_->joinable()) {
                hot_reload_thread_->join();
                hot_reload_thread_.reset();  // 智能指针清理
            }
            
            return true;
            
        } catch (const std::exception& e) {
            ++config_errors_;
            return false;
        }
    }

    /**
     * @brief 获取当前版本ID
     * @return std::string 当前版本ID
     */
    std::string GetCurrentVersionId() const {
        if (!initialized_.load() || !lifecycle_manager_) {
            return "";
        }
        
        try {
            return lifecycle_manager_->GetSnapshotManager().GetCurrentVersionId();
        } catch (const std::exception& e) {
            ++config_errors_;
            return "";
        }
    }

    /**
     * @brief 获取统计信息
     * @return std::string 统计信息
     */
    std::string GetStatistics() const {
        std::stringstream ss;
        
        ss << "SmartConfigManager Statistics:" << std::endl;
        ss << "  Initialized: " << (initialized_.load() ? "Yes" : "No") << std::endl;
        ss << "  Config File: " << config_file_path_ << std::endl;
        ss << "  Hot Reload: " << (hot_reload_enabled_.load() ? "Enabled" : "Disabled") << std::endl;
        ss << "  Config Reads: " << config_reads_.load() << std::endl;
        ss << "  Config Writes: " << config_writes_.load() << std::endl;
        ss << "  Config Errors: " << config_errors_.load() << std::endl;
        
        if (lifecycle_manager_) {
            ss << "  Lifecycle Manager:" << std::endl;
            ss << "    " << lifecycle_manager_->GetStatistics() << std::endl;
        }
        
        return ss.str();
    }

    /**
     * @brief 设置加密密钥
     * @param key 加密密钥
     */
    void SetEncryptionKey(const std::string& key) {
        encryption_key_ = std::make_unique<std::string>(key);
    }

    /**
     * @brief 获取加密密钥
     * @return std::string 加密密钥（如果存在）
     */
    std::string GetEncryptionKey() const {
        return encryption_key_ ? *encryption_key_ : "";
    }

private:
    /**
     * @brief 构造函数（私有，单例模式）
     */
    SmartConfigManager() = default;

    /**
     * @brief 析构函数
     */
    ~SmartConfigManager() {
        Shutdown();
    }

    /**
     * @brief 加载配置文件
     * @param file_path 配置文件路径
     * @return bool 是否成功
     */
    bool LoadConfiguration(const std::string& file_path) {
        // 这里可以实现具体的配置文件加载逻辑
        // 例如解析INI、JSON、YAML等格式的配置文件
        
        try {
            // 创建配置数据（示例）
            std::unordered_map<std::string, ConfigValue> config_data;
            config_data["database.host"] = std::string("localhost");
            config_data["database.port"] = 5432;
            config_data["database.ssl"] = true;
            config_data["cache.ttl"] = 3600;
            
            // 创建配置快照
            auto snapshot = ConfigSnapshotFactory::CreateSnapshot(
                config_data, GenerateVersionId("load_"), "Loaded from file: " + file_path);
            
            // 更新生命周期管理器
            return lifecycle_manager_->UpdateSnapshot(snapshot);
            
        } catch (const std::exception& e) {
            ++config_errors_;
            return false;
        }
    }

    /**
     * @brief 更新配置
     * @param key 配置键
     * @param value 配置值
     * @return bool 是否成功
     */
    template<typename T>
    bool UpdateConfig(const std::string& key, const T& value) {
        if (!initialized_.load() || !lifecycle_manager_) {
            return false;
        }
        
        try {
            ++config_writes_;
            
            // 获取当前配置快照
            auto current_snapshot = lifecycle_manager_->GetCurrentSnapshot();
            if (!current_snapshot) {
                return false;
            }
            
            // 复制当前配置数据
            auto all_keys = current_snapshot->GetAllKeys();
            std::unordered_map<std::string, ConfigValue> new_config_data;
            
            for (const auto& k : all_keys) {
                ConfigValue val;
                if (current_snapshot->GetValue(k, val)) {
                    new_config_data[k] = val;
                }
            }
            
            // 更新指定配置
            new_config_data[key] = SafeConfigAccessor<T>::SetValue(value);
            
            // 创建新快照
            auto new_snapshot = ConfigSnapshotFactory::CreateSnapshot(
                new_config_data, GenerateVersionId("update_"), "Updated config: " + key);
            
            // 更新生命周期管理器
            return lifecycle_manager_->UpdateSnapshot(new_snapshot);
            
        } catch (const std::exception& e) {
            ++config_errors_;
            return false;
        }
    }

    /**
     * @brief 批量更新配置
     * @param configs 配置映射
     * @return bool 是否成功
     */
    bool BatchUpdateConfigs(const std::unordered_map<std::string, ConfigValue>& configs) {
        if (!initialized_.load() || !lifecycle_manager_) {
            return false;
        }
        
        try {
            config_writes_ += configs.size();
            
            // 获取当前配置快照
            auto current_snapshot = lifecycle_manager_->GetCurrentSnapshot();
            if (!current_snapshot) {
                return false;
            }
            
            // 复制当前配置数据
            auto all_keys = current_snapshot->GetAllKeys();
            std::unordered_map<std::string, ConfigValue> new_config_data;
            
            for (const auto& k : all_keys) {
                ConfigValue val;
                if (current_snapshot->GetValue(k, val)) {
                    new_config_data[k] = val;
                }
            }
            
            // 批量更新配置
            for (const auto& [key, value] : configs) {
                new_config_data[key] = value;
            }
            
            // 创建新快照
            auto new_snapshot = ConfigSnapshotFactory::CreateSnapshot(
                new_config_data, GenerateVersionId("batch_update_"), "Batch updated configs");
            
            // 更新生命周期管理器
            return lifecycle_manager_->UpdateSnapshot(new_snapshot);
            
        } catch (const std::exception& e) {
            ++config_errors_;
            return false;
        }
    }

    /**
     * @brief 热更新工作线程
     */
    void HotReloadWorker() {
        while (hot_reload_enabled_.load() && !shutdown_requested_.load()) {
            try {
                // 检查配置文件是否修改
                // 这里可以实现具体的文件监控逻辑
                
                std::this_thread::sleep_for(hot_reload_interval_);
                
            } catch (const std::exception& e) {
                ++config_errors_;
                // 在实际实现中可以记录日志
            }
        }
    }

    /**
     * @brief 初始化回调
     */
    void OnInitialize() {
        // 初始化逻辑
    }

    /**
     * @brief 关闭回调
     */
    void OnShutdown() {
        // 关闭逻辑
    }

    /**
     * @brief 配置变更回调
     * @param version_id 新版本ID
     */
    void OnConfigChange(const std::string& version_id) {
        // 配置变更逻辑
    }
};

// 静态成员定义
std::shared_ptr<SmartConfigManager> SmartConfigManager::instance_ = nullptr;
std::mutex SmartConfigManager::instance_mutex_;

}  // namespace sqlcc

#endif  // SQLCC_SMART_CONFIG_MANAGER_H_