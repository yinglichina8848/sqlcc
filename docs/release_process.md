<<<<<<< Updated upstream
# 发布流程文档
=======
<<<<<<< HEAD
# 自动化发布流程
>>>>>>> Stashed changes

## 概述

本文档描述了SQLCC项目的发布流程，包括如何使用自动化发布脚本来创建新版本。

## 发布脚本

项目提供了一个自动化发布脚本 `scripts/release.sh`，用于简化版本发布流程。

<<<<<<< Updated upstream
=======
## 使用方法
=======
# 发布流程文档

## 概述

本文档描述了SQLCC项目的发布流程，包括如何使用自动化发布脚本来创建新版本。

## 发布脚本

项目提供了一个自动化发布脚本 `scripts/release.sh`，用于简化版本发布流程。

>>>>>>> Stashed changes
### 功能特性

- 自动更新版本信息
- 创建Git标签
- 生成Doxygen文档和代码覆盖率报告
- 将文档更新到docs分支
- 推送所有更改到远程仓库

### 使用方法

#### 基本用法
<<<<<<< Updated upstream
=======
>>>>>>> master
>>>>>>> Stashed changes

```bash
./scripts/release.sh <版本号> [提交信息]
```

<<<<<<< Updated upstream
示例：
=======
<<<<<<< HEAD
### 参数说明

- `版本号`: 必需参数，格式如 `v0.2.4`
- `提交信息`: 可选参数，如果不提供，将使用默认信息 "发布版本 <版本号>"

### 示例

>>>>>>> Stashed changes
```bash
./scripts/release.sh v0.2.7 "添加新功能"
```

#### 查看帮助

```bash
./scripts/release.sh --help
# 或
./scripts/release.sh -h
```

#### 模拟运行（不执行实际操作）

```bash
./scripts/release.sh --dry-run <版本号> [提交信息]
```

示例：
```bash
./scripts/release.sh --dry-run v0.2.7 "测试发布"
```

## 发布流程

1. **准备发布**
   - 确保所有代码更改已提交到master分支
   - 确保测试通过
   - 更新ChangeLog.md文件

2. **执行发布**
   - 使用发布脚本创建新版本
   - 脚本会自动处理以下步骤：
     - 更新VERSION文件
     - 创建Git标签
     - 生成文档
     - 更新docs分支
     - 推送所有更改

3. **验证发布**
   - 检查Gitee上的标签是否创建成功
   - 确认docs分支的文档是否更新
   - 验证版本信息是否正确

## 注意事项

- 发布脚本需要在master分支上执行
- 确保有足够的权限推送代码到远程仓库
- 发布前请确保所有更改已测试并提交
- 建议使用--dry-run选项先模拟运行一次，确认无误后再正式发布

## 版本命名规范

项目使用语义化版本号（Semantic Versioning）：
- 主版本号：不兼容的API修改
- 次版本号：向下兼容的功能性新增
- 修订号：向下兼容的问题修正

示例：
- v0.1.0 - 初始版本
- v0.2.0 - 添加新功能
- v0.2.1 - 修复bug

## 故障排除

### 常见问题

1. **权限错误**
   - 确保有推送代码到远程仓库的权限
   - 检查SSH密钥是否配置正确

2. **文档生成失败**
   - 确保Doxygen和gcov工具已安装
   - 检查Makefile中的docs和coverage目标是否正确

3. **分支冲突**
   - 如果docs分支有冲突，需要手动解决后再重新运行脚本

### 手动发布步骤

<<<<<<< Updated upstream
=======
如果需要自定义发布流程，可以编辑 `scripts/release.sh` 脚本。脚本中的主要变量和命令都有注释说明。
=======
示例：
```bash
./scripts/release.sh v0.2.7 "添加新功能"
```

#### 查看帮助

```bash
./scripts/release.sh --help
# 或
./scripts/release.sh -h
```

#### 模拟运行（不执行实际操作）

```bash
./scripts/release.sh --dry-run <版本号> [提交信息]
```

示例：
```bash
./scripts/release.sh --dry-run v0.2.7 "测试发布"
```

## 发布流程

1. **准备发布**
   - 确保所有代码更改已提交到master分支
   - 确保测试通过
   - 更新ChangeLog.md文件

2. **执行发布**
   - 使用发布脚本创建新版本
   - 脚本会自动处理以下步骤：
     - 更新VERSION文件
     - 创建Git标签
     - 生成文档
     - 更新docs分支
     - 推送所有更改

3. **验证发布**
   - 检查Gitee上的标签是否创建成功
   - 确认docs分支的文档是否更新
   - 验证版本信息是否正确

## 注意事项

- 发布脚本需要在master分支上执行
- 确保有足够的权限推送代码到远程仓库
- 发布前请确保所有更改已测试并提交
- 建议使用--dry-run选项先模拟运行一次，确认无误后再正式发布

## 版本命名规范

项目使用语义化版本号（Semantic Versioning）：
- 主版本号：不兼容的API修改
- 次版本号：向下兼容的功能性新增
- 修订号：向下兼容的问题修正

示例：
- v0.1.0 - 初始版本
- v0.2.0 - 添加新功能
- v0.2.1 - 修复bug

## 故障排除

### 常见问题

1. **权限错误**
   - 确保有推送代码到远程仓库的权限
   - 检查SSH密钥是否配置正确

2. **文档生成失败**
   - 确保Doxygen和gcov工具已安装
   - 检查Makefile中的docs和coverage目标是否正确

3. **分支冲突**
   - 如果docs分支有冲突，需要手动解决后再重新运行脚本

### 手动发布步骤

>>>>>>> Stashed changes
如果自动脚本无法使用，可以手动执行以下步骤：

1. 更新版本信息：
   ```bash
   echo "v0.2.7" > VERSION
   git add VERSION
   git commit -m "更新版本到 v0.2.7"
   ```

2. 创建标签：
   ```bash
   git tag -a v0.2.7 -m "发布版本 v0.2.7"
   ```

3. 生成文档：
   ```bash
   make docs
   make coverage
   ```

4. 更新docs分支：
   ```bash
   git stash
   git checkout docs
   git pull origin docs
   cp -r ../docs/doxygen ../docs/coverage docs/
   git add docs/doxygen/ docs/coverage/
   git commit -m "更新文档 - v0.2.7"
   ```

5. 推送更改：
   ```bash
   git checkout master
   git stash pop
   git push origin master
   git push origin v0.2.7
   git checkout docs
   git push origin docs
<<<<<<< Updated upstream
   ```
=======
   ```
>>>>>>> master
>>>>>>> Stashed changes
