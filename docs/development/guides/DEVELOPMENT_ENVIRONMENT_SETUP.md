# 开发环境安装与配置指南

## 🎯 前言

本文档为**SQLCC项目**提供完整的开发环境搭建指南。目标是在**Ubuntu 20.04+**环境下配置一个专业的C++20数据库开发环境，使用Bazel构建系统和Clang 20编译器。

**预计时间**: 30-60分钟
**难度级别**: ⭐⭐ (中等)
**适用对象**: 数据库开发者、C++开发者

---

## 📋 环境要求总览

### 🖥️ **推荐配置**
```bash
OS:      Ubuntu 22.04 LTS (或更高版本)
CPU:     Intel i5-10400 (6核) 或 AMD Ryzen 5 5600 (6核)
内存:    32GB DDR4 (最低16GB)
存储:    512GB SSD (NVMe优先)
网络:    稳定网络连接 (下载依赖包)
```

### 🔧 **软件环境要求**
- **[Clang 20+](https://clang.llvm.org/)**: C++20编译器和LLVM工具链
- **[Bazel 8.5.0+](https://bazel.build/)**: 构建系统
- **[Google Test 1.14.0](https://github.com/google/googletest)**: 单元测试框架
- **[Python 3.10+](https://www.python.org/)**: 脚本和工具开发
- **[Git](https://git-scm.com/)**: 版本控制
- **[Doxygen](https://www.doxygen.nl/)**: 文档生成工具

---

## 🚀 环境安装步骤

### 步骤1: 更新系统包管理器

```bash
# 更新包列表
sudo apt update

# 升级已安装的包
sudo apt upgrade -y

# 清理无用包
sudo apt autoremove -y
```

### 步骤2: 安装基础开发工具

```bash
# 安装构建工具
sudo apt install -y build-essential python3 python3-pip

# 安装版本控制
sudo apt install -y git

# 安装代码编辑器 (可选)
sudo apt install -y vim neovim emacs-nox

# 安装网络工具
sudo apt install -y curl wget
```

### 步骤3: 安装Clang 20和LLVM工具链

```bash
# 安装Clang 20和LLVM 20
sudo apt install -y clang-20 clang++-20 llvm-20 llvm-cov-20 llvm-profdata-20 libc++-20-dev libc++abi-20-dev

# 设置默认编译器（可选）
sudo update-alternatives --install /usr/bin/clang clang /usr/bin/clang-20 100
sudo update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-20 100

# 验证安装
clang-20 --version
clang++-20 --version
llvm-cov-20 --version
```

### 步骤4: 安装Bazel构建系统

```bash
# 下载Bazel 8.5.0
wget https://github.com/bazelbuild/bazel/releases/download/8.5.0/bazel-8.5.0-linux-x86_64

# 安装Bazel
chmod +x bazel-8.5.0-linux-x86_64
sudo mv bazel-8.5.0-linux-x86_64 /usr/local/bin/bazel

# 验证安装
bazel --version
```

### 步骤5: 安装Google Test测试框架

```bash
# Google Test将通过Bazel自动下载
# 无需手动安装

# 验证Bazel可以访问Google Test
bazel query @com_google_googletest//...
```

### 步骤6: 安装推荐工具

```bash
# 代码格式化
sudo apt install -y clang-format-20

# 静态分析
sudo apt install -y clang-tidy-20 cppcheck

# 性能分析
sudo apt install -y linux-tools-common valgrind perf

# 文档生成
sudo apt install -y doxygen graphviz

# Python开发工具
pip3 install pre-commit black isort mypy pylint

# 验证安装
clang-format-20 --version
clang-tidy-20 --version
```

### 步骤4: 安装Doxygen文档生成工具

```bash
# 安装Doxygen
sudo apt install -y doxygen doxygen-latex graphviz

# 安装Latex支持 (可选，用于生成PDF文档)
sudo apt install -y texlive-latex-base texlive-fonts-recommended

# 验证安装
doxygen --version
```

### 步骤5: 安装代码覆盖率工具

```bash
# 安装lcov (gcov的前端)
sudo apt install -y lcov

# 验证gcov (GCC内置)
gcov --version

# 验证lcov
lcov --version
```

### 步骤6: 配置Git环境

```bash
# 配置用户信息
git config --global user.name "Your Name"
git config --global user.email "your.email@example.com"

# 配置默认分支名
git config --global init.defaultBranch main

# 配置颜色显示
git config --global color.ui auto

# 配置行结束符处理
git config --global core.autocrlf input

# 验证配置
git config --list --show-origin
```

---

## 🧪 项目编译测试

### 步骤1: 下载项目源码

```bash
# 克隆项目
git clone https://gitee.com/yinglichina/sqlcc.git
cd sqlcc

# 查看项目结构
ls -la
tree -L 2  # 如果安装了tree命令
```

### 步骤2: 创建构建目录

```bash
# 创建构建目录
mkdir -p build
cd build

# 配置CMake项目
cmake .. -DCMAKE_BUILD_TYPE=Debug

# 或者配置Release版本 (用于性能测试)
cmake .. -DCMAKE_BUILD_TYPE=Release

# 或者配置覆盖率版本 (用于生成覆盖率报告)
cmake .. -DCMAKE_BUILD_TYPE=Coverage
```

### 步骤3: 编译项目

```bash
# 编译项目 (使用多核编译)
bazel build //...

# 验证编译结果
ls -la bin/  # 可执行程序
ls -la lib/  # 库文件 (如果有)
```

### 步骤4: 运行基础测试

```bash
# 运行单元测试
bazel test //...  # 或 ./bin/sqlcc_test

# 检查测试输出
# 应该看到所有测试通过的信息
```

---

## 📚 生成文档验证

### 生成API文档 (Doxygen)

```bash
# 生成Doxygen文档
make docs  # 或 doxygen Doxyfile

# 查看生成的文档
ls -la docs/doxygen/html/

# 在浏览器中打开
xdg-open docs/doxygen/html/index.html
```

### 生成代码覆盖率报告

```bash
# 运行覆盖率测试
bazel coverage //...

# 查看生成的报告
ls -la coverage/

# 在浏览器中打开
xdg-open coverage/index.html
```

**期望结果**:
- ✅ 覆盖率报告显示 ≥85% 行覆盖率
- ✅ API文档包含完整的类和函数文档
- ✅ 所有测试用例状态为 PASSED

---

## 🎛️ 高级配置选项

### 配置别名 (可选，方便使用)

在 `~/.bashrc` 或 `~/.zshrc` 中添加:

```bash
# SQLCC项目开发别名
alias sqlcc-build='cd ~/sqlcc/build && cmake .. && bazel build //...'
alias sqlcc-test='cd ~/sqlcc/build && bazel test //...'
alias sqlcc-coverage='cd ~/sqlcc/build && bazel coverage //... && xdg-open coverage/index.html'
alias sqlcc-docs='cd ~/sqlcc/build && make docs && xdg-open docs/doxygen/html/index.html'
alias sqlcc-run='cd ~/sqlcc/build && ./bin/isql'
```

应用配置:
```bash
source ~/.bashrc  # 或重新打开终端
```

### IDE配置 (推荐使用字节Trae)

1. **安装字节Trae**: 从官方网站下载并安装
2. **打开项目**: `File` → `Open Folder` → 选择`sqlcc`目录
3. **配置C++编译器**: 在设置中指定GCC路径
4. **安装C++扩展**: C/C++ (Microsoft), CMake Tools

---

## 🚨 故障排除

### 常见问题及解决方案

#### 问题1: CMake版本过低
```bash
# 错误信息: CMake version X.X.X required >= 3.16
# 解决方案:
sudo apt install -y software-properties-common
sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
sudo apt update && sudo apt install -y cmake
```

#### 问题2: CppUnit未找到
```bash
# 错误信息: Could NOT find CppUnit
# 解决方案:
sudo apt update && sudo apt install -y libcppunit-dev cppunit
```

#### 问题3: 编译错误 (GCC版本)
```bash
# 错误信息: GCC version too old for C++17
# 解决方案:
sudo apt install -y gcc-9 g++-9
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-9 90
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-9 90
```

#### 问题4: Git克隆失败
```bash
# 错误信息: Permission denied (publickey)
# 解决方案: 配置SSH密钥 (见SSH配置文档)
# 或使用HTTPS方式:
git clone https://gitee.com/yinglichina/sqlcc.git
```

#### 问题5: Doxygen文档生成失败
```bash
# 错误信息: dot not found
# 解决方案:
sudo apt install -y graphviz plantuml
```

---

## 🎯 环境验证清单

使用以下脚本验证环境完整性:

```bash
#!/bin/bash
# SQLCC环境验证脚本

echo "🔍 SQLCC开发环境验证"
echo "======================="

# 检查工具版本
echo "GCC版本: $(gcc --version | head -n1)"
echo "CMake版本: $(cmake --version | head -n1)"
echo "Git版本: $(git --version)"

# 检查依赖库
echo "CppUnit版本: $(pkg-config --modversion cppunit 2>/dev/null || echo '未找到')"
echo "Doxygen版本: $(doxygen --version 2>/dev/null || echo '未找到')"

# 检查项目
if [ -d "src" ] && [ -d "include" ]; then
    echo "✅ 项目结构完整"
else
    echo "❌ 项目结构不完整"
fi

# 检查编译
if [ -f "build/bin/isql" ]; then
    echo "✅ 项目已编译"
else
    echo "❓ 项目未编译，运行 'mkdir build && cd build && cmake .. && make'"
fi

# 检查测试
if bazel test //... >/dev/null 2>&1; then
    echo "✅ 单元测试通过"
else
    echo "❌ 单元测试失败"
fi

echo "======================="
echo "🎉 环境验证完成！"
```

---

## 📞 获取帮助

如果在环境配置过程中遇到问题:

1. **首先检查**: 是否按照文档步骤完整执行
2. **查看错误**: 仔细阅读错误信息，尝试理解问题
3. **搜索解决**: 在Google/GitHub搜索错误信息
4. **寻求帮助**: 在项目Issues中提问，或联系课程助教

**关键提醒**: 数据库开发环境配置相对复杂，请耐心对待每一环节。

---

**🚀 环境配置完成后，你就可以开始激动人心的数据库系统开发之旅了！**
