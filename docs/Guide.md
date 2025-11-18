# SQLCC 项目使用指南 (v0.3.6)

本指南将帮助您了解如何使用SQLCC（Simple C++ Database Storage Engine）项目，包括代码结构、构建方法、测试运行和文档查看。

## 项目概述

SQLCC是一个简单的C++数据库存储引擎，实现了基本的页式存储管理、缓冲池和LRU替换策略。本项目展示了如何利用AI辅助工具进行高效的软件开发。

**当前版本**：v0.3.6 (2025-11-12)
**主要特性**：
- 基本页式存储管理
- 缓冲池和LRU替换策略
- 完善的设计文档和教学价值
- 详细的Doxygen注释，采用Why-What-How格式
- 线程安全锁定机制
- 批量预取优化
- 性能测试套件

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
│   ├── version.h          # 版本信息
│   └── config_manager.h   # 配置管理器
├── src/                   # 源代码目录
│   ├── page.cc            # 页面类实现
│   ├── disk_manager.cc    # 磁盘管理器类实现
│   ├── buffer_pool.cc     # 缓冲池类实现
│   ├── storage_engine.cc  # 存储引擎类实现
│   ├── logger.cc          # 日志类实现
│   ├── exception.cc       # 异常类实现
│   └── config_manager.cc  # 配置管理器实现
├── tests/                 # 测试目录
│   ├── storage_engine_test.cc  # 单元测试文件
│   ├── performance_tests/      # 性能测试目录
│   └── integration_tests/      # 集成测试目录
├── docs/                  # 文档目录
│   ├── index.md           # 文档索引
│   ├── storage_engine_design.md  # 存储引擎设计文档
│   ├── unit_testing.md    # 单元测试文档
│   ├── BRANCHES.md        # 分支说明文档
│   ├── doxygen/           # Doxygen生成的API文档
│   └── performance_reports/     # 性能测试报告
├── scripts/               # 脚本目录
│   ├── generate_docs.sh   # 文档生成脚本
│   ├── run_tests.sh       # 测试运行脚本
│   └── performance_tests.sh     # 性能测试脚本
├── build/                 # 构建目录（运行cmake后生成）
├── build-coverage/        # 覆盖率测试构建目录
├── CMakeLists.txt         # CMake配置文件
├── Doxyfile              # Doxygen配置文件
├── Makefile              # Makefile，提供便捷命令
├── README.md             # 项目说明文档
├── ChangeLog.md          # 变更日志
├── TRAE-Chat.md          # AI辅助交互记录
├── Architecture.md       # 项目架构文档
└── .gitignore            # Git忽略文件配置
```

## 环境要求

- C++11或更高版本的编译器
- CMake 3.10或更高版本
- Google Test框架
- Doxygen（用于生成API文档）
- gcov（用于代码覆盖率测试）

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

# 安装代码覆盖率工具
sudo apt install gcov lcov
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

#### 单元测试
```bash
# 运行所有单元测试
./build/bin/storage_engine_test

# 运行特定测试
./build/bin/storage_engine_test --gtest_filter=StorageEngineTest.DeletePage

# 使用Makefile运行测试
make test
```

#### 性能测试
```bash
# 运行CPU密集型性能测试
./build/bin/cpu_intensive_test

# 运行长期稳定性测试
./build/bin/long_term_stability_test

# 运行内存压力测试
./build/bin/memory_pressure_test

# 或者使用性能测试脚本
./scripts/performance_tests.sh
```

#### 代码覆盖率测试
```bash
# 构建覆盖率版本
make coverage

# 查看覆盖率报告
open build-coverage/index.html  # macOS
xdg-open build-coverage/index.html  # Linux
```

## 文档查看

### 1. 查看基础文档

```bash
# 查看文档索引
cat docs/index.md

# 查看存储引擎设计文档
cat docs/storage_engine_design.md

# 查看单元测试文档
cat docs/unit_testing.md

# 查看项目架构文档
cat Architecture.md
```

### 2. 查看API文档

```bash
# 在浏览器中打开API文档
xdg-open docs/doxygen/html/index.html  # Linux
open docs/doxygen/html/index.html      # macOS
start docs/doxygen/html/index.html     # Windows
```

### 3. 生成最新文档

```bash
# 生成API文档
make docs

# 清理文档
make clean-docs

# 查看生成的文档
open docs/doxygen/html/index.html
```

## 性能测试详解

### CPU密集型性能测试
测试存储引擎在CPU密集型操作下的表现：
- 查询处理性能
- 索引操作性能
- 事务处理性能

测试结果保存在：`build/cpu_intensive_performance_results.csv`

### 长期稳定性测试
测试系统在长时间运行下的稳定性：
- 连续运行24小时
- 内存泄漏检测
- 性能退化分析

测试结果保存在：`build/long_term_stability_results.csv`

### 内存压力测试
测试系统在内存受限环境下的表现：
- 内存使用峰值监控
- 缓存命中率分析
- 垃圾回收效率

## 常用命令

### 构建和清理
```bash
# 构建项目
make

# 清理构建文件
make clean

# 完全清理（包括文档）
make distclean
```

### 测试相关
```bash
# 运行所有测试
make test

# 运行特定测试
make test-unit
make test-performance
make test-integration

# 生成覆盖率报告
make coverage
```

### 文档相关
```bash
# 生成API文档
make docs

# 清理文档
make clean-docs

# 查看文档
open docs/doxygen/html/index.html
```

## 性能监控

### 实时监控
```bash
# 监控内存使用
watch -n 1 'ps aux | grep storage_engine_test'

# 监控CPU使用
top -p $(pgrep storage_engine_test)
```

### 性能分析
```bash
# 使用gprof进行性能分析
make profile
gprof build/bin/storage_engine_test gmon.out > profile.txt

# 使用valgrind进行内存分析
valgrind --tool=memcheck ./build/bin/storage_engine_test
```

## 故障排除

### 常见问题

1. **编译错误**
   ```bash
   # 清理重新构建
   make clean
   make
   ```

2. **测试失败**
   ```bash
   # 重新运行特定测试
   ./build/bin/storage_engine_test --gtest_filter=TestName
   ```

3. **性能下降**
   ```bash
   # 检查系统资源
   htop
   iotop
   ```

4. **内存泄漏**
   ```bash
   # 使用valgrind检测
   valgrind --leak-check=full ./build/bin/storage_engine_test
   ```

### 日志分析
```bash
# 查看详细日志
tail -f logs/storage_engine.log

# 分析错误日志
grep ERROR logs/storage_engine.log

# 性能日志分析
grep PERFORMANCE logs/storage_engine.log
```

## 开发和贡献

### 代码风格
- 遵循Google C++ Style Guide
- 使用Doxygen注释格式
- 保持函数和类的单一职责原则

### 提交规范
```bash
# 功能提交
git commit -m "feat: add new storage engine feature"

# 修复提交
git commit -m "fix: resolve memory leak in buffer pool"

# 文档更新
git commit -m "docs: update API documentation"
```

## 版本历史

- **v0.3.6** (2025-11-12): 综合性能测试、代码覆盖率深度分析、文档完善
- **v0.3.5** (2025-11-11): 线程安全锁定机制优化，批量预取改进
- **v0.3.0** (2025-11-09): 性能测试套件集成
- **v0.2.3** (2025-06-18): 基础存储引擎功能完善
- **v0.2.0** (2025-06-01): 初始版本发布

**最后更新**：2025-11-11 14:30:00
