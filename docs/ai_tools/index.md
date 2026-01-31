# SQLCC AI 工具索引 v1.3.9

**版本**: v1.3.9  
**更新日期**: 2026-01-30  
**索引范围**: docs/ai_tools/ 目录下所有 AI 辅助开发文档

---

## 📁 文档目录

```
docs/ai_tools/
├── AI_DEVELOPMENT_GUIDELINES.md    # AI 开发规范
├── BUILD_FILE_SPECIFICATION.md     # BUILD 文件规范
├── improvement_guide.md            # 测试规范
├── systematic_refactoring_knowledge_base.md # 重构规范
├── bazel_tools_manual.md          # Bazel 工具手册
└── index.md                       # 本索引文件
```

---

## 🎯 核心规范文档

### 1. AI 开发规范

**文件**: `AI_DEVELOPMENT_GUIDELINES.md`  
**版本**: v1.3.9  
**适用范围**: 所有 AI Agent 参与 SQLCC 项目开发

**核心内容**:
- FIRST 原则 (Find First, Investigate Before Implement, Respect Existing Style, Systematic Approach, Test Everything)
- 约束条件 (C++20, Bazel 8.5.0+, Clang 20+, 智能指针强制使用)
- 禁止行为 (不读取文件直接修改代码等)
- 项目结构理解
- 开发工作流程 (问题理解 → 方案设计 → 代码实现 → 验证测试 → 文档更新)
- 编码规范 (命名规范、头文件规范、智能指针使用、异常处理、注释规范)
- 测试规范 (测试文件结构、测试标签使用、测试运行命令)
- 工具使用指南 (常用 Shell 命令、Python 工具)
- 质量门禁 (提交前检查清单、覆盖率要求)
- 故障排除 (常见问题及解决方案)

### 2. BUILD 文件规范

**文件**: `BUILD_FILE_SPECIFICATION.md`  
**版本**: v1.3.9  
**适用范围**: 所有 SQLCC 模块的 Bazel 构建配置

**核心内容**:
- 核心原则 (模块化设计、一致性优先、可维护性)
- 文件基本结构 (package、头文件导出、核心库目标、可执行文件、测试目标)
- 目标类型规范 (cc_library、cc_binary、cc_test)
- 依赖声明规范 (依赖顺序、标签格式)
- 标签路径规范 (关键规则、路径映射示例)
- 测试配置规范 (测试标签体系、测试运行命令)
- 常见错误与修复 (重复定义、无效标签、循环依赖、头文件找不到)
- 模板示例 (标准模块模板、头文件包模板、测试目录模板)
- 验证清单 (提交前检查)

### 3. 测试规范

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

### 4. 重构规范

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

### 5. Bazel 工具手册

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

## 🔗 文档关系图

```
                    ┌─────────────────────────────────────┐
                    │           AI 开发规范               │
                    │   (AI_DEVELOPMENT_GUIDELINES.md)    │
                    └─────────────┬───────────────────────┘
                                  │
                    ┌─────────────▼───────────────────────┐
                    │           BUILD 规范                 │
                    │   (BUILD_FILE_SPECIFICATION.md)     │
                    └─────────────┬───────────────────────┘
                                  │
                    ┌─────────────▼───────────────────────┐
                    │            测试规范                  │
                    │   (improvement_guide.md)            │
                    └─────────────┬───────────────────────┘
                                  │
                    ┌─────────────▼───────────────────────┐
                    │            重构规范                  │
                    │   (systematic_refactoring_knowledge_base.md) │
                    └─────────────┬───────────────────────┘
                                  │
                    ┌─────────────▼───────────────────────┐
                    │           Bazel 工具手册             │
                    │   (bazel_tools_manual.md)           │
                    └─────────────────────────────────────┘
```

---

## 📊 文档统计信息

| 文档名称 | 版本 | 文件大小 | 主要内容 |
|----------|------|----------|----------|
| AI_DEVELOPMENT_GUIDELINES.md | v1.3.9 | ~50KB | AI 开发规范 |
| BUILD_FILE_SPECIFICATION.md | v1.3.9 | ~30KB | BUILD 文件规范 |
| improvement_guide.md | v1.3.9 | ~40KB | 测试规范 |
| systematic_refactoring_knowledge_base.md | v1.3.9 | ~60KB | 重构规范 |
| bazel_tools_manual.md | v1.2.4 | ~25KB | Bazel 工具手册 |
| index.md | v1.3.9 | ~5KB | 本索引文件 |

---

## 🔧 维护说明

### 新增文档流程

1. **创建文档**: 添加到 `docs/ai_tools/<文档名>.md`
2. **更新索引**: 更新本文档的目录和统计信息
3. **版本管理**: 遵循语义化版本控制
4. **验证**: 确保文档格式正确且内容完整

### 验证命令

```bash
# 验证文档格式
find docs/ai_tools/ -name "*.md" -exec markdownlint {} \;

# 检查文档链接
markdown-link-check docs/ai_tools/*.md

# 生成文档目录
doctoc docs/ai_tools/
```

---

**维护者**: SQLCC AI 开发团队  
**最后更新**: 2026-01-30  
**版本**: v1.3.9