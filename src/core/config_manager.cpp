#include "config_manager.h"
#include <algorithm>
#include <cctype>

namespace sqlcc {

ConfigManager::ConfigManager() {
  // 设置一些默认配置
  config_map_["index.page_size"] = "4096";
  config_map_["index.fanout"] = "50";
  config_map_["index.max_entries"] = "10000";
  config_map_["buffer_pool.size"] = "1000";
  config_map_["buffer_pool.shard_count"] = "4";
  config_map_["buffer_pool.stripe_count"] = "16";
}

void ConfigManager::Set(const std::string &key, const std::string &value) {
  config_map_[key] = value;
}

std::string ConfigManager::Get(const std::string &key,
                               const std::string &default_value) const {
  auto it = config_map_.find(key);
  if (it != config_map_.end()) {
    return it->second;
  }
  return default_value;
}

bool ConfigManager::Has(const std::string &key) const {
  return config_map_.find(key) != config_map_.end();
}

std::string ConfigManager::GetString(const std::string &key,
                                     const std::string &default_value) const {
  return Get(key, default_value);
}

int ConfigManager::GetInt(const std::string &key, int default_value) const {
  auto value = Get(key, "");
  if (value.empty()) {
    return default_value;
  }
  try {
    return std::stoi(value);
  } catch (const std::exception &) {
    return default_value;
  }
}

bool ConfigManager::GetBool(const std::string &key, bool default_value) const {
  auto value = Get(key, "");
  if (value.empty()) {
    return default_value;
  }

  // 转换为小写进行比较
  std::string lower_value = value;
  std::transform(lower_value.begin(), lower_value.end(), lower_value.begin(),
                 ::tolower);

  if (lower_value == "true" || lower_value == "1" || lower_value == "yes" ||
      lower_value == "on") {
    return true;
  } else if (lower_value == "false" || lower_value == "0" ||
             lower_value == "no" || lower_value == "off") {
    return false;
  }

  return default_value;
}

} // namespace sqlcc