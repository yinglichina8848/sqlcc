#ifndef SQLCC_CONFIG_MANAGER_H
#define SQLCC_CONFIG_MANAGER_H

#include <string>
#include <unordered_map>
#include <variant>
#include <memory>
#include <mutex>
#include <vector>

namespace sqlcc {

// 配置值的类型定义
using ConfigValue = std::variant<std::string, int, double, bool>;

// 默认操作超时时间
const int kDefaultOperationTimeoutMs = 30000;

/**
 * ConfigManager 类 - 传统的配置管理器
 * 提供基本的配置读取、设置和管理功能
 */
class ConfigManager {
private:
    // 单例实例
    static std::unique_ptr<ConfigManager> instance_;
    static std::once_flag init_flag_;

    // 配置数据
    mutable std::mutex config_mutex_;
    std::unordered_map<std::string, ConfigValue> config_map_;
    std::string config_file_path_;
    std::string env_;
    int operation_timeout_ms_;

private:
    // 私有构造函数（单例模式）
    ConfigManager();

public:
    // 删除拷贝构造函数和赋值运算符
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;

    // 获取单例实例
    static ConfigManager& GetInstance();

    // 配置操作
    bool SetValue(const std::string &key, const ConfigValue &value);
    bool HasKey(const std::string &key) const;

    // 类型化获取方法
    std::string GetString(const std::string &key, const std::string &default_value = "") const;
    int GetInt(const std::string &key, int default_value = 0) const;
    bool GetBool(const std::string &key, bool default_value = false) const;
    double GetDouble(const std::string &key, double default_value = 0.0) const;

    // 文件操作
    bool LoadConfig(const std::string& config_file_path, const std::string& env = "");
    bool ReloadConfig();
    bool SaveToFile(const std::string& file_path) const;

    // 批量操作
    std::vector<std::string> GetAllKeys() const;
    std::vector<std::string> GetKeysWithPrefix(const std::string& prefix) const;

    // 测试辅助方法
    void ClearAll();

    // 超时设置
    void SetOperationTimeout(int timeout_ms);
    int GetOperationTimeout() const;

private:
    // 私有辅助方法
    void LoadDefaultConfig();
    bool ParseConfigFile(const std::string& file_path);
    bool ParseConfigLine(const std::string& line, std::string& current_section);
};

} // namespace sqlcc

#endif // SQLCC_CONFIG_MANAGER_H