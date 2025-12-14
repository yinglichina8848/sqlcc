#!/bin/bash
bazel build //... --verbose_failures  # 详细构建
bazel test //... --test_output=errors  # 运行测试
# 生成覆盖率报告 (参考SQLCC项目README)
bazel coverage //... --combined_report=lcov
genhtml --output-directory coverage_report bazel-out/_coverage/_coverage_report.dat