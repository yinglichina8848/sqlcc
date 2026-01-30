#include "config_manager.h"
#include <algorithm>
#include <cctype>
#include <mutex>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <iostream>
#include <filesystem>

namespace sqlcc {

// ConfigManager 单例实例
std::unique_ptr<ConfigManager> ConfigManager::instance_;
std::once_flag ConfigManager::init_flag_;

// ConfigManager构造函数实现
ConfigManager::ConfigManager() : operation_timeout_ms_(kDefaultOperationTimeoutMs) {
}

// 设置配置值
bool ConfigManager::SetValue(const std::string& key, const ConfigValue& value) {
  std::lock_guard<std::mutex> lock(config_mutex_);
  config_map_[key] = value;
  return true;
}

// 检查键是否存在
bool ConfigManager::HasKey(const std::string& key) const {
  std::lock_guard<std::mutex> lock(config_mutex_);
  return config_map_.find(key) != config_map_.end();
}

// 获取字符串值
std::string ConfigManager::GetString(const std::string& key, const std::string& default_value) const {
  std::lock_guard<std::mutex> lock(config_mutex_);
  auto it = config_map_.find(key);
  if (it != config_map_.end()) {
    try {
      if (std::holds_alternative<std::string>(it->second)) {
        return std::get<std::string>(it->second);
      } else {
        // 如果不是字符串类型，转换为字符串
        return std::visit([](auto&& arg) -> std::string {
          using T = std::decay_t<decltype(arg)>;
          if constexpr (std::is_same_v<T, std::string>) {
            return arg;
          } else if constexpr (std::is_same_v<T, int>) {
            return std::to_string(arg);
          } else if constexpr (std::is_same_v<T, double>) {
            return std::to_string(arg);
          } else if constexpr (std::is_same_v<T, bool>) {
            return arg ? "true" : "false";
          }
          return "";
        }, it->second);
      }
    } catch (const std::exception&) {
      // 转换失败，返回默认值
    }
  }
  return default_value;
}

// 获取整数值
int ConfigManager::GetInt(const std::string& key, int default_value) const {
  std::lock_guard<std::mutex> lock(config_mutex_);
  auto it = config_map_.find(key);
  if (it != config_map_.end()) {
    try {
      if (std::holds_alternative<int>(it->second)) {
        return std::get<int>(it->second);
      } else if (std::holds_alternative<std::string>(it->second)) {
        return std::stoi(std::get<std::string>(it->second));
      } else if (std::holds_alternative<double>(it->second)) {
        return static_cast<int>(std::get<double>(it->second));
      } else if (std::holds_alternative<bool>(it->second)) {
        return std::get<bool>(it->second) ? 1 : 0;
      }
    } catch (const std::exception&) {
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
        std::string str_val = std::get<std::string>(it->second);
        // 支持 "true", "1", "yes", "on" 等表示true的字符串
        return str_val == "true" || str_val == "1" || str_val == "yes" || str_val == "on";
      } else if (std::holds_alternative<int>(it->second)) {
        return std::get<int>(it->second) != 0;
      } else if (std::holds_alternative<double>(it->second)) {
        return std::get<double>(it->second) != 0.0;
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

// 从文件加载配置
// 修改：不再预先获取锁，让 ParseConfigFile 自己管理锁
bool ConfigManager::LoadConfig(const std::string& config_file_path, const std::string& env) {
  // 清除现有配置（不获取锁，ParseConfigFile 会负责加锁）
  config_map_.clear();
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

// 重新加载配置文件（完整替换）
bool ConfigManager::ReloadConfig(const std::string& new_config_path) {
  config_map_.clear(); // 清除现有配置
  config_file_path_ = new_config_path;
  return ParseConfigFile(new_config_path);
}

// 加载默认配置
// 内部加载默认配置，不获取锁
void ConfigManager::LoadDefaultConfigInternal(std::unordered_map<std::string, ConfigValue>& temp_config) {
  temp_config["buffer_pool.read_lock_timeout_ms"] = 2000;
  temp_config["buffer_pool.write_lock_timeout_ms"] = 5000;
  temp_config["buffer_pool.default_lock_timeout_ms"] = 3000;
  operation_timeout_ms_ = kDefaultOperationTimeoutMs;
}

void ConfigManager::LoadDefaultConfig() {
  // 设置默认配置值
  std::lock_guard<std::mutex> lock(config_mutex_);
  config_map_["buffer_pool.read_lock_timeout_ms"] = 2000;
  config_map_["buffer_pool.write_lock_timeout_ms"] = 5000;
  config_map_["buffer_pool.default_lock_timeout_ms"] = 3000;
  operation_timeout_ms_ = kDefaultOperationTimeoutMs;
}

// 解析配置文件（内部方法，不获取锁）
bool ConfigManager::ParseConfigFileInternal(const std::string& file_path,
                                            std::unordered_map<std::string, ConfigValue>& result_config) {
  std::ifstream file(file_path);
  if (!file.is_open()) {
    std::unordered_map<std::string, ConfigValue> temp_config;
    LoadDefaultConfigInternal(temp_config);
    result_config = std::move(temp_config);
    return true;
  }
  
  std::string line;
  std::string current_section;
  int line_count = 0;
  
  std::unordered_map<std::string, ConfigValue> temp_config;
  
  while (std::getline(file, line)) {
    line_count++;
    
    if (line_count > 10000) {
      break;
    }
    
    // 跳过空行和注释行
    if (line.empty() || line[0] == '#' || line[0] == ';') {
      continue;
    }
    
    // 处理节标题 [section]
    if (line.length() >= 3 && line[0] == '[' && line[line.length() - 1] == ']') {
      current_section = line.substr(1, line.length() - 2);
      continue;
    }
    
    // 解析配置行到临时映射
    if (!ParseConfigLineInternal(line, current_section, temp_config)) {
      continue;
    }
  }
  
  file.close();
  result_config = std::move(temp_config);
  return true;
}

// 解析配置文件（公开方法，获取锁）
bool ConfigManager::ParseConfigFile(const std::string& file_path) {
  std::unordered_map<std::string, ConfigValue> new_config;
  if (!ParseConfigFileInternal(file_path, new_config)) {
    return false;
  }
  
  std::lock_guard<std::mutex> lock(config_mutex_);
  for (const auto& [key, value] : new_config) {
    config_map_[key] = value;
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
  if (line.length() >= 3 && line[0] == '[' && line[line.length() - 1] == ']') {
    current_section = line.substr(1, line.length() - 2);
    return true;
  }

  // 处理键值对 key=value
  size_t equal_pos = line.find('=');
  if (equal_pos != std::string::npos && equal_pos > 0) {
    std::string key = line.substr(0, equal_pos);
    std::string value = line.substr(equal_pos + 1);
    
    // 去除首尾空格
    size_t first_non_space = key.find_first_not_of(" \t");
    if (first_non_space == std::string::npos) return true;
    key = key.substr(first_non_space);
    
    size_t last_non_space = key.find_last_not_of(" \t");
    if (last_non_space != std::string::npos && last_non_space < key.length() - 1) {
      key = key.substr(0, last_non_space + 1);
    }
    
    first_non_space = value.find_first_not_of(" \t");
    if (first_non_space != std::string::npos) {
      value = value.substr(first_non_space);
      last_non_space = value.find_last_not_of(" \t");
      if (last_non_space != std::string::npos && last_non_space < value.length() - 1) {
        value = value.substr(0, last_non_space + 1);
      }
    }
    
    // 如果有节，则加上节前缀
    std::string full_key = current_section.empty() ? key : current_section + "." + key;
    
    // 创建临时配置映射来处理配置行
    std::unordered_map<std::string, ConfigValue> temp_config;
    
    // 尝试解析值类型
    try {
      // 尝试解析为布尔值（首先检查，避免将true/false误认为数字）
      std::string lower_value = value;
      std::transform(lower_value.begin(), lower_value.end(), lower_value.begin(), ::tolower);
      if (lower_value == "true" || lower_value == "false" || 
          lower_value == "yes" || lower_value == "no" ||
          lower_value == "on" || lower_value == "off" ||
          lower_value == "1" || lower_value == "0") {
        bool bool_val = (lower_value == "true" || lower_value == "yes" || 
                        lower_value == "on" || lower_value == "1");
        temp_config[full_key] = bool_val;
      }
      // 尝试解析为整数
      else if (value.find('.') == std::string::npos) {
        size_t end_pos;
        int int_val = std::stoi(value, &end_pos);
        if (end_pos == value.length()) {
          temp_config[full_key] = int_val;
        } else {
          temp_config[full_key] = value;
        }
      }
      // 尝试解析为浮点数
      else if (value.find('.') != std::string::npos) {
        size_t end_pos;
        double double_val = std::stod(value, &end_pos);
        if (end_pos == value.length()) {
          temp_config[full_key] = double_val;
        } else {
          temp_config[full_key] = value;
        }
      }
      // 默认为字符串
      else {
        temp_config[full_key] = value;
      }
    } catch (const std::exception& e) {
      temp_config[full_key] = value;
    }
    
    // 在持有锁的情况下一次性应用配置
    std::lock_guard<std::mutex> lock(config_mutex_);
    for (const auto& [key, val] : temp_config) {
      config_map_[key] = val;
    }
    
    return true;
  }
  
  return true;
}

// 内部解析方法，不会尝试获取锁
bool ConfigManager::ParseConfigLineInternal(const std::string& line, std::string& current_section, 
                                           std::unordered_map<std::string, ConfigValue>& temp_config) const {
  // 跳过空行和注释行
  if (line.empty() || line[0] == '#' || line[0] == ';') {
    return true;
  }

  // 处理节标题 [section]
  if (line.length() >= 3 && line[0] == '[' && line[line.length() - 1] == ']') {
    current_section = line.substr(1, line.length() - 2);
    return true;
  }

  // 处理键值对 key=value
  size_t equal_pos = line.find('=');
  if (equal_pos != std::string::npos && equal_pos > 0) {
    std::string key = line.substr(0, equal_pos);
    std::string value = line.substr(equal_pos + 1);
    
    // 去除首尾空格
    size_t first_non_space = key.find_first_not_of(" \t");
    if (first_non_space == std::string::npos) return true;
    key = key.substr(first_non_space);
    
    size_t last_non_space = key.find_last_not_of(" \t");
    if (last_non_space != std::string::npos && last_non_space < key.length() - 1) {
      key = key.substr(0, last_non_space + 1);
    }
    
    first_non_space = value.find_first_not_of(" \t");
    if (first_non_space != std::string::npos) {
      value = value.substr(first_non_space);
      last_non_space = value.find_last_not_of(" \t");
      if (last_non_space != std::string::npos && last_non_space < value.length() - 1) {
        value = value.substr(0, last_non_space + 1);
      }
    }
    
    // 如果有节，则加上节前缀
    std::string full_key = current_section.empty() ? key : current_section + "." + key;
    
    // 尝试解析值类型
    try {
      // 尝试解析为布尔值（首先检查，避免将true/false误认为数字）
      std::string lower_value = value;
      std::transform(lower_value.begin(), lower_value.end(), lower_value.begin(), ::tolower);
      if (lower_value == "true" || lower_value == "false" || 
          lower_value == "yes" || lower_value == "no" ||
          lower_value == "on" || lower_value == "off" ||
          lower_value == "1" || lower_value == "0") {
        bool bool_val = (lower_value == "true" || lower_value == "yes" || 
                        lower_value == "on" || lower_value == "1");
        temp_config[full_key] = bool_val;
        return true;
      }
      
      // 尝试解析为整数
      if (value.find('.') == std::string::npos) {
        size_t end_pos;
        int int_val = std::stoi(value, &end_pos);
        if (end_pos == value.length()) {
          temp_config[full_key] = int_val;
          return true;
        }
      }
      
      // 尝试解析为浮点数
      if (value.find('.') != std::string::npos) {
        size_t end_pos;
        double double_val = std::stod(value, &end_pos);
        if (end_pos == value.length()) {
          temp_config[full_key] = double_val;
          return true;
        }
      }
      
      // 默认为字符串
      temp_config[full_key] = value;
      return true;
      
    } catch (const std::exception& e) {
      temp_config[full_key] = value;
      return true;
    }
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

// 测试辅助方法 - 重置单例状态
void ConfigManager::ResetForTest() {
  std::lock_guard<std::mutex> lock(config_mutex_);
  config_map_.clear();
  config_file_path_.clear();
  env_.clear();
  operation_timeout_ms_ = kDefaultOperationTimeoutMs;
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