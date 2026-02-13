# S05 Bazel 分层门禁策略

## S05A 执行策略错误模型规范 (COMPLETED)

**状态**: COMPLETED
**负责人**: Gemini
**文档**: `execution_strategy_error_model_spec.md`

### 交付物
- 错误模型规范文档 (218 行)
- 编译修复 (4 个文件)

---

## S05B Bazel 分层门禁检查清单

**状态**: WIP
**负责人**: OpenCode
**目标**: 定义分层门禁策略，确保代码质量

### 分层门禁策略

| 层级 | 目标 | 执行命令 |
|------|------|---------|
| **Gate-L1** | 核心编译 | `bazel build //src/execution:execution` |
| **Gate-L2** | 增量构建 | `bazel build //src/core:core` |
| **Gate-L3** | 单元测试 | `bazel test //tests/level1_foundation/...` |
| **Gate-L4** | 集成测试 | `bazel test //tests/level2_core/...` |

### 检查清单

#### L1 门禁 (必须通过)
- [ ] `bazel build //src/execution:execution` 通过
- [ ] `bazel build //src/sql_parser:sql_parser` 通过
- [ ] 无编译错误

#### L2 门禁 (必须通过)
- [ ] `bazel build //src/core:core` 通过
- [ ] 头文件依赖正确
- [ ] 无链接错误

#### L3 门禁 (必须通过)
- [ ] `bazel test //tests/level1_foundation/config:config_test` 通过
- [ ] `bazel test //tests/level1_foundation/logger:logger_test` 通过
- [ ] `bazel test //tests/level1_foundation/utils:utils_test` 通过

### 验收标准

| 标准 | 描述 | 状态 |
|------|------|------|
| **编译通过** | L1 门禁全部通过 | 待验证 |
| **测试通过** | L3 门禁全部通过 | 待验证 |
| **无回归** | 无破坏性变更 | 待验证 |

### 回滚策略

```bash
# 回滚变更
git revert <commit_hash>
```

---

## S05C 深层架构修复

**状态**: IN_PROGRESS
**负责人**: Gemini + OpenCode

### 问题清单
- [ ] `stmt.join_clause` 成员缺失
- [ ] `stmt.group_by_clause` 成员缺失
- [ ] `DatabaseManager.ExecuteJoinQuery` 方法缺失

### 变更文件
1. `src/execution/dml_execution_strategy.cpp`
2. `src/core/core_database_manager.h`

---

## 验证命令

```bash
# Gate-L1
bazel build //src/execution:execution

# Gate-L2 (增量)
bazel build //src/core:core

# Gate-L3
bazel test //tests/level1_foundation/...
```

---

*Generated: 2026-02-13*
*Version: 1.0*
