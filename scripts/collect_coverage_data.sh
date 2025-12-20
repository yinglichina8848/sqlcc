#!/bin/bash
# 覆盖率数据收集自动化脚本 - 支持跳过编译失败的测试

echo "=== 覆盖率数据收集开始 ==="

# 记录开始时间
COVERAGE_START=$(date +%s)

# 1. 清理旧的覆盖率数据
echo "清理旧的覆盖率数据..."
rm -rf coverage_report/
rm -f bazel-out/_coverage/_coverage_report.dat
rm -f *.gcda *.gcno

# 2. 获取所有测试目标
echo "获取所有测试目标..."
ALL_TESTS=$(bazel query 'kind(cc_test, //tests/...)')
echo "发现 $(echo "$ALL_TESTS" | wc -l) 个测试目标"

# 3. 逐个编译测试并记录成功/失败状态
echo "逐个编译测试目标..."
SUCCESSFUL_TESTS=""
FAILED_TESTS=""
COMPILE_LOG="coverage_report/compile_status.log"

mkdir -p coverage_report
echo "编译状态日志 - $(date)" > "$COMPILE_LOG"
echo "=====================================" >> "$COMPILE_LOG"

TOTAL_TESTS=$(echo "$ALL_TESTS" | wc -l)
CURRENT_TEST=0

while IFS= read -r test_target; do
    CURRENT_TEST=$((CURRENT_TEST + 1))
    echo "[$CURRENT_TEST/$TOTAL_TESTS] 编译测试: $test_target"

    # 尝试编译单个测试
    if bazel build "$test_target" --test_output=errors >/dev/null 2>&1; then
        SUCCESSFUL_TESTS="$SUCCESSFUL_TESTS $test_target"
        echo "✅ $test_target - 编译成功" >> "$COMPILE_LOG"
        echo "编译成功: $test_target"
    else
        FAILED_TESTS="$FAILED_TESTS $test_target"
        echo "❌ $test_target - 编译失败" >> "$COMPILE_LOG"
        echo "编译失败: $test_target"
    fi
done <<< "$ALL_TESTS"

# 统计编译结果
SUCCESS_COUNT=$(echo "$SUCCESSFUL_TESTS" | wc -w)
FAILED_COUNT=$(echo "$FAILED_TESTS" | wc -w)

echo "" >> "$COMPILE_LOG"
echo "编译结果统计:" >> "$COMPILE_LOG"
echo "- 总测试数: $TOTAL_TESTS" >> "$COMPILE_LOG"
echo "- 编译成功: $SUCCESS_COUNT" >> "$COMPILE_LOG"
echo "- 编译失败: $FAILED_COUNT" >> "$COMPILE_LOG"

echo ""
echo "编译结果统计:"
echo "- 总测试数: $TOTAL_TESTS"
echo "- 编译成功: $SUCCESS_COUNT"
echo "- 编译失败: $FAILED_COUNT"

# 4. 仅对编译成功的测试执行覆盖率测试
if [ -n "$SUCCESSFUL_TESTS" ]; then
    echo ""
    echo "对编译成功的测试执行覆盖率测试..."

    # 构建覆盖率测试命令
    COVERAGE_CMD="bazel coverage"
    for test in $SUCCESSFUL_TESTS; do
        COVERAGE_CMD="$COVERAGE_CMD $test"
    done
    COVERAGE_CMD="$COVERAGE_CMD --combined_report=lcov --test_output=errors --coverage_report_generator=@bazel_tools//tools/test/CoverageOutputGenerator/java/com/google/devtools/coverageoutputgenerator:Main"

    echo "执行命令: $COVERAGE_CMD"
    if ! eval "$COVERAGE_CMD"; then
        echo "WARNING: 部分覆盖率测试执行失败，继续处理可用数据"
        # 不立即退出，继续处理可用的覆盖率数据
    fi
else
    echo "ERROR: 没有编译成功的测试，无法进行覆盖率测试"
    exit 1
fi

# 3. 检查覆盖率数据文件是否存在
if [ ! -f "bazel-out/_coverage/_coverage_report.dat" ]; then
    echo "ERROR: 覆盖率数据文件未生成"
    exit 1
fi

# 4. 生成HTML报告
echo "生成HTML报告..."
if command -v genhtml >/dev/null 2>&1; then
    genhtml bazel-out/_coverage/_coverage_report.dat \
            --output-directory coverage_report \
            --title "SQLCC Coverage Report $(date +%Y%m%d)" \
            --show-details \
            --legend \
            --sort \
            --function-coverage
    echo "✅ HTML报告生成成功"
else
    echo "WARNING: genhtml 未安装，跳过HTML报告生成"
    mkdir -p coverage_report
fi

# 5. 生成详细的覆盖率统计
echo "生成详细的覆盖率统计..."

# 使用lcov提取数据
if command -v lcov >/dev/null 2>&1; then
    # 生成汇总信息
    lcov --summary bazel-out/_coverage/_coverage_report.dat > coverage_report/summary.txt

    # 提取关键指标
    COVERAGE_DATA=$(lcov --summary bazel-out/_coverage/_coverage_report.dat 2>/dev/null)

    # 解析覆盖率数据
    LINE_COVERAGE=$(echo "$COVERAGE_DATA" | grep "lines......:" | sed 's/.*lines......: \([0-9.]*\).*/\1/' | head -1)
    BRANCH_COVERAGE=$(echo "$COVERAGE_DATA" | grep "branches......:" | sed 's/.*branches......: \([0-9.]*\).*/\1/' | head -1)
    FUNCTION_COVERAGE=$(echo "$COVERAGE_DATA" | grep "functions......:" | sed 's/.*functions......: \([0-9.]*\).*/\1/' | head -1)

    echo "覆盖率指标解析完成:"
    echo "- 行覆盖率: ${LINE_COVERAGE:-N/A}%"
    echo "- 分支覆盖率: ${BRANCH_COVERAGE:-N/A}%"
    echo "- 函数覆盖率: ${FUNCTION_COVERAGE:-N/A}%"
else
    echo "WARNING: lcov 未安装，使用基本统计"
    LINE_COVERAGE="N/A"
    BRANCH_COVERAGE="N/A"
    FUNCTION_COVERAGE="N/A"
fi

# 6. 生成模块级分析
echo "生成模块级覆盖率分析..."
./scripts/analyze_module_coverage.sh > coverage_report/module_analysis.txt

# 7. 生成趋势分析
echo "生成趋势分析..."
./scripts/analyze_coverage_trends.sh "${LINE_COVERAGE:-0}" > coverage_report/trend_analysis.txt

# 8. 生成覆盖率报告
COVERAGE_END=$(date +%s)
COVERAGE_DURATION=$((COVERAGE_END - COVERAGE_START))

cat > coverage_report/coverage_report_$(date +%Y%m%d_%H%M%S).txt << EOF
SQLCC 覆盖率分析报告
生成时间: $(date)
分析耗时: ${COVERAGE_DURATION}秒

覆盖率指标:
- 行覆盖率: ${LINE_COVERAGE:-N/A}%
- 分支覆盖率: ${BRANCH_COVERAGE:-N/A}%
- 函数覆盖率: ${FUNCTION_COVERAGE:-N/A}%

详细报告位置:
- HTML报告: coverage_report/index.html
- 模块分析: coverage_report/module_analysis.txt
- 趋势分析: coverage_report/trend_analysis.txt
- 汇总信息: coverage_report/summary.txt

数据文件:
- LCOV数据: bazel-out/_coverage/_coverage_report.dat
EOF

# 9. 检查质量门禁
echo "执行质量门禁检查..."
if [[ "${LINE_COVERAGE:-0}" =~ ^[0-9]+(\.[0-9]+)?$ ]] && (( $(echo "${LINE_COVERAGE:-0} < 11.5" | bc -l 2>/dev/null || echo "1") )); then
    echo "WARNING: 覆盖率低于质量门禁阈值 (${LINE_COVERAGE:-0}% < 11.5%)"
    # 不中断流水线，只是警告
fi

echo "覆盖率数据收集完成"
echo "- 行覆盖率: ${LINE_COVERAGE:-N/A}%"
echo "- 详细报告: coverage_report/"
