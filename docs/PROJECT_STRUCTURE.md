# SQLCC 项目结构说明

本文档详细说明SQLCC项目的目录结构、文件用途和设计理念，帮助开发者快速理解项目架构。

## 📁 根目录结构

```
sqlcc/
├── .gitignore              # Git忽略文件配置
├── CMakeLists.txt         # CMake构建配置文件（主构建入口）
├── CMakeList.txt          # 备用CMake配置文件
├── Doxyfile               # Doxygen文档生成配置
├── Makefile               # Makefile构建配置
├── LICENSE                # 开源许可证
├── VERSION                # 版本号文件
├── README.md              # 项目主文档（多语言支持）
├── README.en.md           # 英文版README
├── ChangeLog.md           # 版本变更历史
├── VERSION_SUMMARY.md     # 版本特性总结
├── TODO.md                # 项目开发计划和进度
├── TRAE-Chat.md           # AI辅助开发记录
├── performance_optimization_report.md  # 性能优化报告
└── PROJECT_STRUCTURE.md   # 本文档（项目结构说明）
```

## 🏗️ 源代码目录

### `src/` - 核心源代码
```
src/
├── CMakeLists.txt    # 源码构建配置
├── main.cc          # 主程序入口
├── buffer_pool.cc   # 缓冲池管理器实现
├── disk_manager.cc  # 磁盘管理器实现
├── page.cc          # 页面管理实现
└── storage_engine.cc # 存储引擎主实现
```

**设计理念**：
- **模块化设计**：每个核心组件独立实现，降低耦合度
- **单一职责**：每个文件专注一个核心功能
- **接口清晰**：通过头文件定义清晰的API接口

### `include/` - 头文件目录
```
include/
├── buffer_pool.h    # 缓冲池管理器接口
├── disk_manager.h   # 磁盘管理器接口
├── exception.h      # 异常处理定义
├── logger.h         # 日志管理接口
├── page.h           # 页面管理接口
├── storage_engine.h # 存储引擎主接口
└── version.h        # 版本信息定义
```

**设计原则**：
- **接口与实现分离**：头文件定义接口，源文件实现逻辑
- **前向声明**：减少编译依赖，提升编译速度
- **异常安全**：统一的异常处理机制

## 🧪 测试目录

### `tests/` - 测试框架
```
tests/
├── CMakeLists.txt          # 测试构建配置
├── storage_engine_test.cc  # 存储引擎基础测试
└── performance/           # 性能测试子目录
    ├── CMakeLists_concurrency.txt    # 并发测试配置
    ├── CMakeLists_cpu.txt            # CPU测试配置
    ├── Makefile.stability            # 稳定性测试配置
    ├── performance_test_base.h/cc  # 性能测试基类
    ├── performance_test.cc          # 主性能测试
    ├── *_performance_test.h/cc      # 各类性能测试
    ├── *_test_runner.cc             # 测试运行器
    └── */                           # 测试专用目录
```

**测试策略**：
- **分层测试**：单元测试 → 集成测试 → 性能测试
- **场景覆盖**：CPU密集、并发、内存、I/O、稳定性
- **自动化运行**：支持CI/CD集成的测试脚本

## 📊 性能分析目录

### `analysis/` - 性能分析结果
```
analysis/
├── performance_summary.md      # 性能测试摘要
├── *_performance.png            # 各类性能图表
└── [各类性能分析图片]           # 可视化性能数据
```

**分析内容**：
- **缓冲池性能**：命中率、LRU效率、扩展性
- **I/O性能**：顺序/随机读写、并发I/O性能
- **工作负载**：混合负载、事务大小、读写比例
- **长期稳定性**：24小时连续运行监控

### `performance_results/` - 详细性能数据
```
performance_results/
└── [各类性能测试原始数据]
```

## 📚 文档目录

### `docs/` - 技术文档中心
```
docs/
├── index.md                          # 文档中心入口
├── BRANCHES.md                      # Git分支管理规范
├── Guide.md                         # 开发指南（详细版）
├── storage_engine_design.md         # 存储引擎设计文档
├── unit_testing.md                  # 单元测试文档
├── performance_test_report.md       # 性能测试报告
├── performance_test_implementation_guide.md  # 性能测试指南
├── performance_test_improvement_suggestions.md # 改进建议
├── release_process.md               # 发布流程文档
├── coverage/                        # 代码覆盖率报告
│   ├── index.html                   # 覆盖率主页
│   └── [详细覆盖率文件]            # lcov生成报告
└── doxygen/                        # API文档
    └── html/                       # Doxygen生成文档
        └── index.html              # API文档入口
```

**文档分类**：
- **设计文档**：架构设计、组件说明、算法描述
- **测试文档**：测试框架、测试用例、性能报告
- **API文档**：自动生成，包含类和函数说明
- **流程文档**：开发流程、发布流程、分支管理

## 🛠️ 脚本工具目录

### `scripts/` - 自动化脚本
```
scripts/
├── generate_docs.sh              # 文档生成脚本
├── release.sh                    # 发布流程脚本
├── run_performance_example.sh   # 性能测试运行
├── view_performance_results.sh  # 性能结果查看
├── analyze_performance_results.py # 性能数据分析
└── plot_batch_prefetch_results.py # 批量预取绘图
```

**脚本功能**：
- **文档生成**：自动化生成API文档和覆盖率报告
- **性能测试**：一键运行各类性能测试
- **数据分析**：Python脚本处理性能数据
- **发布管理**：自动化版本发布流程

## 🎯 设计架构说明

### 分层架构
```
┌─────────────────────────────────────┐
│           应用层 (Application)       │
├─────────────────────────────────────┤
│          存储引擎 (Storage Engine)   │
├─────────────────────────────────────┤
│    缓冲池管理 (Buffer Pool Manager)  │
├─────────────────────────────────────┤
│     磁盘管理 (Disk Manager)         │
├─────────────────────────────────────┤
│        页面管理 (Page Manager)      │
├─────────────────────────────────────┤
│        文件系统 (File System)       │
└─────────────────────────────────────┘
```

### 核心组件关系
```
StorageEngine (存储引擎)
    ├── BufferPool (缓冲池管理器)
    │   ├── Page (页面)
    │   └── LRU置换算法
    ├── DiskManager (磁盘管理器)
    │   ├── 文件I/O操作
    │   └── 页面分配/释放
    └── Logger (日志管理器)
        ├── WAL (预写日志)
        └── 错误处理
```

## 🔧 构建系统说明

### CMake构建流程
```bash
mkdir build && cd build
cmake ..                    # 配置构建
make                       # 编译源码
make test                  # 运行测试
make install              # 安装程序
```

### 构建选项
- **Debug模式**: 启用调试符号和断言
- **Release模式**: 优化性能，禁用调试
- **测试构建**: 包含所有测试用例
- **文档构建**: 生成API文档和覆盖率报告

## 📈 性能监控点

### 关键性能指标
- **页面命中率**: 缓冲池效率指标
- **I/O延迟**: 磁盘访问响应时间
- **吞吐量**: 每秒查询数(QPS)
- **并发性能**: 多线程扩展性
- **内存使用**: 内存效率和泄漏检测

### 监控工具
- **性能测试框架**: 7种测试场景覆盖
- **可视化图表**: 自动生成性能趋势图
- **长期稳定性**: 24小时连续运行监控

## 🚀 扩展性设计

### 水平扩展
- **多实例部署**: 支持多个存储引擎实例
- **分片支持**: 数据水平分片机制
- **负载均衡**: 请求分发和负载均衡

### 垂直扩展
- **插件机制**: 支持自定义存储引擎
- **索引扩展**: 多种索引类型支持
- **压缩算法**: 可插拔压缩算法

## 📞 开发指南

### 新增功能开发流程
1. **需求分析**: 明确功能需求和性能目标
2. **架构设计**: 设计组件接口和交互关系
3. **编码实现**: 遵循编码规范和设计原则
4. **单元测试**: 编写对应的测试用例
5. **性能测试**: 验证性能指标和瓶颈分析
6. **文档更新**: 更新相关技术文档
7. **代码审查**: 同行评审和质量检查

### 代码规范
- **命名规范**: 使用驼峰命名法，类名首字母大写
- **注释规范**: 关键算法和复杂逻辑必须注释
- **异常处理**: 统一的异常处理和错误码
- **内存管理**: RAII原则，避免内存泄漏
- **并发安全**: 线程安全设计，避免竞态条件

---

**📌 说明**: 本结构说明文档会根据项目演进持续更新，确保与代码结构保持同步。

**🔄 最后更新**: 2025年11月11日
**📧 反馈**: 结构相关问题请提交Issue或联系项目维护者