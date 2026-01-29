/**
 * @file config_manager_upgrade_guide.md
 * @brief 配置管理器升级指南 - 从传统实现到智能配置管理器
 * 
 * Why: 指导开发者从传统配置管理器升级到新的智能配置管理器
 * What: 提供详细的升级步骤、API对比和迁移示例
 * How: 逐步替换现有代码，确保向后兼容和平滑迁移
 */

# SQLCC v1.2.3 智能配置管理器升级指南

## 🚀 概述

本指南帮助开发者将现有的传统配置管理器升级到新的智能配置管理器，实现内存安全、RAII模式和异常安全配置。

## 📋 升级前准备

### 1. 备份现有代码
```bash
# 创建备份分支
git checkout -b backup-config-manager
git add .
git commit -m "Backup: Traditional ConfigManager before upgrade"

# 回到主分支准备升级
git checkout main
```

### 2. 检查当前使用情况
```bash
# 查找现有ConfigManager的使用位置
grep -r "ConfigManager" src/ include/ --include="*.cpp" --include="*.h"

# 统计使用频率
find src/ include/ -name "*.cpp" -o -name "*.h" | xargs grep -c "ConfigManager" | grep -v ":0"
```

## 🔄 API对比与迁移

### 传统API vs 智能API

| 传统API | 智能API | 迁移说明 |
|---------|---------|----------|
| `ConfigManager::GetInstance()` | `SmartConfigManager::GetInstance()` | 直接替换，返回智能指针 |
| `GetBool(key, default)` | `GetBoolConfig(key, default)` | 方法名变更，异常安全 |
| `GetInt(key, default)` | `GetIntConfig(key, default)` | 方法名变更，异常安全 |
| `GetDouble(key, default)` | `GetDoubleConfig(key, default)` | 方法名变更，异常安全 |
| `GetString(key, default)` | `GetStringConfig(key, default)` | 方法名变更，异常安全 |
| `Reload()` | `UpdateSnapshot()` | 使用快照机制 |
| 无热更新 | `EnableHotReload()` | 新增功能 |
| 无版本管理 | `GetCurrentVersionId()` | 新增功能 |
| 无异步操作 | `UpdateConfigAsync()` | 新增功能 |

### 基础配置访问迁移

#### 传统实现
```cpp
// 传统方式
ConfigManager* config = ConfigManager::GetInstance();
bool debug_mode = config->GetBool("debug.enabled", false);
int port = config->GetInt("server.port", 8080);
std::string host = config->GetString("server.host", "localhost");
```

#### 智能实现
```cpp
// 智能方式
auto config = SmartConfigManager::GetInstance();
bool debug_mode = config->GetBoolConfig("debug.enabled", false);
int port = config->GetIntConfig("server.port", 8080);
std::string host = config->GetStringConfig("server.host", "localhost");
```

### 高级功能迁移

#### 热更新配置
```cpp
// 启用热更新（5秒检查间隔）
auto config = SmartConfigManager::GetInstance();
config->EnableHotReload(std::chrono::milliseconds(5000));

// 配置文件修改后自动生效
// 无需手动调用Reload()
```

#### 版本管理
```cpp
// 获取当前配置版本
std::string version = config->GetCurrentVersionId();
std::cout << "Current config version: " << version << std::endl;

// 异步更新配置
auto future = config->UpdateConfigAsync("new.key", "new_value");
bool success = future.get();  // 等待更新完成
```

#### 批量配置更新
```cpp
// 批量更新配置
std::unordered_map<std::string, ConfigValue> batch_configs = {
    {"database.host", std::string("new_host")},
    {"database.port", 3306},
    {"database.ssl", true}
};

auto future = config->BatchUpdateConfigsAsync(batch_configs);
bool success = future.get();
```

## 🛠️ 逐步迁移策略

### 阶段1: 基础替换（推荐）

1. **更新头文件引用**
```cpp
// 旧的头文件
#include "utils/config_manager.h"

// 新的头文件
#include "utils/smart_config_manager.h"
```

2. **替换获取实例的调用**
```cpp
// 旧的调用
ConfigManager* config = ConfigManager::GetInstance();

// 新的调用
auto config = SmartConfigManager::GetInstance();
```

3. **更新配置访问方法**
```cpp
// 旧的调用
bool value = config->GetBool("key", false);

// 新的调用
bool value = config->GetBoolConfig("key", false);
```

### 阶段2: 高级功能集成

1. **启用热更新**
```cpp
// 在应用启动时启用热更新
config->EnableHotReload(std::chrono::milliseconds(5000));
```

2. **添加版本监控**
```cpp
// 监控配置版本变化
std::string version = config->GetCurrentVersionId();
if (version != last_version) {
    std::cout << "Config updated to version: " << version << std::endl;
    last_version = version;
}
```

3. **使用异步更新**
```cpp
// 对于耗时的配置更新，使用异步操作
auto future = config->UpdateConfigAsync("complex.key", complex_value);
// 继续其他工作...
bool success = future.get();  // 等待完成
```

### 阶段3: 性能优化

1. **批量操作**
```cpp
// 使用批量更新替代多次单个更新
std::unordered_map<std::string, ConfigValue> configs;
// ... 添加配置项
auto future = config->BatchUpdateConfigsAsync(configs);
```

2. **监控统计信息**
```cpp
// 获取性能统计
std::string stats = config->GetStatistics();
std::cout << "Config Manager Stats:\n" << stats << std::endl;
```

## ⚠️ 兼容性注意事项

### 向后兼容
- 智能配置管理器完全兼容现有的配置访问模式
- 支持所有基本数据类型（bool, int, double, string）
- 保持相同的默认值处理机制

### 破坏性变更
1. **方法名变更**：`GetBool()` → `GetBoolConfig()`
2. **返回类型**：某些高级功能返回`std::future`
3. **异常处理**：新方法具有更强的异常安全性

### 临时兼容层（可选）
```cpp
// 如果需要临时兼容，可以创建适配器
class ConfigManagerAdapter {
private:
    std::shared_ptr<SmartConfigManager> smart_config_;
    
public:
    static ConfigManagerAdapter* GetInstance() {
        static ConfigManagerAdapter instance;
        return &instance;
    }
    
    bool GetBool(const std::string& key, bool default_value) {
        return smart_config_->GetBoolConfig(key, default_value);
    }
    
    int GetInt(const std::string& key, int default_value) {
        return smart_config_->GetIntConfig(key, default_value);
    }
    
    // 其他适配方法...
};
```

## 🧪 测试验证

### 1. 编译测试
```bash
# 清理构建
make clean

# 重新编译
make -j$(nproc)

# 检查编译错误
if [ $? -eq 0 ]; then
    echo "✅ Compilation successful"
else
    echo "❌ Compilation failed"
    exit 1
fi
```

### 2. 单元测试
```bash
# 运行智能配置管理器测试
./tests/test_smart_config_manager

# 检查测试结果
if [ $? -eq 0 ]; then
    echo "✅ All tests passed"
else
    echo "❌ Some tests failed"
    exit 1
fi
```

### 3. 集成测试
```cpp
// 创建集成测试文件 test_integration.cpp
#include "utils/smart_config_manager.h"
#include <iostream>

int main() {
    try {
        auto config = sqlcc::SmartConfigManager::GetInstance();
        config->Initialize("config.ini");
        
        // 测试基本功能
        std::string db_host = config->GetStringConfig("database.host", "localhost");
        int db_port = config->GetIntConfig("database.port", 5432);
        
        std::cout << "Database: " << db_host << ":" << db_port << std::endl;
        std::cout << "✅ Integration test passed" << std::endl;
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "❌ Integration test failed: " << e.what() << std::endl;
        return 1;
    }
}
```

## 📊 性能对比

### 内存使用
- **传统实现**：存在内存泄漏风险，裸指针管理
- **智能实现**：零内存泄漏，智能指针自动管理

### 线程安全
- **传统实现**：基础互斥锁，性能瓶颈
- **智能实现**：读写锁分离，支持高并发

### 异常安全
- **传统实现**：异常时可能导致资源泄漏
- **智能实现**：RAII保证异常安全，资源自动清理

## 🔧 故障排除

### 常见问题

#### 1. 编译错误：找不到头文件
```bash
# 确保新文件已添加到构建系统
# 更新CMakeLists.txt或Makefile
echo "src/utils/config_snapshot.cpp" >> CMakeLists.txt
echo "src/utils/config_lifecycle.cpp" >> CMakeLists.txt
echo "src/utils/smart_config_manager.cpp" >> CMakeLists.txt
```

#### 2. 链接错误：未定义引用
```bash
# 确保所有目标文件都被编译和链接
make clean && make
```

#### 3. 运行时崩溃
```cpp
// 确保正确初始化
auto config = SmartConfigManager::GetInstance();
if (!config->Initialize()) {
    std::cerr << "Failed to initialize config manager" << std::endl;
    return 1;
}
```

### 调试技巧

#### 启用详细日志
```cpp
// 在代码中添加调试输出
std::cout << "Config Manager State: " << config->GetStatistics() << std::endl;
```

#### 内存泄漏检查
```bash
# 使用valgrind检查内存泄漏
valgrind --leak-check=full ./your_application
```

#### 线程安全检查
```bash
# 使用ThreadSanitizer
g++ -fsanitize=thread -g -O1 your_code.cpp -o your_app
./your_app
```

## 📈 迁移进度跟踪

### 创建迁移检查清单
```markdown
## 配置管理器迁移进度

### 已完成 ✅
- [x] 替换基础配置访问
- [x] 更新头文件引用
- [x] 通过编译测试
- [x] 通过单元测试

### 进行中 🚧
- [ ] 启用热更新功能
- [ ] 添加版本监控
- [ ] 性能基准测试

### 待完成 📋
- [ ] 文档更新
- [ ] 代码审查
- [ ] 生产环境部署
```

### 代码覆盖率检查
```bash
# 使用gcov检查代码覆盖率
gcc -fprofile-arcs -ftest-coverage your_code.cpp
gcov your_code.cpp
```

## 🎯 最佳实践

### 1. 异常处理
```cpp
try {
    auto config = SmartConfigManager::GetInstance();
    config->Initialize("config.ini");
    
    // 配置访问
    std::string value = config->GetStringConfig("key", "default");
    
} catch (const ConfigLifecycleException& e) {
    std::cerr << "Config error: " << e.what() << std::endl;
    // 优雅降级处理
} catch (const std::exception& e) {
    std::cerr << "Unexpected error: " << e.what() << std::endl;
}
```

### 2. 资源管理
```cpp
// 使用RAII模式
{
    auto config = SmartConfigManager::GetInstance();
    config->Initialize();
    
    // 在作用域内使用配置
    UseConfiguration(config);
    
} // 自动清理资源
```

### 3. 性能优化
```cpp
// 批量操作优于单个操作
std::unordered_map<std::string, ConfigValue> configs;
// ... 填充配置数据
auto future = config->BatchUpdateConfigsAsync(configs);
```

## 📚 相关文档

- [智能配置管理器设计文档](../design/config_manager/config_manager_design.md)
- [实现差距与改进计划](config_manager_improvement_plan.md)
- [v1.2.3版本TODO列表](v1.2.3_TODO.md)

## 🤝 支持

如果在迁移过程中遇到问题，请：

1. 查看本指南的故障排除部分
2. 运行测试用例验证功能
3. 检查相关文档和代码注释
4. 在项目中提交问题报告

---

**记住**：迁移是一个渐进过程，不必一次性完成所有变更。建议按照阶段逐步进行，确保每个阶段都经过充分测试后再进行下一步。