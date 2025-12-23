#!/bin/bash

# SQLCC 自动化构建验证系统
# 用于验证整个项目的构建状态和质量

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

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# 全局变量
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_TYPE="${1:-fastbuild}"
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
REPORT_DIR="${PROJECT_ROOT}/build_reports"
BUILD_REPORT="${REPORT_DIR}/build_validation_${TIMESTAMP}.md"
BAZEL_OUTPUT_DIR="${PROJECT_ROOT}/bazel-bin"

# 创建报告目录
mkdir -p "${REPORT_DIR}"

# 初始化报告
init_report() {
    cat > "${BUILD_REPORT}" << EOF
# SQLCC 构建验证报告
**生成时间:** $(date)
**构建类型:** ${BUILD_TYPE}
**项目根目录:** ${PROJECT_ROOT}

## 构建环境信息
EOF

    # 添加环境信息
    {
        echo "### 系统信息"
        echo "- 操作系统: $(uname -s)"
        echo "- 内核版本: $(uname -r)"
        echo "- 架构: $(uname -m)"
        echo ""

        echo "### 编译器信息"
        if command -v clang++-18 &> /dev/null; then
            echo "- Clang++ 18: $(clang++-18 --version | head -n 1)"
        fi
        if command -v g++ &> /dev/null; then
            echo "- G++: $(g++ --version | head -n 1)"
        fi
        echo ""

        echo "### 构建工具"
        echo "- Bazel: $(bazel version 2>/dev/null | grep "Build label" | cut -d: -f2 | tr -d ' ' || echo "Not found")"
        echo "- Make: $(make --version 2>/dev/null | head -n 1 || echo "Not found")"
        echo ""

        echo "## 构建结果"
        echo ""
    } >> "${BUILD_REPORT}"
}

# 验证依赖关系
validate_dependencies() {
    log_info "验证项目依赖关系..."

    echo "### 依赖验证" >> "${BUILD_REPORT}"

    # 检查必要的工具
    local missing_tools=()

    if ! command -v bazel &> /dev/null; then
        missing_tools+=("bazel")
    fi

    if ! command -v clang++-18 &> /dev/null && ! command -v g++ &> /dev/null; then
        missing_tools+=("clang++-18 or g++")
    fi

    if ! command -v python3 &> /dev/null; then
        missing_tools+=("python3")
    fi

    if [ ${#missing_tools[@]} -ne 0 ]; then
        log_error "缺少必要的工具: ${missing_tools[*]}"
        echo "- ❌ 缺少工具: ${missing_tools[*]}" >> "${BUILD_REPORT}"
        return 1
    else
        log_success "所有必要的工具都已安装"
        echo "- ✅ 所有必要的工具都已安装" >> "${BUILD_REPORT}"
    fi

    echo "" >> "${BUILD_REPORT}"
    return 0
}

# 验证项目结构
validate_project_structure() {
    log_info "验证项目结构..."

    echo "### 项目结构验证" >> "${BUILD_REPORT}"

    local required_files=(
        "WORKSPACE"
        "BUILD.bazel"
        ".bazelrc"
        "include/BUILD.bazel"
        "src/BUILD.bazel"
        "tests/BUILD.bazel"
    )

    local missing_files=()

    for file in "${required_files[@]}"; do
        if [[ ! -f "${PROJECT_ROOT}/${file}" ]]; then
            missing_files+=("${file}")
        fi
    done

    if [ ${#missing_files[@]} -ne 0 ]; then
        log_error "缺少必要的项目文件: ${missing_files[*]}"
        echo "- ❌ 缺少文件: ${missing_files[*]}" >> "${BUILD_REPORT}"
        return 1
    else
        log_success "项目结构完整"
        echo "- ✅ 项目结构完整" >> "${BUILD_REPORT}"
    fi

    # 检查主要目录
    local required_dirs=(
        "include"
        "src"
        "tests"
        "tools"
        "scripts"
        "docs"
    )

    local missing_dirs=()

    for dir in "${required_dirs[@]}"; do
        if [[ ! -d "${PROJECT_ROOT}/${dir}" ]]; then
            missing_dirs+=("${dir}")
        fi
    done

    if [ ${#missing_dirs[@]} -ne 0 ]; then
        log_warning "缺少目录: ${missing_dirs[*]}"
        echo "- ⚠️  缺少目录: ${missing_dirs[*]}" >> "${BUILD_REPORT}"
    else
        echo "- ✅ 所有主要目录存在" >> "${BUILD_REPORT}"
    fi

    echo "" >> "${BUILD_REPORT}"
    return 0
}

# 验证Bazel配置
validate_bazel_config() {
    log_info "验证Bazel配置..."

    echo "### Bazel配置验证" >> "${BUILD_REPORT}"

    # 清理之前的构建
    if [[ -d "${PROJECT_ROOT}/bazel-out" ]]; then
        log_info "清理旧的构建输出..."
        rm -rf "${PROJECT_ROOT}/bazel-out" "${PROJECT_ROOT}/bazel-bin" "${PROJECT_ROOT}/bazel-testlogs"
    fi

    # 验证WORKSPACE文件
    if ! bazel query //... --keep_going &> /dev/null; then
        log_error "Bazel WORKSPACE配置无效"
        echo "- ❌ WORKSPACE配置无效" >> "${BUILD_REPORT}"
        return 1
    else
        echo "- ✅ WORKSPACE配置有效" >> "${BUILD_REPORT}"
    fi

    echo "" >> "${BUILD_REPORT}"
    return 0
}

# 执行构建测试
run_build_tests() {
    log_info "执行构建测试..."

    echo "### 构建测试结果" >> "${BUILD_REPORT}"

    local build_success=true
    local test_targets=(
        "//include:sqlcc_pch"
        "//src/core:sql_executor"
        "//src/core:database_manager"
        "//src/core:user_manager"
        "//src/core:error_handler"
        "//src/core:permission_validator"
    )

    for target in "${test_targets[@]}"; do
        log_info "构建目标: ${target}"
        echo "#### 构建 ${target}" >> "${BUILD_REPORT}"

        if bazel build "${target}" --config="${BUILD_TYPE}" &> /dev/null; then
            log_success "✅ ${target} 构建成功"
            echo "- ✅ 构建成功" >> "${BUILD_REPORT}"
        else
            log_error "❌ ${target} 构建失败"
            echo "- ❌ 构建失败" >> "${BUILD_REPORT}"
            build_success=false
        fi

        echo "" >> "${BUILD_REPORT}"
    done

    if [[ "${build_success}" == "true" ]]; then
        log_success "所有构建测试通过"
        echo "## 总结" >> "${BUILD_REPORT}"
        echo "- ✅ 所有构建测试通过" >> "${BUILD_REPORT}"
        return 0
    else
        log_error "部分构建测试失败"
        echo "## 总结" >> "${BUILD_REPORT}"
        echo "- ❌ 部分构建测试失败" >> "${BUILD_REPORT}"
        return 1
    fi
}

# 生成构建报告
generate_build_report() {
    log_info "生成构建报告..."

    echo "" >> "${BUILD_REPORT}"
    echo "## 构建统计" >> "${BUILD_REPORT}"

    # 添加构建统计信息
    if [[ -d "${BAZEL_OUTPUT_DIR}" ]]; then
        local binary_count=$(find "${BAZEL_OUTPUT_DIR}" -name "*.so" -o -name "*.a" 2>/dev/null | wc -l)
        echo "- 生成的库文件数量: ${binary_count}" >> "${BUILD_REPORT}"
    fi

    echo "- 构建类型: ${BUILD_TYPE}" >> "${BUILD_REPORT}"
    echo "- 构建时间: $(date)" >> "${BUILD_REPORT}"
    echo "- 报告位置: ${BUILD_REPORT}" >> "${BUILD_REPORT}"

    log_success "构建报告已生成: ${BUILD_REPORT}"
}

# 主函数
main() {
    log_info "开始SQLCC构建验证..."
    log_info "项目根目录: ${PROJECT_ROOT}"
    log_info "构建类型: ${BUILD_TYPE}"

    init_report

    local validation_passed=true

    # 执行验证步骤
    if ! validate_dependencies; then
        validation_passed=false
    fi

    if ! validate_project_structure; then
        validation_passed=false
    fi

    if ! validate_bazel_config; then
        validation_passed=false
    fi

    if ! run_build_tests; then
        validation_passed=false
    fi

    generate_build_report

    if [[ "${validation_passed}" == "true" ]]; then
        log_success "🎉 SQLCC构建验证通过！"
        echo "- 🎉 总体结果: 通过" >> "${BUILD_REPORT}"
        exit 0
    else
        log_error "❌ SQLCC构建验证失败！"
        echo "- ❌ 总体结果: 失败" >> "${BUILD_REPORT}"
        echo "" >> "${BUILD_REPORT}"
        echo "## 故障排除建议" >> "${BUILD_REPORT}"
        echo "1. 检查编译器版本是否正确" >> "${BUILD_REPORT}"
        echo "2. 确保所有依赖都已安装" >> "${BUILD_REPORT}"
        echo "3. 查看详细的编译错误信息" >> "${BUILD_REPORT}"
        echo "4. 运行 'bazel clean' 清理构建缓存" >> "${BUILD_REPORT}"
        exit 1
    fi
}

# 执行主函数
main "$@"
