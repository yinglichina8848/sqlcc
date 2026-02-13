# SQLCC SDD-TDD Orchestrator Skill

## Purpose
把任务从“模糊重构”强制拆成可执行的 SDD+TDD 阶段，避免直接进入大范围改码。

## When To Use
1. 任务刚认领，需要先做规范拆分。
2. 出现 scope 漂移或跨层修复冲动。
3. 需要把 R2（并行轨道）或 R3（新项目试验）拆成小步任务。

## Inputs
1. `task_id` (例如 `TASK-1401-S05R`)
2. `track` (`R1` / `R2` / `R3`)
3. `scope` (明确模块边界)

## Output Contract
必须产出 6 段结构化结果：
1. `SPEC_REVIEW` 拆分（requirements/design/tasks）
2. `TEST_REVIEW` 拆分（contract tests + regression tests）
3. `WIP` 拆分（interface -> stub -> adapter -> impl）
4. Gate 命令列表（只增量 target）
5. 回滚点（每阶段 1 个）
6. Issue 回帖模板（CLAIM/WIP/BLOCKER/DONE）

## Workflow
1. Freeze Scope
- 只允许当前层级模块（例如 L2 core），禁止自动扩到 L3/L4。

2. Spec Split
- 写清接口契约、错误模型、依赖边界。

3. Test Split
- 先定义契约测试，再定义最小回归测试。

4. Implementation Split
- 顺序固定：`interfaces_v2` -> `impl_stub_v2` -> `adapters_v2` -> `real_impl_stepN`。

5. Evidence Pack
- 每步附：分支、commit、命令、输出摘要、风险。

## Hard Rules
1. 禁止 `bazel build //...` 作为日常门禁。
2. 禁止“一次 PR 同时改接口+实现+跨层修复”。
3. 连续 2 次 gate 失败必须回到 `SPEC_REVIEW`。

