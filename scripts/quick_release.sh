#!/bin/bash

# SQLCC 快速版本提交脚本
# 用于快速验证和提交版本，跳过完整的构建验证

set -e

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

log_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# 快速验证
quick_verify() {
    log_info "快速验证构建和测试..."
    
    # 使用现有的构建目录
    cd build-coverage
    
    # 运行核心测试 - 运行几个关键测试验证基本功能
    local test_executed=false
    for test_exe in buffer_pool_test page_test storage_engine_test; do
        if [ -f "bin/${test_exe}" ]; then
            log_info "运行 ${test_exe}..."
            if ! ./bin/${test_exe} --gtest_filter="*Basic*:*Simple*"; then
                log_error "${test_exe} 测试失败"
                return 1
            fi
            test_executed=true
            break
        fi
    done
    
    if [ "$test_executed" = false ]; then
        log_error "未找到任何测试可执行文件"
        return 1
    fi
    
    cd ..
    log_info "快速验证通过"
    return 0
}

# 更新版本号
update_version() {
    local version=$1
    
    log_info "更新版本号为 $version"
    
    # 更新VERSION文件
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
}

# 主函数
main() {
    if [ $# -lt 2 ]; then
        echo "用法: $0 <版本号> <提交信息>"
        echo "示例: $0 0.3.4 \"修复测试编译错误\""
        exit 1
    fi
    
    local version=$1
    local commit_message=$2
    
    log_info "开始快速版本提交流程..."
    
    # 快速验证
    if ! quick_verify; then
        log_error "快速验证失败"
        exit 1
    fi
    
    # 更新版本号
    update_version "$version"
    
    # 提交到主分支
    git add VERSION include/version.h
    git commit -m "$commit_message"
    git push origin master
    
    # 生成文档并提交到docs分支
    git checkout docs
    make docs
    make coverage
    git add docs/coverage/ docs/doxygen/ 2>/dev/null || true
    git commit -m "docs: $commit_message"
    git push origin docs
    
    # 切换回主分支
    git checkout master
    
    log_info "快速版本提交完成！"
    log_info "版本 $version 已发布"
}

if [ "${BASH_SOURCE[0]}" == "${0}" ]; then
    main "$@"
fi