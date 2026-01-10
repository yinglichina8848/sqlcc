 # SQLCC 配置管理器设计文档

## 概述

SQLCC配置管理器是一个线程安全的配置管理系统，支持多种数据类型的配置项管理，包括字符串、整数、布尔值和浮点数。采用单例模式和互斥锁确保线程安全，支持配置文件的加载、保存和运行时修改。

## 核心特性

- **线程安全**: 使用std::mutex保护并发访问
- **类型安全**: 使用std::variant支持多种配置值类型
- **单例模式**: 全局唯一配置管理器实例
- **配置文件支持**: 支持INI格式的配置文件
- **运行时修改**: 支持动态配置修改和保存
- **默认值支持**: 为所有配置项提供默认值

## 设计目标

1. **线程安全配置管理**: 多线程环境下安全访问配置
2. **类型安全配置访问**: 防止类型错误和数据损坏
3. **简单易用API**: 直观简洁的配置访问接口
4. **配置文件持久化**: 支持配置的加载和保存
5. **运行时配置修改**: 支持程序运行时修改配置

## 架构设计

### 核心组件

1. **ConfigManager类**: 主配置管理器，采用单例模式
   - std::unordered_map<std::string, ConfigValue> 配置存储
   - std::mutex config_mutex_ 线程安全保护
   - 单例模式的静态实例管理

2. **ConfigValue类型**: 配置值类型定义
   - std::variant<bool, int, double, std::string> 支持四种基本类型
   - 类型安全的配置值存储和访问

3. **配置文件解析**: INI格式配置文件支持
   - 节(section)支持：[section_name]
   - 键值对支持：key=value
   - 注释支持：# 和 ; 开头的行

### 类图设计

```
+------------------------+
|     ConfigManager      |
+------------------------+
| - instance_: unique_ptr |
| - config_mutex_: mutex  |
| - config_map_: map      |
| - config_file_path_: string |
| - env_: string          |
+------------------------+
| + GetInstance(): ref    |
| + LoadConfig(): bool    |
| + ReloadConfig(): bool  |
| + SetValue(): bool      |
| + GetString(): string   |
| + GetInt(): int         |
| + GetBool(): bool       |
| + GetDouble(): double   |
| + HasKey(): bool        |
| + SaveToFile(): bool    |
| + GetAllKeys(): vector  |
| + GetKeysWithPrefix(): vector |
+------------------------+
```

## 实现细节

### 单例模式实现

使用std::call_once确保线程安全的单例创建：

```cpp
class ConfigManager {
public:
    static ConfigManager& GetInstance();
    
private:
    static std::unique_ptr<ConfigManager> instance_;
    static std::once_flag init_flag_;
    
    ConfigManager() = default;
    // 禁止拷贝和移动
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;
};
```

### 配置值类型定义

```cpp
// 配置值类型 - 支持四种基本类型
using ConfigValue = std::variant<bool, int, double, std::string>;
```

### 线程安全配置访问

所有配置访问都通过互斥锁保护：

```cpp
bool ConfigManager::SetValue(const std::string &key, const ConfigValue &value) {
    std::lock_guard<std::mutex> lock(config_mutex_);
    config_map_[key] = value;
    return true;
}

std::string ConfigManager::GetString(const std::string &key, const std::string &default_value) const {
    std::lock_guard<std::mutex> lock(config_mutex_);
    auto it = config_map_.find(key);
    if (it != config_map_.end()) {
        try {
            return std::get<std::string>(it->second);
        } catch (const std::bad_variant_access &) {
            // 类型不匹配时尝试转换
            if (std::holds_alternative<int>(it->second)) {
                return std::to_string(std::get<int>(it->second));
            }
            // ... 其他类型转换
        }
    }
    return default_value;
}
```

### 配置文件格式

支持标准INI格式配置文件：

```ini
# SQLCC配置文件示例
# 支持注释行（以#或;开头）

[database]
# 数据库配置
db_file_path = ./sqlcc.db
max_connections = 100
page_size = 8192

[buffer_pool]
# 缓冲池配置
pool_size = 64
enable_prefetch = true
prefetch_threshold = 0.8

[performance]
# 性能配置
query_cache_size = 100
worker_threads = 8
batch_size = 1000
```

### 配置文件解析

支持节(section)和键值对解析：

```cpp
bool ConfigManager::ParseConfigLine(const std::string& line, std::string& current_section) {
    // 跳过空行和注释
    if (line.empty() || line[0] == '#' || line[0] == ';') {
        return true;
    }

    // 处理节标题 [section]
    if (line[0] == '[' && line.back() == ']') {
        current_section = line.substr(1, line.length() - 2);
        return true;
    }

    // 处理键值对 key=value
    size_t equal_pos = line.find('=');
    if (equal_pos != std::string::npos) {
        std::string key = line.substr(0, equal_pos);
        std::string value = line.substr(equal_pos + 1);

        // 去除首尾空格
        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t") + 1);
        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t") + 1);

        // 组合完整键名
        std::string full_key = current_section.empty() ? key : current_section + "." + key;
        return SetValue(full_key, value);
    }
    return true;
}
```

## 使用方法

### 获取配置管理器实例

```cpp
// 获取单例实例
ConfigManager& config = ConfigManager::GetInstance();
```

### 加载配置文件

```cpp
// 加载配置文件
if (config.LoadConfig("./config/sqlcc.conf", "production")) {
    std::cout << "Config loaded successfully" << std::endl;
} else {
    std::cout << "Failed to load config, using defaults" << std::endl;
}
```

### 配置值访问

```cpp
// 获取字符串配置
std::string db_path = config.GetString("database.db_file_path", "./default.db");

// 获取整数配置
int pool_size = config.GetInt("buffer_pool.pool_size", 64);

// 获取布尔配置
bool enable_prefetch = config.GetBool("buffer_pool.enable_prefetch", true);

// 获取浮点配置
double threshold = config.GetDouble("buffer_pool.prefetch_threshold", 0.8);

// 检查配置是否存在
if (config.HasKey("performance.worker_threads")) {
    int threads = config.GetInt("performance.worker_threads", 4);
}
```

### 配置值设置

```cpp
// 设置配置值
config.SetValue("database.max_connections", 200);
config.SetValue("buffer_pool.enable_prefetch", false);
config.SetValue("performance.query_cache_size", 150);
```

### 保存配置

```cpp
// 保存配置到文件
if (config.SaveToFile("./config/sqlcc_backup.conf")) {
    std::cout << "Config saved successfully" << std::endl;
}
```

### 获取配置键列表

```cpp
// 获取所有配置键
std::vector<std::string> all_keys = config.GetAllKeys();

// 获取指定前缀的配置键
std::vector<std::string> db_keys = config.GetKeysWithPrefix("database");
```

## 线程安全保证

### 互斥锁保护

所有配置访问都通过std::mutex保护：

```cpp
class ConfigManager {
private:
    mutable std::mutex config_mutex_;
    std::unordered_map<std::string, ConfigValue> config_map_;
    
public:
    // 所有公共方法都使用锁保护
    std::string GetString(const std::string &key, const std::string &default_value) const {
        std::lock_guard<std::mutex> lock(config_mutex_);
        // ... 实现
    }
};
```

### 锁的粒度

- **方法级锁**: 每个公共方法独立加锁
- **读写分离**: 目前使用独占锁，可扩展为读写锁
- **异常安全**: 锁在作用域结束时自动释放

## 性能优化

### 哈希表存储

使用std::unordered_map提供O(1)平均查找时间：

```cpp
std::unordered_map<std::string, ConfigValue> config_map_;
```

### 延迟初始化

单例模式使用std::call_once实现延迟初始化：

```cpp
static std::unique_ptr<ConfigManager> instance_;
static std::once_flag init_flag_;

ConfigManager& GetInstance() {
    std::call_once(init_flag_, []() {
        instance_ = std::unique_ptr<ConfigManager>(new ConfigManager());
    });
    return *instance_;
}
```

### 类型转换优化

智能类型转换，支持不同类型间的自动转换：

```cpp
// 字符串转整数
if (std::holds_alternative<std::string>(it->second)) {
    return std::stoi(std::get<std::string>(it->second));
}

// 布尔值转字符串
if (std::holds_alternative<bool>(it->second)) {
    return std::get<bool>(it->second) ? "true" : "false";
}
```

## 测试框架

### 单元测试覆盖

配置管理器提供完整的单元测试：

```bash
# 运行配置管理器测试
cd build && make config_manager_test
./bin/config_manager_test

# 运行线程安全测试
cd build && make config_thread_safety_test
./bin/config_thread_safety_test
```

### 测试用例

1. **基本功能测试**: 配置的设置、获取、保存、加载
2. **类型转换测试**: 不同类型间的自动转换
3. **线程安全测试**: 多线程并发访问测试
4. **异常处理测试**: 配置文件损坏、类型错误等异常情况
5. **边界条件测试**: 空值、超长字符串、大整数等边界情况

## 扩展性设计

### 新配置类型支持

易于扩展支持新的配置值类型：

```cpp
// 添加新类型到variant
using ConfigValue = std::variant<bool, int, double, std::string, std::vector<std::string>>;

// 添加对应的getter方法
std::vector<std::string> GetStringList(const std::string &key, const std::vector<std::string> &default_value) const;
```

### 配置验证器扩展

支持配置值验证：

```cpp
class ConfigValidator {
public:
    virtual bool Validate(const std::string& key, const ConfigValue& value) = 0;
};

// 范围验证器
class RangeValidator : public ConfigValidator {
private:
    int min_value_;
    int max_value_;
    
public:
    bool Validate(const std::string& key, const ConfigValue& value) override {
        if (std::holds_alternative<int>(value)) {
            int int_value = std::get<int>(value);
            return int_value >= min_value_ && int_value <= max_value_;
        }
        return false;
    }
};
```

### 配置监听器扩展

支持配置变更监听：

```cpp
using ConfigChangeCallback = std::function<void(const std::string&, const ConfigValue&)>;

class ConfigListener {
private:
    std::vector<ConfigChangeCallback> callbacks_;
    
public:
    void AddCallback(ConfigChangeCallback callback) {
        callbacks_.push_back(callback);
    }
    
    void NotifyChange(const std::string& key, const ConfigValue& value) {
        for (auto& callback : callbacks_) {
            callback(key, value);
        }
    }
};
```

## 总结

SQLCC配置管理器提供了简单高效的配置管理解决方案：

### 核心优势

1. **线程安全**: std::mutex确保多线程环境下的安全访问
2. **类型安全**: std::variant提供类型安全的配置值存储
3. **易于使用**: 直观的API设计，支持多种配置类型
4. **配置文件支持**: 标准INI格式，支持节和注释
5. **运行时修改**: 支持动态配置修改和持久化

### 性能特点

- **快速访问**: O(1)平均查找时间
- **内存高效**: 最小化内存占用
- **锁优化**: 方法级锁平衡安全性和性能
- **延迟初始化**: 单例模式避免不必要的初始化开销

### 扩展性

- **类型扩展**: 易于添加新的配置值类型
- **验证扩展**: 支持配置值验证器
- **监听扩展**: 支持配置变更监听
- **格式扩展**: 可扩展支持其他配置文件格式

### 应用场景

- **数据库配置**: 连接参数、缓冲池设置等
- **性能调优**: 缓存大小、线程数、超时设置等
- **功能开关**: 特性启用/禁用开关
- **环境配置**: 不同环境的参数配置

这个配置管理器为SQLCC提供了稳定可靠的配置管理基础，支持从简单应用到复杂分布式系统的各种配置需求。
