#!/bin/bash

# SQLCC 构建系统验证脚本
# 遵循构建原则：自动化验证 + 快速反馈

set -e  # 遇到错误立即退出

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

# 检查依赖工具
check_dependencies() {
    log_info "检查构建依赖..."

    local missing_tools=()

    if ! command -v clang-18 &> /dev/null; then
        missing_tools+=("clang-18")
    fi

    if ! command -v clang++-18 &> /dev/null; then
        missing_tools+=("clang++-18")
    fi

    if ! command -v bazel &> /dev/null; then
        missing_tools+=("bazel")
    fi

    if [ ${#missing_tools[@]} -ne 0 ]; then
        log_error "缺少必要的工具: ${missing_tools[*]}"
        log_error "请安装上述工具后再运行验证"
        exit 1
    fi

    log_success "所有构建依赖已就绪"
}

# 验证 Bazel 配置文件
validate_bazel_config() {
    log_info "验证 Bazel 配置..."

    # 检查 .bazelrc 文件
    if [ ! -f ".bazelrc" ]; then
        log_error ".bazelrc 文件不存在"
        exit 1
    fi

    log_success "Bazel 配置文件有效"
}

# 验证 BUILD 文件语法
validate_build_files() {
    log_info "验证 BUILD 文件语法..."

    # 使用 Bazel 查询验证语法
    if ! bazel query "deps(//src/...)" &> /dev/null; then
        log_error "BUILD 文件语法验证失败"
        exit 1
    fi

    log_success "BUILD 文件语法验证通过"
}

# 验证包结构
validate_package_structure() {
    log_info "验证包结构..."

    # 检查必需的包目录
    local required_packages=(
        "src/core"
        "src/logger"
        "src/utils"
        "src/sql_parser"
        "src/execution"
        "src/storage_engine"
        "src/network"
        "src/sql_executor"
        "include"
    )

    for package in "${required_packages[@]}"; do
        if [ ! -f "${package}/BUILD.bazel" ]; then
            log_error "包 ${package} 缺少 BUILD.bazel 文件"
            exit 1
        fi
    done

    log_success "包结构验证通过"
}

# 验证依赖关系
validate_dependencies() {
    log_info "验证依赖关系..."

    # 检查循环依赖
    if bazel query "deps(//src/...)" | grep -q "cycle"; then
        log_error "检测到循环依赖"
        exit 1
    fi

    log_success "依赖关系验证通过"
}

# 运行快速构建测试
run_quick_build_test() {
    log_info "运行快速构建测试..."

    # 构建核心组件
    if ! bazel build //src/core:core --config=modern; then
        log_error "核心组件构建失败"
        exit 1
    fi

    # 构建主应用
    if ! bazel build //:sqlcc --config=modern; then
        log_error "主应用构建失败"
        exit 1
    fi

    log_success "快速构建测试通过"
}

# 生成构建报告
generate_build_report() {
    log_info "生成构建报告..."

    local report_file="build_validation_report_$(date +%Y%m%d_%H%M%S).txt"

    {
        echo "SQLCC 构建系统验证报告"
        echo "生成时间: $(date)"
        echo "验证结果: 成功"
        echo ""
        echo "验证项:"
        echo "- 构建依赖检查 ✓"
        echo "- Bazel 配置验证 ✓"
        echo "- BUILD 文件语法 ✓"
        echo "- 包结构验证 ✓"
        echo "- 依赖关系验证 ✓"
        echo "- 快速构建测试 ✓"
        echo ""
        echo "构建环境信息:"
        echo "- Bazel 版本: $(bazel version 2>/dev/null | grep 'Build label' | cut -d' ' -f3)"
        echo "- Clang 版本: $(clang-18 --version | head -1)"
        echo "- 操作系统: $(uname -a)"
    } > "$report_file"

    log_success "构建报告已生成: $report_file"
}

# 主函数
main() {
    log_info "开始 SQLCC 构建系统验证..."

    check_dependencies
    validate_bazel_config
    validate_build_files
    validate_package_structure
    validate_dependencies
    run_quick_build_test
    generate_build_report

    log_success "🎉 SQLCC 构建系统验证完成！所有检查均通过"
    log_info "您现在可以使用以下命令构建项目:"
    log_info "  bazel build //:sqlcc --config=modern"
    log_info "  bazel test //tests/... --config=modern"
}

# 执行主函数
main "$@"
