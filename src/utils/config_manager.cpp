#include "../../include/utils/config_manager.h"
#include <algorithm>
#include <cctype>
#include <mutex>

namespace sqlcc {

// ConfigManager 单例实例
std::unique_ptr<ConfigManager> ConfigManager::instance_;
std::once_flag ConfigManager::init_flag_;

// ConfigManager构造函数使用默认实现，已在头文件中声明为 = default

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

} // namespace sqlcc
