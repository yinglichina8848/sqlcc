# Level 1 Foundation 完整单元测试 - 快速参考指南 v1.3.8

**版本**: v1.3.8  
**日期**: 2026-01-30  
**状态**: ✅ 完成

---

## 📊 快速统计

| 指标 | 数值 |
|------|------|
| 测试模块数 | 4 (Exception, Types, Logger, Config) |
| 测试文件数 | 4 |
| 测试用例数 | ~160 |
| 异常测试通过率 | 100% (32/32) |
| 真实实现测试 | ✅ 全部使用真实实现 |

---

## 📁 文件结构

```
tests/level1_foundation/
├── exception/
│   ├── BUILD.bazel
│   └── exception_test.cpp (32 tests)
├── types/
│   ├── BUILD.bazel
│   └── types_test.cpp (~60 tests)
├── logger/
│   ├── BUILD.bazel
│   └── logger_test.cpp (~30 tests)
├── config/
│   ├── BUILD.bazel
│   └── config_test.cpp (~40 tests)
└── BUILD.bazel (test suite)
```

---

## 🚀 快速开始

### 运行所有测试

```bash
# 运行 Exception 测试
bazel test //tests/level1_foundation/exception:exception_test

# 运行 Logger 测试
bazel test //tests/level1_foundation/logger:logger_test

# 运行 Config 测试
bazel test //tests/level1_foundation/config:config_test

# 运行 Types 测试
bazel test //tests/level1_foundation/types:types_test
```

### 生成覆盖率报告

```bash
# 1. 合并覆盖率数据
llvm-profdata merge -o merged_coverage_level1.profdata \
  bazel-out/k8-fastbuild/bin/tests/level1_foundation/exception/exception_test.runfiles/_main/default.profraw \
  bazel-out/k8-fastbuild/bin/tests/level1_foundation/logger/logger_test.runfiles/_main/default.profraw \
  bazel-out/k8-fastbuild/bin/tests/level1_foundation/config/config_test.runfiles/_main/default.profraw

# 2. 生成 HTML 报告
llvm-cov show bazel-out/k8-fastbuild/bin/tests/level1_foundation/exception/exception_test \
  -instr-profile=merged_coverage_level1.profdata \
  -format=html \
  -output-dir=coverage_html_report_level1_v1.3.8

# 3. 查看报告
open coverage_html_report_level1_v1.3.8/index.html
```

---

## 📈 覆盖率结果

| 模块 | 行覆盖率 | 函数覆盖率 | 状态 |
|------|----------|------------|------|
| Exception | 100.00% | 100.00% | ✅ 优秀 |
| Logger | 100.00% | 100.00% | ✅ 优秀 |
| Config | 1.12% | 2.04% | ⚠️ 需修复 |
| Types | 待测 | 待测 | ⏳ 待执行 |

---

## 📝 文档清单

- **测试报告**: `LEVEL1_FOUNDATION_TEST_REPORT_v1.3.8.md`
- **覆盖率报告**: `LEVEL1_COVERAGE_REPORT_v1.3.8.md`
- **工作日志**: `WORKLOG.md`
- **HTML 覆盖率**: `coverage_html_report_level1_v1.3.8/`

---

## ⚠️ 已知问题

### Config 模块覆盖率问题

**症状**: Config 模块覆盖率仅为 1.12%

**解决方案**:
```bash
# 清理缓存并重新运行
bazel clean --expunge
bazel test //tests/level1_foundation/config:config_test --nocache_test_results --test_output=all
```

### Types 模块测试未执行

**状态**: 待执行

**命令**:
```bash
bazel test //tests/level1_foundation/types:types_test --test_output=all
```

---

## 🔗 相关链接

- [Level 1 Foundation 测试报告](LEVEL1_FOUNDATION_TEST_REPORT_v1.3.8.md)
- [Level 1 覆盖率报告](LEVEL1_COVERAGE_REPORT_v1.3.8.md)
- [项目 README](README.md)
- [工作日志](WORKLOG.md)
