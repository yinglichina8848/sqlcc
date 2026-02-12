# SQLCC 多Agent跨平台协作规范 (Issue #9)

**title**: [COORDINATION] Unified AI-CLI Workflow via GitHub Issue (Claude/OpenCode/Gemini/Codex)
**author**: yinglichina8848
**state**: OPEN
**comments**: 1

---

## 背景与目标

为避免多个 AI-CLI 在同目录互相污染、互读本地临时文件、流程不一致，统一采用 **GitHub Issue 驱动协作**。

本 Issue 作为总规范：
- 任务发布入口
- 角色分工标准
- 分支/PR 命名规范
- 统一命令模板
- 统一提示词模板
- 质量门禁与合并流程

---

## 0. 基本原则

1. 不跨目录读取其他 AI-CLI 的本地文件。
2. 以 GitHub 为唯一协作事实源：Issue、PR、Comment、CI。
3. 每个 AI-CLI 独立工作目录（worktree 或 clone）+ 独立分支。
4. 任何代码变更必须走 PR，不直接推主分支。
5. 必须经过审核与门禁，不跳过测试。

---

## 1. 角色分工（当前版本）

- **Human (YingLiChina8848)**：最终审批与合并发布。
- **Codex（总控）**：任务拆解、流程协调、门禁清单、合并建议。
- **Claude（主开发）**：核心实现、重构、主测试、主 PR。
- **OpenCode/Kimi2.5（验证）**：编译与测试复核、失败复现、基础测试补充。
- **Gemini-3-pro-preview（复检/文档）**：第二视角审查、文档与变更记录。

---

## 2. 工作目录（约定）

- `/Users/liying/sqlcc`：只读备份，不开发
- `/Users/liying/sqlcc-codex`：总控
- `/Users/liying/sqlcc-claude`：主开发
- `/Users/liying/sqlcc-opencode`：验证
- `/Users/liying/sqlcc-gemini`：复检/文档

---

## 3. 分支命名规范

- Claude：`feat/claude/<task-id>-<short-name>`
- OpenCode：`test/opencode/<task-id>-verify`
- Gemini：`docs/gemini/<task-id>-review`
- Codex（流程类）：`coord/codex/<topic>`

示例：
- `feat/claude/t1401-s07-bufferpool-interface`
- `test/opencode/t1401-s07-verify`
- `docs/gemini/t1401-s07-review`

---

## 4. TASK_ID 规范

格式：`TASK-<version>-<seq>-<topic>`

当前拆分任务：
- `TASK-1401-S01-COLLAB-CORE`
- `TASK-1401-S02-SDD-TDD-FDD`
- `TASK-1401-S03-DESIGN-NORMALIZE`
- `TASK-1401-S04-V140-ARCHIVE`
- `TASK-1401-S05-RELEASE-NOTES-CLEANUP`
- `TASK-1401-S06-DOC-REFORM-TODO`
- `TASK-1401-S07-BUFFERPOOL-INTERFACE`

---

## 5. 统一执行流程（Issue 驱动）

1. Human/Codex 在 Issue 发布任务（TASK_ID + 验收标准）。
2. Claude 认领并开发，提交主 PR（Draft）。
3. OpenCode 在验证分支执行 build/test，提交验证结论或验证 PR。
4. Gemini 提交复检结论与文档更新 PR。
5. Codex 汇总门禁结果，给出 merge 建议。
6. Human 最终批准合并。

---

## 6. 状态机与消息模板

状态机：`OPEN -> CLAIMED -> WIP -> DONE -> FROZEN`

消息模板（评论）

### TASK_CLAIM

```
[TASK_CLAIM]
Task: <TASK_ID>
Agent: <Claude/OpenCode/Gemini/Codex>
Branch: <branch>
ETA: <time>
```

### PROGRESS_UPDATE

```
[PROGRESS_UPDATE]
Task: <TASK_ID>
Agent: <name>
Status: <WIP>
Done: <what finished>
Next: <next step>
Blocker: <none or detail>
```

### BLOCKER_NOTIFICATION

```
[BLOCKER_NOTIFICATION]
Task: <TASK_ID>
Agent: <name>
Blocker: <detail>
Need: <decision/resource>
```

### TASK_COMPLETE

```
[TASK_COMPLETE]
Task: <TASK_ID>
Agent: <name>
PR: <link>
Validation: <build/test summary>
Risk: <low/med/high>
```

---

## 7. 质量门禁（必须）

按变更类型执行：
- 代码 PR：
  - `bazel build <targets>` 通过
  - `bazel test <targets> --test_output=errors` 通过
  - 必要覆盖率检查
- 文档 PR：
  - 链接有效、路径有效、版本号一致
  - 与 CHANGELOG/RELEASE_NOTES 一致

禁止：
- 未验证就声称通过
- 跨角色越权改动（如验证角色改核心架构）
- 直接向 `main`/`release/*` push

---

## 8. 各 AI-CLI 首条提示词（可直接复制）

### Claude（主开发）

```
你是 SQLCC 主开发代理。任务ID: <TASK_ID>。
工作目录: /Users/liying/sqlcc-claude
分支: feat/claude/<task-id>-<short-name>
要求：先读相关代码与 BUILD，再实现；必须执行 bazel build/test；提交可审查 PR。
```

### OpenCode（验证）

```
你是 SQLCC 验证代理。任务ID: <TASK_ID>。
工作目录: /Users/liying/sqlcc-opencode
分支: test/opencode/<task-id>-verify
要求：只做编译测试复核、失败复现、基础测试补充；不改核心架构。
```

### Gemini（复检/文档）

```
你是 SQLCC 复检与文档代理。任务ID: <TASK_ID>。
工作目录: /Users/liying/sqlcc-gemini
分支: docs/gemini/<task-id>-review
要求：给出第二视角审查结论，补充文档与变更记录。
```

### Codex（总控）

```
你是 SQLCC 总控代理。任务ID: <TASK_ID>。
工作目录: /Users/liying/sqlcc-codex
职责：任务拆解、门禁清单、合并建议，不直接替代主开发角色。
```

---

## 9. 推荐命令模板

### Claude

```bash
cd /Users/liying/sqlcc-claude
git fetch origin
git switch -c feat/claude/<task-id>-<short-name> origin/release/1.4
# implement
bazel build <targets>
bazel test <targets> --test_output=errors
git add -A
git commit -m "feat(<module>): <summary> (AI: Claude)"
git push -u origin feat/claude/<task-id>-<short-name>
```

### OpenCode

```bash
cd /Users/liying/sqlcc-opencode
git fetch origin
git switch -c test/opencode/<task-id>-verify origin/feat/claude/<task-id>-<short-name>
bazel build <targets>
bazel test <targets> --test_output=errors
# optional: tests-only patch
```

### Gemini

```bash
cd /Users/liying/sqlcc-gemini
git fetch origin
git switch -c docs/gemini/<task-id>-review origin/feat/claude/<task-id>-<short-name>
# docs/review updates
```

---

## 10. 合并策略

- PR 小步提交，单主题。
- 代码 PR 先合 `release/1.4`，稳定后再回灌 `main`。
- 大 PR（如历史 PR#2）必须拆分后再审。

---

## 11. 执行约束

- 本 Issue 是默认协作契约，后续任务请引用本 Issue 链接。
- 任一 Agent 若偏离流程，由 Codex 在评论中标记并回收任务。
