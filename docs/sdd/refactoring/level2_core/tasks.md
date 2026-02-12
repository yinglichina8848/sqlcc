# Level 2 Core 重构 - 任务清单

**版本**: 2.0
**日期**: 2026-02-12
**状态**: 待开始
**分支**: feature/level2-coverage-improvement

---

## 1. Agent 角色定义

### 1.1 Agent 能力矩阵

| Agent | 主要能力 | 最佳用途 | 避免用途 |
|-------|---------|---------|---------|
| **Claude Code** | 架构设计、代码审查、复杂推理 | SDD 规范、PR 审核、架构决策 | 简单代码生成 |
| **OpenCode** | 代码实现、快速迭代 | 已有明确规范的实现、单元测试 | 自主规划、架构设计 |
| **Codex** | 代码生成、重构、模式匹配 | 模式代码生成、重复代码重构 | 逻辑设计、审查 |
| **Gemini** | 多语言、长文本处理 | 文档生成、翻译、代码解释 | 复杂推理 |

### 1.2 Agent 配置

```yaml
agents:
  - id: claude-code-master
    role: Master Planner
    model: MiniMax-M2.1
    skills: ["sdd", "architecture", "code_review", "planning"]
    max_tasks: 1
    availability: available

  - id: opencode-dev-1
    role: Code Implementer
    skills: ["cpp", "bazel", "interface", "testing"]
    max_tasks: 3
    availability: available

  - id: codex-gen-1
    role: Code Generator
    skills: ["pattern_generation", "refactoring", "cpp"]
    max_tasks: 5
    availability: available

  - id: gemini-docs-1
    role: Documentation
    skills: ["markdown", "docs", "translation"]
    max_tasks: 4
    availability: available
```

### 1.3 AI 身份标注

**GitHub 操作必须标注 AI 身份**：

```markdown
**AI Reviewer**: Claude Code (MiniMax-M2.1)
**Generated**: 2026-02-12
**Role**: Automated Code Review & Testing
```

---

## 2. 任务分配策略

### 2.1 任务类型分配

| 任务类型 | 分配 Agent | 原因 |
|---------|-----------|------|
| SDD 规范制定 | Claude Code | 需要架构设计能力 |
| PR 代码审核 | Claude Code | 需要质量判断能力 |
| 接口实现 | OpenCode | 已有明确规范 |
| 单元测试 | Codex/OpenCode | 模式化代码 |
| 文档编写 | Gemini | 长文本处理 |
| 重构任务 | Codex | 模式匹配能力强 |
| Bug 修复 | Claude Code 分析 + OpenCode 实现 | 需要推理+实现 |

### 2.2 任务状态统计

| 状态 | 计数 | 说明 |
|------|------|------|
| OPEN | 38 | 开放待认领 |
| CLAIMED | 0 | 已认领 |
| WIP | 0 | 进行中 |
| DONE | 0 | 已完成 |
| BLOCKED | 0 | 阻塞 |
| FROZEN | 0 | 冻结 |

---

## 3. 任务看板

### TODO (待认领) - 38 任务

| ID | 任务 | 类型 | 优先级 | 预计时间 | 认领Agent |
|----|------|------|--------|----------|-----------|
| TASK-CORE-001 | 创建接口目录结构 | impl | P0 | 1h | - |
| TASK-CORE-002 | IExecutionContext 接口 | impl | P0 | 4h | - |
| TASK-CORE-003 | IUserContext 接口 | impl | P0 | 2h | - |
| TASK-CORE-004 | IPermissionContext 接口 | impl | P0 | 2h | - |
| TASK-CORE-005 | ITransactionContext 接口 | impl | P0 | 2h | - |
| TASK-CORE-006 | ContextFactory 工厂 | impl | P1 | 2h | - |
| TASK-CORE-007 | DependencyInjection 容器 | impl | P1 | 2h | - |
| TASK-CORE-008 | 接口 BUILD.bazel | build | P1 | 1h | - |
| TASK-CORE-009 | 创建实现目录结构 | impl | P0 | 1h | - |
| TASK-CORE-010 | ExecutionContextImpl | impl | P0 | 8h | - |
| TASK-CORE-011 | UserContextImpl | impl | P1 | 4h | - |
| TASK-CORE-012 | PermissionContextImpl | impl | P1 | 4h | - |
| TASK-CORE-013 | TransactionContextImpl | impl | P1 | 6h | - |
| TASK-CORE-014 | ContextFactory 实现 | impl | P1 | 2h | - |
| TASK-CORE-015 | 实现 BUILD.bazel | build | P1 | 1h | - |
| TASK-CORE-016 | DDL Strategy 适配 | refactor | P1 | 4h | - |
| TASK-CORE-017 | DML Strategy 适配 | refactor | P1 | 4h | - |
| TASK-CORE-018 | DCL Strategy 适配 | refactor | P1 | 4h | - |
| TASK-CORE-019 | Utility Strategy 适配 | refactor | P1 | 4h | - |
| TASK-CORE-020 | UnifiedExecutor 适配 | refactor | P0 | 6h | - |
| TASK-CORE-021 | 执行模块 BUILD.bazel | build | P1 | 2h | - |
| TASK-CORE-022a | MockIExecutionContext | test | P1 | 1h | - |
| TASK-CORE-022b | MockIUserContext | test | P1 | 1h | - |
| TASK-CORE-022c | MockIPermissionContext | test | P1 | 1h | - |
| TASK-CORE-022d | MockITransactionContext | test | P1 | 1h | - |
| TASK-CORE-023 | ExecutionContextImpl 测试 | test | P1 | 4h | - |
| TASK-CORE-024 | UserContextImpl 测试 | test | P2 | 2h | - |
| TASK-CORE-025 | PermissionContextImpl 测试 | test | P2 | 2h | - |
| TASK-CORE-026 | TransactionContextImpl 测试 | test | P2 | 2h | - |
| TASK-CORE-027 | UnifiedExecutor 集成测试 | test | P0 | 6h | - |
| TASK-CORE-028 | 核心接口编译验证 | verify | P1 | 1h | - |
| TASK-CORE-029 | 接口实现编译验证 | verify | P1 | 1h | - |
| TASK-CORE-030 | 执行模块编译验证 | verify | P1 | 1h | - |
| TASK-CORE-031 | 运行所有测试 | test | P0 | 2h | - |
| TASK-CORE-032 | 生成覆盖率报告 | docs | P1 | 1h | - |

### IN PROGRESS (进行中) - 0 任务

| ID | 任务 | 负责人 | 进度 | 预计完成 | 检查点 |
|----|------|--------|------|----------|--------|
| - | - | - | - | - | - |

### IN REVIEW (评审中) - 0 任务

| ID | 任务 | 作者 | 评审人 | 状态 |
|----|------|------|--------|------|
| - | - | - | - | - |

### DONE (已完成) - 0 任务

| ID | 任务 | 负责人 | 完成时间 | 评审人 |
|----|------|--------|----------|--------|
| - | - | - | - | - |

---

## 4. PR 审核流程

### 4.1 审核清单

```markdown
## PR 审核清单

### 1. 编译检查
- [ ] bazel build //src/... 成功
- [ ] 无编译警告

### 2. 测试验证
- [ ] bazel test //tests/... 成功
- [ ] 测试覆盖率达标

### 3. 代码审查
- [ ] SOLID 原则
- [ ] 命名规范
- [ ] 注释完整
- [ ] 依赖关系合理

### 4. 文档检查
- [ ] CHANGELOG 更新
- [ ] 必要文档更新

### 审核结果
- [ ] APPROVE
- [ ] REQUEST_CHANGES
- [ ] COMMENTS
```

### 4.2 质量门禁

```
编译 → 测试 → 覆盖 → 评审 → 合并
  ↓      ↓      ↓      ↓
 PASS   PASS   >=85%  APPROVE
```

---

## 5. 变更历史

| 版本 | 日期 | 变更内容 | 变更人 |
|------|------|---------|--------|
| 2.0 | 2026-02-12 | 新增多Agent分层协作 | Claude Code |
| 1.2 | 2026-02-02 | 初始任务列表 | SQLCC AI |

---

**维护者**: Claude Code (Master Planner)
**最后更新**: 2026-02-12
**版本**: v2.0
