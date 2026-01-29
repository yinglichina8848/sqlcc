# SQLCC 构建和编译测试指南

## 📋 文档概述

本文档提供SQLCC数据库系统的完整构建、编译和测试指南，涵盖从环境配置到自动化测试的完整流程。基于Bazel构建系统和Google Test框架，确保开发者能够快速搭建开发环境并验证代码质量。

---

## 🛠️ 环境配置

### 系统要求
- **操作系统**: Ubuntu 20.04+ / CentOS 7+ / macOS 10.15+
- **内存**: 至少4GB RAM（推荐8GB+）
- **存储**: 至少10GB可用磁盘空间
- **网络**: 稳定的互联网连接（下载依赖）

### 必需工具
```bash
# Ubuntu/Debian
sudo apt update
sudo apt install -y build-essential openjdk-11-jdk python3 git curl unzip

# CentOS/RHEL
sudo yum groupinstall -y "Development Tools"
sudo yum install -y java-11-openjdk-devel python3 git curl unzip

# macOS (使用Homebrew)
brew install openjdk@11 python3 git curl unzip
```

### Bazel安装
```bash
# 下载并安装Bazel
curl -fsSL https://bazel.build/bazel-release.pub.gpg | gpg --deimport
curl -Lo bazel https://github.com/bazelbuild/bazel/releases/download/4.2.1/bazel-4.2.1-linux-x86_64
chmod +x bazel
sudo mv bazel /usr/local/bin/

# 验证安装
bazel version
```

---

## 🏗️ 项目构建

### 获取源码
```bash
# 克隆项目
git clone https://gitee.com/yinglichina/sqlcc.git
cd sqlcc

# 初始化子模块（如有）
git submodule update --init --recursive
```

### 构建配置
```bash
# 查看构建目标
bazel query //...

# 构建所有目标
bazel build //...

# 构建特定模块
bazel build //src/core:sqlcc_core
bazel build //src/storage_engine:sqlcc_storage

# 构建可执行程序
bazel build //:sqlcc
```

### 编译选项
```bash
# 调试版本（默认）
bazel build //... --compilation_mode=dbg

# 优化版本
bazel build //... --compilation_mode=opt

# 启用asan（地址消毒器）
bazel build //... --config=asan

# 并行编译（指定CPU核心数）
bazel build //... --jobs=8

# 增量编译
bazel build //... --keep_going
```

---

## 🧪 测试执行

### 运行测试
```bash
# 运行所有测试
bazel test //...

# 运行特定测试模块
bazel test //src/storage_engine:buffer_pool_test
bazel test //src/sql_parser:parser_test

# 运行测试套件
bazel test //tests/unit/...
bazel test //tests/integration/...

# 带详细输出的测试
bazel test //... --test_output=errors
bazel test //... --test_output=all
```

### 测试配置
```bash
# 测试超时设置
bazel test //... --test_timeout=60

# 运行特定次数
bazel test //... --runs_per_test=3

# 随机种子（用于重现随机测试）
bazel test //... --test_random_seed=12345

# 失败时停止
bazel test //... --keep_going=false
```

### 端到端测试
```bash
# 运行SQL功能测试
./test_basic_sql.sh

# 运行综合测试
./test_comprehensive_sqlcc.sh

# 运行ISQL客户端测试
./test_isql_sqlcc.sh

# 运行存储集成测试
./test_storage_integration.sh
```

### 自动化测试流水线 ⭐新增

SQLCC v1.1.4 引入了完整的自动化测试流水线，提供一键式测试执行体验。

#### 流水线快速开始
```bash
# 执行完整测试流水线
./scripts/test_pipeline.sh

# 查看实时日志
tail -f test_reports/pipeline_report_*.txt

# 查看最终报告
cat test_reports/final_report_*.txt
```

#### 流水线执行模式
```bash
# 仅构建验证（快速检查）
BUILD_ONLY=true ./scripts/test_pipeline.sh

# 跳过通信测试（当网络环境不稳定时）
COMMUNICATION_TEST=false ./scripts/test_pipeline.sh

# 仅覆盖率分析
COVERAGE_ENABLED=true ./scripts/test_pipeline.sh

# 快速测试模式（减少测试范围）
TEST_MODE=quick ./scripts/test_pipeline.sh
```

#### 流水线组件说明

| 脚本 | 功能 | 执行时间 | 输出 |
|------|------|----------|------|
| `prepare_test_environment.sh` | 环境清理和准备 | ~10秒 | 环境状态 |
| `validate_build.sh` | 项目构建验证 | ~30秒 | 构建产物 |
| `run_unit_tests.sh` | 单元测试执行 | ~45秒 | 测试结果 |
| `run_integration_tests.sh` | 集成测试执行 | ~60秒 | 测试结果 |
| `test_communication_protocol.sh` | 通信协议测试 | ~120秒 | 协议验证 |
| `run_e2e_tests.sh` | 端到端测试 | ~180秒 | E2E结果 |
| `collect_coverage_data.sh` | 覆盖率数据收集 | ~90秒 | 覆盖率报告 |
| `generate_test_report.sh` | 综合报告生成 | ~15秒 | 最终报告 |

#### 单独执行测试步骤
```bash
# 仅环境准备
./scripts/prepare_test_environment.sh

# 仅构建验证
./scripts/validate_build.sh

# 仅单元测试
./scripts/run_unit_tests.sh

# 仅覆盖率分析
./scripts/collect_coverage_data.sh

# 查看覆盖率报告
firefox coverage_report/index.html
```

---

## 📊 覆盖率分析

### 生成覆盖率报告
```bash
# 构建覆盖率版本
bazel build //... --collect_code_coverage

# 运行测试并收集覆盖率
bazel test //... --collect_code_coverage --combined_report=lcov

# 生成HTML报告
genhtml bazel-out/_coverage/_coverage_report.dat \
        --output-directory coverage_report \
        --title "SQLCC Coverage Report" \
        --show-details \
        --legend

# 查看报告
firefox coverage_report/index.html
```

### 覆盖率工具安装
```bash
# Ubuntu/Debian
sudo apt install -y lcov

# CentOS/RHEL
sudo yum install -y lcov

# macOS
brew install lcov
```

### 覆盖率阈值检查
```bash
# 检查整体覆盖率
lcov --summary bazel-out/_coverage/_coverage_report.dat

# 生成覆盖率徽章数据
lcov --summary bazel-out/_coverage/_coverage_report.dat | \
grep "lines......:" | sed 's/.*lines......: \([0-9.]*\).*/\1/'
```

---

## 🔍 静态分析

### 代码格式检查
```bash
# 安装clang-format
sudo apt install -y clang-format

# 检查代码格式
find src include -name "*.cpp" -o -name "*.h" | \
xargs clang-format --dry-run --Werror --style=file

# 自动格式化代码
find src include -name "*.cpp" -o -name "*.h" | \
xargs clang-format -i --style=file
```

### 代码质量检查
```bash
# 安装cppcheck
sudo apt install -y cppcheck

# 运行静态分析
cppcheck --enable=all --std=c++17 --language=c++ \
         --suppress=missingIncludeSystem \
         --inline-suppr \
         --xml --xml-version=2 \
         src/ include/ 2> cppcheck_results.xml

# 生成HTML报告
cppcheck-htmlreport --file=cppcheck_results.xml \
                    --report-dir=cppcheck_report \
                    --source-dir=.
```

### 内存泄漏检测
```bash
# 使用asan构建
bazel build //... --config=asan

# 运行测试
bazel test //... --run_under="asan"

# 或者使用valgrind
sudo apt install -y valgrind

# 运行程序检测内存泄漏
valgrind --leak-check=full \
         --show-leak-kinds=all \
         --track-origins=yes \
         ./bazel-bin/sqlcc
```

---

## 🚀 性能测试

### 运行性能基准
```bash
# 构建性能测试
bazel build //test_performance_real

# 运行CRUD性能测试
./bazel-bin/test_performance_real

# 查看结果
cat performance_test_real_results.md
```

### 性能分析工具
```bash
# 安装perf工具
sudo apt install -y linux-tools-common linux-tools-generic

# 性能剖析
perf record -g ./bazel-bin/sqlcc
perf report

# 使用火焰图
sudo apt install -y python3-flamegraph
perf script | stackcollapse-perf.pl | flamegraph.pl > flamegraph.svg
```

### 负载测试
```bash
# 安装sysbench（如果需要）
sudo apt install -y sysbench

# 运行数据库负载测试
sysbench --db-driver=mysql \
         --mysql-host=localhost \
         --mysql-port=18647 \
         --mysql-user=root \
         --mysql-password= \
         --mysql-db=test \
         --table-size=10000 \
         --tables=10 \
         --events=0 \
         --time=60 \
         --threads=8 \
         oltp_read_write run
```

---

## 🔧 调试技巧

### GDB调试
```bash
# 构建调试版本
bazel build //:sqlcc --compilation_mode=dbg

# 启动GDB
gdb ./bazel-bin/sqlcc

# 设置断点
(gdb) break main
(gdb) break BufferPool::FetchPage

# 运行程序
(gdb) run --port 18647

# 查看调用栈
(gdb) bt

# 查看变量
(gdb) print page_id
(gdb) info locals
```

### 日志调试
```bash
# 启用详细日志
export SQLCC_LOG_LEVEL=DEBUG
export SQLCC_LOG_FILE=sqlcc_debug.log

# 运行程序
./bazel-bin/sqlcc --port 18647

# 查看日志
tail -f sqlcc_debug.log
```

### 核心转储分析
```bash
# 启用核心转储
ulimit -c unlimited
echo "core.%e.%p.%t" > /proc/sys/kernel/core_pattern

# 运行程序（崩溃时生成core文件）
./bazel-bin/sqlcc

# 分析核心转储
gdb ./bazel-bin/sqlcc core.sqlcc.12345.1609459200

# 在GDB中查看崩溃信息
(gdb) bt
(gdb) info registers
```

---

## 🤖 自动化脚本

### CI/CD脚本示例
```bash
#!/bin/bash
# ci_build_test.sh

set -e  # 遇到错误立即退出

echo "=== SQLCC CI/CD Pipeline ==="

# 1. 环境检查
echo "检查构建环境..."
bazel version
clang-format --version

# 2. 代码格式检查
echo "检查代码格式..."
if ! find src include -name "*.cpp" -o -name "*.h" | \
     xargs clang-format --dry-run --Werror --style=file; then
    echo "代码格式检查失败"
    exit 1
fi

# 3. 构建项目
echo "构建项目..."
bazel build //...

# 4. 运行单元测试
echo "运行单元测试..."
bazel test //tests/unit/... --test_output=errors

# 5. 运行集成测试
echo "运行集成测试..."
bazel test //tests/integration/...

# 6. 生成覆盖率报告
echo "生成覆盖率报告..."
bazel coverage //... --combined_report=lcov
genhtml bazel-out/_coverage/_coverage_report.dat \
        --output-directory coverage_report

# 7. 性能测试
echo "运行性能测试..."
bazel run //test_performance_real

# 8. 静态分析
echo "运行静态分析..."
cppcheck --enable=all --std=c++17 src/ include/ 2>/dev/null || true

echo "=== CI/CD Pipeline 完成 ==="
```

### 本地开发脚本
```bash
#!/bin/bash
# dev_setup.sh

echo "=== SQLCC 开发环境设置 ==="

# 安装依赖
sudo apt update
sudo apt install -y build-essential openjdk-11-jdk python3 \
                     clang-format cppcheck lcov valgrind git curl

# 安装Bazel
if ! command -v bazel &> /dev/null; then
    curl -Lo bazel https://github.com/bazelbuild/bazel/releases/download/4.2.1/bazel-4.2.1-linux-x86_64
    chmod +x bazel
    sudo mv bazel /usr/local/bin/
fi

# 初始构建
echo "执行初始构建..."
bazel build //...

# 运行基础测试
echo "运行基础测试..."
bazel test //tests/unit:buffer_pool_test

echo "=== 开发环境设置完成 ==="
echo "常用命令："
echo "  bazel build //...          # 构建所有"
echo "  bazel test //...           # 运行所有测试"
echo "  bazel run //:sqlcc         # 运行服务器"
echo "  ./test_basic_sql.sh        # 运行SQL测试"
```

---

## 📋 故障排除

### 常见构建问题

#### 问题：Bazel找不到依赖
```bash
# 解决方案：清理缓存
bazel clean --expunge
bazel build //...
```

#### 问题：编译器版本不匹配
```bash
# 检查GCC版本
gcc --version

# 安装特定版本
sudo apt install -y gcc-9 g++-9
export CC=gcc-9 CXX=g++-9
```

#### 问题：内存不足
```bash
# 减少并行度
bazel build //... --jobs=2

# 增加交换空间
sudo fallocate -l 8G /swapfile
sudo chmod 600 /swapfile
sudo mkswap /swapfile
sudo swapon /swapfile
```

### 测试问题排查

#### 问题：测试超时
```bash
# 增加超时时间
bazel test //... --test_timeout=300

# 或者调试特定测试
bazel test //specific:test --test_output=all
```

#### 问题：内存泄漏检测失败
```bash
# 确保asan正确配置
bazel build //... --config=asan --copt=-fsanitize=address

# 运行测试
ASAN_OPTIONS=detect_leaks=1 bazel test //...
```

### 性能问题诊断

#### 问题：构建速度慢
```bash
# 启用构建缓存
bazel build //... --disk_cache=/tmp/bazel-cache

# 使用ccache（如果安装）
export CC="ccache gcc" CXX="ccache g++"
```

#### 问题：测试运行慢
```bash
# 并行运行测试
bazel test //... --jobs=8 --test_jobs=4

# 运行特定慢测试
bazel test //slow:test --test_timeout=600
```

---

## 📊 质量门禁

### 提交前检查清单
- [ ] `bazel build //...` 构建成功
- [ ] `bazel test //...` 所有测试通过
- [ ] 代码格式检查通过
- [ ] 覆盖率不低于50%
- [ ] 无内存泄漏
- [ ] 静态分析无严重问题

### 质量指标
- **构建成功率**: 100%
- **测试通过率**: ≥95%
- **代码覆盖率**: ≥50%
- **性能基准**: 满足设计要求
- **静态检查**: 0严重问题

---

**文档版本**: v1.1.4
**更新日期**: 2025年12月14日
**适用对象**: 开发者、测试工程师、CI/CD管理员
