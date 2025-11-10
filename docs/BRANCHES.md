<<<<<<< Updated upstream
# 项目分支使用指南
=======
<<<<<<< HEAD
# SQLCC 项目分支说明
>>>>>>> Stashed changes

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

**用途**：专门用于存放Doxygen生成的API文档

**内容**：
- Doxygen生成的完整API文档 (`docs/doxygen/html/`)
- 可能包含额外的文档和示例

**特点**：
- 包含完整的API文档，便于查阅
- 文档更新通常与代码更新同步进行
- 适合不参与开发但需要了解API的用户
- 分支体积较大，克隆速度较慢

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
3. 代码更新后，需要重新生成API文档并更新到文档分支

### 文档更新

1. 基础文档（如README.md、Guide.md等）在主分支更新
2. API文档在文档分支更新（由Doxygen自动生成）
3. 当主分支代码有重大更新时，需要同步更新文档分支的API文档

### 版本发布

1. 版本标记（tag）在主分支创建
2. 发布说明在主分支的ChangeLog.md中更新
3. 文档分支会同步创建相同的版本标记

## 常见问题

### Q: 为什么不把API文档放在主分支？

A: API文档由Doxygen自动生成，文件数量多且体积大，放在主分支会增加仓库体积，影响克隆速度。分离到文档分支可以保持主分支的轻量级，同时为需要详细API信息的用户提供完整文档。

### Q: 如何确保两个分支的同步？

<<<<<<< Updated upstream
=======
```bash
git checkout master
git merge docs --no-ff  # 不使用快进合并，保留分支历史
```
=======
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

**用途**：专门用于存放Doxygen生成的API文档

**内容**：
- Doxygen生成的完整API文档 (`docs/doxygen/html/`)
- 可能包含额外的文档和示例

**特点**：
- 包含完整的API文档，便于查阅
- 文档更新通常与代码更新同步进行
- 适合不参与开发但需要了解API的用户
- 分支体积较大，克隆速度较慢

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
3. 代码更新后，需要重新生成API文档并更新到文档分支

### 文档更新

1. 基础文档（如README.md、Guide.md等）在主分支更新
2. API文档在文档分支更新（由Doxygen自动生成）
3. 当主分支代码有重大更新时，需要同步更新文档分支的API文档

### 版本发布

1. 版本标记（tag）在主分支创建
2. 发布说明在主分支的ChangeLog.md中更新
3. 文档分支会同步创建相同的版本标记

## 常见问题

### Q: 为什么不把API文档放在主分支？

A: API文档由Doxygen自动生成，文件数量多且体积大，放在主分支会增加仓库体积，影响克隆速度。分离到文档分支可以保持主分支的轻量级，同时为需要详细API信息的用户提供完整文档。

### Q: 如何确保两个分支的同步？

>>>>>>> Stashed changes
A: 两个分支的同步方式如下：

1. 代码更新在主分支进行
2. 当需要更新API文档时，切换到文档分支并执行：
   ```bash
   # 在文档分支上执行
   git checkout docs
   git merge master  # 获取最新的代码
   doxygen Doxyfile  # 重新生成API文档
   git add docs/doxygen/html/
   git commit -m "更新API文档"
   git push origin docs
   ```

### Q: 我应该向哪个分支提交贡献？

A: 贡献提交的分支选择：

1. 代码贡献：请向主分支提交Pull Request
2. 基础文档改进：请向主分支提交Pull Request
3. API文档相关：通常不需要手动提交，由Doxygen自动生成
4. 如果需要手动修改API文档模板或样式：请向文档分支提交Pull Request

### Q: 如何获取最新的API文档？

A: 有两种方式：

1. 切换到文档分支查看最新生成的API文档：
   ```bash
   git checkout docs
   # 使用浏览器打开 docs/doxygen/html/index.html
   ```

2. 在本地生成API文档（需要安装Doxygen）：
   ```bash
   # 确保在主分支上，并且代码是最新的
   git checkout master
   git pull origin master
   
   # 生成API文档
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

<<<<<<< Updated upstream
**最后更新：2025年11月07日 | 作者：yinglichina, 李莹**
=======
**最后更新：2025年11月07日 | 作者：yinglichina, 李莹**
>>>>>>> master
>>>>>>> Stashed changes
