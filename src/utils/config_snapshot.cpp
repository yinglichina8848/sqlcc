/**
 * @file config_snapshot.cpp
 * @brief 智能配置快照管理器实现文件
 * 
 * Why: 实现线程安全的配置快照机制，支持无锁配置读取和版本管理
 * What: 提供ConfigSnapshot类和相关的智能指针管理功能的实现
 * How: 使用std::shared_ptr实现配置快照的共享和自动生命周期管理
 */

#include "utils/config_snapshot.h"
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace sqlcc {

// ConfigSnapshotFactory 实现

ConfigSnapshot::SnapshotPtr ConfigSnapshotFactory::CreateSnapshot(
    const std::unordered_map<std::string, ConfigValue>& config_data,
    const std::string& version_id,
    const std::string& description) {
    
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

ConfigSnapshot::SnapshotPtr ConfigSnapshotFactory::CreateEmptySnapshot(
    const std::string& version_id,
    const std::string& description) {
    
    std::unordered_map<std::string, ConfigValue> empty_data;
    return CreateSnapshot(empty_data, version_id, description);
}

ConfigSnapshot::SnapshotPtr ConfigSnapshotFactory::MergeSnapshots(
    const ConfigSnapshot::SnapshotPtr& base_snapshot,
    const ConfigSnapshot::SnapshotPtr& override_snapshot,
    const std::string& new_version_id,
    const std::string& description) {
    
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

// ConfigSnapshotManager 实现

bool ConfigSnapshotManager::AddSnapshot(const ConfigSnapshot::SnapshotPtr& snapshot) {
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

ConfigSnapshot::SnapshotPtr ConfigSnapshotManager::GetSnapshot(const std::string& version_id) const {
    std::shared_lock<std::shared_mutex> lock(snapshots_mutex_);
    
    auto it = snapshots_.find(version_id);
    if (it != snapshots_.end()) {
        return it->second;
    }
    
    return nullptr;
}

ConfigSnapshot::SnapshotPtr ConfigSnapshotManager::GetCurrentSnapshot() const {
    std::shared_lock<std::shared_mutex> lock(snapshots_mutex_);
    return current_snapshot_;
}

std::string ConfigSnapshotManager::GetCurrentVersionId() const {
    std::shared_lock<std::shared_mutex> lock(snapshots_mutex_);
    return current_version_id_;
}

bool ConfigSnapshotManager::RemoveSnapshot(const std::string& version_id) {
    std::unique_lock<std::shared_mutex> lock(snapshots_mutex_);
    
    // 不能删除当前版本
    if (version_id == current_version_id_) {
        return false;
    }
    
    return snapshots_.erase(version_id) > 0;
}

size_t ConfigSnapshotManager::CleanupSnapshots(size_t keep_versions) {
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

std::vector<std::string> ConfigSnapshotManager::GetAllVersionIds() const {
    std::shared_lock<std::shared_mutex> lock(snapshots_mutex_);
    
    std::vector<std::string> version_ids;
    version_ids.reserve(snapshots_.size());
    
    for (const auto& [version_id, snapshot] : snapshots_) {
        version_ids.push_back(version_id);
    }
    
    return version_ids;
}

size_t ConfigSnapshotManager::GetSnapshotCount() const {
    std::shared_lock<std::shared_mutex> lock(snapshots_mutex_);
    return snapshots_.size();
}

bool ConfigSnapshotManager::RollbackToVersion(const std::string& version_id) {
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

std::vector<std::string> ConfigSnapshotManager::GetVersionHistory() const {
    std::shared_lock<std::shared_mutex> lock(snapshots_mutex_);
    return version_history_;
}

// 工具函数实现

/**
 * @brief 生成版本ID
 * @param prefix 前缀
 * @return std::string 版本ID
 */
std::string GenerateVersionId(const std::string& prefix = "v") {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << prefix << std::put_time(std::localtime(&time_t), "%Y%m%d_%H%M%S") 
       << "_" << std::setfill('0') << std::setw(3) << ms.count();
    
    return ss.str();
}

/**
 * @brief 格式化配置值
 * @param value 配置值
 * @return std::string 格式化后的字符串
 */
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

/**
 * @brief 解析配置值
 * @param str 字符串
 * @param value 输出参数，解析后的配置值
 * @return bool 是否成功
 */
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