/**
 * @file config_manager.h
 * @brief 配置管理器头文件
 */

#ifndef SQLCC_CONFIG_MANAGER_H_
#define SQLCC_CONFIG_MANAGER_H_

#include <string>
#include <unordered_map>
#include <variant>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>

namespace sqlcc {

/**
 * @brief 配置值类型，支持多种数据类型
 */
using ConfigValue = std::variant<bool, int, double, std::string>;

/**
 * @brief 配置变更回调函数类型
 */
using ConfigChangeCallback = std::function<void(const std::string&, const ConfigValue&)>;

/**
 * @brief 配置管理器类
 * 
 * 负责加载、解析、管理和提供配置参数访问接口
 */
class ConfigManager {
public:
    /**
     * @brief 获取配置管理器单例实例
     * @return ConfigManager& 配置管理器引用
     */
    static ConfigManager& GetInstance();
    
    /**
     * @brief 加载配置文件
     * @param config_file_path 配置文件路径
     * @param env 环境标识，用于加载环境特定配置
     * @return bool 是否加载成功
     */
    bool LoadConfig(const std::string& config_file_path, const std::string& env = "");
    
    /**
     * @brief 重新加载配置文件
     * @return bool 是否重新加载成功
     */
    bool ReloadConfig();
    
    /**
     * @brief 获取布尔类型配置值
     * @param key 配置键
     * @param default_value 默认值
     * @return bool 配置值
     */
    bool GetBool(const std::string& key, bool default_value = false) const;
    
    /**
     * @brief 获取整数类型配置值
     * @param key 配置键
     * @param default_value 默认值
     * @return int 配置值
     */
    int GetInt(const std::string& key, int default_value = 0) const;
    
    /**
     * @brief 获取双精度浮点类型配置值
     * @param key 配置键
     * @param default_value 默认值
     * @return double 配置值
     */
    double GetDouble(const std::string& key, double default_value = 0.0) const;
    
    /**
     * @brief 获取字符串类型配置值
     * @param key 配置键
     * @param default_value 默认值
     * @return std::string 配置值
     */
    std::string GetString(const std::string& key, const std::string& default_value = "") const;
    
    /**
     * @brief 设置配置值
     * @param key 配置键
     * @param value 配置值
     * @return bool 是否设置成功
     */
    bool SetValue(const std::string& key, const ConfigValue& value);
    
    /**
     * @brief 检查配置键是否存在
     * @param key 配置键
     * @return bool 是否存在
     */
    bool HasKey(const std::string& key) const;
    
    /**
     * @brief 注册配置变更回调函数
     * @param key 配置键
     * @param callback 回调函数
     * @return int 回调ID，用于取消注册
     */
    int RegisterChangeCallback(const std::string& key, ConfigChangeCallback callback);
    
    /**
     * @brief 取消注册配置变更回调函数
     * @param callback_id 回调ID
     * @return bool 是否取消成功
     */
    bool UnregisterChangeCallback(int callback_id);
    
    /**
     * @brief 保存当前配置到文件
     * @param file_path 文件路径
     * @return bool 是否保存成功
     */
    bool SaveToFile(const std::string& file_path) const;
    
    /**
     * @brief 获取所有配置键
     * @return std::vector<std::string> 配置键列表
     */
    std::vector<std::string> GetAllKeys() const;
    
    /**
     * @brief 获取指定前缀的所有配置键
     * @param prefix 键前缀
     * @return std::vector<std::string> 配置键列表
     */
    std::vector<std::string> GetKeysWithPrefix(const std::string& prefix) const;

private:
    /**
     * @brief 构造函数（私有，实现单例模式）
     */
    ConfigManager() = default;
    
    /**
     * @brief 析构函数
     */
    ~ConfigManager() = default;
    
    /**
     * @brief 禁用拷贝构造函数
     */
    ConfigManager(const ConfigManager&) = delete;
    
    /**
     * @brief 禁用赋值操作符
     */
    ConfigManager& operator=(const ConfigManager&) = delete;
    
    /**
     * @brief 解析配置文件
     * @param file_path 文件路径
     * @return bool 是否解析成功
     */
    bool ParseConfigFile(const std::string& file_path);
    
    /**
     * @brief 解析配置行
     * @param line 配置行
     * @param current_section 当前配置节
     * @return bool 是否解析成功
     */
    bool ParseConfigLine(const std::string& line, std::string& current_section);
    
    /**
     * @brief 加载默认配置
     */
    void LoadDefaultConfig();
    
    /**
     * @brief 通知配置变更
     * @param key 配置键
     * @param new_value 新值
     */
    void NotifyConfigChange(const std::string& key, const ConfigValue& new_value);
    
    /**
     * @brief 配置映射表
     */
    std::unordered_map<std::string, ConfigValue> config_map_;
    
    /**
     * @brief 配置变更回调映射表
     */
    std::unordered_map<std::string, std::vector<std::pair<int, ConfigChangeCallback>>> callbacks_;
    
    /**
     * @brief 下一个回调ID
     */
    int next_callback_id_ = 0;
    
    /**
     * @brief 配置文件路径
     */
    std::string config_file_path_;
    
    /**
     * @brief 环境标识
     */
    std::string env_;
    
    /**
     * @brief 互斥锁，保护配置访问
     */
    mutable std::mutex config_mutex_;
};

}  // namespace sqlcc

#endif  // SQLCC_CONFIG_MANAGER_H_