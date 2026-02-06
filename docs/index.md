# SQLCC 文档索引

## 📖 关于本项目

SQLCC（SQL Cloud-native Cluster）是一个企业级内存安全的云原生数据库系统，实现了完整SQL-92标准支持和高性能存储引擎。

## 👥 维护者与协作

- **项目维护者（平级）**：OpenClaw 高小原 / Codex 项目负责人
- **协作规范**：[多Agent跨平台协作规范（Issue/PR驱动）](ISSUE_MULTI_AGENT_COLLABORATION.md)

## 🚀 快速开始

### 🆕 新用户入门 (推荐)
- [**项目概述 (README.md)**](../README.md) - 完整的项目介绍、快速开始和验收标准
- [**开发者快速开始指南**](development/guides/quick_start_developer.md) - 环境搭建、构建、测试、调试全流程
- [**用户指南**](../README.md) - SQLCC的基本使用方法

### 📋 安装和部署
- [**安装指南**](../README.md) - 系统依赖和安装步骤
- [**开发环境设置**](development/guides/DEVELOPMENT_ENVIRONMENT_SETUP.md) - Ubuntu C++环境配置
- [**构建和测试指南**](development/guides/BUILD_AND_TEST_GUIDE.md) - Bazel构建说明

### 🤖 AI辅助开发
- [**AI工具索引**](ai_tools/index.md) - AI工具与规范入口
- [**AI开发原则**](development/ai_development_principles.md) - AI辅助软件工程理念
- [**AI开发规范 ⭐新增**](ai_tools/AI_DEVELOPMENT_GUIDELINES.md) - AI Agent 开发规范 (FIRST 原则)
- [**🤖 多Agent跨平台协作开发 ⭐⭐⭐必读⭐⭐⭐**](ai_tools/MULTI_AGENT_COLLABORATION_GUIDE.md) - **所有AI Agent进入项目必须阅读的第一份文档！**
- [**Issue/PR协作规范 ⭐新增**](ISSUE_MULTI_AGENT_COLLABORATION.md) - 多Agent协作的Issue/PR驱动流程
- [**C++开发规范 ⭐新增**](ai_tools/CPP_DEVELOPMENT_SPECIFICATION.md) - C++ 开发、测试与重构规范
- [**测试驱动开发**](development/guides/TEST_DRIVEN_DEVELOPMENT_GUIDE.md) - TDD理念与实践
- [**TDD 开发指南（项目版）**](TDD_GUIDE.md) - TDD流程与最小覆盖矩阵要求
- [**TDD/SDD 联合框架规范**](TDD_SDD_FRAMEWORK.md) - 规范驱动与测试驱动的联动流程
- [**规范驱动开发 (SDD) ⭐新增**](sdd/SPEC_DRIVEN_DEVELOPMENT.md) - 规范驱动开发方法论
- [**FDD 工作流规范**](fdd/FDD_WORKFLOW.md) - 功能驱动开发流程
- [**FDD 功能分解树**](fdd/FEATURE_DECOMPOSITION.md) - 功能域与功能点清单
- [**FDD 优先级矩阵**](fdd/FEATURE_PRIORITY_MATRIX.md) - 功能优先级与迭代规划

## 🏗️ 架构与设计

### 📐 系统架构
- [**系统架构总览**](design/Architecture.md) - 完整的系统架构说明
- [**架构文档索引**](architecture/DOCUMENT_INDEX.md) - 架构文档入口与阅读路线
- [**架构新手指南**](architecture/BEGINNER_GUIDE.md) - 架构入门与关键概念导航
- [**存储引擎架构**](design/storage_engine/storage_engine_redesign.md) - 存储引擎设计详解
- [**配置管理架构**](design/config_manager/) - 配置系统设计
- [**多任务执行器设计**](design/execution/) - 并发执行架构
- [**缓冲池设计**](design/storage_engine/buffer_pool.md) - 缓存策略详解

### 🧩 核心组件设计
- [**SQL解析器设计**](design/sql_parser/) - 解析器架构说明
- [**SQL执行器设计**](design/sql_executor/) - 执行器架构说明
- [**包设计规范**](design/) - 模块化设计原则

### 🔒 安全与性能
- [**安全架构设计**](design/security/) - 安全系统设计
- [**性能设计**](design/performance/) - 性能优化方案
- [**分布式架构**](design/distributed/) - 分布式设计
- [**部署架构**](design/deployment/) - 部署方案设计

## 📋 功能特性

### ✅ 功能实现状态
- [**功能矩阵和实现状态**](project/versions/v1.3.9/FUNCTION_MATRIX_v1.3.9.md) - SQL-92标准100%符合度(v1.3.9)
- [**功能矩阵历史**](project/versions/v1.2.0/FUNCTION_MATRIX_v1.2.0.md) - v1.2.0版本功能清单
- [**版本功能对比**](releases/version_summary_v1.2.3.md) - 各版本特性总览

### 🚀 最新版本特性
- [**v1.3.9 Release Notes**](releases/RELEASE_NOTES_v1.3.9.md) - Level 1 Foundation完整单元测试
- [**v1.3.8 Release Notes**](releases/RELEASE_NOTES_v1.3.8.md) - SQL Parser模块化重构
- [**v1.3.7 Release Notes**](releases/RELEASE_NOTES_v1.3.7.md) - Bazel构建系统重构
- [**v1.3.6 Release Notes**](releases/RELEASE_NOTES_v1.3.6.md) - LLVM覆盖率工具链完善
- [**v1.3.3 Release Notes**](releases/RELEASE_NOTES_v1.3.3.md) - DDL/DCL功能补全
- [**v1.3.0 Release Notes**](releases/RELEASE_NOTES_v1.3.0.md) - 功能发布
- [**v1.2.x系列**](releases/) - v1.2.0-v1.2.15版本特性

## 🔧 API文档

### 📚 接口文档
- [**API概览**](api/README.md) - 接口文档导航
- [**API类文档**](api/classes/) - 类文档集合
- [**编码标准**](api/code/coding_standards.md) - 代码规范
- [**源码注释指南**](api/code/source_code_comments_guide.md) - 注释规范
- [**API使用案例**](api/examples/) - 使用案例

## 📚 代码与开发

### 💻 编码规范
- [**编码标准**](api/code/coding_standards.md) - 代码规范和最佳实践
- [**源码注释指南**](api/code/source_code_comments_guide.md) - Why-What-How三层注释体系

### 🛠️ 开发工具
- [**Doxygen配置**](development/guides/DOXYGEN_COVERAGE_CONFIGURATION.md) - 文档生成工具
- [**Bazel工具手册**](ai_tools/bazel_tools_manual.md) - 构建工具使用
- [**开发指南**](development/guides/) - 开发指南集合

### 📐 开发规范 ⭐新增
- [**SDD 模板 - 需求规范**](sdd/templates/requirements_template.md) - EARS 格式需求模板
- [**SDD 模板 - 架构设计**](sdd/templates/design_template.md) - Mermaid 图表设计模板
- [**SDD 模板 - 任务清单**](sdd/templates/tasks_template.md) - 带依赖的任务模板
- [**BUILD文件规范**](ai_tools/BUILD_FILE_SPECIFICATION.md) - Bazel BUILD 编写规范
- [**重构知识库**](ai_tools/systematic_refactoring_knowledge_base.md) - 系统化重构方法

## 🚀 项目管理

### 📅 版本规划
- [**产品路线图**](project/versions/) - 长期发展规划
- [**版本总览**](releases/VERSION_OVERVIEW.md) - 所有历史版本
- [**版本摘要**](releases/VERSION_SUMMARY.md) - 版本演进总结
- [**版本详情**](releases/VERSION_DETAILS.md) - 完整版本信息

### 📊 项目进展
- [**项目进展总览**](project/) - 整体项目状态
- [**项目组织计划**](project/project_organization_plan.md) - 项目管理架构
- [**项目总结**](project/PROJECT_SUMMARY.md) - 阶段性成果
- [**SQL命令支持评估**](project/sql_command_support_evaluation.md) - SQL特性评估

### 📋 项目计划
- [**实施计划**](project/plans/) - 各版本实施计划

## 👥 社区与贡献

### 🤝 贡献指南
- [**贡献者指南**](development/guides/CONTRIBUTING.md) - 如何参与贡献
- [**安全政策**](development/guides/SECURITY.md) - 安全问题报告
- [**开发工作流**](ai_tools/bazel_tools_manual.md) - 协作开发流程

### 🔧 工具和方法论
- [**AI重构方法论**](ai_tools/systematic_refactoring_knowledge_base.md) - 系统化重构方法
- [**测试体系概览**](testing/README.md) - 测试文档入口

## 🔍 测试与质量

### 🧪 测试体系
- [**测试文档**](testing/) - 测试文档集合
- [**测试质量评估**](reports/evaluation/) - 测试质量评估报告

### 🎯 质量保证
- [**内存安全培训**](design/security/memory_safety_training.md) - 安全编码培训
- [**测试改进计划**](testing/README.md) - 质量提升计划

## 📊 性能与监控

### ⚡ 性能分析
- [**性能评估报告**](reports/evaluation/) - 性能测试结果
- [**CRUD性能测试**](reports/evaluation/) - 详细性能数据
- [**真实性能测试**](reports/evaluation/) - 实际环境测试

### 📈 监控工具
- [**编译时间基准**](../scripts/benchmark_compile_time.sh) - 构建性能监控
- [**覆盖率分析工具**](../scripts/analyze_module_coverage.sh) - 代码质量监控

## 🔒 安全与合规

### 🛡️ 安全体系
- [**内存安全审计报告**](reports/evaluation/) - 安全审计结果
- [**安全架构设计**](design/security/) - 安全系统设计

## 📝 项目进展记录

### 🎯 最新版本 (v1.3.x)
- [**v1.3.9 版本报告**](project/versions/v1.3.9/) - Level 1 Foundation完整单元测试
- [**v1.3.8 版本报告**](project/versions/v1.3.8/) - SQL Parser模块化重构
- [**v1.3.7 版本报告**](project/versions/v1.3.7/) - Bazel构建系统重构
- [**v1.3.6 版本报告**](project/versions/v1.3.6/) - LLVM覆盖率工具链完善
- [**v1.3.4 版本报告**](project/versions/v1.3.4/) - 版本更新和文档完善
- [**v1.3.4 版本报告**](project/versions/v1.3.4/) - SQL-92特性深度集成
- [**v1.3.3 版本报告**](project/versions/v1.3.3/) - DDL/DCL功能补全
- [**v1.3.2 版本报告**](project/versions/v1.3.2/) - DDL语句真实执行验证
- [**v1.3.1 版本报告**](project/versions/v1.3.1/) - 测试改进
- [**v1.3.0 版本报告**](project/versions/v1.3.0/) - 功能发布
- [**v1.3.x系列**](project/versions/) - v1.3.0-v1.3.9版本特性

### 🚀 v1.2.x系列
- [**v1.2.15 版本报告**](project/versions/v1.2.15/) - Client_Server架构
- [**v1.2.14 版本报告**](project/versions/v1.2.14/) - 测试覆盖率项目
- [**v1.2.13 版本报告**](project/versions/v1.2.13/) - 项目总结
- [**v1.2.12 版本报告**](project/versions/v1.2.12/) - 错误修正
- [**v1.2.11 版本报告**](project/versions/v1.2.11/) - 层次4测试重构
- [**v1.2.10 版本报告**](project/versions/v1.2.10/) - 覆盖率测试系统
- [**v1.2.10 版本报告**](project/versions/v1.2.10/) - 覆盖率改进
- [**v1.2.8 版本报告**](project/versions/v1.2.8/) - 编译改进
- [**v1.2.7 版本报告**](project/versions/v1.2.7/) - 测试目录分析
- [**v1.2.6 版本报告**](project/versions/v1.2.6/) - 核心组件注释补全计划
- [**v1.2.5 版本报告**](project/versions/v1.2.5/) - 依赖分析报告
- [**v1.2.4 版本报告**](project/versions/v1.2.4/) - 系统性测试重构
- [**v1.2.3 版本报告**](project/versions/v1.2.3/) - 内存安全修复总结
- [**v1.2.2 版本报告**](project/versions/v1.2.2/) - 内存安全修复
- [**v1.2.1 版本报告**](project/versions/v1.2.1/) - 评估报告
- [**v1.2.0 版本报告**](project/versions/v1.2.0/) - 功能矩阵
- [**v1.2.x系列**](project/versions/) - v1.2.0-v1.2.15版本特性

### 🏆 v1.1.x系列
- [**v1.1.5 版本报告**](project/versions/v1.1.5/) - SQL解析器完善
- [**v1.1.4 版本报告**](project/versions/v1.1.4/) - 多任务执行器架构
- [**v1.1.3 版本报告**](project/versions/v1.1.3/) - 内存安全革命
- [**v1.1.2 版本报告**](project/versions/v1.1.2/) - 全面测试分析
- [**v1.1.1 版本报告**](project/versions/v1.1.1/) - B+树索引集成
- [**v1.1.0 版本报告**](project/versions/v1.1.0/) - 深度分析报告
- [**v1.1.x系列**](project/versions/) - v1.1.0-v1.1.5版本特性

## 📚 学习资源

### 🎓 教材资源
- [**《数据库系统原理与开发实践》**](textbook/《数据库系统原理与开发实践》.md) - 完整教材内容
- [**教材索引**](textbook/README.md) - 教材与章节索引
- [**千年数据演化**](textbook/第1章.md) - 数据处理历史
- [**技术发展史**](textbook/第2章.md) - 计算机技术演进
- [**关系数据库基础**](textbook/第3章.md) - 数学基础与设计

### 📊 PPT课件
- [**第一章：千年数据演化启示录**](textbook/slides/第一章：千年数据演化启示录 .pptx)
- [**第二章：计算与数据共生史**](textbook/slides/第二章：计算与数据共生史.pptx)
- [**第三章：RDBMS设计思想**](textbook/slides/第三章：RDBMS设计思想与工程实现全景解析（SQLCC）.pptx)
- [**第四章：存储引擎设计**](textbook/slides/第四章：存储引擎-操作系统到RDBMS的桥梁-思想算法全栈透视.pptx)

## 📋 历史文档

### 📜 变更日志
- [**完整变更日志**](releases/CHANGELOG.md) - 所有版本变更记录
- [**v1.3.9 ChangeLog**](releases/CHANGELOG_v1.3.9.md) - Level 1 Foundation完整单元测试
- [**v1.3.8 ChangeLog**](releases/CHANGELOG_v1.3.8.md) - SQL Parser模块化重构
- [**v1.3.7 ChangeLog**](releases/CHANGELOG_v1.3.7.md) - Bazel构建系统重构
- [**v1.3.6 ChangeLog**](releases/CHANGELOG_v1.3.6.md) - LLVM覆盖率工具链完善
- [**v1.3.3 ChangeLog**](releases/CHANGELOG_v1.3.3.md) - DDL/DCL功能补全
- [**Bazel重构日志**](releases/CHANGELOG_BAZEL_REFACTORING.md) - 构建系统变更

### 📈 版本发布 (v1.2.x)
- [**v1.2.0 ChangeLog**](releases/CHANGELOG_v1.2.0.md) - 多线程架构
- [**v1.2.1 ChangeLog**](releases/CHANGELOG_v1.2.1.md) - 约束系统增强
- [**v1.2.3 ChangeLog**](releases/CHANGELOG_v1.2.3.md) - SQL-92支持规划
- [**v1.2.4 ChangeLog**](releases/CHANGELOG_v1.2.4.md) - 系统性测试重构
- [**v1.2.5 ChangeLog**](releases/CHANGELOG_v1.2.5.md) - 依赖分析
- [**v1.2.6 ChangeLog**](releases/CHANGELOG_v1.2.6.md) - 注释补全计划

### 📈 版本发布 (v1.1.x)
- [**v1.1.4 ChangeLog**](releases/CHANGELOG_v1.1.4.md) - 多任务执行器
- [**v1.1.5 ChangeLog**](releases/CHANGELOG_v1.1.5.md) - SQL解析器完善

---

## 📋 文档维护

### 📝 文档规范
- 所有文档使用Markdown格式
- 中英文对照，优先中文
- 保持文档与代码同步
- 定期审查和更新

### 🔄 更新频率
- 版本发布时更新相关文档
- 重大功能变更时更新设计文档
- 定期审查和更新用户指南
- 新功能开发时同步编写文档

### 🏗️ 文档架构
```
docs/
├── index.md                    # 📖 文档导航索引（本文件）
├── ai_tools/                   # 🤖 AI开发工具
├── sdd/                        # 🌟 规范驱动开发 (SDD) ⭐新增
│   ├── SPEC_DRIVEN_DEVELOPMENT.md  # SDD 使用指南
│   └── templates/              # SDD 模板
│       ├── requirements_template.md
│       ├── design_template.md
│       └── tasks_template.md
├── api/                        # 🔌 API文档
├── design/                     # 🏗️ 架构设计
│   ├── architecture/           # 架构文档
│   ├── sql_parser/             # SQL解析器设计
│   ├── sql_executor/           # SQL执行器设计
│   ├── storage_engine/         # 存储引擎设计
│   ├── execution/              # 执行引擎设计
│   ├── security/               # 安全设计
│   ├── performance/            # 性能设计
│   ├── distributed/            # 分布式设计
│   └── deployment/             # 部署设计
├── development/                # 💻 开发指南
│   └── guides/                 # 开发指南集合
├── doxygen/                    # 📚 Doxygen文档
├── project/                    # 📝 项目管理
│   ├── versions/               # 版本历史
│   ├── plans/                  # 实施计划
│   └── milestones/             # 里程碑
├── releases/                   # 🚀 版本发布
├── reports/                    # 📊 分析报告
│   ├── coverage/               # 覆盖率报告
│   ├── testing/                # 测试报告
│   └── evaluation/             # 评估报告
├── testing/                    # 🧪 测试文档
└── textbook/                   # 🎓 教材资源
```

---

**📖 快速导航**: [项目概述](../README.md) • [开发者指南](development/guides/) • [API文档](api/) • [架构设计](design/)

**🆕 最新更新 (v1.3.10)**: 规范驱动开发 (SDD) 体系 • C++ 开发规范 • SDD 模板

*最后更新: 2026-02-02*
