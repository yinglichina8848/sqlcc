#!/bin/bash

# SQLCC 覆盖率测试运行脚本
# 运行真实的覆盖率测试并生成详细报告

set -e

# 配置变量
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
COVERAGE_DIR="${PROJECT_ROOT}/coverage_results"
REPORT_DIR="${PROJECT_ROOT}/coverage_reports"

# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# 日志函数
log_info() {
    echo -e "$(date '+%Y-%m-%d %H:%M:%S') ${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "$(date '+%Y-%m-%d %H:%M:%S') ${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "$(date '+%Y-%m-%d %H:%M:%S') ${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "$(date '+%Y-%m-%d %H:%M:%S') ${RED}[ERROR]${NC} $1"
}

# 显示帮助信息
show_help() {
    echo "用法: $0 [选项]"
    echo ""
    echo "选项:"
    echo "  -u, --unit-only        只运行单元测试覆盖率"
    echo "  -i, --integration-only 只运行集成测试覆盖率"
    echo "  -p, --performance-only 只运行性能测试覆盖率"
    echo "  -a, --all             运行所有覆盖率测试 (默认)"
    echo "  -o, --output DIR      指定输出目录"
    echo "  -h, --help            显示帮助信息"
    echo ""
    echo "示例:"
    echo "  ./scripts/run_coverage_tests.sh --unit-only                    # 只运行单元测试覆盖率"
    echo "  ./scripts/run_coverage_tests.sh --all -o ./my_coverage        # 运行所有测试，输出到指定目录"
}

# 解析命令行参数
parse_args() {
    UNIT_TESTS=true
    INTEGRATION_TESTS=true
    PERFORMANCE_TESTS=true

    while [[ $# -gt 0 ]]; do
        case $1 in
            -u|--unit-only)
                INTEGRATION_TESTS=false
                PERFORMANCE_TESTS=false
                shift
                ;;
            -i|--integration-only)
                UNIT_TESTS=false
                PERFORMANCE_TESTS=false
                shift
                ;;
            -p|--performance-only)
                UNIT_TESTS=false
                INTEGRATION_TESTS=false
                shift
                ;;
            -a|--all)
                UNIT_TESTS=true
                INTEGRATION_TESTS=true
                PERFORMANCE_TESTS=true
                shift
                ;;
            -o|--output)
                REPORT_DIR="$2"
                shift 2
                ;;
            -h|--help)
                show_help
                exit 0
                ;;
            *)
                log_error "未知选项: $1"
                show_help
                exit 1
                ;;
        esac
    done
}

# 创建目录
setup_directories() {
    log_info "创建覆盖率测试目录..."
    mkdir -p "${COVERAGE_DIR}"
    mkdir -p "${REPORT_DIR}"
}

# 构建覆盖率测试
build_coverage_tests() {
    log_info "构建覆盖率测试..."

    # 构建单元测试覆盖率版本
    if [[ "$UNIT_TESTS" == "true" ]]; then
        log_info "构建单元测试覆盖率版本..."
        bazel build //tests/unit:unit_tests_coverage
    fi

    # 构建集成测试覆盖率版本
    if [[ "$INTEGRATION_TESTS" == "true" ]]; then
        log_info "构建集成测试覆盖率版本..."
        bazel build //tests/integration:integration_test_coverage
    fi

    # 构建性能测试覆盖率版本
    if [[ "$PERFORMANCE_TESTS" == "true" ]]; then
        log_info "构建性能测试覆盖率版本..."
        bazel build //tests/performance:crud_performance_test_with_coverage
    fi

    log_success "覆盖率测试构建完成"
}

# 运行覆盖率测试
run_coverage_tests() {
    log_info "运行覆盖率测试..."

    # 设置覆盖率环境变量
    export LLVM_PROFILE_FILE="${COVERAGE_DIR}/coverage-%p.profraw"

    # 运行单元测试覆盖率
    if [[ "$UNIT_TESTS" == "true" ]]; then
        log_info "运行单元测试覆盖率..."
        bazel test \
            --test_output=errors \
            --test_tag_filters=coverage \
            //tests/unit:unit_tests_coverage
        log_success "单元测试覆盖率完成"
    fi

    # 运行集成测试覆盖率
    if [[ "$INTEGRATION_TESTS" == "true" ]]; then
        log_info "运行集成测试覆盖率..."
        bazel test \
            --test_output=errors \
            --test_tag_filters=coverage \
            //tests/integration:integration_test_coverage
        log_success "集成测试覆盖率完成"
    fi

    # 运行性能测试覆盖率
    if [[ "$PERFORMANCE_TESTS" == "true" ]]; then
        log_info "运行性能测试覆盖率..."
        bazel test \
            --test_output=errors \
            --test_tag_filters=coverage \
            //tests/performance:crud_performance_test_with_coverage
        log_success "性能测试覆盖率完成"
    fi
}

# 生成覆盖率报告
generate_coverage_report() {
    log_info "生成覆盖率报告..."

    # 合并覆盖率数据
    log_info "合并覆盖率数据文件..."
    llvm-profdata merge \
        "${COVERAGE_DIR}"/coverage-*.profraw \
        -o "${COVERAGE_DIR}/merged.profdata"

    # 收集所有可执行文件路径
    BINARY_FILES=()

    if [[ "$UNIT_TESTS" == "true" ]]; then
        BINARY_FILES+=(
            "bazel-bin/tests/unit/b_plus_tree_test_coverage"
            "bazel-bin/tests/unit/buffer_pool_test_coverage"
            "bazel-bin/tests/unit/disk_manager_test_coverage"
            "bazel-bin/tests/unit/index_maintenance_test_coverage"
            "bazel-bin/tests/unit/basic_bplus_tree_test_coverage"
            "bazel-bin/tests/unit/logger_test_coverage"
            "bazel-bin/tests/unit/simple_create_test_coverage"
        )
    fi

    if [[ "$INTEGRATION_TESTS" == "true" ]]; then
        BINARY_FILES+=(
            "bazel-bin/tests/integration/ddl_commands_test_coverage"
            "bazel-bin/tests/integration/dml_commands_test_coverage"
            "bazel-bin/tests/integration/error_handling_test_coverage"
            "bazel-bin/tests/integration/procedure_split_test_coverage"
            "bazel-bin/tests/integration/trigger_split_test_coverage"
        )
    fi

    if [[ "$PERFORMANCE_TESTS" == "true" ]]; then
        BINARY_FILES+=(
            "bazel-bin/tests/performance/crud_performance_test_with_coverage"
        )
    fi

    # 生成LCOV格式报告
    log_info "生成LCOV覆盖率报告..."
    llvm-cov export \
        --format=lcov \
        --instr-profile="${COVERAGE_DIR}/merged.profdata" \
        "${BINARY_FILES[@]}" \
        > "${COVERAGE_DIR}/coverage.lcov"

    # 生成HTML报告
    log_info "生成HTML覆盖率报告..."
    genhtml "${COVERAGE_DIR}/coverage.lcov" \
        --output-directory "${REPORT_DIR}" \
        --title "SQLCC Coverage Report" \
        --legend \
        --show-details

    # 生成摘要报告
    generate_summary_report

    log_success "覆盖率报告生成完成"
}

# 生成摘要报告
generate_summary_report() {
    log_info "生成覆盖率摘要报告..."

    # 解析LCOV数据获取覆盖率统计
    local total_lines=0
    local covered_lines=0

    while IFS= read -r line; do
        if [[ $line =~ ^DA: ]]; then
            # DA:行号,执行次数
            local execution_count=$(echo "$line" | cut -d',' -f2)
            ((total_lines++))
            if [[ "$execution_count" -gt 0 ]]; then
                ((covered_lines++))
            fi
        fi
    done < "${COVERAGE_DIR}/coverage.lcov"

    local coverage_percentage=0
    if [[ $total_lines -gt 0 ]]; then
        coverage_percentage=$((covered_lines * 100 / total_lines))
    fi

    # 生成摘要报告
    cat > "${REPORT_DIR}/coverage_summary.txt" << EOF
SQLCC 代码覆盖率报告
生成时间: $(date)
========================================

测试配置:
- 单元测试覆盖率: $([[ "$UNIT_TESTS" == "true" ]] && echo "启用" || echo "禁用")
- 集成测试覆盖率: $([[ "$INTEGRATION_TESTS" == "true" ]] && echo "启用" || echo "禁用")
- 性能测试覆盖率: $([[ "$PERFORMANCE_TESTS" == "true" ]] && echo "启用" || echo "禁用")

覆盖率统计:
- 总代码行数: ${total_lines}
- 已覆盖行数: ${covered_lines}
- 覆盖率百分比: ${coverage_percentage}%

覆盖率等级:
EOF

    if [[ $coverage_percentage -ge 80 ]]; then
        echo "- 优秀 (≥80%): ✅ ${coverage_percentage}%" >> "${REPORT_DIR}/coverage_summary.txt"
    elif [[ $coverage_percentage -ge 70 ]]; then
        echo "- 良好 (70-79%): ⚠️ ${coverage_percentage}%" >> "${REPORT_DIR}/coverage_summary.txt"
    elif [[ $coverage_percentage -ge 60 ]]; then
        echo "- 可接受 (60-69%): ⚠️ ${coverage_percentage}%" >> "${REPORT_DIR}/coverage_summary.txt"
    else
        echo "- 需要改进 (<60%): ❌ ${coverage_percentage}%" >> "${REPORT_DIR}/coverage_summary.txt"
    fi

    cat >> "${REPORT_DIR}/coverage_summary.txt" << EOF

详细报告:
- HTML报告: ${REPORT_DIR}/index.html
- LCOV数据: ${COVERAGE_DIR}/coverage.lcov

覆盖率数据文件:
EOF

    ls -la "${COVERAGE_DIR}"/coverage-*.profraw >> "${REPORT_DIR}/coverage_summary.txt" 2>/dev/null || true

    log_success "覆盖率摘要报告生成完成: ${REPORT_DIR}/coverage_summary.txt"
}

# 显示结果摘要
show_results() {
    log_info "覆盖率测试结果摘要"

    if [[ -f "${REPORT_DIR}/coverage_summary.txt" ]]; then
        echo ""
        echo "========================================="
        cat "${REPORT_DIR}/coverage_summary.txt"
        echo "========================================="
    fi

    echo ""
    echo "报告文件位置:"
    echo "- 摘要报告: ${REPORT_DIR}/coverage_summary.txt"
    echo "- HTML报告: ${REPORT_DIR}/index.html"
    echo "- 覆盖率数据: ${COVERAGE_DIR}/coverage.lcov"
    echo ""
    echo "查看HTML报告: open ${REPORT_DIR}/index.html"
}

# 主函数
main() {
    parse_args "$@"
    setup_directories

    log_info "开始SQLCC覆盖率测试"
    log_info "输出目录: ${REPORT_DIR}"

    build_coverage_tests
    run_coverage_tests
    generate_coverage_report
    show_results

    log_success "SQLCC覆盖率测试完成"
}

# 执行主函数
main "$@"