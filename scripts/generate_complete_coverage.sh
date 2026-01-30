#!/bin/bash

# SQLCC Level 1 完整覆盖率数据收集和报告生成脚本
# 收集所有核心源码的详细覆盖率数据

set -e

PROJECT_ROOT="/home/liying/sqlcc"
COVERAGE_DIR="$PROJECT_ROOT/coverage_report_l1_complete"
PROFDATA="/tmp/complete_coverage.profdata"
BAZEL_CACHE="/home/liying/.cache/bazel/_bazel_liying/68dbc53c53085b82ed46643b8af8ae0d/execroot/_main"

echo "=========================================="
echo "SQLCC Level 1 完整覆盖率数据收集"
echo "=========================================="

mkdir -p "$COVERAGE_DIR"

# 核心模块配置
declare -A MODULES=(
    ["types"]="src/types:bazel-out/k8-fastbuild/bin/src/types/_objs/types_coverage/*.pic.o"
    ["config"]="src/utils:bazel-out/k8-fastbuild/bin/src/utils/_objs/utils_coverage/*.pic.o"
    ["logger"]="src/logger:bazel-out/k8-fastbuild/bin/src/logger/_objs/logger_coverage/*.pic.o"
    ["exception"]="src/exception:bazel-out/k8-fastbuild/bin/src/exception/_objs/exception_coverage/*.pic.o"
)

echo ""
echo "步骤 1: 生成各模块详细覆盖率报告..."

# 查找测试二进制文件
TEST_BINARIES=(
    "$BAZEL_CACHE/bazel-out/k8-fastbuild/bin/tests/level1_foundation/types/types_test"
    "$BAZEL_CACHE/bazel-out/k8-fastbuild/bin/tests/level1_foundation/config/config_test"
    "$BAZEL_CACHE/bazel-out/k8-fastbuild/bin/tests/level1_foundation/logger/logger_test"
    "$BAZEL_CACHE/bazel-out/k8-fastbuild/bin/tests/level1_foundation/exception/exception_test"
    "$BAZEL_CACHE/bazel-out/k8-fastbuild/bin/tests/level1_foundation/utils/utils_test"
    "$BAZEL_CACHE/bazel-out/k8-fastbuild/bin/tests/level1_foundation/basic/basic_test"
)

# 合并所有 profraw 文件
PROFRAW_FILES=$(find "$BAZEL_CACHE" -name "*.profraw" -path "*_coverage*" 2>/dev/null | tr '\n' ' ')
llvm-profdata-20 merge -o "$PROFDATA" $PROFRAW_FILES 2>/dev/null

# 收集所有 object 文件
ALL_OBJ_FILES=$(find "$BAZEL_CACHE" -name "*.pic.o" -path "*_coverage*" 2>/dev/null | tr '\n' ' ')

echo ""
echo "步骤 2: 生成模块级覆盖率报告..."

for MODULE in "${!MODULES[@]}"; do
    INFO="${MODULES[$MODULE]}"
    SOURCE_DIR=$(echo "$INFO" | cut -d: -f1)
    OBJ_PATTERN=$(echo "$INFO" | cut -d: -f2)

    echo "  📊 模块: $MODULE"

    OUTPUT_DIR="$COVERAGE_DIR/$MODULE"
    mkdir -p "$OUTPUT_DIR"

    # 收集该模块的 object 文件
    MODULE_OBJ_FILES=$(find "$BAZEL_CACHE/$OBJ_PATTERN" -name "*.pic.o" 2>/dev/null | tr '\n' ' ')

    if [ -n "$MODULE_OBJ_FILES" ]; then
        # 生成 HTML 报告
        llvm-cov-20 show \
            --instr-profile="$PROFDATA" \
            --format=html \
            --output-dir="$OUTPUT_DIR" \
            $MODULE_OBJ_FILES \
            2>/dev/null || echo "    ⚠️  HTML 报告生成时出现警告"

        # 生成文本详细报告
        llvm-cov-20 show \
            --instr-profile="$PROFDATA" \
            --show-line-counts \
            --show-branches=count \
            --show-expansions \
            $MODULE_OBJ_FILES \
            2>/dev/null > "$OUTPUT_DIR/coverage_detailed.txt" || echo "    ⚠️  详细报告生成失败"

        # 生成汇总报告
        llvm-cov-20 report \
            --instr-profile="$PROFDATA" \
            $MODULE_OBJ_FILES \
            2>/dev/null > "$OUTPUT_DIR/coverage_summary.txt" || echo "    ⚠️  汇总报告生成失败"

        echo "    ✓ 报告已生成: $OUTPUT_DIR/"
    fi
done

echo ""
echo "步骤 3: 生成完整项目汇总报告..."

# 收集所有源文件
ALL_SOURCE_FILES=$(find "$PROJECT_ROOT/src" -name "*.cpp" 2>/dev/null | tr '\n' ' ')

# 生成完整汇总报告
llvm-cov-20 report \
    --instr-profile="$PROFDATA" \
    $ALL_OBJ_FILES \
    2>/dev/null > "$COVERAGE_DIR/full_summary.txt" || echo "  ⚠️  完整汇总报告生成失败"

# 生成完整的覆盖率详情
llvm-cov-20 show \
    --instr-profile="$PROFDATA" \
    --show-line-counts \
    --show-branches=count \
    $ALL_OBJ_FILES \
    2>/dev/null > "$COVERAGE_DIR/complete_coverage.txt" || echo "  ⚠️  完整覆盖率详情生成失败"

echo ""
echo "步骤 4: 生成各模块执行统计..."

# 为每个模块生成执行统计
for MODULE in "${!MODULES[@]}"; do
    INFO="${MODULES[$MODULE]}"
    OBJ_PATTERN=$(echo "$INFO" | cut -d: -f2)

    MODULE_OBJ_FILES=$(find "$BAZEL_CACHE/$OBJ_PATTERN" -name "*.pic.o" 2>/dev/null | tr '\n' ' ')

    if [ -n "$MODULE_OBJ_FILES" ]; then
        llvm-cov-20 show \
            --instr-profile="$PROFDATA" \
            --show-line-counts \
            $MODULE_OBJ_FILES \
            2>/dev/null > "$COVERAGE_DIR/${MODULE}_execution.txt" || true
    fi
done

echo ""
echo "=========================================="
echo "✅ 覆盖率数据收集完成!"
echo "=========================================="
echo ""
echo "📂 报告位置: $COVERAGE_DIR"
echo ""
echo "📁 生成的文件:"
find "$COVERAGE_DIR" -type f -name "*.txt" -o -name "*.html" 2>/dev/null | head -30
