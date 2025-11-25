#include "config_manager.h"
#include <iostream>
#include <fstream>
#include <sstream>

namespace sqlcc {

// 获取单例实例
ConfigManager& ConfigManager::GetInstance() {
    static ConfigManager instance;
    return instance;
}

// 加载配置文件
bool ConfigManager::LoadConfig(const std::string& config_file_path, const std::string& env) {
    config_file_path_ = config_file_path;
    env_ = env;
    return ParseConfigFile(config_file_path);
}

// 重新加载配置文件
bool ConfigManager::ReloadConfig() {
    if (config_file_path_.empty()) {
        return false;
    }
    return ParseConfigFile(config_file_path_);
}

// 加载默认配置
void ConfigManager::LoadDefaultConfig() {
    // 设置默认配置值
    SetValue("buffer_pool.read_lock_timeout_ms", 2000);
    SetValue("buffer_pool.write_lock_timeout_ms", 5000);
    SetValue("buffer_pool.default_lock_timeout_ms", 3000);
    operation_timeout_ms_ = kDefaultOperationTimeoutMs;
}

// 解析配置文件
bool ConfigManager::ParseConfigFile(const std::string& file_path) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        // 文件不存在时加载默认配置
        LoadDefaultConfig();
        return true;
    }

    std::string line;
    std::string current_section;
    
    while (std::getline(file, line)) {
        ParseConfigLine(line, current_section);
    }
    
    file.close();
    return true;
}

// 解析配置行
bool ConfigManager::ParseConfigLine(const std::string& line, std::string& current_section) {
    // 简化的配置行解析
    std::string trimmed = line;
    // 移除首尾空格
    trimmed.erase(0, trimmed.find_first_not_of(" \t"));
    trimmed.erase(trimmed.find_last_not_of(" \t\r\n") + 1);
    
    // 跳过空行和注释
    if (trimmed.empty() || trimmed[0] == '#') {
        return true;
    }
    
    // 处理section行 [section]
    if (trimmed[0] == '[' && trimmed.back() == ']') {
        current_section = trimmed.substr(1, trimmed.size() - 2);
        return true;
    }
    
    // 处理key=value行
    size_t eq_pos = trimmed.find('=');
    if (eq_pos != std::string::npos) {
        std::string key = trimmed.substr(0, eq_pos);
        std::string value = trimmed.substr(eq_pos + 1);
        
        // 移除键和值的空格
        key.erase(key.find_last_not_of(" \t") + 1);
        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t\r\n") + 1);
        
        // 如果有section，添加section前缀
        if (!current_section.empty()) {
            key = current_section + "." + key;
        }
        
        // 尝试解析不同类型的值
        if (value == "true" || value == "false") {
            SetValue(key, value == "true");
        } else if (value.find_first_not_of("-0123456789") == std::string::npos) {
            SetValue(key, std::stoi(value));
        } else if (value.find_first_not_of("-0123456789.") == std::string::npos && 
                  value.find('.') != std::string::npos) {
            SetValue(key, std::stod(value));
        } else {
            // 去掉引号（如果有）
            if (value.size() >= 2 && ((value[0] == '"' && value.back() == '"') || 
                                     (value[0] == '\'' && value.back() == '\''))) {
                value = value.substr(1, value.size() - 2);
            }
            SetValue(key, value);
        }
        
        return true;
    }
    
    return false;
}

// 获取布尔类型配置值
bool ConfigManager::GetBool(const std::string& key, bool default_value) const {
    std::lock_guard<std::mutex> lock(config_mutex_);
    auto it = config_map_.find(key);
    if (it != config_map_.end() && std::holds_alternative<bool>(it->second)) {
        return std::get<bool>(it->second);
    }
    return default_value;
}

// 获取整数类型配置值
int ConfigManager::GetInt(const std::string& key, int default_value) const {
    std::lock_guard<std::mutex> lock(config_mutex_);
    auto it = config_map_.find(key);
    if (it != config_map_.end()) {
        if (std::holds_alternative<int>(it->second)) {
            return std::get<int>(it->second);
        }
        // 尝试从其他类型转换
        if (std::holds_alternative<bool>(it->second)) {
            return std::get<bool>(it->second) ? 1 : 0;
        }
    }
    return default_value;
}

// 获取双精度浮点类型配置值
double ConfigManager::GetDouble(const std::string& key, double default_value) const {
    std::lock_guard<std::mutex> lock(config_mutex_);
    auto it = config_map_.find(key);
    if (it != config_map_.end()) {
        if (std::holds_alternative<double>(it->second)) {
            return std::get<double>(it->second);
        }
        if (std::holds_alternative<int>(it->second)) {
            return static_cast<double>(std::get<int>(it->second));
        }
    }
    return default_value;
}

// 获取字符串类型配置值
std::string ConfigManager::GetString(const std::string& key, const std::string& default_value) const {
    std::lock_guard<std::mutex> lock(config_mutex_);
    auto it = config_map_.find(key);
    if (it != config_map_.end() && std::holds_alternative<std::string>(it->second)) {
        return std::get<std::string>(it->second);
    }
    return default_value;
}

// 设置配置值
bool ConfigManager::SetValue(const std::string& key, const ConfigValue& value) {
    std::lock_guard<std::mutex> lock(config_mutex_);
    config_map_[key] = value;
    return true;
}

// 检查配置键是否存在
bool ConfigManager::HasKey(const std::string& key) const {
    std::lock_guard<std::mutex> lock(config_mutex_);
    return config_map_.find(key) != config_map_.end();
}

// 保存当前配置到文件
bool ConfigManager::SaveToFile(const std::string& file_path) const {
    std::ofstream file(file_path);
    if (!file.is_open()) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(config_mutex_);
    for (const auto& [key, value] : config_map_) {
        file << key << " = ";
        if (std::holds_alternative<bool>(value)) {
            file << (std::get<bool>(value) ? "true" : "false");
        } else if (std::holds_alternative<int>(value)) {
            file << std::get<int>(value);
        } else if (std::holds_alternative<double>(value)) {
            file << std::get<double>(value);
        } else if (std::holds_alternative<std::string>(value)) {
            file << '"' << std::get<std::string>(value) << '"';
        }
        file << std::endl;
    }
    
    file.close();
    return true;
}

// 获取所有配置键
std::vector<std::string> ConfigManager::GetAllKeys() const {
    std::lock_guard<std::mutex> lock(config_mutex_);
    std::vector<std::string> keys;
    keys.reserve(config_map_.size());
    for (const auto& [key, _] : config_map_) {
        keys.push_back(key);
    }
    return keys;
}

// 获取指定前缀的所有配置键
std::vector<std::string> ConfigManager::GetKeysWithPrefix(const std::string& prefix) const {
    std::lock_guard<std::mutex> lock(config_mutex_);
    std::vector<std::string> keys;
    for (const auto& [key, _] : config_map_) {
        if (key.find(prefix) == 0) {
            keys.push_back(key);
        }
    }
    return keys;
}

// 设置操作超时时间
void ConfigManager::SetOperationTimeout(int timeout_ms) {
    operation_timeout_ms_ = timeout_ms;
}

// 获取当前操作超时时间
int ConfigManager::GetOperationTimeout() const {
    return operation_timeout_ms_;
}

}  // namespace sqlcc
