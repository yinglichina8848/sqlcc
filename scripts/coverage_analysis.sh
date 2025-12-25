#!/bin/bash
# SQLCC 覆盖率分析脚本
# 用途: 自动化收集、分析和跟踪覆盖率变化
# 日期: 2025-12-25

set -e

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 目录配置
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
COVERAGE_DIR="${PROJECT_ROOT}/coverage_report"
HISTORY_FILE="${COVERAGE_DIR}/coverage_history.json"
TIMESTAMP=$(date '+%Y%m%d_%H%M%S')

# 默认测试目标
DEFAULT_TESTS=(
    "//tests/unit:logger_test"
    # 添加更多测试目标...
)

# 核心组件库
CORE_LIBS=(
    "//src/logger:logger"
    # "//src/core:core"
    # "//src/storage_engine:storage_engine"
    # "//src/sql_parser:sql_parser"
)

usage() {
    echo "Usage: $0 [command] [options]"
    echo ""
    echo "Commands:"
    echo "  collect     收集覆盖率数据"
    echo "  report      生成覆盖率报告"
    echo "  compare     比较历史覆盖率变化"
    echo "  full        完整流程: 收集 + 报告 + 比较"
    echo "  history     查看覆盖率历史"
    echo ""
    echo "Options:"
    echo "  -t, --target TARGET  指定测试目标 (可多次使用)"
    echo "  -l, --lib LIB        指定要分析的库 (可多次使用)"
    echo "  -o, --output DIR     指定输出目录"
    echo "  -n, --name NAME      指定此次分析的标签名"
    echo "  -h, --help           显示帮助信息"
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

# 初始化目录
init_dirs() {
    mkdir -p "${COVERAGE_DIR}"
    mkdir -p "${COVERAGE_DIR}/history"
    mkdir -p "${COVERAGE_DIR}/html"
    
    if [[ ! -f "${HISTORY_FILE}" ]]; then
        echo '{"records": []}' > "${HISTORY_FILE}"
    fi
}

# 收集覆盖率数据
collect_coverage() {
    local targets=("${@:-${DEFAULT_TESTS[@]}}")
    
    log_info "清理旧的覆盖率数据..."
    bazel clean 2>/dev/null || true
    
    log_info "运行测试并收集覆盖率..."
    for target in "${targets[@]}"; do
        log_info "  执行: ${target}"
        bazel coverage "${target}" --combined_report=lcov 2>&1 | tail -10
    done
    
    log_success "覆盖率数据收集完成"
}

# 合并profraw文件
merge_profdata() {
    local test_target="$1"
    local test_name=$(echo "${test_target}" | sed 's/.*://')
    local testlog_dir="${PROJECT_ROOT}/bazel-out/k8-fastbuild/testlogs/_coverage/tests/unit/${test_name}/test"
    
    if [[ -d "${testlog_dir}" ]]; then
        cd "${testlog_dir}"
        if ls *.profraw 1> /dev/null 2>&1; then
            llvm-profdata-18 merge -sparse *.profraw -o coverage.profdata 2>/dev/null
            echo "${testlog_dir}/coverage.profdata"
        fi
    fi
}

# 分析单个库的覆盖率
analyze_lib_coverage() {
    local lib_target="$1"
    local profdata_file="$2"
    local lib_name=$(echo "${lib_target}" | sed 's/.*://')
    local lib_so="${PROJECT_ROOT}/bazel-bin/src/${lib_name}/lib${lib_name}.so"
    
    if [[ ! -f "${lib_so}" ]]; then
        log_warn "库文件不存在: ${lib_so}"
        return 1
    fi
    
    if [[ ! -f "${profdata_file}" ]]; then
        log_warn "profdata文件不存在: ${profdata_file}"
        return 1
    fi
    
    # 获取覆盖率报告
    local report=$(llvm-cov-18 report "${lib_so}" -instr-profile="${profdata_file}" 2>/dev/null)
    
    # 解析覆盖率数据
    local line_cov=$(echo "${report}" | grep "TOTAL" | awk '{print $10}' | sed 's/%//')
    local func_cov=$(echo "${report}" | grep "TOTAL" | awk '{print $6}' | sed 's/%//')
    local region_cov=$(echo "${report}" | grep "TOTAL" | awk '{print $4}' | sed 's/%//')
    local branch_cov=$(echo "${report}" | grep "TOTAL" | awk '{print $12}' | sed 's/%//')
    
    echo "{\"lib\":\"${lib_name}\",\"line\":${line_cov:-0},\"function\":${func_cov:-0},\"region\":${region_cov:-0},\"branch\":${branch_cov:-0}}"
}

# 生成覆盖率报告
generate_report() {
    local tag_name="${1:-${TIMESTAMP}}"
    local output_dir="${COVERAGE_DIR}/html/${tag_name}"
    
    mkdir -p "${output_dir}"
    
    log_info "生成覆盖率报告..."
    
    # 找到profdata文件
    local profdata_file=$(merge_profdata "//tests/unit:logger_test")
    
    if [[ -z "${profdata_file}" ]]; then
        log_error "无法找到或生成profdata文件"
        return 1
    fi
    
    local results=()
    
    for lib_target in "${CORE_LIBS[@]}"; do
        local lib_name=$(echo "${lib_target}" | sed 's/.*://')
        local lib_so="${PROJECT_ROOT}/bazel-bin/src/${lib_name}/lib${lib_name}.so"
        
        if [[ -f "${lib_so}" ]]; then
            log_info "  分析: ${lib_name}"
            
            # 生成HTML报告
            llvm-cov-18 show "${lib_so}" \
                -instr-profile="${profdata_file}" \
                -path-equivalence=/proc/self/cwd,"${PROJECT_ROOT}" \
                -format=html \
                -output-dir="${output_dir}/${lib_name}" 2>/dev/null || true
            
            # 获取覆盖率数据
            local cov_data=$(analyze_lib_coverage "${lib_target}" "${profdata_file}")
            if [[ -n "${cov_data}" ]]; then
                results+=("${cov_data}")
            fi
        fi
    done
    
    # 生成汇总报告
    generate_summary_report "${tag_name}" "${output_dir}" "${results[@]}"
    
    log_success "报告已生成: ${output_dir}"
}

# 解析JSON字段 (简单实现，不依赖jq)
parse_json_field() {
    local json="$1"
    local field="$2"
    echo "${json}" | grep -oP "\"${field}\":\s*[\"']?[^,}\"']*" | sed "s/\"${field}\":\s*[\"']*//" | sed "s/[\"']$//"
}

# 生成汇总报告
generate_summary_report() {
    local tag_name="$1"
    local output_dir="$2"
    shift 2
    local results=("$@")
    
    local summary_file="${output_dir}/summary.md"
    
    cat > "${summary_file}" << EOF
# SQLCC 覆盖率报告

**生成时间**: $(date '+%Y-%m-%d %H:%M:%S')  
**标签**: ${tag_name}

## 覆盖率汇总

| 组件 | 行覆盖率 | 函数覆盖率 | 区域覆盖率 | 分支覆盖率 |
|------|----------|------------|------------|------------|
EOF

    for result in "${results[@]}"; do
        local lib=$(parse_json_field "${result}" "lib")
        local line=$(parse_json_field "${result}" "line")
        local func=$(parse_json_field "${result}" "function")
        local region=$(parse_json_field "${result}" "region")
        local branch=$(parse_json_field "${result}" "branch")
        
        echo "| ${lib} | ${line}% | ${func}% | ${region}% | ${branch}% |" >> "${summary_file}"
    done
    
    cat >> "${summary_file}" << EOF

## 详细报告

EOF

    for lib_target in "${CORE_LIBS[@]}"; do
        local lib_name=$(echo "${lib_target}" | sed 's/.*://')
        if [[ -d "${output_dir}/${lib_name}" ]]; then
            echo "- [${lib_name}](./${lib_name}/index.html)" >> "${summary_file}"
        fi
    done
    
    # 保存历史记录
    save_history "${tag_name}" "${results[@]}"
}

# 保存历史记录
save_history() {
    local tag_name="$1"
    shift
    local results=("$@")
    
    local timestamp=$(date '+%Y-%m-%dT%H:%M:%S')
    local history_entry="${COVERAGE_DIR}/history/${tag_name}.txt"
    
    echo "timestamp=${timestamp}" > "${history_entry}"
    echo "tag=${tag_name}" >> "${history_entry}"
    for result in "${results[@]}"; do
        echo "result=${result}" >> "${history_entry}"
    done
    
    log_info "历史记录已保存: ${history_entry}"
}

# 比较覆盖率变化
compare_coverage() {
    local num_records="${1:-5}"
    
    log_info "覆盖率变化趋势 (最近${num_records}次):"
    echo ""
    
    # 简单列出最近的历史文件
    ls -t "${COVERAGE_DIR}/history/"*.txt 2>/dev/null | head -${num_records} | while read f; do
        local tag=$(grep "^tag=" "$f" | cut -d= -f2)
        local ts=$(grep "^timestamp=" "$f" | cut -d= -f2)
        local result=$(grep "^result=" "$f" | head -1 | cut -d= -f2-)
        local line=$(parse_json_field "${result}" "line")
        echo "[${tag}] ${ts} - Line: ${line}%"
    done
    echo ""
}

# 查看历史
show_history() {
    log_info "覆盖率历史记录:"
    echo ""
    
    for f in "${COVERAGE_DIR}/history/"*.txt; do
        if [[ -f "$f" ]]; then
            local tag=$(grep "^tag=" "$f" | cut -d= -f2)
            local ts=$(grep "^timestamp=" "$f" | cut -d= -f2)
            echo "[${tag}] ${ts}"
            grep "^result=" "$f" | while read line; do
                local result=$(echo "$line" | cut -d= -f2-)
                local lib=$(parse_json_field "${result}" "lib")
                local lcov=$(parse_json_field "${result}" "line")
                local fcov=$(parse_json_field "${result}" "function")
                echo "  ${lib}: Line=${lcov}% Func=${fcov}%"
            done
            echo ""
        fi
    done
}

# 完整流程
full_analysis() {
    local tag_name="${1:-build_$(git rev-parse --short HEAD 2>/dev/null || echo ${TIMESTAMP})}"
    
    log_info "开始完整覆盖率分析: ${tag_name}"
    echo ""
    
    collect_coverage
    generate_report "${tag_name}"
    compare_coverage
    
    echo ""
    log_success "分析完成！"
    log_info "报告位置: ${COVERAGE_DIR}/html/${tag_name}/summary.md"
}

# 主函数
main() {
    init_dirs
    
    local command="${1:-full}"
    shift || true
    
    case "${command}" in
        collect)
            collect_coverage "$@"
            ;;
        report)
            generate_report "$@"
            ;;
        compare)
            compare_coverage "$@"
            ;;
        full)
            full_analysis "$@"
            ;;
        history)
            show_history
            ;;
        -h|--help|help)
            usage
            ;;
        *)
            log_error "未知命令: ${command}"
            usage
            exit 1
            ;;
    esac
}

# 切换到项目根目录
cd "${PROJECT_ROOT}"

main "$@"
