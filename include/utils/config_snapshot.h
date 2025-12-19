/**
 * @file config_snapshot.h
 * @brief 智能配置快照管理器头文件
 * 
 * Why: 需要实现线程安全的配置快照机制，支持无锁配置读取和版本管理
 * What: 提供ConfigSnapshot类和相关的智能指针管理功能
 * How: 使用std::shared_ptr实现配置快照的共享和自动生命周期管理
 */

#ifndef SQLCC_CONFIG_SNAPSHOT_H_
#define SQLCC_CONFIG_SNAPSHOT_H_

#include <string>
#include <unordered_map>
#include <variant>
#include <memory>
#include <chrono>
#include <atomic>
#include <shared_mutex>

namespace sqlcc {

/**
 * @brief 配置值类型，支持多种数据类型
 * 使用std::variant实现类型安全的配置值存储
 */
using ConfigValue = std::variant<bool, int, double, std::string>;

/**
 * @brief 配置快照类
 * 
 * 提供不可变的配置数据快照，支持线程安全的配置读取和版本管理
 * 使用智能指针管理，确保内存安全和自动资源清理
 */
class ConfigSnapshot {
public:
    /**
     * @brief 智能指针类型定义
     */
    using SnapshotPtr = std::shared_ptr<const ConfigSnapshot>;
    using MutableSnapshotPtr = std::shared_ptr<ConfigSnapshot>;
    using WeakSnapshotPtr = std::weak_ptr<const ConfigSnapshot>;

    /**
     * @brief 快照元数据结构
     */
    struct SnapshotMetadata {
        std::string version_id;                    // 版本ID
        std::string description;                   // 描述信息
        std::chrono::system_clock::time_point create_time;  // 创建时间
        std::string parent_version;               // 父版本ID
        size_t config_count;                      // 配置项数量
        std::string checksum;                     // 数据校验和
    };

private:
    /**
     * @brief 配置数据（不可变）
     */
    const std::unordered_map<std::string, ConfigValue> config_data_;
    
    /**
     * @brief 快照元数据
     */
    const SnapshotMetadata metadata_;
    
    /**
     * @brief 原子引用计数（用于调试和监控）
     */
    mutable std::atomic<int> access_count_{0};

public:
    /**
     * @brief 构造函数
     * @param config_data 配置数据
     * @param metadata 元数据
     */
    explicit ConfigSnapshot(
        const std::unordered_map<std::string, ConfigValue>& config_data,
        const SnapshotMetadata& metadata)
        : config_data_(config_data), metadata_(metadata) {}

    /**
     * @brief 默认析构函数
     */
    ~ConfigSnapshot() = default;

    // 禁止拷贝构造和赋值（使用智能指针管理）
    ConfigSnapshot(const ConfigSnapshot&) = delete;
    ConfigSnapshot& operator=(const ConfigSnapshot&) = delete;

    // 允许移动构造和赋值
    ConfigSnapshot(ConfigSnapshot&&) noexcept = default;
    ConfigSnapshot& operator=(ConfigSnapshot&&) noexcept = default;

    /**
     * @brief 获取配置值
     * @param key 配置键
     * @param value 输出参数，配置值
     * @return bool 是否找到配置
     */
    bool GetValue(const std::string& key, ConfigValue& value) const {
        access_count_.fetch_add(1, std::memory_order_relaxed);
        
        auto it = config_data_.find(key);
        if (it != config_data_.end()) {
            value = it->second;
            return true;
        }
        return false;
    }

    /**
     * @brief 检查配置键是否存在
     * @param key 配置键
     * @return bool 是否存在
     */
    bool HasKey(const std::string& key) const {
        access_count_.fetch_add(1, std::memory_order_relaxed);
        return config_data_.find(key) != config_data_.end();
    }

    /**
     * @brief 获取所有配置键
     * @return std::vector<std::string> 配置键列表
     */
    std::vector<std::string> GetAllKeys() const {
        access_count_.fetch_add(1, std::memory_order_relaxed);
        
        std::vector<std::string> keys;
        keys.reserve(config_data_.size());
        
        for (const auto& [key, value] : config_data_) {
            keys.push_back(key);
        }
        
        return keys;
    }

    /**
     * @brief 获取指定前缀的配置键
     * @param prefix 键前缀
     * @return std::vector<std::string> 匹配的配置键列表
     */
    std::vector<std::string> GetKeysWithPrefix(const std::string& prefix) const {
        access_count_.fetch_add(1, std::memory_order_relaxed);
        
        std::vector<std::string> keys;
        
        for (const auto& [key, value] : config_data_) {
            if (key.find(prefix) == 0) {
                keys.push_back(key);
            }
        }
        
        return keys;
    }

    /**
     * @brief 获取配置数量
     * @return size_t 配置项数量
     */
    size_t GetConfigCount() const {
        return config_data_.size();
    }

    /**
     * @brief 获取快照元数据
     * @return const SnapshotMetadata& 元数据引用
     */
    const SnapshotMetadata& GetMetadata() const {
        return metadata_;
    }

    /**
     * @brief 获取访问计数（用于调试和监控）
     * @return int 访问次数
     */
    int GetAccessCount() const {
        return access_count_.load(std::memory_order_relaxed);
    }

    /**
     * @brief 计算数据校验和
     * @return std::string 校验和
     */
    std::string CalculateChecksum() const {
        // 简单的校验和计算，实际项目中可以使用更复杂的哈希算法
        size_t hash = 0;
        for (const auto& [key, value] : config_data_) {
            std::hash<std::string> hasher;
            hash ^= hasher(key) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            
            // 对配置值也进行哈希
            std::visit([&hash, &hasher](const auto& val) {
                if constexpr (std::is_same_v<std::decay_t<decltype(val)>, std::string>) {
                    hash ^= hasher(val) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
                } else {
                    hash ^= std::hash<std::decay_t<decltype(val)>>{}(val) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
                }
            }, value);
        }
        
        return std::to_string(hash);
    }

    /**
     * @brief 验证快照完整性
     * @return bool 是否有效
     */
    bool ValidateIntegrity() const {
        return CalculateChecksum() == metadata_.checksum;
    }

    /**
     * @brief 创建快照的深拷贝
     * @return SnapshotPtr 新快照的智能指针
     */
    SnapshotPtr Clone() const {
        access_count_.fetch_add(1, std::memory_order_relaxed);
        
        // 创建新的元数据，保持版本ID但更新时间
        SnapshotMetadata new_metadata = metadata_;
        new_metadata.create_time = std::chrono::system_clock::now();
        new_metadata.parent_version = metadata_.version_id;
        
        return std::make_shared<ConfigSnapshot>(config_data_, new_metadata);
    }

    /**
     * @brief 比较两个快照是否相等
     * @param other 另一个快照
     * @return bool 是否相等
     */
    bool Equals(const ConfigSnapshot& other) const {
        return config_data_ == other.config_data_ && 
               metadata_.version_id == other.metadata_.version_id;
    }
};

/**
 * @brief 配置快照工厂类
 * 
 * 提供创建和管理配置快照的工厂方法
 */
class ConfigSnapshotFactory {
public:
    /**
     * @brief 从配置映射创建快照
     * @param config_data 配置数据
     * @param version_id 版本ID
     * @param description 描述信息
     * @return ConfigSnapshot::SnapshotPtr 快照智能指针
     */
    static ConfigSnapshot::SnapshotPtr CreateSnapshot(
        const std::unordered_map<std::string, ConfigValue>& config_data,
        const std::string& version_id,
        const std::string& description = "") {
        
        ConfigSnapshot::SnapshotMetadata metadata;
        metadata.version_id = version_id;
        metadata.description = description;
        metadata.create_time = std::chrono::system_clock::now();
        metadata.config_count = config_data.size();
        
        // 创建临时快照以计算校验和
        ConfigSnapshot temp_snapshot(config_data, metadata);
        metadata.checksum = temp_snapshot.CalculateChecksum();
        
        return std::make_shared<ConfigSnapshot>(config_data, metadata);
    }

    /**
     * @brief 创建空快照
     * @param version_id 版本ID
     * @param description 描述信息
     * @return ConfigSnapshot::SnapshotPtr 快照智能指针
     */
    static ConfigSnapshot::SnapshotPtr CreateEmptySnapshot(
        const std::string& version_id,
        const std::string& description = "") {
        
        std::unordered_map<std::string, ConfigValue> empty_data;
        return CreateSnapshot(empty_data, version_id, description);
    }

    /**
     * @brief 合并多个快照
     * @param base_snapshot 基础快照
     * @param override_snapshot 覆盖快照
     * @param new_version_id 新版本ID
     * @param description 描述信息
     * @return ConfigSnapshot::SnapshotPtr 合并后的快照
     */
    static ConfigSnapshot::SnapshotPtr MergeSnapshots(
        const ConfigSnapshot::SnapshotPtr& base_snapshot,
        const ConfigSnapshot::SnapshotPtr& override_snapshot,
        const std::string& new_version_id,
        const std::string& description = "") {
        
        if (!base_snapshot) {
            return override_snapshot;
        }
        
        if (!override_snapshot) {
            return base_snapshot;
        }
        
        // 获取基础配置数据
        auto base_keys = base_snapshot->GetAllKeys();
        std::unordered_map<std::string, ConfigValue> merged_data;
        
        // 复制基础配置
        for (const auto& key : base_keys) {
            ConfigValue value;
            if (base_snapshot->GetValue(key, value)) {
                merged_data[key] = value;
            }
        }
        
        // 应用覆盖配置
        auto override_keys = override_snapshot->GetAllKeys();
        for (const auto& key : override_keys) {
            ConfigValue value;
            if (override_snapshot->GetValue(key, value)) {
                merged_data[key] = value;  // 覆盖或新增
            }
        }
        
        return CreateSnapshot(merged_data, new_version_id, description);
    }
};

/**
 * @brief 配置快照管理器
 * 
 * 提供快照的存储、检索和管理功能
 */
class ConfigSnapshotManager {
private:
    mutable std::shared_mutex snapshots_mutex_;
    std::unordered_map<std::string, ConfigSnapshot::SnapshotPtr> snapshots_;
    ConfigSnapshot::SnapshotPtr current_snapshot_;
    std::string current_version_id_;
    
    // 快照历史记录（用于回滚）
    std::vector<std::string> version_history_;
    static constexpr size_t kMaxHistorySize = 100;

public:
    ConfigSnapshotManager() = default;
    ~ConfigSnapshotManager() = default;
    
    // 禁止拷贝，允许移动
    ConfigSnapshotManager(const ConfigSnapshotManager&) = delete;
    ConfigSnapshotManager& operator=(const ConfigSnapshotManager&) = delete;
    
    ConfigSnapshotManager(ConfigSnapshotManager&&) noexcept = default;
    ConfigSnapshotManager& operator=(ConfigSnapshotManager&&) noexcept = default;

    /**
     * @brief 添加快照
     * @param snapshot 快照智能指针
     * @return bool 是否成功
     */
    bool AddSnapshot(const ConfigSnapshot::SnapshotPtr& snapshot) {
        if (!snapshot) {
            return false;
        }
        
        std::unique_lock<std::shared_mutex> lock(snapshots_mutex_);
        
        const auto& version_id = snapshot->GetMetadata().version_id;
        snapshots_[version_id] = snapshot;
        
        // 更新当前快照
        current_snapshot_ = snapshot;
        current_version_id_ = version_id;
        
        // 更新版本历史
        version_history_.push_back(version_id);
        if (version_history_.size() > kMaxHistorySize) {
            version_history_.erase(version_history_.begin());
        }
        
        return true;
    }

    /**
     * @brief 获取快照
     * @param version_id 版本ID
     * @return ConfigSnapshot::SnapshotPtr 快照智能指针，不存在时返回nullptr
     */
    ConfigSnapshot::SnapshotPtr GetSnapshot(const std::string& version_id) const {
        std::shared_lock<std::shared_mutex> lock(snapshots_mutex_);
        
        auto it = snapshots_.find(version_id);
        if (it != snapshots_.end()) {
            return it->second;
        }
        
        return nullptr;
    }

    /**
     * @brief 获取当前快照
     * @return ConfigSnapshot::SnapshotPtr 当前快照智能指针
     */
    ConfigSnapshot::SnapshotPtr GetCurrentSnapshot() const {
        std::shared_lock<std::shared_mutex> lock(snapshots_mutex_);
        return current_snapshot_;
    }

    /**
     * @brief 获取当前版本ID
     * @return std::string 当前版本ID
     */
    std::string GetCurrentVersionId() const {
        std::shared_lock<std::shared_mutex> lock(snapshots_mutex_);
        return current_version_id_;
    }

    /**
     * @brief 删除快照
     * @param version_id 版本ID
     * @return bool 是否成功
     */
    bool RemoveSnapshot(const std::string& version_id) {
        std::unique_lock<std::shared_mutex> lock(snapshots_mutex_);
        
        // 不能删除当前版本
        if (version_id == current_version_id_) {
            return false;
        }
        
        return snapshots_.erase(version_id) > 0;
    }

    /**
     * @brief 清理过期快照
     * @param keep_versions 要保留的版本数量
     * @return size_t 清理的快照数量
     */
    size_t CleanupSnapshots(size_t keep_versions = 10) {
        std::unique_lock<std::shared_mutex> lock(snapshots_mutex_);
        
        if (snapshots_.size() <= keep_versions) {
            return 0;
        }
        
        size_t removed_count = 0;
        auto it = snapshots_.begin();
        
        while (it != snapshots_.end() && snapshots_.size() > keep_versions) {
            // 跳过当前版本
            if (it->first == current_version_id_) {
                ++it;
                continue;
            }
            
            it = snapshots_.erase(it);
            ++removed_count;
        }
        
        return removed_count;
    }

    /**
     * @brief 获取所有版本ID
     * @return std::vector<std::string> 版本ID列表
     */
    std::vector<std::string> GetAllVersionIds() const {
        std::shared_lock<std::shared_mutex> lock(snapshots_mutex_);
        
        std::vector<std::string> version_ids;
        version_ids.reserve(snapshots_.size());
        
        for (const auto& [version_id, snapshot] : snapshots_) {
            version_ids.push_back(version_id);
        }
        
        return version_ids;
    }

    /**
     * @brief 获取快照数量
     * @return size_t 快照数量
     */
    size_t GetSnapshotCount() const {
        std::shared_lock<std::shared_mutex> lock(snapshots_mutex_);
        return snapshots_.size();
    }

    /**
     * @brief 回滚到指定版本
     * @param version_id 目标版本ID
     * @return bool 是否成功
     */
    bool RollbackToVersion(const std::string& version_id) {
        std::unique_lock<std::shared_mutex> lock(snapshots_mutex_);
        
        auto it = snapshots_.find(version_id);
        if (it == snapshots_.end()) {
            return false;
        }
        
        // 更新当前快照和版本ID
        current_snapshot_ = it->second;
        current_version_id_ = version_id;
        
        // 添加到历史记录
        version_history_.push_back(version_id);
        
        return true;
    }

    /**
     * @brief 获取版本历史
     * @return std::vector<std::string> 版本历史列表
     */
    std::vector<std::string> GetVersionHistory() const {
        std::shared_lock<std::shared_mutex> lock(snapshots_mutex_);
        return version_history_;
    }
};

}  // namespace sqlcc

#endif  // SQLCC_CONFIG_SNAPSHOT_H_