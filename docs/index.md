# SQLCC 文档索引

## 📖 关于本项目

SQLCC（SQL Cloud-native Cluster）是一个企业级内存安全的云原生数据库系统，实现了完整SQL-92标准支持和高性能存储引擎。

## 🚀 快速开始

### 🆕 新用户入门 (推荐)
- [**项目概述 (README.md)**](README.md) - 完整的项目介绍、快速开始和验收标准
- [**开发者快速开始指南**](guides/quick_start_developer.md) - 环境搭建、构建、测试、调试全流程
- [**用户指南**](user/user_guide.md) - SQLCC的基本使用方法

### 📋 安装和部署
- [**安装指南**](INSTALL.md) - 系统依赖和安装步骤
- [**开发环境设置**](guides/DEVELOPMENT_ENVIRONMENT_SETUP.md) - Ubuntu C++环境配置
- [**构建和测试指南**](guides/BUILD_AND_TEST_GUIDE.md) - CMake/Bazel构建说明

### 🤖 AI辅助开发
- [**AI工具使用指南**](guides/AI_TOOLS_USAGE_GUIDE.md) - 字节Trae最佳实践
- [**AI开发原则**](development/ai_development_principles.md) - AI辅助软件工程理念
- [**测试驱动开发**](guides/TEST_DRIVEN_DEVELOPMENT_GUIDE.md) - TDD理念与实践

## 🏗️ 架构与设计

### 📐 系统架构
- [**系统架构总览**](design/Architecture.md) - 完整的系统架构说明
- [**存储引擎架构**](design/storage_engine/storage_engine_v1.2.3_design.md) - 存储引擎设计详解
- [**配置管理架构**](design/config_manager/config_manager_design.md) - 配置系统设计
- [**多任务执行器设计**](design/multi_task_executor_design.md) - 并发执行架构
- [**缓冲池设计**](design/storage_engine/buffer_pool.md) - 缓存策略详解

### 🧩 核心组件设计
- [**SQL解析器设计**](design/README.md) - 解析器架构说明
- [**包设计规范**](design/sqlcc_package_design.md) - 模块化设计原则

## 📋 功能特性

### ✅ 功能实现状态
- [**功能矩阵和实现状态**](features/implementation_status.md) - 完整的功能清单
- [**版本功能对比**](releases/VERSION_OVERVIEW.md) - 各版本特性总览

### 🚀 最新版本特性
- [**v1.2.6 Release Notes**](releases/RELEASE_NOTES_v1.2.6.md) - 最新版本发布说明
- [**v1.2.5 Release Notes**](releases/RELEASE_NOTES_v1.2.5.md) - 企业级特性版本
- [**v1.2.4 Release Notes**](releases/RELEASE_NOTES_v1.2.4.md) - 测试重构版本
- [**v1.1.5功能矩阵**](releases/FUNCTION_MATRIX_v1.1.4.md) - 核心功能详细说明

## 🔧 API文档

### 📚 接口文档
- [**API概览**](api/README.md) - 接口文档导航
- [**存储引擎API**](api/storage_engine_api.md) - 存储引擎接口
- [**SQL执行器API**](api/sql_executor_api.md) - 查询执行接口

## 📚 代码与开发

### 💻 编码规范
- [**编码标准**](code/coding_standards.md) - 代码规范和最佳实践
- [**源码注释指南**](code/source_code_comments_guide.md) - Why-What-How三层注释体系
- [**API设计原则**](code/api_design_principles.md) - 接口设计规范

### 🛠️ 开发工具
- [**Doxygen配置**](guides/DOXYGEN_COVERAGE_CONFIGURATION.md) - 文档生成工具
- [**Bazel工具手册**](ai-agent/bazel_tools_manual.md) - 构建工具使用
- [**Bazel知识库**](ai-agent/bazel_knowledge_base.md) - 构建系统知识

## 🚀 项目管理

### 📅 版本规划
- [**产品路线图**](versions/roadmap.md) - 长期发展规划
- [**版本总览**](releases/VERSION_OVERVIEW.md) - 所有历史版本
- [**版本摘要**](version_summary_v1.2.3.md) - 简要版本信息

### 📊 项目进展
- [**项目进展总览**](project_progress.md) - 整体项目状态
- [**项目组织计划**](项目进展/project_organization_plan.md) - 项目管理架构
- [**项目总结**](项目进展/PROJECT_SUMMARY.md) - 阶段性成果
- [**SQL命令支持评估**](项目进展/sql_command_support_evaluation.md) - SQL特性评估

## 👥 社区与贡献

### 🤝 贡献指南
- [**贡献者指南**](CONTRIBUTING.md) - 如何参与贡献
- [**安全政策**](SECURITY.md) - 安全问题报告
- [**开发工作流**](ai-agent/bazel_workflow_guide.md) - 协作开发流程

### 🔧 工具和方法论
- [**AI重构方法论**](ai-agent/systematic_refactoring_knowledge_base.md) - 系统化重构方法
- [**测试重构方法论**](ai-agent/test_refactoring_methodology.md) - 测试系统改进
- [**Bazel改进指南**](ai-agent/bazel_improvement_guide.md) - 构建系统优化

## 📖 用户文档

### 📖 使用指南
- [**用户指南**](user/user_guide.md) - 基本使用方法
- [**故障排除指南**](user/troubleshooting.md) - 问题诊断和解决
- [**常见问题FAQ**](user/faq.md) - 实用问题解答

## 🔍 测试与质量

### 🧪 测试体系
- [**系统性测试重构报告**](项目进展/v1.2.4/系统性测试重构项目完成报告.md) - 测试架构重构
- [**覆盖率分析报告**](项目进展/v1.2.3/SQLCC核心组件覆盖率分析报告.md) - 测试覆盖率分析
- [**测试改进综合分析**](项目进展/v1.2.3/测试改进综合分析报告.md) - 测试质量评估

### 🎯 质量保证
- [**内存安全培训**](security/memory_safety_training.md) - 安全编码培训
- [**测试改进实施计划**](项目进展/v1.2.3/测试改进实施计划.md) - 质量提升计划

## 📊 性能与监控

### ⚡ 性能分析
- [**性能评估报告**](performance_test_report_2025_12_10.md) - 性能测试结果
- [**CRUD性能测试**](comprehensive_crud_performance_test_results.md) - 详细性能数据
- [**真实性能测试**](performance_test_real_results.md) - 实际环境测试

### 📈 监控工具
- [**编译时间基准**](scripts/benchmark_compile_time.sh) - 构建性能监控
- [**覆盖率分析工具**](scripts/analyze_module_coverage.sh) - 代码质量监控

## 🔒 安全与合规

### 🛡️ 安全体系
- [**内存安全审计报告**](项目进展/v1.1.4/内存安全审计报告_20251215_025149.md) - 安全审计结果
- [**安全架构设计**](security/security_architecture.md) - 安全系统设计

## 🛠️ 工具与脚本

### 🔧 开发工具
- [**Bazel依赖修复器**](tools/bazel_dep_fixer_enhanced.py) - 构建依赖管理
- [**代码检查器**](tools/bazel_code_checker.py) - 代码质量检查
- [**标签修复器**](tools/bazel_label_fixer.py) - 构建标签管理

### ⚙️ 自动化脚本
- [**测试流水线**](scripts/test_pipeline.sh) - CI/CD测试流程
- [**环境准备**](scripts/prepare_test_environment.sh) - 测试环境配置
- [**覆盖率收集**](scripts/collect_coverage_data.sh) - 测试覆盖率收集

## 📝 项目进展记录

### 🎯 v1.2.6版本 (当前)
- [**核心组件注释补全计划**](项目进展/v1.2.6/core_components_commenting_and_documentation_plan.md) - 8周注释补全规划
- [**头文件依赖优化**](项目进展/v1.2.6/include_dependency_optimization_final_report.md) - 8.22倍代码组织优化
- [**包构建状态报告**](项目进展/v1.2.6/comprehensive_package_build_status_report.md) - 构建系统完善

### 🚀 v1.3.0版本规划
- [**企业级特性评估**](项目进展/v1.3.0/evaluation_v1.3.0.md) - 全面技术评估
- [**企业级特性计划**](项目进展/v1.3.0/v1.3.0_企业级特性实施计划.md) - 8大特性规划
- [**版本开发任务**](项目进展/v1.3.0/v1.3.0_TODO.md) - 详细任务清单

### 📋 v1.2.5版本
- [**依赖分析报告**](项目进展/v1.2.5/dependency_analysis_report.md) - 代码依赖分析
- [**类文件分离分析**](项目进展/v1.2.5/class_file_separation_analysis_report.md) - 架构重构分析
- [**重构改进总结**](项目进展/v1.2.5/sqlcc_refactoring_improvement_summary.md) - 重构成果总结

### 🔄 v1.2.4版本
- [**系统性测试重构**](项目进展/v1.2.4/系统性测试重构项目完成报告.md) - 测试架构重构
- [**测试分析报告**](项目进展/v1.2.4/test_analysis_report.json) - 测试质量分析
- [**包设计规范**](项目进展/v1.2.4/sqlcc_package_design.md) - 模块化设计

### 📈 v1.2.3版本
- [**内存安全修复总结**](项目进展/v1.2.3/v1.2.3_内存安全专项修复总结.md) - 安全修复成果
- [**约束系统实现**](项目进展/v1.2.3/constraint_system_implementation_report.md) - SQL约束实现
- [**配置管理改进**](项目进展/v1.2.3/config_manager_upgrade_guide.md) - 配置系统优化

### 🏆 v1.1.5版本
- [**SQL解析器完善**](项目进展/v1.1.5/sql_parser_enhancement_report.md) - 解析器功能增强
- [**测试覆盖率提升**](项目进展/v1.1.5/testing_coverage_improvement.md) - 测试质量提升

### 🛡️ v1.1.4版本
- [**多任务执行器架构**](项目进展/v1.1.4/task_executor_architecture.md) - 并发架构设计
- [**B+树深度修复**](项目进展/v1.1.4/b_plus_tree_deep_fix.md) - 索引系统修复
- [**网络深度修复**](项目进展/v1.1.4/network_deep_fix.md) - 网络系统修复

### 🔒 v1.1.3版本
- [**内存安全革命**](项目进展/v1.1.3/memory_safety_revolution.md) - 620+安全问题解决
- [**智能指针生态**](项目进展/v1.1.3/smart_pointer_ecosystem.md) - 内存管理重构

## 📚 学习资源

### 🎓 教材资源
- [**《数据库系统原理与开发实践》**](textbook/《数据库系统原理与开发实践》.md) - 完整教材内容
- [**教材提纲**](textbook/教材提纲.md) - 16章教学大纲
- [**千年数据演化**](textbook/第1章.md) - 数据处理历史
- [**技术发展史**](textbook/第2章.md) - 计算机技术演进
- [**关系数据库基础**](textbook/第3章.md) - 数学基础与设计

### 📊 PPT课件
- [**第一章：千年数据演化启示录**](textbook/第一章：千年数据演化启示录.pptx)
- [**第二章：计算与数据共生史**](textbook/第二章：计算与数据共生史.pptx)
- [**第三章：RDBMS设计思想**](textbook/第三章：RDBMS设计思想与工程实现全景解析（SQLCC）.pptx)
- [**第四章：存储引擎设计**](textbook/第四章：存储引擎-操作系统到RDBMS的桥梁-思想算法全栈透视.pptx)

## 📋 历史文档

### 📜 变更日志
- [**完整变更日志**](CHANGELOG.md) - 所有版本变更记录
- [**Bazel重构日志**](CHANGELOG_BAZEL_REFACTORING.md) - 构建系统变更

### 📈 版本发布
- [**v1.1.4 ChangeLog**](releases/CHANGELOG_v1.1.4.md)
- [**v1.1.5 ChangeLog**](releases/CHANGELOG_v1.1.5.md)
- [**v1.2.0 ChangeLog**](releases/CHANGELOG_v1.2.0.md)
- [**v1.2.1 ChangeLog**](releases/CHANGELOG_v1.2.1.md)
- [**v1.2.3 ChangeLog**](releases/CHANGELOG_v1.2.3.md)
- [**v1.2.4 ChangeLog**](releases/CHANGELOG_v1.2.4.md)
- [**v1.2.5 ChangeLog**](releases/CHANGELOG_v1.2.5.md)
- [**v1.2.6 ChangeLog**](releases/CHANGELOG_v1.2.6.md)

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
├── README.md                   # 🚀 项目概述（根目录）
├── guides/                     # 🛠️ 开发指南合集
├── user/                       # 👥 用户文档
├── design/                     # 🏗️ 架构设计
├── features/                   # 📋 功能特性
├── code/                       # 💻 代码规范
├── releases/                   # 🚀 版本发布
├── 项目进展/                   # 📝 项目进展
├── api/                        # 🔌 API文档
├── ai-agent/                   # 🤖 AI开发工具
├── security/                   # 🔒 安全文档
└── textbook/                   # 🎓 教材资源
```

---

**📖 快速导航**: [项目概述](README.md) • [开发者指南](guides/quick_start_developer.md) • [用户指南](user/user_guide.md) • [故障排除](user/troubleshooting.md)

**🆕 最新更新**: v1.2.6 核心组件注释补全计划 • 完整文档索引重构 • FAQ和故障排除指南完善

*最后更新: 2025-12-24*
