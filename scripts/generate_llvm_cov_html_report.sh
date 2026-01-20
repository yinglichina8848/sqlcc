#!/bin/bash
# File: scripts/generate_llvm_cov_html_report.sh
# Description: 工业级 Bazel + LLVM 覆盖率生成脚本
# Usage:
#   bash scripts/generate_llvm_cov_html_report.sh [TEST_TARGET] [OUTPUT_DIR]
#   TEST_TARGET 默认: //tests/...
#   OUTPUT_DIR 默认: ./coverage_html

set -euo pipefail

# ------------------------------
# 1️⃣ 参数配置
# ------------------------------
TEST_TARGET=${1:-"//tests/..."}

# 解析输出目录参数
if [[ "${2:-}" == --* ]]; then
    # 第二个参数是Bazel flag，不是输出目录
    OUTPUT_DIR="$PWD/coverage_html"
else
    OUTPUT_DIR=${2:-"$PWD/coverage_html"}
fi
TMP_PROFDATA="$PWD/coverage.profdata"

echo "========================================="
echo "SQLCC LLVM-COV 覆盖率生成脚本"
echo "目标测试: $TEST_TARGET"
echo "输出目录: $OUTPUT_DIR"
echo "临时 profdata: $TMP_PROFDATA"
echo "========================================="

# ------------------------------
# 2️⃣ 执行 Bazel 覆盖率，生成 .profraw 文件
# ------------------------------
echo "[INFO] 执行 Bazel 覆盖率..."
bazel coverage "$TEST_TARGET" \
    --combined_report=lcov \
    --experimental_use_llvm_covmap \
    --test_output=errors \
    --test_timeout=300

# ------------------------------
# 3️⃣ 收集 .profraw 文件
# ------------------------------
PROFRAW_FILES=$(find bazel-out/ -name "*.profraw")
if [ -z "$PROFRAW_FILES" ]; then
    echo "[ERROR] 未找到 .profraw 文件，请检查 Bazel 覆盖率是否正确启用"
    exit 1
fi
echo "[INFO] 找到 $(echo "$PROFRAW_FILES" | wc -l) 个 .profraw 文件"

# ------------------------------
# 4️⃣ 合并 profraw -> profdata
# ------------------------------
echo "[INFO] 合并 .profraw -> $TMP_PROFDATA"
llvm-profdata merge -sparse $PROFRAW_FILES -o "$TMP_PROFDATA"

# ------------------------------
# 5️⃣ 找到测试 binary
# ------------------------------
BINARIES=$(find bazel-bin/ -type f -executable | grep "_test$")
if [ -z "$BINARIES" ]; then
    echo "[ERROR] 未找到测试 binary"
    exit 1
fi

# ------------------------------
# 6️⃣ 生成 HTML 覆盖率报告
# ------------------------------
mkdir -p "$OUTPUT_DIR"
for BIN in $BINARIES; do
    echo "[INFO] 生成 HTML 报告: $BIN"
    llvm-cov show "$BIN" \
        -instr-profile="$TMP_PROFDATA" \
        -format=html \
        -output-dir="$OUTPUT_DIR" \
        -ignore-filename-regex="bazel-out|external" \
        -Xdemangler=c++filt
done

# ------------------------------
# 7️⃣ 输出覆盖率 summary
# ------------------------------
for BIN in $BINARIES; do
    echo "[INFO] 覆盖率 summary: $BIN"
    llvm-cov report "$BIN" -instr-profile="$TMP_PROFDATA"
done

echo "========================================="
echo "🎉 SQLCC LLVM-COV 覆盖率生成完成!"
echo "📁 HTML 报告目录: $OUTPUT_DIR"
echo "========================================="
