#!/bin/bash
# SQLCC CI/CD 覆盖率集成脚本
# 用于在CI/CD流水线中自动运行覆盖率测试和报告生成
# 日期: 2026-01-18

set -e

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 环境变量
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
COVERAGE_SCRIPT="${PROJECT_ROOT}/scripts/coverage_analysis.sh"
MIN_LINE_COVERAGE="${MIN_LINE_COVERAGE:-80}"
MIN_FUNCTION_COVERAGE="${MIN_FUNCTION_COVERAGE:-75}"

# CI环境检测
IS_CI="${CI:-false}"
IS_PR="${PULL_REQUEST:-false}"
BRANCH_NAME="${BRANCH_NAME:-$(git rev-parse --abbrev-ref HEAD)}"
COMMIT_SHA="${COMMIT_SHA:-$(git rev-parse HEAD)}"

usage() {
    echo "Usage: $0 [options]"
    echo ""
    echo "CI/CD 覆盖率集成脚本"
    echo ""
    echo "Options:"
    echo "  --min-line-cov PERCENT      最小行覆盖率要求 (默认: 80)"
    echo "  --min-func-cov PERCENT      最小函数覆盖率要求 (默认: 75)"
    echo "  --fail-on-threshold         覆盖率不达标时失败"
    echo "  --upload-report             上传覆盖率报告"
    echo "  --compare-baseline          比较基准分支覆盖率"
    echo "  --baseline-branch BRANCH    基准分支 (默认: main)"
    echo "  --help                      显示帮助信息"
}

log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# 检查环境
check_environment() {
    log_info "检查环境..."

    # 检查必要的工具
    local tools=("bazel" "llvm-cov" "llvm-profdata")
    for tool in "${tools[@]}"; do
        if ! command -v "${tool}" &> /dev/null; then
            log_error "缺少必要工具: ${tool}"
            exit 1
        fi
    done

    # 检查覆盖率脚本
    if [[ ! -x "${COVERAGE_SCRIPT}" ]]; then
        log_error "覆盖率脚本不存在或不可执行: ${COVERAGE_SCRIPT}"
        exit 1
    fi

    log_success "环境检查通过"
}

# 运行覆盖率测试
run_coverage_tests() {
    local tag_name="$1"

    log_info "运行覆盖率测试..."

    # 设置Bazel覆盖率配置
    export BAZEL_CXXOPTS="-fprofile-instr-generate -fcoverage-mapping"

    # 运行覆盖率分析
    if "${COVERAGE_SCRIPT}" full "${tag_name}"; then
        log_success "覆盖率测试完成"
        return 0
    else
        log_error "覆盖率测试失败"
        return 1
    fi
}

# 验证覆盖率阈值
validate_coverage_thresholds() {
    local report_file="$1"
    local fail_on_threshold="${2:-false}"

    log_info "验证覆盖率阈值..."

    if [[ ! -f "${report_file}" ]]; then
        log_error "覆盖率报告文件不存在: ${report_file}"
        return 1
    fi

    # 解析覆盖率数据 (简化实现)
    local line_cov=$(grep -A 10 "覆盖率汇总" "${report_file}" | grep "|" | head -2 | tail -1 | awk -F'|' '{print $3}' | sed 's/%//;s/ //g')
    local func_cov=$(grep -A 10 "覆盖率汇总" "${report_file}" | grep "|" | head -2 | tail -1 | awk -F'|' '{print $4}' | sed 's/%//;s/ //g')

    log_info "当前覆盖率 - 行: ${line_cov}%, 函数: ${func_cov}%"
    log_info "阈值要求 - 行: ${MIN_LINE_COVERAGE}%, 函数: ${MIN_FUNCTION_COVERAGE}%"

    local failed=false

    if [[ -n "${line_cov}" ]] && [[ "${line_cov}" -lt "${MIN_LINE_COVERAGE}" ]]; then
        log_error "行覆盖率不达标: ${line_cov}% < ${MIN_LINE_COVERAGE}%"
        failed=true
    fi

    if [[ -n "${func_cov}" ]] && [[ "${func_cov}" -lt "${MIN_FUNCTION_COVERAGE}" ]]; then
        log_error "函数覆盖率不达标: ${func_cov}% < ${MIN_FUNCTION_COVERAGE}%"
        failed=true
    fi

    if [[ "${failed}" == "true" ]] && [[ "${fail_on_threshold}" == "true" ]]; then
        log_error "覆盖率不达标，构建失败"
        return 1
    elif [[ "${failed}" == "true" ]]; then
        log_warn "覆盖率不达标，但继续构建"
        return 0
    else
        log_success "覆盖率检查通过"
        return 0
    fi
}

# 比较基准分支覆盖率
compare_baseline_coverage() {
    local baseline_branch="${1:-main}"

    log_info "比较基准分支覆盖率: ${baseline_branch}"

    # 保存当前分支
    local current_branch="${BRANCH_NAME}"

    # 获取基准分支的最新覆盖率
    git fetch origin "${baseline_branch}" 2>/dev/null || true

    # 这里可以实现更复杂的比较逻辑
    # 暂时只记录信息
    log_info "基准分支比较功能待实现"
}

# 上传覆盖率报告
upload_coverage_report() {
    local report_dir="$1"

    log_info "上传覆盖率报告..."

    if [[ "${IS_CI}" != "true" ]]; then
        log_warn "非CI环境，跳过上传"
        return 0
    fi

    # 这里可以实现上传到代码质量平台的逻辑
    # 例如: Codecov, SonarQube, etc.
    log_info "报告上传功能待实现"
}

# 生成PR注释
generate_pr_comment() {
    local report_file="$1"

    if [[ "${IS_PR}" != "true" ]]; then
        return 0
    fi

    log_info "生成PR覆盖率注释..."

    # 读取覆盖率数据
    local coverage_info=""
    if [[ -f "${report_file}" ]]; then
        coverage_info=$(head -20 "${report_file}")
    fi

    # 生成Markdown注释
    cat << EOF
## 📊 覆盖率报告

**分支**: ${BRANCH_NAME}
**提交**: ${COMMIT_SHA:0:8}

${coverage_info}

### 覆盖率阈值
- 行覆盖率: ${MIN_LINE_COVERAGE}% (最低要求)
- 函数覆盖率: ${MIN_FUNCTION_COVERAGE}% (最低要求)

EOF
}

# 主函数
main() {
    local fail_on_threshold=false
    local upload_report=false
    local compare_baseline=false
    local baseline_branch="main"

    # 解析命令行参数
    while [[ $# -gt 0 ]]; do
        case $1 in
            --min-line-cov)
                MIN_LINE_COVERAGE="$2"
                shift 2
                ;;
            --min-func-cov)
                MIN_FUNCTION_COVERAGE="$2"
                shift 2
                ;;
            --fail-on-threshold)
                fail_on_threshold=true
                shift
                ;;
            --upload-report)
                upload_report=true
                shift
                ;;
            --compare-baseline)
                compare_baseline=true
                shift
                ;;
            --baseline-branch)
                baseline_branch="$2"
                shift 2
                ;;
            --help)
                usage
                exit 0
                ;;
            *)
                log_error "未知参数: $1"
                usage
                exit 1
                ;;
        esac
    done

    log_info "开始CI/CD覆盖率集成..."
    log_info "分支: ${BRANCH_NAME}"
    log_info "提交: ${COMMIT_SHA:0:8}"
    log_info "CI环境: ${IS_CI}"
    log_info "PR构建: ${IS_PR}"

    # 检查环境
    check_environment

    # 生成标签名
    local tag_name="ci_${BRANCH_NAME}_${COMMIT_SHA:0:8}_$(date +%Y%m%d_%H%M%S)"

    # 运行覆盖率测试
    if ! run_coverage_tests "${tag_name}"; then
        log_error "覆盖率测试失败"
        exit 1
    fi

    # 确定报告文件位置
    local report_file="${PROJECT_ROOT}/coverage_report/html/${tag_name}/summary.md"

    # 验证覆盖率阈值
    if ! validate_coverage_thresholds "${report_file}" "${fail_on_threshold}"; then
        exit 1
    fi

    # 比较基准分支
    if [[ "${compare_baseline}" == "true" ]]; then
        compare_baseline_coverage "${baseline_branch}"
    fi

    # 上传报告
    if [[ "${upload_report}" == "true" ]]; then
        upload_coverage_report "${PROJECT_ROOT}/coverage_report/html/${tag_name}"
    fi

    # 生成PR注释
    generate_pr_comment "${report_file}"

    log_success "CI/CD覆盖率集成完成"
    log_info "报告位置: ${report_file}"
}

# 切换到项目根目录
cd "${PROJECT_ROOT}"

main "$@"
