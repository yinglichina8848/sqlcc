# SQLCC AI 协作开发指南 v2.0

**版本**: 2.0
**日期**: 2026-02-12
**适用范围**: 所有参与 SQLCC 项目的 AI Agent

---

## 1. 概述

### 1.1 目的
本文档为 SQLCC 项目中的 AI Agent 提供**多Agent分层协作**的完整指南，区分主规划者（Master Planner）和执行者（Worker）的职责。

### 1.2 核心原则

| 原则 | 说明 |
|------|------|
| **主从分离** | Claude Code 负责规划+审核，Agent 负责执行 |
| **规范优先** | 先 SDD 规划，后实现，再审核 |
| **质量门禁** | 编译 → 测试 → 覆盖 → 评审 → 合并 |

### 1.3 前置要求

**所有 Agent 必须首先阅读**：
1. `docs/sdd/SPEC_DRIVEN_DEVELOPMENT.md` - SDD 规范
2. `.github/ISSUE_MULTI_AGENT_COLLABORATION.md` - 多Agent协作契约（Issue #9）
3. `docs/ai_tools/AI_DEVELOPMENT_GUIDELINES.md` - AI 开发规范
4. `docs/ai_tools/CPP_DEVELOPMENT_SPECIFICATION.md` - C++ 开发规范

---

## 2. Agent 角色定义

### 2.1 Agent 能力矩阵

| Agent | 主要能力 | 最佳用途 | 避免用途 |
|-------|---------|---------|---------|
| **Claude Code** | 架构设计、代码审查、复杂推理 | SDD 规范、PR 审核、架构决策 | 简单代码生成 |
| **OpenCode** | 代码实现、快速迭代 | 已有明确规范的实现、单元测试 | 自主规划、架构设计 |
| **Codex** | 代码生成、重构、模式匹配 | 模式代码生成、重复代码重构 | 逻辑设计、审查 |
| **Gemini** | 多语言、长文本处理 | 文档生成、翻译、代码解释 | 复杂推理 |

### 2.2 角色职责

| 角色 | Agent | 职责 | 权限 |
|------|-------|------|------|
| **Master Planner** | Claude Code | SDD 规范制定、任务分解、PR 审核、质量门禁 | 全权限 |
| **Code Implementer** | OpenCode | 代码实现、单元测试、编译验证 | 代码读写 |
| **Code Generator** | Codex | 模式代码生成、重构辅助 | 代码读写 |
| **Documentation** | Gemini | 文档编写、翻译、代码解释 | 文档读写 |

### 2.3 AI 身份标注规范

**所有 GitHub 操作必须标注 AI 身份**：

```markdown
**AI Reviewer**: Claude Code (MiniMax-M2.1)
**Generated**: 2026-02-12
**Role**: Automated Code Review & Testing
```

**Agent 提交时使用**：
```bash
# Claude Code 审核评论
gh pr comment 4 --body "**AI Reviewer**: Claude Code (MiniMax-M2.1)\n**Role**: PR Review"

# OpenCode 实现提交
git commit -m "feat(core): 实现接口 (AI: OpenCode)"
```

### 2.2 身份配置（必做）

所有参与 SQLCC 的 Agent 必须配置唯一身份，用于提交记录与审计追溯。

推荐方式：
```bash
source scripts/sqlcc-agent-config.sh <agent-id>
git config user.name && git config user.email
```

手动方式：
```bash
git config user.name "SQLCC-AI(Codex-IDE)"
git config user.email "sqlcc+codex-ide@users.noreply.github.com"
```

> 说明：本仓库由李哥指挥的 Codex（AI-IDE）工作时，提交与文档记录统一使用 Codex 身份。

---

## 3. 分层工作流

### 3.1 工作流架构

```
┌─────────────────────────────────────────────────────────────────┐
│                    Multi-Agent Workflow                          │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  人类 (You)                                                     │
│     │                                                          │
│     ▼                                                          │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │  CLAUDE CODE (Master Planner)                           │   │
│  │  ─────────────────────────────────────────────────────  │   │
│  │  • SDD 规范制定                                          │   │
│  │  • 任务分解 (tasks.md)                                   │   │
│  │  • PR 审核 (Review)                                     │   │
│  │  • 质量门禁 (Compile → Test → Coverage → Review)        │   │
│  └──────────────────────────┬──────────────────────────────┘   │
│                             │                                   │
│                             │ 任务分发                          │
│                             ▼                                   │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │  执行者池 (Worker Pool)                                  │   │
│  │  ┌───────────┐  ┌───────────┐  ┌───────────┐          │   │
│  │  │ OPENCOD E  │  │   CODEX   │  │  GEMINI   │          │   │
│  │  │ 执行者     │  │ 生成者    │  │ 文档者    │          │   │
│  │  └───────────┘  └───────────┘  └───────────┘          │   │
│  └─────────────────────────────────────────────────────────┘   │
│                             │                                   │
│                             │ 提交 PR                            │
│                             ▼                                   │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │  GitHub PR                                               │   │
│  │  • OpenCode 提交 PR                                      │   │
│  │  • Claude Code 触发审核                                   │   │
│  │  • 合并决策                                               │   │
│  └─────────────────────────────────────────────────────────┘   │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### 3.2 任务生命周期

```
                    ┌─────────────────────────────────────┐
                    │           任务生命周期              │
                    └─────────────────────────────────────┘

        ┌─────────┐     ┌─────────┐     ┌─────────┐     ┌─────────┐
        │  OPEN   │────▶│ CLAIMED │────▶│  WIP    │────▶│  DONE   │
        │  开放   │     │  已认领  │     │  进行中  │     │  完成   │
        └─────────┘     └─────────┘     └─────────┘     └─────────┘
              │               │               │               │
              │               │               │               │
              ▼               ▼               ▼               ▼
        ┌─────────┐     ┌─────────┐     ┌─────────┐     ┌─────────┐
        │ BLOCKED │     │ YIELDED │     │ PAUSED  │     │ FROZEN  │
        │  阻塞   │     │  让出   │     │  暂停   │     │  冻结   │
        └─────────┘     └─────────┘     └─────────┘     └─────────┘
```

| 状态 | 英文 | 代码 | 说明 |
|------|------|------|------|
| 开放 | OPEN | 0 | 任务待认领，任何 Agent 可认领 |
| 已认领 | CLAIMED | 1 | 已被 Agent 认领，尚未开始 |
| 进行中 | WIP | 2 | 正在积极开发中 |
| 暂停 | PAUSED | 3 | 暂时停止，等待资源或依赖 |
| 让出 | YIELDED | 4 | Agent 主动释放，需重新认领 |
| 阻塞 | BLOCKED | 5 | 等待前置任务或外部依赖 |
| 完成 | DONE | 6 | 任务完成，已验证 |
| 冻结 | FROZEN | 7 | 代码冻结，待发布 |

---

## 4. 任务分配策略

### 4.1 任务类型分配

| 任务类型 | 分配 Agent | 原因 |
|---------|-----------|------|
| SDD 规范制定 | Claude Code | 需要架构设计能力 |
| PR 代码审核 | Claude Code | 需要质量判断能力 |
| 接口实现 | OpenCode | 已有明确规范 |
| 单元测试 | Codex/OpenCode | 模式化代码 |
| 文档编写 | Gemini | 长文本处理 |
| 重构任务 | Codex | 模式匹配能力强 |
| Bug 修复 | Claude Code 分析 + OpenCode 实现 | 需要推理+实现 |

### 4.2 任务队列管理

**任务队列位置**: `docs/sdd/refactoring/level2_core/tasks.md`

```markdown
## 任务看板 - [模块名称]

### TODO (待认领)
| ID | 任务 | 类型 | 优先级 | 预计时间 | 认领Agent |
|----|------|------|--------|----------|-----------|
| TASK-001 | 实现接口 X | impl | P0 | 2h | - |
| TASK-002 | 编写测试 Y | test | P1 | 1h | - |

### IN PROGRESS (进行中)
| ID | 任务 | 负责人 | 进度 | 预计完成 |
|----|------|--------|------|----------|
| TASK-003 | 实现接口 Z | OpenCode | 50% | 17:00 |

### IN REVIEW (评审中)
| ID | 任务 | 作者 | 评审人 | 状态 |
|----|------|------|--------|------|
| TASK-004 | 接口 Z | OpenCode | Claude Code | PENDING |

### DONE (已完成)
| ID | 任务 | 负责人 | 完成时间 |
|----|------|--------|----------|
| TASK-005 | 接口 W | OpenCode | 2026-02-12 |
```

---

## 5. PR 审核触发机制

### 5.1 触发流程

```
1. Agent 提交 PR
   └── GitHub webhook 触发
           │
           ▼
2. Claude Code 收到通知 (via GitHub App 或手动)
   └── 读取 PR 内容
           │
           ▼
3. 执行审核流程
   ├── 编译检查
   ├── 测试验证
   ├── 代码审查
   └── 输出 Review 结果
           │
           ▼
4. 合并决策
   ├── APPROVE → Agent 合并
   └── REQUEST_CHANGES → Agent 修复
```

### 5.2 Claude Code 审核清单

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

### 5.3 GitHub Actions 自动触发 (可选)

```yaml
# .github/workflows/ai-review-trigger.yml
name: AI Review Trigger
on:
  pull_request:
    types: [opened, synchronize, reopened]

jobs:
  notify-claude:
    runs-on: ubuntu-latest
    steps:
      - name: Trigger Claude Code Review
        run: |
          echo "PR #${PR_NUMBER} ready for review"
          # Claude Code 监听此 webhook 并自动执行审核
```

---

## 6. 消息通信协议

### 6.1 消息类型

| 消息类型 | 方向 | 说明 |
|----------|------|------|
| `TASK_CLAIM` | Agent → Claude Code | 任务认领 |
| `TASK_RELEASE` | Agent → Claude Code | 任务释放 |
| `PROGRESS_UPDATE` | Agent → Claude Code | 进度更新 |
| `BLOCKER_NOTIFICATION` | Agent → Claude Code | 阻塞通知 |
| `TASK_COMPLETE` | Agent → Claude Code | 任务完成 |
| `REVIEW_REQUEST` | Agent → Claude Code | 评审请求 |
| `REVIEW_RESULT` | Claude Code → Agent | 审核结果 |

### 6.2 沟通频率

| 事件 | 触发条件 | 响应时间 |
|------|----------|----------|
| 任务认领 | 认领时 | 即时 |
| 进度更新 | 每 30 分钟或里程碑达成 | 5 分钟内 |
| 阻塞通知 | 发现阻塞时 | 即时 |
| 任务完成 | 完成所有验收标准 | 即时 |
| 协助请求 | 需要帮助时 | 15 分钟内 |

---

## 7. 验收标准

### 7.1 任务验收清单

| 检查项 | 执行者 | 状态 |
|--------|-------|------|
| 编译通过 (bazel build) | OpenCode | ☐ |
| 测试通过 (bazel test) | OpenCode | ☐ |
| 覆盖率达标 | Claude Code | ☐ |
| 代码评审通过 | Claude Code | ☐ |
| 文档完整 | Gemini | ☐ |
| CHANGELOG 已更新 | OpenCode | ☐ |

### 7.2 覆盖率要求

| Level | 函数覆盖率 | 行覆盖率 | 分支覆盖率 |
|-------|-----------|----------|-----------|
| Level 1 | >= 95% | >= 80% | >= 70% |
| Level 2 | >= 85% | >= 70% | >= 60% |
| Level 3+ | >= 75% | >= 60% | >= 50% |

---

## 8. 常用命令

### 8.1 开发命令

```bash
# 构建所有目标
bazel build //...

# 构建特定模块
bazel build //src/core:core

# 运行所有测试
bazel test //... --test_output=errors

# 运行特定测试
bazel test //tests/level2_core:all

# 生成覆盖率
bazel coverage //tests/level2_core:all
```

### 8.2 协作命令

```bash
# 检查任务状态
./scripts/check_task_status.sh tasks.md

# 同步进度
./scripts/sync_progress.sh tasks.md
```

---

## 9. 相关文档

| 文档 | 路径 | 说明 |
|------|------|------|
| SDD 规范 | `docs/sdd/SPEC_DRIVEN_DEVELOPMENT.md` | 规范驱动开发指南 |
| AI 开发规范 | `docs/ai_tools/AI_DEVELOPMENT_GUIDELINES.md` | AI Agent 开发指南 |
| C++ 开发规范 | `docs/ai_tools/CPP_DEVELOPMENT_SPECIFICATION.md` | C++ 编码规范 |
| 任务队列 | `docs/sdd/refactoring/level2_core/tasks.md` | 任务跟踪 |

---

## 10. 变更历史

| 版本 | 日期 | 变更内容 | 变更人 |
|------|------|---------|--------|
| 2.0 | 2026-02-12 | 新增多Agent分层协作设计 | Claude Code |
| 1.0 | 2026-02-02 | 初始版本 | SQLCC AI |

---

**维护者**: Claude Code (Master Planner)
**最后更新**: 2026-02-12
**版本**: v2.0
