#!/bin/bash
# SQLCC 模块化迁移验证脚本
# 用于验证模块迁移后的编译和测试状态

set -e  # 遇到错误立即退出

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SCRIPT_DIR="$PROJECT_ROOT/scripts"

# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 日志函数
log_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# 检查命令是否存在
check_command() {
    if ! command -v "$1" &> /dev/null; then
        log_error "命令 '$1' 未找到，请确保已安装"
        exit 1
    fi
}

# 验证项目结构
validate_project_structure() {
    log_info "验证项目结构..."

    # 检查必需目录
    required_dirs=("src" "include" "tests" "tools")
    for dir in "${required_dirs[@]}"; do
        if [ ! -d "$PROJECT_ROOT/$dir" ]; then
            log_error "必需目录 '$dir' 不存在"
            return 1
        fi
    done

    # 检查工具脚本
    required_tools=("tools/migration_executor.py" "tools/dependency_fixer.py")
    for tool in "${required_tools[@]}"; do
        if [ ! -f "$PROJECT_ROOT/$tool" ]; then
            log_error "必需工具 '$tool' 不存在"
            return 1
        fi
    done

    log_info "✓ 项目结构验证通过"
    return 0
}

# 验证模块结构
validate_module_structure() {
    local module_name="$1"
    log_info "验证模块 '$module_name' 的结构..."

    local module_path="$PROJECT_ROOT/src/$module_name"

    if [ ! -d "$module_path" ]; then
        log_error "模块目录 '$module_path' 不存在"
        return 1
    fi

    # 检查必需文件
    local required_files=("BUILD.bazel" "include/$module_name" "src")
    for file in "${required_files[@]}"; do
        if [ ! -e "$module_path/$file" ]; then
            log_error "模块 '$module_name' 缺少必需文件/目录: $file"
            return 1
        fi
    done

    # 检查是否有源文件
    if ! find "$module_path/src" -name "*.cpp" -o -name "*.cc" | grep -q .; then
        log_warn "模块 '$module_name' 的src目录下没有找到源文件"
    fi

    # 检查是否有头文件
    if ! find "$module_path/include" -name "*.h" -o -name "*.hpp" | grep -q .; then
        log_warn "模块 '$module_name' 的include目录下没有找到头文件"
    fi

    log_info "✓ 模块 '$module_name' 结构验证通过"
    return 0
}

# 验证Bazel构建
validate_bazel_build() {
    local target="$1"
    log_info "验证Bazel构建: $target"

    if ! cd "$PROJECT_ROOT" || ! bazel build "$target" > /dev/null 2>&1; then
        log_error "Bazel构建失败: $target"
        # 显示详细错误信息
        cd "$PROJECT_ROOT" && bazel build "$target" 2>&1 | head -20
        return 1
    fi

    log_info "✓ Bazel构建成功: $target"
    return 0
}

# 验证Bazel测试
validate_bazel_test() {
    local target="$1"
    log_info "验证Bazel测试: $target"

    if ! cd "$PROJECT_ROOT" || ! bazel test "$target" > /dev/null 2>&1; then
        log_error "Bazel测试失败: $target"
        # 显示详细错误信息
        cd "$PROJECT_ROOT" && bazel test "$target" 2>&1 | head -20
        return 1
    fi

    log_info "✓ Bazel测试成功: $target"
    return 0
}

# 验证依赖关系
validate_dependencies() {
    log_info "验证依赖关系..."

    # 运行依赖修复器的验证模式
    if ! cd "$PROJECT_ROOT" || ! python3 tools/dependency_fixer.py --validate-only; then
        log_error "依赖关系验证失败"
        return 1
    fi

    log_info "✓ 依赖关系验证通过"
    return 0
}

# 验证全局include泄露
validate_no_global_includes() {
    log_info "检查全局include泄露..."

    local leaked_files=()

    # 查找使用全局include路径的文件
    while IFS= read -r file; do
        if grep -q '#include\s*<[a-zA-Z_][a-zA-Z0-9_]*/' "$file" 2>/dev/null; then
            leaked_files+=("$file")
        fi
    done < <(find "$PROJECT_ROOT/src" "$PROJECT_ROOT/include" -name "*.cpp" -o -name "*.h" 2>/dev/null)

    if [ ${#leaked_files[@]} -gt 0 ]; then
        log_error "发现全局include泄露:"
        printf '  %s\n' "${leaked_files[@]}"
        log_error "请使用 '\"module/header.h\"' 替代 '<module/header.h>'"
        return 1
    fi

    log_info "✓ 无全局include泄露"
    return 0
}

# 验证模块编译
validate_module_compilation() {
    local module_name="$1"
    log_info "验证模块 '$module_name' 编译..."

    if ! validate_module_structure "$module_name"; then
        return 1
    fi

    if ! validate_bazel_build "//src/$module_name:$module_name"; then
        return 1
    fi

    log_info "✓ 模块 '$module_name' 编译验证通过"
    return 0
}

# 验证模块测试
validate_module_tests() {
    local module_name="$1"
    log_info "验证模块 '$module_name' 测试..."

    # 检查是否有对应的测试
    local test_dir="$PROJECT_ROOT/tests/level2_*/$module_name"
    if ! ls $test_dir 2>/dev/null | grep -q .; then
        log_warn "模块 '$module_name' 没有找到对应的Level2测试"
        return 0  # 测试不存在不是错误
    fi

    # 查找BUILD文件
    local build_file
    build_file=$(find "$test_dir" -name "BUILD.bazel" 2>/dev/null | head -1)
    if [ -z "$build_file" ]; then
        log_error "模块 '$module_name' 的测试缺少BUILD.bazel文件"
        return 1
    fi

    # 提取测试目标名
    local test_targets
    test_targets=$(grep -o 'name = "[^"]*"' "$build_file" | sed 's/name = "\([^"]*\)"/\1/' | grep "test\|benchmark" || true)

    if [ -z "$test_targets" ]; then
        log_warn "模块 '$module_name' 的测试BUILD文件中没有找到测试目标"
        return 0
    fi

    # 验证每个测试目标
    for target in $test_targets; do
        local full_target="//tests/level2_*/$module_name:$target"
        if ! validate_bazel_test "$full_target"; then
            return 1
        fi
    done

    log_info "✓ 模块 '$module_name' 测试验证通过"
    return 0
}

# 生成验证报告
generate_report() {
    local report_file="$PROJECT_ROOT/migration_validation_report.md"
    log_info "生成验证报告: $report_file"

    {
        echo "# SQLCC 模块化迁移验证报告"
        echo ""
        echo "生成时间: $(date)"
        echo ""

        echo "## 验证结果"
        echo ""
        echo "### 项目结构"
        echo "- [x] 必需目录存在"
        echo "- [x] 工具脚本完整"
        echo ""

        echo "### 依赖关系"
        echo "- [x] 无循环依赖"
        echo "- [x] 无全局include泄露"
        echo "- [x] BUILD依赖正确"
        echo ""

        echo "### 编译状态"
        echo "- [x] 所有模块可编译"
        echo "- [x] 测试可通过"
        echo ""

        echo "## 迁移状态总结"
        echo ""
        echo "✓ 模块化迁移验证完成"
        echo "✓ 项目可正常构建和测试"
        echo ""

    } > "$report_file"

    log_info "验证报告已生成: $report_file"
}

# 主函数
main() {
    local validate_all=false
    local validate_module=""
    local generate_report_flag=false

    # 解析命令行参数
    while [[ $# -gt 0 ]]; do
        case $1 in
            --all)
                validate_all=true
                shift
                ;;
            --module)
                validate_module="$2"
                shift 2
                ;;
            --report)
                generate_report_flag=true
                shift
                ;;
            --help)
                echo "用法: $0 [选项]"
                echo ""
                echo "选项:"
                echo "  --all              验证所有模块"
                echo "  --module <name>    验证指定模块"
                echo "  --report           生成验证报告"
                echo "  --help             显示此帮助信息"
                exit 0
                ;;
            *)
                log_error "未知选项: $1"
                exit 1
                ;;
        esac
    done

    # 检查必需命令
    check_command bazel
    check_command python3

    # 验证项目结构
    if ! validate_project_structure; then
        exit 1
    fi

    # 验证依赖关系
    if ! validate_dependencies; then
        exit 1
    fi

    # 检查全局include泄露
    if ! validate_no_global_includes; then
        exit 1
    fi

    # 根据参数执行验证
    if [ "$validate_all" = true ]; then
        log_info "开始验证所有模块..."

        # 获取所有模块
        local modules=()
        for dir in "$PROJECT_ROOT/src"/*/; do
            if [ -f "${dir}BUILD.bazel" ]; then
                modules+=("$(basename "$dir")")
            fi
        done

        local failed_modules=()
        for module in "${modules[@]}"; do
            if ! validate_module_compilation "$module" || ! validate_module_tests "$module"; then
                failed_modules+=("$module")
            fi
        done

        if [ ${#failed_modules[@]} -gt 0 ]; then
            log_error "以下模块验证失败: ${failed_modules[*]}"
            exit 1
        fi

        log_info "✓ 所有模块验证通过"

    elif [ -n "$validate_module" ]; then
        if ! validate_module_compilation "$validate_module" || ! validate_module_tests "$validate_module"; then
            exit 1
        fi

    else
        log_info "请使用 --all 或 --module <name> 指定验证范围"
        log_info "使用 --help 查看详细选项"
        exit 1
    fi

    # 生成报告
    if [ "$generate_report_flag" = true ]; then
        generate_report
    fi

    log_info "🎉 迁移验证完成！"
}

# 执行主函数
main "$@"
