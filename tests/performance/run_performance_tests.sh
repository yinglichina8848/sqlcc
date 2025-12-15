#!/bin/bash

# SQLCC 性能测试执行脚本
# 支持小、中、大规模CRUD性能测试
# 测试输出重定向到文件，避免影响性能

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

# 创建日志文件
LOG_FILE="$RESULTS_DIR/performance_test_$(date +%Y%m%d_%H%M%S).log"

echo "=== SQLCC 性能测试系统 ===" | tee -a "$LOG_FILE"
echo "开始时间: $(date)" | tee -a "$LOG_FILE"
echo "测试规模: $SCALE" | tee -a "$LOG_FILE"
echo "日志文件: $LOG_FILE"
echo ""

# 编译性能测试
BUILD_DIR="build_performance"
if [ ! -d "$BUILD_DIR" ]; then
    mkdir -p "$BUILD_DIR"
    echo "创建构建目录: $BUILD_DIR" >> "$LOG_FILE"
fi

cd "$BUILD_DIR"

# 配置CMake
echo "配置CMake..." >> "$LOG_FILE"
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_PERFORMANCE_TESTS=ON >> "$LOG_FILE" 2>&1

# 编译
echo "编译性能测试..." >> "$LOG_FILE"
make -j$(nproc) crud_performance_test >> "$LOG_FILE" 2>&1

# 运行测试
echo "=== 开始性能测试 ===" >> "$LOG_FILE"

# 根据测试规模运行不同的测试
case $SCALE in
    "small")
        echo "运行小规模测试 (1000条记录)..." >> "$LOG_FILE"
        ./tests/performance/crud/crud_performance_test --scale=small > /dev/null 2>&1
        ;;
    "medium") 
        echo "运行中等规模测试 (10000条记录)..." >> "$LOG_FILE"
        ./tests/performance/crud/crud_performance_test --scale=medium > /dev/null 2>&1
        ;;
    "large")
        echo "运行大规模测试 (50000条记录)..." >> "$LOG_FILE"
        ./tests/performance/crud/crud_performance_test --scale=large > /dev/null 2>&1
        ;;
    "xlarge")
        echo "运行超大规模测试 (100000条记录)..." >> "$LOG_FILE"
        ./tests/performance/crud/crud_performance_test --scale=xlarge > /dev/null 2>&1
        ;;
    "all")
        echo "运行所有规模测试..." >> "$LOG_FILE"
        ./tests/performance/crud/crud_performance_test --scale=small > /dev/null 2>&1
        ./tests/performance/crud/crud_performance_test --scale=medium > /dev/null 2>&1
        ./tests/performance/crud/crud_performance_test --scale=large > /dev/null 2>&1
        ./tests/performance/crud/crud_performance_test --scale=xlarge > /dev/null 2>&1
        ;;
    *)
        echo "错误: 未知的测试规模 '$SCALE'" | tee -a "$LOG_FILE"
        echo "可用选项: small, medium, large, xlarge, all" | tee -a "$LOG_FILE"
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

echo "=== 性能测试完成 ===" | tee -a "$LOG_FILE"
echo "结束时间: $(date)" | tee -a "$LOG_FILE"
echo "测试结果保存在: $RESULTS_DIR/" | tee -a "$LOG_FILE"

# 生成测试摘要
echo "=== 测试摘要 ===" | tee -a "$LOG_FILE"
if [ -d "$RESULTS_DIR" ]; then
    ls -la "$RESULTS_DIR/" | grep -E "(report|csv)" | tail -5 >> "$LOG_FILE"
fi

echo ""
echo "性能测试执行完毕！"
echo "详细日志请查看: $LOG_FILE"
echo "测试结果文件保存在: $RESULTS_DIR/"