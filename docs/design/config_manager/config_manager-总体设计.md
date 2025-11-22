# 配置管理器组件

## 概述

配置管理器是SQLCC数据库管理系统的基础组件，负责加载、解析、管理和提供配置参数访问接口。它采用单例模式设计，支持动态配置变更和线程安全访问。

## 核心类

1. [ConfigManager](config_manager.md) - 配置管理器主类，提供统一的配置管理接口
2. [ConfigValue](config_value.md) - 配置值类，支持多种数据类型的配置值

## 组件关系

```
ConfigManager
    |
    |-- ConfigValue (多种数据类型支持)
    |-- Configuration Files (配置文件解析)
    `-- Change Notifications (变更通知)
```

## 主要功能

1. **统一配置管理**：提供统一的配置参数管理接口，支持多种数据类型
2. **动态配置**：支持运行时修改配置参数，并通知相关组件
3. **配置持久化**：支持将配置保存到文件，并在系统启动时加载
4. **配置变更通知**：支持注册配置变更回调函数，当配置值发生变化时自动通知
5. **线程安全**：确保多线程环境下的配置访问安全
6. **环境特定配置**：支持不同环境（开发、测试、生产）的配置加载

## 支持的数据类型

1. **布尔类型**：true/false值
2. **整数类型**：整数值
3. **浮点类型**：双精度浮点数值
4. **字符串类型**：字符串值

## 配置文件格式

配置文件采用简单的键值对格式，支持节（section）组织：

```ini
[database]
file_path = /path/to/database/file
page_size = 8192

[buffer_pool]
size = 1024
replacement_policy = lru

[performance]
enable_prefetch = true
prefetch_size = 8
```

## 配置变更通知

配置管理器支持注册回调函数，当特定配置项发生变化时自动通知相关组件：

```cpp
// 注册配置变更回调
config_manager.RegisterChangeCallback("buffer_pool.size", [](const std::string& key, const ConfigValue& value) {
    // 处理缓冲池大小变更
    AdjustBufferPoolSize(value.GetInt());
});
```