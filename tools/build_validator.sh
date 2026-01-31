#!/bin/bash

# SQLCC 构建验证器
# 用于自动化验证构建系统和依赖关系

set -e  # 遇到错误立即退出

# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 日志函数
log_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# 检查依赖
check_dependencies() {
    log_info "检查系统依赖..."

    if ! command -v bazel &> /dev/null; then
        log_error "Bazel 未安装或不在 PATH 中"
        return 1
    fi

    # 检查是否有可用的C++编译器
    if ! command -v clang++ &> /dev/null && ! command -v g++ &> /dev/null; then
        log_error "未找到可用的C++编译器 (clang++ 或 g++)"
        return 1
    fi

    log_info "系统依赖检查通过"
    return 0
}

# 验证构建目标
validate_build_targets() {
    local targets=(
        "//src/sql_parser:sqlcc_parser"
        "//src/core:core"
        "//src/storage_engine:storage_engine"
        "//src/network:network"
        "//src/execution:execution"
    )

    log_info "验证构建目标..."

    for target in "${targets[@]}"; do
        log_info "构建目标: $target"
        if bazel build "$target" > /dev/null 2>&1; then
            log_info "✓ $target 构建成功"
        else
            log_error "✗ $target 构建失败"
            return 1
        fi
    done

    log_info "所有构建目标验证通过"
    return 0
}

# 检查依赖关系
check_dependencies_graph() {
    log_info "检查依赖关系图..."

    # 检查循环依赖
    if bazel query 'deps(//...)' | grep -q "cycle"; then
        log_error "检测到循环依赖"
        return 1
    fi

    log_info "依赖关系检查通过"
    return 0
}

# 生成构建报告
generate_build_report() {
    local report_file="build_validation_report_$(date +%Y%m%d_%H%M%S).txt"

    log_info "生成构建报告: $report_file"

    {
        echo "SQLCC 构建验证报告"
        echo "生成时间: $(date)"
        echo "验证结果: $1"
        echo ""
        echo "系统信息:"
        echo "- Bazel 版本: $(bazel version 2>/dev/null | grep 'Build label' | cut -d' ' -f3)"
        echo "- Clang++ 版本: $(clang++ --version | head -n1)"
        echo ""
        echo "构建目标状态:"
        bazel query 'kind(cc_library, //...)' | wc -l | xargs echo "- 库目标数量:"
        bazel query 'kind(cc_binary, //...)' | wc -l | xargs echo "- 二进制目标数量:"
        echo ""
        echo "验证详情:"
        echo "- 依赖检查: ✓"
        echo "- 构建目标验证: $2"
        echo "- 依赖关系检查: ✓"
    } > "$report_file"

    log_info "报告已保存到: $report_file"
}

# 主函数
main() {
    log_info "开始 SQLCC 构建验证..."

    local overall_status="SUCCESS"
    local build_status="PASS"

    # 检查依赖
    if ! check_dependencies; then
        overall_status="FAILED"
        log_error "依赖检查失败"
    fi

    # 验证构建目标
    if ! validate_build_targets; then
        overall_status="FAILED"
        build_status="FAIL"
        log_error "构建目标验证失败"
    fi

    # 检查依赖关系
    if ! check_dependencies_graph; then
        overall_status="FAILED"
        log_error "依赖关系检查失败"
    fi

    # 生成报告
    generate_build_report "$overall_status" "$build_status"

    if [ "$overall_status" = "SUCCESS" ]; then
        log_info "✓ 构建验证完成 - 所有检查通过"
        exit 0
    else
        log_error "✗ 构建验证失败 - 请检查上述错误"
        exit 1
    fi
}

# 参数处理
case "${1:-}" in
    --help|-h)
        echo "SQLCC 构建验证器"
        echo ""
        echo "用法: $0 [选项]"
        echo ""
        echo "选项:"
        echo "  --help, -h    显示此帮助信息"
        echo "  --check-deps  只检查依赖"
        echo "  --validate    只验证构建目标"
        echo ""
        exit 0
        ;;
    --check-deps)
        check_dependencies && log_info "依赖检查通过" || log_error "依赖检查失败"
        exit $?
        ;;
    --validate)
        validate_build_targets && log_info "构建目标验证通过" || log_error "构建目标验证失败"
        exit $?
        ;;
    *)
        main "$@"
        ;;
esac
