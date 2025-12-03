# 配置管理器单元测试

## 概述

本目录包含配置管理器（ConfigManager）的单元测试，旨在验证配置管理器的功能正确性和稳定性。

## 测试内容

### 基本功能测试

1. **单例模式测试**：验证ConfigManager::GetInstance()返回的是同一个实例
2. **配置加载测试**：验证LoadConfig方法能否正确加载配置文件
3. **环境特定配置测试**：验证LoadConfig方法能否正确加载环境特定配置
4. **配置值获取测试**：验证GetBool、GetInt、GetDouble、GetString方法
5. **配置值设置测试**：验证SetValue方法能否正确设置配置值
6. **配置键检查测试**：验证HasKey方法能否正确检查配置键是否存在

### 高级功能测试

7. **配置变更回调测试**：验证RegisterChangeCallback和UnregisterChangeCallback方法
8. **配置保存测试**：验证SaveToFile方法能否正确保存配置到文件
9. **获取所有配置键测试**：验证GetAllKeys方法能否正确返回所有配置键
10. **获取指定前缀的配置键测试**：验证GetKeysWithPrefix方法能否正确返回指定前缀的配置键
11. **配置重新加载测试**：验证ReloadConfig方法能否正确重新加载配置
12. **多线程安全性测试**：验证配置管理器在多线程环境下的安全性

## 测试框架

本测试使用Google Test框架编写，包括以下特性：

- 使用TEST_F宏编写测试用例
- 使用SetUp和TearDown方法管理测试环境
- 使用EXPECT_*和ASSERT_*宏进行断言
- 使用临时文件和目录进行测试

## 运行测试

```bash
# 构建测试
cd /home/liying/sqlcc/build
make config_manager_test

# 运行测试
./tests/unit/config_manager_test

# 运行测试并生成覆盖率报告
./tests/unit/config_manager_test --gtest_output=xml:config_manager_test_results.xml
gcov -r ../src/config_manager.cc
```

## 测试覆盖率

本测试旨在达到90%以上的代码覆盖率，包括：

- 所有公共方法的测试
- 边界条件测试
- 错误处理测试
- 多线程安全性测试

## 测试数据

测试使用临时配置文件，包含以下配置项：

```
database.page_size = 4096
database.buffer_pool_size = 1024
database.enable_logging = true
performance.max_threads = 8
performance.query_timeout = 30.5
system.log_level = INFO
```

## 注意事项

- 测试使用临时文件和目录，测试结束后会自动清理
- 多线程测试可能需要较长时间完成
- 确保测试环境有足够的权限创建临时文件和目录