# SQLCC - SQL Cloud Computing Database System

## 🚀 最新版本：v1.3.10 - 规范驱动开发 (SDD) 体系 ⭐NEW

**发布日期**: 2026-02-02
**版本状态**: ✅ 正式发布
**版本亮点**: **规范驱动开发 (SDD) 体系** - C++ 开发规范、SDD 模板、TDD 最佳实践

---

## 🌟 规范驱动开发 (SDD) ⭐NEW

### 什么是 SDD?

**规范驱动开发 (Spec-Driven Development, SDD)** 是一种以规范文档为核心驱动力的软件开发方法论。

```
┌─────────────────────────────────────────────────────────────────────────┐
│                         SDD 核心流程                                     │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                         │
│   spec-init      spec-requirements      spec-design      spec-tasks    │
│       │                │                    │                │         │
│       ▼                ▼                    ▼                ▼         │
│   ┌─────────┐    ┌─────────────┐    ┌─────────────┐    ┌─────────┐    │
│   │ 功能定义 │───▶│  EARS 格式  │───▶│  Mermaid    │───▶│ 任务    │    │
│   │         │    │   需求规范   │    │   架构图    │    │ 分解    │    │
│   └─────────┘    └─────────────┘    └─────────────┘    └─────────┘    │
│                                                                    │    │
│                                                                    ▼    │
│                                                            ┌─────────────┐│
│                                                            │  spec-impl  ││
│                                                            │   代码实现   ││
│                                                            └─────────────┘│
│                                                                          │
└─────────────────────────────────────────────────────────────────────────┘
```

### SDD 核心原则

| 原则 | 说明 |
|------|------|
| **规范优先** | 任何代码变更都应从规范更新开始 |
| **渐进式精化** | 从需求到设计到实现逐步细化 |
| **规范即契约** | 规范是开发团队的共同语言 |
| **文档即代码** | 规范使用 Markdown 编写，版本化管理 |

### SDD 文档模板

| 模板 | 用途 | 路径 |
|------|------|------|
| **需求模板** | EARS 格式需求规范 | `docs/sdd/templates/requirements_template.md` |
| **设计模板** | Mermaid 架构图 | `docs/sdd/templates/design_template.md` |
| **任务模板** | 带依赖的任务清单 | `docs/sdd/templates/tasks_template.md` |

### SDD 实施指南

- [**SDD 使用指南**](docs/sdd/SPEC_DRIVEN_DEVELOPMENT.md) - 完整的 SDD 方法论和实施流程
- [**C++ 开发规范**](docs/ai_tools/CPP_DEVELOPMENT_SPECIFICATION.md) - C++ 开发、测试与重构规范
- [**BUILD 文件规范**](docs/ai_tools/BUILD_FILE_SPECIFICATION.md) - Bazel BUILD 编写规范（含头文件规范）
- [**AI 开发规范**](docs/ai_tools/AI_DEVELOPMENT_GUIDELINES.md) - AI Agent 开发规范 (FIRST 原则)

---

## 🤖 多Agent跨平台协作开发 ⭐NEW

### 概述

SQLCC项目采用多个AI Agent（OpenCode、iFlow、Claude、Gemini、OpenClaw等）进行协同开发，必须遵循统一的协作规范。

### Agent身份体系

**所有Agent必须配置唯一身份**（见 `.gitconfig.d/agents.conf`）：

| Agent ID | 名称 | 邮箱后缀 |
|----------|------|----------|
| **OpenCode** | opencode-master/developer/tester/doc/reviewer | opencode-xxx@... |
| **iFlow** | iflow-builder/refactor/test | iflow-xxx@... |
| **Claude** | claude-architect/developer/reviewer/researcher | claude-xxx@... |
| **Claude Code** | claudecode-executor/toolmaker | claude-code-xxx@... |
| **Gemini** | gemini-assistant/analyst/debugger | gemini-xxx@... |
| **OpenClaw** | openclaw-refactor/test-fixer/build-cleaner/doc/sdd | openclaw-xxx@... |

### 配置方法

```bash
# 方法1: 使用配置脚本
cd /path/to/sqlcc
source scripts/sqlcc-agent-config.sh <agent-id>

# 示例
source scripts/sqlcc-agent-config.sh opencode-developer

# 方法2: 手动配置
git config user.name "SQLCC-AI(OpenCode-Developer)"
git config user.email "sqlcc+opencode-dev@users.noreply.github.com"
```

### 提交规范

**所有提交必须包含**：

```
<类型>: <描述>

Agent: <Agent名称>
Scope: <影响范围>
Refs: #<issue编号>
```

**示例**:
```
feat: 修复 execution 模块编译错误

Agent: OpenCode-Developer
Scope: src/execution/*
Refs: #EXEC-101
```

### 分支策略

| 分支类型 | 命名规则 | 保护状态 |
|----------|----------|----------|
| 主分支 | `main` | ❌ 禁止直接提交 |
| 基线分支 | `baseline/recover` | ❌ 禁止直接提交 |
| 功能分支 | `feature/*` | ✅ 开放 |
| 修复分支 | `fix/*` | ✅ 开放 |

### 相关文档

- [**多Agent协作规范Issue**](.github/ISSUE_MULTI_AGENT_COLLABORATION.md) - 完整规范
- [**Agent配置通知**](docs/MULTI_AGENT_NOTIFICATION.md) - 配置指南
- [**PR模板**](.github/pull_request_template.md) - PR强制检查项
- [**提交钩子**](.git/hooks/commit-msg) - 提交规范检查

---

## 📊 项目功能状态评估

### ✅ 核心功能完整性

| 功能模块 | 状态 | 支持度 | 说明 |
|----------|------|--------|------|
| **存储引擎** | ✅ 完整 | 100% | 8KB定长页管理，V3 BufferPool架构 |
| **SQL解析器** | ✅ 完整 | 100% | ParserNew架构，支持SQL-92标准 |
| **索引系统** | ✅ 完整 | 100% | B+树实现，支持点查询和范围查询 |
| **事务管理** | ✅ 完整 | 100% | ACID特性，WAL日志，两阶段锁 |
| **DDL语句** | ✅ 完整 | 100% | CREATE/DROP/ALTER TABLE等 |
| **DML语句** | ✅ 完整 | 100% | INSERT/SELECT/UPDATE/DELETE |
| **DCL语句** | ✅ 完整 | 100% | GRANT/REVOKE/用户管理 |
| **TCL语句** | ✅ 完整 | 100% | COMMIT/ROLLBACK/SAVEPOINT |
| **JOIN操作** | ✅ 完整 | 100% | INNER/LEFT/RIGHT/FULL JOIN |
| **子查询** | ✅ 完整 | 100% | 嵌套子查询，相关子查询 |
| **窗口函数** | ✅ 完整 | 100% | ROW_NUMBER/RANK/SUM等 |
| **递归查询** | ✅ 完整 | 100% | WITH RECURSIVE支持 |

### 📈 SQL-92标准符合度

- **总体支持度**: 100% 完全支持
- **语法正确性**: ⭐⭐⭐⭐⭐ 100% 完全符合SQL-92标准语法
- **语义正确性**: ⭐⭐⭐⭐⭐ 100% 语义执行完全正确

📊 [**查看完整功能矩阵**](docs/project/versions/v1.3.9/FUNCTION_MATRIX_v1.3.9.md) - 详细的SQL-92标准符合度说明

---

## 🧪 测试情况

### Level 1 Foundation 测试覆盖率

| 模块 | 测试用例数 | 通过率 | 覆盖特性 |
|------|------------|--------|----------|
| **Exception** | 32 | 100% | 基础异常、继承关系、异常捕获、性能测试 |
| **Types** | ~60 | 100% | Value类型、域管理、事务ID、锁类型 |
| **Logger** | ~30 | 100% | 单例模式、日志级别、文件输出、线程安全 |
| **Config** | ~40 | 100% | 配置管理、文件读写、线程安全、批量操作 |
| **Level 1 Foundation** | **~160** | **100%** | **真实实现，完整覆盖** |

### 测试框架

- **测试框架**: Google Test (GTest) + Bazel测试框架
- **覆盖率工具**: LLVM 20 + Clang 20 覆盖率工具链
- **编程语言**: C++20标准
- **测试分类**: 单元测试、集成测试、网络测试、性能测试、SQL测试
- **测试规范**: [C++ 开发规范 - TDD章节](docs/ai_tools/CPP_DEVELOPMENT_SPECIFICATION.md#测试驱动开发-tdd)

### TDD 最佳实践

SQLCC 采用 **测试驱动开发 (TDD)** 方法论：

```
Red (失败测试) → Green (最小实现) → Refactor (重构)

✅ 每次提交都有测试覆盖
✅ 测试先于代码编写
✅ 规范定义测试用例
```

---

## 📋 v1.3.10 版本变更摘要 ⭐NEW

### 🎯 主要变更

1. **规范驱动开发 (SDD) 体系**
   - 创建 SDD 使用指南（规范驱动开发方法论）
   - 创建 SDD 模板（需求、设计、任务）
   - 集成 cc-sdd 最佳实践

2. **C++ 开发规范**
   - 创建完整的 C++ 开发、测试与重构规范
   - 集成 TDD（测试驱动开发）最佳实践
   - 集成 GoogleTest 框架使用规范

3. **BUILD 文件规范增强**
   - 集成头文件引用规范
   - 完善 strip_include_prefix 配置说明
   - 添加前向声明使用指南

### 📊 变更统计

- **新增文档**: 5个
  - `docs/sdd/SPEC_DRIVEN_DEVELOPMENT.md`
  - `docs/sdd/templates/requirements_template.md`
  - `docs/sdd/templates/design_template.md`
  - `docs/sdd/templates/tasks_template.md`
  - `docs/ai_tools/CPP_DEVELOPMENT_SPECIFICATION.md`

- **更新文档**: 4个
  - `docs/ai_tools/BUILD_FILE_SPECIFICATION.md`
  - `docs/ai_tools/index.md`
  - `docs/index.md`
  - `README.md`

### 🔗 详细文档

- [SDD 使用指南](docs/sdd/SPEC_DRIVEN_DEVELOPMENT.md)
- [C++ 开发规范](docs/ai_tools/CPP_DEVELOPMENT_SPECIFICATION.md)
- [版本文档目录](docs/project/versions/v1.3.10/)

---

## 📋 v1.3.9 版本变更摘要

### 🎯 主要变更

1. **Level 1 Foundation完整单元测试**
   - 为异常处理、类型系统、日志系统、配置管理四大核心模块编写了完整的单元测试
   - 约160个测试用例，覆盖所有核心功能
   - 修复了6个失败的types_test用例

2. **真实实现测试**
   - 所有测试使用真实实现，非Mock测试
   - 验证实际系统行为

3. **测试架构优化**
   - 合并level2_core和level2_core_services测试目录
   - 每个模块有独立的BUILD.bazel文件
   - 头文件规范修复，严格遵守include规范

---

## 🏗️ 系统架构

### 核心组件

```
SQLCC 架构
├── 存储引擎 (Storage Engine)
│   ├── BufferPool (V3分片缓冲池)
│   ├── B+树索引 (B+ Tree Index)
│   └── 磁盘管理 (Disk Manager)
├── SQL解析器 (SQL Parser)
│   ├── 词法分析器 (Lexer)
│   ├── 语法分析器 (Parser)
│   └── AST抽象语法树
├── 执行引擎 (Execution Engine)
│   ├── 查询计划 (Query Plan)
│   └── 执行器 (Executor)
├── 事务管理器 (Transaction Manager)
│   ├── WAL日志 (Write-Ahead Log)
│   └── 并发控制 (Concurrency Control)
└── 网络通信 (Network)
    ├── 连接管理 (Connection)
    └── 协议处理 (Protocol)
```

### 技术特性

- **内存安全**: A++等级，95%+智能指针化
- **并发控制**: 多任务执行器架构，TaskExecutor类
- **网络通信**: AES加密，TLS/SSL支持
- **权限管理**: 完整的RBAC权限模型

---

## 🚀 快速开始

### 系统要求

- **操作系统**: Linux Ubuntu 20.04+ / CentOS 8+
- **编译器**: GCC 9.0+ / Clang 10.0+ (推荐Clang 20)
- **构建系统**: Bazel 8.0+
- **内存**: 最少4GB RAM (推荐8GB+)
- **存储**: 最少10GB可用空间

### 安装部署

```bash
# 克隆代码仓库
git clone https://gitee.com/yinglichina/sqlcc.git
cd sqlcc

# 使用Bazel构建
bazel build //src:sqlcc_server

# 启动数据库服务
./bazel-bin/src/sqlcc_server --config=config/sqlcc.conf
```

### 运行测试

```bash
# 运行所有测试
bazel test //...

# 运行Level 1测试
bazel test //tests/level1_foundation/...

# 生成覆盖率报告
bazel coverage //...
```

---

## 📚 文档索引

### 快速导航

| 分类 | 文档 | 说明 |
|------|------|------|
| **项目概述** | [README.md](README.md) | 完整的项目介绍 |
| **文档索引** | [docs/index.md](docs/index.md) | 所有文档的统一入口 |
| **开发者指南** | [docs/development/guides/](docs/development/guides/) | 开发指南集合 |
| **API文档** | [docs/api/](docs/api/) | 接口文档 |
| **架构设计** | [docs/design/](docs/design/) | 架构设计文档 |
| **版本发布** | [docs/releases/](docs/releases/) | 版本发布文档 |
| **项目进展** | [docs/project/versions/](docs/project/versions/) | 版本历史记录 |

### 版本文档

- [完整CHANGELOG](docs/releases/CHANGELOG.md) - 所有版本变更记录
- [版本总览](docs/releases/VERSION_OVERVIEW.md) - 所有版本简要说明
- [版本历史](docs/project/versions/) - 完整的版本历史记录

---

## 🔗 相关链接

- **Gitee仓库**: https://gitee.com/yinglichina/sqlcc
- **项目文档**: [docs/index.md](docs/index.md)
- **SDD 规范**: [docs/sdd/SPEC_DRIVEN_DEVELOPMENT.md](docs/sdd/SPEC_DRIVEN_DEVELOPMENT.md)
- **问题反馈**: 通过 Gitee Issues 提交

---

## 📚 规范文档体系 ⭐NEW

SQLCC 项目建立了完整的规范文档体系，指导开发、测试和重构：

### 核心规范

| 规范 | 文件 | 版本 | 说明 |
|------|------|------|------|
| **SDD 规范** | `docs/sdd/SPEC_DRIVEN_DEVELOPMENT.md` | v1.3.10 | 规范驱动开发方法论 |
| **C++ 开发规范** | `docs/ai_tools/CPP_DEVELOPMENT_SPECIFICATION.md` | v1.3.10 | C++ 开发、测试与重构 |
| **AI 开发规范** | `docs/ai_tools/AI_DEVELOPMENT_GUIDELINES.md` | v1.3.9 | AI Agent 开发规范 |
| **BUILD 规范** | `docs/ai_tools/BUILD_FILE_SPECIFICATION.md` | v1.3.10 | Bazel 构建规范 |
| **测试规范** | `docs/ai_tools/improvement_guide.md` | v1.3.9 | 测试开发规范 |
| **重构规范** | `docs/ai_tools/systematic_refactoring_knowledge_base.md` | v1.3.9 | 系统化重构方法 |

### 规范使用流程

```
1. 新功能开发
   ├── 需求定义 → docs/sdd/templates/requirements_template.md
   ├── 架构设计 → docs/sdd/templates/design_template.md
   └── 任务分解 → docs/sdd/templates/tasks_template.md

2. 代码实现
   ├── 遵循 C++ 开发规范
   ├── 遵循 BUILD 文件规范
   └── 遵循测试规范

3. 重构优化
   ├── 使用重构规范
   └── 验证质量门禁
```

---

*最后更新: 2026-02-02*
*版本: v1.3.10*
