# SQLCC Issue Control Skill

## Purpose
把多 AI 协作固定在 Issue 状态机中，减少人工盯盘成本。

## Required Issue State Machine
`TASK_CLAIM -> PROGRESS_UPDATE -> (BLOCKER_NOTIFICATION) -> TASK_COMPLETE`

## Checks
1. 是否缺 `TASK_CLAIM`
2. 是否超 30 分钟无 `PROGRESS_UPDATE`
3. 是否出现执行态越界（例如 BRAINSTORM 阶段启动实现）
4. 是否缺可复核证据（branch/commit/commands）

## Output
1. 当前状态（OK/AT_RISK/BLOCKED）
2. 缺失项列表
3. 纠偏指令模板

## Coordinator Message Templates

### Missing Claim
`[COORDINATOR_CHECK] 缺 TASK_CLAIM，请先认领后继续。`

### Missing Progress
`[COORDINATOR_CHECK] 超过 30 分钟未更新，请补 PROGRESS_UPDATE。`

### Scope Drift
`[COORDINATOR_SCOPE_GUARD] 检测到越界执行，回到当前范围并提交纠偏说明。`

