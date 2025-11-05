# 《数据库原理》期末大作业：AI驱动的微型数据库系统开发 - 完整指南

## 📖 项目简介

本指南详细记录了咨询AI以完成《数据库原理》期末作业的全部过程，从环境搭建到代码提交的全流程，方便新手学习和实践。

---

## 📊 第一阶段完成总结（项目初始化）

### ✅ 已完成任务

#### 1. 项目结构搭建
- [x] 创建了标准的C++项目目录结构
- [x] 配置了`.gitignore`文件，排除构建产物
- [x] 设置了CMake构建系统
- [x] 创建了版本管理头文件

#### 2. 构建系统配置
- [x] CMakeLists.txt配置完成
- [x] 支持C++20标准
- [x] 启用了完整的编译警告
- [x] 配置了调试构建模式

#### 3. 基础程序实现
- [x] 主程序框架搭建
- [x] 版本信息显示功能
- [x] 程序正常编译和运行

#### 4. 版本控制与协作
- [x] Git仓库初始化完成
- [x] 首次提交成功
- [x] Gitee远程仓库配置完成
- [x] 标签v0.0.1创建并推送

### 🎯 当前项目状态

**项目版本**：v0.1.0  
**构建状态**：✅ 编译成功  
**运行状态**：✅ 正常运行  
**代码质量**：✅ 无警告，无错误  

### 📈 性能指标
- 编译时间：< 1秒
- 可执行文件大小：约 20KB
- 内存使用：正常
- 启动速度：瞬时

### 🔧 技术栈确认
- **语言**：C++20
- **构建工具**：CMake 3.22+
- **编译器**：GCC 13.3.0
- **开发环境**：WSL + Linux
- **版本控制**：Git + Gitee

### 📝 经验教训

1. **CMake配置要点**：
   - 必须正确设置项目结构和文件路径
   - 编译警告选项有助于发现潜在问题
   - 构建目录应该与源代码分离

2. **版本管理最佳实践**：
   - 使用语义化版本号（Semantic Versioning）
   - 及时创建标签标记重要里程碑
   - 保持提交信息清晰和有意义

3. **开发环境搭建**：
   - WSL提供了良好的Linux开发体验
   - CMake跨平台特性确保项目可移植性
   - 完整的.gitignore避免提交不必要的文件

### 🚀 下一步计划

进入第二阶段：**存储引擎核心开发**

1. 实现磁盘管理器（Disk Manager）
2. 开发缓冲池（Buffer Pool）
3. 设计页式存储结构
4. 实现基本的文件I/O操作

---

## 🛠️ 环境准备

### 1. 安装 TRAE 开发工具
参考教程：[TRAE国内版安装教程](https://www.cnblogs.com/aidigitialfuture/p/18875823/trae-domestic-version-installation-tutorial-1jadigitial)

### 2. 安装 WSL + Ubuntu 环境
- 准备类 Ubuntu 的开发环境
- 习惯纯 Windows 环境的同学可跳过此步
- 需自行准备 C++/Java 等开发环境
- 相关教程丰富，请自行搜索安装

---

## 📦 项目初始化

### 1. Gitee 仓库设置
```bash
# 1. 登录 Gitee 完成注册（手机号+验证码）
# 2. 创建仓库：sqlcc
# 3. 仓库地址：https://gitee.com/yinglichina/sqlcc
```

### 2. AI 咨询记录
**问题：**
> "准备采用 C/C++ 完成数据库系统的开发，本地环境：Win11 WSL + Ubuntu 24.04，TRAE 的开发工具，Gitee上定义了仓库，如何配置本地项目并与 Gitee 同步？"

**解决方案：**
提供从零到可编译、可推送的完整流程，在 Win11 WSL + Ubuntu 24.04 环境下，10分钟即可完成配置。

---

## 🚀 详细操作步骤

### 第一步：WSL 环境配置

#### 1.1 系统更新与工具安装
```bash
sudo apt update && sudo apt upgrade -y
sudo apt install -y build-essential gdb cmake git ninja-build clang-format clang-tidy
```

#### 1.2 Git 全局配置
```bash
git config --global user.name "你的名字"
git config --global user.email "你的邮箱@xxx.com"
```

#### 1.3 SSH 密钥生成与配置
```bash
# 生成 SSH 密钥
ssh-keygen -t ed25519 -C "你的邮箱@xxx.com"
# 一路回车即可

# 查看公钥
cat ~/.ssh/id_ed25519.pub

# 测试连接
ssh -T git@gitee.com
```
将公钥添加到 Gitee：设置 → SSH 公钥 → 新增

### 第二步：克隆仓库到本地
```bash
mkdir -p ~/proj && cd ~/proj
git clone git@gitee.com:yinglichina/sqlcc.git
cd sqlcc
```

### 第三步：构建 CMake 项目结构

#### 3.1 创建项目目录结构
```bash
mkdir -p {src,include,tests,docs,scripts}
touch README.md .gitignore CMakeLists.txt
```

#### 3.2 配置 .gitignore 文件
```bash
cat > .gitignore <<'EOF'
/build/
/cmake-build-*/
/.vscode/
/.idea/
*.o
*.a
*.so
*.exe
compile_commands.json
EOF
```

#### 3.3 配置根目录 CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 3.22)
project(sqlcc VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 警告配置
add_compile_options(-Wall -Wextra -Wpedantic -Wshadow -Wconversion)

include_directories(${CMAKE_SOURCE_DIR}/include)

# 核心静态库
add_subdirectory(src)

# 可执行文件
add_executable(sqlcc_cli main.cc)
target_link_libraries(sqlcc_cli PRIVATE sqlcc_core)

# 单元测试（可选）
enable_testing()
add_subdirectory(tests)
```

#### 3.4 配置 src 目录 CMakeLists.txt
```cmake
add_library(sqlcc_core STATIC
    buffer_pool.cc
    disk_manager.cc
    bpt.cc
    index.cc
    record.cc
    sql_parser.cc
)

target_include_directories(sqlcc_core PUBLIC ${CMAKE_SOURCE_DIR}/include)
```

#### 3.5 创建主程序文件
**main.cc:**
```cpp
#include <iostream>
#include "version.h"

int main() {
    std::cout << "SqlCC " << SQLCC_VERSION << " startup!" << std::endl;
    return 0;
}
```

#### 3.6 创建版本头文件
**include/version.h:**
```cpp
#pragma once
#define SQLCC_VERSION "0.1.0"
```

### 第四步：首次编译测试

#### 4.1 手工编译项目（推荐方式）
```bash
# 方法1：使用CMake传统方式
mkdir -p build
cd build
cmake ..
make

# 方法2：使用现代CMake语法（推荐）
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build

# 方法3：使用Ninja构建系统（更快）
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

#### 4.2 运行测试程序
```bash
# 进入构建目录
cd build

# 运行程序
./sqlcc

# 预期输出：
# SqlCC 0.1.0 startup!
```

#### 4.3 编译验证检查清单
- [x] CMake配置成功，无错误提示
- [x] 编译过程无警告和错误
- [x] 可执行文件生成成功（build/sqlcc）
- [x] 程序运行输出正确版本信息
- [x] 程序正常退出（返回码0）

### 第五步：提交到 Gitee
```bash
git add .
git commit -m "feat: init CMake skeleton, CLI prints version"
git push origin main
```

---

## 🔧 开发工具配置

### TRAE 集成配置
1. 用 TRAE 打开路径：`\\wsl$\Ubuntu-24.04\home<user>\proj\sqlcc`
2. 自动识别 `compile_commands.json`，启用代码补全和跳转
3. 如遇问题，在 `settings.json` 中添加：
```json
"clangd.path": "/usr/bin/clangd"
```

---

## ⚡ 开发工作流

### 自动化开发脚本
```bash
cat > scripts/dev.sh <<'EOF'
#!/usr/bin/env bash
set -e
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
EOF
chmod +x scripts/dev.sh
```

### 日常开发命令
```bash
./scripts/dev.sh && git add -A && git commit -m "功能描述" && git push
```

---

## 👥 团队协作规范

### 分支管理策略
```
main（保护分支） ← develop ← feature/xxx
```

### 协作流程
1. **每日同步**：`git pull --rebase origin main`
2. **任务分配**：
   - A 负责 `buffer_pool.*`
   - B 负责 `bpt.*` 
   - C 负责 `sql_parser.*`
3. **代码审查**：Gitee Pull Request + 至少1人Review

---

## ❗ 常见问题排查

| 问题 | 解决方案 |
|------|----------|
| WSL 路径大小写问题 | 注意 `#include` 语句的大小写一致性 |
| SSH 权限错误 | 执行 `ssh-add ~/.ssh/id_ed25519` |
| Git 配置冲突 | 确保 WSL 内外 global config 一致 |

---

## ✅ 完成状态

- [x] 开发环境配置
- [x] 项目结构初始化  
- [x] CMake 构建系统配置
- [x] 首次编译测试
- [x] 代码仓库同步
- [ ] 数据库核心功能开发（下一步）

---

## 🎯 下一步计划

开始实现数据库存储引擎核心功能：
- 磁盘管理器 (disk_manager)
- 缓冲池 (buffer_pool)  
- B+树索引 (bpt)
- SQL 解析器 (sql_parser)

---

**祝大家期末大作业顺利通过！** 🚀

> *最后更新：2025年10月29日 | 作者：yinglichina, 李莹*
yinglichina@gmail.com,  1820393151@qq.com

---

## 🆕 新版本更新与补充

### 版本升级指南

**Why（为什么需要版本升级指南）**：随着项目发展，新的功能和改进不断加入，版本升级指南可以帮助用户平滑过渡到新版本，了解新特性和可能的兼容性变化，确保升级过程顺利。

**What（版本升级指南是什么）**：版本升级指南详细记录了各版本之间的变化、新功能、改进点以及可能的迁移步骤，是用户升级项目时的重要参考资料。

**Who（谁需要版本升级指南）**：所有使用SQLCC项目的用户和开发者都需要版本升级指南，因为它直接影响项目的使用和开发体验。

**How（如何使用版本升级指南）**：用户可以根据当前使用的版本，查找对应版本的升级指南，按照步骤进行升级，了解新功能的使用方法。

### v0.2.0 新特性

#### 1. 文档体系完善

**Why（为什么需要完善文档体系）**：完善的文档体系是项目成功的关键，它可以帮助新用户快速上手，为开发者提供详细的技术参考，提高项目的可用性和可维护性。

**What（文档体系完善包括什么）**：文档体系完善包括创建Architecture.md架构设计文档，更新docs/index.md文档导航中心，改进README.md项目说明，并确保各文档之间的逻辑自洽。

**Who（谁需要完善的文档体系）**：所有项目参与者，包括开发者、用户和学习者，都需要完善的文档体系。

**How（如何完善文档体系）**：通过分析用户需求，设计文档结构，编写内容，建立文档间的交叉引用，提供针对不同读者的阅读路径。

#### 2. 针对不同读者的阅读路径

**Why（为什么需要针对不同读者的阅读路径）**：不同背景的读者有不同的学习目标和知识基础，提供针对性的阅读路径可以提高学习效率，减少信息过载，满足不同读者的需求。

**What（针对不同读者的阅读路径是什么）**：针对不同读者的阅读路径是根据读者背景和需求设计的文档阅读顺序和重点，帮助读者快速找到所需信息。

**Who（谁需要针对不同读者的阅读路径）**：所有使用SQLCC文档的读者，特别是AI辅助编程学习者和DBMS学习者。

**How（如何设计针对不同读者的阅读路径）**：通过分析读者背景和需求，设计不同的阅读顺序和重点，提供针对性的指导和建议。

#### 3. 架构设计文档

**Why（为什么需要架构设计文档）**：架构设计文档是项目的技术蓝图，它详细描述了系统的整体结构、模块功能、性能要求和实现思路，是开发和维护项目的重要参考。

**What（架构设计文档是什么）**：架构设计文档详细描述了SQLCC数据库系统的整体架构，包括存储引擎、SQL解析、索引、CRUD、事务和工具六个模块的功能清单、性能要求和实现思路。

**Who（谁需要架构设计文档）**：项目开发者、系统架构师和希望深入了解系统设计的用户。

**How（如何使用架构设计文档）**：开发者可以根据架构设计文档进行模块开发，用户可以了解系统设计思路，架构师可以参考设计模式和决策。

### 新版本AI提示词与命令

#### 1. 文档生成提示词

**AI提示词**：
> "为SQLCC项目创建一个完整的文档体系，包括架构设计文档、文档导航中心、项目说明和针对不同读者的阅读路径。确保文档之间逻辑自洽，内容准确，格式统一。"

**AI响应**：
- 创建了Architecture.md架构设计文档
- 更新了docs/index.md文档导航中心
- 改进了README.md项目说明
- 设计了针对AI辅助编程学习者和DBMS学习者的阅读路径

**评价**：
- **优点**：提示词全面指定了文档体系的要求，AI响应完整，涵盖了文档体系的主要方面
- **缺点**：提示词没有指定具体的文档格式或风格指南，可能导致文档风格不一致
- **改进建议**：可以在提示词中添加更多格式要求，如"为SQLCC项目创建一个完整的文档体系，包括架构设计文档、文档导航中心、项目说明和针对不同读者的阅读路径。使用Markdown格式，遵循一致的标题层级和代码块风格，确保文档之间逻辑自洽，内容准确，格式统一"

#### 2. 架构设计提示词

**AI提示词**：
> "设计SQLCC数据库系统的架构，包括存储引擎、SQL解析、索引、CRUD、事务和工具六个模块。详细描述每个模块的功能清单、性能要求、模块关系和实现思路。使用Why-What-Who-How分析框架，确保设计合理性和可解释性。"

**AI响应**：
- 设计了六个模块的功能清单和性能要求
- 分析了模块间的关系和依赖
- 使用Why-What-Who-How框架解释了设计的合理性
- 提供了初步的实现思路和技术选型

**评价**：
- **优点**：提示词明确指定了需要设计的模块和分析框架，AI响应全面，涵盖了架构设计的主要方面
- **缺点**：提示词没有指定具体的性能指标或约束条件，可能导致设计缺乏具体的量化目标
- **改进建议**：可以在提示词中添加更多量化要求，如"设计SQLCC数据库系统的架构，包括存储引擎、SQL解析、索引、CRUD、事务和工具六个模块。详细描述每个模块的功能清单、性能要求（如10万次INSERT后文件体积≤1.2倍理论值）、模块关系和实现思路。使用Why-What-Who-How分析框架，确保设计合理性和可解释性"

#### 3. 阅读路径设计提示词

**AI提示词**：
> "为SQLCC项目设计针对不同读者的阅读路径，包括AI辅助编程学习者和DBMS学习者。为每类读者提供合适的文档阅读顺序和重点，确保他们能够高效地获取所需信息。"

**AI响应**：
- 设计了针对AI辅助编程学习者的阅读路径
- 设计了针对DBMS学习者的阅读路径
- 为每类读者提供了文档阅读顺序和重点
- 提供了快速体验者的简化路径

**评价**：
- **优点**：提示词明确指定了目标读者群体和设计要求，AI响应全面，涵盖了主要读者类型
- **缺点**：提示词没有指定具体的读者背景或学习目标，可能导致阅读路径设计不够精准
- **改进建议**：可以在提示词中添加更多读者背景信息，如"为SQLCC项目设计针对不同读者的阅读路径，包括AI辅助编程学习者和DBMS学习者。AI辅助编程学习者希望学习如何利用AI工具构建数据库系统，DBMS学习者希望了解和使用SQLCC数据库。为每类读者提供合适的文档阅读顺序和重点，确保他们能够高效地获取所需信息"

### 升级步骤

#### 从v0.1.0升级到v0.2.0

1. **更新文档**：
   ```bash
   git pull origin main
   # 查看新增的Architecture.md、更新的docs/index.md和README.md
   ```

2. **了解新功能**：
   - 阅读[Architecture.md](Architecture.md)了解系统架构
   - 查看[docs/index.md](docs/index.md)了解文档导航
   - 阅读README.md中的阅读路径指南

3. **调整开发流程**：
   - 根据Architecture.md中的设计思路进行开发
   - 参考文档导航找到所需信息
   - 根据自身背景选择合适的阅读路径

### 未来版本计划

#### v0.3.0计划特性

1. **SQL解析器实现**：完成SQL语句的解析和AST生成
2. **B+树索引实现**：完成B+树索引结构和操作
3. **CRUD操作实现**：完成基本的数据操作功能
4. **事务系统实现**：完成WAL日志和锁管理

#### v0.4.0计划特性

1. **查询优化器**：实现基本的查询优化策略
2. **并发控制**：完善多线程并发访问控制
3. **性能优化**：优化系统性能和资源使用
4. **工具完善**：完善命令行工具和管理功能

---

**最后更新：2025年11月06日 | 作者：yinglichina, 李莹**
yinglichina@gmail.com,  1820393151@qq.com