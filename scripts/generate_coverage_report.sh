#!/bin/bash

# SQLCC v1.2.10 覆盖率数据生成脚本
# 专门用于生成已通过测试的覆盖率数据

set -e

echo "=========================================="
echo "SQLCC v1.2.10 覆盖率数据生成"
echo "=========================================="
echo "开始时间: $(date)"
echo ""

# 创建覆盖率数据目录
COVERAGE_DIR="/tmp/coverage_data"
mkdir -p "$COVERAGE_DIR"

# 只运行已验证成功的测试
SUCCESSFUL_TESTS=(
    "comprehensive_bplus_tree_test"
    "final_bplus_tree_test"
    "minimal_bplus_tree_test"
)

echo "将为以下成功测试生成覆盖率数据:"
for test in "${SUCCESSFUL_TESTS[@]}"; do
    echo "  - $test"
done
echo ""

# 执行测试并收集覆盖率
for test in "${SUCCESSFUL_TESTS[@]}"; do
    echo "----------------------------------------"
    echo "收集 $test 的覆盖率数据"
    echo "----------------------------------------"

    # 设置覆盖率环境变量
    export LLVM_PROFILE_FILE="$COVERAGE_DIR/coverage_$test.profraw"

    # 执行测试
    if bazel test "//tests/storage_engine:$test" --test_timeout=60 --test_output=summary; then
        echo "✅ $test - 覆盖率数据收集成功"
    else
        echo "❌ $test - 测试失败，跳过覆盖率数据收集"
        continue
    fi

    echo ""
done

# 生成覆盖率报告
echo "=========================================="
echo "生成覆盖率报告"
echo "=========================================="

# 检查是否有覆盖率数据
if ls "$COVERAGE_DIR"/*.profraw 1> /dev/null 2>&1; then
    echo "发现覆盖率数据文件，正在合并..."

    # 合并覆盖率数据
    llvm-profdata merge "$COVERAGE_DIR"/*.profraw -o "$COVERAGE_DIR/coverage.profdata"
    echo "覆盖率数据合并完成"

    # 生成文本覆盖率报告
    echo "生成文本覆盖率报告..."
    llvm-cov show \
        --instr-profile="$COVERAGE_DIR/coverage.profdata" \
        --object=bazel-bin/tests/storage_engine/comprehensive_bplus_tree_test \
        --object=bazel-bin/tests/storage_engine/final_bplus_tree_test \
        --object=bazel-bin/tests/storage_engine/minimal_bplus_tree_test \
        --format=text \
        --output-dir="$COVERAGE_DIR/text" \
        src/storage_engine/ > "$COVERAGE_DIR/coverage_text_report.txt"

    # 生成HTML覆盖率报告
    echo "生成HTML覆盖率报告..."
    llvm-cov show \
        --instr-profile="$COVERAGE_DIR/coverage.profdata" \
        --object=bazel-bin/tests/storage_engine/comprehensive_bplus_tree_test \
        --format=html \
        --output-dir="$COVERAGE_DIR/html" \
        src/storage_engine/

    # 生成覆盖率摘要
    echo "生成覆盖率摘要..."
    llvm-cov report \
        --instr-profile="$COVERAGE_DIR/coverage.profdata" \
        --object=bazel-bin/tests/storage_engine/comprehensive_bplus_tree_test \
        --object=bazel-bin/tests/storage_engine/final_bplus_tree_test \
        --object=bazel-bin/tests/storage_engine/minimal_bplus_tree_test \
        src/storage_engine/ > "$COVERAGE_DIR/coverage_summary.txt"

    echo ""
    echo "=========================================="
    echo "覆盖率报告生成完成"
    echo "=========================================="
    echo "覆盖率数据目录: $COVERAGE_DIR"
    echo "HTML报告: $COVERAGE_DIR/html/index.html"
    echo "文本摘要: $COVERAGE_DIR/coverage_summary.txt"
    echo "详细文本报告: $COVERAGE_DIR/coverage_text_report.txt"

else
    echo "❌ 未找到覆盖率数据文件"
    echo "可能的原因:"
    echo "  - 测试执行失败"
    echo "  - LLVM_PROFILE_FILE环境变量未正确设置"
    echo "  - Clang编译器未启用覆盖率支持"
fi

# 清理环境变量
unset LLVM_PROFILE_FILE

echo ""
echo "=========================================="
echo "执行完成"
echo "=========================================="
echo "结束时间: $(date)"

exit 0
