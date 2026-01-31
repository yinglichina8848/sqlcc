# SQLCC 开发者快速开始指南

## 概述

欢迎加入SQLCC开发团队！本指南将帮助您快速搭建开发环境，了解项目结构，并开始贡献代码。SQLCC采用现代化的开发流程，结合AI辅助开发，让您能够快速上手并做出贡献。

## 环境准备

### 系统要求

- **操作系统**: Linux Ubuntu 20.04+ / CentOS 8+ (推荐Ubuntu 22.04+)
- **CPU**: x86_64架构，8核+推荐
- **内存**: 16GB RAM以上（推荐32GB）
- **存储**: 100GB可用空间
- **网络**: 稳定的互联网连接

### 开发工具

#### 必备工具
```bash
# C++编译器 (Clang 20+)
sudo apt install clang-20 clang++-20 libc++-20-dev libc++abi-20-dev

# Bazel构建系统 (8.5.0+)
# 使用Bazel安装脚本
wget https://github.com/bazelbuild/bazel/releases/download/8.5.0/bazel-8.5.0-linux-x86_64
chmod +x bazel-8.5.0-linux-x86_64
sudo mv bazel-8.5.0-linux-x86_64 /usr/local/bin/bazel

# 版本控制
sudo apt install git

# Python开发环境
sudo apt install python3 python3-pip

# 代码编辑器 (推荐VSCode或CLion)
# 下载并安装你喜欢的编辑器
```

#### 推荐工具
```bash
# LLVM工具链 (覆盖率分析)
sudo apt install llvm-20 llvm-cov-20 llvm-profdata-20

# 代码格式化
sudo apt install clang-format-20

# 静态分析
sudo apt install clang-tidy-20 cppcheck

# 性能分析
sudo apt install linux-tools-common valgrind perf

# 文档生成
sudo apt install doxygen graphviz

# AI开发工具
pip3 install pre-commit black isort mypy
```

## 项目获取

### 克隆仓库

```bash
# 克隆主仓库
git clone https://gitee.com/yinglichina/sqlcc.git
cd sqlcc

# 初始化子模块（如果有）
git submodule update --init --recursive

# 查看项目结构
ls -la
tree -L 2  # 需要安装tree命令
```

### 分支管理

```bash
# 查看所有分支
git branch -a

# 创建开发分支
git checkout -b feature/your-feature-name

# 保持与主分支同步
git fetch origin
git rebase origin/main
```

## 构建项目

### 快速构建（推荐）

```bash
# 使用Bazel构建所有目标
bazel build //...

# 构建特定模块
bazel build //src/core:core
bazel build //src/storage_engine:storage_engine
bazel build //src/sql_parser:sql_parser
```

### 高级构建选项

```bash
# 构建并运行测试
bazel test //...

# 构建特定层级的测试
bazel test //tests/level1_foundation:all
bazel test //tests/level2_core:all
bazel test //tests/level2_storage_engine:all

# 使用标签过滤测试
bazel test //tests/... --test_tag_filters=foundation
bazel test //tests/... --test_tag_filters=-manual,-slow

# 生成覆盖率报告（LLVM工具链）
bazel coverage //tests/level1_foundation:all

# 增量编译（快速迭代）
bazel build //... --no-cache_test_results

# 清理构建缓存
bazel clean

# 彻底清理（包括外部依赖）
bazel clean --expunge
```

### 构建产物

```bash
# 查看构建结果
ls -la bazel-bin/
ls -la bazel-out/

# 主要构建输出目录
bazel-bin/                    # 所有编译产物的符号链接
bazel-out/k8-fastbuild/bin/   # 实际的构建输出
bazel-testlogs/               # 测试日志和输出

# 可执行文件（示例）
bazel-bin/src/core/sqlcc-server       # 数据库服务器
bazel-bin/src/core/sqlcc-client       # 命令行客户端
bazel-bin/examples/demo/advanced_sql_demo  # 示例程序

# 测试可执行文件
bazel-bin/tests/level1_foundation/...   # Level 1测试
bazel-bin/tests/level2_core/...         # Level 2测试
```

## 运行和测试

### 运行测试

```bash
# 运行所有测试
bazel test //...

# 查看详细测试输出
bazel test //... --test_output=all

# 只显示失败的测试输出
bazel test //... --test_output=errors

# 运行特定层级的测试
bazel test //tests/level1_foundation:all
bazel test //tests/level2_core:all
bazel test //tests/level2_storage_engine/b_plus_tree:all

# 使用标签过滤
bazel test //tests/... --test_tag_filters=foundation
bazel test //tests/... --test_tag_filters=core
bazel test //tests/... --test_tag_filters=storage
bazel test //tests/... --test_tag_filters=-manual,-slow

# 运行单个测试
bazel test //tests/level1_foundation/exception:exception_test

# 查看测试日志
cat bazel-testlogs/tests/level1_foundation/exception/exception_test/test.log
```

### 生成覆盖率报告

```bash
# 使用LLVM覆盖率工具链生成覆盖率报告
bazel coverage //tests/level1_foundation:all

# 生成HTML覆盖率报告
mkdir -p coverage_html
genhtml --ignore-errors unsupported,inconsistent,corrupt \
  ~/.cache/bazel/_bazel_*/execroot/_main/bazel-out/_coverage/_coverage_report.dat \
  -o coverage_html

# 使用项目脚本生成报告
chmod +x scripts/generate_llvm_cov_html_report.sh
bash scripts/generate_llvm_cov_html_report.sh //tests/... coverage_html

# 查看覆盖率报告
# 在浏览器中打开 coverage_html/index.html
```

### 运行示例程序

```bash
# 构建示例程序
bazel build //examples:all

# 运行高级SQL演示
bazel-bin/examples/demo/advanced_sql_demo

# 运行事务管理演示
bazel-bin/examples/demo/demonstrate_transaction_manager

# 运行统一执行器演示
bazel-bin/examples/demo/unified_executor_demo
```

### 调试运行

```bash
# 使用GDB调试
gdb ./build/sqlcc-server
(gdb) run --dev-mode
(gdb) bt  # 崩溃时查看堆栈

# 使用Valgrind检测内存泄漏
valgrind --leak-check=full ./build/sqlcc-server --dev-mode

# 使用AddressSanitizer
export ASAN_OPTIONS=detect_leaks=1
./build/sqlcc-server --dev-mode
```

## 代码开发

### 项目结构

```
sqlcc/
├── src/                          # 源代码（按模块组织）
│   ├── core/                     # 核心数据库组件
│   ├── storage_engine/           # 存储引擎实现
│   │   ├── buffer_pool/          # V3分片缓冲池架构
│   │   ├── b_plus_tree/          # B+树索引系统
│   │   ├── table_storage/        # 表存储管理
│   │   ├── disk_manager/         # 磁盘I/O管理
│   │   └── index_manager/        # 索引管理器
│   ├── sql_parser/               # SQL解析器（ParserNew架构）
│   │   ├── parsers/              # 各类SQL解析器
│   │   ├── ast/                  # 抽象语法树
│   │   └── function/             # 函数解析
│   ├── transaction/              # 事务管理器（ACID、WAL、2PL）
│   ├── execution_ast/            # SQL执行引擎
│   ├── network/                  # 网络通信（AES/TLS加密）
│   ├── exception/                # 异常处理系统
│   ├── logger/                   # 日志系统
│   ├── types/                    # 类型系统
│   ├── config_manager/           # 配置管理器
│   └── utils/                    # 工具类
├── tests/                        # 分层测试架构
│   ├── level1_foundation/        # 基础层（异常、日志、配置、类型、工具类）
│   ├── level2_core/              # 核心层（存储引擎、缓冲池）
│   ├── level2_storage_engine/    # 存储引擎专项测试
│   ├── level3_transaction_manager/  # 事务管理测试
│   ├── level4_sql_processing/    # SQL处理测试
│   ├── level5_network/           # 网络通信测试
│   ├── level6_integration/       # 集成测试
│   └── level7_integration/       # 高级集成测试
├── tools/                        # 开发工具（Python脚本）
├── scripts/                      # 构建和测试脚本
├── docs/                         # 项目文档
│   ├── index.md                  # 文档索引
│   ├── ai_tools/                 # AI工具和规范
│   ├── api/                      # API文档
│   ├── design/                   # 设计文档
│   ├── development/              # 开发指南
│   ├── project/                  # 项目管理
│   ├── releases/                 # 版本发布
│   ├── reports/                  # 分析报告
│   └── testing/                  # 测试文档
├── examples/                     # 示例代码
├── data/                         # 数据文件
├── backups/                      # 备份目录
├── temporary/                    # 临时文件目录
├── third_party/                  # 第三方依赖
├── BUILD.bazel                   # 根目录构建配置
├── MODULE.bazel                  # Bazel模块配置
├── WORKSPACE                     # Bazel工作区配置
├── .bazelrc                      # Bazel配置文件
├── README.md                     # 项目说明
├── CHANGELOG.md                  # 变更日志
├── AGENTS.md                     # AI代理编码指南
└── VERSION                       # 版本信息
```

### 编码规范

#### 基本要求
- **智能指针优先**: 95%+代码使用`std::unique_ptr`、`std::shared_ptr`
- **RAII模式**: 所有资源使用RAII模式管理
- **异常安全**: 强异常安全保证
- **命名规范**: 类用PascalCase，函数用snake_case

#### 注释标准
采用Why-What-How三层注释体系：

```cpp
/**
 * WHY: 为什么需要这个类？
 * 解释设计决策和架构选择
 *
 * WHAT: 这个类做什么？
 * 描述功能和接口
 *
 * HOW: 如何实现的？
 * 说明关键的技术实现细节
 */
class ExampleClass {
public:
    /**
     * WHY: 为什么这个函数很重要？
     * WHAT: 执行什么操作？
     * HOW: 使用了什么算法？
     */
    void important_function();
};
```

### 开发工作流

#### 1. 选择任务

```bash
# 查看TODO列表
cat docs/development/TODO.md

# 查看GitHub Issues
# https://github.com/sqlcc/sqlcc/issues

# 选择适合自己的任务
```

#### 2. 创建分支

```bash
# 从main分支创建特性分支
git checkout main
git pull origin main
git checkout -b feature/add-new-feature

# 或者修复bug
git checkout -b fix/bug-description
```

#### 3. 编写代码

```bash
# 遵循编码规范
# 添加必要的注释
# 编写测试代码

# 格式化代码
clang-format -i src/your_file.cpp include/your_header.h

# 静态检查
clang-tidy src/your_file.cpp
```

#### 4. 编写测试

```bash
# 创建测试文件
touch tests/unit/your_component_test.cpp

# 编写单元测试
#include <gtest/gtest.h>
#include "your_component.h"

TEST(YourComponentTest, BasicFunctionality) {
    // 测试代码
}

# 运行测试
cd build && make test
```

#### 5. 提交代码

```bash
# 添加文件
git add .

# 提交（使用约定式提交格式）
git commit -m "feat: add new feature description

- What was changed
- Why it was changed
- How it was implemented"

# 推送到远程
git push origin feature/add-new-feature
```

#### 6. 创建Pull Request

```bash
# 在GitHub上创建PR
# 填写详细的PR描述
# 等待代码审查
# 根据反馈修改代码
```

## 调试技巧

### 常见调试场景

#### 内存问题调试
```cpp
# 使用AddressSanitizer
cmake -DCMAKE_BUILD_TYPE=Debug -DENABLE_SANITIZERS=ON ..
make
ASAN_OPTIONS=detect_leaks=1 ./build/sqlcc-server

# 使用Valgrind
valgrind --leak-check=full --show-leak-kinds=all ./build/sqlcc-server
```

#### 性能问题分析
```cpp
# 使用perf
perf record -g ./build/sqlcc-server
perf report

# 使用火焰图
# 安装perf-map-agent
# 生成火焰图数据
```

#### 并发问题调试
```cpp
# 使用ThreadSanitizer
cmake -DCMAKE_BUILD_TYPE=Debug -DENABLE_TSAN=ON ..
make
./build/sqlcc-server

# 启用详细日志
export SQLCC_LOG_LEVEL=DEBUG
./build/sqlcc-server
```

### 日志分析

```bash
# 查看应用日志
tail -f logs/sqlcc.log

# 搜索错误
grep "ERROR" logs/sqlcc.log

# 分析性能日志
grep "PERF" logs/sqlcc.log | awk '{print $2, $4}' > perf_data.txt
```

## 测试策略

### 分层测试架构

| 层级 | 目录 | 测试范围 | 标签 |
|------|------|----------|------|
| **Level 1** | `tests/level1_foundation/` | 基础组件（异常、日志、配置、类型、工具类） | `foundation` |
| **Level 2** | `tests/level2_core/` | 核心组件（DB管理器、执行上下文） | `core` |
| **Level 2** | `tests/level2_storage_engine/` | 存储引擎（缓冲池、B+树、磁盘管理） | `storage` |
| **Level 3** | `tests/level3_transaction_manager/` | 事务管理、查询执行 | `transaction` |
| **Level 4** | `tests/level4_sql_processing/` | SQL处理和解析 | `sql_processing` |
| **Level 5** | `tests/level5_network/` | 网络通信、协议处理 | `network` |
| **Level 6** | `tests/level6_integration/` | 集成测试 | `integration` |
| **Level 7** | `tests/level7_integration/` | 企业级测试（性能、压力、可靠性） | `enterprise` |

### 测试类型

#### 单元测试
```cpp
// tests/level1_foundation/exception/exception_test.cpp
TEST(ExceptionTest, BasicExceptionHandling) {
    // 测试异常捕获和抛出
    EXPECT_THROW(throw std::runtime_error("test"), std::runtime_error);
}
```

#### 集成测试
```cpp
// tests/level6_integration/end_to_end/basic_crud_test.cpp
TEST(BasicCRUDTest, InsertQueryDelete) {
    // 测试完整的CRUD操作流程
    // 验证数据一致性和事务完整性
}
```

#### 性能测试
```cpp
// tests/level7_integration/performance/concurrent_insert_test.cpp
TEST(PerformanceTest, HighConcurrencyInsert) {
    // 高并发插入测试
    // 测量吞吐量和延迟
}
```

### 测试运行

```bash
# 运行所有测试
bazel test //...

# 运行特定层级测试
bazel test //tests/level1_foundation:all
bazel test //tests/level2_core:all
bazel test //tests/level2_storage_engine/b_plus_tree:all

# 使用标签过滤测试
bazel test //tests/... --test_tag_filters=foundation
bazel test //tests/... --test_tag_filters=core
bazel test //tests/... --test_tag_filters=storage
bazel test //tests/... --test_tag_filters=-manual,-slow

# 查看测试输出
bazel test //tests/... --test_output=all
bazel test //tests/... --test_output=errors

# 生成覆盖率报告
bazel coverage //tests/level1_foundation:all

# 运行性能测试
bazel test //tests/level7_integration/performance:all
```

## 文档维护

### 代码注释更新

```cpp
# 当修改代码时，同时更新注释
# 确保注释准确反映实际行为
# 添加必要的Why/What/How说明
```

### 文档更新

```bash
# 修改相关文档
vim docs/design/storage_engine.md

# 运行文档检查
./scripts/check_documentation.sh

# 提交文档变更
git add docs/
git commit -m "docs: update design documentation"
```

## 贡献指南

### 提交前检查

- [ ] 代码通过所有测试
- [ ] 代码格式正确（clang-format）
- [ ] 静态检查通过（clang-tidy）
- [ ] 注释完整且准确
- [ ] 相关文档已更新
- [ ] 性能测试通过

### 代码审查要点

- **功能正确性**: 实现需求，边界条件处理正确
- **代码质量**: 遵循编码规范，结构清晰
- **测试充分性**: 包含异常情况，覆盖率达标
- **文档完整性**: 注释和文档同步更新
- **性能影响**: 无明显性能退化

### 沟通协作

- **Issue讨论**: 在开始开发前充分讨论需求
- **PR描述**: 详细说明变更内容和原因
- **及时响应**: 积极响应代码审查意见
- **知识分享**: 主动分享技术经验和教训

## 进阶学习

### 核心组件深入

1. **存储引擎**: 理解B+树、缓冲池、WAL机制
2. **查询处理器**: 掌握SQL解析、优化、执行流程
3. **事务管理器**: 学习MVCC、锁协议、死锁检测
4. **网络通信**: 了解连接池、协议处理、安全机制

### 性能优化

1. **热点分析**: 使用perf定位性能瓶颈
2. **算法优化**: 改进数据结构和算法复杂度
3. **并发优化**: 减少锁竞争，提高并发度
4. **内存优化**: 减少内存分配，提高缓存命中率

### 架构设计

1. **模块化**: 理解组件间的依赖关系
2. **可扩展性**: 学习插件架构和扩展机制
3. **容错性**: 掌握错误处理和恢复策略
4. **监控运维**: 了解监控指标和运维工具

## 常见问题

### 构建问题
**Q: 编译失败，提示缺少依赖？**
A: 检查系统依赖安装，参考INSTALL.md

**Q: Bazel构建卡住？**
A: 清除缓存 `bazel clean --expunge`

### 运行问题
**Q: 数据库启动失败？**
A: 检查端口占用和权限，查看日志文件

**Q: 连接被拒绝？**
A: 确认服务器运行状态和防火墙设置

### 测试问题
**Q: 测试失败但不确定原因？**
A: 运行 `ctest --output-on-failure -V`

**Q: 性能测试结果不稳定？**
A: 确保系统负载稳定，多次运行取平均值

## 获取帮助

- **文档中心**: https://docs.sqlcc.org/
- **开发者论坛**: https://forum.sqlcc.org/developers
- **邮件列表**: sqlcc-dev@groups.io
- **导师制度**: 联系项目维护者获得一对一指导

---

欢迎加入SQLCC开发团队！我们致力于打造一个现代化、高质量的数据库系统，为开源社区贡献力量，也为您的技术成长提供平台。

**最后更新: 2026-01-30**  
**当前版本: v1.3.9**  
**构建系统: Bazel 8.5.0+**  
**编译器: Clang 20+**  
**C++标准: C++20**
