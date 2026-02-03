# SQLCC AI 工具索引 v1.3.11

**版本**: v1.3.11
**更新日期**: 2026-02-02
**索引范围**: docs/ai_tools/ 目录下所有 AI 辅助开发文档

---

## 🚀 多Agent跨平台协作开发 ⭐NEW

**⭐ 必读文档**: [MULTI_AGENT_COLLABORATION_GUIDE.md](./MULTI_AGENT_COLLABORATION_GUIDE.md)

这是**所有AI Agent进入SQLCC项目必须阅读的第一份文档**！

```
┌─────────────────────────────────────────────────────────────────────────┐
│              🤖 AI Agent 快速入门                                         │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                         │
│   1️⃣  阅读多Agent协作指南                                                │
│       📖 docs/ai_tools/MULTI_AGENT_COLLABORATION_GUIDE.md               │
│                                                                         │
│   2️⃣  配置Agent身份                                                     │
│       ⚡ source scripts/sqlcc-agent-config.sh <agent-id>                │
│                                                                         │
│   3️⃣  开始协作开发                                                      │
│       📝 遵循提交规范: Agent: + Scope: + Refs:                           │
│       🔀 使用feature/*分支                                                │
│       📋 提交PR前检查AI Self-Check                                       │
│                                                                         │
└─────────────────────────────────────────────────────────────────────────┘
```

**核心内容**:
- ✅ Agent身份体系（20个Agent ID）
- ✅ 协作流程（任务状态机 + 消息协议）
- ✅ 提交规范（Agent: + Scope: + Refs:）
- ✅ PR规范（强制检查项）
- ✅ 质量门禁（构建 + 覆盖率）
- ✅ 快速开始指南

---

## 📁 文档目录

```
docs/ai_tools/
├── MULTI_AGENT_COLLABORATION_GUIDE.md  # ⭐⭐⭐ AI Agent 必读 ⭐⭐⭐
├── AI_COLLABORATION_GUIDE.md           # 多Agent并行协作指南
├── AI_DEVELOPMENT_GUIDELINES.md        # AI 开发规范
├── BUILD_FILE_SPECIFICATION.md         # BUILD 文件规范 (含头文件规范)
├── CPP_DEVELOPMENT_SPECIFICATION.md    # C++ 开发、测试与重构规范
├── improvement_guide.md                # 测试规范
├── systematic_refactoring_knowledge_base.md # 重构规范
├── bazel_tools_manual.md               # Bazel 工具手册
└── index.md                            # 本索引文件

docs/sdd/                               # 规范驱动开发 (SDD)
├── SPEC_DRIVEN_DEVELOPMENT.md          # SDD 使用指南
└── templates/                          # SDD 模板
    ├── requirements_template.md        # 需求模板
    ├── design_template.md              # 设计模板
    ├── tasks_template.md               # 任务模板
    ├── verification_template.md        # 验证模板
    └── multi_agent_collaboration_template.md # 多Agent协作模板
```

---

## 🎯 核心规范文档

### ⭐⭐⭐ 多Agent协作指南 ⭐⭐⭐

**文件**: `MULTI_AGENT_COLLABORATION_GUIDE.md`
**版本**: v1.0
**适用范围**: **所有参与 SQLCC 项目的 AI Agent（必读）**

**为什么必须阅读**:
1. 定义了20个Agent身份，确保协作可追溯
2. 规定了提交规范，防止混乱提交
3. 定义了PR流程，确保代码质量
4. 提供了快速开始指南，新Agent 5分钟上手

**快速开始**:
```bash
# 1. 阅读指南
cat docs/ai_tools/MULTI_AGENT_COLLABORATION_GUIDE.md

# 2. 配置身份
source scripts/sqlcc-agent-config.sh opencode-developer

# 3. 验证
git config user.name  # 应显示: SQLCC-AI(OpenCode-Developer)
```

### 1. 多Agent协作指南

**文件**: `AI_COLLABORATION_GUIDE.md`
**版本**: v1.0
**适用范围**: 所有参与 SQLCC 项目的 AI Agent

**核心内容**:
- **Agent 角色定义**: Master, Developer, Tester, Documenter, Reviewer
- **SDD 规范遵从**: 任务状态机、消息协议、沟通频率
- **消息通信协议**: TASK_CLAIM, PROGRESS_UPDATE, BLOCKER_NOTIFICATION, TASK_COMPLETE
- **并行开发规范**: 任务并行度配置、资源冲突检测
- **任务看板管理**: TODO/IN PROGRESS/IN REVIEW/DONE/BLOCKED
- **故障处理**: Agent 故障处理、任务恢复流程

### 2. BUILD 文件规范

**文件**: `BUILD_FILE_SPECIFICATION.md`
**版本**: v1.3.10
**适用范围**: 所有 SQLCC 模块的 Bazel 构建配置

**核心内容**:
- 核心原则 (模块化设计、一致性优先、可维护性)
- **头文件引用规范** ⭐新增 (路径语法、组织结构、include 顺序、前向声明)
- 文件基本结构 (package、头文件导出、核心库目标、可执行文件、测试目标)
- 目标类型规范 (cc_library、cc_binary、cc_test)
- 依赖声明规范 (依赖顺序、标签格式)
- 标签路径规范 (关键规则、路径映射示例)
- **头文件配置规范** ⭐新增 (strip_include_prefix、导出配置)
- 测试配置规范 (测试标签体系、测试运行命令)
- 常见错误与修复 (重复定义、无效标签、循环依赖、头文件找不到)
- 模板示例 (标准模块模板、头文件包模板、测试目录模板)
- 验证清单 (提交前检查)

### 3. C++ 开发规范 ⭐新增

**文件**: `CPP_DEVELOPMENT_SPECIFICATION.md`
**版本**: v1.3.10
**适用范围**: SQLCC 项目（ Bazel + Clang-20 + C++20 ）

**核心内容**:
- **核心原则**: FIRST 原则、约束条件、禁止行为
- **开发规范**: 代码质量工具链、命名规范、语法规范、注释规范
- **测试驱动开发 (TDD)**: TDD 流程、GoogleTest 框架使用、测试原则、测试分类
- **Bazel + Clang-20 构建规范**: .bazelrc 配置、BUILD.bazel 模板、头文件引用规范
- **重构规范**: 重构原则、重构流程、重构模式、重构验证
- **质量门禁**: 提交前检查清单、覆盖率要求、测试标签体系
- **工具链集成**: 日常开发命令、代码质量工具、项目专用工具、CI/CD 集成

### 4. 测试规范

**文件**: `improvement_guide.md`
**版本**: v1.3.9
**适用范围**: 所有 SQLCC 项目的测试开发和维护

**核心内容**:
- 测试分层架构 (Level 1-7 测试体系)
- 测试文件规范 (文件命名规范、测试文件结构、测试夹具规范)
- 测试配置规范 (BUILD.bazel 配置、测试标签体系、测试运行命令)
- 测试覆盖率要求 (各层次目标覆盖率、覆盖率计算规则)
- 测试类型规范 (单元测试、集成测试、性能测试、边界测试)
- 测试工具与辅助类 (测试工具类、测试数据工厂)
- 测试质量指标 (测试覆盖率指标、测试质量指标)
- 测试禁忌 (绝对禁止、相对禁止)
- 测试支持 (文档、项目规范)

### 5. 重构规范

**文件**: `systematic_refactoring_knowledge_base.md`
**版本**: v1.3.9
**适用范围**: 所有 SQLCC 项目的重构和代码优化

**核心内容**:
- 重构核心原则 (系统性思维、渐进式实施、质量保障)
- 重构实施流程 (阶段1: 分析与规划 → 阶段2: 紧急修复 → 阶段3: 系统性重构 → 阶段4: 验证与优化)
- 问题模式识别 (Include 路径问题、依赖关系问题)
- 重构工具链 (分析工具、修复工具)
- 重构质量指标 (技术指标、质量指标、风险指标)
- AI 学习模式 (模式识别学习、决策框架学习)

### 6. Bazel 工具手册

**文件**: `bazel_tools_manual.md`
**版本**: v1.2.4
**适用范围**: SQLCC 项目 Bazel 构建系统的专用工具使用

**核心内容**:
- 工具概览 (bazel_code_checker.py、bazel_label_fixer.py、bazel_check_deps.sh、bazel_debug.sh、bazel_fixer.sh)
- 详细使用指南 (每个工具的功能特性、基本用法、高级选项、输出示例、最佳实践)
- 工具集成使用 (日常开发流程、CI/CD 集成流程、问题诊断流程)
- 性能优化建议 (工具使用优化、配置优化)
- 故障排除 (常见问题及解决方案)
- 扩展开发 (添加新的检查规则、集成新的修复工具)
- 版本信息和贡献指南

---

## 🌟 规范驱动开发 (SDD) ⭐新增

**概述**: 规范驱动开发 (Spec-Driven Development, SDD) 是一种以规范文档为核心驱动力的软件开发方法论。

### SDD 核心文档

**文件**: `docs/sdd/SPEC_DRIVEN_DEVELOPMENT.md`
**版本**: v1.3.10

**核心内容**:
- **SDD 概念**: 什么是规范驱动开发、SDD vs 传统开发、SDD 价值主张
- **SDD 核心原则**: 规范优先、渐进式精化、规范即契约、文档即代码
- **SDD 工作流程**: 标准流程、增强现有代码流程
- **SQLCC SDD 模板**: 需求模板、设计模板、任务模板
- **实施指南**: 创建新功能 SDD、模板使用流程、模板位置
- **与其他规范的关系**: 规范层级、关系图

### SDD 模板

| 模板文件 | 用途 |
|----------|------|
| `templates/requirements_template.md` | 需求规范模板 (EARS 格式) |
| `templates/design_template.md` | 架构设计模板 (含 Mermaid) |
| `templates/tasks_template.md` | 任务清单模板 (含依赖图) |

---

## 🔗 文档关系图

```
                           ┌─────────────────────────────────────┐
                           │       规范驱动开发 (SDD)            │
                           │   docs/sdd/SPEC_DRIVEN_DEVELOPMENT  │
                           └─────────────────┬───────────────────┘
                                             │
                                             ▼
┌─────────────────────────────────────────────────────────────────────────┐
│                         SQLCC 规范体系                                   │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                         │
│    ┌─────────────────────────────────────────────────────────────┐     │
│    │                    核心流程规范                              │     │
│    │  ┌───────────────────────────────────────────────────────┐  │     │
│    │  │                   SDD 规范                            │  │     │
│    │  │   需求 → 设计 → 任务 → 实现 → 验证                     │  │     │
│    │  └───────────────────────────────────────────────────────┘  │     │
│    └─────────────────────────────────────────────────────────────┘     │
│                                                                         │
│    ┌─────────────────────────────────────────────────────────────┐     │
│    │                    质量保障规范                              │     │
│    │  ┌───────────────┐  ┌───────────────┐  ┌───────────────┐   │     │
│    │  │  AI 开发规范  │  │ BUILD 规范    │  │ 测试规范      │   │     │
│    │  │   AI_        │  │   BUILD_FILE_ │  │   improve-    │   │     │
│    │  │   DEVELOP-   │  │   SPECIF-     │  │   MENT_       │   │     │
│    │  │   MENT_      │  │   ICATION     │  │   GUIDE       │   │     │
│    │  │   GUIDELINES │  │               │  │               │   │     │
│    │  └───────────────┘  └───────────────┘  └───────────────┘   │     │
│    └─────────────────────────────────────────────────────────────┘     │
│                                                                         │
│    ┌─────────────────────────────────────────────────────────────┐     │
│    │                    实施规范                                  │     │
│    │  ┌───────────────┐  ┌───────────────┐  ┌───────────────┐   │     │
│    │  │  C++ 开发规范 │  │  重构规范     │  │ Bazel 工具   │   │     │
│    │  │   CPP_       │  │   systematic_  │  │   bazel_     │   │     │
│    │  │   DEVELOP-   │  │   refactoring_ │  │   tools_     │   │     │
│    │  │   MENT_      │  │   knowledge_   │  │   manual     │   │     │
│    │  │   SPECIF-    │  │   base         │  │               │   │     │
│    │  │   ICATION    │  │               │  │               │   │     │
│    │  └───────────────┘  └───────────────┘  └───────────────┘   │     │
│    └─────────────────────────────────────────────────────────────┘     │
│                                                                         │
└─────────────────────────────────────────────────────────────────────────┘
```

---

## 📊 文档统计信息

| 文档名称 | 版本 | 主要内容 |
|----------|------|----------|
| `AI_COLLABORATION_GUIDE.md` | v1.0 | ⭐多Agent并行协作指南 |
| `AI_DEVELOPMENT_GUIDELINES.md` | v1.3.9 | AI 开发规范 |
| `BUILD_FILE_SPECIFICATION.md` | v1.3.10 | BUILD 文件规范 (含头文件规范) |
| `CPP_DEVELOPMENT_SPECIFICATION.md` | v1.3.10 | C++ 开发、测试与重构规范 |
| `improvement_guide.md` | v1.3.9 | 测试规范 |
| `systematic_refactoring_knowledge_base.md` | v1.3.9 | 重构规范 |
| `bazel_tools_manual.md` | v1.2.4 | Bazel 工具手册 |
| `SPEC_DRIVEN_DEVELOPMENT.md` | v1.3.10 | SDD 使用指南 |
| `templates/requirements_template.md` | v1.0 | 需求模板 (EARS) |
| `templates/design_template.md` | v1.0 | 设计模板 (Mermaid) |
| `templates/tasks_template.md` | v1.0 | 任务模板 |
| `templates/verification_template.md` | v1.0 | 验证模板 |
| `templates/multi_agent_collaboration_template.md` | v1.0 | 多Agent协作模板 |

---

## 🔧 维护说明

### 新增文档流程

1. **创建文档**: 添加到 `docs/ai_tools/<文档名>.md`
2. **更新索引**: 更新本文档的目录和统计信息
3. **版本管理**: 遵循语义化版本控制
4. **验证**: 确保文档格式正确且内容完整

### SDD 规范创建流程

1. **创建 SDD 目录**: `docs/sdd/features/<feature_name>/`
2. **使用模板创建文档**:
   - `docs/sdd/templates/requirements_template.md` → `requirements.md`
   - `docs/sdd/templates/design_template.md` → `design.md`
   - `docs/sdd/templates/tasks_template.md` → `tasks.md`
3. **更新总览**: 在本文档中记录新功能

### 验证命令

```bash
# 验证文档格式
find docs/ai_tools/ docs/sdd/ -name "*.md" -exec markdownlint {} \;

# 检查文档链接
markdown-link-check docs/ai_tools/*.md docs/sdd/*.md

# 生成文档目录
doctoc docs/ai_tools/
doctoc docs/sdd/
```

---

**维护者**: SQLCC AI 开发团队
**最后更新**: 2026-02-02
**版本**: v1.3.10
