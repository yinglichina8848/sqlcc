# Bazel构建系统工作流程指南

## 概述

本文档提供了系统化的Bazel构建系统维护和改进工作流程，基于SQLCC项目的成功经验。

## 快速检查清单

### 🔍 问题诊断清单

```bash
# 1. 快速构建检查
bazel build //src/utils:utils

# 2. 标签路径验证
bazel query 'allpaths(//..., //include:*)' 2>&1 | head -10

# 3. 代码质量检查
python3 tools/bazel_code_checker.py src/utils/

# 4. 依赖关系分析
bazel query 'deps(//src/utils:utils)' --output graph
```

### 🚀 紧急修复流程

**当构建失败时，按以下顺序执行：**

1. **立即停止** - 不要继续构建其他目标
2. **隔离问题** - 确定失败的具体位置
3. **应用修复** - 使用自动化工具或手动修复
4. **验证修复** - 确保修复有效且不引入新问题

## 详细工作流程

### Phase 1: 问题识别

#### 1.1 构建状态分析

```bash
# 记录构建日志
bazel build //... 2>&1 | tee build.log

# 分析错误类型
grep -E "(error|Error|ERROR)" build.log | head -20

# 统计错误分布
grep "error:" build.log | sed 's/.*error: //' | sort | uniq -c | sort -nr
```

#### 1.2 问题分类

**编译错误类型：**
- ❌ Multiple definition - 函数重复定义
- ❌ Invalid label - 标签路径错误
- ❌ Missing dependency - 依赖缺失
- ❌ Template instantiation - 模板实例化问题

**使用工具自动分类：**

```bash
#!/bin/bash
# classify_build_errors.sh

echo "=== 构建错误分类分析 ==="

# 统计各类错误
echo "🔢 错误统计:"
grep "error:" build.log | wc -l
echo

echo "🏷️  标签错误:"
grep "Invalid label" build.log | wc -l
echo

echo "🔗 链接错误:"
grep "multiple definition" build.log | wc -l
echo

echo "📦 依赖错误:"
grep "Missing dependency" build.log | wc -l
echo

echo "🛠️  编译错误详情 (前10个):"
grep "error:" build.log | head -10
```

### Phase 2: 问题修复

#### 2.1 自动化修复

```bash
# 1. 修复标签路径
python3 tools/bazel_label_fixer.py . --dry-run
python3 tools/bazel_label_fixer.py .

# 2. 检查代码质量
python3 tools/bazel_code_checker.py src/ --check-duplicates

# 3. 验证修复
bazel build //src/utils:utils
```

#### 2.2 手动修复策略

**策略1: 隔离修复**
```bash
# 只修复一个包，避免连锁反应
bazel build //src/utils:utils
# 修复问题后，逐步扩展到其他包
bazel build //src/core:core
```

**策略2: 分层修复**
```bash
# 底层包优先
bazel build //src/utils:utils     # 最底层
bazel build //src/core:core       # 依赖utils
bazel build //src/sql_parser:*    # 依赖core
```

**策略3: 增量验证**
```bash
# 添加成功包到白名单
echo "//src/utils:utils" >> verified_targets.txt

# 批量验证
cat verified_targets.txt | xargs bazel build
```

### Phase 3: 验证和回归测试

#### 3.1 构建验证

```bash
#!/bin/bash
# validate_build.sh

echo "=== 构建验证流程 ==="

# 1. 核心包验证
echo "🔧 核心包测试..."
bazel build //src/utils:utils && echo "✅ utils OK" || exit 1
bazel build //src/core:core && echo "✅ core OK" || exit 1

# 2. 功能包验证
echo "📦 功能包测试..."
bazel build //src/sql_parser:sql_parser && echo "✅ sql_parser OK" || exit 1
bazel build //src/storage_engine:storage_engine && echo "✅ storage_engine OK" || exit 1

# 3. 集成测试
echo "🔗 集成测试..."
bazel build //src/network:network && echo "✅ network OK" || exit 1

# 4. 完整构建
echo "🏗️  完整构建..."
timeout 300 bazel build //... && echo "✅ 全构建成功" || echo "⚠️  部分构建失败"

echo "🎉 验证完成"
```

#### 3.2 回归测试

```bash
# 运行测试套件
bazel test //tests/unit/...

# 性能基准测试
bazel run //tools:benchmark

# 内存安全检查
./scripts/memory_safety_audit.sh
```

## 高级工具使用指南

### 1. 批量修复脚本

```bash
#!/bin/bash
# batch_fix.sh - 一键修复常见问题

echo "=== Bazel批量修复工具 ==="

# 1. 备份当前状态
echo "📦 创建备份..."
tar -czf backup_$(date +%Y%m%d_%H%M%S).tar.gz BUILD.bazel src/ include/ tests/

# 2. 修复标签路径
echo "🏷️  修复标签路径..."
python3 tools/bazel_label_fixer.py .

# 3. 检查代码问题
echo "🔍 检查代码质量..."
python3 tools/bazel_code_checker.py . > code_quality.log

# 4. 验证修复
echo "✅ 验证修复结果..."
bazel build //src/utils:utils && echo "基础包修复成功" || echo "需要进一步检查"

echo "📋 查看修复报告: code_quality.log"
```

### 2. 智能诊断工具

```bash
#!/bin/bash
# smart_diagnose.sh - 智能问题诊断

echo "=== Bazel智能诊断 ==="

# 分析构建日志
if [ -f "build.log" ]; then
    echo "📊 分析构建日志..."

    # 检测最常见问题
    if grep -q "Invalid label" build.log; then
        echo "🔍 发现标签路径问题，建议运行标签修复工具"
        echo "运行: python3 tools/bazel_label_fixer.py ."
    fi

    if grep -q "multiple definition" build.log; then
        echo "🔍 发现重复定义问题，建议检查代码"
        echo "运行: python3 tools/bazel_code_checker.py . --check-duplicates"
    fi

    if grep -q "test_suite" build.log; then
        echo "🔍 发现测试配置问题，检查BUILD.bazel"
    fi
else
    echo "❌ 未找到build.log文件，请先运行构建"
    exit 1
fi
```

### 3. 持续监控脚本

```bash
#!/bin/bash
# monitor_build.sh - 构建监控

echo "=== 构建监控系统 ==="

# 持续监控构建状态
while true; do
    echo "$(date): 检查构建状态..."

    # 快速检查核心目标
    if ! bazel build //src/utils:utils >/dev/null 2>&1; then
        echo "❌ 检测到构建失败，发送告警"
        # 这里可以集成邮件或Slack通知
        exit 1
    fi

    echo "✅ 构建状态正常"
    sleep 300  # 5分钟检查一次
done
```

## 故障排除指南

### 问题: 构建卡住或超时

**原因**: 依赖循环或资源不足

**解决方案**:
```bash
# 1. 检查依赖循环
bazel query 'somepath(//src/..., //src/...)' | head -20

# 2. 限制并发
bazel build --jobs=2 //...

# 3. 清理缓存
bazel clean --expunge
```

### 问题: 内存不足

**原因**: 大项目编译内存需求高

**解决方案**:
```bash
# 减少内存使用
bazel build --jobs=1 --local_ram_resources=4096 //...

# 分批编译
bazel build //src/utils:utils
bazel build //src/core:core
# ... 逐步增加
```

### 问题: 缓存污染

**原因**: 旧缓存导致问题

**解决方案**:
```bash
# 完全清理
bazel clean --expunge

# 重新构建
bazel build //...
```

## 最佳实践

### 1. 日常维护

- **每日构建**: 确保主分支始终可构建
- **增量检查**: 新代码优先检查构建状态
- **问题跟踪**: 使用issue跟踪构建问题

### 2. 代码审查

- **BUILD文件审查**: 每次提交都要检查BUILD.bazel变更
- **依赖检查**: 确保新增依赖合理
- **标签规范**: 强制使用正确的标签格式

### 3. 自动化集成

```yaml
# .github/workflows/ci.yml 示例
name: CI
on: [push, pull_request]
jobs:
  build:
    runs-on: ubuntu-latest
    steps:
    - uses: actions/checkout@v2
    - name: Setup Bazel
      uses: bazelbuild/setup-bazelisk@v1
    - name: Check Labels
      run: python3 tools/bazel_label_fixer.py . --dry-run
    - name: Code Quality
      run: python3 tools/bazel_code_checker.py .
    - name: Build
      run: bazel build //...
    - name: Test
      run: bazel test //tests/...
```

## 总结

遵循这个工作流程，可以：

1. **快速定位问题** - 系统化的诊断流程
2. **高效修复问题** - 自动化工具和最佳实践
3. **预防问题发生** - 持续监控和代码审查
4. **保证质量** - 完善的验证和测试流程

**记住**: 好的构建系统是演进出来的，不是一蹴而就的。通过持续的改进和自动化工具的使用，可以显著提高开发效率和代码质量。
