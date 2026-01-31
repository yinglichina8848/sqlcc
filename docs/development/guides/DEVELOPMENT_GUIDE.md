# SQLCC 开发指南

本指南为开发者提供从环境搭建到功能实现的完整开发流程，帮助快速上手SQLCC项目开发。

## 🚀 快速开始

### 1. 环境准备

**系统要求**:
- Linux Ubuntu 22.04+ / CentOS 8+ / macOS 12+ (推荐Linux)
- Clang 20+ 编译器和LLVM 20工具链
- Bazel 8.5.0+ 构建系统
- Python 3.10+ (用于脚本工具)

**安装依赖**:
```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install clang-20 clang++-20 llvm-20 llvm-cov-20 llvm-profdata-20 libc++-20-dev libc++abi-20-dev bazel python3 python3-pip git

# CentOS/RHEL
sudo yum install clang-20 clang++-20 llvm-20 bazel python3 git

# macOS
brew install clang@20 llvm@20 bazel python3 git
```

### 2. 项目克隆与构建

```bash
# 克隆项目
git clone https://gitee.com/yinglichina/sqlcc.git
cd sqlcc

# 使用Bazel构建
bazel build //...

# 运行测试
bazel test //...

# 生成覆盖率报告
bazel coverage //tests/level1_foundation:all
```

### 3. 开发环境配置

**VS Code配置**:
- 安装C/C++扩展
- 安装Bazel扩展
- 配置`.vscode/settings.json`:
```json
{
    "bazel.buildifier.fixOnFormat": true,
    "C_Cpp.default.cppStandard": "c++20",
    "C_Cpp.default.compilerPath": "/usr/bin/clang++-20",
    "files.associations": {
        "*.cpp": "cpp",
        "*.h": "cpp",
        "*.cc": "cpp"
    }
}
```

**CLion配置**:
- 直接打开项目根目录
- 配置Bazel构建工具链
- 设置C++20标准
- 配置Clang 20编译器

## 📋 开发流程

### 功能开发流程

1. **需求分析**
   - 阅读相关issue或需求文档
   - 理解功能目标和性能要求
   - 分析对现有架构的影响

2. **设计阶段**
   - 设计接口和类结构
   - 考虑异常处理和边界情况
   - 规划测试用例

3. **编码实现**
   - 遵循编码规范
   - 编写单元测试
   - 添加必要的注释

4. **测试验证**
   - 运行单元测试
   - 执行集成测试
   - 性能测试验证

5. **代码审查**
   - 自测和静态代码检查
   - 提交Pull Request
   - 同行代码审查

6. **文档更新**
   - 更新API文档
   - 完善设计文档
   - 更新CHANGELOG

### 分支管理策略

**分支命名规范**:
- `main`: 主分支，稳定版本
- `develop`: 开发分支，集成新功能
- `feature/xxx`: 功能分支
- `bugfix/xxx`: 缺陷修复分支
- `hotfix/xxx`: 紧急修复分支

**开发工作流**:
```bash
# 1. 从develop创建功能分支
git checkout develop
git pull origin develop
git checkout -b feature/new-storage-engine

# 2. 开发并提交
# ... 开发代码 ...
git add .
git commit -m "feat: implement new storage engine"

# 3. 推送到远程
git push origin feature/new-storage-engine

# 4. 创建Pull Request
# 在Gitee/GitHub上创建PR到develop分支

# 5. 代码审查通过后合并
# 删除功能分支
git branch -d feature/new-storage-engine
```

## 🏗️ 架构设计

### 核心组件架构

```
┌─────────────────────────────────────┐
│           SQL解析器 (SQL Parser)     │
├─────────────────────────────────────┤
│          查询执行器 (Query Executor)  │
├─────────────────────────────────────┤
│          事务管理器 (Transaction)    │
├─────────────────────────────────────┤
│          存储引擎 (Storage Engine)   │
├─────────────────────────────────────┤
│    缓冲池管理 (Buffer Pool Manager)  │
├─────────────────────────────────────┤
│     磁盘管理 (Disk Manager)         │
├─────────────────────────────────────┤
│        文件系统 (File System)       │
└─────────────────────────────────────┘
```

### 关键设计原则

1. **模块化设计**
   - 每个组件职责单一
   - 清晰的接口定义
   - 低耦合高内聚

2. **异常安全**
   - RAII资源管理
   - 强异常保证
   - 错误码统一处理

3. **性能优化**
   - 批量操作优先
   - 内存池管理
   - 异步I/O支持

4. **可扩展性**
   - 插件化架构
   - 配置驱动
   - 热插拔支持

## 🧪 测试策略

### 测试金字塔

```
    🎯 系统测试 (端到端)
        ↓
    🔗 集成测试 (模块间)
        ↓
    🧪 单元测试 (函数级)
```

### 测试类型

**1. 单元测试 (Unit Tests)**
```cpp
// 示例：缓冲池测试
TEST(BufferPoolTest, BasicOperations) {
    BufferPool pool(1024);  // 1MB缓冲池
    
    // 测试页面分配
    page_id_t page_id = pool.AllocatePage();
    EXPECT_NE(page_id, INVALID_PAGE_ID);
    
    // 测试页面获取
    Page* page = pool.GetPage(page_id);
    EXPECT_NE(page, nullptr);
    
    // 测试页面释放
    EXPECT_TRUE(pool.FreePage(page_id));
}
```

**2. 集成测试 (Integration Tests)**
```cpp
// 示例：存储引擎集成测试
TEST(StorageEngineTest, CRUDOperations) {
    StorageEngine engine("test.db");
    
    // 创建表
    TableSchema schema = CreateTestSchema();
    table_id_t table_id = engine.CreateTable(schema);
    
    // 插入数据
    Record record = CreateTestRecord();
    RID rid = engine.InsertRecord(table_id, record);
    EXPECT_NE(rid, INVALID_RID);
    
    // 查询数据
    Record retrieved = engine.GetRecord(table_id, rid);
    EXPECT_EQ(record, retrieved);
}
```

**3. 性能测试 (Performance Tests)**
```cpp
// 示例：性能基准测试
static void BM_InsertPerformance(benchmark::State& state) {
    StorageEngine engine("perf_test.db");
    TableSchema schema = CreateTestSchema();
    table_id_t table_id = engine.CreateTable(schema);
    
    for (auto _ : state) {
        Record record = CreateRandomRecord();
        engine.InsertRecord(table_id, record);
    }
    
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_InsertPerformance)->Range(1000, 1000000);
```

### 测试数据管理

**测试数据库**:
```bash
# 创建测试数据库
./build/tests/test_runner --create-test-db

# 运行特定测试
./build/tests/test_runner --test-case=BufferPoolTest

# 性能测试
./build/tests/performance_test --benchmark
```

**测试覆盖率**:
```bash
# 生成覆盖率报告
cd build
make coverage
# 查看报告
open docs/coverage/index.html
```

## 📊 性能优化

### 性能分析工具

**1. 性能分析器**
```bash
# 使用perf分析
perf record -g ./sqlcc
perf report

# 使用gprof分析
gcc -pg -o sqlcc main.cc
gprof sqlcc gmon.out > profile.txt
```

**2. 内存分析**
```bash
# 使用valgrind检测内存泄漏
valgrind --leak-check=full ./sqlcc

# 使用heaptrack分析内存使用
heaptrack ./sqlcc
heaptrack_gui heaptrack.sqlcc.zst
```

**3. 性能监控**
```cpp
// 内置性能监控
class PerformanceMonitor {
public:
    void StartTiming(const std::string& operation);
    void EndTiming(const std::string& operation);
    void RecordMetric(const std::string& name, double value);
    void GenerateReport();
};
```

### 优化策略

**1. I/O优化**
- 批量预取：一次读取多个页面
- 异步I/O：重叠计算和I/O操作
- 压缩存储：减少磁盘占用

**2. 内存优化**
- 对象池：重用对象减少分配
- 内存映射：大文件高效访问
- 缓存友好：数据结构优化

**3. 并发优化**
- 读写锁：提高并发度
- 无锁结构：减少锁竞争
- 工作窃取：负载均衡

## 🛠️ 调试技巧

### 调试工具配置

**GDB调试**:
```bash
# 编译调试版本
cmake .. -DCMAKE_BUILD_TYPE=Debug

# 启动GDB
gdb ./sqlcc
(gdb) break main
(gdb) run
(gdb) backtrace
(gdb) print variable_name
```

**VS Code调试**:
```json
// .vscode/launch.json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Debug SQLCC",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/build/sqlcc",
            "args": [],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}",
            "environment": [],
            "externalConsole": false,
            "MIMode": "gdb"
        }
    ]
}
```

### 常见问题排查

**1. 段错误 (Segmentation Fault)**
```bash
# 使用gdb快速定位
gdb ./sqlcc core
(gdb) where
```

**2. 内存泄漏**
```bash
# valgrind详细分析
valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all ./sqlcc
```

**3. 性能瓶颈**
```bash
# perf采样分析
perf record -g -p $(pidof sqlcc)
perf report
```

## 📚 文档维护

### 文档类型

**1. API文档**
```bash
# 生成Doxygen文档
cd docs
doxygen Doxyfile
# 查看文档
open doxygen/html/index.html
```

**2. 设计文档**
- 存储引擎设计：`docs/storage_engine_design.md`
- 性能测试报告：`docs/performance_test_report.md`
- 单元测试文档：`docs/unit_testing.md`

**3. 流程文档**
- 发布流程：`docs/release_process.md`
- 分支管理：`docs/BRANCHES.md`

### 文档规范

**Markdown格式**:
```markdown
# 一级标题
## 二级标题
### 三级标题

**粗体文本**
*斜体文本*
`代码片段`

[链接文本](url)
![图片alt](image.png)
```

**代码文档注释**:
```cpp
/**
 * @brief 缓冲池管理器
 * @details 负责管理内存中的数据库页面，实现LRU置换算法
 * 
 * @param pool_size 缓冲池大小（页面数）
 * @return 缓冲池实例
 */
class BufferPool {
public:
    /**
     * @brief 分配新页面
     * @return 页面ID，失败返回INVALID_PAGE_ID
     */
    page_id_t AllocatePage();
};
```

## 🔄 持续集成

### CI/CD配置

**GitHub Actions** (`.github/workflows/ci.yml`):
```yaml
name: CI

on: [push, pull_request]

jobs:
  build:
    runs-on: ubuntu-latest
    
    steps:
    - uses: actions/checkout@v2
    
    - name: Install dependencies
      run: |
        sudo apt-get update
        sudo apt-get install -y cmake g++
    
    - name: Configure
      run: cmake -B build -DCMAKE_BUILD_TYPE=Release
    
    - name: Build
      run: cmake --build build --parallel
    
    - name: Test
      run: cd build && ctest --output-on-failure
    
    - name: Coverage
      run: |
        cd build
        make coverage
        bash <(curl -s https://codecov.io/bash)
```

### 质量门禁

**代码质量检查**:
```bash
# 静态代码分析
cppcheck --enable=all --std=c++17 src/

# 代码格式化
clang-format -i src/*.cc include/*.h

# 复杂度分析
lizard src/
```

**测试覆盖率要求**:
- 单元测试覆盖率 ≥ 80%
- 核心模块覆盖率 ≥ 90%
- 新增代码覆盖率 ≥ 85%

## 📞 获取帮助

### 支持渠道

**1. 文档资源**
- 📖 [项目结构说明](PROJECT_STRUCTURE.md)
- 🚀 [快速开始指南](README.md)
- 📊 [性能分析报告](performance_optimization_report.md)

**2. 问题反馈**
- 提交Issue到项目仓库
- 发送邮件至项目维护者
- 在讨论区发起技术讨论

**3. 社区支持**
- 参与代码审查
- 贡献测试用例
- 分享使用经验

### 最佳实践建议

**1. 开发前准备**
- 阅读相关设计文档
- 理解现有代码结构
- 设计清晰的接口

**2. 编码过程中**
- 遵循编码规范
- 及时编写测试
- 保持代码简洁

**3. 提交前检查**
- 运行所有测试
- 更新相关文档
- 进行自我审查

---

**💡 提示**: 本开发指南会根据项目演进持续更新，建议定期查看最新版本。

**🔄 最后更新**: 2025年11月11日
**👥 维护团队**: SQLCC开发团队