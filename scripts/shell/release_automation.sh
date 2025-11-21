#!/bin/bash

# SQLCC 自动化版本提交脚本
# 该脚本确保在提交源码前完成编译、单元测试和代码覆盖率测试

set -e  # 遇到错误立即退出

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 日志函数
log_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

# 检查命令是否存在
check_command() {
    if ! command -v $1 &> /dev/null; then
        log_error "$1 命令未找到，请先安装"
        exit 1
    fi
}

# 验证构建
verify_build() {
    log_info "开始验证构建..."
    
    # 清理之前的构建
    if [ -d "build" ]; then
        rm -rf build
    fi
    
    # 创建构建目录
    mkdir -p build
    cd build
    
    # 配置CMake
    if ! cmake ..; then
        log_error "CMake 配置失败"
        return 1
    fi
    
    # 编译
    if ! make -j$(nproc); then
        log_error "编译失败"
        return 1
    fi
    
    cd ..
    log_info "构建验证成功"
    return 0
}

# 运行单元测试
run_unit_tests() {
    log_info "开始运行单元测试..."
    
    cd build
    
    # 运行所有测试
    if ! ctest --output-on-failure; then
        log_error "单元测试失败"
        return 1
    fi
    
    cd ..
    log_info "单元测试通过"
    return 0
}

# 生成代码覆盖率报告
generate_coverage() {
    log_info "开始生成代码覆盖率报告..."
    
    # 清理并重新生成覆盖率报告
    make clean-coverage 2>/dev/null || true
    
    if ! make coverage; then
        log_error "代码覆盖率生成失败"
        return 1
    fi
    
    log_info "代码覆盖率报告生成成功"
    return 0
}

# 提交源码到主分支
commit_source_code() {
    local version=$1
    local commit_message=$2
    
    log_info "准备提交源码到主分支..."
    
    # 确保在主分支
    current_branch=$(git rev-parse --abbrev-ref HEAD)
    if [ "$current_branch" != "master" ]; then
        git checkout master
    fi
    
    # 检查是否有未提交的更改
    if ! git diff-index --quiet HEAD --; then
        log_error "存在未提交的更改，请先处理"
        return 1
    fi
    
    # 更新版本文件
    echo "v$version" > VERSION
    
    # 更新版本头文件
    cat > include/version.h << EOF
/**
 * @file version.h
 * @brief SQLCC项目版本信息定义
 * 
 * 定义了SQLCC数据库系统的版本号，用于版本管理和发布跟踪。
 * 版本号遵循语义化版本控制规范（SemVer）。
 */

#pragma once

/** @brief SQLCC项目版本号 */
#define SQLCC_VERSION "$version"
EOF
    
    # 添加到git
    git add VERSION include/version.h
    
    # 提交
    if ! git commit -m "$commit_message"; then
        log_error "提交失败"
        return 1
    fi
    
    # 推送
    if ! git push origin master; then
        log_error "推送失败"
        return 1
    fi
    
    log_info "源码提交成功"
    return 0
}

# 提交文档到docs分支
commit_docs() {
    local version=$1
    local commit_message=$2
    
    log_info "准备提交文档到docs分支..."
    
    # 切换到docs分支
    git checkout docs
    
    # 生成文档
    if ! make docs; then
        log_error "文档生成失败"
        return 1
    fi
    
    # 生成覆盖率报告
    if ! make coverage; then
        log_error "覆盖率报告生成失败"
        return 1
    fi
    
    # 添加文档文件
    git add docs/coverage/ docs/doxygen/ 2>/dev/null || true
    git add *.html *.css *.info 2>/dev/null || true
    
    # 提交
    if ! git commit -m "$commit_message"; then
        log_error "文档提交失败"
        return 1
    fi
    
    # 推送
    if ! git push origin docs; then
        log_error "文档推送失败"
        return 1
    fi
    
    log_info "文档提交成功"
    return 0
}

# 主函数
main() {
    # 检查参数
    if [ $# -lt 2 ]; then
        echo "用法: $0 <版本号> <提交信息>"
        echo "示例: $0 0.3.4 \"发布版本0.3.4：修复测试编译错误\""
        exit 1
    fi
    
    local version=$1
    local commit_message=$2
    
    log_info "开始自动化版本提交流程..."
    log_info "版本号: $version"
    log_info "提交信息: $commit_message"
    
    # 检查必要命令
    check_command git
    check_command cmake
    check_command make
    check_command ctest
    
    # 验证构建
    if ! verify_build; then
        log_error "构建验证失败，中止提交"
        exit 1
    fi
    
    # 运行单元测试
    if ! run_unit_tests; then
        log_error "单元测试失败，中止提交"
        exit 1
    fi
    
    # 生成代码覆盖率
    if ! generate_coverage; then
        log_error "代码覆盖率生成失败，中止提交"
        exit 1
    fi
    
    # 提交源码到主分支
    if ! commit_source_code "$version" "$commit_message"; then
        log_error "源码提交失败"
        exit 1
    fi
    
    # 提交文档到docs分支
    if ! commit_docs "$version" "docs: $commit_message"; then
        log_error "文档提交失败"
        exit 1
    fi
    
    log_info "自动化版本提交流程完成！"
    log_info "版本 $version 已成功发布"
    log_info "源码已提交到主分支，文档已提交到docs分支"
}

# 如果直接运行脚本
if [ "${BASH_SOURCE[0]}" == "${0}" ]; then
    main "$@"
fi