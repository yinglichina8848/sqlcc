#!/bin/bash

# SQLCC 文档检查脚本
# 用于验证文档的完整性和一致性

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
DOCS_DIR="$PROJECT_ROOT/docs"

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

# 检查结果统计
ERRORS=0
WARNINGS=0

# 检查Markdown文件是否存在
check_file_exists() {
    local file="$1"
    if [[ ! -f "$file" ]]; then
        log_error "文件不存在: $file"
        ((ERRORS++))
        return 1
    fi
    return 0
}

# 检查文档链接
check_links() {
    local file="$1"
    log_info "检查文档链接: $(basename "$file")"

    # 查找所有markdown链接
    grep -n '\[.*\](\([^)]*\))' "$file" | while IFS=: read -r line_num content; do
        link=$(echo "$content" | grep -o '\[.*\](\([^)]*\))' | sed 's/.*(\([^)]*\)).*/\1/')

        # 跳过外部链接
        if [[ "$link" =~ ^https?:// ]]; then
            continue
        fi

        # 转换为绝对路径
        if [[ "$link" =~ ^/ ]]; then
            # 绝对路径，从docs目录开始
            target="$DOCS_DIR${link}"
        else
            # 相对路径
            target="$(dirname "$file")/$link"
        fi

        # 处理.md扩展名
        if [[ "$target" != *.md ]] && [[ "$target" != */ ]]; then
            target="$target.md"
        fi

        # 检查目标文件是否存在
        if [[ ! -f "$target" ]] && [[ ! -d "$target" ]]; then
            log_warning "损坏的链接 ($file:$line_num): $link -> $target"
            ((WARNINGS++))
        fi
    done
}

# 检查文档格式
check_formatting() {
    local file="$1"
    log_info "检查文档格式: $(basename "$file")"

    # 检查行长度（建议不超过120字符）
    local long_lines=$(awk 'length($0) > 120 {print NR ": " $0}' "$file" | wc -l)
    if [[ $long_lines -gt 0 ]]; then
        log_warning "发现 $long_lines 行过长的文本 (>120字符)"
    fi

    # 检查中英文混用
    if grep -q "。" "$file" && grep -q "[a-zA-Z]" "$file"; then
        # 检查中英文之间缺少空格
        if grep -q "。[a-zA-Z]" "$file" || grep -q "[a-zA-Z]。" "$file"; then
            log_warning "发现中英文之间可能缺少空格"
        fi
    fi

    # 检查标题格式
    if ! head -1 "$file" | grep -q '^# '; then
        log_warning "文档可能缺少正确的标题格式"
    fi
}

# 检查代码块
check_code_blocks() {
    local file="$1"
    log_info "检查代码块: $(basename "$file")"

    # 检查未闭合的代码块
    local open_blocks=$(grep -c '```' "$file")
    if [[ $((open_blocks % 2)) -ne 0 ]]; then
        log_error "发现未闭合的代码块"
        ((ERRORS++))
    fi
}

# 检查TODO项
check_todos() {
    local file="$1"
    log_info "检查TODO项: $(basename "$file")"

    # 查找TODO注释
    local todos=$(grep -c -i "TODO\|FIXME\|XXX" "$file" || true)
    if [[ $todos -gt 0 ]]; then
        log_info "发现 $todos 个TODO项待处理"
    fi
}

# 检查文档更新日期
check_update_date() {
    local file="$1"
    log_info "检查更新日期: $(basename "$file")"

    # 检查最后更新时间
    if ! grep -q "*最后更新" "$file"; then
        log_warning "文档缺少最后更新时间标记"
        ((WARNINGS++))
    fi
}

# 主要检查函数
main() {
    log_info "开始SQLCC文档检查..."
    log_info "文档目录: $DOCS_DIR"

    # 查找所有Markdown文件
    local markdown_files=()
    while IFS= read -r -d '' file; do
        markdown_files+=("$file")
    done < <(find "$DOCS_DIR" -name "*.md" -type f -print0)

    log_info "发现 ${#markdown_files[@]} 个文档文件"

    # 检查核心文档是否存在
    log_info "检查核心文档..."
    check_file_exists "$DOCS_DIR/index.md" || true
    check_file_exists "$DOCS_DIR/features/implementation_status.md" || true
    check_file_exists "$DOCS_DIR/code/coding_standards.md" || true
    check_file_exists "$DOCS_DIR/versions/roadmap.md" || true
    check_file_exists "$DOCS_DIR/user/user_guide.md" || true

    # 检查每个文档文件
    for file in "${markdown_files[@]}"; do
        log_info "处理文档: ${file#$PROJECT_ROOT/}"

        check_links "$file"
        check_formatting "$file"
        check_code_blocks "$file"
        check_todos "$file"
        check_update_date "$file"

        echo
    done

    # 生成报告
    echo "========================================"
    log_info "文档检查完成"
    echo "========================================"
    log_info "检查的文件数量: ${#markdown_files[@]}"
    log_info "发现的错误: $ERRORS"
    log_info "发现的警告: $WARNINGS"

    # 检查README.md中的链接
    if [[ -f "$PROJECT_ROOT/README.md" ]]; then
        log_info "检查README.md链接..."
        check_links "$PROJECT_ROOT/README.md"
    fi

    # 总结
    if [[ $ERRORS -eq 0 ]] && [[ $WARNINGS -eq 0 ]]; then
        log_success "所有检查通过！"
        return 0
    elif [[ $ERRORS -eq 0 ]]; then
        log_warning "发现 $WARNINGS 个警告，请检查"
        return 0
    else
        log_error "发现 $ERRORS 个错误，请修复"
        return 1
    fi
}

# 检查依赖
check_dependencies() {
    local missing_deps=()

    if ! command -v grep &> /dev/null; then
        missing_deps+=("grep")
    fi

    if ! command -v sed &> /dev/null; then
        missing_deps+=("sed")
    fi

    if ! command -v awk &> /dev/null; then
        missing_deps+=("awk")
    fi

    if [[ ${#missing_deps[@]} -gt 0 ]]; then
        log_error "缺少必要的工具: ${missing_deps[*]}"
        exit 1
    fi
}

# 参数处理
while [[ $# -gt 0 ]]; do
    case $1 in
        --help|-h)
            echo "SQLCC 文档检查脚本"
            echo ""
            echo "用法: $0 [选项]"
            echo ""
            echo "选项:"
            echo "  --help, -h    显示帮助信息"
            echo "  --verbose     详细输出"
            echo ""
            exit 0
            ;;
        --verbose)
            VERBOSE=1
            shift
            ;;
        *)
            log_error "未知选项: $1"
            exit 1
            ;;
    esac
done

# 执行检查
check_dependencies
main

exit $?
