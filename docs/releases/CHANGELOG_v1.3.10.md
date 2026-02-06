# SQLCC v1.3.10 变更记录 (ChangeLog)

## 📅 发布日期：2026-02-02


### 🌟 规范驱动开发 (SDD) 体系 ✅ COMPLETED

#### 新增 SDD 核心文档 ✅ COMPLETED

##### SDD 使用指南 ✅ COMPLETED
- **文件**: `docs/sdd/SPEC_DRIVEN_DEVELOPMENT.md`
- **内容**:
  - SDD 核心概念和原则
  - SDD 工作流程 (spec-init → spec-requirements → spec-design → spec-tasks → spec-impl)
  - 与其他规范的关系和层级
  - 最佳实践

##### SDD 模板 ✅ COMPLETED
- **需求模板**: `docs/sdd/templates/requirements_template.md`
  - EARS 格式需求定义
  - 测试用例定义
  - 验收标准

- **设计模板**: `docs/sdd/templates/design_template.md`
  - 架构决策记录 (ADR)
  - Mermaid 图表 (类图、时序图、状态图)
  - BUILD 配置示例

- **任务模板**: `docs/sdd/templates/tasks_template.md`
  - 任务列表和依赖关系
  - 燃尽图数据
  - 风险跟踪

#### 新增 C++ 开发规范 ✅ COMPLETED

##### C++ 开发、测试与重构规范 ✅ COMPLETED
- **文件**: `docs/ai_tools/CPP_DEVELOPMENT_SPECIFICATION.md`
- **版本**: v1.3.10
- **内容**:
  - FIRST 原则和约束条件
  - 代码质量工具链
  - 命名规范和语法规范
  - 测试驱动开发 (TDD) 流程
  - GoogleTest 框架使用
  - Bazel + Clang-20 构建规范
  - 重构原则和模式
  - 质量门禁和覆盖率要求

#### BUILD 文件规范增强 ✅ COMPLETED

##### 头文件引用规范 ✅ COMPLETED
- **文件**: `docs/ai_tools/BUILD_FILE_SPECIFICATION.md`
- **新增内容**:
  - 头文件路径语法（逻辑路径 vs 相对路径）
  - 头文件组织结构
  - Include 顺序规范
  - 前向声明使用
  - strip_include_prefix 配置
  - 头文件依赖原则

#### 文档索引更新 ✅ COMPLETED

##### AI 工具索引更新 ✅ COMPLETED
- **文件**: `docs/ai_tools/index.md`
- **更新**:
  - 添加 C++ 开发规范索引
  - 添加 SDD 规范索引
  - 更新文档关系图

##### 项目文档索引更新 ✅ COMPLETED
- **文件**: `docs/index.md`
- **更新**:
  - 添加 SDD 规范链接
  - 更新文档架构
  - 添加规范文档链接

##### README 更新 ✅ COMPLETED
- **文件**: `README.md`
- **更新**:
  - 版本更新到 v1.3.10
  - 添加 SDD 概念介绍
  - 添加规范文档体系
  - 添加 TDD 最佳实践

---
