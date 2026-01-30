#!/bin/bash
#
# SQLCC Level1-Level2 覆盖率测试脚本（修复版）
# 使用Bazel内置Coverage功能
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
echo "  SQLCC Level1-Level2 覆盖率测试"
echo "  修复版本"
echo "========================================="
echo "开始时间: $(date)"
echo ""

# 检查依赖
log_info "检查依赖工具..."

if ! command -v bazel &> /dev/null; then
    log_error "Bazel 未安装"
    exit 1
fi

if ! command -v llvm-cov &> /dev/null && ! command -v llvm-cov-18 &> /dev/null && ! command -v llvm-cov-20 &> /dev/null; then
    log_warn "llvm-cov 未找到，将尝试使用Bazel默认配置"
fi

if command -v lcov &> /dev/null; then
    log_success "lcov 已安装"
else
    log_warn "lcov 未安装，HTML报告生成可能受限"
fi

if command -v genhtml &> /dev/null; then
    log_success "genhtml 已安装"
else
    log_warn "genhtml 未安装，HTML报告生成可能受限"
fi

# 创建输出目录
OUTPUT_DIR="${PROJECT_ROOT}/coverage_results_real"
TIMESTAMP=$(date +%Y%m%d_%H%M%S)
REPORT_DIR="${OUTPUT_DIR}/${TIMESTAMP}"

log_info "创建输出目录: ${REPORT_DIR}"
mkdir -p "${REPORT_DIR}/reports"
mkdir -p "${REPORT_DIR}/html"

# 检查.bazelrc配置
log_info "检查Bazel配置..."

if grep -q "coverage" .bazelrc 2>/dev/null; then
    log_success "找到coverage配置"
else
    log_warn "未找到coverage配置，将使用默认参数"
fi

# 清理旧的覆盖率数据
log_info "清理旧的覆盖率数据..."
bazel clean --expunge 2>/dev/null || true
rm -f bazel-out/_coverage/_coverage_report.dat 2>/dev/null || true

# 运行Level1测试
log_info "运行Level1测试..."

if bazel test //tests/level1_foundation/... \
    --config=coverage \
    --combined_report=lcov \
    --instrumentation_filter="//include/..." \
    --test_output=errors \
    2>&1 | tee "${REPORT_DIR}/level1_test.log"; then
    log_success "Level1测试完成"
else
    log_warn "Level1测试部分失败，继续执行..."
fi

# 运行Level2测试
log_info "运行Level2测试..."

if bazel test //tests/level2_core_services/... \
    --config=coverage \
    --combined_report=lcov \
    --instrumentation_filter="//include/..." \
    --test_output=errors \
    2>&1 | tee "${REPORT_DIR}/level2_test.log"; then
    log_success "Level2测试完成"
else
    log_warn "Level2测试部分失败，继续执行..."
fi

# 查找覆盖率报告
log_info "查找覆盖率报告..."

COVERAGE_REPORT=$(find bazel-out/_coverage -name "_coverage_report.dat" 2>/dev/null | head -1)

if [ -n "$COVERAGE_REPORT" ] && [ -f "$COVERAGE_REPORT" ]; then
    log_success "找到覆盖率报告: $COVERAGE_REPORT"
    
    # 复制覆盖率报告
    cp "$COVERAGE_REPORT" "${REPORT_DIR}/reports/combined_coverage.dat"
    
    # 生成文本报告
    if command -v lcov &> /dev/null; then
        log_info "生成文本报告..."
        lcov --summary "${REPORT_DIR}/reports/combined_coverage.dat" \
            > "${REPORT_DIR}/reports/coverage_summary.txt"
        
        # 显示覆盖率统计
        echo ""
        echo "=== 覆盖率统计 ==="
        cat "${REPORT_DIR}/reports/coverage_summary.txt"
        echo ""
    fi
    
    # 生成HTML报告
    if command -v genhtml &> /dev/null; then
        log_info "生成HTML报告..."
        genhtml --ignore-errors inconsistent,corrupt \
            "${REPORT_DIR}/reports/combined_coverage.dat" \
            -o "${REPORT_DIR}/html" \
            --title "Level1-Level2 Coverage Report ${TIMESTAMP}"
        
        log_success "HTML报告生成完成"
    fi
    
    # 生成详细报告
    if command -v lcov &> /dev/null; then
        log_info "生成详细覆盖率报告..."
        lcov --list "${REPORT_DIR}/reports/combined_coverage.dat" \
            > "${REPORT_DIR}/reports/coverage_details.txt"
        
        # 显示覆盖率最低的文件
        echo ""
        echo "=== 覆盖率最低的文件 ==="
        lcov --list "${REPORT_DIR}/reports/combined_coverage.dat" | \
            awk '{print $5}' | sort -n | head -10
        echo ""
    fi
else
    log_error "未找到覆盖率报告"
    echo "测试执行失败，请检查日志文件"
    echo "Level1测试日志: ${REPORT_DIR}/level1_test.log"
    echo "Level2测试日志: ${REPORT_DIR}/level2_test.log"
    exit 1
fi

# 创建最新报告链接
log_info "创建最新报告链接..."

rm -f "${OUTPUT_DIR}/latest_level1_level2"
ln -s "$REPORT_DIR" "${OUTPUT_DIR}/latest_level1_level2"

log_success "链接创建完成: ${OUTPUT_DIR}/latest_level1_level2"

# 生成分析报告
log_info "生成分析报告..."

cat > "${REPORT_DIR}/coverage_analysis.md" << EOF
# Level1-Level2 覆盖率测试报告

**生成时间**: $(date '+%Y-%m-%d %H:%M:%S')  
**测试范围**: Level1基础组件 + Level2核心服务  
**覆盖率工具**: Bazel Coverage + LLVM

## 测试执行情况

### Level1 测试
- **测试范围**: //tests/level1_foundation/...
- **执行状态**: 参见 ${REPORT_DIR}/level1_test.log
- **测试组件**:
  - 基础功能测试 (basic_test)
  - 异常处理测试 (exception_test)
  - 工具类测试 (utils_test)

### Level2 测试
- **测试范围**: //tests/level2_core_services/...
- **执行状态**: 参见 ${REPORT_DIR}/level2_test.log
- **测试组件**:
  - 配置管理器 (config_manager_test)
  - 数据库管理器 (database_manager_test)
  - 用户管理器 (user_manager_test)
  - 权限验证器 (permission_validator_test)
  - SQL解析器 (constraint_parser_test, window_function_test)

## 覆盖率统计

### 整体覆盖率
\`\`\`
$(cat "${REPORT_DIR}/reports/coverage_summary.txt" 2>/dev/null || echo "覆盖率数据不可用")
\`\`\`

### 详细覆盖率数据
\`\`\`
$(cat "${REPORT_DIR}/reports/coverage_details.txt" 2>/dev/null | head -50 || echo "详细数据不可用")
\`\`\`

## 报告位置

- **文本摘要**: ${REPORT_DIR}/reports/coverage_summary.txt
- **详细数据**: ${REPORT_DIR}/reports/coverage_details.txt
- **HTML报告**: ${REPORT_DIR}/html/index.html
- **原始数据**: ${REPORT_DIR}/reports/combined_coverage.dat

## 下一步行动

1. 分析覆盖率结果
2. 识别覆盖率低的模块
3. 补充测试用例
4. 提升覆盖率到目标值

---
*报告生成: ${TIMESTAMP}*
EOF

log_success "分析报告生成完成"

echo ""
echo "========================================="
log_success "覆盖率测试完成!"
echo "========================================="
echo ""
echo "报告位置:"
echo "  - 文本摘要: ${REPORT_DIR}/reports/coverage_summary.txt"
echo "  - 详细数据: ${REPORT_DIR}/reports/coverage_details.txt"
echo "  - HTML报告: ${REPORT_DIR}/html/index.html"
echo "  - 分析报告: ${REPORT_DIR}/coverage_analysis.md"
echo "  - 最新链接: ${OUTPUT_DIR}/latest_level1_level2"
echo ""
echo "测试日志:"
echo "  - Level1: ${REPORT_DIR}/level1_test.log"
echo "  - Level2: ${REPORT_DIR}/level2_test.log"
echo ""
echo "完成时间: $(date)"
echo "========================================="