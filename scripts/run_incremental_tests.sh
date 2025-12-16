#!/bin/bash
# SQLCC 逐步递进测试执行器
# 支持容错执行，详细状态报告，多层次测试体系

set -u  # 检测未定义变量

# 配置参数
TEST_MODE="${TEST_MODE:-full}"
VERBOSE="${VERBOSE:-false}"
CONTINUE_ON_FAILURE="${CONTINUE_ON_FAILURE:-true}"
PARALLEL_BUILD="${PARALLEL_BUILD:-true}"
REPORT_FORMAT="${REPORT_FORMAT:-detailed}"

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 测试状态跟踪
declare -A TEST_RESULTS
declare -A TEST_DETAILS
declare -A COMPILATION_ERRORS
declare -A RUNTIME_ERRORS

# 日志函数
log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

log_verbose() {
    if [ "$VERBOSE" = "true" ]; then
        echo -e "${BLUE}[VERBOSE]${NC} $1"
    fi
}

# 初始化测试环境
init_test_env() {
    log_info "初始化测试环境..."

    # 创建测试报告目录
    mkdir -p test_reports

    # 创建临时目录
    TEST_TMP_DIR="/tmp/sqlcc_tests_$$"
    mkdir -p "$TEST_TMP_DIR"

    # 记录开始时间
    TEST_START_TIME=$(date +%s)

    log_success "测试环境初始化完成"
}

# 编译检查函数
check_compilation() {
    local target="$1"
    local category="$2"

    log_info "编译检查: $category - $target"

    # 记录编译开始时间
    local compile_start=$(date +%s)

    # 执行编译检查
    if bazel build "$target" > "$TEST_TMP_DIR/compile_${category}.log" 2>&1; then
        local compile_end=$(date +%s)
        local duration=$((compile_end - compile_start))

        TEST_RESULTS["${category}_compilation"]="PASS"
        TEST_DETAILS["${category}_compilation"]="编译成功 (${duration}s)"

        log_success "✅ $category 编译成功 (${duration}s)"
        return 0
    else
        local compile_end=$(date +%s)
        local duration=$((compile_end - compile_start))

        TEST_RESULTS["${category}_compilation"]="FAIL"
        TEST_DETAILS["${category}_compilation"]="编译失败 (${duration}s)"

        # 保存编译错误
        COMPILATION_ERRORS["$category"]=$(cat "$TEST_TMP_DIR/compile_${category}.log")

        log_error "❌ $category 编译失败 (${duration}s)"
        return 1
    fi
}

# 运行测试函数
run_test_suite() {
    local target="$1"
    local category="$2"
    local expected_tests="$3"

    log_info "运行测试: $category - $target"

    # 检查是否已编译
    if [ "${TEST_RESULTS[${category}_compilation]}" != "PASS" ]; then
        log_warning "⚠️  $category 编译失败，跳过测试执行"
        TEST_RESULTS["${category}_tests"]="SKIP"
        TEST_DETAILS["${category}_tests"]="因编译失败跳过"
        return 1
    fi

    # 记录测试开始时间
    local test_start=$(date +%s)

    # 执行测试
    if bazel test "$target" --test_output=errors --test_timeout=300 > "$TEST_TMP_DIR/test_${category}.log" 2>&1; then
        local test_end=$(date +%s)
        local duration=$((test_end - test_start))

        # 解析测试结果
        local test_output=$(cat "$TEST_TMP_DIR/test_${category}.log")
        local total_tests=$(echo "$test_output" | grep -o "[0-9]\+ / [0-9]\+" | tail -1 | cut -d' ' -f3 || echo "0")
        local passed_tests=$(echo "$test_output" | grep -o "[0-9]\+ passed" | cut -d' ' -f1 || echo "0")
        local failed_tests=$(echo "$test_output" | grep -o "[0-9]\+ failed" | cut -d' ' -f1 || echo "0")

        # 计算通过率
        local pass_rate="0"
        if [ -n "$total_tests" ] && [ "$total_tests" -gt 0 ]; then
            pass_rate=$((passed_tests * 100 / total_tests))
        fi

        TEST_RESULTS["${category}_tests"]="PASS"
        TEST_DETAILS["${category}_tests"]="${passed_tests}/${total_tests} 通过 (${pass_rate}%) - ${duration}s"

        if [ "$pass_rate" -ge 80 ]; then
            log_success "✅ $category 测试成功: ${passed_tests}/${total_tests} (${pass_rate}%)"
        else
            log_warning "⚠️  $category 测试部分失败: ${passed_tests}/${total_tests} (${pass_rate}%)"
        fi

        return 0
    else
        local test_end=$(date +%s)
        local duration=$((test_end - test_start))

        # 解析测试结果
        local test_output=$(cat "$TEST_TMP_DIR/test_${category}.log")
        local total_tests=$(echo "$test_output" | grep -o "[0-9]\+ / [0-9]\+" | tail -1 | cut -d' ' -f3 || echo "0")
        local passed_tests=$(echo "$test_output" | grep -o "[0-9]\+ passed" | cut -d' ' -f1 || echo "0")
        local failed_tests=$(echo "$test_output" | grep -o "[0-9]\+ failed" | cut -d' ' -f1 || echo "0")

        # 保存运行时错误
        RUNTIME_ERRORS["$category"]=$(cat "$TEST_TMP_DIR/test_${category}.log")

        local pass_rate="0"
        if [ -n "$total_tests" ] && [ "$total_tests" -gt 0 ]; then
            pass_rate=$((passed_tests * 100 / total_tests))
        fi

        TEST_RESULTS["${category}_tests"]="FAIL"
        TEST_DETAILS["${category}_tests"]="${passed_tests}/${total_tests} 通过 (${pass_rate}%) - ${duration}s"

        log_error "❌ $category 测试失败: ${passed_tests}/${total_tests} (${pass_rate}%)"
        return 1
    fi
}

# 收集覆盖率数据
collect_coverage() {
    log_info "收集覆盖率数据..."

    # 执行覆盖率测试
    if bazel coverage //tests/... --combined_report=lcov > "$TEST_TMP_DIR/coverage.log" 2>&1; then
        log_success "✅ 覆盖率数据收集成功"

        # 解析覆盖率数据
        if [ -f "bazel-out/_coverage/_coverage_report.dat" ]; then
            local coverage_data=$(lcov --summary bazel-out/_coverage/_coverage_report.dat 2>/dev/null || echo "")

            local line_coverage=$(echo "$coverage_data" | grep "lines......:" | sed 's/.*lines......: \([0-9.]*\).*/\1/' | head -1)
            local branch_coverage=$(echo "$coverage_data" | grep "branches......:" | sed 's/.*branches......: \([0-9.]*\).*/\1/' | head -1)
            local function_coverage=$(echo "$coverage_data" | grep "functions......:" | sed 's/.*functions......: \([0-9.]*\).*/\1/' | head -1)

            TEST_RESULTS["coverage"]="PASS"
            TEST_DETAILS["coverage"]="行: ${line_coverage}%, 分支: ${branch_coverage}%, 函数: ${function_coverage}%"

            log_info "覆盖率统计: 行 ${line_coverage}%, 分支 ${branch_coverage}%, 函数 ${function_coverage}%"
        else
            TEST_RESULTS["coverage"]="SKIP"
            TEST_DETAILS["coverage"]="覆盖率数据文件不存在"
            log_warning "⚠️  覆盖率数据文件不存在"
        fi
    else
        TEST_RESULTS["coverage"]="FAIL"
        TEST_DETAILS["coverage"]="覆盖率收集失败"
        log_error "❌ 覆盖率数据收集失败"
    fi
}

# 生成测试报告
generate_report() {
    local report_file="test_reports/incremental_test_report_$(date +%Y%m%d_%H%M%S).txt"

    log_info "生成详细测试报告: $report_file"

    cat > "$report_file" << EOF
================================================================================
                         SQLCC 逐步递进测试执行报告
================================================================================

执行时间: $(date)
测试模式: $TEST_MODE
详细模式: $VERBOSE
容错模式: $CONTINUE_ON_FAILURE

================================================================================
测试执行摘要
================================================================================

EOF

    # 计算总体统计
    local total_categories=0
    local passed_categories=0
    local failed_categories=0
    local skipped_categories=0

    # 显示各层测试结果
    echo "测试层次执行结果:" >> "$report_file"
    echo "" >> "$report_file"

    # 1. 编译检查层
    echo "1. 编译检查 (Compilation Checks)" >> "$report_file"
    local compile_passed=0
    local compile_total=0

    for category in "unit" "integration" "system" "performance"; do
        if [ -n "${TEST_RESULTS[${category}_compilation]}" ]; then
            ((compile_total++))
            echo "   ├── $category: ${TEST_RESULTS[${category}_compilation]} - ${TEST_DETAILS[${category}_compilation]}" >> "$report_file"
            if [ "${TEST_RESULTS[${category}_compilation]}" = "PASS" ]; then
                ((compile_passed++))
            fi
        fi
    done

    echo "   └── 编译通过率: $compile_passed/$compile_total ($(($compile_passed * 100 / $compile_total))%)" >> "$report_file"
    echo "" >> "$report_file"

    # 2. 单元测试层
    echo "2. 单元测试 (Unit Tests)" >> "$report_file"
    for category in "unit_core" "unit_network" "unit_storage" "unit_parser" "unit_executor"; do
        if [ -n "${TEST_RESULTS[${category}_tests]}" ]; then
            echo "   ├── $category: ${TEST_RESULTS[${category}_tests]} - ${TEST_DETAILS[${category}_tests]}" >> "$report_file"
        fi
    done
    echo "" >> "$report_file"

    # 3. 集成测试层
    echo "3. 集成测试 (Integration Tests)" >> "$report_file"
    for category in "integration" "communication" "e2e"; do
        if [ -n "${TEST_RESULTS[${category}_tests]}" ]; then
            echo "   ├── $category: ${TEST_RESULTS[${category}_tests]} - ${TEST_DETAILS[${category}_tests]}" >> "$report_file"
        fi
    done
    echo "" >> "$report_file"

    # 4. 系统测试层
    echo "4. 系统测试 (System Tests)" >> "$report_file"
    if [ -n "${TEST_RESULTS[system_tests]}" ]; then
        echo "   ├── system: ${TEST_RESULTS[system_tests]} - ${TEST_DETAILS[system_tests]}" >> "$report_file"
    fi
    echo "" >> "$report_file"

    # 5. 性能测试层
    echo "5. 性能测试 (Performance Tests)" >> "$report_file"
    if [ -n "${TEST_RESULTS[performance_tests]}" ]; then
        echo "   ├── performance: ${TEST_RESULTS[performance_tests]} - ${TEST_DETAILS[performance_tests]}" >> "$report_file"
    fi
    echo "" >> "$report_file"

    # 6. 覆盖率分析
    echo "6. 覆盖率分析 (Coverage Analysis)" >> "$report_file"
    if [ -n "${TEST_RESULTS[coverage]}" ]; then
        echo "   ├── coverage: ${TEST_RESULTS[coverage]} - ${TEST_DETAILS[coverage]}" >> "$report_file"
    fi
    echo "" >> "$report_file"

    # 编译错误详情
    if [ ${#COMPILATION_ERRORS[@]} -gt 0 ]; then
        echo "================================================================================
编译错误详情 (Compilation Errors)
================================================================================
" >> "$report_file"

        for category in "${!COMPILATION_ERRORS[@]}"; do
            echo "=== $category 编译错误 ===" >> "$report_file"
            echo "${COMPILATION_ERRORS[$category]}" >> "$report_file"
            echo "" >> "$report_file"
        done
    fi

    # 运行时错误详情
    if [ ${#RUNTIME_ERRORS[@]} -gt 0 ]; then
        echo "================================================================================
运行时错误详情 (Runtime Errors)
================================================================================
" >> "$report_file"

        for category in "${!RUNTIME_ERRORS[@]}"; do
            echo "=== $category 运行时错误 ===" >> "$report_file"
            echo "${RUNTIME_ERRORS[$category]}" >> "$report_file"
            echo "" >> "$report_file"
        done
    fi

    # 执行时间统计
    local end_time=$(date +%s)
    local total_duration=$((end_time - TEST_START_TIME))

    echo "================================================================================
执行统计 (Execution Statistics)
================================================================================

总执行时间: ${total_duration}秒
测试模式: $TEST_MODE
报告格式: $REPORT_FORMAT
临时文件目录: $TEST_TMP_DIR

================================================================================
质量评估 (Quality Assessment)
================================================================================
" >> "$report_file"

    # 质量评估
    local overall_status="PASS"
    local critical_failures=0

    # 检查关键指标
    for category in "unit" "integration"; do
        if [ "${TEST_RESULTS[${category}_compilation]}" = "FAIL" ]; then
            ((critical_failures++))
        fi
    done

    if [ $critical_failures -gt 0 ]; then
        overall_status="FAIL"
        echo "❌ 整体状态: 失败 (存在 $critical_failures 个关键编译失败)" >> "$report_file"
    else
        echo "✅ 整体状态: 通过" >> "$report_file"
    fi

    echo "" >> "$report_file"
    echo "改进建议:" >> "$report_file"

    # 生成改进建议
    if [ ${#COMPILATION_ERRORS[@]} -gt 0 ]; then
        echo "- 优先修复编译错误，确保所有模块能够成功构建" >> "$report_file"
    fi

    if [ ${#RUNTIME_ERRORS[@]} -gt 0 ]; then
        echo "- 分析运行时错误，提升测试用例的有效性" >> "$report_file"
    fi

    if [ "${TEST_DETAILS[coverage]}" = "覆盖率数据文件不存在" ]; then
        echo "- 完善覆盖率数据收集流程" >> "$report_file"
    fi

    echo "- 持续提升测试覆盖率，目标达到 30%+" >> "$report_file"

    cat >> "$report_file" << EOF

================================================================================
测试执行完成
================================================================================
EOF

    log_success "测试报告生成完成: $report_file"

    # 显示关键指标摘要
    echo ""
    echo "=== 测试执行摘要 ==="
    echo "总执行时间: ${total_duration}秒"
    echo "编译通过: $compile_passed/$compile_total"
    echo "覆盖率: ${TEST_DETAILS[coverage]}"
    echo "详细报告: $report_file"
}

# 主函数
main() {
    echo "================================================================================
                    SQLCC 逐步递进测试执行器
================================================================================
测试模式: $TEST_MODE
详细输出: $VERBOSE
容错执行: $CONTINUE_ON_FAILURE
并行构建: $PARALLEL_BUILD

开始时间: $(date)
================================================================================
"

    # 初始化
    init_test_env

    # 阶段1: 编译检查 (从简单到复杂)
    echo ""
    echo "=== 阶段1: 编译检查 (Compilation Checks) ==="

    # 1.1 单元测试编译检查
    check_compilation "//tests/unit/..." "unit"

    # 1.2 集成测试编译检查
    check_compilation "//tests/integration/..." "integration"

    # 1.3 系统测试编译检查
    check_compilation "//tests/system/..." "system" || true  # 允许失败

    # 1.4 性能测试编译检查
    check_compilation "//tests/performance/..." "performance" || true  # 允许失败

    # 阶段2: 单元测试执行
    echo ""
    echo "=== 阶段2: 单元测试执行 (Unit Tests) ==="

    # 2.1 核心单元测试
    run_test_suite "//tests/unit/core/..." "unit_core" "50" || [ "$CONTINUE_ON_FAILURE" = "true" ]

    # 2.2 网络单元测试
    run_test_suite "//tests/unit/network/..." "unit_network" "20" || [ "$CONTINUE_ON_FAILURE" = "true" ]

    # 2.3 存储引擎单元测试
    run_test_suite "//tests/unit/storage/..." "unit_storage" "30" || [ "$CONTINUE_ON_FAILURE" = "true" ]

    # 2.4 SQL解析器单元测试
    run_test_suite "//tests/unit/parser/..." "unit_parser" "40" || [ "$CONTINUE_ON_FAILURE" = "true" ]

    # 2.5 执行器单元测试
    run_test_suite "//tests/unit/executor/..." "unit_executor" "25" || [ "$CONTINUE_ON_FAILURE" = "true" ]

    # 阶段3: 集成测试执行
    if [ "$TEST_MODE" != "unit_only" ]; then
        echo ""
        echo "=== 阶段3: 集成测试执行 (Integration Tests) ==="

        # 3.1 集成测试
        run_test_suite "//tests/integration/..." "integration" "30" || [ "$CONTINUE_ON_FAILURE" = "true" ]

        # 3.2 通信协议测试
        run_test_suite "//tests/communication/..." "communication" "15" || [ "$CONTINUE_ON_FAILURE" = "true" ]

        # 阶段4: 端到端测试
        if [ "$TEST_MODE" = "full" ]; then
            echo ""
            echo "=== 阶段4: 端到端测试 (End-to-End Tests) ==="

            run_test_suite "//tests/e2e/..." "e2e" "10" || [ "$CONTINUE_ON_FAILURE" = "true" ]
        fi

        # 阶段5: 性能测试
        if [ "$TEST_MODE" = "full" ] || [ "$TEST_MODE" = "performance" ]; then
            echo ""
            echo "=== 阶段5: 性能测试 (Performance Tests) ==="

            run_test_suite "//tests/performance/..." "performance" "20" || [ "$CONTINUE_ON_FAILURE" = "true" ]
        fi
    fi

    # 阶段6: 覆盖率分析
    echo ""
    echo "=== 阶段6: 覆盖率分析 (Coverage Analysis) ==="

    collect_coverage

    # 生成报告
    echo ""
    generate_report

    # 清理临时文件
    if [ "$VERBOSE" != "true" ]; then
        rm -rf "$TEST_TMP_DIR"
    fi

    echo ""
    echo "================================================================================
测试执行完成
================================================================================
"

    # 返回状态
    local exit_code=0
    for status in "${TEST_RESULTS[@]}"; do
        if [ "$status" = "FAIL" ]; then
            exit_code=1
            break
        fi
    done

    exit $exit_code
}

# 参数解析
while [[ $# -gt 0 ]]; do
    case $1 in
        --mode)
            TEST_MODE="$2"
            shift 2
            ;;
        --verbose)
            VERBOSE=true
            shift
            ;;
        --fail-fast)
            CONTINUE_ON_FAILURE=false
            shift
            ;;
        --no-parallel)
            PARALLEL_BUILD=false
            shift
            ;;
        --report-format)
            REPORT_FORMAT="$2"
            shift 2
            ;;
        --help)
            echo "用法: $0 [选项]"
            echo ""
            echo "选项:"
            echo "  --mode MODE          测试模式 (unit_only, integration, full, performance) [默认: full]"
            echo "  --verbose           详细输出模式"
            echo "  --fail-fast         遇到失败立即停止"
            echo "  --no-parallel       禁用并行构建"
            echo "  --report-format FMT 报告格式 (summary, detailed) [默认: detailed]"
            echo "  --help              显示此帮助信息"
            exit 0
            ;;
        *)
            echo "未知选项: $1"
            echo "使用 --help 查看帮助信息"
            exit 1
            ;;
    esac
done

# 执行主函数
main "$@"
