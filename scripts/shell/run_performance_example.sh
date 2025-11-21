#!/bin/bash

# SQLCC性能测试示例脚本
# 此脚本演示如何构建、运行和分析SQLCC性能测试

set -e  # 遇到错误时退出

echo "SQLCC性能测试示例"
echo "=================="
echo ""

# 检查是否在项目根目录
if [ ! -f "CMakeLists.txt" ] || [ ! -d "src" ]; then
    echo "错误: 请在SQLCC项目根目录下运行此脚本"
    exit 1
fi

# 创建构建目录
echo "1. 创建构建目录..."
mkdir -p build
cd build

# 配置CMake，启用性能测试
echo "2. 配置CMake，启用性能测试..."
cmake .. -DENABLE_PERFORMANCE_TESTS=ON

# 构建项目
echo "3. 构建项目..."
make -j$(nproc)

# 创建结果目录
RESULTS_DIR="performance_results_$(date +%Y%m%d_%H%M%S)"
mkdir -p "$RESULTS_DIR"

# 运行性能测试
echo "4. 运行性能测试..."
echo "   运行缓冲池性能测试..."
./tests/performance/performance_test --type=buffer_pool --output="$RESULTS_DIR" --verbose

echo "   运行磁盘I/O性能测试..."
./tests/performance/performance_test --type=disk_io --output="$RESULTS_DIR" --verbose

echo "   运行混合工作负载测试..."
./tests/performance/performance_test --type=mixed_workload --output="$RESULTS_DIR" --verbose

# 查看测试结果
echo "5. 查看测试结果..."
cd ..
./scripts/view_performance_results.sh "build/$RESULTS_DIR"

# 安装Python依赖（如果尚未安装）
echo ""
echo "6. 检查Python依赖..."
if ! python3 -c "import pandas, matplotlib, seaborn" 2>/dev/null; then
    echo "   安装Python依赖..."
    pip3 install pandas matplotlib seaborn
else
    echo "   Python依赖已安装"
fi

# 生成分析图表
echo "7. 生成分析图表..."
python3 scripts/analyze_performance_results.py "build/$RESULTS_DIR"

# 显示分析结果
echo ""
echo "8. 分析结果已保存至: build/$RESULTS_DIR/analysis/"
echo "   主要图表包括:"
ls -la "build/$RESULTS_DIR/analysis/"*.png

echo ""
echo "性能测试完成！"
echo "结果目录: build/$RESULTS_DIR"
echo "要查看详细报告，请打开: build/$RESULTS_DIR/analysis/performance_summary.md"