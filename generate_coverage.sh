#!/bin/bash
# generate_coverage.sh
# 作用: 完整生成 SQL Parser Lexer 测试的 LLVM 覆盖率 HTML 报告

set -e

# 1️⃣ 设置工作目录
WORKDIR="/home/liying/sqlcc"
cd "$WORKDIR"

# 2️⃣ 清理旧的 coverage 文件
echo "[INFO] 清理旧的 profraw/profdata 文件..."
rm -f *.profraw *.profdata
rm -rf coverage_html

# 3️⃣ 编译测试目标
TEST_TARGET="//tests/level2_core_services/sql_parser/lexer:test_concat_operator_extension"
echo "[INFO] 编译测试目标 $TEST_TARGET ..."
bazel build "$TEST_TARGET"

# 4️⃣ 执行测试并生成覆盖率数据
echo "[INFO] 运行测试并收集覆盖率..."
bazel test "$TEST_TARGET" \
    --collect_code_coverage \
    --instrumentation_filter="//src/sql_parser/.*" \
    --test_output=streamed \
    --combined_report=lcov \
    --jobs=1

# 5️⃣ 找到生成的 .profdata 文件
PROFDATA=$(find . -name "*.profdata" | head -1)
if [ -z "$PROFDATA" ]; then
    echo "[ERROR] 未找到 .profdata 文件，请检查 Bazel 测试输出"
    exit 1
fi
echo "[INFO] 使用 profdata 文件: $PROFDATA"

# 6️⃣ 使用 llvm-cov 生成 HTML 覆盖率报告
EXE_PATH=$(bazel info bazel-bin)/tests/level2_core_services/sql_parser/lexer/test_concat_operator_extension
echo "[INFO] 使用可执行文件: $EXE_PATH"

echo "[INFO] 生成 HTML 覆盖率报告..."
llvm-cov-20 show "$EXE_PATH" \
    -instr-profile="$PROFDATA" \
    --format=html \
    --show-line-counts-or-regions \
    --output-dir=coverage_html

echo "[INFO] HTML 报告生成完成，打开 index.html 查看:"
echo "file://$WORKDIR/coverage_html/index.html"

# 7️⃣ 可选：自动打开浏览器（Linux系统）
if command -v xdg-open &>/dev/null; then
    xdg-open "$WORKDIR/coverage_html/index.html"
fi

echo "[DONE] LLVM 覆盖率报告生成完毕！"
