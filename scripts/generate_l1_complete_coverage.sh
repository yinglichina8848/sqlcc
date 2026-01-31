#!/bin/bash

# SQLCC Level 1 完整覆盖率报告生成脚本
# 收集所有核心源码的覆盖率数据并生成专业报告

set -e

PROJECT_ROOT="/home/liying/sqlcc"
COVERAGE_DIR="$PROJECT_ROOT/coverage_report_l1_complete"
BAZEL_CACHE="/home/liying/.cache/bazel/_bazel_liying/68dbc53c53085b82ed46643b8af8ae0d/execroot/_main"

echo "=========================================="
echo "SQLCC Level 1 完整覆盖率报告生成"
echo "=========================================="

mkdir -p "$COVERAGE_DIR"

# 核心模块配置
declare -A CORE_MODULES=(
    ["types"]="src/types"
    ["config"]="src/utils"
    ["logger"]="src/logger"
    ["exception"]="src/exception"
    ["utils"]="src/utils"
    ["basic"]="src/exception"
)

# 测试模块配置
declare -A TEST_MODULES=(
    ["types"]="tests/level1_foundation/types:types_test"
    ["config"]="tests/level1_foundation/config:config_test"
    ["logger"]="tests/level1_foundation/logger:logger_test"
    ["exception"]="tests/level1_foundation/exception:exception_test"
    ["utils"]="tests/level1_foundation/utils:utils_test"
    ["basic"]="tests/level1_foundation/basic:basic_test"
)

echo "步骤 1: 验证测试结果..."
cd "$PROJECT_ROOT"
bazel test //tests/level1_foundation/... --test_output=errors 2>&1 | tail -10

echo ""
echo "步骤 2: 收集各模块覆盖率数据..."

# 收集每个测试模块的 profraw 文件
for MODULE in "${!TEST_MODULES[@]}"; do
    TEST_TARGET="${TEST_MODULES[$MODULE]}"

    echo "  📊 收集: $MODULE"

    # 查找覆盖率测试输出目录
    TEST_SUBDIR="${MODULE}_test"
    COV_DIR=$(find "$BAZEL_CACHE" -path "*_coverage*" -path "*/$TEST_SUBDIR/test" -type d 2>/dev/null | head -1)

    if [ -n "$COV_DIR" ] && [ -d "$COV_DIR" ]; then
        # 复制 profraw 文件到输出目录
        mkdir -p "$COVERAGE_DIR/$MODULE"
        cp "$COV_DIR"/*.profraw "$COVERAGE_DIR/$MODULE/" 2>/dev/null || true

        # 合并 profraw 文件
        if ls "$COVERAGE_DIR/$MODULE"/*.profraw 1> /dev/null 2>&1; then
            llvm-profdata-20 merge -o "$COVERAGE_DIR/$MODULE/$MODULE.profdata" "$COVERAGE_DIR/$MODULE"/*.profraw 2>/dev/null || true
            echo "    ✓ 覆盖率数据: $COVERAGE_DIR/$MODULE/$MODULE.profdata"
        fi
    fi
done

echo ""
echo "步骤 3: 生成各模块 HTML 覆盖率报告..."

for MODULE in "${!CORE_MODULES[@]}"; do
    SOURCE_DIR="${CORE_MODULES[$MODULE]}"
    TEST_COVERAGE_DIR="$COVERAGE_DIR/$MODULE"

    echo "  📊 生成报告: $MODULE"

    OUTPUT_DIR="$COVERAGE_DIR/$MODULE"
    mkdir -p "$OUTPUT_DIR"

    # 查找 object 文件
    OBJ_FILES=$(find "$BAZEL_CACHE" -path "*/$SOURCE_DIR/*_coverage/*.pic.o" 2>/dev/null | tr '\n' ' ')

    if [ -f "$TEST_COVERAGE_DIR/$MODULE.profdata" ] && [ -n "$OBJ_FILES" ]; then
        # 生成 HTML 报告
        llvm-cov-20 show \
            --instr-profile="$TEST_COVERAGE_DIR/$MODULE.profdata" \
            --format=html \
            --output-dir="$OUTPUT_DIR" \
            $OBJ_FILES \
            2>/dev/null || echo "    ⚠️  HTML 报告生成时出现警告"

        # 生成文本报告
        llvm-cov-20 show \
            --instr-profile="$TEST_COVERAGE_DIR/$MODULE.profdata" \
            --show-line-counts \
            --show-branches=count \
            $OBJ_FILES \
            2>/dev/null > "$OUTPUT_DIR/coverage_detailed.txt" || echo "    ⚠️  详细报告生成失败"

        # 生成汇总报告
        llvm-cov-20 report \
            --instr-profile="$TEST_COVERAGE_DIR/$MODULE.profdata" \
            $OBJ_FILES \
            2>/dev/null > "$OUTPUT_DIR/coverage_summary.txt" || echo "    ⚠️  汇总报告生成失败"
    fi

    # 验证报告生成
    if [ -f "$OUTPUT_DIR/index.html" ]; then
        echo "    ✓ 报告已生成: $OUTPUT_DIR/index.html"
    fi
done

echo ""
echo "步骤 4: 合并所有覆盖率数据..."

# 合并所有 profdata 文件
ALL_PROFDATA=$(find "$COVERAGE_DIR" -name "*.profdata" 2>/dev/null | tr '\n' ' ')
if [ -n "$ALL_PROFDATA" ]; then
    llvm-profdata-20 merge -o "$COVERAGE_DIR/merged_all.profdata" $ALL_PROFDATA 2>/dev/null || true
    echo "  ✓ 合并覆盖率数据: $COVERAGE_DIR/merged_all.profdata"
fi

echo ""
echo "=========================================="
echo "✅ 覆盖率报告生成完成!"
echo "=========================================="
echo ""
echo "📂 报告位置: $COVERAGE_DIR"
echo ""
echo "📁 生成的文件:"
find "$COVERAGE_DIR" -type f \( -name "*.html" -o -name "*.profdata" -o -name "*.txt" \) 2>/dev/null | head -20
echo ""
echo "🌐 查看报告:"
echo "  firefox $COVERAGE_DIR/index.html"
