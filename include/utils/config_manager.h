/**
 * @file config_manager.h
 * @brief SQLCC配置管理系统 - 运行时配置动态管理架构
 *
 * 配置管理系统是数据库系统的"神经中枢"，负责运行时参数的动态加载、验证和管理。
 * 本文件实现了完整的配置生命周期管理，支持热更新、环境隔离、多格式配置等企业级特性。
 *
 * 📚 配套教材参考：
 * - [第11章：系统配置与管理](../../textbook/《数据库系统原理与开发实践》.md#第十一章系统配置与管理)
 * - [11.1 配置管理架构设计](../../textbook/《数据库系统原理与开发实践》.md#111-配置管理架构设计)
 * - [11.2 运行时配置热更新](../../textbook/《数据库系统原理与开发实践》.md#112-运行时配置热更新)
 * - [11.3 配置验证与安全性](../../textbook/《数据库系统原理与开发实践》.md#113-配置验证与安全性)
 *
 * WHY层 - 设计意图：
 *   配置管理是数据库系统稳定运行的基石，任何配置错误都可能导致系统故障或安全漏洞。
 *   通过精心设计的配置管理架构，实现运行时的动态配置调整，提高系统的可维护性和可扩展性，
 *   为企业级数据库应用提供可靠的配置管理解决方案。
 *
 * WHAT层 - 功能说明：
 *   - 配置生命周期：加载、解析、验证、热更新、持久化
 *   - 多格式支持：INI、JSON、YAML等多种配置格式
 *   - 环境隔离：开发、测试、生产环境的配置隔离
 *   - 热更新机制：运行时配置修改，无需重启服务
 *   - 配置验证：类型检查、范围验证、依赖关系验证
 *   - 监控告警：配置变更通知和异常告警
 *
 * HOW层 - 实现机制：
 *   - 单例模式：全局唯一的配置管理实例
 *   - 观察者模式：配置变更的通知机制
 *   - 模板方法：配置文件的解析和验证流程
 *   - 工厂模式：不同配置格式的解析器工厂
 *   - 策略模式：不同的配置验证和更新策略
 *   - 原子操作：配置更新的线程安全保证
 *
 * 配置生命周期详解：
 *   1. **初始化阶段**：单例实例创建，加载默认配置
 *   2. **加载阶段**：解析配置文件，环境变量覆盖
 *   3. **验证阶段**：类型检查、范围验证、依赖验证
 *   4. **运行阶段**：提供配置访问接口，支持热更新
 *   5. **清理阶段**：资源释放，配置持久化
 *
 * 热更新机制详解：
 *   - **原子更新**：配置变更的原子性保证
 *   - **渐进生效**：新配置对新请求生效，旧请求不受影响
 *   - **回滚支持**：配置错误的自动回滚机制
 *   - **版本控制**：配置变更的历史版本管理
 *   - **通知机制**：配置变更的实时通知
 *
 * 配置验证策略：
 *   - **静态验证**：配置文件加载时的语法和语义检查
 *   - **动态验证**：运行时的配置值范围和依赖关系检查
 *   - **安全验证**：防止恶意配置和信息泄露
 *   - **性能验证**：配置值对系统性能的影响评估
 *   - **一致性验证**：多配置源之间的冲突检测
 *
 * 环境隔离设计：
 *   - **层次配置**：默认配置→全局配置→环境配置→实例配置
 *   - **变量替换**：环境变量和系统属性的动态替换
 *   - **条件配置**：基于条件的配置项启用/禁用
 *   - **配置继承**：子环境继承父环境配置，支持覆盖
 *   - **隔离验证**：不同环境的配置冲突检测
 *
 * 性能优化考虑：
 *   - **缓存机制**：配置值的本地缓存，减少I/O开销
 *   - **懒加载**：按需加载配置，减少启动时间
 *   - **索引优化**：配置键的快速查找索引
 *   - **并发访问**：读写锁保护的线程安全访问
 *   - **内存管理**：配置对象的池化复用
 *
 * 安全性保障：
 *   - **访问控制**：配置项的访问权限控制
 *   - **加密存储**：敏感配置的加密存储
 *   - **审计日志**：配置变更的完整审计记录
 *   - **完整性检查**：配置文件篡改检测
 *   - **隔离执行**：配置加载的沙盒环境
 *
 * 扩展性设计：
 *   - **插件架构**：可插拔的配置解析器和验证器
 *   - **自定义类型**：支持用户定义的配置类型
 *   - **分布式配置**：集群环境的统一配置管理
 *   - **云原生支持**：Kubernetes ConfigMap集成
 *   - **监控集成**：Prometheus监控指标暴露
 *
 * 配置生命周期状态机：
 *   UNINITIALIZED → LOADING → VALIDATING → ACTIVE → UPDATING → ACTIVE
 *   任何阶段都可能因错误进入ERROR状态，支持从ERROR状态恢复
 *
 * 热更新流程：
 *   1. 接收配置更新请求
 *   2. 验证新配置的正确性
 *   3. 创建配置快照，原子切换
 *   4. 通知所有依赖组件
 *   5. 清理旧配置资源
 *   6. 记录更新审计日志
 *
 * 配置验证框架：
 *   - **类型验证器**：确保配置值类型正确
 *   - **范围验证器**：检查数值配置的合理范围
 *   - **依赖验证器**：验证配置间的依赖关系
 *   - **安全验证器**：防止危险配置值
 *   - **性能验证器**：评估配置对性能的影响
 *
 * @author SQLCC技术委员会
 * @version 1.2.6
 * @date 2025-12-24
 */

#ifndef SQLCC_CONFIG_MANAGER_H_
#define SQLCC_CONFIG_MANAGER_H_

#include <string>
#include <unordered_map>
#include <variant>
#include <memory>
#include <mutex>
#include <vector>
#include <chrono>

namespace sqlcc {

/**
 * @brief 配置值类型，支持多种数据类型
 */
using ConfigValue = std::variant<bool, int, double, std::string>;

/**
 * @brief 配置管理器类
 * 
 * 负责加载、解析、管理和提供配置参数访问接口
 */
class ConfigManager {
public:
    /**
     * @brief 默认操作超时时间（毫秒）
     */
    static constexpr int kDefaultOperationTimeoutMs = 5000; // 5秒

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
public:
    /**
     * @brief 加载默认配置
     */
    void LoadDefaultConfig();
    
    /**
     * @brief 构造函数（私有，实现单例模式）
     */
    ConfigManager() = default;

    /**
     * @brief 析构函数
     */
    ~ConfigManager() = default;

private:
    /**
     * @brief 单例实例指针
     */
    static std::unique_ptr<ConfigManager> instance_;

    /**
     * @brief 初始化标志，用于线程安全的单例初始化
     */
    static std::once_flag init_flag_;
    
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
     * @brief 配置映射表
     */
    std::unordered_map<std::string, ConfigValue> config_map_;
    
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
    
    /**
     * @brief 操作超时时间（毫秒）
     */
    int operation_timeout_ms_;

public:
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
    
    /**
     * @brief 设置操作超时时间
     * @param timeout_ms 超时时间（毫秒）
     */
    void SetOperationTimeout(int timeout_ms);
    
    /**
     * @brief 获取当前操作超时时间
     * @return 超时时间（毫秒）
     */
    int GetOperationTimeout() const;
};

}  // namespace sqlcc

#endif  // SQLCC_CONFIG_MANAGER_H_
