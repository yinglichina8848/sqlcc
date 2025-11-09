#!/bin/bash

# SQLCC性能测试结果查看脚本

# 检查参数
if [ $# -eq 0 ]; then
    echo "用法: $0 <结果目录>"
    echo "示例: $0 /home/liying/sqlcc/test_results/performance_20231115_143022"
    exit 1
fi

RESULTS_DIR=$1

# 检查结果目录是否存在
if [ ! -d "$RESULTS_DIR" ]; then
    echo "错误: 结果目录不存在: $RESULTS_DIR"
    exit 1
fi

echo "SQLCC性能测试结果"
echo "=================="
echo "结果目录: $RESULTS_DIR"
echo ""

# 显示摘要信息
if [ -f "$RESULTS_DIR/performance_summary.txt" ]; then
    echo "摘要信息:"
    echo "--------"
    cat "$RESULTS_DIR/performance_summary.txt"
    echo ""
fi

# 显示CSV文件
echo "测试结果数据:"
echo "------------"
for csv_file in "$RESULTS_DIR"/*.csv; do
    if [ -f "$csv_file" ]; then
        echo "$(basename "$csv_file"):"
        # 显示前5行
        head -n 5 "$csv_file"
        echo "..."
        echo ""
    fi
done

# 检查是否有分析结果
if [ -d "$RESULTS_DIR/analysis" ]; then
    echo "分析结果:"
    echo "--------"
    ls -la "$RESULTS_DIR/analysis"
    echo ""
    echo "要查看分析图表，请使用以下命令:"
    echo "  python3 scripts/analyze_performance_results.py $RESULTS_DIR"
else
    echo "未找到分析结果。要生成分析图表，请使用以下命令:"
    echo "  python3 scripts/analyze_performance_results.py $RESULTS_DIR"
fi