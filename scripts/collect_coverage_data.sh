#!/bin/bash
# 覆盖率数据收集自动化脚本

echo "=== 覆盖率数据收集开始 ==="

# 记录开始时间
COVERAGE_START=$(date +%s)

# 1. 清理旧的覆盖率数据
echo "清理旧的覆盖率数据..."
rm -rf coverage_report/
rm -f bazel-out/_coverage/_coverage_report.dat
rm -f *.gcda *.gcno

# 2. 执行覆盖率测试
echo "执行覆盖率测试..."
if ! bazel coverage //tests/... --combined_report=lcov --test_output=errors --coverage_report_generator=@bazel_tools//tools/test/CoverageOutputGenerator/java/com/google/devtools/coverageoutputgenerator:Main; then
    echo "ERROR: 覆盖率测试执行失败"
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
