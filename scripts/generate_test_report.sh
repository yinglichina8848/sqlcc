#!/bin/bash
# 测试报告生成和汇总脚本

echo "生成测试报告汇总..."

# 记录生成时间
REPORT_TIME=$(date +%Y%m%d_%H%M%S)

# 1. 收集所有测试报告
echo "收集测试结果..."

# 查找最新的测试报告文件
UNIT_REPORT=$(ls -t test_reports/unit_tests_*.txt 2>/dev/null | head -1)
INTEGRATION_REPORT=$(ls -t test_reports/integration_tests_*.txt 2>/dev/null | head -1)
COMMUNICATION_REPORT=$(ls -t test_reports/communication_tests_*.txt 2>/dev/null | head -1)
E2E_REPORT=$(ls -t test_reports/e2e_tests_*.txt 2>/dev/null | head -1)

# 2. 解析各个报告的数据
echo "解析测试数据..."

# 解析单元测试数据
if [ -f "$UNIT_REPORT" ]; then
    UNIT_TOTAL=$(grep "总测试数:" "$UNIT_REPORT" | sed 's/.*总测试数: \([0-9]*\).*/\1/')
    UNIT_PASSED=$(grep "通过测试:" "$UNIT_REPORT" | sed 's/.*通过测试: \([0-9]*\).*/\1/')
    UNIT_FAILED=$(grep "失败测试:" "$UNIT_REPORT" | sed 's/.*失败测试: \([0-9]*\).*/\1/')
    UNIT_RATE=$(grep "通过率:" "$UNIT_REPORT" | sed 's/.*通过率: \([0-9]*\)%.*/\1/')
else
    UNIT_TOTAL="N/A"
    UNIT_PASSED="N/A"
    UNIT_FAILED="N/A"
    UNIT_RATE="N/A"
fi

# 解析集成测试数据
if [ -f "$INTEGRATION_REPORT" ]; then
    INTEGRATION_TOTAL=$(grep "总测试数:" "$INTEGRATION_REPORT" | sed 's/.*总测试数: \([0-9]*\).*/\1/')
    INTEGRATION_PASSED=$(grep "通过测试:" "$INTEGRATION_REPORT" | sed 's/.*通过测试: \([0-9]*\).*/\1/')
    INTEGRATION_FAILED=$(grep "失败测试:" "$INTEGRATION_REPORT" | sed 's/.*失败测试: \([0-9]*\).*/\1/')
    INTEGRATION_RATE=$(grep "通过率:" "$INTEGRATION_REPORT" | sed 's/.*通过率: \([0-9]*\)%.*/\1/')
else
    INTEGRATION_TOTAL="N/A"
    INTEGRATION_PASSED="N/A"
    INTEGRATION_FAILED="N/A"
    INTEGRATION_RATE="N/A"
fi

# 解析通信测试数据
if [ -f "$COMMUNICATION_REPORT" ]; then
    COMM_SUCCESS=$(grep -c "✅" "$COMMUNICATION_REPORT")
    COMM_TOTAL=$(grep -c "通过\|失败" "$COMMUNICATION_REPORT")
    if [ "$COMM_TOTAL" -gt 0 ]; then
        COMM_RATE=$((COMM_SUCCESS * 100 / COMM_TOTAL))
    else
        COMM_RATE=0
    fi
else
    COMM_RATE="N/A"
fi

# 解析端到端测试数据
if [ -f "$E2E_REPORT" ]; then
    E2E_TOTAL=$(grep "执行的脚本数:" "$E2E_REPORT" | sed 's/.*执行的脚本数: \([0-9]*\).*/\1/')
    E2E_PASSED=$(grep "成功的脚本数:" "$E2E_REPORT" | sed 's/.*成功的脚本数: \([0-9]*\).*/\1/')
    E2E_RATE=$(grep "通过率:" "$E2E_REPORT" | sed 's/.*通过率: \([0-9]*\)%.*/\1/')
else
    E2E_TOTAL="N/A"
    E2E_PASSED="N/A"
    E2E_RATE="N/A"
fi

# 3. 收集覆盖率数据
echo "收集覆盖率数据..."

if [ -f "coverage_report/summary.txt" ]; then
    LINE_COVERAGE=$(grep "lines......:" coverage_report/summary.txt | sed 's/.*lines......: \([0-9.]*\).*/\1/' | head -1)
    BRANCH_COVERAGE=$(grep "branches......:" coverage_report/summary.txt | sed 's/.*branches......: \([0-9.]*\).*/\1/' | head -1)
    FUNCTION_COVERAGE=$(grep "functions......:" coverage_report/summary.txt | sed 's/.*functions......: \([0-9.]*\).*/\1/' | head -1)
else
    LINE_COVERAGE="N/A"
    BRANCH_COVERAGE="N/A"
    FUNCTION_COVERAGE="N/A"
fi

# 4. 计算总体统计
echo "计算总体统计..."

# 计算总体通过率（简化计算）
TOTAL_PASS_RATE="N/A"
if [[ "$UNIT_RATE" =~ ^[0-9]+$ ]] && [[ "$INTEGRATION_RATE" =~ ^[0-9]+$ ]]; then
    TOTAL_PASS_RATE=$(( (UNIT_RATE + INTEGRATION_RATE) / 2 ))
fi

# 5. 生成汇总报告
echo "生成汇总报告..."

cat > test_reports/final_report_${REPORT_TIME}.txt << EOF
================================================================================
                         SQLCC 自动化测试流水线最终报告
================================================================================

生成时间: $(date)
报告ID: ${REPORT_TIME}

================================================================================
测试执行结果汇总
================================================================================

单元测试:
- 总测试数: ${UNIT_TOTAL}
- 通过测试: ${UNIT_PASSED}
- 失败测试: ${UNIT_FAILED}
- 通过率: ${UNIT_RATE}%

集成测试:
- 总测试数: ${INTEGRATION_TOTAL}
- 通过测试: ${INTEGRATION_PASSED}
- 失败测试: ${INTEGRATION_FAILED}
- 通过率: ${INTEGRATION_RATE}%

通信协议测试:
- 通过率: ${COMM_RATE}%

端到端测试:
- 执行脚本数: ${E2E_TOTAL}
- 成功脚本数: ${E2E_PASSED}
- 通过率: ${E2E_RATE}%

================================================================================
覆盖率分析结果
================================================================================

覆盖率指标:
- 行覆盖率: ${LINE_COVERAGE}%
- 分支覆盖率: ${BRANCH_COVERAGE}%
- 函数覆盖率: ${FUNCTION_COVERAGE}%

覆盖率报告位置:
- HTML报告: coverage_report/index.html (如果genhtml已安装)
- 详细数据: bazel-out/_coverage/_coverage_report.dat
- 模块分析: coverage_report/module_analysis.txt
- 趋势分析: coverage_report/trend_analysis.txt

================================================================================
质量评估
================================================================================

质量门禁检查:
EOF

# 添加质量门禁结果
if [[ "$UNIT_RATE" =~ ^[0-9]+$ ]] && [ "$UNIT_RATE" -ge 80 ]; then
    echo "- ✅ 单元测试通过率 ≥ 80%: ${UNIT_RATE}%" >> test_reports/final_report_${REPORT_TIME}.txt
else
    echo "- ❌ 单元测试通过率 < 80%: ${UNIT_RATE}%" >> test_reports/final_report_${REPORT_TIME}.txt
fi

if [[ "$INTEGRATION_RATE" =~ ^[0-9]+$ ]] && [ "$INTEGRATION_RATE" -ge 85 ]; then
    echo "- ✅ 集成测试通过率 ≥ 85%: ${INTEGRATION_RATE}%" >> test_reports/final_report_${REPORT_TIME}.txt
else
    echo "- ❌ 集成测试通过率 < 85%: ${INTEGRATION_RATE}%" >> test_reports/final_report_${REPORT_TIME}.txt
fi

if [[ "$LINE_COVERAGE" =~ ^[0-9]+(\.[0-9]+)?$ ]] && (( $(echo "$LINE_COVERAGE >= 11.5" | bc -l 2>/dev/null || echo "0") )); then
    echo "- ✅ 覆盖率 ≥ 11.5%: ${LINE_COVERAGE}%" >> test_reports/final_report_${REPORT_TIME}.txt
else
    echo "- ⚠️  覆盖率 < 11.5%: ${LINE_COVERAGE}%" >> test_reports/final_report_${REPORT_TIME}.txt
fi

cat >> test_reports/final_report_${REPORT_TIME}.txt << EOF

================================================================================
改进建议
================================================================================

基于测试结果，建议采取以下改进措施:

1. 测试覆盖率提升:
   - 优先改进网络服务模块 (当前8.9% → 目标25%)
   - 重点完善权限管理模块 (当前6.7% → 目标25%)
   - 扩展SQL解析器和执行引擎测试覆盖

2. 测试质量优化:
   - 完善通信协议测试用例
   - 增加端到端测试场景覆盖
   - 建立自动化回归测试机制

3. 基础设施改进:
   - 完善覆盖率数据收集流程
   - 建立测试结果趋势分析
   - 实施持续集成质量门禁

================================================================================
报告文件位置
================================================================================

详细报告文件:
- 单元测试: ${UNIT_REPORT:-未生成}
- 集成测试: ${INTEGRATION_REPORT:-未生成}
- 通信测试: ${COMMUNICATION_REPORT:-未生成}
- 端到端测试: ${E2E_REPORT:-未生成}
- 构建验证: test_reports/build_validation_*.txt

覆盖率文件:
- HTML报告: coverage_report/index.html
- 汇总数据: coverage_report/summary.txt
- 模块分析: coverage_report/module_analysis.txt

================================================================================
测试流水线完成
================================================================================
EOF

# 6. 显示最终状态
echo "=== 测试报告生成完成 ==="
echo "最终报告: test_reports/final_report_${REPORT_TIME}.txt"

# 显示关键指标
echo ""
echo "关键指标汇总:"
echo "- 单元测试通过率: ${UNIT_RATE}%"
echo "- 集成测试通过率: ${INTEGRATION_RATE}%"
echo "- 通信测试通过率: ${COMM_RATE}%"
echo "- 端到端测试通过率: ${E2E_RATE}%"
echo "- 行覆盖率: ${LINE_COVERAGE}%"

echo "测试报告生成完成"
