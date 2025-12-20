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
    // 单例实例（指针管理）
    static SmartConfigManager* instance_;
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
    mutable std::atomic<size_t> config_reads_{0};
    mutable std::atomic<size_t> config_writes_{0};
    mutable std::atomic<size_t> config_errors_{0};
    
    // 加密支持（预留接口）
    std::unique_ptr<std::string> encryption_key_;  // 使用unique_ptr管理可选资源

public:
    /**
     * @brief 获取单例实例（线程安全）
     * @return SmartConfigManager* 单例实例指针
     */
    static SmartConfigManager* GetInstance() {
        std::lock_guard<std::mutex> lock(instance_mutex_);

        if (!instance_) {
            instance_ = new SmartConfigManager();
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
            delete instance_;
            instance_ = nullptr;
        }
    }

    /**
     * @brief 初始化配置管理器
     * @param config_file_path 配置文件路径
     * @return bool 是否成功
     */
    bool Initialize(const std::string& config_file_path = "");

    /**
     * @brief 关闭配置管理器
     * @return bool 是否成功
     */
    bool Shutdown();

    /**
     * @brief 获取配置值（异常安全）
     * @param key 配置键
     * @param default_value 默认值
     * @return T 配置值或默认值
     */
    template<typename T>
    T GetConfig(const std::string& key, const T& default_value = T{}) const;

    /**
     * @brief 获取字符串配置（专用重载）
     * @param key 配置键
     * @param default_value 默认值
     * @return std::string 配置值或默认值
     */
    std::string GetStringConfig(const std::string& key, const std::string& default_value = "") const;

    /**
     * @brief 获取整数配置（专用重载）
     * @param key 配置键
     * @param default_value 默认值
     * @return int 配置值或默认值
     */
    int GetIntConfig(const std::string& key, int default_value = 0) const;

    /**
     * @brief 获取布尔配置（专用重载）
     * @param key 配置键
     * @param default_value 默认值
     * @return bool 配置值或默认值
     */
    bool GetBoolConfig(const std::string& key, bool default_value = false) const;

    /**
     * @brief 获取双精度配置（专用重载）
     * @param key 配置键
     * @param default_value 默认值
     * @return double 配置值或默认值
     */
    double GetDoubleConfig(const std::string& key, double default_value = 0.0) const;

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
    std::future<bool> BatchUpdateConfigsAsync(const std::unordered_map<std::string, ConfigValue>& configs);

    /**
     * @brief 启用热更新
     * @param check_interval_ms 检查间隔（毫秒）
     * @return bool 是否成功
     */
    bool EnableHotReload(std::chrono::milliseconds check_interval = std::chrono::milliseconds(5000));

    /**
     * @brief 停止热更新
     * @return bool 是否成功
     */
    bool StopHotReload();

    /**
     * @brief 获取当前版本ID
     * @return std::string 当前版本ID
     */
    std::string GetCurrentVersionId() const;

    /**
     * @brief 获取统计信息
     * @return std::string 统计信息
     */
    std::string GetStatistics() const;

    /**
     * @brief 设置加密密钥
     * @param key 加密密钥
     */
    void SetEncryptionKey(const std::string& key);

    /**
     * @brief 获取加密密钥
     * @return std::string 加密密钥（如果存在）
     */
    std::string GetEncryptionKey() const;

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
    bool LoadConfiguration(const std::string& file_path);

    /**
     * @brief 更新配置
     * @param key 配置键
     * @param value 配置值
     * @return bool 是否成功
     */
    template<typename T>
    bool UpdateConfig(const std::string& key, const T& value);

    /**
     * @brief 批量更新配置
     * @param configs 配置映射
     * @return bool 是否成功
     */
    bool BatchUpdateConfigs(const std::unordered_map<std::string, ConfigValue>& configs);

    /**
     * @brief 热更新工作线程
     */
    void HotReloadWorker();

    /**
     * @brief 初始化回调
     */
    void OnInitialize();

    /**
     * @brief 关闭回调
     */
    void OnShutdown();

    /**
     * @brief 配置变更回调
     * @param version_id 新版本ID
     */
    void OnConfigChange(const std::string& version_id);
};

// 静态成员定义
SmartConfigManager* SmartConfigManager::instance_ = nullptr;
std::mutex SmartConfigManager::instance_mutex_;

}  // namespace sqlcc

#endif  // SQLCC_SMART_CONFIG_MANAGER_H_
