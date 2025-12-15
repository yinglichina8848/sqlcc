#!/bin/bash

# SQLCC 静默性能测试执行脚本
# 专为生产环境设计，完全静默执行，避免任何输出影响性能

set -e

# 创建测试结果目录
RESULTS_DIR="performance_results"
if [ ! -d "$RESULTS_DIR" ]; then
    mkdir -p "$RESULTS_DIR"
fi

# 设置测试规模
SCALE="all"  # 默认运行所有规模测试
if [ $# -ge 1 ]; then
    SCALE=$1
fi

# 创建静默日志文件
LOG_FILE="$RESULTS_DIR/silent_performance_test_$(date +%Y%m%d_%H%M%S).log"

# 静默执行，所有输出重定向到日志文件
echo "开始静默性能测试..." > "$LOG_FILE"
echo "测试规模: $SCALE" >> "$LOG_FILE"
echo "开始时间: $(date)" >> "$LOG_FILE"

# 编译性能测试
BUILD_DIR="build_performance"
if [ ! -d "$BUILD_DIR" ]; then
    mkdir -p "$BUILD_DIR"
fi

cd "$BUILD_DIR"

# 静默配置CMake
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_PERFORMANCE_TESTS=ON >> "$LOG_FILE" 2>&1

# 静默编译
make -j$(nproc) crud_performance_test >> "$LOG_FILE" 2>&1

# 静默运行测试
echo "开始执行性能测试..." >> "$LOG_FILE"

case $SCALE in
    "small")
        ./tests/performance/crud/crud_performance_test --scale=small > /dev/null 2>&1
        ;;
    "medium") 
        ./tests/performance/crud/crud_performance_test --scale=medium > /dev/null 2>&1
        ;;
    "large")
        ./tests/performance/crud/crud_performance_test --scale=large > /dev/null 2>&1
        ;;
    "xlarge")
        ./tests/performance/crud/crud_performance_test --scale=xlarge > /dev/null 2>&1
        ;;
    "all")
        ./tests/performance/crud/crud_performance_test --scale=small > /dev/null 2>&1
        ./tests/performance/crud/crud_performance_test --scale=medium > /dev/null 2>&1
        ./tests/performance/crud/crud_performance_test --scale=large > /dev/null 2>&1
        ./tests/performance/crud/crud_performance_test --scale=xlarge > /dev/null 2>&1
        ;;
    *)
        echo "错误: 未知的测试规模 '$SCALE'" >> "$LOG_FILE"
        echo "可用选项: small, medium, large, xlarge, all" >> "$LOG_FILE"
        exit 1
        ;;
esac

# 移动测试结果到结果目录
cd ..
if [ -f "crud_performance_report.txt" ]; then
    mv "crud_performance_report.txt" "$RESULTS_DIR/crud_performance_report_$(date +%Y%m%d_%H%M%S).txt"
fi

if [ -f "performance_report.csv" ]; then
    mv "performance_report.csv" "$RESULTS_DIR/performance_report_$(date +%Y%m%d_%H%M%S).csv"
fi

echo "性能测试完成" >> "$LOG_FILE"
echo "结束时间: $(date)" >> "$LOG_FILE"

# 检查测试结果
echo "=== 测试结果检查 ===" >> "$LOG_FILE"
if [ -d "$RESULTS_DIR" ]; then
    ls -la "$RESULTS_DIR/" | grep -E "(report|csv)" | tail -5 >> "$LOG_FILE"
fi

echo "静默性能测试执行完毕！"
echo "详细日志请查看: $LOG_FILE"
echo "测试结果文件保存在: $RESULTS_DIR/"