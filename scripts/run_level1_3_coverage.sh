#!/bin/bash
#
# SQLCC Level1-Level3 覆盖率测试脚本
# 使用Bazel内置Coverage功能运行Level1-Level3测试并生成覆盖率报告
# 包含所有新增的level1和level2单元测试
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
echo "  SQLCC Level1-Level3 覆盖率测试"
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
mkdir -p "${OUTPUT_DIR}/results"

# 清理旧的覆盖率数据
log_info "清理旧的覆盖率数据..."
bazel clean --expunge 2>/dev/null || true
rm -f bazel-out/_coverage/_coverage_report.dat 2>/dev/null || true

# 定义测试范围
log_info "测试范围: Level1-Level3"
echo "  - Level1: 基础组件测试"
echo "  - Level2: 核心服务和存储引擎测试"
echo "  - Level3: 事务管理器测试"
echo ""

# 运行覆盖率测试
log_info "运行Level1-Level3覆盖率测试..."

if bazel coverage \
    --config=coverage \
    --test_output=all \
    //tests/level1_foundation/... \
    //tests/level2_core/... \
    //tests/level2_core_services/... \
    //tests/level2_storage_engine/... \
    //tests/level3_transaction_manager/... \
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
        
        # 生成覆盖率最高的文件列表
        lcov --list "${REPORT_DIR}/coverage_report.dat" | \
            awk '{print $5}' | sort -rn | head -10 \
            > "${REPORT_DIR}/text/coverage_highest.txt"
        
        log_success "文本报告生成完成"
    fi
    
    # 使用llvm-cov生成HTML报告
    log_info "使用llvm-cov生成HTML报告..."
    
    # 收集所有.profraw文件
    PROFDATA_FILES=$(find bazel-out -name "*.profraw" 2>/dev/null)
    
    if [ -n "$PROFDATA_FILES" ]; then
        log_info "找到$(echo "$PROFDATA_FILES" | wc -l)个.profraw文件"
        
        # 合并覆盖率数据
        MERGED_PROFDATA="${REPORT_DIR}/merged.profdata"
        if llvm-profdata-20 merge $PROFDATA_FILES -o "$MERGED_PROFDATA" 2>&1; then
            log_success "覆盖率数据合并完成"
            
            # 查找所有可执行文件
            BINARY_FILES=$(find bazel-out -name "test_*" -type f -executable 2>/dev/null | head -20)
            
            if [ -n "$BINARY_FILES" ]; then
                log_info "找到$(echo "$BINARY_FILES" | wc -l)个可执行文件"
                
                # 使用第一个二进制文件生成报告
                FIRST_BINARY=$(echo "$BINARY_FILES" | head -1)
                log_info "使用二进制文件生成报告: $FIRST_BINARY"
                
                # 生成HTML报告
                if llvm-cov-20 report "$FIRST_BINARY" \
                    -instr-profile="$MERGED_PROFDATA" \
                    -format=html \
                    -output-dir="${REPORT_DIR}/html" \
                    -title="SQLCC Level1-Level3 Coverage Report ${TIMESTAMP}" 2>&1; then
                    log_success "HTML报告生成完成"
                else
                    log_warn "llvm-cov HTML报告生成失败，尝试使用genhtml"
                    
                    # 回退到genhtml
                    if command -v genhtml &> /dev/null; then
                        genhtml --ignore-errors inconsistent,corrupt \
                            "${REPORT_DIR}/coverage_report.dat" \
                            -o "${REPORT_DIR}/html" \
                            --title "SQLCC Level1-Level3 Coverage Report ${TIMESTAMP}"
                        log_success "HTML报告生成完成（使用genhtml）"
                    fi
                fi
                
                # 生成文本格式的覆盖率报告
                llvm-cov-20 report "$FIRST_BINARY" \
                    -instr-profile="$MERGED_PROFDATA" \
                    > "${REPORT_DIR}/text/llvm_cov_report.txt" 2>&1 || true
                
                log_success "llvm-cov报告生成完成"
            else
                log_warn "未找到可执行文件，使用genhtml生成报告"
                
                # 使用genhtml生成报告
                if command -v genhtml &> /dev/null; then
                    genhtml --ignore-errors inconsistent,corrupt \
                        "${REPORT_DIR}/coverage_report.dat" \
                        -o "${REPORT_DIR}/html" \
                        --title "SQLCC Level1-Level3 Coverage Report ${TIMESTAMP}"
                    log_success "HTML报告生成完成（使用genhtml）"
                fi
            fi
        else
            log_warn "覆盖率数据合并失败，使用genhtml生成报告"
            
            # 使用genhtml生成报告
            if command -v genhtml &> /dev/null; then
                genhtml --ignore-errors inconsistent,corrupt \
                    "${REPORT_DIR}/coverage_report.dat" \
                    -o "${REPORT_DIR}/html" \
                    --title "SQLCC Level1-Level3 Coverage Report ${TIMESTAMP}"
                log_success "HTML报告生成完成（使用genhtml）"
            fi
        fi
    else
        log_warn "未找到.profraw文件，使用genhtml生成报告"
        
        # 使用genhtml生成报告
        if command -v genhtml &> /dev/null; then
            genhtml --ignore-errors inconsistent,corrupt \
                "${REPORT_DIR}/coverage_report.dat" \
                -o "${REPORT_DIR}/html" \
                --title "SQLCC Level1-Level3 Coverage Report ${TIMESTAMP}"
            log_success "HTML报告生成完成（使用genhtml）"
        fi
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

# 提取测试统计信息
TEST_COUNT=$(grep -c "^PASS\|^FAIL\|^OK" "${OUTPUT_DIR}/logs/coverage/coverage_test_${TIMESTAMP}.log" 2>/dev/null || echo "0")
PASS_COUNT=$(grep -c "^PASS" "${OUTPUT_DIR}/logs/coverage/coverage_test_${TIMESTAMP}.log" 2>/dev/null || echo "0")
FAIL_COUNT=$(grep -c "^FAIL" "${OUTPUT_DIR}/logs/coverage/coverage_test_${TIMESTAMP}.log" 2>/dev/null || echo "0")

# 提取覆盖率统计
if [ -f "${REPORT_DIR}/text/coverage_summary.txt" ]; then
    LINES=$(grep -E "^lines" "${REPORT_DIR}/text/coverage_summary.txt" 2>/dev/null || echo "")
    FUNCTIONS=$(grep -E "^functions" "${REPORT_DIR}/text/coverage_summary.txt" 2>/dev/null || echo "")
    BRANCHES=$(grep -E "^branches" "${REPORT_DIR}/text/coverage_summary.txt" 2>/dev/null || echo "")
else
    LINES="N/A"
    FUNCTIONS="N/A"
    BRANCHES="N/A"
fi

cat > "${OUTPUT_DIR}/results/coverage_summary_${TIMESTAMP}.json" << EOF
{
  "timestamp": "${TIMESTAMP}",
  "test_date": "$(date '+%Y-%m-%d %H:%M:%S')",
  "test_scope": "Level1-Level3",
  "status": "${COVERAGE_STATUS}",
  "coverage_report": "${REPORT_DIR}/coverage_report.dat",
  "test_stats": {
    "total_tests": "${TEST_COUNT}",
    "passed": "${PASS_COUNT}",
    "failed": "${FAIL_COUNT}"
  },
  "coverage_stats": {
    "lines": "${LINES}",
    "functions": "${FUNCTIONS}",
    "branches": "${BRANCHES}"
  },
  "reports": {
    "text_summary": "${REPORT_DIR}/text/coverage_summary.txt",
    "text_details": "${REPORT_DIR}/text/coverage_details.txt",
    "html_report": "${REPORT_DIR}/html/index.html",
    "lowest_coverage": "${REPORT_DIR}/text/coverage_lowest.txt",
    "highest_coverage": "${REPORT_DIR}/text/coverage_highest.txt",
    "llvm_cov_report": "${REPORT_DIR}/text/llvm_cov_report.txt"
  },
  "logs": {
    "coverage_test": "${OUTPUT_DIR}/logs/coverage/coverage_test_${TIMESTAMP}.log"
  }
}
EOF

# 生成Markdown格式的分析报告
cat > "${REPORT_DIR}/coverage_analysis.md" << EOF
# SQLCC Level1-Level3 覆盖率测试报告

**生成时间**: $(date '+%Y-%m-%d %H:%M:%S')  
**测试范围**: Level1-Level3  
**状态**: ${COVERAGE_STATUS}

## 测试统计

- **总测试数**: ${TEST_COUNT}
- **通过**: ${PASS_COUNT}
- **失败**: ${FAIL_COUNT}

## 覆盖率统计

### 行覆盖率
${LINES}

### 函数覆盖率
${FUNCTIONS}

### 分支覆盖率
${BRANCHES}

## 测试范围

### Level1: 基础组件测试
- 基础类型测试
- 工具函数测试
- 日志系统测试
- 配置管理测试

### Level2: 核心服务和存储引擎测试
- 执行上下文测试
- 用户管理测试
- 数据库管理测试
- 权限验证测试
- SQL解析器测试
- B+树索引测试
- 缓冲池测试
- 磁盘管理测试
- 索引管理测试

### Level3: 事务管理器测试
- 事务管理器测试
- 配置测试
- 任务执行器测试

## 报告文件

- [HTML报告](html/index.html)
- [文本摘要](text/coverage_summary.txt)
- [详细报告](text/coverage_details.txt)
- [最低覆盖率文件](text/coverage_lowest.txt)
- [最高覆盖率文件](text/coverage_highest.txt)
- [LLVM COV报告](text/llvm_cov_report.txt)

## 改进建议

### 低覆盖率模块
$(cat "${REPORT_DIR}/text/coverage_lowest.txt" 2>/dev/null || echo "无数据")

### 高覆盖率模块
$(cat "${REPORT_DIR}/text/coverage_highest.txt" 2>/dev/null || echo "无数据")

## 下一步

1. 分析低覆盖率模块
2. 补充测试用例
3. 重新运行测试
4. 验证覆盖率提升

---

**报告生成**: run_level1_3_coverage.sh  
**版本**: v1.3.8
EOF

log_success "测试结果摘要生成完成"

echo ""
echo "========================================="
echo "覆盖率测试完成!"
echo "========================================="
echo ""
echo "测试状态: ${COVERAGE_STATUS}"
echo ""
echo "测试统计:"
echo "  - 总测试数: ${TEST_COUNT}"
echo "  - 通过: ${PASS_COUNT}"
echo "  - 失败: ${FAIL_COUNT}"
echo ""
echo "报告位置:"
echo "  - HTML报告: ${REPORT_DIR}/html/index.html"
echo "  - 文本摘要: ${REPORT_DIR}/text/coverage_summary.txt"
echo "  - 详细报告: ${REPORT_DIR}/text/coverage_details.txt"
echo "  - 最低覆盖率: ${REPORT_DIR}/text/coverage_lowest.txt"
echo "  - 最高覆盖率: ${REPORT_DIR}/text/coverage_highest.txt"
echo "  - LLVM COV报告: ${REPORT_DIR}/text/llvm_cov_report.txt"
echo "  - 分析报告: ${REPORT_DIR}/coverage_analysis.md"
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