# SQLCC v1.4.0 文档重构 TODO 汇总

**版本**: v1.4.0
**创建日期**: 2026-02-07
**负责人**: Codex（AI-IDE）

---

## 1. 断链修复（P0）

**目标**: `docs/index.md` 与 `docs/design/设计文档总览.md` 断链为 0

**已识别高频断链（示例）**
- `docs/index.md`
  - `ai_tools/AI_TOOLS_USAGE_GUIDE.md`
  - `releases/RELEASE_NOTES_v1.3.5.md`
  - `releases/RELEASE_NOTES_v1.3.4.md`
  - `releases/RELEASE_NOTES_v1.3.2.md`
  - `releases/RELEASE_NOTES_v1.3.1.md`
  - `ai_tools/bazel_knowledge_base.md`
  - `ai_tools/bazel_workflow_guide.md`
  - `ai_tools/bazel_improvement_guide.md`
  - `testing/测试框架改进和性能测试扩展计划.md`
  - `testing/coverage/`
  - `project/versions/v1.3.5/`
  - `project/versions/v1.2.9/`
  - `textbook/教材提纲.md`
  - `textbook/*.pptx`
- `docs/design/设计文档总览.md`
  - 多个指向 `docs/design/...` 的路径不符合当前目录层级

**验收**
- 核心入口断链为 0
- 其余断链分批修复（目标 <= 100）

---

## 2. 设计文档现状/规划标注（P0）

**目标**: 设计文档必须明确“现状已实现 vs 规划未实现”

**最小要求**
- 设计文档首段加入“现状/规划”标识块
- 对未实现模块加“规划/占位”标注

---

## 3. v1.4.0 相关 TODO 汇总（P1）

**来源（已发现）**
- `CORE_STORAGE_DEPENDENCY_ANALYSIS.md`
  - [ ] 更新 TODO.md
- `ISSUE_001.md`
  - [ ] TODO 完成状态更新
- `GEMINI_REVIEW_REPORT.md`
  - 内容待补充
- `GEMINI_STRICT_REVIEW.md`
  - 内容待补充

**动作**
- 将以上 TODO 汇总到本文件
- 统一标注优先级与验收标准

---

## 4. 版本边界整理（P1）

**要求**
- v1.4.x: 编译基线 + 解耦
- v1.5.x: 功能补充与扩展

---

## 5. 后续建议

- 生成断链报告（自动脚本）
- 建立“文档一致性检查”任务
- 将文档修复任务加入 v1.4.0 任务清单

---

**维护者**: Codex（AI-IDE）
