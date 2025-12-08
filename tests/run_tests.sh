#!/bin/bash
# SQLCC v1.1.1 集成测试运行脚本
# 生成时间: 2025-12-08 04:53:16

set -e

# 配置参数
BUILD_DIR="${1:-./build}"
TEST_SUITE="${2:-full_suite}"
VERBOSE="${3:-false}"

echo "🚀 SQLCC v1.1.1 集成测试框架"
echo "📅 测试时间: $(date)"
echo "🏗️  构建目录: $BUILD_DIR"
echo "📋 测试套件: $TEST_SUITE"

# 检查构建目录
if [ ! -d "$BUILD_DIR" ]; then
    echo "❌ 构建目录不存在: $BUILD_DIR"
    echo "💡 请先运行: mkdir -p $BUILD_DIR && cd $BUILD_DIR && cmake .. && make"
    exit 1
fi

# 切换到构建目录
cd "$BUILD_DIR"

# 运行测试
if [ "$VERBOSE" = "true" ]; then
    echo "🔍 详细模式运行测试..."
    ctest -V -R "$TEST_SUITE"
else
    echo "⚡ 标准模式运行测试..."
    ctest -R "$TEST_SUITE" --output-on-failure
fi

echo "✅ 测试完成"