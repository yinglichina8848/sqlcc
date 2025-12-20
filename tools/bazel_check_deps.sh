#!/bin/bash
# tools/bazel_check_deps.sh - Bazel依赖检查工具
# 分析和验证包之间的依赖关系

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
Bazel依赖检查工具

用法:
    $0 [选项] [目标]

选项:
    -h, --help          显示帮助信息
    -v, --verbose       详细输出
    -c, --circular      检查循环依赖
    -m, --missing       检查缺失依赖
    -u, --unused        检查未使用的依赖
    -g, --graph         生成依赖图
    --format=<format>   输出格式 (text|dot|json)

目标:
    //...               检查所有目标
    //:sqlcc           检查主程序
    //src/...          检查源代码包
    //tests/...        检查测试

示例:
    $0 --circular //:sqlcc                # 检查主程序的循环依赖
    $0 --missing //src/...                # 检查源代码包的缺失依赖
    $0 --graph --format=dot //:sqlcc      # 生成主程序依赖图
    $0 --unused //tests/...               # 检查测试的未使用依赖

EOF
}

# 检查Bazel是否可用
check_bazel() {
    if ! command -v bazel &> /dev/null; then
        log_error "Bazel未安装或不在PATH中"
        exit 1
    fi
}

# 检查循环依赖
check_circular_deps() {
    local target="$1"
    local verbose="${2:-false}"

    log_info "检查循环依赖: $target"

    # 使用Bazel查询循环依赖
    local circular_deps=$(bazel query "deps($target)" --output=xml 2>/dev/null |
        grep -o 'name="[^"]*"' | sed 's/name="//;s/"//g' |
        sort | uniq -c | sort -nr | head -10)

    if [[ -n "$circular_deps" ]]; then
        echo "=== 可能的循环依赖 ==="
        echo "$circular_deps"

        # 更精确的循环依赖检测
        local cycles=$(bazel query "somepath($target, $target)" 2>/dev/null)
        if [[ -n "$cycles" ]]; then
            log_warn "发现循环依赖路径:"
            echo "$cycles"
        fi
    else
        log_success "未发现明显的循环依赖"
    fi
}

# 检查缺失依赖
check_missing_deps() {
    local target="$1"
    local verbose="${2:-false}"

    log_info "检查缺失依赖: $target"

    # 尝试构建并捕获错误
    local build_output=$(bazel build "$target" 2>&1)
    local missing_count=$(echo "$build_output" | grep -c "missing dependency" || echo 0)

    if [[ $missing_count -gt 0 ]]; then
        log_warn "发现 $missing_count 个缺失依赖"

        if [[ "$verbose" == "true" ]]; then
            echo "=== 缺失依赖详情 ==="
            echo "$build_output" | grep "missing dependency"
        fi
    else
        log_success "未发现缺失依赖"
    fi
}

# 检查未使用的依赖
check_unused_deps() {
    local target="$1"
    local verbose="${2:-false}"

    log_info "检查未使用的依赖: $target"

    # 获取目标的BUILD文件
    local build_file=$(bazel query "$target" --output=buildfiles 2>/dev/null | head -1)

    if [[ -f "$build_file" ]]; then
        log_info "分析BUILD文件: $build_file"

        # 提取依赖列表
        local deps=$(grep -o 'deps = \[.*\]' "$build_file" 2>/dev/null | sed 's/deps = //' | sed 's/\[//' | sed 's/\]//' | tr ',' '\n' | sed 's/^[[:space:]]*//' | sed 's/[[:space:]]*$//' | grep -v '^$' || echo "")

        if [[ -n "$deps" ]]; then
            echo "=== 声明的依赖 ==="
            echo "$deps"

            # 这里可以添加更复杂的未使用依赖检测逻辑
            # 目前只是列出依赖
        else
            log_info "目标没有声明依赖"
        fi
    else
        log_error "找不到BUILD文件: $build_file"
    fi
}

# 生成依赖图
generate_dep_graph() {
    local target="$1"
    local format="${2:-text}"

    log_info "生成依赖图: $target (格式: $format)"

    case $format in
        dot)
            echo "digraph deps {"
            bazel query "deps($target)" --output=graph 2>/dev/null | sed 's/-->/->/g'
            echo "}"
            ;;
        json)
            bazel query "deps($target)" --output=jsonproto 2>/dev/null
            ;;
        text|*)
            echo "=== 依赖层次结构 ==="
            bazel query "deps($target)" --output=label_kind 2>/dev/null
            ;;
    esac
}

# 分析依赖健康度
analyze_dep_health() {
    local target="$1"

    log_info "分析依赖健康度: $target"

    # 计算各种指标
    local total_deps=$(bazel query "deps($target)" --output=label 2>/dev/null | wc -l)
    local direct_deps=$(bazel query "deps($target, 1)" --output=label 2>/dev/null | wc -l)
    local test_deps=$(bazel query "deps($target)" --output=label 2>/dev/null | grep -c "_test" || echo 0)

    echo "=== 依赖统计 ==="
    echo "总依赖数: $total_deps"
    echo "直接依赖数: $direct_deps"
    echo "测试依赖数: $test_deps"

    # 计算依赖深度
    local depth=$(bazel query "maxrank($target)" 2>/dev/null | cut -d: -f2 | tr -d ' ' || echo "N/A")
    echo "依赖深度: $depth"

    # 依赖健康度评分
    local health_score=100

    if [[ $total_deps -gt 1000 ]]; then
        health_score=$((health_score - 20))
        log_warn "依赖数过多 (>1000)"
    elif [[ $total_deps -gt 500 ]]; then
        health_score=$((health_score - 10))
        log_warn "依赖数较多 (>500)"
    fi

    if [[ $depth != "N/A" && $depth -gt 10 ]]; then
        health_score=$((health_score - 15))
        log_warn "依赖深度过深 (>10)"
    fi

    echo "依赖健康度评分: $health_score/100"

    if [[ $health_score -ge 80 ]]; then
        log_success "依赖健康度良好"
    elif [[ $health_score -ge 60 ]]; then
        log_warn "依赖健康度一般"
    else
        log_error "依赖健康度较差"
    fi
}

# 主函数
main() {
    local verbose=false
    local check_circular=false
    local check_missing=false
    local check_unused=false
    local generate_graph=false
    local analyze_health=false
    local format="text"
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
            -c|--circular)
                check_circular=true
                shift
                ;;
            -m|--missing)
                check_missing=true
                shift
                ;;
            -u|--unused)
                check_unused=true
                shift
                ;;
            -g|--graph)
                generate_graph=true
                shift
                ;;
            --health)
                analyze_health=true
                shift
                ;;
            --format=*)
                format="${1#*=}"
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

    # 如果没有指定检查类型，默认进行全面检查
    if [[ "$check_circular" == false && "$check_missing" == false &&
          "$check_unused" == false && "$generate_graph" == false &&
          "$analyze_health" == false ]]; then
        check_circular=true
        check_missing=true
        analyze_health=true
    fi

    # 执行检查
    if [[ "$check_circular" == true ]]; then
        check_circular_deps "$target" "$verbose"
        echo
    fi

    if [[ "$check_missing" == true ]]; then
        check_missing_deps "$target" "$verbose"
        echo
    fi

    if [[ "$check_unused" == true ]]; then
        check_unused_deps "$target" "$verbose"
        echo
    fi

    if [[ "$generate_graph" == true ]]; then
        generate_dep_graph "$target" "$format"
        echo
    fi

    if [[ "$analyze_health" == true ]]; then
        analyze_dep_health "$target"
    fi
}

# 执行主函数
main "$@"
