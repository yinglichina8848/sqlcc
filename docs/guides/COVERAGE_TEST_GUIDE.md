# SQLCC 覆盖率测试使用指南

## 概述

本文档介绍如何在SQLCC项目中使用覆盖率测试功能。该功能允许在保证测试正常运行的同时，自动收集代码覆盖率数据，用于分析测试的完整性和识别未测试的代码路径。

## 功能特性

- **无缝集成**: 覆盖率收集不影响正常测试执行
- **Clang原生支持**: 使用LLVM的覆盖率映射技术
- **智能过滤**: 自动排除测试代码和系统库
- **多格式输出**: 支持文本和HTML报告
- **并行安全**: 支持并发测试执行

## 快速开始

### 1. 运行带覆盖率的测试

```bash
# 运行单个测试并收集覆盖率
bazel coverage //tests/storage_engine:buffer_pool_test_with_coverage

# 运行测试套件并收集覆盖率
bazel coverage //tests/storage_engine:storage_engine_coverage_tests

# 生成覆盖率报告
bazel run //tools:generate_coverage_report
```

### 2. 查看覆盖率报告

```bash
# 文本报告
cat bazel-out/coverage/coverage.txt

# HTML报告（在浏览器中打开）
open bazel-out/coverage/html/index.html
```

## 配置说明

### Bazel配置 (.bazelrc)

项目已预配置了覆盖率相关的Bazel选项：

```bash
# 覆盖率配置
coverage --combined_report=lcov
coverage --instrumentation_filter="//src/.*"
coverage --instrumentation_filter="//include/.*"
coverage --exclude_pattern=".*test.*"
coverage --exclude_pattern=".*Test.*"

# Clang覆盖率专用配置
coverage --copt=-fprofile-instr-generate
coverage --copt=-fcoverage-mapping
coverage --linkopt=-fprofile-instr-generate
coverage --linkopt=-fcoverage-mapping
coverage --action_env=LLVM_PROFILE_FILE=/home/liying/coverage_%p.profraw

# 覆盖率报告生成
coverage --coverage_report_generator=@bazel_tools//tools/test/CoverageOutputGenerator/java/com/google/devtools/coverageoutputgenerator:Main

# 覆盖率测试优化
coverage --test_timeout=600
coverage --test_strategy=exclusive
coverage --spawn_strategy=standalone
```

### BUILD文件模板

使用预定义的覆盖率测试模板：

```python
load("//tools:bazel_coverage_build_template.bzl", "sqlcc_coverage_test", "sqlcc_coverage_test_suite")

# 创建带覆盖率的测试
sqlcc_coverage_test(
    name = "my_test_with_coverage",
    srcs = ["my_test.cpp"],
    deps = [
        "//src/my_library",
        "@com_google_googletest//:gtest_main",
    ],
)

# 创建覆盖率测试套件
sqlcc_coverage_test_suite(
    name = "coverage_tests",
    tests = [
        ":my_test_with_coverage",
        # 其他测试...
    ],
)
```

## 使用场景

### 开发阶段

```bash
# 在开发过程中运行覆盖率测试
bazel coverage //tests/unit:my_feature_test

# 检查新增代码的覆盖率
bazel coverage //tests/unit:my_new_feature_test
```

### CI/CD集成

```bash
# 在CI流水线中收集覆盖率
bazel coverage //tests:all_tests

# 生成覆盖率报告
bazel run //tools:coverage_report_generator

# 检查覆盖率阈值
./scripts/check_coverage_threshold.sh 80
```

### 调试和优化

```bash
# 分析特定组件的覆盖率
bazel coverage //tests/storage_engine:storage_engine_coverage_tests

# 生成详细的HTML报告用于分析
bazel run //tools:html_coverage_report
```

## 覆盖率数据管理

### 数据存储位置

- **原始数据**: `/tmp/coverage/*.profraw`
- **合并数据**: `bazel-out/coverage/coverage.profdata`
- **文本报告**: `bazel-out/coverage/coverage.txt`
- **HTML报告**: `bazel-out/coverage/html/`

### 数据清理

```bash
# 清理覆盖率数据
rm -rf /tmp/coverage/*.profraw
rm -rf bazel-out/coverage/

# 清理所有构建产物
bazel clean --expunge
```

## 最佳实践

### 1. 测试设计

- **单元测试**: 每个函数至少有一个测试用例
- **边界测试**: 包含边界条件和异常情况
- **集成测试**: 测试组件间的交互

### 2. 覆盖率目标

- **语句覆盖率**: > 80%
- **分支覆盖率**: > 75%
- **函数覆盖率**: > 90%

### 3. 性能考虑

- 覆盖率测试比普通测试慢20-50%
- 在CI环境中使用专用机器
- 定期清理覆盖率数据

### 4. 调试技巧

```bash
# 查看哪些行没有被覆盖
grep "0|" bazel-out/coverage/coverage.txt

# 分析特定文件的覆盖率
llvm-cov show -instr-profile=bazel-out/coverage/coverage.profdata src/my_file.cpp
```

## 故障排除

### 常见问题

1. **覆盖率数据丢失**
   ```bash
   # 检查环境变量
   echo $LLVM_PROFILE_FILE

   # 确保目录存在
   mkdir -p /tmp/coverage
   ```

2. **报告生成失败**
   ```bash
   # 检查llvm-cov版本
   llvm-cov --version

   # 重新生成报告
   bazel coverage --combined_report=lcov //tests:target
   ```

3. **性能问题**
   ```bash
   # 使用更快的策略
   bazel coverage --spawn_strategy=local //tests:target
   ```

## 高级配置

### 自定义过滤器

```python
# 在BUILD文件中自定义覆盖率过滤器
sqlcc_coverage_test(
    name = "custom_coverage_test",
    srcs = ["test.cpp"],
    deps = ["//src/lib"],
    tags = ["coverage"],
    # 自定义环境变量
    env = {
        "LLVM_PROFILE_FILE": "/custom/path/%p.profraw",
    },
)
```

### 多配置支持

```bash
# 不同配置下的覆盖率测试
bazel coverage --config=asan //tests:target     # 内存检查 + 覆盖率
bazel coverage --config=debug //tests:target   # 调试模式 + 覆盖率
```

## 集成工具

### 与CI/CD集成

```yaml
# GitHub Actions 示例
- name: Run Coverage Tests
  run: |
    bazel coverage //tests:all
    bazel run //tools:coverage_report

- name: Upload Coverage
  uses: codecov/codecov-action@v3
  with:
    file: bazel-out/coverage/coverage.lcov
```

### IDE集成

- **VS Code**: 使用Coverage Gutters扩展查看覆盖率
- **CLion**: 内置覆盖率可视化
- **Vim**: 使用vim-cov插件

## 总结

覆盖率测试是确保代码质量的重要工具。通过合理配置和使用，可以有效识别未测试的代码路径，提升测试的完整性和可靠性。

---

**文档版本**: v1.0
**最后更新**: 2025年12月26日
**维护者**: SQLCC开发团队
