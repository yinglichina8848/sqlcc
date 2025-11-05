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