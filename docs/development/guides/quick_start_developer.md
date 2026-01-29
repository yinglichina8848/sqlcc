# SQLCC 开发者快速开始指南

## 概述

欢迎加入SQLCC开发团队！本指南将帮助您快速搭建开发环境，了解项目结构，并开始贡献代码。SQLCC采用现代化的开发流程，结合AI辅助开发，让您能够快速上手并做出贡献。

## 环境准备

### 系统要求

- **操作系统**: Linux Ubuntu 18.04+ / CentOS 7+ / macOS 10.15+
- **CPU**: x86_64架构，4核+
- **内存**: 8GB RAM以上
- **存储**: 50GB可用空间
- **网络**: 稳定的互联网连接

### 开发工具

#### 必备工具
```bash
# C++编译器 (推荐Clang)
sudo apt install clang-18 clang++-18

# 构建系统
sudo apt install cmake ninja-build

# 版本控制
sudo apt install git

# 代码编辑器 (推荐VSCode或CLion)
# 下载并安装你喜欢的编辑器
```

#### 推荐工具
```bash
# 代码格式化
sudo apt install clang-format

# 静态分析
sudo apt install clang-tidy cppcheck

# 性能分析
sudo apt install linux-tools-common valgrind

# 文档生成
sudo apt install doxygen graphviz
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
# 使用提供的构建脚本
./scripts/build.sh

# 或者手动构建
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_C_COMPILER=clang-18 \
      -DCMAKE_CXX_COMPILER=clang++-18 \
      ..
make -j$(nproc)
```

### 高级构建选项

```bash
# 启用所有调试功能
cmake -DCMAKE_BUILD_TYPE=Debug \
      -DENABLE_TESTS=ON \
      -DENABLE_COVERAGE=ON \
      -DENABLE_SANITIZERS=ON \
      ..

# 性能优化构建
cmake -DCMAKE_BUILD_TYPE=Release \
      -DENABLE_LTO=ON \
      -DENABLE_PGO=ON \
      ..

# 交叉编译
cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchains/arm64.cmake ..
```

### 构建产物

```bash
# 查看构建结果
ls -la build/

# 可执行文件
./build/sqlcc-server     # 数据库服务器
./build/sqlcc-client     # 命令行客户端
./build/sqlcc-benchmark  # 性能测试工具

# 库文件
./build/libsqlcc.so      # 核心库
./build/libsqlcc_test.a  # 测试库

# 测试文件
./build/tests/           # 所有测试可执行文件
```

## 运行和测试

### 启动数据库

```bash
# 创建数据目录
mkdir -p data
mkdir -p logs

# 启动服务器
./build/sqlcc-server --config config/sqlcc.conf --data-dir ./data --log-dir ./logs

# 或者使用开发配置
./build/sqlcc-server --dev-mode
```

### 连接数据库

```bash
# 使用内置客户端
./build/sqlcc-client

# 或者使用系统安装的客户端
sqlcc-client -h localhost -P 3306
```

### 运行测试

```bash
# 运行所有测试
cd build
ctest --output-on-failure

# 运行特定测试
ctest -R "buffer_pool" --verbose

# 运行性能测试
./tests/performance/buffer_pool_performance_test

# 生成覆盖率报告
make coverage
# 查看 coverage/index.html
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
├── include/           # 头文件
│   ├── core/         # 核心组件接口
│   ├── storage/      # 存储引擎接口
│   ├── sql_parser/   # SQL解析器接口
│   └── ...
├── src/              # 源代码
│   ├── core/         # 核心组件实现
│   ├── storage_engine/ # 存储引擎实现
│   ├── sql_parser/   # SQL解析器实现
│   └── ...
├── tests/            # 测试代码
│   ├── unit/         # 单元测试
│   ├── integration/  # 集成测试
│   └── performance/  # 性能测试
├── docs/             # 文档
├── scripts/          # 构建和工具脚本
├── tools/            # 开发工具
└── config/           # 配置文件
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

### 测试类型

#### 单元测试
```cpp
// tests/unit/storage_engine/buffer_pool_test.cpp
TEST(BufferPoolTest, AllocatePage) {
    auto bpm = std::make_unique<BufferPoolManager>();
    auto page = bpm->allocate_page();
    ASSERT_NE(page, nullptr);
}
```

#### 集成测试
```cpp
// tests/integration/client_server_test.cpp
TEST(ClientServerTest, BasicCRUD) {
    // 启动服务器
    // 连接客户端
    // 执行CRUD操作
    // 验证结果
}
```

#### 性能测试
```cpp
// tests/performance/concurrency_test.cpp
BENCHMARK(BM_ConcurrentInserts) {
    // 高并发插入测试
    // 测量吞吐量和延迟
}
```

### 测试运行

```bash
# 运行所有测试
bazel test //...

# 运行特定组件测试
bazel test //tests/unit/storage_engine/...

# 生成测试覆盖率
bazel coverage //tests/...

# 性能基准测试
bazel run //tests/performance:benchmark
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

**最后更新: 2025-12-24**
