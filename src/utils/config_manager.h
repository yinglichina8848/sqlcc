/**
 * @file config_manager.h
 * @brief SQLCC配置管理器 - 系统参数与运行时配置中心
 *
 * ConfigManager 是数据库系统的"中枢神经"，负责管理所有启动参数、
 * 运行时配置和环境设置。它提供了统一的接口来读取、修改和持久化配置，
 * 确保系统组件能够根据预设策略正确运行。
 *
 * 📚 配套教材参考：
 * - [第12章：系统管理与维护](../../textbook/《数据库系统原理与开发实践》.md#第十二章系统管理与维护)
 * - [12.1 数据库参数配置](../../textbook/《数据库系统原理与开发实践》.md#121-数据库参数配置)
 * - [12.2 动态性能调优](../../textbook/《数据库系统原理与开发实践》.md#122-动态性能调优)
 *
 * WHY层 - 设计意图：
 *   1. **集中管理**：避免配置散落在代码硬编码中，提供单一的修改入口。
 *   2. **动态调整**：支持在不重启数据库的情况下热加载配置（Hot Reload），满足高可用需求。
 *   3. **类型安全**：封装了配置值的类型转换（int, bool, string...），防止类型错误。
 *   4. **持久化**：确保运行时修改的配置能够保存到磁盘，系统重启后生效。
 *
 * WHAT层 - 功能说明：
 *   - 配置读取：支持从文件（ini/json/yaml风格）加载配置。
 *   - 类型访问：提供 GetInt, GetString, GetBool 等强类型接口。
 *   - 热更新：支持 ReloadConfig 重新加载配置文件。
 *   - 线程安全：所有操作均受互斥锁保护，支持并发访问。
 *   - 默认值机制：在配置缺失时提供安全的默认值回退。
 *
 * HOW层 - 实现机制：
 *   - **单例模式**：全局唯一的 ConfigManager 实例，保证配置的一致性。
 *   - **键值存储**：内部使用 `unordered_map<string, ConfigValue>` 存储配置项，
 *     其中 `ConfigValue` 是 `std::variant` 类型，支持多种数据类型。
 *   - **读写锁策略**：配置读取远多于修改，内部使用互斥锁（未来可优化为读写锁）保护。
 *   - **原子文件写入**：保存配置时先写入临时文件再重命名，防止文件损坏。
 *
 * @author SQLCC技术委员会
 * @version 1.2.6
 * @date 2026-02-02
 */

#ifndef SQLCC_CONFIG_MANAGER_H
#define SQLCC_CONFIG_MANAGER_H

#include <string>
#include <unordered_map>
#include <variant>
#include <memory>
#include <mutex>
#include <vector>

namespace sqlcc {

/**
 * @brief 配置值类型定义
 * 使用 std::variant 支持多种基础类型，替代传统的 void* 或 string 转换。
 */
using ConfigValue = std::variant<bool, int, double, std::string>;

// 默认操作超时时间
const int kDefaultOperationTimeoutMs = 30000;

/**
 * @class ConfigManager
 * @brief 配置管理器 - 线程安全的单例配置中心
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

    /**
     * @brief 获取单例实例
     * 线程安全的懒汉式单例获取。
     */
    static ConfigManager& GetInstance();

    // --- 配置操作 ---

    /**
     * @brief 设置配置值
     * @param key 配置键（通常是 section.key 格式）
     * @param value 配置值（自动推导类型）
     * @return 是否设置成功
     */
    bool SetValue(const std::string &key, const ConfigValue &value);

    /**
     * @brief 检查配置键是否存在
     */
    bool HasKey(const std::string &key) const;

    // --- 类型化获取方法 ---

    /**
     * @brief 获取字符串配置
     * @param key 配置键
     * @param default_value 默认值（当键不存在或类型不匹配时返回）
     */
    std::string GetString(const std::string &key, const std::string &default_value = "") const;

    /**
     * @brief 获取整数配置
     */
    int GetInt(const std::string &key, int default_value = 0) const;

    /**
     * @brief 获取布尔配置
     */
    bool GetBool(const std::string &key, bool default_value = false) const;

    /**
     * @brief 获取浮点数配置
     */
    double GetDouble(const std::string &key, double default_value = 0.0) const;

    // --- 文件操作 ---

    /**
     * @brief 加载配置文件
     * @param config_file_path 配置文件路径
     * @param env 环境标识（如 "prod", "dev"），用于加载特定环境覆盖
     */
    bool LoadConfig(const std::string& config_file_path, const std::string& env = "");

    /**
     * @brief 重新加载当前配置文件
     * 用于运行时热更新配置。
     */
    bool ReloadConfig();

    /**
     * @brief 加载新文件（完整替换）
     */
    bool ReloadConfig(const std::string& new_config_path);

    /**
     * @brief 将当前内存配置持久化到文件
     */
    bool SaveToFile(const std::string& file_path) const;

    // --- 批量操作 ---

    std::vector<std::string> GetAllKeys() const;
    std::vector<std::string> GetKeysWithPrefix(const std::string& prefix) const;

    // --- 测试辅助方法 ---
    void ClearAll();
    void ResetForTest();

    // --- 超时设置 ---
    void SetOperationTimeout(int timeout_ms);
    int GetOperationTimeout() const;

private:
    // 私有辅助方法
    void LoadDefaultConfig();
    void LoadDefaultConfigInternal(std::unordered_map<std::string, ConfigValue>& temp_config);
    bool ParseConfigFile(const std::string& file_path);
    bool ParseConfigFileInternal(const std::string& file_path,
                                 std::unordered_map<std::string, ConfigValue>& result_config);
    bool ParseConfigLine(const std::string& line, std::string& current_section);
    
    // 无锁解析方法，避免死锁
    bool ParseConfigLineInternal(const std::string& line, std::string& current_section, 
                                 std::unordered_map<std::string, ConfigValue>& temp_config) const;
};

} // namespace sqlcc

#endif // SQLCC_CONFIG_MANAGER_H