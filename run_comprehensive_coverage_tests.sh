#!/bin/bash

# SQLCC 全面覆盖率测试运行脚本
# 运行多个测试目标来收集更完整的覆盖率数据

set -e

echo "=== SQLCC 全面覆盖率测试开始 ==="
echo "开始时间: $(date)"

# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

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

# 测试目标列表
declare -a test_targets=(
    "//tests/level1_foundation:basic_test"
    "//tests/unit/core:config_manager_test"
    "//tests/unit/utils:logger_test"
    "//tests/unit/exception:exception_test"
    "//tests/storage_engine:buffer_pool_test"
    "//tests/sql_parser:parser_test"
)

# 覆盖率报告目录
COVERAGE_DIR="./comprehensive_coverage_$(date +%Y%m%d_%H%M%S)"
mkdir -p "${COVERAGE_DIR}"

log_info "将运行以下测试目标收集覆盖率数据:"
for target in "${test_targets[@]}"; do
    echo "  - ${target}"
done

# 运行测试并收集覆盖率
total_tests=0
passed_tests=0
failed_tests=0

for target in "${test_targets[@]}"; do
    log_info "运行测试: ${target}"

    if bazel coverage "${target}" --combined_report=lcov --test_output=summary; then
        log_success "测试 ${target} 通过"
        ((passed_tests++))
    else
        log_error "测试 ${target} 失败"
        ((failed_tests++))
    fi

    ((total_tests++))
done

# 收集覆盖率报告
log_info "收集覆盖率报告..."

# 查找最新的覆盖率报告文件
LATEST_COVERAGE=$(find /home/liying/.cache/bazel -name "_coverage_report.dat" -type f -printf '%T@ %p\n' 2>/dev/null | sort -n | tail -1 | cut -d' ' -f2-)

if [ -n "${LATEST_COVERAGE}" ]; then
    cp "${LATEST_COVERAGE}" "${COVERAGE_DIR}/comprehensive_coverage.dat"
    log_success "覆盖率数据已保存到: ${COVERAGE_DIR}/comprehensive_coverage.dat"
else
    log_warning "未找到覆盖率报告文件"
fi

# 生成覆盖率统计
log_info "生成覆盖率统计..."

cat > "${COVERAGE_DIR}/test_execution_summary.txt" << EOF
SQLCC 全面覆盖率测试执行总结
生成时间: $(date)
=====================================

测试执行统计:
- 总测试目标: ${total_tests}
- 通过测试: ${passed_tests}
- 失败测试: ${failed_tests}
- 成功率: $(awk "BEGIN {printf \"%.1f\", ${passed_tests}/${total_tests}*100}")%

覆盖率数据:
- 数据文件: comprehensive_coverage.dat
- 格式: LCOV (LLVM Coverage)
- 工具链: LLVM Clang-20 + llvm-cov-20

测试目标列表:
EOF

for target in "${test_targets[@]}"; do
    echo "- ${target}" >> "${COVERAGE_DIR}/test_execution_summary.txt"
done

# 分析覆盖率数据（如果有的话）
if [ -f "${COVERAGE_DIR}/comprehensive_coverage.dat" ]; then
    TOTAL_FILES=$(grep -c "^SF:" "${COVERAGE_DIR}/comprehensive_coverage.dat" || echo "0")
    SOURCE_FILES=$(grep "^SF:" "${COVERAGE_DIR}/comprehensive_coverage.dat" | grep -c "\.cpp" || echo "0")
    HEADER_FILES=$((TOTAL_FILES - SOURCE_FILES))

    cat >> "${COVERAGE_DIR}/test_execution_summary.txt" << EOF

覆盖率文件分析:
- 总文件数: ${TOTAL_FILES}
- 源文件数: ${SOURCE_FILES}
- 头文件数: ${HEADER_FILES}

注意: 覆盖率数据可能需要进一步处理以生成完整的报告。
建议使用 llvm-cov 工具进行详细分析。
EOF
fi

log_success "测试执行总结已保存到: ${COVERAGE_DIR}/test_execution_summary.txt"

echo ""
echo "=== 测试执行结果 ==="
echo "总测试目标: ${total_tests}"
echo "通过测试: ${passed_tests}"
echo "失败测试: ${failed_tests}"
echo "覆盖率数据目录: ${COVERAGE_DIR}"

if [ ${failed_tests} -eq 0 ]; then
    log_success "所有测试执行成功！"
else
    log_warning "${failed_tests} 个测试执行失败，请检查详细日志"
fi

echo ""
echo "=== SQLCC 全面覆盖率测试完成 ==="
echo "结束时间: $(date)"
echo "结果目录: ${COVERAGE_DIR}"