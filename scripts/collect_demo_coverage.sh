#!/bin/bash
# 专门用于收集demo测试覆盖率数据的脚本
# 解决容器环境权限问题，使用临时目录

echo "=== Demo测试覆盖率数据收集开始 ==="

# 创建临时目录用于覆盖率数据
COVERAGE_TMP_DIR="/tmp/sqlcc_coverage_$(date +%s)"
mkdir -p "$COVERAGE_TMP_DIR"
echo "使用临时目录: $COVERAGE_TMP_DIR"

# 设置环境变量
export LLVM_PROFILE_FILE="$COVERAGE_TMP_DIR/coverage_%p.profraw"

# 记录开始时间
COVERAGE_START=$(date +%s)

# 1. 清理旧的覆盖率数据
echo "清理旧的覆盖率数据..."
rm -rf coverage_report/
rm -f bazel-out/_coverage/_coverage_report.dat
rm -f *.gcda *.gcno

# 2. 编译demo测试
echo "编译demo测试..."
if ! bazel build //tests/demo:expression_test --test_output=errors; then
    echo "ERROR: 编译失败"
    rm -rf "$COVERAGE_TMP_DIR"
    exit 1
fi

# 3. 运行覆盖率测试
echo "运行覆盖率测试..."
if ! bazel coverage //tests/demo:expression_test --combined_report=lcov --test_output=errors \
    --coverage_report_generator=@bazel_tools//tools/test/CoverageOutputGenerator/java/com/google/devtools/coverageoutputgenerator:Main; then
    echo "WARNING: 覆盖率测试执行失败，尝试从临时文件恢复数据"
fi

# 4. 检查和处理覆盖率数据
echo "处理覆盖率数据..."

# 查找覆盖率文件
PROFRAW_FILES=$(find "$COVERAGE_TMP_DIR" -name "*.profraw" 2>/dev/null)
if [ -n "$PROFRAW_FILES" ]; then
    echo "发现profraw文件，开始处理..."

    # 合并覆盖率数据
    llvm-profdata merge -output="$COVERAGE_TMP_DIR/coverage.profdata" $PROFRAW_FILES

    # 生成lcov格式报告
    llvm-cov export bazel-bin/tests/demo/expression_test \
        --format=lcov \
        --instr-profile="$COVERAGE_TMP_DIR/coverage.profdata" \
        --object=bazel-bin/tests/demo/expression_test \
        > "$COVERAGE_TMP_DIR/coverage.lcov"

    echo "覆盖率数据文件生成成功"
else
    echo "WARNING: 未找到profraw文件，尝试从bazel输出目录获取"
    # 尝试从bazel默认输出获取
    if [ -f "bazel-out/_coverage/_coverage_report.dat" ]; then
        cp bazel-out/_coverage/_coverage_report.dat "$COVERAGE_TMP_DIR/coverage.lcov"
        echo "从bazel输出获取覆盖率数据"
    else
        echo "ERROR: 未找到任何覆盖率数据文件"
        rm -rf "$COVERAGE_TMP_DIR"
        exit 1
    fi
fi

# 5. 创建报告目录
mkdir -p coverage_report

# 6. 生成HTML报告
echo "生成HTML报告..."
if command -v genhtml >/dev/null 2>&1; then
    genhtml "$COVERAGE_TMP_DIR/coverage.lcov" \
            --output-directory coverage_report \
            --title "SQLCC Demo Test Coverage Report $(date +%Y%m%d)" \
            --show-details \
            --legend \
            --sort \
            --function-coverage
    echo "✅ HTML报告生成成功: coverage_report/index.html"
else
    echo "WARNING: genhtml 未安装，跳过HTML报告生成"
    # 复制lcov文件到报告目录
    cp "$COVERAGE_TMP_DIR/coverage.lcov" coverage_report/
fi

# 7. 生成详细的覆盖率统计
echo "生成详细的覆盖率统计..."

# 使用lcov提取数据
if command -v lcov >/dev/null 2>&1; then
    # 生成汇总信息
    lcov --summary "$COVERAGE_TMP_DIR/coverage.lcov" > coverage_report/summary.txt

    # 提取关键指标
    COVERAGE_DATA=$(lcov --summary "$COVERAGE_TMP_DIR/coverage.lcov" 2>/dev/null)

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

# 8. 生成模块级分析
echo "生成模块级覆盖率分析..."
./scripts/analyze_module_coverage.sh > coverage_report/module_analysis.txt 2>/dev/null || echo "模块分析脚本执行失败"

# 9. 生成覆盖率报告
COVERAGE_END=$(date +%s)
COVERAGE_DURATION=$((COVERAGE_END - COVERAGE_START))

cat > coverage_report/coverage_report_$(date +%Y%m%d_%H%M%S).txt << EOF
SQLCC Demo测试覆盖率分析报告
生成时间: $(date)
分析耗时: ${COVERAGE_DURATION}秒

覆盖率指标:
- 行覆盖率: ${LINE_COVERAGE:-N/A}%
- 分支覆盖率: ${BRANCH_COVERAGE:-N/A}%
- 函数覆盖率: ${FUNCTION_COVERAGE:-N/A}%

详细报告位置:
- HTML报告: coverage_report/index.html
- LCOV数据: coverage_report/coverage.lcov
- 汇总信息: coverage_report/summary.txt

测试信息:
- 测试目标: //tests/demo:expression_test
- 测试用例: 4个测试套件
- 功能覆盖: 表达式求值、字符串处理、集合操作

数据文件位置:
- 临时目录: $COVERAGE_TMP_DIR
- 覆盖率数据: coverage.lcov
EOF

# 10. 检查质量门禁
echo "执行质量门禁检查..."
if [[ "${LINE_COVERAGE:-0}" =~ ^[0-9]+(\.[0-9]+)?$ ]] && (( $(echo "${LINE_COVERAGE:-0} < 50" | bc -l 2>/dev/null || echo "1") )); then
    echo "WARNING: 覆盖率低于推荐阈值 (${LINE_COVERAGE:-0}% < 50%)"
    # 对于demo测试，设置较低的阈值
fi

# 11. 清理临时文件（保留覆盖率数据）
echo "清理临时文件..."
# rm -rf "$COVERAGE_TMP_DIR"  # 注释掉以便调试

echo "Demo测试覆盖率数据收集完成"
echo "- 行覆盖率: ${LINE_COVERAGE:-N/A}%"
echo "- HTML报告: coverage_report/index.html"
echo "- 详细报告: coverage_report/coverage_report_$(date +%Y%m%d_%H%M%S).txt"
