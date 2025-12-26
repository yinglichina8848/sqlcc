#!/bin/bash

# SQLCC v1.2.10 存储引擎测试覆盖率执行脚本
# 执行用户指定的编译成功的测试，并收集覆盖率数据

set -e  # 遇到错误立即退出

echo "=========================================="
echo "SQLCC v1.2.10 存储引擎测试覆盖率执行"
echo "=========================================="
echo "开始时间: $(date)"
echo ""

# 创建覆盖率数据目录
COVERAGE_DIR="/tmp/coverage_data"
mkdir -p "$COVERAGE_DIR"

# 测试列表（用户指定的编译成功的测试）
TESTS=(
    "comprehensive_bplus_tree_test"
    "final_bplus_tree_test"
    "minimal_bplus_tree_test"
    "test_bplus_tree_fix"
    "b_plus_tree_core_test"
    "storage_engine_comprehensive_test"
    "storage_engine_boundary_test"
    "page_allocator_test"
    "index_manager_test"
    "buffer_pool_test"
    "concurrency_control_test"
    "data_integrity_test"
    "disk_manager_test"
)

echo "将执行以下测试（带覆盖率）:"
for test in "${TESTS[@]}"; do
    echo "  - $test"
done
echo ""

# 执行测试并收集覆盖率
FAILED_TESTS=()
PASSED_TESTS=()

for test in "${TESTS[@]}"; do
    echo "----------------------------------------"
    echo "执行测试: $test"
    echo "----------------------------------------"

    # 设置覆盖率环境变量
    export LLVM_PROFILE_FILE="$COVERAGE_DIR/coverage_$test.profraw"

    # 执行测试
    if bazel test "//tests/storage_engine:$test" --test_timeout=60 --test_output=summary; then
        echo "✅ $test - PASSED"
        PASSED_TESTS+=("$test")
    else
        echo "❌ $test - FAILED"
        FAILED_TESTS+=("$test")
    fi

    echo ""
done

# 生成覆盖率报告
echo "=========================================="
echo "生成覆盖率报告"
echo "=========================================="

# 合并覆盖率数据
echo "合并覆盖率数据..."
llvm-profdata merge "$COVERAGE_DIR"/*.profraw -o "$COVERAGE_DIR/coverage.profdata"

# 生成文本报告
echo "生成文本覆盖率报告..."
llvm-cov show \
    --instr-profile="$COVERAGE_DIR/coverage.profdata" \
    --object=bazel-bin/tests/storage_engine/comprehensive_bplus_tree_test \
    --object=bazel-bin/tests/storage_engine/final_bplus_tree_test \
    --object=bazel-bin/tests/storage_engine/minimal_bplus_tree_test \
    --object=bazel-bin/tests/storage_engine/test_bplus_tree_fix \
    --object=bazel-bin/tests/storage_engine/b_plus_tree_core_test \
    --object=bazel-bin/tests/storage_engine/storage_engine_comprehensive_test \
    --object=bazel-bin/tests/storage_engine/storage_engine_boundary_test \
    --object=bazel-bin/tests/storage_engine/page_allocator_test \
    --object=bazel-bin/tests/storage_engine/index_manager_test \
    --object=bazel-bin/tests/storage_engine/buffer_pool_test \
    --object=bazel-bin/tests/storage_engine/concurrency_control_test \
    --object=bazel-bin/tests/storage_engine/data_integrity_test \
    --object=bazel-bin/tests/storage_engine/disk_manager_test \
    --format=text \
    --output-dir="$COVERAGE_DIR/html" \
    src/ include/

# 生成HTML报告
echo "生成HTML覆盖率报告..."
llvm-cov show \
    --instr-profile="$COVERAGE_DIR/coverage.profdata" \
    --object=bazel-bin/tests/storage_engine/comprehensive_bplus_tree_test \
    --format=html \
    --output-dir="$COVERAGE_DIR/html" \
    src/ include/

# 生成覆盖率摘要
echo "生成覆盖率摘要..."
llvm-cov report \
    --instr-profile="$COVERAGE_DIR/coverage.profdata" \
    --object=bazel-bin/tests/storage_engine/comprehensive_bplus_tree_test \
    --object=bazel-bin/tests/storage_engine/final_bplus_tree_test \
    --object=bazel-bin/tests/storage_engine/minimal_bplus_tree_test \
    --object=bazel-bin/tests/storage_engine/test_bplus_tree_fix \
    --object=bazel-bin/tests/storage_engine/b_plus_tree_core_test \
    --object=bazel-bin/tests/storage_engine/storage_engine_comprehensive_test \
    --object=bazel-bin/tests/storage_engine/storage_engine_boundary_test \
    --object=bazel-bin/tests/storage_engine/page_allocator_test \
    --object=bazel-bin/tests/storage_engine/index_manager_test \
    --object=bazel-bin/tests/storage_engine/buffer_pool_test \
    --object=bazel-bin/tests/storage_engine/concurrency_control_test \
    --object=bazel-bin/tests/storage_engine/data_integrity_test \
    --object=bazel-bin/tests/storage_engine/disk_manager_test \
    src/ include/ > "$COVERAGE_DIR/coverage_summary.txt"

# 统计结果
echo "=========================================="
echo "测试执行结果统计"
echo "=========================================="
echo "总测试数量: ${#TESTS[@]}"
echo "通过测试数量: ${#PASSED_TESTS[@]}"
echo "失败测试数量: ${#FAILED_TESTS[@]}"
echo ""

if [ ${#PASSED_TESTS[@]} -gt 0 ]; then
    echo "✅ 通过的测试:"
    for test in "${PASSED_TESTS[@]}"; do
        echo "  - $test"
    done
    echo ""
fi

if [ ${#FAILED_TESTS[@]} -gt 0 ]; then
    echo "❌ 失败的测试:"
    for test in "${FAILED_TESTS[@]}"; do
        echo "  - $test"
    done
    echo ""
fi

echo "=========================================="
echo "覆盖率报告位置"
echo "=========================================="
echo "覆盖率数据目录: $COVERAGE_DIR"
echo "HTML报告: $COVERAGE_DIR/html/index.html"
echo "文本摘要: $COVERAGE_DIR/coverage_summary.txt"
echo ""

echo "=========================================="
echo "执行完成"
echo "=========================================="
echo "结束时间: $(date)"

# 清理环境变量
unset LLVM_PROFILE_FILE

exit 0
