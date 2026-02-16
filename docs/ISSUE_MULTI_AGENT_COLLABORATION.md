# SQLCC 多Agent跨平台协作规范（Issue/PR驱动）

**版本**: 1.1  
**日期**: 2026-02-04  
**适用范围**: 所有参与 SQLCC 项目的 AI Agent 与人类开发者

---

## 1. 目标与范围

本规范用于统一 SQLCC 的多 Agent 协作方式，确保：

- ✅ 任务分解清晰、责任可追踪
- ✅ 沟通路径统一（Issue/PR/文档）
- ✅ 文档与代码同步更新
- ✅ 交付质量可控

---

## 2. 负责人体系

### 2.1 项目负责人（平级）

当前项目负责人为平级协作关系：

- **OpenClaw 高小原**（维护者）
- **Codex 项目负责人**（本仓库的协作负责人，ChatGPT Codex）

### 2.2 Codex 身份定义

**身份**: SQLCC 文档与流程协作负责人（Codex）  
**职责**:
- 维护与同步多 Agent 协作规范
- 协助修正文档一致性与质量问题
- 通过 Issue/PR/文档记录变更与决策

**当前已执行的变更**:
- 新增本协作规范文档并纳入 AGENTS.md 的 P0 列表
- 标注本文件中 Codex 的身份与职责

**沟通约束**：
由于平台隔离，双方**无法直接即时沟通**。所有同步需通过以下渠道完成：

1. **Issue**（优先）：需求澄清、任务分配、阻塞与决策记录  
2. **PR**：实现变更、评审意见与合并记录  
3. **文档**：规范、计划、验收记录的异步更新

---

## 3. 协作流程（Issue 驱动）

1. **创建 Issue**  
   - 需求/变更/缺陷必须先创建 Issue  
   - Issue 必须包含背景、目标、验收标准  

2. **任务分解与认领**  
   - Master 负责拆分任务  
   - Developer 认领任务并更新状态  

3. **实现与验证**  
   - 按 SDD 流程执行  
   - 测试与覆盖率报告要可追溯  

4. **PR 评审与合并**  
   - Reviewer 进行规范检查  
   - 评审通过后合并

---

## 4. 角色与职责

| 角色 | 职责 |
|------|------|
| **Master** | 任务分解、进度跟踪、最终交付 |
| **Developer** | 代码实现、单元测试 |
| **Tester** | 测试执行、覆盖率分析 |
| **Documenter** | 文档编写、SDD维护 |
| **Reviewer** | 代码评审、规范检查 |

---

## 5. 状态机与消息协议

任务状态机：
`OPEN → CLAIMED → WIP → DONE → FROZEN`

消息协议：
`TASK_CLAIM / PROGRESS_UPDATE / BLOCKER_NOTIFICATION / TASK_COMPLETE`

---

## 6. 质量门禁

- 编译通过 (`bazel build //...`)
- 测试通过 (`bazel test //...`)
- 覆盖率达标（按项目标准）
- 文档完整（requirements/design/tasks/verification）
- CHANGELOG 更新

---

## 7. 与其他规范的关系

- SDD 规范：`docs/sdd/SPEC_DRIVEN_DEVELOPMENT.md`
- AI 协作指南：`docs/ai_tools/AI_COLLABORATION_GUIDE.md`
- 多 Agent 协作指南：`docs/ai_tools/MULTI_AGENT_COLLABORATION_GUIDE.md`

---

**维护者**: OpenClaw 高小原 / Codex 项目负责人  
**最后更新**: 2026-02-04
