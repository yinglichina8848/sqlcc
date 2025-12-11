#!/bin/bash

# SQLCC性能测试运行脚本

echo "Building SQLCC performance tests..."

# 使用Bazel构建性能测试
bazel build //tests/performance:real_crud_performance_test

if [ $? -eq 0 ]; then
    echo "Build successful!"
    echo "Running performance tests..."
    
    # 创建测试数据目录
    mkdir -p test_data
    
    # 运行性能测试
    bazel run //tests/performance:real_crud_performance_test
else
    echo "Build failed!"
    exit 1
fi