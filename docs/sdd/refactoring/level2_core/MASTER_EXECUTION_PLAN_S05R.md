# TASK-1401 S05R 总体推进方案（接口先行 + 空实现 + 增量集成）

**版本**: v1.0-draft  
**日期**: 2026-02-13  
**状态**: `SPEC_REVIEW`（仅规划，禁止直接重构）  
**适用范围**: Issue #9 / TASK-1401（Level2 Core 重构链路）  
**关联文档**:
- `docs/sdd/SPEC_DRIVEN_DEVELOPMENT.md`
- `docs/sdd/refactoring/level2_core/design.md`
- `docs/sdd/refactoring/level2_core/tasks.md`
- `docs/ai_tools/AI_COLLABORATION_GUIDE.md`

---

## 1. 背景与问题定义

### 1.1 当前问题

1. 任务范围漂移：Level2 Core 问题处理中混入了 Execution（L3）实现修复，导致目标不清。
2. 阶段倒置：设计与接口评审未闭环，已提前进入实现/大范围编译。
3. 证据链不稳定：不同 Agent 使用的 gate 口径不一致（L2 目标存在冲突历史）。
4. 试错成本高：在“屎山”上直接修补，引入更多编译和依赖连锁问题。

### 1.2 本文目标

建立可执行、可审计、可回滚的推进方案，优先保证：
1. 目标清晰（先 L2 边界，后 L3/L4 扩展）
2. 流程正确（SDD/TDD 阶段化）
3. 门禁稳定（单一口径、单点放行）
4. 协作可靠（多 Agent 不跑偏）

---

## 2. 路线对比（必须评审后决策）

### 2.1 方案 A：原地修补（在现有 `src` 直接修）

**优点**
- 改动面小，短期可见结果快。

**缺点**
- 高耦合区域容易反复破编译。
- 容易再次触发范围外问题（L3/L4）。
- 审计难度高，回滚颗粒粗。

### 2.2 方案 B：仓内并行轨道（推荐候选）

**定义**: 在现仓内创建 `v2` 接口与最小实现轨道，旧实现不删除；通过适配层增量接入。

**优点**
- 风险隔离，避免一次性重构。
- 可先实现“可编译最小闭环”，再替换真实逻辑。
- 证据链清晰，适合 PR 小步快跑。

**缺点**
- 短期存在双轨维护成本。
- 需要更严格的命名与目录治理。

### 2.3 方案 C：新项目重写（新仓）

**优点**
- 架构最干净，可完全按新规范推进。

**缺点**
- 与现有 SQLCC 演进脱节，迁移成本极高。
- 验收周期长，短期无法解决当前阻塞。
- 测试资产复用率低。

### 2.4 评审决策机制

在 Issue #9 通过 `DESIGN_REVIEW_PACKET` 投票并记录结论：
1. Master（Codex）给出风险评估
2. Claude/OpenCode/Gemini 各给出 `REVIEW_REPORT`
3. 满足“2/3 同意 + 无 P0 反对”才可进入执行阶段

> 当前建议：**先按方案 B 规划与试点**，但不在本文件中直接定版执行。

---

## 3. 执行原则（强约束）

1. **先规范后代码**：未通过 `SPEC_REVIEW + TEST_REVIEW`，不得进入实现 PR。
2. **先接口后实现**：先定义契约（头文件/API/错误模型），再做空实现。
3. **先最小编译后功能**：先通过最小 gate，再逐步替换真实实现。
4. **禁大范围编译**：每个任务只跑对应 target，禁止 `bazel build //...` 作为日常 gate。
5. **单任务单目标**：一个任务只解决一个问题域（接口、stub、适配、测试、实现之一）。
6. **证据即门禁**：无命令、无输出、无分支/commit 证据 = 未完成。

---

## 4. 版本、基线与分支策略

### 4.1 版本策略

1. 规划版本：`S05R-Plan v1.x`（文档与评审）
2. 实施版本：`S05R-Impl v2.x`（接口/空实现/集成）
3. 发布版本：跟随 `release/1.4.x` 的增量补丁

### 4.2 基线策略

1. 代码基线：`release/1.4`
2. 协作基线：Issue #9（控制面） + Issue #10（Ops Log）
3. 门禁基线：按任务定义固定 target，禁止临时改口径

### 4.3 分支模型

**阶段 0（仅规划）**
- `plan/t1401-s05r-interface-first`

**阶段 1（接口契约）**
- `feat/t1401-s05r-l2-interface-spec`

**阶段 2（空实现最小闭环）**
- `feat/t1401-s05r-l2-stub-compile`

**阶段 3（适配层接入）**
- `feat/t1401-s05r-l2-adapter-integration`

**阶段 4（真实实现替换）**
- `feat/t1401-s05r-l2-real-impl-stepN`

规则：
1. 禁止跨阶段复用同一长期分支。
2. 每个分支只对应一个 PR（必要时 stack PR，但必须有依赖图）。

---

## 5. 目录与代码组织建议（方案 B 对应）

### 5.1 新增并行目录（建议）

1. `src/core/interfaces_v2/`：纯接口与数据契约
2. `src/core/impl_stub_v2/`：空实现（可编译、可注入）
3. `src/core/adapters_v2/`：旧实现到新接口的适配层
4. `tests/level2_core_v2/contract/`：接口契约测试

### 5.2 既有代码处理

1. 旧代码保持可用，不做大删改。
2. 只允许“最小兼容修改”（例如 include 修正、命名冲突修复）。
3. 禁止在同一个 PR 同时做“接口变更 + 大实现重构 + 跨层修复”。

---

## 6. SDD+TDD 阶段任务设计（先评审再执行）

### 6.1 阶段与准入

1. `SPEC_REVIEW`
- 产物：requirements/design/tasks（含接口边界与错误模型）
- 准入：三方评审通过，关键分歧关闭

2. `TEST_REVIEW`
- 产物：契约测试列表、最小回归测试列表、失败预期定义
- 准入：测试与接口一一映射

3. `WIP`（实现）
- 先 `interfaces_v2` 再 `impl_stub_v2`，最后适配与真实实现
- 每一步均要求独立 gate

4. `CODE_REVIEW -> DONE`
- 需包含：命令、结果、文件清单、风险、回滚方案

### 6.2 最小任务切片模板（建议）

1. `S05R-A1` 接口文档与头文件草案（不落地复杂实现）
2. `S05R-A2` 契约测试样例（先 failing 或 mock 驱动）
3. `S05R-B1` 空实现 + Bazel target
4. `S05R-B2` 适配层最小链路
5. `S05R-C1` 真实实现替换（单接口单功能）

---

## 7. Gate 与验收标准（防止跑偏）

### 7.1 Gate 顺序

1. Gate-P0（规划门禁）
- 文档齐全、评审结论明确、任务边界冻结

2. Gate-L2A（接口门禁）
- `bazel build //src/core/interfaces_v2:interfaces_v2`

3. Gate-L2B（空实现门禁）
- `bazel build //src/core/impl_stub_v2:impl_stub_v2`
- `bazel test //tests/level2_core_v2/contract:all`

4. Gate-L2C（适配门禁）
- `bazel build //src/core/adapters_v2:adapters_v2`

5. Gate-L2D（真实实现增量门禁）
- 仅运行本任务相关 target + 契约回归

### 7.2 放行条件

1. 无 P0 blocker
2. 所有 gate 证据可复核（命令/输出/commit）
3. 任务链满足 `TASK_CLAIM -> WIP -> DONE`
4. Issue #9 与 #10 双写一致

---

## 8. PR 管理与变更控制

### 8.1 PR 类型

1. `spec`：只改文档，不改实现
2. `test`：只加/改测试与 mock
3. `refactor`：接口或结构性改造（无功能扩展）
4. `fix`：针对明确编译/行为问题的小修复

### 8.2 PR 模板强制字段

1. 任务 ID 与状态
2. 变更范围（明确不包含内容）
3. Gate 命令与结果
4. 风险与回滚
5. 关联 Issue comment 链接

### 8.3 合并策略

1. 小 PR（建议 < 300 行核心代码变更）
2. 必须 reviewer 签字（至少 1 人，关键任务 2 人）
3. 禁止“评审未结论先合并”

---

## 9. AI 协作机制（Issue 驱动）

### 9.1 角色分工

1. Codex（协调者/门禁）：任务分解、范围锁定、放行决策
2. Claude（主开发）：接口与实现主线产出
3. OpenCode（验证/复核）：复现实验、门禁复核、反例验证
4. Gemini（交叉评审/Linux 复检）：设计评审、Linux 编译验证

### 9.2 通信协议

1. 开工必须 `TASK_CLAIM`
2. 每 30 分钟 `PROGRESS_UPDATE`
3. 阻塞 5 分钟内 `BLOCKER_NOTIFICATION`
4. 完成后 `TASK_COMPLETE`（必须含可复核证据）

### 9.3 反漂移规则

1. 任意 Agent 不得私自扩展任务范围
2. 发现跨层问题只能提 `SCOPE_CHANGE_REQUEST`
3. 未经协调者批准，不得把 L2 问题升级为 L3 实现重构

---

## 10. Skills 工作流纳入方案

### 10.1 目标

减少口径不一致与重复试错，将常用动作固化为可复用技能流程。

### 10.2 建议技能包（最小集合）

1. `sqlcc-sdd-gate-check`
- 输入：任务 ID、目标 gate
- 输出：规范检查清单 + 可执行命令集

2. `sqlcc-issue-sync`
- 输入：Issue #9/#10 内容
- 输出：状态机一致性审计（是否缺 CLAIM/WIP/DONE）

3. `sqlcc-pr-audit`
- 输入：PR diff 与任务边界
- 输出：越界变更告警（跨层、超范围、缺测试）

### 10.3 使用约束

1. 技能只做流程约束，不替代架构决策。
2. 技能输出必须入 Issue 留痕，作为门禁证据。

---

## 11. 风险清单与回滚策略

### 11.1 关键风险

1. 双轨目录长期并存导致维护成本升高
2. 适配层设计不当导致性能/语义偏差
3. PR 粒度失控导致评审失败
4. AI 协作消息不规范造成证据链断裂

### 11.2 回滚策略

1. 文档阶段可直接回滚到上一个已评审版本
2. 代码阶段按分支粒度回滚，不跨分支混回滚
3. 任一 gate 连续 2 轮失败，自动回到 `SPEC_REVIEW`

---

## 12. 成功标准（Definition of Success）

1. 目标稳定：连续 3 轮无 scope 漂移
2. 门禁稳定：L2A/L2B/L2C 连续通过
3. 协作稳定：三方消息协议合规率 100%
4. 变更可控：PR 平均审查周期下降，返工率下降

---

## 13. 待评审决议（必须在 #9 明确）

1. 三选一：A/B/C 路线最终定版
2. 是否冻结当前 S05C 实现类改动，仅保留分析与设计
3. 是否批准新增 `v2` 并行目录
4. 是否启用 Skills 最小集合试运行

> 未完成以上决议前，本方案仅为“执行前蓝图”，不进入实现阶段。

---

## 14. 工作量评估与成功率分析

### 14.1 评估口径

1. 以 1 个 Agent 日 = 6 小时有效工程时间估算。
2. 估算包含：设计、评审、实现、复核、门禁修复，不包含跨月长期优化。
3. 成功率定义：在不扩 scope 的前提下，于既定 gate 达成 `DONE` 的概率。

### 14.2 三路线工作量对比（A/B/C）

| 路线 | 预计工作量 | 主要风险 | 失败模式 | 短期成功率（Issue #9） | 中长期架构收益 |
|------|------------|----------|----------|------------------------|----------------|
| A 原地修补 | 8-15 Agent 日 | 连锁编译、隐式耦合 | 修一处炸三处 | 中（40-60%） | 低 |
| B 仓内并行轨道 | 12-22 Agent 日 | 双轨维护、适配层复杂度 | 目录治理失控 | 高（65-80%） | 高 |
| C 新项目重写 | 25-60 Agent 日 | 交付周期、迁移与验收 | 长期未并轨、目标漂移 | 低（20-40%） | 很高（若完整落地） |

### 14.3 结论（分目标）

1. **若目标是尽快让 TASK-1401 稳定收口**：优先 B（风险最低、成功率最高）。
2. **若目标是教学演进且允许推倒重来**：可启动 C 的“独立教学轨道”，但不替代 #9 收口任务。
3. 建议双轨治理：
- 主线（#9）按 B 收口当前问题；
- 支线（新 Issue）评估 C，采用严格里程碑，不与主线门禁混用。

### 14.4 推荐里程碑（B 路线）

1. M0（2-3 Agent 日）：完成 `SPEC_REVIEW + TEST_REVIEW`，冻结边界。
2. M1（3-5 Agent 日）：`interfaces_v2` + 契约测试样例，过 Gate-L2A。
3. M2（3-6 Agent 日）：`impl_stub_v2`，过 Gate-L2B。
4. M3（4-8 Agent 日）：`adapters_v2` 最小集成，过 Gate-L2C。
5. M4（可迭代）：按单接口替换真实实现，逐步达成 L2D。

---

## 15. 协同工作流能力基线（2026-02 核验）

### 15.1 已核验可用能力（优先采用）

1. Claude Code Subagents（官方文档）  
  用途：独立上下文执行专门任务，适合评审、测试、文档分工。
2. Claude Code Agent Teams（官方文档，实验特性）  
  用途：多会话协作、共享任务列表、队友消息、delegate 模式、hook 质量门禁。
3. Claude Skills（官方仓库与文档）  
  用途：固化可复用流程（门禁检查、PR审计、Issue同步）。
4. Hooks + Permissions + MCP（官方设置文档）  
  用途：把质量门禁、敏感文件保护、外部工具接入流程化。

### 15.2 未核验或命名不稳定项（降级处理）

1. `TeammateTool`、`Swarm Orchestration Skill`（未见官方稳定命名）
2. `CoWokers`（未定位到明确官方项目定义）
3. `TeamAgent`（检索到“概念/标签”多于单一权威框架）

处理规则：
1. 不把未核验术语写入强约束流程。
2. 若后续出现官方规范，再增量纳入本方案。

### 15.3 对 SQLCC 的落地映射

1. Master 使用 Agent Teams 的 delegate 模式，只做协调与门禁，不写实现。
2. Developer 子 Agent 仅处理单一任务切片（接口/stub/适配/测试）。
3. Reviewer 子 Agent 强制运行 `sqlcc-pr-audit` 技能并输出证据。
4. `TaskCompleted` hook 拦截：未附 gate 证据的任务不得标记完成。

---

## 16. 复杂项目场景适配（网站复刻/游戏/浏览器）

### 16.1 能力判断

1. Skills/Subagents/Agent Teams 能提升复杂项目分工效率。
2. 但成功关键不在“自动化程度”，而在“边界拆分 + 门禁 + 回滚设计”。
3. 对复杂项目，必须先定义“可演示最小闭环（MVP gate）”，禁止一口气全量实现。

### 16.2 统一增量模板（可复用）

1. 阶段 A：规格与接口（UI/API/模块契约）
2. 阶段 B：空实现与冒烟链路（先跑通，不求完整功能）
3. 阶段 C：单模块替换真实实现（每次只换一个组件）
4. 阶段 D：回归测试与性能门禁

该模板与本方案一致，避免再次“先重构后澄清目标”的循环。

---

## 17. 参考来源（核验时间：2026-02-13）

1. Claude Code Subagents: https://code.claude.com/docs/en/sub-agents
2. Claude Code Agent Teams: https://code.claude.com/docs/en/agent-teams
3. Claude Code Settings（skills/plugins/permissions）: https://code.claude.com/docs/en/settings
4. Anthropic Skills Repo: https://github.com/anthropics/skills
5. Claude Agent Skills Overview: https://docs.claude.com/en/docs/agents-and-tools/agent-skills
6. OpenAI Skills Catalog（对比参考）: https://github.com/openai/skills
7. AWS Agent Squad（多 Agent 编排参考）: https://github.com/awslabs/agent-squad

---

## 18. 方案 B 具体细化（执行蓝图）

### 18.1 总体目标（仅 Level2 Core）

1. 在不扩大到 L3 实现重构的前提下，建立 L2 可编译、可测试、可替换的接口轨道。
2. 以最小增量完成从“接口契约”到“空实现”再到“适配接入”的闭环。
3. 每一步都能独立回滚，避免一次性大改。

### 18.2 任务波次（Wave）设计

| Wave | 目标 | 产出 | 负责人 | 预计工作量 |
|------|------|------|--------|------------|
| W0 | 方案与边界冻结 | 审批版 requirements/design/tasks | Codex+Claude+Gemini+OpenCode | 2-3 Agent 日 |
| W1 | 接口契约落地 | `interfaces_v2` + API 注释 + BUILD | Claude（开发）Gemini（评审） | 3-4 Agent 日 |
| W2 | 测试先行 | `contract` 契约测试 + mock/stub 规范 | OpenCode（测试）Gemini（复核） | 2-3 Agent 日 |
| W3 | 空实现闭环 | `impl_stub_v2` 可编译实现 + 注入点 | Claude（开发）OpenCode（验证） | 3-5 Agent 日 |
| W4 | 适配层最小接入 | `adapters_v2` 接旧实现最小链路 | Claude（开发）Gemini（Linux复检） | 4-6 Agent 日 |
| W5 | 单接口增量替换 | `real_impl_stepN`（每次一接口） | Claude 主改，OpenCode/Gemini 双评审 | 持续迭代 |

### 18.3 每波次进入/退出条件

1. 进入条件（Entry）
- 上一波次 gate 全通过；
- Issue #9/#10 证据链完整；
- 无未决 P0 blocker。

2. 退出条件（Exit）
- 对应 target 编译/测试通过；
- 评审结论达到 PASS 或 PASS_WITH_CONDITIONS（条件已闭环）；
- 回滚方案已记录。

---

## 19. 多 AI 协作执行模型（方案 B）

### 19.1 RACI 分工矩阵

| 活动 | Codex | Claude | OpenCode | Gemini |
|------|-------|--------|----------|--------|
| 范围定义/冻结 | A/R | C | C | C |
| 接口设计与落地 | C | A/R | C | C |
| 契约测试设计 | C | C | A/R | C |
| Linux clang-20 复检 | C | C | C | A/R |
| Gate 复核与反例 | A | C | R | R |
| 最终放行建议 | A/R | C | C | C |

说明：A=Accountable，R=Responsible，C=Consulted。

### 19.2 协作节奏（固定 cadence）

1. 每 30 分钟：各 Agent 在 #9 发 `PROGRESS_UPDATE`。
2. 任意阻塞 > 5 分钟：立即发 `BLOCKER_NOTIFICATION`，附已尝试方案。
3. 每 2 小时：Codex 发一次 `COORDINATOR_STATUS_REFRESH`（状态/风险/下一步）。
4. 每波次结束：提交 `WAVE_CLOSEOUT_PACKET`（证据汇总+未决风险）。

### 19.3 交接物标准（Handoff Contract）

每次交接必须包含：
1. 分支名与 commit（可 fetch）
2. 文件清单（新增/修改）
3. gate 命令与关键输出
4. 未解决风险与建议下一步
5. 回滚点（上一个稳定 commit）

### 19.4 冲突与跑偏处理

1. 发现跨层改动：立即 `SCOPE_CHANGE_REQUEST`，默认不执行。
2. 连续两轮 gate 失败：自动回退到上一波次（W-1）并补设计。
3. 评审意见冲突：触发 `JOINT_REVIEW_PACKET`，由 Codex 做最终门禁裁决。

---

## 20. 方案 B 的 PR Stack 设计（可直接执行）

### 20.1 推荐 PR 序列

1. PR-0（spec）  
- 仅文档：冻结边界与 gate 口径  
- 合并条件：三方评审通过

2. PR-1（refactor）  
- 新增 `interfaces_v2`（不接业务实现）  
- 合并条件：Gate-L2A 通过

3. PR-2（test）  
- 新增 `tests/level2_core_v2/contract`  
- 合并条件：测试可运行（允许先 fail，需记录预期）

4. PR-3（refactor）  
- 新增 `impl_stub_v2`，使契约测试可编译运行  
- 合并条件：Gate-L2B 通过

5. PR-4（refactor）  
- 新增 `adapters_v2` 最小接入  
- 合并条件：Gate-L2C 通过

6. PR-5+（refactor/fix）  
- 单接口替换真实实现（一次只做一个接口）  
- 合并条件：L2D 增量 gate 通过

### 20.2 PR 审查重点

1. 是否越界（是否引入 L3/L4 实现改动）
2. 是否一 PR 多目标（接口+实现+测试混改）
3. 是否具备可复核证据（命令、输出、commit）
4. 是否包含回滚路径

---

## 21. 协作工具链建议（含 Skills）

### 21.1 最小工具链

1. 控制面：Issue #9（任务状态）+ Issue #10（Ops 日志）
2. 代码面：Stacked PR + 小步合并
3. 质量面：Bazel 增量 target + Linux clang-20 复检
4. 流程面：Skills + hooks（自动门禁）

### 21.2 Skills 实施清单（项目级）

1. `.claude/skills/sqlcc-sdd-gate-check/`
- 校验任务是否满足 SDD 阶段准入

2. `.claude/skills/sqlcc-issue-sync/`
- 校验 #9/#10 是否双写一致

3. `.claude/skills/sqlcc-pr-audit/`
- 检测 PR 是否越界或缺 gate 证据

### 21.3 Hooks 建议（项目级）

1. `TaskCompleted` hook：无 gate 证据则拒绝标记完成。
2. `PreToolUse(Bash)` hook：拦截 `bazel build //...`（要求改为增量 target）。
3. `TeammateIdle` hook（如使用 Agent Teams）：自动分配下一任务或要求补证据。

---

## 22. 方案 B 成功概率提升动作（关键 7 条）

1. 先冻结边界，再动任何代码。
2. 每次只推进一个 Wave，不并发跨波次实现。
3. 只跑增量 gate，严禁全仓构建作为日常门禁。
4. 先 contract test，再写 stub，再接 adapter。
5. 每个 PR 必须可独立回滚。
6. 三方评审必须包含“反例验证”。
7. 连续失败两轮立即回退设计，不硬闯实现层。
