# SQLCC 模块化迁移指南

## 概述

本指南介绍如何使用SQLCC模块化迁移工具链，将项目从传统的include/src全局结构迁移到现代的模块化结构。

## 迁移工具链

### 1. 迁移执行器 (`tools/migration_executor.py`)

**用途**: 自动执行单个模块的完整迁移流程

**使用方法**:
```bash
# 迁移单个模块
python3 tools/migration_executor.py exception

# 指定项目根目录
python3 tools/migration_executor.py utils --project-root /path/to/project
```

**执行流程**:
1. 创建标准目录结构 (`src/module/include/module/`, `src/module/src/`)
2. 移动头文件从 `include/module/` 到 `src/module/include/module/`
3. 移动源文件从 `src/module/` 到 `src/module/src/`
4. 自动生成BUILD.bazel文件
5. 更新所有引用该模块的include语句和BUILD依赖
6. 验证编译结果

### 2. 依赖修复器 (`tools/dependency_fixer.py`)

**用途**: 修复迁移后的依赖关系问题

**使用方法**:
```bash
# 验证依赖关系
python3 tools/dependency_fixer.py --validate-only

# 自动修复所有依赖问题
python3 tools/dependency_fixer.py

# 生成依赖关系报告
python3 tools/dependency_fixer.py --generate-report
```

**修复内容**:
- 将 `<module/header.h>` 改为 `"module/header.h"`
- 将 `//include:module` 改为 `//src/module:module`
- 验证无循环依赖

### 3. 验证脚本 (`scripts/validate_migration.sh`)

**用途**: 验证迁移结果的完整性和正确性

**使用方法**:
```bash
# 验证所有模块
./scripts/validate_migration.sh --all

# 验证单个模块
./scripts/validate_migration.sh --module utils

# 生成验证报告
./scripts/validate_migration.sh --all --report
```

**验证内容**:
- 项目结构完整性
- 模块目录结构正确性
- Bazel编译通过
- 测试执行通过
- 依赖关系正确
- 无全局include泄露

## 迁移工作流程

### 阶段1: 准备工作

1. **备份项目**
   ```bash
   git checkout -b migration-backup
   # 或创建完整备份
   cp -r project project.backup
   ```

2. **运行初始验证**
   ```bash
   ./scripts/validate_migration.sh --report
   ```

### 阶段2: 核心模块迁移

按依赖顺序迁移模块：

```bash
# 1. 无依赖的基础模块
python3 tools/migration_executor.py exception
python3 tools/migration_executor.py types

# 2. 工具模块
python3 tools/migration_executor.py utils
python3 tools/migration_executor.py config_manager

# 3. 核心服务模块
python3 tools/migration_executor.py core
python3 tools/migration_executor.py logger
python3 tools/migration_executor.py monitoring
python3 tools/migration_executor.py security
```

### 阶段3: 存储引擎迁移

```bash
# 基础存储组件
python3 tools/migration_executor.py disk_manager
python3 tools/migration_executor.py page_allocator
python3 tools/migration_executor.py wal

# 核心存储功能
python3 tools/migration_executor.py buffer_pool
python3 tools/migration_executor.py b_plus_tree
python3 tools/migration_executor.py index
python3 tools/migration_executor.py index_manager

# 存储引擎集成
python3 tools/migration_executor.py storage_engine
```

### 阶段4: SQL处理模块迁移

```bash
# SQL解析器
python3 tools/migration_executor.py sql_parser

# SQL执行器
python3 tools/migration_executor.py sql_executor

# 执行引擎
python3 tools/migration_executor.py execution
```

### 阶段5: 网络和服务迁移

```bash
# 网络组件
python3 tools/migration_executor.py network
python3 tools/migration_executor.py isql_network

# 服务组件
python3 tools/migration_executor.py sqlcc_server
```

### 阶段6: 依赖修复和验证

```bash
# 修复所有依赖关系
python3 tools/dependency_fixer.py

# 最终验证
./scripts/validate_migration.sh --all --report
```

## 故障排除

### 常见问题

#### 1. 编译失败
```bash
# 检查模块结构
./scripts/validate_migration.sh --module <module_name>

# 查看详细编译错误
bazel build //src/<module_name>:<module_name> 2>&1
```

#### 2. 依赖关系错误
```bash
# 生成依赖报告
python3 tools/dependency_fixer.py --generate-report

# 检查循环依赖
bazel query 'deps(//src/...)' | grep -A 10 -B 10 "cycle"
```

#### 3. 测试失败
```bash
# 验证测试配置
./scripts/validate_migration.sh --module <module_name>

# 检查测试BUILD文件
cat tests/level2_*/<module_name>/BUILD.bazel
```

### 回滚策略

1. **单模块回滚**
   ```bash
   git checkout HEAD~1 -- src/<module_name>
   git checkout HEAD~1 -- include/<module_name>
   # 然后重新运行依赖修复器
   ```

2. **分支回滚**
   ```bash
   git checkout migration-backup
   git branch -D migration-main
   ```

## 最佳实践

### 迁移原则
1. **小步快跑**: 每次只迁移一个模块
2. **立即验证**: 每个模块迁移后立即验证
3. **依赖优先**: 按依赖关系顺序迁移
4. **备份第一**: 重要操作前创建备份

### 质量保证
1. **自动化验证**: 使用验证脚本确保质量
2. **持续集成**: 每次迁移后运行完整CI
3. **代码审查**: 重要模块迁移后进行审查
4. **文档同步**: 更新相关文档和配置

### 性能监控
1. **编译时间**: 监控迁移对编译时间的影响
2. **测试覆盖率**: 确保测试覆盖率不下降
3. **依赖清晰度**: 验证模块依赖关系清晰

## 工具扩展

### 添加新的验证规则

在 `scripts/validate_migration.sh` 中添加自定义验证：

```bash
validate_custom_rule() {
    log_info "执行自定义验证规则..."
    # 你的验证逻辑
}
```

### 扩展迁移执行器

在 `tools/migration_executor.py` 中添加新的迁移步骤：

```python
def custom_migration_step(self, module_name: str) -> bool:
    """自定义迁移步骤"""
    # 你的迁移逻辑
    return True
```

## 参考资料

- [SQLCC模块化设计文档](docs/design/sqlcc_package_design.md)
- [Bazel构建标准](docs/guides/BUILD_STANDARD_TEMPLATE.md)
- [测试体系分层标准](docs/testing/SQLCC_TEST_LEVELING_STANDARD.md)

## 常见问题解答

**Q: 迁移过程中出现编译错误怎么办？**
A: 使用验证脚本定位问题，通常是include路径或依赖配置错误。

**Q: 如何处理复杂的模块依赖关系？**
A: 先迁移基础模块，再逐步迁移依赖它们的模块。

**Q: 迁移后的性能会下降吗？**
A: 不会，模块化后Bazel的并行编译会更高效。

**Q: 可以只迁移部分模块吗？**
A: 可以，但建议完整迁移以获得最佳效果。

---

*本文档会随着工具链的更新而同步更新。如有问题，请参考最新的工具代码或提交Issue。*
