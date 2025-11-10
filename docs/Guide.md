<<<<<<< Updated upstream
# SQLCC 项目使用指南 (v0.2.3)
=======
<<<<<<< HEAD
# SQLCC 项目使用指南
=======
# SQLCC 项目使用指南 (v0.2.3)
>>>>>>> master
>>>>>>> Stashed changes

本指南将帮助您了解如何使用SQLCC（Simple C++ Database Storage Engine）项目，包括代码结构、构建方法、测试运行和文档查看。

## 项目概述

SQLCC是一个简单的C++数据库存储引擎，实现了基本的页式存储管理、缓冲池和LRU替换策略。本项目展示了如何利用AI辅助工具进行高效的软件开发。

<<<<<<< Updated upstream
=======
<<<<<<< HEAD
=======
>>>>>>> Stashed changes
**当前版本**：v0.2.3 (2025-06-18)
**主要特性**：
- 基本页式存储管理
- 缓冲池和LRU替换策略
- 完善的设计文档和教学价值
- 详细的Doxygen注释，采用Why-What-How格式

<<<<<<< Updated upstream
=======
>>>>>>> master
>>>>>>> Stashed changes
## 项目结构

```
sqlcc/
├── include/                # 头文件目录
│   ├── page.h             # 页面类定义
│   ├── disk_manager.h     # 磁盘管理器类定义
│   ├── buffer_pool.h      # 缓冲池类定义
│   ├── storage_engine.h   # 存储引擎类定义
│   ├── logger.h           # 日志类定义
│   ├── exception.h        # 异常类定义
│   └── version.h          # 版本信息
├── src/                   # 源代码目录
│   ├── page.cc            # 页面类实现
│   ├── disk_manager.cc    # 磁盘管理器类实现
│   ├── buffer_pool.cc     # 缓冲池类实现
│   ├── storage_engine.cc  # 存储引擎类实现
│   ├── logger.cc          # 日志类实现
│   └── exception.cc       # 异常类实现
├── tests/                 # 测试目录
│   └── storage_engine_test.cc  # 单元测试文件
├── docs/                  # 文档目录
│   ├── index.md           # 文档索引
│   ├── storage_engine_design.md  # 存储引擎设计文档
│   ├── unit_testing.md    # 单元测试文档
│   ├── BRANCHES.md        # 分支说明文档
│   └── doxygen/           # Doxygen生成的API文档（在docs分支）
├── scripts/               # 脚本目录
│   └── generate_docs.sh   # 文档生成脚本
├── build/                 # 构建目录（运行cmake后生成）
├── CMakeLists.txt         # CMake配置文件
├── Doxyfile              # Doxygen配置文件
├── Makefile              # Makefile，提供便捷命令
├── README.md             # 项目说明文档
├── ChangeLog.md          # 变更日志
├── TRAE-Chat.md          # AI辅助交互记录
└── .gitignore            # Git忽略文件配置
```

## 分支管理

本项目采用分支管理策略，分离代码和文档：

- **master分支**：包含核心代码和基础文档
- **docs分支**：包含完整的API文档和项目文档

详细说明请参考 [BRANCHES.md](BRANCHES.md)

## 环境要求

- C++11或更高版本的编译器
- CMake 3.10或更高版本
- Google Test框架
- Doxygen（用于生成API文档）

## 构建和测试

### 1. 克隆项目

```bash
git clone https://gitee.com/your-username/sqlcc.git
cd sqlcc
```

### 2. 安装依赖

```bash
# 安装Google Test（Ubuntu/Debian）
sudo apt update
sudo apt install libgtest-dev cmake build-essential

# 安装Doxygen（用于生成文档）
sudo apt install doxygen
```

### 3. 构建项目

```bash
# 使用Makefile（推荐）
make

# 或者手动构建
mkdir -p build
cd build
cmake ..
make
```

### 4. 运行测试

```bash
# 运行所有测试
./build/bin/storage_engine_test

# 运行特定测试
./build/bin/storage_engine_test --gtest_filter=StorageEngineTest.DeletePage

# 使用Makefile运行测试
make test
```

## 文档查看

### 1. 查看基础文档

基础文档（如README.md、设计文档等）可以直接在master分支查看：

```bash
# 查看文档索引
cat docs/index.md

# 查看存储引擎设计文档
cat docs/storage_engine_design.md

# 查看单元测试文档
cat docs/unit_testing.md
```

### 2. 查看API文档

API文档由Doxygen生成，存储在docs分支：

```bash
# 切换到docs分支
git checkout docs

# 在浏览器中打开API文档
# 使用您喜欢的浏览器打开 docs/doxygen/html/index.html
xdg-open docs/doxygen/html/index.html  # Linux
open docs/doxygen/html/index.html      # macOS
start docs/doxygen/html/index.html     # Windows
```

### 3. 生成最新文档

如果您修改了源代码并希望生成最新的API文档：

```bash
# 确保在docs分支
git checkout docs

# 合并master分支的最新代码
git merge master

# 生成文档
make docs

# 提交更新的文档
git add docs/doxygen
git commit -m "更新API文档"
git push origin docs
```

## 常用命令

### 构建和清理

```bash
# 构建项目
make

# 运行测试
make test

# 生成文档
make docs

# 清理构建文件
make clean

# 清理所有文件（包括文档）
make clean-all

# 查看所有可用命令
make help
```

### Git操作

```bash
# 查看当前分支
git branch

# 切换到master分支
git checkout master

# 切换到docs分支
git checkout docs

# 查看分支状态
git status

# 提交更改
git add .
git commit -m "提交信息"
git push origin <branch-name>
```

## 项目版本

<<<<<<< Updated upstream
当前版本：v0.2.4
=======
当前版本：v0.3.5
>>>>>>> Stashed changes

版本历史请参考 [ChangeLog.md](../ChangeLog.md)

## 🆕 版本升级指南

### 从 v0.3.4 升级到 v0.3.5

v0.3.5 版本主要关注线程安全锁定机制优化，主要变更包括：

1. **线程安全锁定机制**：为关键I/O方法添加递归互斥锁保护
2. **RAII锁定模式**：采用RAII模式管理锁生命周期，自动释放避免死锁
3. **死锁检测机制**：实现死锁检测和预防机制，确保系统稳定性
4. **性能优化**：锁定机制引入的性能下降控制在5%以内
5. **测试验证**：33/33单元测试通过，4个并发测试全部通过

升级步骤：
1. 备份现有代码
2. 拉取最新代码：`git pull origin master`
3. 重新编译项目：`make clean && make`
4. 运行单元测试：`make test`
5. 运行并发测试：`make concurrency-test`
6. 验证性能基准：`make performance-test`

### 从 v0.2.x 升级到 v0.3.x

v0.3.x 版本主要关注性能优化和测试框架完善，主要变更包括：

1. **性能测试框架**：新增全面的性能测试框架
2. **批量预取优化**：实现高效的批量页面预取机制
3. **代码覆盖率测试**：集成gcovr代码覆盖率测试工具
4. **配置管理器重构**：基于单例模式的配置管理器实现

升级步骤：
1. 备份现有代码
2. 拉取最新代码：`git pull origin master`
3. 重新编译项目：`make clean && make`
4. 运行测试验证：`make test`
5. 检查性能基准：`make performance-test`

## 开发指南

如果您想参与项目开发，请遵循以下步骤：

1. Fork项目到您的Gitee账户
2. 克隆您的Fork
3. 创建新分支进行开发
4. 提交您的更改
5. 创建Pull Request

### 代码风格

- 使用4个空格进行缩进
- 类名使用PascalCase（大驼峰）
- 函数名和变量名使用camelCase（小驼峰）
- 常量使用全大写字母，单词间用下划线分隔
- 每个公共函数和方法都应有Doxygen风格的注释

### 测试要求

- 所有新功能都应有相应的单元测试
- 确保所有测试通过后再提交代码
- 测试覆盖率应保持在80%以上

## 问题反馈

如果您在使用过程中遇到问题，请通过以下方式反馈：

1. 在Gitee上提交Issue
2. 发送邮件至项目维护者
3. 在项目讨论区发起讨论

## 致谢

感谢所有为SQLCC项目做出贡献的开发者和用户。

---

<<<<<<< Updated upstream
**最后更新**：2025-11-07 14:41:26
**项目版本**：SQLCC v0.2.4
=======
<<<<<<< HEAD
**最后更新**：2025-11-05 15:30:00
**项目版本**：SQLCC v0.3.0
=======
**最后更新**：2025-11-07 14:41:26
**项目版本**：SQLCC v0.2.4
>>>>>>> master
>>>>>>> Stashed changes
