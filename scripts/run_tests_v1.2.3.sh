#!/bin/bash

# SQLCC v1.2.3 测试执行脚本
# 逐个编译和执行测试，记录失败和通过的情况

set -e

# 配置
REPORT_DIR="/home/liying/sqlcc/docs/项目进展/v1.2.3"
REPORT_FILE="$REPORT_DIR/test_summary_report.md"
BAZEL_LOG="$REPORT_DIR/bazel_test_log.txt"

# 创建报告目录
mkdir -p "$REPORT_DIR"

# 初始化报告
echo "# SQLCC v1.2.3 测试总结报告" > "$REPORT_FILE"
echo "生成时间: $(date)" >> "$REPORT_FILE"
echo "" >> "$REPORT_FILE"

# 测试统计
total_tests=0
passed_tests=0
failed_tests=0

# 测试类型统计
unit_total=0
unit_passed=0
unit_failed=0

integration_total=0
integration_passed=0
integration_failed=0

performance_total=0
performance_passed=0
performance_failed=0

# 记录测试结果的函数
record_result() {
    local test_type=$1
    local test_name=$2
    local status=$3
    local details=$4
    
    echo "- $test_name: $status ($details)" >> "$REPORT_FILE"
    
    # 更新统计
    total_tests=$((total_tests + 1))
    if [ "$status" = "PASSED" ]; then
        passed_tests=$((passed_tests + 1))
        case $test_type in
            "UNIT")
                unit_passed=$((unit_passed + 1))
                ;;
            "INTEGRATION")
                integration_passed=$((integration_passed + 1))
                ;;
            "PERFORMANCE")
                performance_passed=$((performance_passed + 1))
                ;;
        esac
    else
        failed_tests=$((failed_tests + 1))
        case $test_type in
            "UNIT")
                unit_failed=$((unit_failed + 1))
                ;;
            "INTEGRATION")
                integration_failed=$((integration_failed + 1))
                ;;
            "PERFORMANCE")
                performance_failed=$((performance_failed + 1))
                ;;
        esac
    fi
}

# 运行单个测试的函数
run_test() {
    local test_target=$1
    local test_type=$2
    local test_name=$3
    
    echo "运行测试: $test_name ($test_target)"
    echo "========================"
    
    # 尝试运行测试
    if bazel test "$test_target" --test_output=errors > "$BAZEL_LOG" 2>&1; then
        record_result "$test_type" "$test_name" "PASSED" "测试通过"
        echo "✅ 测试通过: $test_name"
    else
        # 获取错误信息
        error_msg=$(tail -n 20 "$BAZEL_LOG" | tr '\n' ' ')
        record_result "$test_type" "$test_name" "FAILED" "$error_msg"
        echo "❌ 测试失败: $test_name - $error_msg"
    fi
    
    echo ""
}

# 更新测试类型统计
update_test_type_stats() {
    local test_type=$1
    local count=$2
    
    case $test_type in
        "UNIT")
            unit_total=$((unit_total + count))
            ;;
        "INTEGRATION")
            integration_total=$((integration_total + count))
            ;;
        "PERFORMANCE")
            performance_total=$((performance_total + count))
            ;;
    esac
}

# 开始测试执行
echo "开始执行 SQLCC v1.2.3 测试..."
echo ""

# 1. 运行单元测试
echo "## 1. 单元测试结果" >> "$REPORT_FILE"
echo "" >> "$REPORT_FILE"

# 运行我们已经验证可以工作的简单测试
run_test "//tests/unit:simple_network_test" "UNIT" "简单网络测试"

# 尝试运行其他单元测试（如果存在）
if bazel query 'tests(//tests/unit:*)' > /dev/null 2>&1; then
    unit_tests=$(bazel query 'tests(//tests/unit:*)' 2>/dev/null | head -5)
    for test in $unit_tests; do
        # 跳过我们已经运行过的测试
        if [ "$test" != "//tests/unit:simple_network_test" ]; then
            test_name=$(basename "$test")
            run_test "$test" "UNIT" "$test_name"
        fi
    done
fi

update_test_type_stats "UNIT" $((unit_passed + unit_failed))

# 2. 运行集成测试
echo "## 2. 集成测试结果" >> "$REPORT_FILE"
echo "" >> "$REPORT_FILE"

# 尝试运行集成测试
if bazel query 'tests(//tests/integration:*)' > /dev/null 2>&1; then
    integration_tests=$(bazel query 'tests(//tests/integration:*)' 2>/dev/null | head -3)
    for test in $integration_tests; do
        test_name=$(basename "$test")
        run_test "$test" "INTEGRATION" "$test_name"
    done
fi

update_test_type_stats "INTEGRATION" $((integration_passed + integration_failed))

# 3. 运行性能测试
echo "## 3. 性能测试结果" >> "$REPORT_FILE"
echo "" >> "$REPORT_FILE"

# 尝试运行性能测试
if bazel query 'tests(//tests/performance:*)' > /dev/null 2>&1; then
    performance_tests=$(bazel query 'tests(//tests/performance:*)' 2>/dev/null | head -3)
    for test in $performance_tests; do
        test_name=$(basename "$test")
        run_test "$test" "PERFORMANCE" "$test_name"
    done
fi

update_test_type_stats "PERFORMANCE" $((performance_passed + performance_failed))

# 添加测试统计摘要
echo "## 4. 测试统计摘要" >> "$REPORT_FILE"
echo "" >> "$REPORT_FILE"

echo "### 总体统计" >> "$REPORT_FILE"
echo "- 总测试数: $total_tests" >> "$REPORT_FILE"
echo "- 通过测试: $passed_tests" >> "$REPORT_FILE"
echo "- 失败测试: $failed_tests" >> "$REPORT_FILE"
echo "- 通过率: $(awk "BEGIN {printf \"%.2f\", $passed_tests*100/$total_tests}")%" >> "$REPORT_FILE"
echo "" >> "$REPORT_FILE"

echo "### 按测试类型统计" >> "$REPORT_FILE"
echo "" >> "$REPORT_FILE"

echo "#### 单元测试" >> "$REPORT_FILE"
echo "- 总数: $unit_total" >> "$REPORT_FILE"
echo "- 通过: $unit_passed" >> "$REPORT_FILE"
echo "- 失败: $unit_failed" >> "$REPORT_FILE"
if [ $unit_total -gt 0 ]; then
    echo "- 通过率: $(awk "BEGIN {printf \"%.2f\", $unit_passed*100/$unit_total}")%" >> "$REPORT_FILE"
fi
echo "" >> "$REPORT_FILE"

echo "#### 集成测试" >> "$REPORT_FILE"
echo "- 总数: $integration_total" >> "$REPORT_FILE"
echo "- 通过: $integration_passed" >> "$REPORT_FILE"
echo "- 失败: $integration_failed" >> "$REPORT_FILE"
if [ $integration_total -gt 0 ]; then
    echo "- 通过率: $(awk "BEGIN {printf \"%.2f\", $integration_passed*100/$integration_total}")%" >> "$REPORT_FILE"
fi
echo "" >> "$REPORT_FILE"

echo "#### 性能测试" >> "$REPORT_FILE"
echo "- 总数: $performance_total" >> "$REPORT_FILE"
echo "- 通过: $performance_passed" >> "$REPORT_FILE"
echo "- 失败: $performance_failed" >> "$REPORT_FILE"
if [ $performance_total -gt 0 ]; then
    echo "- 通过率: $(awk "BEGIN {printf \"%.2f\", $performance_passed*100/$performance_total}")%" >> "$REPORT_FILE"
fi
echo "" >> "$REPORT_FILE"

# 添加测试环境信息
echo "## 5. 测试环境信息" >> "$REPORT_FILE"
echo "" >> "$REPORT_FILE"
echo "- 测试时间: $(date)" >> "$REPORT_FILE"
echo "- Bazel版本: $(bazel version 2>/dev/null | head -1)" >> "$REPORT_FILE"
echo "- 系统信息: $(uname -a)" >> "$REPORT_FILE"
echo "" >> "$REPORT_FILE"

echo "测试执行完成。详细报告已生成: $REPORT_FILE"