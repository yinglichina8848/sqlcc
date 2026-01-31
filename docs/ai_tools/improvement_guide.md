# SQLCC 测试规范 v1.3.9

**版本**: 1.3.9  
**创建日期**: 2026-01-30  
**适用范围**: 所有 SQLCC 项目的测试开发和维护

---

## 🎯 测试分层架构

### 分层测试体系

| 层级 | 目录 | 测试范围 | 标签 | 目标覆盖率 |
|------|------|----------|------|------------|
| **Level 1** | `tests/level1_foundation/` | 基础组件（异常、日志、配置、类型、工具） | `foundation` | 100% |
| **Level 2** | `tests/level2_core/` | 核心组件（DB管理器、执行上下文、用户管理） | `core` | 80% |
| **Level 2** | `tests/level2_storage_engine/` | 存储引擎（缓冲池、B+树、磁盘管理） | `storage` | 80% |
| **Level 3** | `tests/level3_transaction_manager/` | 事务管理、查询执行 | `transaction` | 70% |
| **Level 4** | `tests/level4_sql_processing/` | SQL处理（解析器、执行器） | `sql_processing` | 70% |
| **Level 5** | `tests/level5_network/` | 网络通信、协议处理 | `network` | 60% |
| **Level 6** | `tests/level6_integration/` | 端到端集成测试 | `integration` | 60% |
| **Level 7** | `tests/level7_integration/` | 企业级集成测试 | `enterprise` | 50% |

### 测试类型

| 类型 | 描述 | 示例 |
|------|------|------|
| **单元测试** | 测试单个函数/类 | `ValueTest`, `BufferPoolTest` |
| **集成测试** | 测试模块间交互 | `TransactionIntegrationTest` |
| **性能测试** | 测试性能指标 | `PerformanceBenchmark` |
| **边界测试** | 测试边界条件 | `BoundaryConditionTest` |
| **异常测试** | 测试异常处理 | `ExceptionHandlingTest` |
| **并发测试** | 测试多线程场景 | `ConcurrentAccessTest` |

---

## 🧪 测试文件规范

### 文件命名规范

| 类型 | 规范 | 示例 |
|------|------|------|
| **测试文件** | `被测模块_test.cpp` | `buffer_pool_test.cpp` |
| **边界测试** | `被测模块_boundary_test.cpp` | `buffer_pool_boundary_test.cpp` |
| **性能测试** | `被测模块_performance_test.cpp` | `buffer_pool_performance_test.cpp` |
| **集成测试** | `被测模块_integration_test.cpp` | `transaction_integration_test.cpp` |

### 测试文件结构

```cpp
// tests/level1_foundation/types/types_test.cpp

#include "types/value.h"  // 被测头文件
#include <gtest/gtest.h>   // 测试框架

namespace sqlcc {
namespace types {

// 使用测试夹具
class ValueTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 测试前准备
    }
    
    void TearDown() override {
        // 测试后清理
    }
    
    Value int_value_{42};
    Value string_value_{"hello"};
};

// 基本功能测试
TEST_F(ValueTest, ConstructorInt) {
    EXPECT_EQ(int_value_.GetType(), DataType::INTEGER);
    EXPECT_EQ(int_value_.GetInt(), 42);
}

// 边界条件测试
TEST_F(ValueTest, MaxIntValue) {
    Value max_val{std::numeric_limits<int32_t>::max()};
    EXPECT_EQ(max_val.GetInt(), std::numeric_limits<int32_t>::max());
}

// 异常测试
TEST_F(ValueTest, InvalidTypeCast) {
    EXPECT_THROW(int_value_.GetString(), TypeMismatchException);
}

}  // namespace types
}  // namespace sqlcc
```

### 测试夹具规范

```cpp
class ModuleTest : public ::testing::Test {
protected:
    // SetUp 在每个测试前执行
    void SetUp() override {
        // 初始化测试环境
        module_ = std::make_unique<Module>(config_);
        // 创建测试数据
        test_data_ = CreateTestData();
    }
    
    // TearDown 在每个测试后执行
    void TearDown() override {
        // 清理测试环境
        module_.reset();
        // 验证清理状态
        EXPECT_TRUE(VerifyCleanup());
    }
    
    // 测试工具函数
    void PerformAction() {
        // 执行测试操作
        module_->PerformAction();
    }
    
    // 测试数据
    std::unique_ptr<Module> module_;
    Config config_;
    TestData test_data_;
};
```

---

## 🏗️ 测试配置规范

### BUILD.bazel 配置

```bazel
cc_test(
    name = "module_test",
    srcs = ["module_test.cpp"],
    deps = [
        ":module",  # 被测模块
        "@com_google_googletest//:gtest_main",
    ],
    tags = [
        "foundation",      # 层次标签
        "unit",            # 类型标签
        "performance",     # 特性标签
    ],
    timeout = "short",     # 超时设置
    size = "small",        # 测试大小
)
```

### 测试标签体系

```bazel
tags = [
    # 层次标签 (必选其一)
    "foundation",      # Level 1: 基础组件
    "core",            # Level 2: 核心组件
    "storage",         # Level 2: 存储引擎
    "transaction",     # Level 3: 事务管理
    "sql_processing",  # Level 4: SQL处理
    "network",         # Level 5: 网络通信
    "integration",     # Level 6-7: 集成测试
    
    # 特性标签 (可选)
    "performance",     # 性能测试
    "security",        # 安全测试
    "boundary",        # 边界测试
    "concurrent",      # 并发测试
    
    # 执行标签 (可选)
    "slow",            # 慢速测试
    "flaky",           # 不稳定测试
    "manual",          # 手动执行
],
```

### 测试运行命令

```bash
# 运行特定层次测试
bazel test //tests/... --test_tag_filters=foundation
bazel test //tests/... --test_tag_filters=core

# 运行非慢速测试
bazel test //tests/... --test_tag_filters=-slow

# 组合过滤
bazel test //tests/... --test_tag_filters=core,-slow

# 运行并查看输出
bazel test //tests/... --test_output=all

# 运行单个测试
bazel test //tests/module:test --test_filter=TestName
```

---

## ✅ 测试覆盖率要求

### 覆盖率目标

| 层次 | 目标覆盖率 | 最低覆盖率 | 重要性 |
|------|-----------|-----------|--------|
| Level 1 (Foundation) | 100% | 90% | ⭐⭐⭐⭐⭐ |
| Level 2 (Core) | 80% | 70% | ⭐⭐⭐⭐ |
| Level 2 (Storage) | 80% | 70% | ⭐⭐⭐⭐ |
| Level 3 (Transaction) | 70% | 60% | ⭐⭐⭐ |
| Level 4 (SQL Processing) | 70% | 60% | ⭐⭐⭐ |
| Level 5 (Network) | 60% | 50% | ⭐⭐ |
| Level 6-7 (Integration) | 60% | 50% | ⭐ |

### 覆盖率计算规则

```cpp
// 必须覆盖的路径
- 构造函数和析构函数
- 所有 public 方法
- 所有 protected 方法
- 关键 private 方法
- 异常处理路径
- 边界条件分支

// 可接受不覆盖的路径
- 纯 getter/setter
- 简单委托方法
- 调试代码
- 废弃代码
```

---

## 🧪 测试类型规范

### 单元测试规范

```cpp
// 1. 测试命名规范
TEST_F(ModuleTest, Method_Success) {
    // 测试成功路径
}

TEST_F(ModuleTest, Method_Failure) {
    // 测试失败路径
}

TEST_F(ModuleTest, Method_Boundary) {
    // 测试边界条件
}

// 2. 断言规范
EXPECT_EQ(expected, actual);        // 期望相等
EXPECT_NE(expected, actual);        // 期望不等
EXPECT_TRUE(condition);             // 期望为真
EXPECT_FALSE(condition);            // 期望为假
EXPECT_THROW(expression, Exception); // 期望抛出异常
EXPECT_NO_THROW(expression);        // 期望不抛出异常
```

### 集成测试规范

```cpp
class IntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建完整系统
        system_ = CreateSystem();
        // 初始化数据库
        db_ = system_->GetDatabase();
        // 创建测试数据
        InsertTestData();
    }
    
    void TearDown() override {
        // 清理数据库
        CleanupDatabase();
        // 验证系统状态
        EXPECT_TRUE(system_->IsClean());
    }
    
    System* system_;
    Database* db_;
};

TEST_F(IntegrationTest, FullWorkflow) {
    // 执行完整工作流
    auto result = system_->ExecuteQuery("SELECT * FROM users");
    // 验证结果
    EXPECT_EQ(result.size(), expected_size);
    EXPECT_TRUE(ValidateResult(result));
}
```

### 性能测试规范

```cpp
class PerformanceTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建性能测试环境
        benchmark_ = CreateBenchmark();
        // 预热
        WarmUp();
    }
    
    void TearDown() override {
        // 收集性能数据
        auto stats = benchmark_->GetStats();
        // 验证性能指标
        EXPECT_LE(stats.avg_time, max_avg_time);
        EXPECT_LE(stats.max_time, max_max_time);
    }
    
    Benchmark* benchmark_;
};

TEST_F(PerformanceTest, QueryPerformance) {
    // 执行性能测试
    auto result = benchmark_->RunQuery("SELECT * FROM large_table");
    // 验证性能
    EXPECT_LE(result.time, max_time);
    EXPECT_EQ(result.rows, expected_rows);
}
```

### 边界测试规范

```cpp
TEST_F(ModuleTest, Boundary_MinValue) {
    // 测试最小值边界
    EXPECT_NO_THROW(module_->SetValue(std::numeric_limits<int>::min()));
}

TEST_F(ModuleTest, Boundary_MaxValue) {
    // 测试最大值边界
    EXPECT_NO_THROW(module_->SetValue(std::numeric_limits<int>::max()));
}

TEST_F(ModuleTest, Boundary_EmptyString) {
    // 测试空字符串
    EXPECT_NO_THROW(module_->SetString(""));
}

TEST_F(ModuleTest, Boundary_NullPointer) {
    // 测试空指针
    EXPECT_THROW(module_->SetPointer(nullptr), NullPointerException);
}
```

---

## 🔧 测试工具与辅助类

### 测试工具类

```cpp
// tests/utils/test_helpers.h

class TestHelper {
public:
    static TestData CreateTestData() {
        // 创建测试数据
        return TestData{...};
    }
    
    static void VerifyData(const TestData& data) {
        // 验证测试数据
        EXPECT_TRUE(data.IsValid());
    }
    
    static void CleanupTestData(TestData& data) {
        // 清理测试数据
        data.Clear();
    }
};

class MockDependency : public DependencyInterface {
public:
    MOCK_METHOD(void, DoAction, (), (override));
    MOCK_METHOD(int, GetValue, (), (const, override));
};
```

### 测试数据工厂

```cpp
class TestDataFactory {
public:
    static User CreateUser(int id, const std::string& name) {
        return User{id, name, /* other fields */};
    }
    
    static std::vector<User> CreateUserList(size_t count) {
        std::vector<User> users;
        for (size_t i = 0; i < count; ++i) {
            users.push_back(CreateUser(i, "User" + std::to_string(i)));
        }
        return users;
    }
    
    static Database CreateTestDatabase() {
        // 创建测试数据库
        Database db;
        db.Initialize(":memory:");
        return db;
    }
};
```

---

## 📊 测试质量指标

### 测试覆盖率指标

| 指标 | 计算方式 | 目标值 |
|------|----------|--------|
| 行覆盖率 | 覆盖的行数 / 总行数 | ≥80% |
| 分支覆盖率 | 覆盖的分支数 / 总分支数 | ≥70% |
| 函数覆盖率 | 覆盖的函数数 / 总函数数 | ≥90% |
| 语句覆盖率 | 覆盖的语句数 / 总语句数 | ≥85% |

### 测试质量指标

| 指标 | 计算方式 | 目标值 |
|------|----------|--------|
| 测试通过率 | 通过的测试数 / 总测试数 | ≥95% |
| 测试执行时间 | 所有测试执行时间 | ≤5分钟 |
| 测试稳定性 | 稳定运行的测试数 / 总测试数 | ≥90% |
| 测试维护成本 | 修改测试代码的时间 | ≤开发时间的20% |

---

## ⚠️ 测试禁忌

### 绝对禁止

- ❌ 测试中访问外部资源（网络、文件、数据库）
- ❌ 测试中产生副作用（修改全局状态）
- ❌ 测试中依赖时间（使用固定的时间点）
- ❌ 测试中依赖随机性（使用固定的种子）
- ❌ 测试中依赖环境（使用容器化环境）

### 相对禁止

- ⚠️ 过长的测试（单个测试超过100行）
- ⚠️ 过多的断言（单个测试超过10个断言）
- ⚠️ 复杂的测试（测试多个功能点）
- ⚠️ 慢速的测试（单个测试超过1秒）

---

## 📞 测试支持

### 文档

- 测试策略: `docs/testing/test_strategy.md`
- 测试规范: `docs/testing/test_standards.md`
- 测试工具: `docs/testing/test_tools.md`
- 测试报告: `docs/reports/test_reports.md`

### 项目规范

- 项目规范: `AGENTS.md`
- 编码规范: `AGENTS.md`
- 测试规范: `AGENTS.md`

---

**维护者**: SQLCC 测试开发团队  
**最后更新**: 2026-01-30  
**版本**: v1.3.9
