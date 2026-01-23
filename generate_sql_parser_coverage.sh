#!/bin/bash
# generate_sql_parser_coverage.sh
# 作用: 完整生成 SQL Parser Lexer 测试的 LLVM 覆盖率 HTML 报告

set -euo pipefail  # 启用严格模式

# 颜色输出函数
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

# 1️⃣ 设置工作目录
WORKDIR="/home/liying/sqlcc"
cd "$WORKDIR" || {
    log_error "无法切换到工作目录: $WORKDIR"
    exit 1
}
log_info "工作目录: $PWD"

# 2️⃣ 清理旧的 coverage 文件
log_info "清理旧的 profraw/profdata 文件..."
rm -f *.profraw *.profdata
rm -rf coverage_html coverage_html_report
find . -name "*.profraw" -o -name "*.profdata" -delete 2>/dev/null || true

# 3️⃣ 检查测试目标是否存在
TEST_TARGET="//tests/level2_core_services/sql_parser/lexer:test_concat_operator_extension"
log_info "检查测试目标是否存在..."
if ! bazel query "$TEST_TARGET" >/dev/null 2>&1; then
    log_error "测试目标不存在: $TEST_TARGET"
    exit 1
fi
log_success "测试目标存在: $TEST_TARGET"

# 4️⃣ 编译测试目标
log_info "编译测试目标 $TEST_TARGET ..."
if ! bazel build "$TEST_TARGET" --verbose_failures; then
    log_error "编译失败"
    exit 1
fi
log_success "编译完成"

# 5️⃣ 执行测试并生成覆盖率数据
log_info "运行测试并收集覆盖率..."
if ! bazel test "$TEST_TARGET" \
    --collect_code_coverage \
    --instrumentation_filter="//src/sql_parser/.*" \
    --test_output=streamed \
    --combined_report=lcov \
    --jobs=1 \
    --test_timeout=300 \
    --verbose_failures; then
    log_error "测试执行失败"
    exit 1
fi
log_success "测试执行完成"

# 6️⃣ 查找生成的 profdata 文件
log_info "查找生成的 .profdata 文件..."
PROFDATA_FILES=$(find . -name "*.profdata" -type f 2>/dev/null | head -5)
if [ -z "$PROFDATA_FILES" ]; then
    log_error "未找到 .profdata 文件"
    log_info "在以下位置搜索覆盖率文件:"
    find . -name "*.profraw" -o -name "*.profdata" 2>/dev/null || true
    exit 1
fi

# 选择最新的profdata文件
PROFDATA=$(echo "$PROFDATA_FILES" | head -1)
log_info "使用 profdata 文件: $PROFDATA"

# 验证profdata文件有效性
if ! llvm-profdata-20 show "$PROFDATA" >/dev/null 2>&1; then
    log_error "profdata文件无效: $PROFDATA"
    exit 1
fi
log_success "profdata文件验证通过"

# 7️⃣ 查找可执行文件
log_info "查找测试可执行文件..."
EXE_PATH=$(bazel info bazel-bin 2>/dev/null)/tests/level2_core_services/sql_parser/lexer/test_concat_operator_extension
if [ ! -f "$EXE_PATH" ]; then
    log_error "未找到可执行文件: $EXE_PATH"
    log_info "尝试查找可执行文件位置..."
    find "$(bazel info bazel-bin 2>/dev/null)" -name "*test_concat_operator_extension*" 2>/dev/null || true
    exit 1
fi
log_info "使用可执行文件: $EXE_PATH"

# 8️⃣ 使用 llvm-cov 生成 HTML 覆盖率报告
log_info "生成 HTML 覆盖率报告..."
if ! llvm-cov-20 show \
    "$EXE_PATH" \
    -instr-profile="$PROFDATA" \
    --format=html \
    --show-line-counts-or-regions \
    --output-dir=coverage_html \
    --project-title="SQL Parser Lexer Coverage Report"; then
    log_error "HTML报告生成失败"
    exit 1
fi

# 验证HTML报告生成
if [ ! -f "coverage_html/index.html" ]; then
    log_error "HTML报告文件未生成"
    exit 1
fi

log_success "HTML 报告生成完成"

# 9️⃣ 显示报告信息
REPORT_PATH="$PWD/coverage_html/index.html"
log_info "覆盖率报告位置: $REPORT_PATH"
log_info "文件大小: $(du -h coverage_html/index.html | cut -f1)"

# 1️⃣0️⃣ 可选：自动打开浏览器（Linux系统）
if command -v xdg-open >/dev/null 2>&1; then
    log_info "尝试自动打开浏览器..."
    if xdg-open "$REPORT_PATH" >/dev/null 2>&1; then
        log_success "浏览器已打开"
    else
        log_warn "无法自动打开浏览器，请手动打开: $REPORT_PATH"
    fi
else
    log_info "请手动打开浏览器查看报告: $REPORT_PATH"
fi

log_success "LLVM 覆盖率报告生成完毕！"
echo ""
echo "📊 报告摘要:"
echo "  - 报告位置: $REPORT_PATH"
echo "  - 浏览器命令: xdg-open $REPORT_PATH"
echo "  - 原始数据: $PROFDATA"
echo "  - 可执行文件: $EXE_PATH"
