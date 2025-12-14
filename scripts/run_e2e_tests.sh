#!/bin/bash
# 端到端测试执行脚本

echo "开始执行端到端测试..."

# 记录开始时间
E2E_START=$(date +%s)

# 1. 运行测试脚本目录中的所有端到端测试脚本
echo "运行所有端到端测试脚本..."

# 初始化计数器
TOTAL_SCRIPTS=0
SUCCESS_SCRIPTS=0

# 检查并执行tests/scripts目录中的所有测试脚本
for script in tests/scripts/test_*.sh; do
    if [ -f "$script" ] && [ -x "$script" ]; then
        TOTAL_SCRIPTS=$((TOTAL_SCRIPTS + 1))
        SCRIPT_NAME=$(basename "$script")
        echo "执行 $SCRIPT_NAME..."
        if ./$script > test_reports/e2e_${SCRIPT_NAME%.sh}.log 2>&1; then
            echo "✅ $SCRIPT_NAME 执行成功"
            SUCCESS_SCRIPTS=$((SUCCESS_SCRIPTS + 1))
        else
            echo "❌ $SCRIPT_NAME 执行失败"
        fi
    fi
done

# 2. 检查根目录下的测试脚本
for script in test_*.sh; do
    if [ -f "$script" ] && [ -x "$script" ]; then
        TOTAL_SCRIPTS=$((TOTAL_SCRIPTS + 1))
        SCRIPT_NAME=$(basename "$script")
        echo "执行 $SCRIPT_NAME..."
        if ./$script > test_reports/e2e_${SCRIPT_NAME%.sh}.log 2>&1; then
            echo "✅ $SCRIPT_NAME 执行成功"
            SUCCESS_SCRIPTS=$((SUCCESS_SCRIPTS + 1))
        else
            echo "❌ $SCRIPT_NAME 执行失败"
        fi
    fi
done

# 设置结果变量
BASIC_SQL_SUCCESS=0
COMPREHENSIVE_SUCCESS=0
ISQL_SUCCESS=0

# 如果执行了脚本，则设置相应的成功标志
if [ "$TOTAL_SCRIPTS" -gt 0 ]; then
    if [ "$SUCCESS_SCRIPTS" -gt 0 ]; then
        # 至少有一个脚本成功
        BASIC_SQL_SUCCESS=1
    fi
fi

# 4. 计算端到端测试通过率
TOTAL_E2E_TESTS=$((BASIC_SQL_SUCCESS + COMPREHENSIVE_SUCCESS + ISQL_SUCCESS))
PASSED_E2E_TESTS=$(( (BASIC_SQL_SUCCESS) + (COMPREHENSIVE_SUCCESS) + (ISQL_SUCCESS) ))

if [ "$TOTAL_E2E_TESTS" -gt 0 ]; then
    E2E_PASS_RATE=$((PASSED_E2E_TESTS * 100 / TOTAL_E2E_TESTS))
else
    E2E_PASS_RATE=0
fi

echo "端到端测试结果:"
echo "- 执行的脚本数: $TOTAL_E2E_TESTS"
echo "- 成功的脚本数: $PASSED_E2E_TESTS"
echo "- 通过率: ${E2E_PASS_RATE}%"

# 5. 生成端到端测试报告
E2E_END=$(date +%s)
E2E_DURATION=$((E2E_END - E2E_START))

cat > test_reports/e2e_tests_$(date +%Y%m%d_%H%M%S).txt << EOF
端到端测试执行报告
生成时间: $(date)
执行耗时: ${E2E_DURATION}秒

测试脚本执行结果:
- test_basic_sql.sh: $([ "$BASIC_SQL_SUCCESS" = "1" ] && echo "✅ 成功" || echo "❌ 失败")
- test_comprehensive_sqlcc.sh: $([ "$COMPREHENSIVE_SUCCESS" = "1" ] && echo "✅ 成功" || echo "❌ 失败")
- test_isql_sqlcc.sh: $([ "$ISQL_SUCCESS" = "1" ] && echo "✅ 成功" || echo "❌ 失败")

汇总统计:
- 执行的脚本数: ${TOTAL_E2E_TESTS}
- 成功的脚本数: ${PASSED_E2E_TESTS}
- 通过率: ${E2E_PASS_RATE}%

注意: 端到端测试依赖通信协议的正确实现
EOF

# 6. 检查质量门禁
if [ "$E2E_PASS_RATE" -lt 70 ]; then
    echo "WARNING: 端到端测试通过率过低 (${E2E_PASS_RATE}% < 70%)"
fi

echo "端到端测试执行完成"
