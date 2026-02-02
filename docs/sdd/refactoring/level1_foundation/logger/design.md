# Logger 模块重构设计

**模块**: logger  
**版本**: v1.3.8  
**日期**: 2026-01-30  
**状态**: ✅ 已完成

---

## 1. 架构决策

| 决策 | 内容 | 理由 | 状态 |
|------|------|------|------|
| ADR-L001 | 测试框架使用 GoogleTest | 项目标准 | 已批准 |
| ADR-L002 | 覆盖所有日志级别 | DEBUG, INFO, WARN, ERROR | 已批准 |

---

## 2. 测试用例

| 测试组 | 测试数 | 覆盖功能 |
|--------|--------|----------|
| LoggerBasicTest | 4 | 级别、初始化 |
| LoggerFileTest | 3 | 文件写入、追加 |
| LoggerThreadSafetyTest | 3 | 并发安全 |
| LoggerPerformanceTest | 2 | 性能基准 |
| LoggerMacroTest | 2 | 宏定义 |
| LoggerIntegrationTest | 2 | 集成测试 |
| LoggerExceptionSafetyTest | 2 | 异常安全 |
| LoggerCustomTest | 2 | 自定义测试 |

---

## 3. BUILD 配置

```bazel
# tests/level1_foundation/logger/BUILD.bazel

cc_test(
    name = "logger_test",
    srcs = ["logger_test.cpp"],
    copts = [
        "-fprofile-instr-generate",
        "-fcoverage-mapping",
    ],
    linkopts = ["-fprofile-instr-generate"],
    deps = [
        "@com_google_googletest//:gtest",
        "@com_google_googletest//:gtest_main",
        "//src/logger:logger_coverage",
    ],
    tags = ["coverage", "level1", "logger"],
)
```

---

**维护者**: SQLCC Team  
**完成日期**: 2026-01-30
