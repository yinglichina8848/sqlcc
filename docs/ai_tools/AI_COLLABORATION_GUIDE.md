# SQLCC AI 协作开发指南 v1.1

**版本**: 1.1
**日期**: 2026-02-03
**适用范围**: 所有参与 SQLCC 项目的 AI Agent

---

## 📋 文档身份与控制关系

| 属性 | 值 |
|------|-----|
| **文档管理者** | SQLCC-AI(OpenCode Doc) |
| **当前控制者** | YingLi (人类) |
| **未来控制者** | OpenClaw (待配置) |
| **上次更新** | 2026-02-03 - 新增基线治理规范 |

### 身份层级

```
控制链:
  YingLi (人类, 最终控制者)
    └── OpenClaw (多实例协作框架, 待配置)
          └── SQLCC-AI(OpenCode Doc) ← 当前活跃身份
```

---

## 1. 概述

### 1.1 目的
本文档为 SQLCC 项目中的 AI Agent 提供多Agent并行协作的完整指南，确保多个 Agent 能够像人类团队一样高效协同工作。

### 1.2 适用范围
- 单用户多会话 Agent 并行开发
- 团队协作开发（人类+AI 混合）
- 分布式任务分解与执行

### 1.3 前置要求
**所有 Agent 必须首先阅读以下文档**：
1. `docs/sdd/SPEC_DRIVEN_DEVELOPMENT.md` - SDD 规范
2. `docs/ai_tools/AI_DEVELOPMENT_GUIDELINES.md` - AI 开发规范
3. `docs/ai_tools/CPP_DEVELOPMENT_SPECIFICATION.md` - C++ 开发规范
4. `docs/ai_tools/index.md` - AI 工具索引

---

## 1.4 基线治理规范

### 1.4.1 基线分支定义

| 分支 | 名称 | 用途 | 保护级别 |
|------|------|------|----------|
| `baseline/recover` | 基线恢复分支 | 所有开发的唯一合法起点 | ❌ 禁止直接提交 |
| `main` | 主分支 | PR合并后版本 | ❌ 禁止直接提交 |
| `feature/*` | 功能分支 | 日常开发 | ✅ 开放 |
| `fix/*` | 修复分支 | Bug修复 | ✅ 开放 |

### 1.4.2 强制规则：必须基于基线开发

**所有功能分支和修复分支必须基于 `baseline/recover` 创建：**

```bash
# ✅ 正确：基于基线创建分支
git checkout baseline/recover
git checkout -b feature/xxx-xxx

# ❌ 错误：基于main创建分支
git checkout main
git checkout -b feature/xxx-xxx
```

**禁止行为：**
- ❌ 以 `main` 为 base 创建分支
- ❌ 将 `main` 作为 PR 的 target
- ❌ 直接向 `main` 或 `baseline/recover` 提交代码

**允许行为：**
- ✅ 从 `baseline/recover` 创建 `feature/*` 或 `fix/*` 分支
- ✅ PR 的 target 始终为 `baseline/recover`
- ✅ 合并顺序：`feature/*` → `baseline/recover` → `main`

### 1.4.3 分支关系图

```
baseline/recover (基线，不可污染)
│
├── feature/level2-coverage-improvement (当前开发)
├── feature/parser-refactor
├── fix/exception-build-error
│
└── → PR 合并回 baseline/recover → → PR 合并到 main
```

### 1.4.4 分支重置规范

当开发分支偏离基线时，必须重置：

```bash
# 检查分支是否基于基线
git merge-base --is-ancestor baseline/recover HEAD && echo "OK" || echo "NEED RESET"

# 重置到基线
git reset --hard baseline/recover
```

---

## 2. Agent 角色定义

| 角色 | 标识 | 职责 | 权限 |
|------|------|------|------|
| **主Agent (Master)** | 🤖 | 任务分解、进度汇总、最终交付 | 全权限 |
| **开发Agent (Developer)** | 🔨 | 代码实现、单元测试 | 代码读写 |
| **测试Agent (Tester)** | 🧪 | 测试执行、覆盖率分析 | 测试执行 |
| **文档Agent (Documenter)** | 📝 | 文档编写、SDD维护 | 文档读写 |
| **评审Agent (Reviewer)** | 👀 | 代码评审、规范检查 | 只读+评论 |

### 2.1 Agent ID 命名规范

| 角色 | ID格式 | 示例 |
|------|--------|------|
| Master | `agent-master-{n}` | agent-master-1 |
| Developer | `agent-developer-{n}` | agent-developer-1 |
| Tester | `agent-tester-{n}` | agent-tester-1 |
| Documenter | `agent-documenter-{n}` | agent-documenter-1 |
| Reviewer | `agent-reviewer-{n}` | agent-reviewer-1 |

---

## 3. SDD 规范遵从

### 3.1 SDD 工作流程

所有 Agent 必须遵循 SDD 规范：

```
spec-init → spec-req → spec-design → spec-tasks → spec-impl → spec-verification → release
```

### 3.2 SDD 文档要求

| 阶段 | 文档 | 责任Agent | 说明 |
|------|------|----------|------|
| spec-init | README.md | Master | 项目初始化 |
| spec-req | requirements.md | Master | 需求定义 (EARS格式) |
| spec-design | design.md | Master/Reviewer | 架构设计 (Mermaid) |
| spec-tasks | tasks.md | Master | 任务分解 |
| spec-impl | 代码实现 | Developer | 代码编写 |
| spec-verification | verification.md | Tester | 验证确认 |

### 3.3 任务状态机

```
                    ┌─────────────────────────────────────┐
                    │              任务生命周期             │
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
| 开放 | OPEN | `0` | 任务待认领，任何Agent可认领 |
| 已认领 | CLAIMED | `1` | 已被Agent认领，尚未开始 |
| 进行中 | WIP | `2` | 正在积极开发中 |
| 暂停 | PAUSED | `3` | 暂时停止，等待资源或依赖 |
| 让出 | YIELDED | `4` | Agent主动释放，需重新认领 |
| 阻塞 | BLOCKED | `5` | 等待前置任务或外部依赖 |
| 完成 | DONE | `6` | 任务完成，已验证 |
| 冻结 | FROZEN | `7` | 代码冻结，待发布 |

---

## 4. 消息通信协议

### 4.1 消息类型

| 消息类型 | 方向 | 说明 |
|----------|------|------|
| `TASK_CLAIM` | Agent → Master | 任务认领 |
| `TASK_RELEASE` | Agent → Master | 任务释放 |
| `PROGRESS_UPDATE` | Agent → Master | 进度更新 |
| `BLOCKER_NOTIFICATION` | Agent → Master | 阻塞通知 |
| `TASK_COMPLETE` | Agent → Master | 任务完成 |
| `ASSISTANCE_REQUEST` | Agent → Agent | 协助请求 |
| `REVIEW_REQUEST` | Agent → Reviewer | 评审请求 |
| `HEARTBEAT` | Agent → Master | 心跳保活 |

### 4.2 消息格式

#### 任务认领消息
```json
{
  "type": "TASK_CLAIM",
  "task_id": "TASK-XXX-001",
  "agent_id": "agent-developer-1",
  "timestamp": "2026-02-02T10:00:00Z",
  "message": "认领 TASK-XXX-001: [任务名称]",
  "estimated_duration": "4h"
}
```

#### 进度更新消息
```json
{
  "type": "PROGRESS_UPDATE",
  "task_id": "TASK-XXX-001",
  "agent_id": "agent-developer-1",
  "timestamp": "2026-02-02T12:00:00Z",
  "status": "WIP",
  "progress_percent": 50,
  "message": "已完成接口定义，开始实现核心逻辑",
  "blockers": [],
  "next_steps": ["实现函数A", "实现函数B"]
}
```

#### 阻塞通知消息
```json
{
  "type": "BLOCKER_NOTIFICATION",
  "task_id": "TASK-XXX-002",
  "agent_id": "agent-developer-2",
  "timestamp": "2026-02-02T14:00:00Z",
  "blocked_by": ["TASK-XXX-001"],
  "message": "等待 TASK-XXX-001 完成依赖接口",
  "severity": "P1",
  "suggestion": "请 TASK-XXX-001 优先完成 [具体接口]"
}
```

#### 任务完成消息
```json
{
  "type": "TASK_COMPLETE",
  "task_id": "TASK-XXX-001",
  "agent_id": "agent-developer-1",
  "timestamp": "2026-02-02T16:00:00Z",
  "status": "DONE",
  "summary": "完成接口定义和核心实现",
  "files_created": ["src/xxx.cpp"],
  "files_modified": ["src/xxx.h"],
  "tests_added": ["tests/xxx_test.cpp"],
  "verification": {
    "compile": "SUCCESS",
    "test": "SUCCESS",
    "coverage": "95%"
  }
}
```

### 4.3 沟通频率

| 事件 | 触发条件 | 响应时间 |
|------|----------|----------|
| 任务认领 | 认领时 | 即时 |
| 进度更新 | 每30分钟或里程碑达成 | 5分钟内Master确认 |
| 阻塞通知 | 发现阻塞时 | 即时 |
| 任务完成 | 完成所有验收标准 | 即时 |
| 协助请求 | 需要帮助时 | 15分钟内响应 |
| 心跳 | 每5分钟 | - |

---

## 5. 并行开发规范

### 5.1 任务并行度

```yaml
parallel_development:
  # 最大并行Agent数
  max_concurrent_agents: 4

  # 任务分配策略
  assignment_strategy: "skill_based"  # skill_based / load_balanced / round_robin

  # 任务依赖检查
  dependency_check: true

  # 冲突检测
  conflict_detection: true

  # 心跳间隔 (秒)
  heartbeat_interval: 300

  # 进度更新间隔 (秒)
  progress_update_interval: 1800
```

### 5.2 资源冲突检测

Agent 在认领任务前必须检查文件冲突：

```bash
# 检查任务文件是否与其他任务冲突
./scripts/detect_file_conflicts.sh --task TASK-XXX-001
```

### 5.3 任务分配策略

**基于技能分配** (skill_based):
- 根据 Agent 技能标签匹配任务需求
- 优先分配给最适合的 Agent

**负载均衡分配** (load_balanced):
- 统计各 Agent 当前任务数
- 优先分配给任务最少的 Agent

**轮询分配** (round_robin):
- 按 Agent 列表顺序轮流分配
- 确保任务均匀分布

---

## 6. 任务看板管理

### 6.1 看板视图

Agent 必须维护最新的看板状态：

```markdown
## 任务看板 - [功能名称]

### TODO (待认领)
| ID | 任务 | 优先级 | 预计时间 | 认领Agent |
|----|------|--------|----------|-----------|
| TASK-001 | [任务名] | P0 | 1h | - |

### IN PROGRESS (进行中)
| ID | 任务 | 负责人 | 进度 | 预计完成 |
|----|------|--------|------|----------|
| TASK-002 | [任务名] | @agent-1 | 50% | 17:00 |

### IN REVIEW (评审中)
| ID | 任务 | 作者 | 评审人 | 状态 |
|----|------|------|--------|------|
| TASK-003 | [任务名] | @agent-1 | @agent-reviewer-1 | PENDING |

### DONE (已完成)
| ID | 任务 | 负责人 | 完成时间 |
|----|------|--------|----------|
| TASK-004 | [任务名] | @agent-2 | 2026-02-02 |

### BLOCKED (阻塞)
| ID | 任务 | 阻塞原因 | 解决方案 |
|----|------|----------|----------|
| TASK-005 | [任务名] | 等待 TASK-001 | TASK-001 优先 |
```

### 6.2 燃尽图跟踪

| 日期 | 剩余任务 | 累计完成 | 状态 |
|------|----------|----------|------|
| 2026-02-02 | 10 | 0 | 🟢 Day 1 |
| 2026-02-03 | 8 | 2 | 🟢 Day 2 |
| 2026-02-04 | 5 | 5 | 🟡 Day 3 |
| 2026-02-05 | 2 | 8 | 🟡 Day 4 |
| 2026-02-06 | 0 | 10 | ✅ Day 5 |

---

## 7. 验收标准

### 7.1 任务验收清单

| 检查项 | Agent | 状态 |
|--------|-------|------|
| 编译通过 (bazel build) | Developer | ☐ |
| 测试通过 (bazel test) | Tester | ☐ |
| 覆盖率达标 | Tester | ☐ |
| 代码评审通过 | Reviewer | ☐ |
| 文档完整 | Documenter | ☐ |
| CHANGELOG 已更新 | Documenter | ☐ |

### 7.2 Agent 验收清单

| 检查项 | 状态 | 说明 |
|--------|------|------|
| 任务状态正确 | ☐ | 按状态机流转 |
| 进度更新及时 | ☐ | 符合沟通频率 |
| 消息格式规范 | ☐ | 符合协议格式 |
| 阻塞及时上报 | ☐ | 无遗漏阻塞 |

### 7.3 覆盖率要求

| Level | 函数覆盖率 | 行覆盖率 | 分支覆盖率 |
|-------|-----------|----------|-----------|
| Level 1 | >= 95% | >= 80% | >= 70% |
| Level 2 | >= 85% | >= 70% | >= 60% |
| Level 3+ | >= 75% | >= 60% | >= 50% |

---

## 8. 故障处理

### 8.1 Agent 故障处理

| 故障类型 | 检测方式 | 处理措施 |
|----------|----------|----------|
| Agent 超时 | 心跳缺失 (>10分钟) | 任务重新分配 |
| Agent 失联 | 消息无响应 (>30分钟) | 任务释放，通知 Master |
| Agent 冲突 | 文件冲突检测 | 协调执行顺序或重新分配 |
| Agent 重复 | 同一任务多次认领 | 按优先级保留，其他释放 |

### 8.2 任务恢复流程

```markdown
## 任务恢复流程

### 步骤 1: 检测故障
- Master Agent 检测心跳超时
- 或用户报告 Agent 失联

### 步骤 2: 确认状态
- 检查任务最后状态
- 评估已完成工作量

### 步骤 3: 重新分配
- 发送 TASK_RECOVERY 消息给其他 Agent
- 提供任务上下文和已完成工作

### 步骤 4: 继续执行
- 新 Agent 从最后状态继续
- 必要时回滚部分代码
```

---

## 9. 常用命令

### 9.1 开发命令

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

### 9.2 协作命令

```bash
# 启动并行开发
./scripts/start_parallel_agents.sh --config .claude/parallel-config.yaml

# 检查任务状态
./scripts/check_task_status.sh tasks.md

# 同步进度
./scripts/sync_progress.sh tasks.md

# 检测冲突
./scripts/detect_conflicts.sh tasks.md
```

---

## 10. 相关文档

| 文档 | 路径 | 说明 |
|------|------|------|
| SDD 规范 | `docs/sdd/SPEC_DRIVEN_DEVELOPMENT.md` | 规范驱动开发指南 |
| AI 开发规范 | `docs/ai_tools/AI_DEVELOPMENT_GUIDELINES.md` | AI Agent 开发指南 |
| C++ 开发规范 | `docs/ai_tools/CPP_DEVELOPMENT_SPECIFICATION.md` | C++ 编码规范 |
| 测试规范 | `docs/ai_tools/improvement_guide.md` | 测试开发规范 |
| 构建规范 | `docs/ai_tools/BUILD_FILE_SPECIFICATION.md` | Bazel BUILD 文件规范 |
| 多Agent模板 | `docs/sdd/templates/multi_agent_collaboration_template.md` | 多Agent协作模板 |
| 验证模板 | `docs/sdd/templates/verification_template.md` | 验收验证模板 |

---

## 11. 变更历史

| 版本 | 日期 | 变更内容 | 变更人 |
|------|------|---------|--------|
| 1.0 | 2026-02-02 | 初始版本 | SQLCC AI |

---

**维护者**: SQLCC AI 开发团队
**最后更新**: 2026-02-02
**版本**: v1.0
