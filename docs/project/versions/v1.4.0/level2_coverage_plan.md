# Level 2 覆盖率改进计划 v1.4.0

**创建时间**: 2026-02-04
**负责人**: 高小原 (agent-master-1)
**状态**: 待开始（v1.4.0）

---

## 1. 背景

当前 Level 2 测试覆盖率较低，需要补充测试用例以达到 70% 目标覆盖率。

**基线数据来源（必须补齐）**:
- 覆盖率报告: [填写最新覆盖率报告路径]
- 统计时间: [YYYY-MM-DD]
- 统计口径: [函数/行/分支覆盖率]

## 2. 模块清单与现状

### Level 2 Core Services

| 模块 | 测试文件数 | 当前覆盖率(基线) | 优先级 | 待补充 |
|------|-----------|------------------|--------|--------|
| sql_parser | 40个 | [待填] | ✅ 已完善 | 无 |
| storage_engine | 6个 | [待填] | 中等 | 操作测试 |
| b_plus_tree | 6个 | [待填] | 中等 | B+树操作 |
| buffer_pool | 4个 | [待填] | 中等 | 缓冲区操作 |
| **index_manager** | **2个** | [待填] | ⭐ 高 | 索引CRUD |
| **config_manager** | **2个** | [待填] | ⭐ 高 | 配置操作 |
| wal_system | 1个 | [待填] | 🔴 高 | WAL操作 |
| wal | 1个 | [待填] | 🔴 高 | 日志操作 |
| user_manager | 1个 | [待填] | 🔴 中等 | 用户管理 |
| system_database | 1个 | [待填] | 🔴 中等 | 系统数据库 |
| schema_manager | 1个 | [待填] | 🔴 中等 | 模式管理 |
| permission_validator | 1个 | [待填] | 🔴 中等 | 权限验证 |
| index | 1个 | [待填] | 🔴 低 | 索引基础 |
| execution_result | 1个 | [待填] | 🔴 低 | 执行结果 |
| execution_context | 1个 | [待填] | 🔴 低 | 上下文 |
| disk_manager | 1个 | [待填] | 🔴 低 | 磁盘管理 |
| disk_management | 1个 | [待填] | 🔴 低 | 磁盘管理 |
| database_manager | 1个 | [待填] | 🔴 低 | 数据库管理 |

### Level 2 Storage Engine

| 模块 | 测试文件数 | 当前覆盖率(基线) | 优先级 |
|------|-----------|------------------|--------|
| b_plus_tree | 6个 | [待填] | 中等 |
| buffer_pool | 4个 | [待填] | 中等 |
| storage_engine | 6个 | [待填] | 中等 |
| index_manager | 2个 | [待填] | ⭐ 高 |
| wal_system | 1个 | [待填] | 🔴 高 |
| wal | 1个 | [待填] | 🔴 高 |
| index | 1个 | [待填] | 🔴 低 |
| disk_manager | 1个 | [待填] | 🔴 低 |
| disk_management | 1个 | [待填] | 🔴 低 |

## 3. 任务分配

### 任务池

#### 🔵 简单任务 (1-2小时/个)

| ID | 模块 | 任务 | 预计时间 | 状态 | 执行者 | 验收标准 |
|----|------|------|---------|------|--------|----------|
| T-L2-001 | config_manager | 补充边界值测试 | 1h | OPEN | 待分配 | 覆盖率提升记录 + 新增测试通过 |
| T-L2-002 | index_manager | 补充索引CRUD测试 | 2h | OPEN | 待分配 | CRUD 用例覆盖 + 新增测试通过 |
| T-L2-003 | execution_context | 补充上下文测试 | 1h | OPEN | 待分配 | 关键路径覆盖 + 新增测试通过 |

#### 🟡 中等任务 (半天/个)

| ID | 模块 | 任务 | 预计时间 | 状态 | 执行者 | 验收标准 |
|----|------|------|---------|------|--------|----------|
| T-L2-004 | b_plus_tree | 补充B+树操作测试 | 4h | OPEN | 待分配 | 插入/删除/范围查询覆盖 + 测试通过 |
| T-L2-005 | storage_engine | 补充存储引擎测试 | 4h | OPEN | 待分配 | 读写/恢复路径覆盖 + 测试通过 |

#### 🔴 复杂任务 (1天/个)

| ID | 模块 | 任务 | 预计时间 | 状态 | 执行者 | 验收标准 |
|----|------|------|---------|------|--------|----------|
| T-L2-006 | wal | 补充WAL日志测试 | 6h | OPEN | 待分配 | WAL异常/恢复覆盖 + 测试通过 |
| T-L2-007 | permission_validator | 补充权限验证测试 | 6h | OPEN | 待分配 | 权限边界/拒绝路径覆盖 + 测试通过 |

## 4. 执行计划

### 第一阶段：简单任务 (第1天)

1. **config_manager** - 边界值测试
2. **index_manager** - 索引CRUD测试

### 第二阶段：中等任务 (第2-3天)

1. **b_plus_tree** - B+树操作测试
2. **storage_engine** - 存储引擎测试

### 第三阶段：复杂任务 (第4-5天)

1. **wal** - WAL日志测试
2. **permission_validator** - 权限验证测试

## 5. 验收标准

- [ ] 所有新增测试通过 `bazel test //tests/...`
- [ ] 覆盖率提升至 70%（以明确的基线报告为准）
- [ ] 代码评审通过
- [ ] 文档更新完成（tasks.md + verification.md）

## 6. 进度跟踪

| 日期 | 完成任务 | 覆盖率 | 状态 |
|------|---------|--------|------|
| 2026-02-03 | - | <10% | 📋 规划中 |
| 2026-02-04 | - | - | ⏳ 待开始 |
| 2026-02-05 | - | - | ⏳ 待开始 |

## 7. 相关文档

- [Level 1 覆盖率报告](../v1.3.9/coverage_report.md)
- [测试规范](../../ai_tools/improvement_guide.md)
- [BUILD 文件规范](../../ai_tools/BUILD_FILE_SPECIFICATION.md)
- [C++ 开发规范](../../ai_tools/CPP_DEVELOPMENT_SPECIFICATION.md)

---

## 附录：测试模板

### 测试用例模板

```cpp
#include <gtest/gtest.h>
#include <memory>

namespace sqlcc {
namespace test {

TEST(ModuleNameTest, FeatureName) {
    // Arrange
    auto obj = std::make_unique<ModuleClass>();

    // Act
    auto result = obj->operation();

    // Assert
    EXPECT_TRUE(result.isOk());
}

}  // namespace test
}  // namespace sqlcc
```

### BUILD.bazel 模板

```python
cc_test(
    name = "module_name_tests",
    srcs = glob(["*.cpp"]),
    deps = [
        "@com_google_googletest//:gtest_main",
        "//src/module:module",
    ],
    tags = ["coverage", "core", "module_name"],
)
```

---

**维护者**: 高小原 (agent-master-1)
**版本**: v1.0
