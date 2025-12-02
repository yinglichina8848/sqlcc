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
    
    return true;
}

// 解析配置行
void ConfigManager::ParseConfigLine(const std::string& line, std::string& current_section) {
    // 跳过空行和注释行
    if (line.empty() || line[0] == '#' || line[0] == ';') {
        return;
    }

    // 处理节标题 [section]
    if (line[0] == '[' && line.back() == ']') {
        current_section = line.substr(1, line.length() - 2);
        return;
    }

    // 处理键值对 key=value
    size_t equal_pos = line.find('=');
    if (equal_pos != std::string::npos) {
        std::string key = line.substr(0, equal_pos);
        std::string value = line.substr(equal_pos + 1);
        
        // 去除首尾空格
        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t") + 1);
        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t") + 1);
        
        // 如果有节，则加上节前缀
        std::string full_key = current_section.empty() ? key : current_section + "." + key;
        SetValue(full_key, value);
    }
}

// 设置配置值
void ConfigManager::SetValue(const std::string& key, const std::variant<bool, int, double, std::string>& value) {
    config_values_[key] = value;
}

// 获取布尔值配置
bool ConfigManager::GetBool(const std::string& key, bool default_value) const {
    auto it = config_values_.find(key);
    if (it != config_values_.end()) {
        if (std::holds_alternative<bool>(it->second)) {
            return std::get<bool>(it->second);
        } else if (std::holds_alternative<std::string>(it->second)) {
            std::string str_value = std::get<std::string>(it->second);
            std::transform(str_value.begin(), str_value.end(), str_value.begin(), ::tolower);
            return (str_value == "true" || str_value == "1" || str_value == "yes" || str_value == "on");
        }
    }
    return default_value;
}

// 获取整数值配置
int ConfigManager::GetInt(const std::string& key, int default_value) const {
    auto it = config_values_.find(key);
    if (it != config_values_.end()) {
        if (std::holds_alternative<int>(it->second)) {
            return std::get<int>(it->second);
        } else if (std::holds_alternative<std::string>(it->second)) {
            try {
                return std::stoi(std::get<std::string>(it->second));
            } catch (const std::exception&) {
                // 转换失败，返回默认值
            }
        }
    }
    return default_value;
}

// 获取双精度浮点数配置
double ConfigManager::GetDouble(const std::string& key, double default_value) const {
    auto it = config_values_.find(key);
    if (it != config_values_.end()) {
        if (std::holds_alternative<double>(it->second)) {
            return std::get<double>(it->second);
        } else if (std::holds_alternative<int>(it->second)) {
            return static_cast<double>(std::get<int>(it->second));
        } else if (std::holds_alternative<std::string>(it->second)) {
            try {
                return std::stod(std::get<std::string>(it->second));
            } catch (const std::exception&) {
                // 转换失败，返回默认值
            }
        }
    }
    return default_value;
}

// 获取字符串配置
std::string ConfigManager::GetString(const std::string& key, const std::string& default_value) const {
    auto it = config_values_.find(key);
    if (it != config_values_.end()) {
        if (std::holds_alternative<std::string>(it->second)) {
            return std::get<std::string>(it->second);
        } else if (std::holds_alternative<bool>(it->second)) {
            return std::get<bool>(it->second) ? "true" : "false";
        } else if (std::holds_alternative<int>(it->second)) {
            return std::to_string(std::get<int>(it->second));
        } else if (std::holds_alternative<double>(it->second)) {
            return std::to_string(std::get<double>(it->second));
        }
    }
    return default_value;
}

// 检查键是否存在
bool ConfigManager::HasKey(const std::string& key) const {
    return config_values_.find(key) != config_values_.end();
}

// 设置操作超时时间
void ConfigManager::SetOperationTimeout(int timeout_ms) {
    operation_timeout_ms_ = timeout_ms;
}

// 获取操作超时时间
int ConfigManager::GetOperationTimeout() const {
    return operation_timeout_ms_;
}

// 保存配置到文件
bool ConfigManager::SaveToFile(const std::string& file_path) const {
    std::ofstream file(file_path);
    if (!file.is_open()) {
        return false;
    }

    for (const auto& pair : config_values_) {
        const std::string& key = pair.first;
        const auto& value = pair.second;

        if (std::holds_alternative<bool>(value)) {
            file << key << "=" << (std::get<bool>(value) ? "true" : "false") << "\n";
        } else if (std::holds_alternative<int>(value)) {
            file << key << "=" << std::get<int>(value) << "\n";
        } else if (std::holds_alternative<double>(value)) {
            file << key << "=" << std::get<double>(value) << "\n";
        } else if (std::holds_alternative<std::string>(value)) {
            file << key << "=" << std::get<std::string>(value) << "\n";
        }
    }

    return true;
}

// 获取所有键
std::vector<std::string> ConfigManager::GetAllKeys() const {
    std::vector<std::string> keys;
    for (const auto& pair : config_values_) {
        keys.push_back(pair.first);
    }
    return keys;
}

// 根据前缀获取键
std::vector<std::string> ConfigManager::GetKeysWithPrefix(const std::string& prefix) const {
    std::vector<std::string> keys;
    for (const auto& pair : config_values_) {
        if (pair.first.substr(0, prefix.length()) == prefix) {
            keys.push_back(pair.first);
        }
    }
    return keys;
}

}  // namespace sqlcc