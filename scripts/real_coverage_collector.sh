#!/bin/bash
#
# SQLCC 真实覆盖率收集脚本
# 基于 Bazel coverage 和 LLVM llvm-cov 工具
#

set -e

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
OUTPUT_DIR="${PROJECT_ROOT}/coverage_results_real"
TIMESTAMP=$(date +%Y%m%d_%H%M%S)
REPORT_DIR="${OUTPUT_DIR}/${TIMESTAMP}"

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

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

# 检查依赖
check_dependencies() {
    log_info "检查依赖工具..."
    
    local deps_missing=0
    
    if ! command -v bazel &> /dev/null; then
        log_error "Bazel 未安装"
        deps_missing=1
    fi
    
    if ! command -v llvm-cov-20 &> /dev/null && ! command -v llvm-cov &> /dev/null; then
        log_warn "llvm-cov 未找到，将尝试使用默认路径"
    fi
    
    if ! command -v lcov &> /dev/null; then
        log_warn "lcov 未安装，HTML报告生成可能受限"
    fi
    
    if [ $deps_missing -eq 1 ]; then
        exit 1
    fi
    
    log_success "依赖检查完成"
}

# 创建输出目录
setup_output_dir() {
    log_info "创建输出目录: ${REPORT_DIR}"
    mkdir -p "${REPORT_DIR}"
    mkdir -p "${REPORT_DIR}/raw_data"
    mkdir -p "${REPORT_DIR}/reports"
    log_success "目录创建完成"
}

# 运行测试并收集覆盖率数据
run_tests_with_coverage() {
    log_info "开始运行测试并收集覆盖率数据..."
    
    cd "${PROJECT_ROOT}"
    
    # 设置覆盖率输出路径
    export LLVM_PROFILE_FILE="${REPORT_DIR}/raw_data/coverage_%p.profraw"
    
    # 运行所有测试
    log_info "执行: bazel test //tests/... --config=coverage"
    
    if bazel test //tests/... \
        --config=coverage \
        --test_output=errors \
        --experimental_fetch_all_coverage_outputs \
        2>&1 | tee "${REPORT_DIR}/bazel_test.log"; then
        log_success "测试执行完成"
    else
        log_warn "部分测试失败，继续收集覆盖率数据..."
    fi
    
    # 查找并合并覆盖率数据
    log_info "收集原始覆盖率数据..."
    
    # 查找 Bazel 生成的覆盖率数据
    local bazel_output=$(bazel info output_path 2>/dev/null || echo "bazel-out")
    find "${PROJECT_ROOT}/${bazel_output}" -name "*.profraw" -o -name "_coverage_report.dat" 2>/dev/null | head -5 > "${REPORT_DIR}/raw_data/coverage_files.list"
    
    log_success "覆盖率数据收集完成"
}

# 生成覆盖率报告
generate_coverage_report() {
    log_info "生成覆盖率报告..."
    
    local bazel_bin=$(bazel info bazel-bin 2>/dev/null || echo "bazel-bin")
    
    # 尝试从 Bazel 输出中提取覆盖率报告
    local coverage_report=$(find "${PROJECT_ROOT}/bazel-out" -name "_coverage_report.dat" 2>/dev/null | head -1)
    
    if [ -n "$coverage_report" ] && [ -f "$coverage_report" ]; then
        log_info "找到 Bazel 覆盖率报告: $coverage_report"
        cp "$coverage_report" "${REPORT_DIR}/reports/coverage_report.dat"
        
        # 生成 LCOV 报告
        if command -v genhtml &> /dev/null; then
            log_info "生成 HTML 报告..."
            genhtml --ignore-errors inconsistent,corrupt \
                "$coverage_report" \
                -o "${REPORT_DIR}/reports/html" \
                --title "SQLCC Coverage Report ${TIMESTAMP}" \
                2>&1 | tee "${REPORT_DIR}/genhtml.log" || true
        fi
        
        # 提取覆盖率统计
        log_info "提取覆盖率统计..."
        if command -v lcov &> /dev/null; then
            lcov --summary "$coverage_report" 2>&1 | tee "${REPORT_DIR}/reports/coverage_summary.txt"
        fi
    else
        log_warn "未找到 Bazel 覆盖率报告文件"
        echo "未找到覆盖率报告" > "${REPORT_DIR}/reports/coverage_summary.txt"
    fi
    
    log_success "报告生成完成"
}

# 生成简单的覆盖率统计（基于测试执行结果）
generate_simple_report() {
    log_info "生成简化版覆盖率报告..."
    
    local report_file="${REPORT_DIR}/reports/simple_coverage_report.md"
    
    cat > "$report_file" << EOF
# SQLCC 覆盖率报告

**生成时间**: $(date '+%Y-%m-%d %H:%M:%S')  
**报告类型**: 真实测试覆盖率  
**收集工具**: Bazel Coverage + LLVM

## 测试执行结果

### 执行日志
\`\`\`
$(tail -50 "${REPORT_DIR}/bazel_test.log" 2>/dev/null || echo "日志不可用")
\`\`\`

## 覆盖率数据位置

- 原始数据: \`${REPORT_DIR}/raw_data/\`
- 详细报告: \`${REPORT_DIR}/reports/\`

## 注意事项

1. 本报告基于实际测试执行结果生成
2. 覆盖率数据仅包含实际执行的代码路径
3. 部分模块可能因测试未执行而无覆盖率数据

## 改进建议

1. 补充缺失的单元测试
2. 增加集成测试覆盖率
3. 修复失败的测试用例

---
*报告生成: ${TIMESTAMP}*
EOF

    log_success "简化报告生成完成: $report_file"
}

# 创建最新报告链接
create_latest_link() {
    log_info "创建最新报告链接..."
    
    local latest_link="${OUTPUT_DIR}/latest"
    
    rm -f "$latest_link"
    ln -s "$REPORT_DIR" "$latest_link"
    
    log_success "链接创建完成: $latest_link -> $REPORT_DIR"
}

# 主函数
main() {
    echo "========================================="
    echo "  SQLCC 真实覆盖率收集脚本"
    echo "  版本: 1.0.0"
    echo "========================================="
    echo ""
    
    check_dependencies
    setup_output_dir
    run_tests_with_coverage
    generate_coverage_report
    generate_simple_report
    create_latest_link
    
    echo ""
    echo "========================================="
    log_success "覆盖率收集完成!"
    echo "========================================="
    echo ""
    echo "报告位置:"
    echo "  - 完整报告: ${REPORT_DIR}/reports/"
    echo "  - 最新链接: ${OUTPUT_DIR}/latest"
    echo "  - 原始数据: ${REPORT_DIR}/raw_data/"
    echo ""
}

# 执行主函数
main "$@"
