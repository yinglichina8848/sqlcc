# SQLCC Bazel构建工具使用手册

## 概述

本文档详细介绍SQLCC项目Bazel构建系统的专用工具使用方法。这些工具专为解决构建系统中的常见问题而设计，提高开发和维护效率。

## 工具概览

| 工具名称 | 功能描述 | 适用场景 |
|---------|---------|---------|
| bazel_code_checker.py | 代码质量检查 | 编译前质量验证 |
| bazel_label_fixer.py | 标签路径修复 | BUILD文件维护 |
| bazel_check_deps.sh | 依赖关系分析 | 依赖问题诊断 |
| bazel_debug.sh | 构建调试辅助 | 错误分析和定位 |
| bazel_fixer.sh | 批量修复脚本 | 自动化问题修复 |

## 详细使用指南

### 1. bazel_code_checker.py - 代码质量检查工具

#### 功能特性

- **重复函数定义检查**: 检测源码中的重复函数声明
- **模板实例化风险评估**: 识别潜在的链接冲突
- **编译选项验证**: 检查编译选项的正确性
- **依赖完整性验证**: 确保依赖声明的完整性

#### 基本用法

```bash
# 检查单个文件
python3 tools/bazel_code_checker.py src/utils/smart_config_manager.cpp

# 检查整个目录
python3 tools/bazel_code_checker.py src/utils/

# 检查整个项目
python3 tools/bazel_code_checker.py .
```

#### 高级选项

```bash
# 仅检查重复函数
python3 tools/bazel_code_checker.py --check-duplicates src/

# 检查模板实例化风险
python3 tools/bazel_code_checker.py --check-templates src/

# 自动修复发现的问题
python3 tools/bazel_code_checker.py --fix src/utils/
```

#### 输出示例

```
检查文件: src/utils/smart_config_manager.cpp
✅ 无重复函数定义
✅ 无显式模板实例化问题
⚠️  发现未使用参数: OnConfigChange::version_id
🔧 自动修复: 添加 (void)version_id; 消除警告
```

#### 最佳实践

1. **每日构建前检查**
   ```bash
   python3 tools/bazel_code_checker.py src/ --check-all
   ```

2. **提交前验证**
   ```bash
   python3 tools/bazel_code_checker.py --fix changed_files/
   ```

3. **CI/CD集成**
   ```yaml
   - name: Code Quality Check
     run: python3 tools/bazel_code_checker.py . --ci-mode
   ```

### 2. bazel_label_fixer.py - 标签路径修复工具

#### 功能特性

- **标签路径规范化**: 自动修复错误的Bazel标签路径
- **批量处理能力**: 支持同时修复多个文件
- **预览模式**: 显示修复内容而不实际修改
- **回滚支持**: 生成修复日志便于回滚

#### 基本用法

```bash
# 修复单个文件
python3 tools/bazel_label_fixer.py src/core/BUILD.bazel

# 修复整个目录
python3 tools/bazel_label_fixer.py src/

# 预览修复内容
python3 tools/bazel_label_fixer.py --dry-run include/
```

#### 修复规则

| 错误格式 | 正确格式 | 示例 |
|---------|---------|------|
| //include:file.h | //include/package:file.h | //include:core/user_manager.h → //include/core:user_manager.h |
| //src:file.cpp | //src/package:file.cpp | //src:config.cpp → //src/utils:config.cpp |

#### 输出示例

```
修复文件: include/BUILD.bazel
📊 修复统计: 修复了 15 个标签
✅ 修复标签: //include:core/user_manager.h → //include/core:user_manager.h
✅ 修复标签: //include:storage/wal_writer.h → //include/storage:wal_writer.h
✅ 修复标签: //src:sql_executor.cpp → //src/core:sql_executor.cpp
```

#### 高级用法

```bash
# 生成修复报告
python3 tools/bazel_label_fixer.py --report include/ > label_fix_report.md

# 仅修复特定类型的标签
python3 tools/bazel_label_fixer.py --include-only include/ src/include/
```

### 3. bazel_check_deps.sh - 依赖关系分析工具

#### 功能特性

- **依赖健康评分**: 量化评估依赖关系的健康度
- **循环依赖检测**: 自动发现和报告循环依赖
- **依赖深度分析**: 计算依赖关系的深度和复杂度
- **可视化报告**: 生成依赖关系图表

#### 基本用法

```bash
# 检查包的依赖健康度
./tools/bazel_check_deps.sh --health //src/utils:utils

# 检测循环依赖
./tools/bazel_check_deps.sh --circular //src/...

# 生成详细报告
./tools/bazel_check_deps.sh --report //src/core:core > deps_report.md
```

#### 输出示例

```
[INFO] : //src/utils:utils
=== 依赖统计 ===
总依赖数: 41
直接依赖数: 11
测试依赖数: 0
0
依赖健康度评分: 100/100
[SUCCESS] 依赖健康度良好

=== 依赖深度分析 ===
最大深度: 3
平均深度: 2.1
复杂度评分: 低
```

#### 监控模式

```bash
# 持续监控依赖健康度
./tools/bazel_check_deps.sh --monitor //src/...

# 设置健康度阈值
./tools/bazel_check_deps.sh --health-threshold 90 //src/...
```

### 4. bazel_debug.sh - 构建调试辅助工具

#### 功能特性

- **错误日志分析**: 智能解析Bazel错误信息
- **问题分类**: 自动分类不同类型的构建问题
- **修复建议**: 提供针对性的修复建议
- **调试信息增强**: 显示详细的调试信息

#### 基本用法

```bash
# 分析构建错误日志
bazel build //target 2>&1 | ./tools/bazel_debug.sh --analyze

# 调试特定目标
./tools/bazel_debug.sh --debug //src/core:core

# 生成调试报告
./tools/bazel_debug.sh --report error.log > debug_report.md
```

#### 错误分类和建议

| 错误类型 | 识别特征 | 修复建议 |
|---------|---------|---------|
| 标签路径错误 | "is invalid" | 使用bazel_label_fixer.py修复 |
| 依赖缺失 | "undeclared inclusion" | 添加相应的deps声明 |
| 编译选项错误 | "unknown warning option" | 检查copts配置 |
| 循环依赖 | 构建超时 | 使用bazel_check_deps.sh分析 |

### 5. bazel_fixer.sh - 批量修复脚本

#### 功能特性

- **一键修复**: 自动执行常见问题的修复
- **批量处理**: 同时修复多个问题类型
- **安全模式**: 支持预览和确认模式
- **回滚支持**: 保留修复历史便于回滚

#### 基本用法

```bash
# 自动修复整个项目
./tools/bazel_fixer.sh --auto-fix

# 预览修复内容
./tools/bazel_fixer.sh --preview

# 修复特定目录
./tools/bazel_fixer.sh --target src/ --fix
```

#### 修复流程

```bash
🔍 分析阶段: 扫描所有BUILD文件和源码
📊 诊断阶段: 识别问题类型和位置
🔧 修复阶段: 按优先级执行修复
✅ 验证阶段: 检查修复结果
📝 报告阶段: 生成修复报告
```

## 工具集成使用

### 日常开发流程

```bash
# 1. 代码修改后质量检查
python3 tools/bazel_code_checker.py modified_files/

# 2. 构建测试
bazel build //target

# 3. 如果构建失败，使用调试工具
bazel build //target 2>&1 | ./tools/bazel_debug.sh --analyze

# 4. 自动修复发现的问题
./tools/bazel_fixer.sh --auto-fix

# 5. 验证修复结果
bazel build //target
```

### CI/CD集成流程

```yaml
name: Bazel Build Check
on: [push, pull_request]

jobs:
  build-check:
    runs-on: ubuntu-latest
    steps:
    - uses: actions/checkout@v2

    - name: Setup Bazel
      run: |
        # 安装Bazel和依赖

    - name: Code Quality Check
      run: python3 tools/bazel_code_checker.py . --ci-mode

    - name: Dependency Health Check
      run: ./tools/bazel_check_deps.sh --health //src/...

    - name: Build Test
      run: bazel build //...

    - name: Test Execution
      run: bazel test //tests/...
```

### 问题诊断流程

```bash
# 1. 快速诊断
bazel build //target 2>&1 | head -20

# 2. 详细分析
bazel build //target 2>&1 | ./tools/bazel_debug.sh --analyze > debug.log

# 3. 检查依赖问题
./tools/bazel_check_deps.sh --circular //target

# 4. 代码质量检查
python3 tools/bazel_code_checker.py target_sources/

# 5. 自动修复
./tools/bazel_fixer.sh --target target/ --fix

# 6. 验证修复
bazel build //target
```

## 性能优化建议

### 工具使用优化

1. **批量处理**: 优先使用批量处理模式
2. **增量检查**: 只检查变更的文件
3. **缓存利用**: 利用Bazel缓存减少重复检查
4. **并行执行**: 在多核系统上启用并行检查

### 配置优化

```bash
# 启用Bazel缓存
export BAZEL_CACHE_DIR=/tmp/bazel-cache

# 设置并行作业数
export BAZEL_JOBS=8

# 启用远程缓存
bazel build --remote_cache=grpc://cache.example.com
```

## 故障排除

### 常见问题

**工具无法执行**
```bash
# 检查Python版本
python3 --version  # 需要3.6+

# 检查权限
ls -la tools/

# 检查依赖
pip install -r requirements.txt
```

**修复结果不正确**
```bash
# 查看修复日志
cat bazel_fixer.log

# 手动回滚
git checkout -- BUILD.bazel

# 重新分析问题
./tools/bazel_debug.sh --analyze error.log
```

**性能问题**
```bash
# 启用性能模式
export BAZEL_PERFORMANCE_MODE=1

# 减少检查深度
python3 tools/bazel_code_checker.py --fast-mode src/
```

## 扩展开发

### 添加新的检查规则

```python
# 在bazel_code_checker.py中添加新规则
def check_custom_rule(self, file_path):
    """检查自定义规则"""
    # 实现检查逻辑
    pass
```

### 集成新的修复工具

```bash
# 在bazel_fixer.sh中添加新修复器
function fix_custom_issue() {
    # 实现修复逻辑
}
```

## 版本信息

- **工具版本**: v1.2.4
- **兼容性**: Bazel 4.0+ , Python 3.6+
- **支持平台**: Linux, macOS, Windows
- **最后更新**: 2025-12-20

## 贡献指南

1. **问题报告**: 使用GitHub Issues报告工具问题
2. **功能请求**: 详细描述新功能需求
3. **代码贡献**: 遵循项目的编码规范
4. **文档更新**: 及时更新使用手册

---

*此手册持续更新，反映工具的最新功能和最佳实践*
