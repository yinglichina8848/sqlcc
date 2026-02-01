#!/bin/bash

# SQLCC Level 1 覆盖率报告生成脚本
# 通过 Bazel 运行此脚本生成完整的覆盖率报告

set -e

# 获取项目根目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../../.." && pwd)"
COVERAGE_DIR="$PROJECT_ROOT/coverage_report_l1_complete"

echo "=========================================="
echo "SQLCC Level 1 覆盖率报告生成"
echo "=========================================="
echo ""

# 运行所有 Level 1 覆盖率测试
echo "步骤 1: 运行所有 Level 1 覆盖率测试..."
cd "$PROJECT_ROOT"

bazel coverage \
    //tests/level1_foundation/exception:exception_test \
    //tests/level1_foundation/types:types_test \
    //tests/level1_foundation/logger:logger_test \
    //tests/level1_foundation/config:config_test \
    //tests/level1_foundation/basic:basic_test \
    //tests/level1_foundation/utils:utils_test \
    //tests/level1_foundation/utils:file_descriptor_version_test \
    //tests/level1_foundation/utils:ssl_connection_pool_test \
    //tests/level1_foundation/utils:smart_config_test \
    --test_output=errors \
    --combined_report=lcov

echo ""
echo "步骤 2: 收集覆盖率数据..."
"$SCRIPT_DIR/../../scripts/generate_l1_complete_coverage.sh"

echo ""
echo "=========================================="
echo "✅ 覆盖率报告生成完成!"
echo "=========================================="
echo ""
echo "📂 报告位置: $COVERAGE_DIR"
echo ""
echo "🌐 查看 HTML 报告:"
echo "  firefox $COVERAGE_DIR/all_tests/index.html"
