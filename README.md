# SQLCC - 基于AI的微型数据库系统

> 🎯 **数据库开发指南 for 大二学生** - 用AI辅助从零构建自己的数据库系统！

## 📦 当前版本: 1.3.1

### 🆕 v1.3.1 新特性 (全面测试与质量保障)
- **完整CRUD实现**: 全面增强CRUD操作支持，包含INSERT、UPDATE、DELETE功能的完整实现
- **事务性CRUD操作**: 确保所有CRUD操作在事务中原子性执行，保证数据一致性
- **性能优化**: 优化CRUD操作性能，确保单操作延迟<5ms（SSD环境下10000-100000行数据）
- **INSERT操作性能**: 单插入操作平均延迟约0.1ms，远低于5ms要求
- **UPDATE操作性能**: 单更新操作平均延迟约0.2ms，满足性能要求
- **DELETE操作性能**: 单删除操作平均延迟约0.15ms，满足性能要求
- **SELECT操作性能**: 点查询延迟约0.05ms，范围查询性能随数据量线性增长

## 📊 代码规模统计

### 核心代码统计 (v1.3.1)
| 指标 | 数量 | 描述 |
|------|------|------|
| **源码行数** | 4,712行 | 总核心C++代码行数 |
| **源码文件** | 56个 | 总.cc和.h文件数 |
| **类数量** | 11个 | 总核心类定义数 |
| **主要模块** | 5个 | 存储引擎、缓冲池、磁盘管理、配置管理、日志系统 |
| **行覆盖率** | 83.3% | 整体代码行覆盖率 |
| **函数覆盖率** | 90.7% | 整体函数覆盖率 |

### 🧪 测试文件结构
- **临时测试文件**: 组织在`tests/temporary/`目录下
  - `tests/temporary/test_simple.cc` - 缓冲池基本功能快速验证测试
  - `tests/temporary/test_page_id_fix.cc` - 页面ID分配逻辑修复验证测试
  - `tests/temporary/test_sync_functionality.cc` - 磁盘同步功能验证测试
  - `tests/temporary/test_deadlock_fix_simple.cc` - 死锁修复验证测试
- 详细文档: [TEMPORARY_TEST_FILES.md](docs/testing/TEMPORARY_TEST_FILES.md)

### 核心性能指标
| 测试类型 | 吞吐量 | 延迟 | 扩展性 |
|----------|--------|------|--------|
| **8线程并发** | 2,044.99 ops/sec | 3.628ms/op | 基准 |
| **4线程并发** | 1,015.23 ops/sec | 3.596ms/op | 线性扩展 |
| **2线程并发** | 535.33 ops/sec | 3.526ms/op | 线性扩展 |
| **1线程基准** | 261.57 ops/sec | 3.629ms/op | 基准 |

## 🚀 快速开始

### 克隆项目
```bash
git clone https://gitee.com/yinglichina/sqlcc.git
cd sqlcc
```

### 编译和测试
```bash
# 清理编译
make clean

# 编译项目
make -j$(nproc)

# 运行单元测试
make test

# 生成覆盖率报告
make coverage

# 运行性能测试
make perf_test
```

## 📚 相关资源

- **API文档**: `docs/doxygen/html/index.html`
- **覆盖率报告**: `coverage/index.html`
- **性能测试报告**: `build/perf_results.json`
- **变更日志**: `CHANGELOG.md`
- **发布说明**: `RELEASE_NOTES.md`

## 🤝 贡献指南

1. Fork本项目
2. 创建特性分支 (`git checkout -b feature/AmazingFeature`)
3. 提交变更 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 创建Pull Request

## 📄 许可证

本项目采用MIT许可证 - 查看[LICENSE](LICENSE)文件了解详情。

## 🙏 致谢

感谢字节跳动Trae AI提供强大的AI辅助编程环境，让数据库系统开发变得更加高效和有趣！

---

**🎯 记住：用AI编程，但不要被AI编程！**

## 📁 项目结构

```
sqlcc/
├── include/              # 头文件目录
├── src/                  # 源代码目录
├── tests/                # 测试代码目录
├── docs/                 # 文档目录
├── examples/             # 示例代码目录
├── scripts/              # 脚本目录
├── tools/                # 工具目录
├── config/               # 配置文件目录
├── data/                 # 数据文件目录
├── coverage_results/     # 覆盖率测试结果
├── BUILD.bazel          # Bazel构建配置
├── MODULE.bazel         # Bazel模块配置
├── WORKSPACE            # Bazel工作空间配置
├── CHANGELOG.md         # 变更日志
├── RELEASE_NOTES.md     # 发布说明
├── VERSION              # 版本文件
└── LICENSE              # 许可证文件
```

## 🏗️ 构建系统

本项目使用[Bazel](https://bazel.build/)作为构建系统，支持现代C++20特性和模块化开发。

### 系统要求
- **操作系统**: Linux/macOS/Windows
- **编译器**: Clang 18+ 或 GCC 11+
- **构建工具**: Bazel 6.0+
- **依赖管理**: 支持Bazel MODULE系统

### 构建命令
```bash
# 完整构建
bazel build //...

# 运行测试
bazel test //tests/...

# 生成文档
bazel build //docs:doxygen

# 代码覆盖率
bazel coverage //tests/...
```

## 🧪 测试体系

项目采用分层测试策略，包含：
- **单元测试**: 核心组件功能测试
- **集成测试**: 模块间协作测试
- **系统测试**: 端到端功能测试
- **性能测试**: 基准性能测试
- **覆盖率测试**: LLVM代码覆盖率分析

## 📈 质量指标

- **代码覆盖率**: 目标60%+ (当前56.1%)
- **编译警告**: 零警告
- **单元测试**: 100%通过
- **性能基准**: 持续监控
- **文档覆盖**: 80%+ API文档

## 🎯 开发路线图

### v1.3.x 系列 (当前)
- ✅ 代码覆盖率测试系统
- ✅ 项目结构全面整理
- ✅ 测试体系完善

### v1.4.x 系列 (计划)
- 🔄 SQL-92标准完整支持
- 🔄 分布式架构基础
- 🔄 企业级特性增强

### v2.0.x 系列 (远期)
- 🔄 云原生架构
- 🔄 多租户支持
- 🔄 高可用集群

## 📞 联系我们

- **项目主页**: https://gitee.com/yinglichina/sqlcc
- **问题反馈**: [Issues](https://gitee.com/yinglichina/sqlcc/issues)
- **文档中心**: [docs/](docs/)

---

*最后更新: 2026-01-12*