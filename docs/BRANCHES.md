# 项目分支使用指南

## 概述

本项目使用Git分支管理不同的开发阶段和文档类型，确保代码和文档的清晰分离。了解分支结构有助于您更好地理解项目组织方式和协作流程。

## 分支结构

### 主分支 (master)

**用途**：包含项目源代码和基础文档

**内容**：
- 源代码 (`src/`, `include/`, `tests/`)
- 基础文档 (`README.md`, `Guide.md`, `ChangeLog.md`, `Architecture.md`)
- 项目配置文件 (`CMakeLists.txt`, `Makefile`, `Doxyfile`)
- 版本信息 (`include/version.h`)

**特点**：
- 始终保持可编译状态
- 不包含Doxygen生成的API文档
- 定期发布稳定版本

### 文档分支 (docs)

**用途**：包含主分支所有内容以及完整的API文档

**内容**：
- 主分支的所有内容
- Doxygen生成的完整API文档 (`docs/doxygen/html/`)
- 可能包含额外的文档和示例

**特点**：
- 包含完整的API文档，便于查阅
- 文档更新可能比代码更新频繁
- 适合不参与开发但需要了解API的用户

## 分支切换指南

### 查看API文档

如果您需要查看完整的API文档，请执行以下命令：

```bash
# 切换到文档分支
git checkout docs

# 打开API文档
# 使用浏览器打开 docs/doxygen/html/index.html
# 例如：
# firefox docs/doxygen/html/index.html
# 或
# google-chrome docs/doxygen/html/index.html
```

### 返回主分支

查看完API文档后，可以返回主分支继续开发：

```bash
# 切换回主分支
git checkout master
```

## 分支维护策略

### 代码开发

1. 所有代码开发都在主分支进行
2. 主分支的每次提交都应确保项目可以正常编译
3. 定期将主分支的更改合并到文档分支

### 文档更新

1. 基础文档（如README.md、Guide.md等）在主分支更新
2. API文档在文档分支更新（由Doxygen自动生成）
3. 定期将文档分支中的文档更改合并回主分支（如果有的话）

### 版本发布

1. 版本标记（tag）在主分支创建
2. 发布说明在主分支的ChangeLog.md中更新
3. 文档分支会同步获取版本标记

## 常见问题

### Q: 为什么不把API文档放在主分支？

A: API文档由Doxygen自动生成，文件数量多且体积大，放在主分支会增加仓库体积，影响克隆速度。分离到文档分支可以保持主分支的轻量级，同时为需要详细API信息的用户提供完整文档。

### Q: 如何确保两个分支的同步？

A: 项目维护者会定期将主分支的更改合并到文档分支：

```bash
# 在文档分支上执行
git checkout docs
git merge master
```

### Q: 我应该向哪个分支提交贡献？

A: 如果您贡献的是代码或基础文档，请向主分支提交。如果您贡献的是API文档相关的内容，请向文档分支提交。

### Q: 如何获取最新的API文档？

A: 有两种方式：

1. 切换到文档分支查看最新生成的API文档
2. 在本地生成API文档（需要安装Doxygen）：

```bash
# 在主分支上
cd /home/liying/sqlcc
doxygen Doxyfile
# 生成的文档将位于 docs/doxygen/html/ 目录
```

## 分支历史

- **初始版本**：项目创建时只有主分支
- **v0.1.0**：添加了基本的源代码结构
- **v0.2.0**：实现了存储引擎核心功能
- **v0.2.1**：完善了单元测试和文档
- **v0.2.2**：完善了文档体系，添加了架构设计文档

---

**最后更新：2025年11月06日 | 作者：yinglichina, 李莹**