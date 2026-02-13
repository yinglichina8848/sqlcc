# S05R SDD-TDD 任务拆分（可直接执行）

**范围**: Level2 Core  
**路线**: 主线 `R2` + 试验线 `R3` + 应急线 `R1`

## 1. 主线 R2（仓内并行轨道）

### 阶段 S1: SPEC_REVIEW
1. `R2-S1-REQ`: 需求与边界冻结
2. `R2-S1-DES`: 接口与错误模型设计
3. `R2-S1-TASKS`: 任务与依赖图冻结

Gate:
1. 文档齐全（requirements/design/tasks）
2. 三方评审通过

### 阶段 S2: TEST_REVIEW
1. `R2-S2-CT`: 契约测试清单
2. `R2-S2-RT`: 最小回归测试清单

Gate:
1. 测试映射接口（1:1）

### 阶段 S3: WIP（增量实现）
1. `R2-S3-I`: `interfaces_v2`
2. `R2-S3-S`: `impl_stub_v2`
3. `R2-S3-A`: `adapters_v2`
4. `R2-S3-Rn`: `real_impl_stepN`

Gate:
1. `bazel build //src/core/interfaces_v2:interfaces_v2`
2. `bazel build //src/core/impl_stub_v2:impl_stub_v2`
3. `bazel test //tests/level2_core_v2/contract:all`
4. `bazel build //src/core/adapters_v2:adapters_v2`

## 2. 试验线 R3（教学重建）

### 阶段 E1: 教学目标定义
1. `R3-E1-LEARN-OBJECTIVE`
2. `R3-E1-MVP-SCOPE`

### 阶段 E2: 从零最小实现
1. `R3-E2-CORE-SKELETON`
2. `R3-E2-CONTRACT-TEST`
3. `R3-E2-MINIMAL-RUN`

规则:
1. R3 与 R2 证据链隔离（独立 issue/分支/报告）
2. 不反向阻塞 R2 主线

## 3. 应急线 R1（短期补丁）

### 使用条件
1. 仅当 R2 主线出现紧急阻塞。
2. 必须定义失效时间和回滚点。

### 禁止项
1. 不允许演变为长期主线。
2. 不允许跨层扩散。

## 4. 人机协作分工（最省力版）
1. Codex: 拆分任务、审计门禁、发纠偏指令
2. Claude/OpenCode/Gemini: 执行与复核
3. Owner: 只做路线决策与最终确认

