#!/bin/bash
# 单元测试执行脚本

echo "开始执行单元测试..."

# 记录开始时间
UNIT_START=$(date +%s)

# 1. 执行基础单元测试
echo "执行基础单元测试..."
bazel test //tests/unit/... --test_output=errors --test_timeout=60

# 2. 执行组件单元测试
echo "执行组件单元测试..."
bazel test //tests/components/... --test_output=errors --test_timeout=60

# 3. 执行网络测试
echo "执行网络测试..."
bazel test //tests/network/... --test_output=errors --test_timeout=60

# 4. 执行存储引擎测试
echo "执行存储引擎测试..."
bazel test //tests/storage_engine/... --test_output=errors --test_timeout=60

# 5. 执行SQL解析器测试
echo "执行SQL解析器测试..."
bazel test //tests/sql_parser/... --test_output=errors --test_timeout=60

# 6. 执行SQL执行器测试
echo "执行SQL执行器测试..."
bazel test //tests/sql_executor/... --test_output=errors --test_timeout=60

# 7. 统计测试结果
echo "统计测试结果..."

# 运行测试并捕获输出
UNIT_TEST_OUTPUT=$(bazel test //tests/unit/... //tests/components/... //tests/network/... //tests/storage_engine/... //tests/sql_parser/... //tests/sql_executor/... --test_output=summary 2>&1)
echo "$UNIT_TEST_OUTPUT"

# 解析测试结果
TOTAL_TESTS=$(echo "$UNIT_TEST_OUTPUT" | grep -o "[0-9]\+ / [0-9]\+" | tail -1 | cut -d' ' -f3)
PASSED_TESTS=$(echo "$UNIT_TEST_OUTPUT" | grep -o "[0-9]\+ passed" | cut -d' ' -f1)
FAILED_TESTS=$(echo "$UNIT_TEST_OUTPUT" | grep -o "[0-9]\+ failed" | cut -d' ' -f1)

# 计算通过率
if [ -n "$TOTAL_TESTS" ] && [ "$TOTAL_TESTS" -gt 0 ]; then
    PASS_RATE=$((PASSED_TESTS * 100 / TOTAL_TESTS))
else
    PASS_RATE=0
fi

echo "单元测试结果:"
echo "- 总测试数: ${TOTAL_TESTS:-0}"
echo "- 通过测试: ${PASSED_TESTS:-0}"
echo "- 失败测试: ${FAILED_TESTS:-0}"
echo "- 通过率: ${PASS_RATE}%"

# 4. 生成单元测试报告
UNIT_END=$(date +%s)
UNIT_DURATION=$((UNIT_END - UNIT_START))

cat > test_reports/unit_tests_$(date +%Y%m%d_%H%M%S).txt << EOF
单元测试执行报告
生成时间: $(date)
执行耗时: ${UNIT_DURATION}秒

测试结果:
- 总测试数: ${TOTAL_TESTS:-0}
- 通过测试: ${PASSED_TESTS:-0}
- 失败测试: ${FAILED_TESTS:-0}
- 通过率: ${PASS_RATE}%

详细输出:
$UNIT_TEST_OUTPUT
EOF

# 5. 检查质量门禁
if [ "$PASS_RATE" -lt 80 ]; then
    echo "WARNING: 单元测试通过率过低 (${PASS_RATE}% < 80%)"
    # 不中断流水线，只是警告
fi

echo "单元测试执行完成"
