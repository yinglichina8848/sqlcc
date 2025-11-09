#!/usr/bin/env python3
"""
SQLCC性能测试结果分析和可视化工具
"""

import os
import sys
import argparse
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

class PerformanceAnalyzer:
    """性能测试结果分析器"""
    
    def __init__(self, results_dir):
        """
        初始化分析器
        
        Args:
            results_dir: 性能测试结果目录
        """
        self.results_dir = Path(results_dir)
        self.output_dir = self.results_dir / "analysis"
        self.output_dir.mkdir(exist_ok=True)
        
        # 设置seaborn样式
        sns.set_style("whitegrid")
        sns.set_palette("husl")
    
    def load_csv_data(self, csv_file):
        """
        加载CSV数据
        
        Args:
            csv_file: CSV文件路径
            
        Returns:
            DataFrame: 加载的数据
        """
        try:
            file_path = self.results_dir / csv_file
            return pd.read_csv(file_path)
        except Exception as e:
            print(f"Error loading {csv_file}: {e}")
            return pd.DataFrame()
    
    def analyze_buffer_pool_results(self):
        """分析缓冲池性能测试结果"""
        print("分析缓冲池性能测试结果...")
        
        # 加载缓存命中率测试结果
        hit_rate_data = self.load_csv_data("buffer_pool_cache_hit_rate.csv")
        if not hit_rate_data.empty:
            self.plot_buffer_pool_hit_rate(hit_rate_data)
        
        # 加载LRU效率测试结果
        lru_data = self.load_csv_data("buffer_pool_lru_efficiency.csv")
        if not lru_data.empty:
            self.plot_lru_efficiency(lru_data)
        
        # 加载访问模式测试结果
        access_pattern_data = self.load_csv_data("buffer_pool_access_pattern.csv")
        if not access_pattern_data.empty:
            self.plot_access_pattern(access_pattern_data)
        
        # 加载缓冲池大小扩展性测试结果
        scalability_data = self.load_csv_data("buffer_pool_size_scalability.csv")
        if not scalability_data.empty:
            self.plot_pool_size_scalability(scalability_data)
    
    def plot_buffer_pool_hit_rate(self, data):
        """绘制缓存命中率图表"""
        plt.figure(figsize=(10, 6))
        
        # 提取数据
        pool_sizes = []
        hit_rates = []
        
        for _, row in data.iterrows():
            pool_size = int(row['Pool Size'])
            hit_rate = float(row['Hit Rate'].rstrip('%'))
            pool_sizes.append(pool_size)
            hit_rates.append(hit_rate)
        
        # 绘制柱状图
        plt.bar(pool_sizes, hit_rates, color='skyblue')
        plt.xlabel('缓冲池大小（页面数）')
        plt.ylabel('缓存命中率（%）')
        plt.title('不同缓冲池大小下的缓存命中率')
        plt.xticks(pool_sizes)
        plt.ylim(0, 100)
        
        # 添加数值标签
        for i, (pool_size, hit_rate) in enumerate(zip(pool_sizes, hit_rates)):
            plt.text(pool_size, hit_rate + 1, f'{hit_rate:.1f}%', ha='center')
        
        plt.tight_layout()
        plt.savefig(self.output_dir / 'buffer_pool_hit_rate.png', dpi=300)
        plt.close()
    
    def plot_lru_efficiency(self, data):
        """绘制LRU效率图表"""
        plt.figure(figsize=(10, 6))
        
        # 提取数据
        working_set_sizes = []
        hit_rates = []
        
        for _, row in data.iterrows():
            working_set_size = int(row['Working Set Size'])
            hit_rate = float(row['Hit Rate'].rstrip('%'))
            working_set_sizes.append(working_set_size)
            hit_rates.append(hit_rate)
        
        # 绘制折线图
        plt.plot(working_set_sizes, hit_rates, marker='o', linestyle='-', color='coral')
        plt.xlabel('工作集大小（页面数）')
        plt.ylabel('缓存命中率（%）')
        plt.title('不同工作集大小下的LRU效率')
        plt.grid(True)
        plt.ylim(0, 100)
        
        plt.tight_layout()
        plt.savefig(self.output_dir / 'lru_efficiency.png', dpi=300)
        plt.close()
    
    def plot_access_pattern(self, data):
        """绘制访问模式性能图表"""
        plt.figure(figsize=(12, 8))
        
        # 提取数据
        patterns = []
        throughputs = []
        avg_latencies = []
        
        for _, row in data.iterrows():
            pattern = row['Access Pattern']
            throughput = row['Throughput(ops/sec)']
            avg_latency = row['Avg Latency(ms)']
            patterns.append(pattern)
            throughputs.append(throughput)
            avg_latencies.append(avg_latency)
        
        # 创建子图
        fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 6))
        
        # 绘制吞吐量
        bars1 = ax1.bar(patterns, throughputs, color='lightgreen')
        ax1.set_xlabel('访问模式')
        ax1.set_ylabel('吞吐量（ops/sec）')
        ax1.set_title('不同访问模式下的吞吐量')
        
        # 添加数值标签
        for bar, throughput in zip(bars1, throughputs):
            height = bar.get_height()
            ax1.text(bar.get_x() + bar.get_width()/2., height + height*0.01,
                    f'{throughput:.0f}', ha='center', va='bottom')
        
        # 绘制平均延迟
        bars2 = ax2.bar(patterns, avg_latencies, color='salmon')
        ax2.set_xlabel('访问模式')
        ax2.set_ylabel('平均延迟（ms）')
        ax2.set_title('不同访问模式下的平均延迟')
        
        # 添加数值标签
        for bar, latency in zip(bars2, avg_latencies):
            height = bar.get_height()
            ax2.text(bar.get_x() + bar.get_width()/2., height + height*0.01,
                    f'{latency:.3f}', ha='center', va='bottom')
        
        plt.tight_layout()
        plt.savefig(self.output_dir / 'access_pattern_performance.png', dpi=300)
        plt.close()
    
    def plot_pool_size_scalability(self, data):
        """绘制缓冲池大小扩展性图表"""
        plt.figure(figsize=(12, 8))
        
        # 提取数据
        pool_sizes = []
        throughputs = []
        avg_latencies = []
        
        for _, row in data.iterrows():
            pool_size = int(row['Pool Size'])
            throughput = row['Throughput(ops/sec)']
            avg_latency = row['Avg Latency(ms)']
            pool_sizes.append(pool_size)
            throughputs.append(throughput)
            avg_latencies.append(avg_latency)
        
        # 创建子图
        fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 6))
        
        # 绘制吞吐量
        ax1.plot(pool_sizes, throughputs, marker='o', linestyle='-', color='royalblue')
        ax1.set_xlabel('缓冲池大小（页面数）')
        ax1.set_ylabel('吞吐量（ops/sec）')
        ax1.set_title('缓冲池大小与吞吐量的关系')
        ax1.grid(True)
        
        # 绘制平均延迟
        ax2.plot(pool_sizes, avg_latencies, marker='s', linestyle='-', color='crimson')
        ax2.set_xlabel('缓冲池大小（页面数）')
        ax2.set_ylabel('平均延迟（ms）')
        ax2.set_title('缓冲池大小与平均延迟的关系')
        ax2.grid(True)
        
        plt.tight_layout()
        plt.savefig(self.output_dir / 'pool_size_scalability.png', dpi=300)
        plt.close()
    
    def analyze_disk_io_results(self):
        """分析磁盘I/O性能测试结果"""
        print("分析磁盘I/O性能测试结果...")
        
        # 加载顺序读写测试结果
        sequential_data = self.load_csv_data("disk_io_sequential.csv")
        if not sequential_data.empty:
            self.plot_sequential_io(sequential_data)
        
        # 加载随机读写测试结果
        random_data = self.load_csv_data("disk_io_random.csv")
        if not random_data.empty:
            self.plot_random_io(random_data)
        
        # 加载不同页面大小测试结果
        varying_page_data = self.load_csv_data("disk_io_varying_page_size.csv")
        if not varying_page_data.empty:
            self.plot_varying_page_size(varying_page_data)
        
        # 加载并发I/O测试结果
        concurrent_data = self.load_csv_data("disk_io_concurrent.csv")
        if not concurrent_data.empty:
            self.plot_concurrent_io(concurrent_data)
    
    def plot_sequential_io(self, data):
        """绘制顺序I/O性能图表"""
        plt.figure(figsize=(12, 8))
        
        # 分离读取和写入数据
        read_data = data[data['Test Name'].str.contains('Read')]
        write_data = data[data['Test Name'].str.contains('Write')]
        
        # 提取数据
        read_page_sizes = []
        read_throughputs = []
        
        for _, row in read_data.iterrows():
            page_size = int(row['Page Size'])
            throughput = float(row['Throughput(MB/s)'])
            read_page_sizes.append(page_size)
            read_throughputs.append(throughput)
        
        write_page_sizes = []
        write_throughputs = []
        
        for _, row in write_data.iterrows():
            page_size = int(row['Page Size'])
            throughput = float(row['Throughput(MB/s)'])
            write_page_sizes.append(page_size)
            write_throughputs.append(throughput)
        
        # 绘制图表
        plt.figure(figsize=(10, 6))
        plt.plot(read_page_sizes, read_throughputs, marker='o', linestyle='-', label='顺序读取')
        plt.plot(write_page_sizes, write_throughputs, marker='s', linestyle='-', label='顺序写入')
        
        plt.xlabel('页面大小（字节）')
        plt.ylabel('吞吐量（MB/s）')
        plt.title('顺序I/O性能与页面大小的关系')
        plt.legend()
        plt.grid(True)
        
        plt.tight_layout()
        plt.savefig(self.output_dir / 'sequential_io_performance.png', dpi=300)
        plt.close()
    
    def plot_random_io(self, data):
        """绘制随机I/O性能图表"""
        plt.figure(figsize=(10, 6))
        
        # 提取数据
        operations = []
        throughputs = []
        
        for _, row in data.iterrows():
            operation = '随机读取' if 'Read' in row['Test Name'] else '随机写入'
            throughput = float(row['Throughput(MB/s)'])
            operations.append(operation)
            throughputs.append(throughput)
        
        # 绘制柱状图
        plt.bar(operations, throughputs, color=['lightblue', 'lightcoral'])
        plt.ylabel('吞吐量（MB/s）')
        plt.title('随机I/O性能')
        
        # 添加数值标签
        for i, (operation, throughput) in enumerate(zip(operations, throughputs)):
            plt.text(i, throughput + throughput*0.01, f'{throughput:.2f}', ha='center')
        
        plt.tight_layout()
        plt.savefig(self.output_dir / 'random_io_performance.png', dpi=300)
        plt.close()
    
    def plot_varying_page_size(self, data):
        """绘制不同页面大小下的I/O性能图表"""
        plt.figure(figsize=(10, 6))
        
        # 提取数据
        page_sizes = []
        throughputs = []
        
        for _, row in data.iterrows():
            page_size = int(row['Page Size'])
            throughput = float(row['Throughput(MB/s)'])
            page_sizes.append(page_size)
            throughputs.append(throughput)
        
        # 绘制折线图
        plt.plot(page_sizes, throughputs, marker='o', linestyle='-', color='mediumseagreen')
        plt.xlabel('页面大小（字节）')
        plt.ylabel('吞吐量（MB/s）')
        plt.title('混合I/O性能与页面大小的关系')
        plt.grid(True)
        
        plt.tight_layout()
        plt.savefig(self.output_dir / 'varying_page_size_performance.png', dpi=300)
        plt.close()
    
    def plot_concurrent_io(self, data):
        """绘制并发I/O性能图表"""
        plt.figure(figsize=(12, 8))
        
        # 提取数据
        thread_counts = []
        throughputs = []
        avg_latencies = []
        
        for _, row in data.iterrows():
            thread_count = int(row['Thread Count'])
            throughput = float(row['Throughput(MB/s)'])
            avg_latency = float(row['Avg Latency(ms)'])
            thread_counts.append(thread_count)
            throughputs.append(throughput)
            avg_latencies.append(avg_latency)
        
        # 创建子图
        fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 6))
        
        # 绘制吞吐量
        ax1.plot(thread_counts, throughputs, marker='o', linestyle='-', color='dodgerblue')
        ax1.set_xlabel('线程数')
        ax1.set_ylabel('吞吐量（MB/s）')
        ax1.set_title('并发I/O吞吐量')
        ax1.grid(True)
        
        # 绘制平均延迟
        ax2.plot(thread_counts, avg_latencies, marker='s', linestyle='-', color='tomato')
        ax2.set_xlabel('线程数')
        ax2.set_ylabel('平均延迟（ms）')
        ax2.set_title('并发I/O平均延迟')
        ax2.grid(True)
        
        plt.tight_layout()
        plt.savefig(self.output_dir / 'concurrent_io_performance.png', dpi=300)
        plt.close()
    
    def analyze_mixed_workload_results(self):
        """分析混合工作负载性能测试结果"""
        print("分析混合工作负载性能测试结果...")
        
        # 加载读写比例测试结果
        rw_ratio_data = self.load_csv_data("mixed_workload_read_write_ratio.csv")
        if not rw_ratio_data.empty:
            self.plot_read_write_ratio(rw_ratio_data)
        
        # 加载事务大小测试结果
        tx_size_data = self.load_csv_data("mixed_workload_transaction_size.csv")
        if not tx_size_data.empty:
            self.plot_transaction_size(tx_size_data)
        
        # 加载长时间运行测试结果
        long_running_data = self.load_csv_data("mixed_workload_long_running.csv")
        if not long_running_data.empty:
            self.plot_long_running(long_running_data)
        
        # 加载并发测试结果
        concurrent_data = self.load_csv_data("mixed_workload_concurrent.csv")
        if not concurrent_data.empty:
            self.plot_concurrent_workload(concurrent_data)
    
    def plot_read_write_ratio(self, data):
        """绘制读写比例性能图表"""
        plt.figure(figsize=(12, 8))
        
        # 提取数据
        test_names = []
        read_ratios = []
        throughputs = []
        
        for _, row in data.iterrows():
            test_name = row['Test Name']
            read_ratio = float(row['Read Ratio'].rstrip('%'))
            throughput = row['Throughput(ops/sec)']
            test_names.append(test_name)
            read_ratios.append(read_ratio)
            throughputs.append(throughput)
        
        # 绘制散点图
        plt.figure(figsize=(10, 6))
        plt.scatter(read_ratios, throughputs, s=100, alpha=0.7, c='purple')
        
        # 添加标签
        for i, (test_name, read_ratio, throughput) in enumerate(zip(test_names, read_ratios, throughputs)):
            plt.annotate(test_name, (read_ratio, throughput), 
                        xytext=(5, 5), textcoords='offset points')
        
        plt.xlabel('读操作比例（%）')
        plt.ylabel('吞吐量（ops/sec）')
        plt.title('读写比例与吞吐量的关系')
        plt.grid(True)
        
        plt.tight_layout()
        plt.savefig(self.output_dir / 'read_write_ratio_performance.png', dpi=300)
        plt.close()
    
    def plot_transaction_size(self, data):
        """绘制事务大小性能图表"""
        plt.figure(figsize=(12, 8))
        
        # 提取数据
        tx_sizes = []
        throughputs = []
        avg_latencies = []
        
        for _, row in data.iterrows():
            tx_size = int(row['Transaction Size'])
            throughput = row['Throughput(ops/sec)']
            avg_latency = float(row['Avg Latency(ms)'])
            tx_sizes.append(tx_size)
            throughputs.append(throughput)
            avg_latencies.append(avg_latency)
        
        # 创建子图
        fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 6))
        
        # 绘制吞吐量
        ax1.plot(tx_sizes, throughputs, marker='o', linestyle='-', color='forestgreen')
        ax1.set_xlabel('事务大小（页面数）')
        ax1.set_ylabel('吞吐量（ops/sec）')
        ax1.set_title('事务大小与吞吐量的关系')
        ax1.grid(True)
        
        # 绘制平均延迟
        ax2.plot(tx_sizes, avg_latencies, marker='s', linestyle='-', color='firebrick')
        ax2.set_xlabel('事务大小（页面数）')
        ax2.set_ylabel('平均延迟（ms）')
        ax2.set_title('事务大小与平均延迟的关系')
        ax2.grid(True)
        
        plt.tight_layout()
        plt.savefig(self.output_dir / 'transaction_size_performance.png', dpi=300)
        plt.close()
    
    def plot_long_running(self, data):
        """绘制长时间运行性能图表"""
        plt.figure(figsize=(10, 6))
        
        # 提取数据
        test_names = []
        durations = []
        throughputs = []
        
        for _, row in data.iterrows():
            test_name = row['Test Name']
            duration = row['Duration(ms)'] / 1000  # 转换为秒
            throughput = row['Throughput(ops/sec)']
            test_names.append(test_name)
            durations.append(duration)
            throughputs.append(throughput)
        
        # 绘制柱状图
        plt.bar(test_names, throughputs, color='gold')
        plt.ylabel('吞吐量（ops/sec）')
        plt.title('长时间运行性能')
        plt.xticks(rotation=45, ha='right')
        
        # 添加数值标签
        for i, (test_name, throughput) in enumerate(zip(test_names, throughputs)):
            plt.text(i, throughput + throughput*0.01, f'{throughput:.0f}', ha='center')
        
        plt.tight_layout()
        plt.savefig(self.output_dir / 'long_running_performance.png', dpi=300)
        plt.close()
    
    def plot_concurrent_workload(self, data):
        """绘制并发工作负载性能图表"""
        plt.figure(figsize=(12, 8))
        
        # 提取数据
        thread_counts = []
        throughputs = []
        avg_latencies = []
        
        for _, row in data.iterrows():
            thread_count = int(row['Thread Count'])
            throughput = row['Throughput(ops/sec)']
            avg_latency = float(row['Avg Latency(ms)'])
            thread_counts.append(thread_count)
            throughputs.append(throughput)
            avg_latencies.append(avg_latency)
        
        # 创建子图
        fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 6))
        
        # 绘制吞吐量
        ax1.plot(thread_counts, throughputs, marker='o', linestyle='-', color='mediumorchid')
        ax1.set_xlabel('线程数')
        ax1.set_ylabel('吞吐量（ops/sec）')
        ax1.set_title('并发工作负载吞吐量')
        ax1.grid(True)
        
        # 绘制平均延迟
        ax2.plot(thread_counts, avg_latencies, marker='s', linestyle='-', color='darkorange')
        ax2.set_xlabel('线程数')
        ax2.set_ylabel('平均延迟（ms）')
        ax2.set_title('并发工作负载平均延迟')
        ax2.grid(True)
        
        plt.tight_layout()
        plt.savefig(self.output_dir / 'concurrent_workload_performance.png', dpi=300)
        plt.close()
    
    def generate_summary_report(self):
        """生成性能测试摘要报告"""
        print("生成性能测试摘要报告...")
        
        report_path = self.output_dir / "performance_summary.md"
        
        with open(report_path, 'w', encoding='utf-8') as f:
            f.write("# SQLCC 性能测试摘要报告\n\n")
            f.write(f"生成时间: {pd.Timestamp.now()}\n\n")
            
            f.write("## 测试环境\n\n")
            f.write("- 操作系统: Linux\n")
            f.write("- CPU: 多核处理器\n")
            f.write("- 内存: 充足内存支持大缓冲池测试\n")
            f.write("- 存储: 高性能SSD\n\n")
            
            f.write("## 测试结果概览\n\n")
            f.write("本报告包含以下性能测试结果:\n\n")
            f.write("1. 缓冲池性能测试\n")
            f.write("2. 磁盘I/O性能测试\n")
            f.write("3. 混合工作负载性能测试\n\n")
            
            f.write("## 图表说明\n\n")
            f.write("所有性能图表已保存至 `analysis` 目录:\n\n")
            
            # 列出生成的图表
            charts = [
                "buffer_pool_hit_rate.png - 缓冲池命中率",
                "lru_efficiency.png - LRU效率",
                "access_pattern_performance.png - 访问模式性能",
                "pool_size_scalability.png - 缓冲池大小扩展性",
                "sequential_io_performance.png - 顺序I/O性能",
                "random_io_performance.png - 随机I/O性能",
                "varying_page_size_performance.png - 不同页面大小性能",
                "concurrent_io_performance.png - 并发I/O性能",
                "read_write_ratio_performance.png - 读写比例性能",
                "transaction_size_performance.png - 事务大小性能",
                "long_running_performance.png - 长时间运行性能",
                "concurrent_workload_performance.png - 并发工作负载性能"
            ]
            
            for chart in charts:
                f.write(f"- {chart}\n")
            
            f.write("\n## 性能基准\n\n")
            f.write("- 单线程页面读取: >10,000 ops/sec\n")
            f.write("- 单线程页面写入: >5,000 ops/sec\n")
            f.write("- 缓冲池命中率: >90%（合适的工作负载）\n")
            f.write("- P99延迟: <5ms（页面读取）\n\n")
            
            f.write("## 结论与建议\n\n")
            f.write("通过全面的性能测试，我们可以:\n\n")
            f.write("1. 全面了解SQLCC存储引擎的性能特征\n")
            f.write("2. 识别性能瓶颈并指导优化工作\n")
            f.write("3. 建立性能基准，确保版本迭代不引入性能回归\n")
            f.write("4. 为不同应用场景提供性能参考数据\n\n")
            
            f.write("性能测试是数据库系统开发的重要环节，将为SQLCC项目的持续优化提供坚实的数据基础。\n")
        
        print(f"性能测试摘要报告已保存至: {report_path}")
    
    def run_analysis(self):
        """运行完整的性能测试分析"""
        print(f"开始分析性能测试结果，结果目录: {self.results_dir}")
        
        # 分析各类测试结果
        self.analyze_buffer_pool_results()
        self.analyze_disk_io_results()
        self.analyze_mixed_workload_results()
        
        # 生成摘要报告
        self.generate_summary_report()
        
        print(f"性能测试分析完成，结果已保存至: {self.output_dir}")


def main():
    """主函数"""
    parser = argparse.ArgumentParser(description='SQLCC性能测试结果分析工具')
    parser.add_argument('results_dir', help='性能测试结果目录')
    
    args = parser.parse_args()
    
    # 检查结果目录是否存在
    if not os.path.exists(args.results_dir):
        print(f"错误: 结果目录不存在: {args.results_dir}")
        sys.exit(1)
    
    # 创建分析器并运行分析
    analyzer = PerformanceAnalyzer(args.results_dir)
    analyzer.run_analysis()


if __name__ == "__main__":
    main()