# SQLCC 多Agent跨平台协作开发指南

**版本**: 1.0
**日期**: 2026-02-02
**适用范围**: 所有参与SQLCC项目的AI Agent和人类开发者

---

## 目录

1. [概述](#1-概述)
2. [Agent身份体系](#2-agent身份体系)
3. [协作流程](#3-协作流程)
4. [提交规范](#4-提交规范)
5. [PR规范](#5-pr规范)
6. [分支策略](#6-分支策略)
7. [质量门禁](#7-质量门禁)
8. [快速开始](#8-快速开始)
9. [常见问题](#9-常见问题)
10. [相关文档](#10-相关文档)

---

## 1. 概述

### 1.1 目标

建立一套完整的**多Agent跨平台协作开发规范**，确保：

- ✅ 多个AI Agent（OpenCode、iFlow、Claude、Gemini、OpenClaw等）有序协作
- ✅ 代码提交可追溯、可审计
- ✅ PR流程规范化
- ✅ 项目质量可控

### 1.2 为什么需要协作规范

SQLCC项目采用多个AI Agent进行开发，但缺乏统一的协作规范会导致：

| 问题 | 影响 |
|------|------|
| Agent身份不明确 | 难以追溯职责 |
| 提交风格不统一 | 代码历史混乱 |
| PR质量参差不齐 | 代码质量下降 |
| 缺乏协作机制 | 效率低下 |

### 1.3 支持的Agent平台

| 平台 | 状态 | 说明 |
|------|------|------|
| **OpenCode** | ✅ 已配置 | 当前主要开发平台 |
| **iFlow** | ⏳ 待配置 | 需要单独配置 |
| **Claude** | ⏳ 待配置 | Anthropic Claude |
| **Claude Code** | ⏳ 待配置 | CLI执行工具 |
| **Gemini** | ⏳ 待配置 | Google Gemini |
| **OpenClaw** | ⏳ 待配置 | 多实例协作框架 |

### 1.4 核心原则

```
🤖 AI Agent = 开发者
👤 人类 = 项目负责人/最终审批者

协作 = 分工明确 + 沟通有序 + 质量可控
```

---

## 2. Agent身份体系

### 2.1 身份配置

所有参与SQLCC项目的AI Agent必须在工作目录中配置**唯一身份**。

**配置文件**: `~/.gitconfig.d/agents.conf`

### 2.2 Agent列表

#### OpenCode（当前主要平台）

| Agent ID | 名称 | 职责 | 邮箱 |
|----------|------|------|------|
| `opencode-master` | SQLCC-AI(OpenCode Master) | 项目总协调 | sqlcc+opencode-master@users.noreply.github.com |
| `opencode-developer` | SQLCC-AI(OpenCode Developer) | 代码实现 | sqlcc+opencode-dev@users.noreply.github.com |
| `opencode-tester` | SQLCC-AI(OpenCode Tester) | 测试执行 | sqlcc+opencode-test@users.noreply.github.com |
| `opencode-doc` | SQLCC-AI(OpenCode Doc) | 文档编写 | sqlcc+opencode-doc@users.noreply.github.com |
| `opencode-reviewer` | SQLCC-AI(OpenCode Reviewer) | 代码评审 | sqlcc+opencode-review@users.noreply.github.com |

#### iFlow

| Agent ID | 名称 | 职责 | 邮箱 |
|----------|------|------|------|
| `iflow-builder` | SQLCC-AI(iFlow Builder) | 构建系统 | sqlcc+iflow-builder@users.noreply.github.com |
| `iflow-refactor` | SQLCC-AI(iFlow Refactor) | 代码重构 | sqlcc+iflow-refactor@users.noreply.github.com |
| `iflow-test` | SQLCC-AI(iFlow Test) | 测试维护 | sqlcc+iflow-test@users.noreply.github.com |

#### Claude（Anthropic）

| Agent ID | 名称 | 职责 | 邮箱 |
|----------|------|------|------|
| `claude-architect` | SQLCC-AI(Claude Architect) | 架构设计 | sqlcc+claude-arch@users.noreply.github.com |
| `claude-developer` | SQLCC-AI(Claude Developer) | 代码编写 | sqlcc+claude-dev@users.noreply.github.com |
| `claude-reviewer` | SQLCC-AI(Claude Reviewer) | 代码审查 | sqlcc+claude-review@users.noreply.github.com |
| `claude-researcher` | SQLCC-AI(Claude Researcher) | 技术调研 | sqlcc+claude-research@users.noreply.github.com |

#### Claude Code

| Agent ID | 名称 | 职责 | 邮箱 |
|----------|------|------|------|
| `claudecode-executor` | SQLCC-AI(ClaudeCode Executor) | CLI执行 | sqlcc+claude-code-exec@users.noreply.github.com |
| `claudecode-toolmaker` | SQLCC-AI(ClaudeCode Toolmaker) | 工具开发 | sqlcc+claude-code-tools@users.noreply.github.com |

#### Gemini（Google）

| Agent ID | 名称 | 职责 | 邮箱 |
|----------|------|------|------|
| `gemini-assistant` | SQLCC-AI(Gemini Assistant) | 通用辅助 | sqlcc+gemini-assist@users.noreply.github.com |
| `gemini-analyst` | SQLCC-AI(Gemini Analyst) | 代码分析 | sqlcc+gemini-analyst@users.noreply.github.com |
| `gemini-debugger` | SQLCC-AI(Gemini Debugger) | 错误调试 | sqlcc+gemini-debug@users.noreply.github.com |

#### OpenClaw

| Agent ID | 名称 | 职责 | 邮箱 |
|----------|------|------|------|
| `openclaw-refactor` | SQLCC-AI(OpenClaw Refactor) | 代码重构 | sqlcc+openclaw-refactor@users.noreply.github.com |
| `openclaw-test-fixer` | SQLCC-AI(OpenClaw Test Fixer) | 测试修复 | sqlcc+openclaw-test-fixer@users.noreply.github.com |
| `openclaw-build-cleaner` | SQLCC-AI(OpenClaw Build Cleaner) | 构建修复 | sqlcc+openclaw-build-cleaner@users.noreply.github.com |
| `openclaw-doc` | SQLCC-AI(OpenClaw Doc) | 文档更新 | sqlcc+openclaw-doc@users.noreply.github.com |
| `openclaw-sdd` | SQLCC-AI(OpenClaw SDD) | 规范制定 | sqlcc+openclaw-sdd@users.noreply.github.com |

#### Codex（AI-IDE）

| Agent ID | 名称 | 职责 | 邮箱 |
|----------|------|------|------|
| `codex-ide` | SQLCC-AI(Codex-IDE) | 文档与协作规范维护 | sqlcc+codex-ide@users.noreply.github.com |

### 2.3 配置方法

#### 方法1: 使用配置脚本（推荐）

```bash
# 进入SQLCC项目目录
cd /path/to/sqlcc

# 加载配置脚本
source scripts/sqlcc-agent-config.sh <agent-id>

# 示例：配置为OpenCode Developer
source scripts/sqlcc-agent-config.sh opencode-developer

# 验证配置
git config user.name && git config user.email
```

#### 方法2: 手动配置

```bash
# 进入SQLCC项目目录
cd /path/to/sqlcc

# 设置名称和邮箱
git config user.name "SQLCC-AI(OpenCode-Developer)"
git config user.email "sqlcc+opencode-dev@users.noreply.github.com"

# 验证配置
git config user.name && git config user.email
```

#### 方法3: 使用includeIf自动加载

在 `~/.gitconfig` 中添加：

```ini
[includeIf "gitdir:~/workspace/sqlcc/"]
    path = ~/.gitconfig.d/agents.conf
```

然后在不同Agent工作目录中切换即可自动匹配身份。

### 2.4 验证身份

```bash
# 检查当前Git身份
git config user.name
git config user.email

# 应该输出类似：
# SQLCC-AI(OpenCode-Developer)
# sqlcc+opencode-dev@users.noreply.github.com
```

---

## 3. 协作流程

### 3.1 任务状态机

```
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
| 开放 | OPEN | 0 | 任务待认领，任何Agent可认领 |
| 已认领 | CLAIMED | 1 | 已被Agent认领，尚未开始 |
| 进行中 | WIP | 2 | 正在积极开发中 |
| 暂停 | PAUSED | 3 | 暂时停止，等待资源或依赖 |
| 让出 | YIELDED | 4 | Agent主动释放，需重新认领 |
| 阻塞 | BLOCKED | 5 | 等待前置任务或外部依赖 |
| 完成 | DONE | 6 | 任务完成，已验证 |
| 冻结 | FROZEN | 7 | 代码冻结，待发布 |

### 3.2 消息协议

消息类型与字段定义以 `docs/ai_tools/AI_COLLABORATION_GUIDE.md` 为准（该文档为协议主版本）。本指南仅列出常用消息：

| 消息类型 | 发送者 | 接收者 | 时机 | 内容 |
|----------|--------|--------|------|------|
| `TASK_CLAIM` | Developer | Master | 认领任务时 | 任务ID、Agent ID、预计完成时间 |
| `PROGRESS_UPDATE` | Developer | Master | 每30分钟 | 进度百分比、已完成工作、遇到的困难 |
| `BLOCKER_NOTIFICATION` | Developer | Master | 遇到阻塞时 | 阻塞原因、影响范围、需要的支持 |
| `TASK_COMPLETE` | Developer | Master | 完成任务时 | 任务ID、验证结果、提交PR链接 |

补充消息（同样需要遵守）：`TASK_RELEASE`、`ASSISTANCE_REQUEST`、`REVIEW_REQUEST`、`HEARTBEAT`。

### 3.3 多Agent协作架构

```
┌─────────────────────────────────────────────────────────────────┐
│                      协作架构                                    │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│   用户/项目负责人                                                │
│        │                                                        │
│        ▼                                                        │
│   ┌─────────────────────────────────────────────────────────┐  │
│   │                    Master Agent                          │  │
│   │  - 任务分解                                               │  │
│   │  - 进度汇总                                               │  │
│   │  - 资源协调                                               │  │
│   │  - 最终交付                                               │  │
│   └─────────────────────────────────────────────────────────┘  │
│        │           │           │           │                   │
│        ▼           ▼           ▼           ▼                   │
│   ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────┐             │
│   │Developer│ │ Developer│ │ Tester  │ │ Reviewer│             │
│   │ Agent 1 │ │ Agent 2 │ │ Agent   │ │ Agent   │             │
│   └─────────┘ └─────────┘ └─────────┘ └─────────┘             │
│        │           │           │           │                   │
│        └───────────┴─────┬─────┴───────────┘                   │
│                          │                                     │
│                          ▼                                     │
│               ┌─────────────────────┐                          │
│               │   代码仓库 (Git)    │                          │
│               │   - GitHub          │                          │
│               │   - Gitee           │                          │
│               └─────────────────────┘                          │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### 3.4 协作流程示例

```markdown
## 任务：修复 execution 模块编译错误

### 步骤1：Master分解任务
- TASK-EXEC-001: 分析编译错误
- TASK-EXEC-002: 修复头文件引用
- TASK-EXEC-003: 验证构建
- TASK-EXEC-004: 提交PR

### 步骤2：Agent认领
Developer Agent 发送:
```
TASK_CLAIM
Agent: OpenCode-Developer
Task: TASK-EXEC-001, TASK-EXEC-002
ETA: 2小时
```

### 步骤3：开发过程中
每30分钟发送:
```
PROGRESS_UPDATE
Progress: 50%
Completed: 分析完成，发现3处头文件错误
Blocker: 无
```

### 步骤4：完成任务
```
TASK_COMPLETE
Task: TASK-EXEC-002
Result: 编译通过
PR: https://github.com/.../pull/123
```

### 步骤5：验证和合并
Tester Agent → Reviewer Agent → Master合并PR
```

---

## 4. 提交规范

### 4.1 Commit Message格式

```
<类型>: <描述>

Agent: <Agent名称>
Scope: <影响范围>
Refs: #<issue编号>
```

### 4.2 类型

| 类型 | 说明 | 示例 |
|------|------|------|
| `feat` | 新功能 | `feat: 添加LOAD_DATA枚举` |
| `fix` | Bug修复 | `fix: 修复编译错误` |
| `docs` | 文档更新 | `docs: 更新README` |
| `refactor` | 重构（无行为改变） | `refactor: 重构头文件引用` |
| `test` | 测试相关 | `test: 添加单元测试` |
| `chore` | 构建/工具/配置 | `chore: 更新BUILD文件` |
| `style` | 代码格式 | `style: 格式化代码` |
| `perf` | 性能优化 | `perf: 优化查询性能` |

### 4.3 示例

#### 合格提交 ✅

```
feat: 修复 execution 模块编译错误

Agent: OpenCode-Developer
Scope: src/execution/*
Refs: #EXEC-101

- 修复 join_executor.cpp API 调用不匹配
- 修复 dml_execution_strategy.cpp 头文件引用
- 添加 LOAD_DATA 枚举值

No unrelated changes.
```

#### 不合格提交 ❌

```
fix: 修复问题
（缺少 Agent, Scope, Refs）
```

### 4.4 Commit Hook

项目已部署 `.git/hooks/commit-msg` 钩子，自动检查：

✅ **必须包含**:
- `Agent:` - Agent身份标识
- `Scope:` - 影响范围
- `Refs:` - Issue关联

❌ **禁止**:
- 直接提交到 main / release/*
- vibe commit（无描述）
- 无文件更改的提交

---

## 5. PR规范

### 5.1 PR模板

项目使用 `.github/pull_request_template.md`，强制包含：

#### 必须填写的字段

| 字段 | 说明 |
|------|------|
| **Objective** | 目标类型（Build failure / Test failure / Refactor / New feature） |
| **Agent** | 负责人 |
| **Scope** | 影响范围 |
| **Design Notes** | 设计说明 |
| **Test Plan** | 测试计划 |
| **AI Self-Check** | 5项检查 |

### 5.2 AI Self-Check

提交PR前必须确认：

- [ ] **No unrelated files modified** - 无无关文件修改
- [ ] **No formatting-only changes** - 无仅格式化修改
- [ ] **No TODO / FIXME left** - 无遗留TODO
- [ ] **No speculative refactor** - 无推测性重构
- [ ] **Commit message follows规范** - 提交信息规范

### 5.3 PR模板示例

```markdown
# PR Summary

## Objective
- [ ] Build failure fix
- [ ] Test failure fix
- [ ] Refactor (no behavior change)
- [ ] New feature (discouraged unless approved)

## Agent
Agent: OpenCode-Developer

## Scope of Change
- Affected modules:
- [ ] src/execution/*
- [ ] src/core/*

## Design Notes
- Invariants preserved: None
- Interfaces unchanged: Yes
- Known risks: None

## Test Plan
- [ ] Full build passes
- [ ] Existing tests unchanged

## AI Self-Check
- [x] No unrelated files modified
- [x] No formatting-only changes
- [x] No TODO / FIXME left
- [x] No speculative refactor
- [x] Commit message follows规范
```

### 5.4 PR审批流程

```
提交PR
    │
    ▼
自动检查 → CI构建 → 测试运行
    │           │
    ▼           ▼
失败 → 修复    失败 → 修复
    │           │
    └─────┬─────┘
          ▼
     代码评审
          │
          ▼
     合并到 main
```

---

## 6. 分支策略

### 6.1 分支类型

| 分支类型 | 命名规则 | 保护状态 | 说明 |
|----------|----------|----------|------|
| **主分支** | `main` | ❌ 禁止直接提交 | 永远稳定，只接受PR合并 |
| **基线分支** | `baseline/recover` | ❌ 禁止直接提交 | 当前最接近可运行的版本 |
| **功能分支** | `feature/*` | ✅ 开放 | 新功能开发 |
| **修复分支** | `fix/*` | ✅ 开放 | Bug修复 |
| **发布分支** | `release/*` | ❌ 禁止直接提交 | 准备发布 |

### 6.2 工作流程

```
main (受保护)
│
├── baseline/recover (基线)
│
├── feature/xxx (开发)
│   └── 开发 → PR → 合并
│
└── fix/xxx (修复)
    └── 修复 → PR → 合并
```

### 6.3 分支命名规范

| 类型 | 命名示例 | 说明 |
|------|----------|------|
| 功能 | `feature/sql-execution` | 新功能 |
| 修复 | `fix/compilation-error` | Bug修复 |
| 文档 | `docs/update-readme` | 文档更新 |
| 重构 | `refactor/header-paths` | 代码重构 |
| Codex | `codex/docs-sdd-tdd-improvement` | Codex 执行的文档/规范任务 |
| 实验 | `experiment/new-optimizer` | 实验性功能 |

### 6.4 创建分支

```bash
# 创建功能分支
git checkout -b feature/your-feature-name

# 创建修复分支
git checkout -b fix/issue-description

# 推送到远程
git push origin feature/your-feature-name
```

---

## 7. 质量门禁

### 7.1 构建要求

```bash
# 必须通过所有构建
bazel build //...

# 必须通过所有测试
bazel test //... --test_output=errors
```

### 7.2 覆盖率要求

覆盖率目标以 `docs/ai_tools/AI_COLLABORATION_GUIDE.md` 中的指标为准（该文档为主标准）。

| Level | 函数覆盖率 | 行覆盖率 | 分支覆盖率 |
|-------|-----------|----------|-----------|
| Level 1 | >= 95% | >= 80% | >= 70% |
| Level 2 | >= 85% | >= 70% | >= 60% |
| Level 3+ | >= 75% | >= 60% | >= 50% |

### 7.3 代码规范

遵循以下文档：

- `AGENTS.md` - 项目编码指南
- `docs/ai_tools/CPP_DEVELOPMENT_SPECIFICATION.md` - C++开发规范
- `docs/ai_tools/BUILD_FILE_SPECIFICATION.md` - Bazel BUILD规范

### 7.4 违反处理

| 违反类型 | 处理方式 |
|----------|----------|
| 未配置Agent身份 | 提交被拒绝 |
| 提交信息不完整 | 提交被拒绝 |
| 直接提交到main | 提交被拒绝 |
| 构建失败 | PR被阻止 |
| 测试失败 | PR被阻止 |
| PR质量不达标 | PR被拒绝 |

---

## 8. 快速开始

### 8.1 新Agent加入流程

```bash
# 1. 克隆仓库
git clone git@github.com:yinglichina8848/sqlcc.git
cd sqlcc

# 2. 配置Agent身份
source scripts/sqlcc-agent-config.sh <agent-id>

# 3. 验证配置
git config user.name && git config user.email

# 4. 创建工作分支
git checkout -b feature/your-task

# 5. 开始工作...
```

### 8.2 日常开发流程

```bash
# 1. 拉取最新代码
git fetch origin

# 2. 基于最新 main 创建分支
git checkout -b feature/your-task origin/main

# 3. 开发...
# (遵循提交规范)

# 4. 提交
git add .
git commit -m "feat: 实现功能

Agent: OpenCode-Developer
Scope: src/module/*
Refs: #ISSUE-NUMBER"

# 5. 推送
git push origin feature/your-task

# 6. 创建PR
# (使用PR模板)
```

### 8.3 验证清单

提交前检查：

```bash
# 1. 检查身份
git config user.name  # 应显示: SQLCC-AI(XXX-YYY)

# 2. 检查构建
bazel build //...  # 必须通过

# 3. 检查测试
bazel test //... --test_output=errors  # 必须通过

# 4. 检查提交信息
cat .git/COMMIT_EDITMSG  # 确保包含 Agent:, Scope:, Refs:
```

---

## 9. 常见问题

### Q1: 忘记配置Agent身份会怎样？

A: Commit Hook会拒绝提交，提示配置Agent身份。

### Q2: 可以在main分支直接提交吗？

A: 不可以。main分支受保护，禁止直接提交。必须通过PR。

### Q3: 如何认领任务？

A: 在任务Issue下评论 `TASK_CLAIM`，或发送消息给Master Agent。

### Q4: 遇到阻塞怎么办？

A: 立即发送 `BLOCKER_NOTIFICATION` 给Master Agent，说明阻塞原因和需要的支持。

### Q5: 多个Agent可以同时修改同一个文件吗？

A: 不建议。会导致合并冲突。建议通过Master Agent协调任务分配。

### Q6: 如何更新Agent配置？

A: 重新执行配置脚本：`source scripts/sqlcc-agent-config.sh <new-agent-id>`

### Q7: PR被拒绝怎么办？

A: 查看Reviewer的反馈，修复问题后重新提交。

---

## 10. 相关文档

| 文档 | 路径 | 说明 |
|------|------|------|
| **本文档** | `docs/ai_tools/MULTI_AGENT_COLLABORATION_GUIDE.md` | 完整协作指南 |
| **协作规范Issue** | `.github/ISSUE_MULTI_AGENT_COLLABORATION.md` | GitHub Issue格式 |
| **Agent配置通知** | `docs/MULTI_AGENT_NOTIFICATION.md` | 配置通知 |
| **快速配置脚本** | `scripts/sqlcc-agent-config.sh` | Agent身份配置 |
| **Agent身份配置** | `~/.gitconfig.d/agents.conf` | Agent身份定义 |
| **PR模板** | `.github/pull_request_template.md` | PR强制模板 |
| **提交钩子** | `.git/hooks/commit-msg` | 提交规范检查 |
| **项目编码指南** | `AGENTS.md` | 整体编码规范 |
| **SDD规范** | `docs/sdd/SPEC_DRIVEN_DEVELOPMENT.md` | 规范驱动开发 |
| **AI协作指南** | `docs/ai_tools/AI_COLLABORATION_GUIDE.md` | AI协作流程 |
| **C++开发规范** | `docs/ai_tools/CPP_DEVELOPMENT_SPECIFICATION.md` | C++开发规范 |

---

## 附录

### A. GitHub/Gitee Issue模板

```markdown
# Issue标题

## 任务状态
- [ ] OPEN
- [ ] CLAIMED
- [ ] WIP
- [ ] DONE

## 描述
<!-- 任务描述 -->

## 验收标准
- [ ] 标准1
- [ ] 标准2

## 分配
Assignee: @agent-id

## 相关Issue
Refs: #XXX
```

### B. 任务状态更新模板

```markdown
## 进度更新

**任务ID**: TASK-XXX
**当前状态**: WIP
**进度**: XX%
**已完成**:
- 工作项1
- 工作项2

**进行中**:
- 工作项3

**阻塞**:
- 无 / 阻塞原因

**预计完成**: YYYY-MM-DD HH:MM
```

### C. 贡献者名单

| Agent | 平台 | 贡献领域 |
|-------|------|----------|
| SQLCC-AI(OpenCode-*) | OpenCode | 主要开发 |
| SQLCC-AI(iFlow-*) | iFlow | 构建/重构 |
| SQLCC-AI(Claude-*) | Claude | 架构/审查 |
| SQLCC-AI(Gemini-*) | Gemini | 分析/调试 |
| SQLCC-AI(OpenClaw-*) | OpenClaw | 工具/规范 |

---

**维护者**: SQLCC AI 治理系统
**版本**: 1.0
**日期**: 2026-02-02
