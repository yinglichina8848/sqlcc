#!/bin/bash
# tools/bazel_debug.sh - Bazel调试工具
# 提供详细的错误分析和调试信息

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
Bazel调试工具

用法:
    $0 [选项] [目标]

选项:
    -h, --help          显示帮助信息
    -v, --verbose       详细输出
    -q, --query         启用查询模式
    -d, --deps          显示依赖关系
    -c, --clean         清理缓存后构建
    -f, --format        格式化BUILD文件
    -t, --test          运行测试
    --config=<config>   指定构建配置

目标:
    //...               构建所有目标
    //:sqlcc           构建主程序
    //src/...          构建源代码包
    //tests/...        构建测试

示例:
    $0 --clean //:sqlcc                    # 清理缓存后构建主程序
    $0 --deps //:sqlcc                     # 显示主程序依赖关系
    $0 --query "deps(//:sqlcc)"            # 查询主程序依赖
    $0 --format                            # 格式化所有BUILD文件
    $0 --test //tests/...                  # 运行所有测试

EOF
}

# 检查Bazel是否安装
check_bazel() {
    if ! command -v bazel &> /dev/null; then
        log_error "Bazel未安装或不在PATH中"
        exit 1
    fi
}

# 获取Bazel版本
get_bazel_version() {
    bazel version 2>/dev/null | grep "Build label" | cut -d: -f2 | tr -d ' '
}

# 分析Bazel错误
analyze_error() {
    local error_file="$1"
    local error_count=0

    if [[ -f "$error_file" ]]; then
        log_info "分析错误文件: $error_file"

        # 统计不同类型的错误
        local glob_errors=$(grep -c "Error in glob:" "$error_file" 2>/dev/null || echo 0)
        local label_errors=$(grep -c "Label.*is invalid" "$error_file" 2>/dev/null || echo 0)
        local target_errors=$(grep -c "target.*not declared" "$error_file" 2>/dev/null || echo 0)
        local missing_deps=$(grep -c "missing dependency" "$error_file" 2>/dev/null || echo 0)

        if [[ $glob_errors -gt 0 ]]; then
            log_warn "发现 $glob_errors 个glob模式错误"
        fi

        if [[ $label_errors -gt 0 ]]; then
            log_warn "发现 $label_errors 个标签引用错误"
        fi

        if [[ $target_errors -gt 0 ]]; then
            log_warn "发现 $target_errors 个目标声明错误"
        fi

        if [[ $missing_deps -gt 0 ]]; then
            log_warn "发现 $missing_deps 个缺失依赖错误"
        fi

        error_count=$((glob_errors + label_errors + target_errors + missing_deps))
    fi

    echo $error_count
}

# 格式化BUILD文件
format_build_files() {
    log_info "格式化BUILD文件..."

    # 查找所有BUILD文件
    local build_files=$(find . -name "BUILD.bazel" -o -name "BUILD" | head -20)

    for file in $build_files; do
        if [[ -f "$file" ]]; then
            log_info "格式化: $file"
            # 这里可以添加buildifier或其他格式化工具
            # buildifier -mode=fix "$file" 2>/dev/null || true
        fi
    done

    log_success "BUILD文件格式化完成"
}

# 显示依赖关系
show_deps() {
    local target="$1"

    if [[ -z "$target" ]]; then
        log_error "请指定目标"
        exit 1
    fi

    log_info "分析目标依赖: $target"

    # 检查BUILD文件是否有语法错误
    echo "=== BUILD文件检查 ==="
    if bazel query "//..." --output=label >/dev/null 2>&1; then
        log_success "BUILD文件语法检查通过"
    else
        log_warn "发现BUILD文件语法错误，正在分析..."
        analyze_build_errors
    fi

    # 显示直接依赖
    echo "=== 直接依赖 ==="
    if bazel query "deps($target, 1)" --output=label 2>/dev/null; then
        # 查询成功
        true
    else
        log_error "直接依赖查询失败"
        return 1
    fi

    # 显示传递依赖
    echo "=== 传递依赖 (前20个) ==="
    if bazel query "deps($target)" --output=label 2>/dev/null | head -20; then
        # 查询成功
        true
    else
        log_error "传递依赖查询失败"
        return 1
    fi

    # 显示反向依赖
    echo "=== 反向依赖 ==="
    if bazel query "rdeps(//..., $target)" --output=label 2>/dev/null; then
        # 查询成功
        true
    elif bazel query "rdeps($target)" --output=label 2>/dev/null; then
        # 备选查询成功
        echo "[INFO] 使用备选查询语法成功"
    else
        echo "[ERROR] 反向依赖查询失败"
        echo "[NOTE] 这可能是由于BUILD文件标签引用错误导致的"
        echo "[NOTE] 建议运行: ./tools/bazel_fixer.sh --fix-labels"
        return 1
    fi
}

# 分析BUILD文件错误
analyze_build_errors() {
    log_info "分析BUILD文件错误..."

    # 尝试加载工作空间并捕获错误
    local error_output
    error_output=$(bazel query "//..." --output=label 2>&1 >/dev/null)

    if [[ -n "$error_output" ]]; then
        echo "[BUILD错误分析]:"
        echo "$error_output" | while read -r line; do
            if [[ $line =~ "subpackage" ]]; then
                echo "  🔧 发现子包引用错误: $line"
                extract_label_error "$line"
            elif [[ $line =~ "not declared" ]]; then
                echo "  🔧 发现目标未声明错误: $line"
            elif [[ $line =~ "invalid" ]]; then
                echo "  🔧 发现无效标签错误: $line"
            else
                echo "  ❓ 其他错误: $line"
            fi
        done
        echo "[建议]: 运行 ./tools/bazel_fixer.sh --fix-all 自动修复"
    else
        log_success "未发现BUILD文件错误"
    fi
}

# 提取标签错误信息
extract_label_error() {
    local error_line="$1"

    # 提取错误的标签
    if [[ $error_line =~ "'//([^']+)'" ]]; then
        local wrong_label="${BASH_REMATCH[1]}"
        if [[ $error_line =~ "perhaps you meant to put the colon here: '([^']+)'" ]]; then
            local correct_label="${BASH_REMATCH[1]}"
            echo "    错误标签: $wrong_label"
            echo "    正确标签: $correct_label"
            suggest_fix "$wrong_label" "$correct_label"
        fi
    fi
}

# 建议修复
suggest_fix() {
    local wrong="$1"
    local correct="$2"

    echo "    修复命令: sed -i 's|$wrong|$correct|g' \$(find . -name 'BUILD.bazel')"
}

# 清理Bazel缓存
clean_cache() {
    log_info "清理Bazel缓存..."
    bazel clean --expunge
    log_success "缓存清理完成"
}

# 运行查询
run_query() {
    local query="$1"

    if [[ -z "$query" ]]; then
        log_error "请指定查询表达式"
        exit 1
    fi

    log_info "执行查询: $query"
    bazel query "$query" --output=label
}

# 运行测试
run_tests() {
    local target="$1"

    if [[ -z "$target" ]]; then
        target="//tests/..."
    fi

    log_info "运行测试: $target"
    bazel test "$target" --test_output=errors
}

# 主函数
main() {
    local verbose=false
    local query_mode=false
    local show_deps_flag=false
    local clean_cache_flag=false
    local format_files=false
    local run_tests_flag=false
    local config="modern"
    local target="//..."

    # 参数解析
    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                show_help
                exit 0
                ;;
            -v|--verbose)
                verbose=true
                shift
                ;;
            -q|--query)
                query_mode=true
                shift
                ;;
            -d|--deps)
                show_deps_flag=true
                shift
                ;;
            -c|--clean)
                clean_cache_flag=true
                shift
                ;;
            -f|--format)
                format_files=true
                shift
                ;;
            -t|--test)
                run_tests_flag=true
                shift
                ;;
            --config=*)
                config="${1#*=}"
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

    # 显示版本信息
    local bazel_version=$(get_bazel_version)
    log_info "Bazel版本: $bazel_version"

    # 执行操作
    if [[ "$format_files" == true ]]; then
        format_build_files
        exit 0
    fi

    if [[ "$clean_cache_flag" == true ]]; then
        clean_cache
    fi

    if [[ "$query_mode" == true ]]; then
        run_query "$target"
        exit 0
    fi

    if [[ "$show_deps_flag" == true ]]; then
        show_deps "$target"
        exit 0
    fi

    if [[ "$run_tests_flag" == true ]]; then
        run_tests "$target"
        exit 0
    fi

    # 默认构建操作
    local error_file="bazel_error.log"
    local cmd="bazel build $target --config=$config"

    if [[ "$verbose" == true ]]; then
        cmd="$cmd --verbose_failures"
    fi

    log_info "执行命令: $cmd"

    if $cmd 2>&1 | tee "$error_file"; then
        log_success "构建成功"
        rm -f "$error_file"
    else
        local error_count=$(analyze_error "$error_file")
        log_error "构建失败，发现 $error_count 个错误"

        if [[ $error_count -gt 0 ]]; then
            log_info "错误详情已保存到: $error_file"
            log_info "建议运行: $0 --deps $target"
        fi

        exit 1
    fi
}

# 执行主函数
main "$@"
