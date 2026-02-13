# Codex + Skills 本地配置说明（SQLCC）

## 结论
1. 可以在仓库内启用 `Codex + Skills` 作为总控规则中心。
2. 你仍需在 OpenCode/Claude 各自会话中触发执行（Codex 无法直接接管独立 CLI 会话）。
3. Superworks 可以作为流程编排参考，但当前仓库内先落地“本地可控方案”更稳。

## 已落地文件
1. `.claude/skills/sqlcc-sdd-tdd-orchestrator/SKILL.md`
2. `.claude/skills/sqlcc-issue-control/SKILL.md`
3. `docs/sdd/refactoring/level2_core/S05R_SDD_TDD_TASK_SPLIT.md`

## 使用方式

### 1) 任务拆分
在任务开始前先执行 `sqlcc-sdd-tdd-orchestrator`，得到分阶段任务和 gate 列表。

### 2) 协作巡检
每 30 分钟执行 `sqlcc-issue-control`，检查是否缺 CLAIM/WIP/BLOCKER/DONE 证据。

### 3) 只允许增量 gate
示例：
```bash
bazel build //src/core/interfaces_v2:interfaces_v2
bazel build //src/core/impl_stub_v2:impl_stub_v2
bazel test //tests/level2_core_v2/contract:all
```

## 关于 Superworks
1. Superworks/类似工作流平台属于“流程编排层”。
2. 可用于后续可视化编排，但不替代 SDD-TDD 的技术拆分与门禁证据。
3. 建议先跑通本地 `Codex + Skills`，再对接外部编排器。

