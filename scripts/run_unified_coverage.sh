#!/bin/bash
#
# SQLCC 统一覆盖率测试脚本
# 使用Bazel内置Coverage功能运行Level1-Level2测试并生成覆盖率报告
#

set -e

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "${PROJECT_ROOT}"

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

echo "========================================="
echo "  SQLCC 统一覆盖率测试脚本"
echo "========================================="
echo "开始时间: $(date)"
echo ""

# 配置输出目录
OUTPUT_DIR="${PROJECT_ROOT}/tests/test_output"
TIMESTAMP=$(date +%Y%m%d_%H%M%S)
COVERAGE_DIR="${OUTPUT_DIR}/coverage"
REPORT_DIR="${COVERAGE_DIR}/reports/${TIMESTAMP}"

# 创建输出目录
log_info "创建输出目录: ${REPORT_DIR}"
mkdir -p "${REPORT_DIR}/html"
mkdir -p "${REPORT_DIR}/text"
mkdir -p "${REPORT_DIR}/json"
mkdir -p "${COVERAGE_DIR}/raw"
mkdir -p "${COVERAGE_DIR}/reports/latest"
mkdir -p "${OUTPUT_DIR}/logs/coverage"

# 清理旧的覆盖率数据
log_info "清理旧的覆盖率数据..."
bazel clean --expunge 2>/dev/null || true
rm -f bazel-out/_coverage/_coverage_report.dat 2>/dev/null || true

# 运行覆盖率测试
log_info "运行Level1-Level2覆盖率测试..."

if bazel coverage \
    //tests/level1_foundation/... \
    //tests/level2_core_services/... \
    //tests/level2_storage_engine/... \
    --instrumentation_filter="//include/..." \
    --combined_report=lcov \
    --test_output=errors \
    2>&1 | tee "${OUTPUT_DIR}/logs/coverage/coverage_test_${TIMESTAMP}.log"; then
    log_success "覆盖率测试完成"
    COVERAGE_STATUS="✅ 成功"
else
    log_warn "覆盖率测试部分失败，继续生成报告..."
    COVERAGE_STATUS="⚠️ 部分失败"
fi

# 查找覆盖率报告
log_info "查找覆盖率报告..."

COVERAGE_REPORT=$(find bazel-out/_coverage -name "_coverage_report.dat" 2>/dev/null | head -1)

if [ -n "$COVERAGE_REPORT" ] && [ -f "$COVERAGE_REPORT" ]; then
    log_success "找到覆盖率报告: $COVERAGE_REPORT"
    
    # 复制覆盖率报告
    cp "$COVERAGE_REPORT" "${REPORT_DIR}/coverage_report.dat"
    
    # 生成文本报告
    if command -v lcov &> /dev/null; then
        log_info "生成文本报告..."
        
        # 生成覆盖率摘要
        lcov --summary "${REPORT_DIR}/coverage_report.dat" \
            > "${REPORT_DIR}/text/coverage_summary.txt"
        
        # 显示覆盖率统计
        echo ""
        echo "=== 覆盖率统计 ==="
        cat "${REPORT_DIR}/text/coverage_summary.txt"
        echo ""
        
        # 生成详细报告
        lcov --list "${REPORT_DIR}/coverage_report.dat" \
            > "${REPORT_DIR}/text/coverage_details.txt"
        
        # 生成覆盖率最低的文件列表
        lcov --list "${REPORT_DIR}/coverage_report.dat" | \
            awk '{print $5}' | sort -n | head -10 \
            > "${REPORT_DIR}/text/coverage_lowest.txt"
        
        log_success "文本报告生成完成"
    fi
    
    # 生成HTML报告
    if command -v genhtml &> /dev/null; then
        log_info "生成HTML报告..."
        genhtml --ignore-errors inconsistent,corrupt \
            "${REPORT_DIR}/coverage_report.dat" \
            -o "${REPORT_DIR}/html" \
            --title "SQLCC Level1-Level2 Coverage Report ${TIMESTAMP}"
        
        log_success "HTML报告生成完成"
    fi
    
    # 创建最新报告链接
    rm -f "${COVERAGE_DIR}/reports/latest"
    ln -s "${TIMESTAMP}" "${COVERAGE_DIR}/reports/latest"
    
    log_success "最新报告链接创建完成"
else
    log_error "未找到覆盖率报告"
    echo "测试执行失败，请检查日志文件"
    echo "日志位置: ${OUTPUT_DIR}/logs/coverage/coverage_test_${TIMESTAMP}.log"
    exit 1
fi

# 生成测试结果摘要
log_info "生成测试结果摘要..."

cat > "${OUTPUT_DIR}/results/coverage_summary_${TIMESTAMP}.json" << EOF
{
  "timestamp": "${TIMESTAMP}",
  "test_date": "$(date '+%Y-%m-%d %H:%M:%S')",
  "test_scope": "Level1-Level2",
  "status": "${COVERAGE_STATUS}",
  "coverage_report": "${REPORT_DIR}/coverage_report.dat",
  "reports": {
    "text_summary": "${REPORT_DIR}/text/coverage_summary.txt",
    "text_details": "${REPORT_DIR}/text/coverage_details.txt",
    "html_report": "${REPORT_DIR}/html/index.html",
    "lowest_coverage": "${REPORT_DIR}/text/coverage_lowest.txt"
  },
  "logs": {
    "coverage_test": "${OUTPUT_DIR}/logs/coverage/coverage_test_${TIMESTAMP}.log"
  }
}
EOF

log_success "测试结果摘要生成完成"

echo ""
echo "========================================="
echo "覆盖率测试完成!"
echo "========================================="
echo ""
echo "测试状态: ${COVERAGE_STATUS}"
echo ""
echo "报告位置:"
echo "  - HTML报告: ${REPORT_DIR}/html/index.html"
echo "  - 文本摘要: ${REPORT_DIR}/text/coverage_summary.txt"
echo "  - 详细报告: ${REPORT_DIR}/text/coverage_details.txt"
echo "  - 最低覆盖率: ${REPORT_DIR}/text/coverage_lowest.txt"
echo "  - 原始数据: ${REPORT_DIR}/coverage_report.dat"
echo "  - 最新链接: ${COVERAGE_DIR}/reports/latest/"
echo ""
echo "日志位置:"
echo "  - ${OUTPUT_DIR}/logs/coverage/coverage_test_${TIMESTAMP}.log"
echo ""
echo "结果摘要:"
echo "  - ${OUTPUT_DIR}/results/coverage_summary_${TIMESTAMP}.json"
echo ""
echo "完成时间: $(date)"
echo "========================================="