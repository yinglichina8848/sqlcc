# 配置管理器设计文档

## 概述

配置管理器是SQLCC数据库系统的核心组件之一，负责加载、解析、管理和提供配置参数访问接口。本文档详细描述了配置管理器的设计、实现和使用方法。

## 设计目标

1. **统一配置管理**：提供统一的配置参数管理接口，支持多种数据类型
2. **动态配置**：支持运行时修改配置参数，并通知相关组件
3. **配置持久化**：支持将配置保存到文件，并在系统启动时加载
4. **配置变更通知**：支持注册配置变更回调函数，当配置值发生变化时自动通知
5. **线程安全**：确保多线程环境下的配置访问安全

## 架构设计

### 核心组件

1. **ConfigManager类**：配置管理器的主要实现类，采用单例模式
2. **ConfigValue类型**：支持多种数据类型的配置值（bool、int、double、string）
3. **ConfigChangeCallback类型**：配置变更回调函数类型
4. **配置文件解析器**：负责解析配置文件内容

### 类图

```
+----------------+
|  ConfigManager |
+----------------+
| - config_map_  |
| - callbacks_   |
| - next_callback_id_ |
+----------------+
| + GetInstance()|
| + LoadConfig() |
| + GetBool()    |
| + GetInt()     |
| + GetDouble()  |
| + GetString()  |
| + SetValue()   |
| + HasKey()     |
| + RegisterChangeCallback() |
| + UnregisterChangeCallback() |
| + SaveToFile() |
| + GetAllKeys() |
| + GetKeysWithPrefix() |
+----------------+
```

## 实现细节

### 单例模式

配置管理器采用单例模式，确保整个系统中只有一个配置管理器实例：

```cpp
class ConfigManager {
public:
    static ConfigManager& GetInstance();
    
private:
    ConfigManager() = default;
    ~ConfigManager() = default;
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;
};
```

### 配置值类型

使用`std::variant`实现多类型配置值：

```cpp
using ConfigValue = std::variant<bool, int, double, std::string>;
```

### 配置文件格式

配置文件采用简单的键值对格式，支持节（section）组织：

```ini
[database]
db_file_path = "./sqlcc.db"
page_size = 8192

[buffer_pool]
pool_size = 64
enable_prefetch = true
prefetch_threshold = 0.8
```

### 配置变更回调

支持注册配置变更回调函数，当配置值发生变化时自动通知：

```cpp
using ConfigChangeCallback = std::function<void(const std::string&, const ConfigValue&)>;

int RegisterChangeCallback(const std::string& key, ConfigChangeCallback callback);
bool UnregisterChangeCallback(int callback_id);
```

## 使用方法

### 获取配置管理器实例

```cpp
ConfigManager& config = ConfigManager::GetInstance();
```

### 加载配置文件

```cpp
if (config.LoadConfig("./config/sqlcc.conf")) {
    std::cout << "Config file loaded successfully!" << std::endl;
} else {
    std::cout << "Failed to load config file, using default settings" << std::endl;
}
```

### 获取配置值

```cpp
// 获取字符串配置
std::string db_path = config.GetString("database.db_file_path", "./default.db");

// 获取整数配置
int pool_size = config.GetInt("buffer_pool.pool_size", 64);

// 获取布尔配置
bool enable_prefetch = config.GetBool("buffer_pool.enable_prefetch", true);

// 获取双精度配置
double prefetch_threshold = config.GetDouble("buffer_pool.prefetch_threshold", 0.8);
```

### 设置配置值

```cpp
// 设置字符串配置
config.SetValue("database.db_file_path", std::string("./test.db"));

// 设置整数配置
config.SetValue("buffer_pool.pool_size", 128);

// 设置布尔配置
config.SetValue("buffer_pool.enable_prefetch", false);

// 设置双精度配置
config.SetValue("buffer_pool.prefetch_threshold", 0.9);
```

### 注册配置变更回调

```cpp
bool callback_called = false;
std::string callback_key;
ConfigValue callback_value;

// 注册回调函数
int callback_id = config.RegisterChangeCallback("test.value", 
    [&callback_called, &callback_key, &callback_value](const std::string& key, const ConfigValue& value) {
        callback_called = true;
        callback_key = key;
        callback_value = value;
        std::cout << "Callback triggered for key: " << key << std::endl;
    });

// 修改配置值，触发回调
config.SetValue("test.value", 42);
```

### 保存配置到文件

```cpp
if (config.SaveToFile("./config/sqlcc_test.conf")) {
    std::cout << "Config file saved successfully!" << std::endl;
} else {
    std::cout << "Failed to save config file!" << std::endl;
}
```

## 线程安全

配置管理器使用互斥锁确保多线程环境下的配置访问安全：

```cpp
class ConfigManager {
private:
    mutable std::mutex config_mutex_;
    
    // 在所有访问和修改配置的方法中使用
    std::lock_guard<std::mutex> lock(config_mutex_);
    // ...
};
```

## 性能考虑

1. **配置缓存**：所有配置值都缓存在内存中，避免频繁的文件访问
2. **读写锁**：读多写少的场景下，可以考虑使用读写锁提高性能
3. **延迟通知**：配置变更回调可以采用异步通知方式，避免阻塞配置设置操作

## 扩展性

1. **配置加密**：可以扩展支持配置值的加密存储
2. **配置分层**：可以支持多层次的配置，如全局配置、用户配置等
3. **配置验证**：可以添加配置值的验证规则，确保配置值的有效性
4. **配置热重载**：可以支持监控配置文件变化，自动重载配置

## 测试

配置管理器提供了完整的测试程序，验证所有功能的正确性：

```bash
cd /home/liying/sqlcc/build && make config_test
./build/bin/config_test
```

测试包括：
1. 配置文件加载测试
2. 各种类型配置值的获取和设置测试
3. 配置保存测试
4. 配置变更回调测试

## 总结

配置管理器是SQLCC数据库系统的重要组件，提供了灵活、安全、高效的配置管理功能。通过单例模式、多类型支持、变更回调等特性，满足了系统对配置管理的各种需求。