#!/bin/bash

# SQLCC 编译时间基准测试脚本
# 用于测量预编译头文件优化效果

set -e

echo "🔬 SQLCC 编译时间基准测试"
echo "================================"

# 测试参数
TARGET_MODULE="src/core:core"
ITERATIONS=3
OUTPUT_FILE="benchmark_results_$(date +%Y%m%d_%H%M%S).txt"

# 清理之前的构建
echo "🧹 清理之前的构建..."
bazel clean --expunge

echo "⏱️  开始基准测试..."
echo "目标模块: $TARGET_MODULE"
echo "测试轮数: $ITERATIONS"
echo ""

# 记录结果
{
    echo "SQLCC 编译时间基准测试结果"
    echo "=============================="
    echo "测试时间: $(date)"
    echo "目标模块: $TARGET_MODULE"
    echo "测试轮数: $ITERATIONS"
    echo ""

    total_time=0

    for i in $(seq 1 $ITERATIONS); do
        echo "📊 第 $i 轮测试开始..."

        # 记录开始时间
        start_time=$(date +%s.%3N)

        # 执行编译
        if bazel build $TARGET_MODULE >/dev/null 2>&1; then
            # 记录结束时间
            end_time=$(date +%s.%3N)

            # 计算耗时
            elapsed=$(echo "$end_time - $start_time" | bc)

            echo "✅ 第 $i 轮: ${elapsed}秒"
            total_time=$(echo "$total_time + $elapsed" | bc)
        else
            echo "❌ 第 $i 轮: 编译失败"
            exit 1
        fi

        # 清理构建缓存，为下一轮做准备
        bazel clean --expunge >/dev/null 2>&1
    done

    echo ""
    echo "📈 统计结果:"
    average_time=$(echo "scale=3; $total_time / $ITERATIONS" | bc)
    echo "平均编译时间: ${average_time}秒"

    echo ""
    echo "🔍 构建信息:"
    echo "预编译头文件: 已启用 (include/sqlcc_pch.h)"
    echo "编译器: clang++-18"
    echo "C++标准: C++20"

} | tee "$OUTPUT_FILE"

echo ""
echo "📄 结果已保存到: $OUTPUT_FILE"
echo "✅ 基准测试完成!"
