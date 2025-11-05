# SQLCC 文档中心

欢迎来到SQLCC（Simple C++ Database Storage Engine）文档中心。这里包含了项目的所有技术文档，帮助您了解和使用SQLCC存储引擎。

## 📚 文档导航

### 核心文档
- [存储引擎设计文档](storage_engine_design.md) - 存储引擎的架构设计、组件实现和关键算法
- [单元测试文档](unit_testing.md) - 单元测试框架介绍和测试用例详解

### API文档
- [Doxygen API文档](doxygen/html/index.html) - 详细的类和函数API参考

### 项目文档
- [README.md](../README.md) - 项目概述、开发指南和快速入门
- [ChangeLog.md](../ChangeLog.md) - 项目变更历史和版本记录

## 🚀 快速开始

1. **了解项目**：从[README.md](../README.md)开始，了解项目概述和功能
2. **学习架构**：阅读[存储引擎设计文档](storage_engine_design.md)，理解系统架构
3. **查看API**：浏览[Doxygen API文档](doxygen/html/index.html)，了解接口使用
4. **运行测试**：参考[单元测试文档](unit_testing.md)，了解测试框架和用例

## 📖 文档说明

### 存储引擎设计文档
详细介绍了SQLCC存储引擎的设计和实现，包括：
- 系统架构概览
- 核心组件设计（DiskManager、BufferPool、Page、StorageEngine）
- 页面生命周期管理
- 错误处理机制
- 性能考虑和优化

### 单元测试文档
全面介绍了项目的测试框架和测试用例，包括：
- 测试环境配置
- 测试类结构
- 测试用例详解
- 测试覆盖范围
- 测试执行指南

### Doxygen API文档
自动生成的API文档，包含：
- 类层次结构
- 公共接口说明
- 函数参数和返回值
- 代码示例
- 交叉引用

## 🔧 文档生成

### 生成Doxygen文档
```bash
# 使用脚本生成
./scripts/generate_docs.sh

# 或直接使用doxygen命令
doxygen Doxyfile
```

### 更新文档
文档更新流程：
1. 更新源代码中的Doxygen注释
2. 修改Markdown文档
3. 运行文档生成脚本
4. 提交更改

## 🤝 贡献指南

欢迎为文档做出贡献！您可以：
- 修正文档中的错误
- 添加缺失的说明
- 改进文档结构
- 增加示例和教程

## 📞 联系方式

如有文档相关问题，请通过以下方式联系：
- 提交Issue
- 发送邮件至：1820393151@qq.com

---

**最后更新**：2025-11-05 15:07:00