# ConfigManager类详细设计

## 概述

ConfigManager是配置管理器的核心类，采用单例模式设计，负责加载、解析、管理和提供配置参数访问接口。它支持多种数据类型的配置值，并提供线程安全的访问机制。

## 类定义

```cpp
class ConfigManager {
public:
    static constexpr int kDefaultOperationTimeoutMs = 5000;
    
    static ConfigManager& GetInstance();
    bool LoadConfig(const std::string& config_file_path, const std::string& env = "");
    bool ReloadConfig();
    void LoadDefaultConfig();
    
    bool GetBool(const std::string& key, bool default_value = false) const;
    int GetInt(const std::string& key, int default_value = 0) const;
    double GetDouble(const std::string& key, double default_value = 0.0) const;
    std::string GetString(const std::string& key, const std::string& default_value = "") const;
    bool SetValue(const std::string& key, const ConfigValue& value);
    bool HasKey(const std::string& key) const;
    bool SaveToFile(const std::string& file_path) const;
    std::vector<std::string> GetAllKeys() const;
    std::vector<std::string> GetKeysWithPrefix(const std::string& prefix) const;
    void SetOperationTimeout(int timeout_ms);
    int GetOperationTimeout() const;

private:
    ConfigManager() = default;
    ~ConfigManager() = default;
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;
    bool ParseConfigFile(const std::string& file_path);
    bool ParseConfigLine(const std::string& line, std::string& current_section);

private:
    std::unordered_map<std::string, ConfigValue> config_map_;
    std::string config_file_path_;
    std::string env_;
    mutable std::mutex config_mutex_;
    int operation_timeout_ms_;
};
```

## 静态成员

### static constexpr int kDefaultOperationTimeoutMs

默认操作超时时间：

1. 设置为5000毫秒（5秒）
2. 用于配置操作的默认超时时间

## 构造函数和析构函数

### ConfigManager()

私有构造函数（实现单例模式）：

1. 默认构造函数
2. 初始化成员变量

### ~ConfigManager()

析构函数：

1. 默认析构函数
2. 无需特殊处理

### ConfigManager(const ConfigManager&) = delete

禁用拷贝构造函数：

1. 防止意外复制单例对象

### ConfigManager& operator=(const ConfigManager&) = delete

禁用赋值操作符：

1. 防止意外赋值单例对象

## 公共方法

### static ConfigManager& GetInstance()

获取配置管理器单例实例：

1. 返回ConfigManager的单例引用
2. 使用局部静态变量实现线程安全的单例模式

### bool LoadConfig(const std::string& config_file_path, const std::string& env)

加载配置文件：

1. 获取配置互斥锁
2. 存储配置文件路径和环境标识
3. 解析配置文件
4. 加载环境特定配置（如果指定）
5. 返回是否加载成功

### bool ReloadConfig()

重新加载配置文件：

1. 使用当前配置文件路径重新加载配置
2. 保持环境标识不变

### void LoadDefaultConfig()

加载默认配置：

1. 设置系统默认配置值
2. 包括缓冲池大小、页面大小等关键参数

### bool GetBool(const std::string& key, bool default_value) const

获取布尔类型配置值：

1. 获取配置互斥锁
2. 在配置映射表中查找指定键
3. 如果找到且类型匹配则返回值，否则返回默认值

### int GetInt(const std::string& key, int default_value) const

获取整数类型配置值：

1. 获取配置互斥锁
2. 在配置映射表中查找指定键
3. 如果找到且类型匹配则返回值，否则返回默认值

### double GetDouble(const std::string& key, double default_value) const

获取双精度浮点类型配置值：

1. 获取配置互斥锁
2. 在配置映射表中查找指定键
3. 如果找到且类型匹配则返回值，否则返回默认值

### std::string GetString(const std::string& key, const std::string& default_value) const

获取字符串类型配置值：

1. 获取配置互斥锁
2. 在配置映射表中查找指定键
3. 如果找到则返回值，否则返回默认值

### bool SetValue(const std::string& key, const ConfigValue& value)

设置配置值：

1. 获取配置互斥锁
2. 在配置映射表中设置指定键值对
3. 触发配置变更回调（如果有注册）
4. 返回是否设置成功

### bool HasKey(const std::string& key) const

检查配置键是否存在：

1. 获取配置互斥锁
2. 检查配置映射表是否包含指定键

### bool SaveToFile(const std::string& file_path) const

保存当前配置到文件：

1. 获取配置互斥锁
2. 打开指定文件
3. 将配置映射表内容写入文件
4. 返回是否保存成功

### std::vector<std::string> GetAllKeys() const

获取所有配置键：

1. 获取配置互斥锁
2. 收集配置映射表中的所有键
3. 返回键列表

### std::vector<std::string> GetKeysWithPrefix(const std::string& prefix) const

获取指定前缀的所有配置键：

1. 获取配置互斥锁
2. 遍历配置映射表
3. 收集具有指定前缀的键
4. 返回键列表

### void SetOperationTimeout(int timeout_ms)

设置操作超时时间：

1. 设置operation_timeout_ms_成员变量

### int GetOperationTimeout() const

获取当前操作超时时间：

1. 返回operation_timeout_ms_成员变量

## 私有方法

### bool ParseConfigFile(const std::string& file_path)

解析配置文件：

1. 打开配置文件
2. 逐行解析配置内容
3. 调用ParseConfigLine处理每行
4. 返回是否解析成功

### bool ParseConfigLine(const std::string& line, std::string& current_section)

解析配置行：

1. 跳过空白行和注释行
2. 识别节（section）定义
3. 解析键值对
4. 将配置项添加到配置映射表

## 成员变量

### std::unordered_map<std::string, ConfigValue> config_map_

配置映射表：

1. 存储所有配置项的键值对
2. 键为字符串，值为ConfigValue对象

### std::string config_file_path_

配置文件路径：

1. 存储当前加载的配置文件路径
2. 用于重新加载配置

### std::string env_

环境标识：

1. 存储当前环境标识（如dev、test、prod）
2. 用于加载环境特定配置

### mutable std::mutex config_mutex_

互斥锁：

1. 保护配置访问的线程安全
2. 使用mutable关键字允许在const方法中使用

### int operation_timeout_ms_

操作超时时间（毫秒）：

1. 存储配置操作的超时时间
2. 用于控制配置访问的等待时间