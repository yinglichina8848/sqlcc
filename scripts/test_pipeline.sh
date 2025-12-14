#!/bin/bash
# SQLCC 自动化测试流水线主脚本

set -e

# 配置参数
export TEST_MODE="${TEST_MODE:-full}"
export COVERAGE_ENABLED="${COVERAGE_ENABLED:-true}"
export COMMUNICATION_TEST="${COMMUNICATION_TEST:-true}"
export BUILD_ONLY="${BUILD_ONLY:-false}"

echo "=== SQLCC 自动化测试流水线启动 ==="
echo "测试模式: $TEST_MODE"
echo "覆盖率收集: $COVERAGE_ENABLED"
echo "通信测试: $COMMUNICATION_TEST"
echo "仅构建: $BUILD_ONLY"
echo "开始时间: $(date)"

# 记录开始时间
START_TIME=$(date +%s)

# 创建测试报告目录
mkdir -p test_reports
REPORT_FILE="test_reports/pipeline_report_$(date +%Y%m%d_%H%M%S).txt"

echo "=== SQLCC 自动化测试流水线报告 ===" > "$REPORT_FILE"
echo "开始时间: $(date)" >> "$REPORT_FILE"
echo "测试模式: $TEST_MODE" >> "$REPORT_FILE"
echo "" >> "$REPORT_FILE"

# 1. 环境准备
echo "=== 步骤1: 环境准备 ==="
echo "步骤1: 环境准备" >> "$REPORT_FILE"
if ./scripts/prepare_test_environment.sh >> "$REPORT_FILE" 2>&1; then
    echo "✅ 环境准备成功" | tee -a "$REPORT_FILE"
else
    echo "❌ 环境准备失败" | tee -a "$REPORT_FILE"
    exit 1
fi

# 2. 构建验证
echo "=== 步骤2: 构建验证 ==="
echo "" >> "$REPORT_FILE"
echo "步骤2: 构建验证" >> "$REPORT_FILE"
if ./scripts/validate_build.sh >> "$REPORT_FILE" 2>&1; then
    echo "✅ 构建验证成功" | tee -a "$REPORT_FILE"
else
    echo "❌ 构建验证失败" | tee -a "$REPORT_FILE"
    exit 1
fi

if [ "$BUILD_ONLY" = "true" ]; then
    echo "仅构建模式，跳过测试执行"
    echo "仅构建模式，跳过测试执行" >> "$REPORT_FILE"
else
    # 3. 单元测试执行
    echo "=== 步骤3: 单元测试执行 ==="
    echo "" >> "$REPORT_FILE"
    echo "步骤3: 单元测试执行" >> "$REPORT_FILE"
    if ./scripts/run_unit_tests.sh >> "$REPORT_FILE" 2>&1; then
        echo "✅ 单元测试执行成功" | tee -a "$REPORT_FILE"
    else
        echo "❌ 单元测试执行失败" | tee -a "$REPORT_FILE"
        # 单元测试失败不中断整个流水线
    fi

    # 4. 集成测试执行
    echo "=== 步骤4: 集成测试执行 ==="
    echo "" >> "$REPORT_FILE"
    echo "步骤4: 集成测试执行" >> "$REPORT_FILE"
    if ./scripts/run_integration_tests.sh >> "$REPORT_FILE" 2>&1; then
        echo "✅ 集成测试执行成功" | tee -a "$REPORT_FILE"
    else
        echo "❌ 集成测试执行失败" | tee -a "$REPORT_FILE"
    fi

    # 5. 全面测试执行（可选）
    if [ "$TEST_MODE" = "full" ]; then
        echo "=== 步骤5: 全面测试执行 ==="
        echo "" >> "$REPORT_FILE"
        echo "步骤5: 全面测试执行" >> "$REPORT_FILE"
        echo "执行全面测试套件..." >> "$REPORT_FILE"
        if bazel test //tests:comprehensive_tests --test_output=errors >> "$REPORT_FILE" 2>&1; then
            echo "✅ 全面测试执行成功" | tee -a "$REPORT_FILE"
        else
            echo "❌ 全面测试执行失败" | tee -a "$REPORT_FILE"
        fi
    fi

    # 5. 通信协议测试
    if [ "$COMMUNICATION_TEST" = "true" ]; then
        echo "=== 步骤5: 通信协议测试 ==="
        echo "" >> "$REPORT_FILE"
        echo "步骤5: 通信协议测试" >> "$REPORT_FILE"
        if ./scripts/test_communication_protocol.sh >> "$REPORT_FILE" 2>&1; then
            echo "✅ 通信协议测试成功" | tee -a "$REPORT_FILE"
        else
            echo "❌ 通信协议测试失败" | tee -a "$REPORT_FILE"
        fi
    fi

    # 6. 端到端测试
    echo "=== 步骤6: 端到端测试 ==="
    echo "" >> "$REPORT_FILE"
    echo "步骤6: 端到端测试" >> "$REPORT_FILE"
    if ./scripts/run_e2e_tests.sh >> "$REPORT_FILE" 2>&1; then
        echo "✅ 端到端测试成功" | tee -a "$REPORT_FILE"
    else
        echo "❌ 端到端测试失败" | tee -a "$REPORT_FILE"
    fi

    # 7. 覆盖率数据收集
    if [ "$COVERAGE_ENABLED" = "true" ]; then
        echo "=== 步骤7: 覆盖率数据收集 ==="
        echo "" >> "$REPORT_FILE"
        echo "步骤7: 覆盖率数据收集" >> "$REPORT_FILE"
        if ./scripts/collect_coverage_data.sh >> "$REPORT_FILE" 2>&1; then
            echo "✅ 覆盖率数据收集成功" | tee -a "$REPORT_FILE"
        else
            echo "❌ 覆盖率数据收集失败" | tee -a "$REPORT_FILE"
        fi
    fi
fi

# 8. 结果汇总和报告
echo "=== 步骤8: 结果汇总和报告 ==="
echo "" >> "$REPORT_FILE"
echo "步骤8: 结果汇总和报告" >> "$REPORT_FILE"
if ./scripts/generate_test_report.sh >> "$REPORT_FILE" 2>&1; then
    echo "✅ 报告生成成功" | tee -a "$REPORT_FILE"
else
    echo "❌ 报告生成失败" | tee -a "$REPORT_FILE"
fi

# 计算执行时间
END_TIME=$(date +%s)
DURATION=$((END_TIME - START_TIME))

echo "" >> "$REPORT_FILE"
echo "结束时间: $(date)" >> "$REPORT_FILE"
echo "总执行时间: ${DURATION}秒" >> "$REPORT_FILE"
echo "" >> "$REPORT_FILE"
echo "=== 自动化测试流水线完成 ===" >> "$REPORT_FILE"

echo "=== 自动化测试流水线完成 ==="
echo "总执行时间: ${DURATION}秒"
echo "详细报告: $REPORT_FILE"

# 显示最终状态
if grep -q "❌" "$REPORT_FILE"; then
    echo "流水线状态: ⚠️  部分失败"
    exit 1
else
    echo "流水线状态: ✅ 全部成功"
    exit 0
fi
