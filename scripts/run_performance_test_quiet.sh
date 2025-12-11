#!/bin/bash

# SQLCC性能测试运行脚本（静默模式）

echo "Building SQLCC performance tests..."

# 使用Bazel构建性能测试
bazel build //tests/performance:real_crud_performance_test

if [ $? -eq 0 ]; then
    echo "Build successful!"
    echo "Running performance tests (quiet mode)..."
    
    # 创建测试数据目录
    mkdir -p test_data
    
    # 运行性能测试并将输出重定向到文件
    bazel run //tests/performance:real_crud_performance_test > performance_test_run.log 2>&1
    
    echo "Performance tests completed. Check the following files for results:"
    echo "  - crud_performance_output.txt (detailed test output)"
    echo "  - crud_performance_report.txt (summary report)"
    echo "  - performance_test_run.log (execution log)"
else
    echo "Build failed! Check the build logs for details."
    exit 1
fi