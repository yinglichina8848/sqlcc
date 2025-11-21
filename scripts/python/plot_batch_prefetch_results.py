#!/usr/bin/env python3
"""
批量预取性能测试结果分析和可视化工具
"""

import os
import sys
import pandas as pd
import matplotlib
# 设置非交互式后端
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np
from pathlib import Path

# 设置matplotlib支持中文显示
plt.rcParams['font.sans-serif'] = ['WenQuanYi Zen Hei', 'WenQuanYi Micro Hei', 'DejaVu Sans']
plt.rcParams['axes.unicode_minus'] = False

# 强制使用英文标签，避免中文字符显示问题
force_english_labels = True

def plot_batch_prefetch_results():
    """绘制批量预取性能测试结果"""
    
    # 读取CSV文件
    current_dir = Path(".")
    
    # 单页读取 vs 单页预取
    single_read_data = pd.read_csv(current_dir / "single_page_read.csv")
    single_prefetch_data = pd.read_csv(current_dir / "single_page_prefetch.csv")
    
    # 批量读取结果
    batch_read_data = pd.read_csv(current_dir / "batch_page_read.csv")
    
    # 批量预取结果
    batch_prefetch_data = pd.read_csv(current_dir / "batch_prefetch.csv")
    
    # 混合访问模式结果
    mixed_access_data = pd.read_csv(current_dir / "mixed_access_pattern.csv")
    
    # 不同批量大小的结果
    varying_batch_data = pd.read_csv(current_dir / "varying_batch_size.csv")
    
    # 创建输出目录
    output_dir = current_dir / "analysis"
    output_dir.mkdir(exist_ok=True)
    
    # 设置seaborn样式
    sns.set_style("whitegrid")
    sns.set_palette("husl")
    
    # 1. 单页访问 vs 预取性能对比
    plt.figure(figsize=(10, 6))
    
    # 使用英文标签
    test_names = ['Single Page Read', 'Single Page Prefetch']
    ylabel = 'Throughput (ops/sec)'
    title = 'Single Page Access vs Prefetch Performance Comparison'
    
    throughputs = [single_read_data['Throughput(ops/sec)'].iloc[0], 
                   single_prefetch_data['Throughput(ops/sec)'].iloc[0]]
    
    bars = plt.bar(test_names, throughputs, color=['skyblue', 'lightgreen'])
    plt.ylabel(ylabel)
    plt.title(title)
    
    # 添加数值标签
    for bar, throughput in zip(bars, throughputs):
        height = bar.get_height()
        plt.text(bar.get_x() + bar.get_width()/2., height + height*0.01,
                f'{throughput:.0f}', ha='center', va='bottom')
    
    plt.tight_layout()
    plt.savefig(output_dir / 'single_vs_prefetch.png', dpi=300)
    plt.close()
    
    # 2. 不同批量大小的性能对比
    plt.figure(figsize=(12, 8))
    
    batch_sizes = batch_read_data['batch_size'].values
    batch_read_throughputs = batch_read_data['Throughput(ops/sec)'].values
    batch_prefetch_throughputs = batch_prefetch_data['Throughput(ops/sec)'].values
    
    # 使用英文标签
    xlabel = 'Batch Size'
    ylabel = 'Throughput (ops/sec)'
    title = 'Performance Comparison for Different Batch Sizes'
    label1 = 'Batch Read'
    label2 = 'Batch Prefetch'
    
    plt.plot(batch_sizes, batch_read_throughputs, marker='o', linestyle='-', label=label1)
    plt.plot(batch_sizes, batch_prefetch_throughputs, marker='s', linestyle='--', label=label2)
    
    plt.xlabel(xlabel)
    plt.ylabel(ylabel)
    plt.title(title)
    plt.legend()
    plt.grid(True)
    
    plt.tight_layout()
    plt.savefig(output_dir / 'batch_size_comparison.png', dpi=300)
    plt.close()
    
    # 3. 混合访问模式性能对比
    plt.figure(figsize=(12, 8))
    
    mixed_names = mixed_access_data['Test Name'].values
    mixed_throughputs = mixed_access_data['Throughput(ops/sec)'].values
    
    # 使用英文标签
    ylabel = 'Throughput (ops/sec)'
    title = 'Mixed Access Pattern Performance Comparison'
    simplified_names = []
    for name in mixed_names:
        if 'Single' in name:
            simplified_names.append('Single Page')
        elif 'Batch' in name:
            simplified_names.append('Batch Read')
        elif 'Prefetch' in name:
            simplified_names.append('Prefetch')
    
    bars = plt.bar(simplified_names, mixed_throughputs, color=['skyblue', 'lightgreen', 'salmon'])
    plt.ylabel(ylabel)
    plt.title(title)
    
    # 添加数值标签
    for bar, throughput in zip(bars, mixed_throughputs):
        height = bar.get_height()
        plt.text(bar.get_x() + bar.get_width()/2., height + height*0.01,
                f'{throughput:.0f}', ha='center', va='bottom')
    
    plt.tight_layout()
    plt.savefig(output_dir / 'mixed_access_pattern.png', dpi=300)
    plt.close()
    
    # 4. 性能提升倍数
    plt.figure(figsize=(10, 6))
    
    # 计算性能提升倍数
    single_read_throughput = single_read_data['Throughput(ops/sec)'].iloc[0]
    single_prefetch_throughput = single_prefetch_data['Throughput(ops/sec)'].iloc[0]
    batch_read_throughput = batch_read_data[batch_read_data['batch_size'] == 8]['Throughput(ops/sec)'].iloc[0]
    batch_prefetch_throughput = batch_prefetch_data[batch_prefetch_data['batch_size'] == 8]['Throughput(ops/sec)'].iloc[0]
    
    # 使用英文标签
    ylabel = 'Performance Improvement (x)'
    title = 'Performance Improvement with Different Optimizations'
    test_types = ['Single Page Prefetch\nvs Single Page Read', 
                  'Batch Read\nvs Single Page Read', 
                  'Batch Prefetch\nvs Single Page Read']
    
    improvements = [
        single_prefetch_throughput / single_read_throughput,
        batch_read_throughput / single_read_throughput,
        batch_prefetch_throughput / single_read_throughput
    ]
    
    bars = plt.bar(test_types, improvements, color=['lightgreen', 'skyblue', 'salmon'])
    plt.ylabel(ylabel)
    plt.title(title)
    plt.axhline(y=1, color='gray', linestyle='--', alpha=0.7)  # 基准线
    
    # 添加数值标签
    for bar, improvement in zip(bars, improvements):
        height = bar.get_height()
        plt.text(bar.get_x() + bar.get_width()/2., height + 0.05,
                f'{improvement:.2f}x', ha='center', va='bottom')
    
    plt.tight_layout()
    plt.savefig(output_dir / 'performance_improvements.png', dpi=300)
    plt.close()
    
    print(f"图表已保存至: {output_dir}")

if __name__ == "__main__":
    plot_batch_prefetch_results()