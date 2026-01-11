#!/bin/bash

# SQLCC注释质量检查脚本
# 用于检查头文件的注释是否符合WHY/WHAT/HOW标准

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 统计变量
total_files=0
passed_files=0
failed_files=0

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

# 检查单个文件的注释质量
check_file_comments() {
    local file="$1"
    local basename=$(basename "$file")

    log_info "检查文件: $basename"

    # 检查文件是否存在
    if [[ ! -f "$file" ]]; then
        log_error "文件不存在: $file"
        return 1
    fi

    # 检查是否为C/C++头文件
    if [[ ! "$file" =~ \.(h|hpp)$ ]]; then
        log_warning "跳过非头文件: $basename"
        return 0
    fi

    ((total_files++))

    # 读取文件内容
    local content
    content=$(cat "$file")

    # 检查WHY注释
    if ! echo "$content" | grep -q "^ \* WHY:"; then
        log_error "$basename: 缺少WHY注释部分"
        ((failed_files++))
        return 1
    fi

    # 检查WHAT注释
    if ! echo "$content" | grep -q "^ \* WHAT:"; then
        log_error "$basename: 缺少WHAT注释部分"
        ((failed_files++))
        return 1
    fi

    # 检查HOW注释
    if ! echo "$content" | grep -q "^ \* HOW:"; then
        log_error "$basename: 缺少HOW注释部分"
        ((failed_files++))
        return 1
    fi

    # 检查设计模式说明
    if ! echo "$content" | grep -q "🏗️ 设计模式："; then
        log_error "$basename: 缺少设计模式说明"
        ((failed_files++))
        return 1
    fi

    # 检查SOLID原则说明
    if ! echo "$content" | grep -q "SOLID原则体现："; then
        log_error "$basename: 缺少SOLID原则说明"
        ((failed_files++))
        return 1
    fi

    log_success "$basename: 注释质量检查通过"
    ((passed_files++))
    return 0
}

# 递归检查目录中的所有头文件
check_directory() {
    local dir="$1"
    local pattern="$2"

    log_info "扫描目录: $dir"

    if [[ ! -d "$dir" ]]; then
        log_error "目录不存在: $dir"
        return 1
    fi

    # 查找匹配的头文件
    while IFS= read -r -d '' file; do
        check_file_comments "$file"
    done < <(find "$dir" -name "$pattern" -type f -print0 2>/dev/null)
}

# 生成报告
generate_report() {
    echo
    echo "========================================"
    echo "     SQLCC注释质量检查报告"
    echo "========================================"
    echo "检查时间: $(date)"
    echo "检查目录: $PROJECT_ROOT/include"
    echo
    echo "统计结果:"
    echo "  总文件数: $total_files"
    echo "  通过文件: $passed_files"
    echo "  失败文件: $failed_files"
    echo

    if [[ $failed_files -eq 0 ]]; then
        log_success "所有文件注释质量检查通过! 🎉"
        return 0
    else
        local pass_rate=$((passed_files * 100 / total_files))
        log_warning "通过率: ${pass_rate}%"
        log_error "$failed_files 个文件需要修复注释"
        return 1
    fi
}

# 主函数
main() {
    log_info "开始SQLCC注释质量检查..."
    log_info "项目根目录: $PROJECT_ROOT"

    # 检查include目录中的所有头文件
    check_directory "$PROJECT_ROOT/include" "*.h"
    check_directory "$PROJECT_ROOT/include" "*.hpp"

    # 生成报告
    generate_report
}

# 参数处理
case "${1:-}" in
    --help|-h)
        echo "SQLCC注释质量检查脚本"
        echo
        echo "用法: $0 [选项]"
        echo
        echo "选项:"
        echo "  --help, -h    显示帮助信息"
        echo "  --version, -v 显示版本信息"
        echo
        echo "检查标准:"
        echo "  - WHY: 解释设计意图和价值"
        echo "  - WHAT: 详细描述功能和接口"
        echo "  - HOW: 说明实现机制和算法"
        echo "  - 设计模式: 说明使用的设计模式"
        echo "  - SOLID原则: 说明遵循的原则"
        exit 0
        ;;
    --version|-v)
        echo "SQLCC注释质量检查脚本 v1.0.0"
        exit 0
        ;;
    *)
        main "$@"
        ;;
esac
