#ifndef SQLCC_CONFIG_MANAGER_H
#define SQLCC_CONFIG_MANAGER_H

#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace sqlcc {

// 配置值类型，支持多种数据类型
using ConfigValue = std::variant<int, double, std::string, bool>;

class ConfigManager {
public:
  // 单例模式
  static ConfigManager &GetInstance();

  ConfigManager();
  ~ConfigManager() = default;

  // 配置管理
  bool HasKey(const std::string &key) const;

  // 类型安全的配置获取方法
  std::string GetString(const std::string &key,
                        const std::string &default_value = "") const;
  int GetInt(const std::string &key, int default_value = 0) const;
  bool GetBool(const std::string &key, bool default_value = false) const;
  double GetDouble(const std::string &key, double default_value = 0.0) const;

  // 配置文件操作
  bool LoadConfig(const std::string &config_file_path,
                  const std::string &env = "");
  bool ReloadConfig();
  bool SaveToFile(const std::string &file_path) const;

  // 高级配置操作
  void SetValue(const std::string &key,
                const std::variant<bool, int, double, std::string> &value);
  void SetOperationTimeout(int timeout_ms);
  int GetOperationTimeout() const;
  std::vector<std::string> GetAllKeys() const;
  std::vector<std::string> GetKeysWithPrefix(const std::string &prefix) const;

  // 索引相关配置
  size_t GetIndexPageSize() const { return 4096; }    // 默认索引页面大小
  size_t GetIndexFanout() const { return 50; }        // 默认索引扇出
  size_t GetMaxIndexEntries() const { return 10000; } // 默认最大索引条目数

private:
  // 私有方法
  void LoadDefaultConfig();
  bool ParseConfigFile(const std::string &file_path);
  void ParseConfigLine(const std::string &line, std::string &current_section);

  // 私有成员变量
  std::unordered_map<std::string, std::string> config_map_;
  std::unordered_map<std::string, std::variant<bool, int, double, std::string>>
      config_values_;
  std::string config_file_path_;
  std::string env_;
  int operation_timeout_ms_;
  static constexpr int kDefaultOperationTimeoutMs = 5000;
};

} // namespace sqlcc

#endif // SQLCC_CONFIG_MANAGER_H