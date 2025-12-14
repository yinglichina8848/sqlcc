#!/bin/bash
# 集成测试执行脚本

echo "开始执行集成测试..."

# 记录开始时间
INTEGRATION_START=$(date +%s)

# 1. 执行集成测试
echo "执行集成测试套件..."
bazel test //tests/integration/... --test_output=errors --test_timeout=120

# 2. 执行安全测试
echo "执行安全测试..."
bazel test //tests/components/security:security_component_tests --test_output=errors --test_timeout=120

# 3. 执行高级SQL测试
echo "执行高级SQL测试..."
bazel test //tests/advanced_sql/... --test_output=errors --test_timeout=120

# 4. 执行性能测试（可选）
if [ "$RUN_PERFORMANCE_TESTS" = "true" ]; then
    echo "执行性能测试..."
    bazel test //tests/performance/... --test_output=errors --test_timeout=300
fi

# 5. 统计测试结果
echo "统计集成测试结果..."

# 运行测试并捕获输出
INTEGRATION_TEST_OUTPUT=$(bazel test //tests/integration/... //tests/components/security:security_component_tests //tests/advanced_sql/... --test_output=summary 2>&1)
echo "$INTEGRATION_TEST_OUTPUT"

# 解析测试结果
INTEGRATION_TOTAL=$(echo "$INTEGRATION_TEST_OUTPUT" | grep -o "[0-9]\+ / [0-9]\+" | tail -1 | cut -d' ' -f3)
INTEGRATION_PASSED=$(echo "$INTEGRATION_TEST_OUTPUT" | grep -o "[0-9]\+ passed" | cut -d' ' -f1)
INTEGRATION_FAILED=$(echo "$INTEGRATION_TEST_OUTPUT" | grep -o "[0-9]\+ failed" | cut -d' ' -f1)

# 计算通过率
if [ -n "$INTEGRATION_TOTAL" ] && [ "$INTEGRATION_TOTAL" -gt 0 ]; then
    INTEGRATION_PASS_RATE=$((INTEGRATION_PASSED * 100 / INTEGRATION_TOTAL))
else
    INTEGRATION_PASS_RATE=0
fi

echo "集成测试结果:"
echo "- 总测试数: ${INTEGRATION_TOTAL:-0}"
echo "- 通过测试: ${INTEGRATION_PASSED:-0}"
echo "- 失败测试: ${INTEGRATION_FAILED:-0}"
echo "- 通过率: ${INTEGRATION_PASS_RATE}%"

# 4. 生成集成测试报告
INTEGRATION_END=$(date +%s)
INTEGRATION_DURATION=$((INTEGRATION_END - INTEGRATION_START))

cat > test_reports/integration_tests_$(date +%Y%m%d_%H%M%S).txt << EOF
集成测试执行报告
生成时间: $(date)
执行耗时: ${INTEGRATION_DURATION}秒

测试结果:
- 总测试数: ${INTEGRATION_TOTAL:-0}
- 通过测试: ${INTEGRATION_PASSED:-0}
- 失败测试: ${INTEGRATION_FAILED:-0}
- 通过率: ${INTEGRATION_PASS_RATE}%

详细输出:
$INTEGRATION_TEST_OUTPUT
EOF

# 5. 检查质量门禁
if [ "$INTEGRATION_PASS_RATE" -lt 85 ]; then
    echo "WARNING: 集成测试通过率过低 (${INTEGRATION_PASS_RATE}% < 85%)"
fi

echo "集成测试执行完成"
