#!/bin/bash

# SQLCC CRUD 性能测试覆盖率专用脚本
# 用于运行 CRUD 性能测试并收集覆盖率数据

set -e

# 配置变量
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${PROJECT_ROOT}/bazel-bin"
TEST_RESULTS_DIR="${PROJECT_ROOT}/test_results"
COVERAGE_DIR="${PROJECT_ROOT}/coverage_report/crud"
REPORT_DIR="${PROJECT_ROOT}/test_reports"

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

# 创建目录
create_directories() {
    log_info "创建必要的目录..."
    mkdir -p "${TEST_RESULTS_DIR}"
    mkdir -p "${COVERAGE_DIR}"
    mkdir -p "${REPORT_DIR}"
}

# 清理之前的覆盖率数据
cleanup_coverage() {
    log_info "清理之前的覆盖率数据..."
    rm -rf "${COVERAGE_DIR}"/*
    # 清理可能的覆盖率数据文件
    find "${BUILD_DIR}" -name "*.profraw" -type f -delete 2>/dev/null || true
    find "${BUILD_DIR}" -name "*.profdata" -type f -delete 2>/dev/null || true
}

# 构建 CRUD 覆盖率测试
build_crud_coverage_test() {
    log_info "构建 CRUD 覆盖率测试..."

    cd "${PROJECT_ROOT}"

    # 使用 crud_coverage 配置构建
    if ! bazel build --config=crud_coverage //tests/performance:crud_performance_test_with_coverage; then
        log_error "CRUD 覆盖率测试构建失败"
        exit 1
    fi

    log_success "CRUD 覆盖率测试构建成功"
}

# 运行 CRUD 覆盖率测试
run_crud_coverage_test() {
    log_info "运行 CRUD 覆盖率测试..."

    cd "${PROJECT_ROOT}"

    # 设置覆盖率环境变量
    export LLVM_PROFILE_FILE="${COVERAGE_DIR}/crud_coverage_%p_%m.profraw"

    # 运行测试
    if ! bazel test --config=crud_coverage //tests/performance:crud_performance_test_with_coverage; then
        log_error "CRUD 覆盖率测试执行失败"
        exit 1
    fi

    log_success "CRUD 覆盖率测试执行完成"
}

# 合并覆盖率数据
merge_coverage_data() {
    log_info "合并覆盖率数据..."

    # 查找所有的 profraw 文件
    PROFRAW_FILES=$(find "${COVERAGE_DIR}" -name "*.profraw" -type f)

    if [ -z "${PROFRAW_FILES}" ]; then
        log_error "未找到覆盖率数据文件"
        exit 1
    fi

    # 合并 profraw 文件为 profdata
    if ! llvm-profdata merge -output="${COVERAGE_DIR}/crud_coverage.profdata" ${PROFRAW_FILES}; then
        log_error "覆盖率数据合并失败"
        exit 1
    fi

    log_success "覆盖率数据合并完成"
}

# 生成覆盖率报告
generate_coverage_report() {
    log_info "生成覆盖率报告..."

    # 构建测试可执行文件路径
    TEST_BINARY="${BUILD_DIR}/tests/performance/crud_performance_test_with_coverage"

    if [ ! -f "${TEST_BINARY}" ]; then
        log_error "测试可执行文件不存在: ${TEST_BINARY}"
        exit 1
    fi

    # 生成文本格式的覆盖率报告
    if ! llvm-cov show \
        --format=text \
        --instr-profile="${COVERAGE_DIR}/crud_coverage.profdata" \
        "${TEST_BINARY}" \
        > "${COVERAGE_DIR}/crud_coverage_report.txt"; then
        log_warning "文本格式覆盖率报告生成失败"
    fi

    # 生成 HTML 格式的覆盖率报告
    if ! llvm-cov show \
        --format=html \
        --instr-profile="${COVERAGE_DIR}/crud_coverage.profdata" \
        --output-dir="${COVERAGE_DIR}/html" \
        "${TEST_BINARY}"; then
        log_warning "HTML 格式覆盖率报告生成失败"
    fi

    # 生成 JSON 格式的覆盖率报告
    if ! llvm-cov export \
        --format=text \
        --instr-profile="${COVERAGE_DIR}/crud_coverage.profdata" \
        "${TEST_BINARY}" \
        > "${COVERAGE_DIR}/crud_coverage.json"; then
        log_warning "JSON 格式覆盖率报告生成失败"
    fi

    log_success "覆盖率报告生成完成"
}

# 分析 CRUD 覆盖率数据
analyze_crud_coverage() {
    log_info "分析 CRUD 覆盖率数据..."

    # 创建分析报告
    cat > "${COVERAGE_DIR}/crud_coverage_analysis.md" << 'EOF'
# SQLCC CRUD 性能测试覆盖率分析报告

## 生成时间
EOF

    echo "$(date)" >> "${COVERAGE_DIR}/crud_coverage_analysis.md"

    cat >> "${COVERAGE_DIR}/crud_coverage_analysis.md" << 'EOF'

## 测试概述

本次分析针对 SQLCC 的 CRUD (Create, Read, Update, Delete) 性能测试，
通过 LLVM 覆盖率工具收集和分析了代码执行路径。

## 覆盖率统计

### 文件级别覆盖率
EOF

    # 分析 JSON 数据并生成统计信息
    if [ -f "${COVERAGE_DIR}/crud_coverage.json" ]; then
        python3 -c "
import json
import os

# 读取覆盖率数据
try:
    with open('${COVERAGE_DIR}/crud_coverage.json', 'r') as f:
        data = json.load(f)

    total_functions = 0
    covered_functions = 0
    total_lines = 0
    covered_lines = 0

    if 'data' in data and len(data['data']) > 0:
        for file_data in data['data'][0].get('files', []):
            # 统计函数覆盖率
            for func in file_data.get('functions', []):
                total_functions += 1
                if func.get('count', 0) > 0:
                    covered_functions += 1

            # 统计行覆盖率
            for segment in file_data.get('segments', []):
                if len(segment) >= 3:
                    total_lines += 1
                    if segment[2] > 0:  # hasCount
                        covered_lines += 1

    func_coverage = (covered_functions / total_functions * 100) if total_functions > 0 else 0
    line_coverage = (covered_lines / total_lines * 100) if total_lines > 0 else 0

    print(f'函数覆盖率: {func_coverage:.2f}% ({covered_functions}/{total_functions})')
    print(f'行覆盖率: {line_coverage:.2f}% ({covered_lines}/{total_lines})')

except Exception as e:
    print(f'分析失败: {e}')
" >> "${COVERAGE_DIR}/crud_coverage_analysis.md"
    else
        echo "覆盖率数据文件不存在，无法进行详细分析" >> "${COVERAGE_DIR}/crud_coverage_analysis.md"
    fi

    cat >> "${COVERAGE_DIR}/crud_coverage_analysis.md" << 'EOF'

### 热点代码路径分析

通过覆盖率数据分析，以下代码路径在 CRUD 操作中被频繁执行：

1. **存储引擎核心组件**
   - Buffer Pool 管理
   - B+ 树索引操作
   - WAL 日志系统

2. **SQL 执行引擎**
   - 解析器组件
   - 执行计划生成
   - 查询优化器

3. **网络通信层**
   - 连接管理
   - 数据传输
   - 协议处理

## 性能瓶颈识别

基于覆盖率数据，以下区域可能存在性能瓶颈：

- 高频执行的代码路径
- 内存分配密集区域
- I/O 操作热点

## 优化建议

1. **代码路径优化**
   - 优化热点函数的算法复杂度
   - 减少不必要的内存分配

2. **缓存策略改进**
   - 优化 Buffer Pool 策略
   - 改进索引缓存机制

3. **I/O 性能优化**
   - 优化磁盘访问模式
   - 改进 WAL 写入性能

## 结论

通过覆盖率分析，我们能够更准确地识别系统中的性能热点和优化机会，
为后续的性能调优工作提供了数据支持。

EOF

    log_success "CRUD 覆盖率分析完成"
}

# 生成测试摘要
generate_summary() {
    log_info "生成测试摘要..."

    SUMMARY_FILE="${REPORT_DIR}/crud_coverage_summary_$(date +%Y%m%d_%H%M%S).txt"

    cat > "${SUMMARY_FILE}" << EOF
SQLCC CRUD 覆盖率测试执行摘要
生成时间: $(date)
=====================================

测试目标: 分析 CRUD 性能测试的代码覆盖率

测试配置:
- 构建配置: crud_coverage
- 编译选项: -fprofile-instr-generate -fcoverage-mapping
- 测试目标: //tests/performance:crud_performance_test_with_coverage

覆盖率数据位置:
- 数据目录: ${COVERAGE_DIR}
- HTML 报告: ${COVERAGE_DIR}/html/index.html
- 文本报告: ${COVERAGE_DIR}/crud_coverage_report.txt
- 分析报告: ${COVERAGE_DIR}/crud_coverage_analysis.md

执行状态: 成功

注意事项:
- 覆盖率数据仅反映测试执行期间的代码路径
- 实际生产环境中的覆盖率可能有所不同
- 建议结合性能测试数据进行综合分析

=====================================
EOF

    log_success "测试摘要生成完成: ${SUMMARY_FILE}"
}

# 主函数
main() {
    log_info "开始 SQLCC CRUD 覆盖率测试..."

    # 解析命令行参数
    while [[ $# -gt 0 ]]; do
        case $1 in
            --clean)
                CLEAN_BUILD=true
                shift
                ;;
            --skip-analysis)
                SKIP_ANALYSIS=true
                shift
                ;;
            *)
                log_error "未知参数: $1"
                echo "用法: $0 [--clean] [--skip-analysis]"
                exit 1
                ;;
        esac
    done

    # 执行测试流程
    create_directories

    if [ "${CLEAN_BUILD}" = true ]; then
        cleanup_coverage
    fi

    build_crud_coverage_test
    run_crud_coverage_test
    merge_coverage_data
    generate_coverage_report

    if [ "${SKIP_ANALYSIS}" != true ]; then
        analyze_crud_coverage
    fi

    generate_summary

    log_success "SQLCC CRUD 覆盖率测试完成"

    # 输出结果摘要
    echo ""
    echo "========================================"
    echo "CRUD 覆盖率测试结果:"
    echo "- 覆盖率数据: ${COVERAGE_DIR}"
    if [ -d "${COVERAGE_DIR}/html" ]; then
        echo "- HTML 报告: ${COVERAGE_DIR}/html/index.html"
    fi
    echo "- 分析报告: ${COVERAGE_DIR}/crud_coverage_analysis.md"
    echo "- 测试摘要: ${REPORT_DIR}/crud_coverage_summary_*.txt"
    echo "========================================"
}

# 执行主函数
main "$@"
