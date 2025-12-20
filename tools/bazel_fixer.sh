#!/bin/bash
# tools/bazel_fixer.sh - Bazel自动修复工具
# 自动修复常见的Bazel配置问题

set -e

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 日志函数
log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1" >&2
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

# 显示帮助信息
show_help() {
    cat << EOF
Bazel自动修复工具

用法:
    $0 [选项] [目标]

选项:
    -h, --help          显示帮助信息
    -f, --format        格式化BUILD文件
    -c, --clean         清理Bazel缓存
    -l, --label-fix     修复标签引用问题
    -g, --glob-fix      修复glob模式问题
    -d, --deps-fix      修复依赖问题
    -a, --auto-fix      自动修复所有问题
    -v, --verbose       详细输出

目标:
    //...               处理所有目标
    //:sqlcc           处理主程序
    //src/...          处理源代码包

示例:
    $0 --format                            # 格式化所有BUILD文件
    $0 --clean                             # 清理缓存
    $0 --label-fix //:sqlcc               # 修复主程序的标签引用
    $0 --auto-fix                         # 自动修复所有问题

EOF
}

# 检查Bazel是否可用
check_bazel() {
    if ! command -v bazel &> /dev/null; then
        log_error "Bazel未安装或不在PATH中"
        exit 1
    fi
}

# 格式化BUILD文件
format_build_files() {
    log_info "格式化BUILD文件..."

    # 查找所有BUILD文件
    local build_files=$(find . -name "BUILD.bazel" -o -name "BUILD" -o -name "WORKSPACE" | grep -v bazel- | head -20)

    for file in $build_files; do
        if [[ -f "$file" ]]; then
            log_info "格式化: $file"
            # 使用buildifier格式化（如果可用）
            if command -v buildifier &> /dev/null; then
                buildifier -mode=fix "$file" 2>/dev/null || log_warn "buildifier格式化失败: $file"
            else
                log_warn "buildifier未安装，使用基本格式化"
                # 基本格式化：修复缩进和行尾空格
                sed -i 's/[[:space:]]*$//' "$file"
            fi
        fi
    done

    log_success "BUILD文件格式化完成"
}

# 清理Bazel缓存
clean_cache() {
    log_info "清理Bazel缓存..."

    # 清理输出目录
    bazel clean --expunge 2>/dev/null || log_warn "清理缓存失败"

    # 删除临时文件
    rm -rf bazel-*
    rm -f bazel_error.log

    log_success "缓存清理完成"
}

# 修复标签引用问题
fix_label_references() {
    local target="$1"
    local verbose="${2:-false}"

    log_info "修复标签引用问题..."

    # 获取构建错误
    local error_output=$(bazel build "$target" 2>&1 | grep "Label.*is invalid" || echo "")

    if [[ -z "$error_output" ]]; then
        log_success "未发现标签引用问题"
        return 0
    fi

    log_warn "发现标签引用问题，正在修复..."

    # 分析并修复每个错误
    while IFS= read -r line; do
        if [[ "$line" == *"Label '"*"' is invalid"* ]]; then
            # 提取错误信息
            local invalid_label=$(echo "$line" | sed "s/.*Label '\([^']*\)' is invalid.*/\1/")
            local suggestion=$(echo "$line" | sed "s/.*perhaps you meant to put the colon here: '\([^']*\)'.*/\1/")

            if [[ "$invalid_label" != "$suggestion" && -n "$suggestion" ]]; then
                log_info "修复: $invalid_label -> $suggestion"

                # 查找并替换错误的标签引用
                find . -name "BUILD.bazel" -exec grep -l "$invalid_label" {} \; | while read -r file; do
                    if [[ "$verbose" == "true" ]]; then
                        log_info "修复文件: $file"
                    fi
                    sed -i "s|$invalid_label|$suggestion|g" "$file"
                done
            fi
        fi
    done <<< "$error_output"

    log_success "标签引用修复完成"
}

# 修复glob模式问题
fix_glob_patterns() {
    local target="$1"
    local verbose="${2:-false}"

    log_info "修复glob模式问题..."

    # 获取构建错误
    local error_output=$(bazel build "$target" 2>&1 | grep "Error in glob:" || echo "")

    if [[ -z "$error_output" ]]; then
        log_success "未发现glob模式问题"
        return 0
    fi

    log_warn "发现glob模式问题，正在修复..."

    # 分析并修复glob错误
    while IFS= read -r line; do
        if [[ "$line" == *"Error in glob:"* ]]; then
            # 提取文件路径和错误模式
            local file_path=$(echo "$line" | sed 's|.*File "\([^"]*\)".*|\1|')
            local pattern=$(echo "$line" | sed 's/.*glob pattern '\''\([^'\'']*\)'\'' didn.*/\1/')

            if [[ -f "$file_path" ]]; then
                log_info "修复文件: $file_path (模式: $pattern)"

                # 对于不存在的文件模式，添加allow_empty=True
                if [[ "$pattern" == *"!"* ]]; then
                    # 排除模式，移除不存在的文件
                    sed -i "/$pattern/d" "$file_path"
                else
                    # 普通模式，添加allow_empty
                    sed -i 's|glob(\[|glob([|, s|glob(\[.*\])|glob([..., allow_empty = True)|' "$file_path"
                fi
            fi
        fi
    done <<< "$error_output"

    log_success "glob模式修复完成"
}

# 修复依赖问题
fix_dependency_issues() {
    local target="$1"
    local verbose="${2:-false}"

    log_info "修复依赖问题..."

    # 获取构建错误
    local error_output=$(bazel build "$target" 2>&1 | grep -E "(missing dependency|target.*not declared)" || echo "")

    if [[ -z "$error_output" ]]; then
        log_success "未发现依赖问题"
        return 0
    fi

    log_warn "发现依赖问题，正在分析..."

    # 这里可以实现更复杂的依赖修复逻辑
    # 目前只是报告问题
    if [[ "$verbose" == "true" ]]; then
        echo "依赖问题详情:"
        echo "$error_output"
    fi

    log_info "建议手动检查依赖关系"
}

# 自动修复所有问题
auto_fix_all() {
    local target="$1"

    log_info "开始自动修复..."

    # 按顺序执行修复
    format_build_files
    clean_cache
    fix_glob_patterns "$target" true
    fix_label_references "$target" true
    fix_dependency_issues "$target" true

    log_success "自动修复完成，建议重新构建验证"
}

# 主函数
main() {
    local format_files=false
    local clean_cache_flag=false
    local fix_labels=false
    local fix_globs=false
    local fix_deps=false
    local auto_fix=false
    local verbose=false
    local target="//..."

    # 参数解析
    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                show_help
                exit 0
                ;;
            -f|--format)
                format_files=true
                shift
                ;;
            -c|--clean)
                clean_cache_flag=true
                shift
                ;;
            -l|--label-fix)
                fix_labels=true
                shift
                ;;
            -g|--glob-fix)
                fix_globs=true
                shift
                ;;
            -d|--deps-fix)
                fix_deps=true
                shift
                ;;
            -a|--auto-fix)
                auto_fix=true
                shift
                ;;
            -v|--verbose)
                verbose=true
                shift
                ;;
            -*)
                log_error "未知选项: $1"
                show_help
                exit 1
                ;;
            *)
                target="$1"
                shift
                ;;
        esac
    done

    # 检查Bazel
    check_bazel

    # 执行操作
    if [[ "$auto_fix" == true ]]; then
        auto_fix_all "$target"
    else
        if [[ "$format_files" == true ]]; then
            format_build_files
        fi

        if [[ "$clean_cache_flag" == true ]]; then
            clean_cache
        fi

        if [[ "$fix_globs" == true ]]; then
            fix_glob_patterns "$target" "$verbose"
        fi

        if [[ "$fix_labels" == true ]]; then
            fix_label_references "$target" "$verbose"
        fi

        if [[ "$fix_deps" == true ]]; then
            fix_dependency_issues "$target" "$verbose"
        fi
    fi

    # 如果没有指定任何操作，显示帮助
    if [[ "$format_files" == false && "$clean_cache_flag" == false &&
          "$fix_labels" == false && "$fix_globs" == false &&
          "$fix_deps" == false && "$auto_fix" == false ]]; then
        show_help
    fi
}

# 执行主函数
main "$@"
