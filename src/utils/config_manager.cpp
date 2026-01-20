#include "config_manager.h"
#include <algorithm>
#include <cctype>
#include <mutex>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <filesystem>

namespace sqlcc {

// ConfigManager 单例实例
std::unique_ptr<ConfigManager> ConfigManager::instance_;
std::once_flag ConfigManager::init_flag_;

// ConfigManager构造函数实现
ConfigManager::ConfigManager() : operation_timeout_ms_(kDefaultOperationTimeoutMs) {
}

bool ConfigManager::SetValue(const std::string &key, const ConfigValue &value) {
  std::lock_guard<std::mutex> lock(config_mutex_);
  config_map_[key] = value;
  return true;
}

bool ConfigManager::HasKey(const std::string &key) const {
  std::lock_guard<std::mutex> lock(config_mutex_);
  return config_map_.find(key) != config_map_.end();
}

std::string ConfigManager::GetString(const std::string &key,
                                     const std::string &default_value) const {
  std::lock_guard<std::mutex> lock(config_mutex_);
  auto it = config_map_.find(key);
  if (it != config_map_.end()) {
    try {
      return std::get<std::string>(it->second);
    } catch (const std::bad_variant_access &) {
      // 如果类型不匹配，尝试转换为字符串
      if (std::holds_alternative<int>(it->second)) {
        return std::to_string(std::get<int>(it->second));
      } else if (std::holds_alternative<double>(it->second)) {
        return std::to_string(std::get<double>(it->second));
      } else if (std::holds_alternative<bool>(it->second)) {
        return std::get<bool>(it->second) ? "true" : "false";
      }
    }
  }
  return default_value;
}

int ConfigManager::GetInt(const std::string &key, int default_value) const {
  std::lock_guard<std::mutex> lock(config_mutex_);
  auto it = config_map_.find(key);
  if (it != config_map_.end()) {
    try {
      if (std::holds_alternative<int>(it->second)) {
        return std::get<int>(it->second);
      } else if (std::holds_alternative<std::string>(it->second)) {
        return std::stoi(std::get<std::string>(it->second));
      }
    } catch (const std::exception &) {
      // 转换失败，返回默认值
    }
  }
  return default_value;
}

bool ConfigManager::GetBool(const std::string &key, bool default_value) const {
  std::lock_guard<std::mutex> lock(config_mutex_);
  auto it = config_map_.find(key);
  if (it != config_map_.end()) {
    try {
      if (std::holds_alternative<bool>(it->second)) {
        return std::get<bool>(it->second);
      } else if (std::holds_alternative<std::string>(it->second)) {
        std::string value = std::get<std::string>(it->second);
        std::transform(value.begin(), value.end(), value.begin(), ::tolower);
        if (value == "true" || value == "1" || value == "yes" ||
            value == "on") {
          return true;
        } else if (value == "false" || value == "0" || value == "no" ||
                   value == "off") {
          return false;
        }
      } else if (std::holds_alternative<int>(it->second)) {
        return std::get<int>(it->second) != 0;
      }
    } catch (const std::exception &) {
      // 转换失败，返回默认值
    }
  }
  return default_value;
}

double ConfigManager::GetDouble(const std::string &key,
                                double default_value) const {
  std::lock_guard<std::mutex> lock(config_mutex_);
  auto it = config_map_.find(key);
  if (it != config_map_.end()) {
    try {
      if (std::holds_alternative<double>(it->second)) {
        return std::get<double>(it->second);
      } else if (std::holds_alternative<int>(it->second)) {
        return static_cast<double>(std::get<int>(it->second));
      } else if (std::holds_alternative<std::string>(it->second)) {
        return std::stod(std::get<std::string>(it->second));
      }
    } catch (const std::exception &) {
      // 转换失败，返回默认值
    }
  }
  return default_value;
}

// GetInstance 单例模式实现
ConfigManager& ConfigManager::GetInstance() {
  std::call_once(init_flag_, []() {
    instance_ = std::unique_ptr<ConfigManager>(new ConfigManager());
  });
  return *instance_;
}

// 加载配置文件
bool ConfigManager::LoadConfig(const std::string& config_file_path, const std::string& env) {
  std::lock_guard<std::mutex> lock(config_mutex_);
  config_file_path_ = config_file_path;
  env_ = env;
  return ParseConfigFile(config_file_path);
}

// 重新加载配置文件
bool ConfigManager::ReloadConfig() {
  std::lock_guard<std::mutex> lock(config_mutex_);
  if (config_file_path_.empty()) {
    return false;
  }
  return ParseConfigFile(config_file_path_);
}

// 加载默认配置
void ConfigManager::LoadDefaultConfig() {
  std::lock_guard<std::mutex> lock(config_mutex_);
  // 设置默认配置值
  config_map_["buffer_pool.read_lock_timeout_ms"] = 2000;
  config_map_["buffer_pool.write_lock_timeout_ms"] = 5000;
  config_map_["buffer_pool.default_lock_timeout_ms"] = 3000;
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
bool ConfigManager::ParseConfigLine(const std::string& line, std::string& current_section) {
  // 跳过空行和注释行
  if (line.empty() || line[0] == '#' || line[0] == ';') {
    return true;
  }

  // 处理节标题 [section]
  if (line[0] == '[' && line.back() == ']') {
    current_section = line.substr(1, line.length() - 2);
    return true;
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
    return SetValue(full_key, value);
  }
  return true;
}

// 保存配置到文件
bool ConfigManager::SaveToFile(const std::string& file_path) const {
  std::lock_guard<std::mutex> lock(config_mutex_);
  std::ofstream file(file_path);
  if (!file.is_open()) {
    return false;
  }

  std::string current_section;
  for (const auto& [key, value] : config_map_) {
    size_t dot_pos = key.find('.');
    std::string section = (dot_pos != std::string::npos) ? key.substr(0, dot_pos) : "";
    std::string item_key = (dot_pos != std::string::npos) ? key.substr(dot_pos + 1) : key;
    
    if (section != current_section) {
      if (!current_section.empty()) {
        file << std::endl;
      }
      file << "[" << section << "]" << std::endl;
      current_section = section;
    }
    
    if (std::holds_alternative<std::string>(value)) {
      file << item_key << "=" << std::get<std::string>(value) << std::endl;
    } else if (std::holds_alternative<int>(value)) {
      file << item_key << "=" << std::get<int>(value) << std::endl;
    } else if (std::holds_alternative<double>(value)) {
      file << item_key << "=" << std::get<double>(value) << std::endl;
    } else if (std::holds_alternative<bool>(value)) {
      file << item_key << "=" << (std::get<bool>(value) ? "true" : "false") << std::endl;
    }
  }
  
  return true;
}

// 获取所有配置键
std::vector<std::string> ConfigManager::GetAllKeys() const {
  std::lock_guard<std::mutex> lock(config_mutex_);
  std::vector<std::string> keys;
  for (const auto& [key, value] : config_map_) {
    keys.push_back(key);
  }
  return keys;
}

// 获取指定前缀的所有配置键
std::vector<std::string> ConfigManager::GetKeysWithPrefix(const std::string& prefix) const {
  std::lock_guard<std::mutex> lock(config_mutex_);
  std::vector<std::string> keys;
  for (const auto& [key, value] : config_map_) {
    if (key.find(prefix) == 0) {
      keys.push_back(key);
    }
  }
  return keys;
}

// 清除所有配置项（用于测试）
void ConfigManager::ClearAll() {
  std::lock_guard<std::mutex> lock(config_mutex_);
  config_map_.clear();
}

// 设置操作超时时间
void ConfigManager::SetOperationTimeout(int timeout_ms) {
  std::lock_guard<std::mutex> lock(config_mutex_);
  operation_timeout_ms_ = timeout_ms;
}

// 获取操作超时时间
int ConfigManager::GetOperationTimeout() const {
  std::lock_guard<std::mutex> lock(config_mutex_);
  return operation_timeout_ms_;
}

} // namespace sqlcc
