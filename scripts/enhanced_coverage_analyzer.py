#!/usr/bin/env python3
# SQLCC 增强覆盖率分析器
# 提供多维度覆盖率分析、模块级报告和趋势可视化

import json
import os
import re
import subprocess
import sys
from datetime import datetime, timedelta
from pathlib import Path
from typing import Dict, List, Any, Tuple, Optional
from collections import defaultdict
import matplotlib.pyplot as plt
import numpy as np

class EnhancedCoverageAnalyzer:
    """增强覆盖率分析器"""

    def __init__(self, coverage_data_dir: str = "bazel-out/_coverage"):
        self.coverage_data_dir = Path(coverage_data_dir)
        self.project_root = Path.cwd()
        self.reports_dir = Path("test_reports")
        self.reports_dir.mkdir(exist_ok=True)

    def parse_lcov_info(self, info_file: Path) -> Dict[str, Any]:
        """解析 LCOV info 文件"""
        coverage_data = {
            "summary": {
                "line_coverage": 0.0,
                "branch_coverage": 0.0,
                "function_coverage": 0.0,
                "line_hit": 0,
                "line_total": 0,
                "branch_hit": 0,
                "branch_total": 0,
                "function_hit": 0,
                "function_total": 0
            },
            "files": {},
            "modules": defaultdict(lambda: {
                "line_hit": 0, "line_total": 0,
                "branch_hit": 0, "branch_total": 0,
                "function_hit": 0, "function_total": 0,
                "files": []
            })
        }

        if not info_file.exists():
            print(f"警告: 覆盖率数据文件不存在: {info_file}")
            return coverage_data

        current_file = None

        try:
            with open(info_file, 'r', encoding='utf-8') as f:
                for line in f:
                    line = line.strip()
                    if not line:
                        continue

                    if line.startswith('SF:'):
                        # 源文件开始
                        file_path = line[3:]
                        current_file = file_path
                        coverage_data["files"][file_path] = {
                            "line_hit": 0, "line_total": 0,
                            "branch_hit": 0, "branch_total": 0,
                            "function_hit": 0, "function_total": 0,
                            "lines": {}
                        }

                        # 确定模块
                        module = self._get_module_from_path(file_path)
                        coverage_data["modules"][module]["files"].append(file_path)

                    elif line.startswith('DA:') and current_file:
                        # 行覆盖率数据: DA:line,hit_count
                        parts = line[3:].split(',')
                        if len(parts) == 2:
                            line_num = int(parts[0])
                            hit_count = int(parts[1])

                            file_data = coverage_data["files"][current_file]
                            file_data["line_total"] += 1
                            if hit_count > 0:
                                file_data["line_hit"] += 1
                            file_data["lines"][line_num] = hit_count

                    elif line.startswith('BRDA:') and current_file:
                        # 分支覆盖率数据: BRDA:line,block,branch,hit_count
                        parts = line[5:].split(',')
                        if len(parts) == 4:
                            hit_count = int(parts[3])
                            file_data = coverage_data["files"][current_file]
                            file_data["branch_total"] += 1
                            if hit_count > 0:
                                file_data["branch_hit"] += 1

                    elif line.startswith('FN:') and current_file:
                        # 函数定义: FN:line,function_name
                        file_data = coverage_data["files"][current_file]
                        file_data["function_total"] += 1

                    elif line.startswith('FNDA:') and current_file:
                        # 函数覆盖率: FNDA:hit_count,function_name
                        parts = line[5:].split(',')
                        if len(parts) == 2:
                            hit_count = int(parts[0])
                            file_data = coverage_data["files"][current_file]
                            if hit_count > 0:
                                file_data["function_hit"] += 1

                    elif line.startswith('end_of_record'):
                        current_file = None

            # 计算汇总数据
            self._calculate_summary(coverage_data)

        except Exception as e:
            print(f"错误: 解析覆盖率数据失败: {e}")

        return coverage_data

    def _get_module_from_path(self, file_path: str) -> str:
        """从文件路径确定模块名称"""
        path_parts = Path(file_path).parts

        # 根据路径结构确定模块
        if 'src' in path_parts:
            src_index = path_parts.index('src')
            if src_index + 1 < len(path_parts):
                module = path_parts[src_index + 1]
                # 标准化模块名称
                module_map = {
                    'sql_parser': 'SQL解析器',
                    'storage_engine': '存储引擎',
                    'network': '网络服务',
                    'execution': '执行引擎',
                    'core': '核心模块',
                    'sqlcc_server': '服务器',
                    'isql_network': '网络客户端'
                }
                return module_map.get(module, module)

        # 默认分类
        if 'include' in file_path:
            return '头文件'
        elif 'test' in file_path or 'tests' in file_path:
            return '测试文件'
        else:
            return '其他'

    def _calculate_summary(self, coverage_data: Dict[str, Any]):
        """计算汇总覆盖率数据"""
        summary = coverage_data["summary"]

        # 汇总所有文件的覆盖率数据
        for file_data in coverage_data["files"].values():
            summary["line_hit"] += file_data["line_hit"]
            summary["line_total"] += file_data["line_total"]
            summary["branch_hit"] += file_data["branch_hit"]
            summary["branch_total"] += file_data["branch_total"]
            summary["function_hit"] += file_data["function_hit"]
            summary["function_total"] += file_data["function_total"]

        # 计算覆盖率百分比
        summary["line_coverage"] = (
            summary["line_hit"] / summary["line_total"] * 100
            if summary["line_total"] > 0 else 0.0
        )
        summary["branch_coverage"] = (
            summary["branch_hit"] / summary["branch_total"] * 100
            if summary["branch_total"] > 0 else 0.0
        )
        summary["function_coverage"] = (
            summary["function_hit"] / summary["function_total"] * 100
            if summary["function_total"] > 0 else 0.0
        )

        # 计算各模块覆盖率
        for module_name, module_data in coverage_data["modules"].items():
            for file_path in module_data["files"]:
                if file_path in coverage_data["files"]:
                    file_data = coverage_data["files"][file_path]
                    module_data["line_hit"] += file_data["line_hit"]
                    module_data["line_total"] += file_data["line_total"]
                    module_data["branch_hit"] += file_data["branch_hit"]
                    module_data["branch_total"] += file_data["branch_total"]
                    module_data["function_hit"] += file_data["function_hit"]
                    module_data["function_total"] += file_data["function_total"]

    def generate_module_report(self, coverage_data: Dict[str, Any]) -> str:
        """生成模块级覆盖率报告"""
        report = """================================================================================
                         SQLCC 模块级覆盖率分析报告
================================================================================

生成时间: {timestamp}

================================================================================
总体覆盖率汇总
================================================================================

行覆盖率: {line_coverage:.1f}% ({line_hit}/{line_total})
分支覆盖率: {branch_coverage:.1f}% ({branch_hit}/{branch_total})
函数覆盖率: {function_coverage:.1f}% ({function_hit}/{function_total})

================================================================================
模块覆盖率详情
================================================================================
""".format(
            timestamp=datetime.now().strftime('%Y-%m-%d %H:%M:%S'),
            **coverage_data["summary"]
        )

        # 按覆盖率排序模块
        modules = coverage_data["modules"]
        sorted_modules = sorted(
            modules.items(),
            key=lambda x: x[1]["line_hit"] / x[1]["line_total"] if x[1]["line_total"] > 0 else 0,
            reverse=True
        )

        for module_name, module_data in sorted_modules:
            if module_data["line_total"] == 0:
                continue

            line_cov = (module_data["line_hit"] / module_data["line_total"] * 100
                       if module_data["line_total"] > 0 else 0.0)
            branch_cov = (module_data["branch_hit"] / module_data["branch_total"] * 100
                         if module_data["branch_total"] > 0 else 0.0)
            function_cov = (module_data["function_hit"] / module_data["function_total"] * 100
                           if module_data["function_total"] > 0 else 0.0)

            report += """
模块: {module_name}
├── 文件数量: {file_count}
├── 行覆盖率: {line_cov:.1f}% ({line_hit}/{line_total})
├── 分支覆盖率: {branch_cov:.1f}% ({branch_hit}/{branch_total})
└── 函数覆盖率: {function_cov:.1f}% ({function_hit}/{function_total})""".format(
                module_name=module_name,
                file_count=len(module_data["files"]),
                line_cov=line_cov,
                branch_cov=branch_cov,
                function_cov=function_cov,
                **module_data
            )

        report += """

================================================================================
覆盖率改进建议
================================================================================

"""

        # 生成改进建议
        low_coverage_modules = [
            (name, data) for name, data in modules.items()
            if data["line_total"] > 0 and
            (data["line_hit"] / data["line_total"]) < 0.5  # 低于50%
        ]

        if low_coverage_modules:
            report += "需要重点改进的模块 (覆盖率 < 50%):\n"
            for module_name, module_data in sorted(low_coverage_modules,
                                                  key=lambda x: x[1]["line_hit"] / x[1]["line_total"]):
                line_cov = module_data["line_hit"] / module_data["line_total"] * 100
                report += f"- {module_name}: {line_cov:.1f}% ({module_data['line_hit']}/{module_data['line_total']})\n"
        else:
            report += "所有模块覆盖率均良好 (> 50%)\n"

        # 分析未覆盖的文件
        uncovered_files = [
            file_path for file_path, file_data in coverage_data["files"].items()
            if file_data["line_hit"] == 0 and file_data["line_total"] > 0
        ]

        if uncovered_files:
            report += f"\n完全未覆盖的文件 ({len(uncovered_files)} 个):\n"
            for file_path in uncovered_files[:10]:  # 最多显示10个
                report += f"- {file_path}\n"
            if len(uncovered_files) > 10:
                report += f"... 还有 {len(uncovered_files) - 10} 个文件\n"

        return report

    def generate_trend_report(self, days: int = 30) -> str:
        """生成覆盖率趋势报告"""
        from test_status_tracker import TestStatusTracker

        tracker = TestStatusTracker()
        trend_data = tracker.get_coverage_trend(days)

        if not trend_data:
            return f"没有找到最近 {days} 天的覆盖率数据"

        report = f"""================================================================================
                         SQLCC 覆盖率趋势分析报告
================================================================================

分析时间范围: 最近 {days} 天
数据点数量: {len(trend_data)}

================================================================================
趋势数据
================================================================================

日期                      行覆盖率    分支覆盖率  函数覆盖率  测试时长(秒)
{'-'*75}
"""

        for entry in trend_data:
            timestamp = entry["timestamp"][:19]  # YYYY-MM-DD HH:MM:SS
            coverage = entry["coverage"]
            duration = entry.get("duration", 0)

            line_cov = coverage.get("line", 0)
            branch_cov = coverage.get("branch", 0)
            function_cov = coverage.get("function", 0)

            report += f"{timestamp:<24} {line_cov:>8.1f}% {branch_cov:>10.1f}% {function_cov:>10.1f}% {duration:>12}\n"

        # 计算趋势
        if len(trend_data) >= 2:
            first_entry = trend_data[0]["coverage"]
            last_entry = trend_data[-1]["coverage"]

            line_trend = last_entry.get("line", 0) - first_entry.get("line", 0)
            branch_trend = last_entry.get("branch", 0) - first_entry.get("branch", 0)
            function_trend = last_entry.get("function", 0) - first_entry.get("function", 0)

            report += f"""

================================================================================
趋势分析
================================================================================

行覆盖率变化: {line_trend:+.1f}%
分支覆盖率变化: {branch_trend:+.1f}%
函数覆盖率变化: {function_trend:+.1f}%

趋势状态: {'上升' if line_trend > 0 else '下降' if line_trend < 0 else '稳定'}
"""

        return report

    def create_coverage_chart(self, trend_data: List[Dict[str, Any]], output_file: str = "coverage_trend.png"):
        """创建覆盖率趋势图表"""
        if not trend_data:
            print("没有足够的数据生成图表")
            return

        try:
            # 提取数据
            dates = []
            line_coverages = []
            branch_coverages = []
            function_coverages = []

            for entry in trend_data:
                date_obj = datetime.fromisoformat(entry["timestamp"])
                dates.append(date_obj)

                coverage = entry["coverage"]
                line_coverages.append(coverage.get("line", 0))
                branch_coverages.append(coverage.get("branch", 0))
                function_coverages.append(coverage.get("function", 0))

            # 创建图表
            plt.figure(figsize=(12, 6))

            plt.plot(dates, line_coverages, 'b-o', label='行覆盖率', linewidth=2, markersize=4)
            plt.plot(dates, branch_coverages, 'r-s', label='分支覆盖率', linewidth=2, markersize=4)
            plt.plot(dates, function_coverages, 'g-^', label='函数覆盖率', linewidth=2, markersize=4)

            plt.xlabel('日期')
            plt.ylabel('覆盖率 (%)')
            plt.title('SQLCC 覆盖率趋势图')
            plt.legend()
            plt.grid(True, alpha=0.3)

            # 格式化x轴日期
            plt.gcf().autofmt_xdate()

            # 保存图表
            plt.savefig(output_file, dpi=150, bbox_inches='tight')
            plt.close()

            print(f"覆盖率趋势图已保存到: {output_file}")

        except ImportError:
            print("警告: matplotlib 未安装，无法生成图表")
        except Exception as e:
            print(f"错误: 生成图表失败: {e}")

    def analyze_uncovered_code(self, coverage_data: Dict[str, Any]) -> str:
        """分析未覆盖的代码"""
        report = """================================================================================
                         SQLCC 未覆盖代码分析
================================================================================

"""

        # 查找未覆盖的文件
        uncovered_files = []
        partially_covered_files = []

        for file_path, file_data in coverage_data["files"].items():
            if file_data["line_total"] == 0:
                continue

            coverage_ratio = file_data["line_hit"] / file_data["line_total"]
            if coverage_ratio == 0.0:
                uncovered_files.append((file_path, file_data))
            elif coverage_ratio < 0.8:  # 覆盖率低于80%
                partially_covered_files.append((file_path, file_data, coverage_ratio))

        report += f"完全未覆盖的文件: {len(uncovered_files)} 个\n"
        report += f"部分覆盖的文件 (<80%): {len(partially_covered_files)} 个\n\n"

        if uncovered_files:
            report += "=== 完全未覆盖的文件 ===\n"
            for file_path, file_data in uncovered_files[:20]:  # 最多显示20个
                report += f"- {file_path} ({file_data['line_total']} 行)\n"
            if len(uncovered_files) > 20:
                report += f"... 还有 {len(uncovered_files) - 20} 个文件\n"

        if partially_covered_files:
            report += "\n=== 部分覆盖的文件 ===\n"
            # 按覆盖率排序
            partially_covered_files.sort(key=lambda x: x[2])

            for file_path, file_data, coverage_ratio in partially_covered_files[:20]:
                report += f"- {file_path}: {coverage_ratio:.1f}% ({file_data['line_hit']}/{file_data['line_total']})\n"
            if len(partially_covered_files) > 20:
                report += f"... 还有 {len(partially_covered_files) - 20} 个文件\n"

        # 分析未覆盖的行
        report += "\n=== 未覆盖代码行分析 ===\n"

        total_uncovered_lines = 0
        module_uncovered = defaultdict(int)

        for file_path, file_data in coverage_data["files"].items():
            uncovered_lines = [
                line_num for line_num, hit_count in file_data["lines"].items()
                if hit_count == 0
            ]

            if uncovered_lines:
                total_uncovered_lines += len(uncovered_lines)
                module = self._get_module_from_path(file_path)
                module_uncovered[module] += len(uncovered_lines)

        report += f"总未覆盖行数: {total_uncovered_lines}\n\n"
        report += "按模块统计:\n"

        for module, count in sorted(module_uncovered.items(), key=lambda x: x[1], reverse=True):
            report += f"- {module}: {count} 行\n"

        return report

    def run_full_analysis(self, output_dir: str = "test_reports"):
        """运行完整覆盖率分析"""
        output_dir = Path(output_dir)
        output_dir.mkdir(exist_ok=True)

        print("=== SQLCC 增强覆盖率分析器 ===")
        print(f"分析时间: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
        print(f"输出目录: {output_dir}")
        print()

        # 查找覆盖率数据文件
        info_file = self.coverage_data_dir / "_coverage_report.dat"
        if not info_file.exists():
            # 尝试其他可能的位置
            possible_files = [
                Path("bazel-out/_coverage/_coverage_report.dat"),
                Path("coverage.info"),
                Path("lcov.info")
            ]

            for possible_file in possible_files:
                if possible_file.exists():
                    info_file = possible_file
                    break

        if not info_file.exists():
            print(f"错误: 未找到覆盖率数据文件")
            print("请确保已运行过覆盖率测试: bazel coverage //tests/... --combined_report=lcov")
            return False

        print(f"使用覆盖率数据文件: {info_file}")

        # 解析覆盖率数据
        print("解析覆盖率数据...")
        coverage_data = self.parse_lcov_info(info_file)

        # 生成模块报告
        print("生成模块覆盖率报告...")
        module_report = self.generate_module_report(coverage_data)
        module_report_file = output_dir / f"module_coverage_report_{datetime.now().strftime('%Y%m%d_%H%M%S')}.txt"

        with open(module_report_file, 'w', encoding='utf-8') as f:
            f.write(module_report)

        print(f"模块报告已保存: {module_report_file}")

        # 生成趋势报告
        print("生成趋势分析报告...")
        trend_report = self.generate_trend_report(days=30)
        trend_report_file = output_dir / f"coverage_trend_report_{datetime.now().strftime('%Y%m%d_%H%M%S')}.txt"

        with open(trend_report_file, 'w', encoding='utf-8') as f:
            f.write(trend_report)

        print(f"趋势报告已保存: {trend_report_file}")

        # 生成未覆盖代码分析
        print("分析未覆盖代码...")
        uncovered_report = self.analyze_uncovered_code(coverage_data)
        uncovered_report_file = output_dir / f"uncovered_code_analysis_{datetime.now().strftime('%Y%m%d_%H%M%S')}.txt"

        with open(uncovered_report_file, 'w', encoding='utf-8') as f:
            f.write(uncovered_report)

        print(f"未覆盖代码分析已保存: {uncovered_report_file}")

        # 创建趋势图表
        print("生成覆盖率趋势图...")
        from test_status_tracker import TestStatusTracker
        tracker = TestStatusTracker()
        trend_data = tracker.get_coverage_trend(days=30)

        if trend_data:
            chart_file = output_dir / f"coverage_trend_{datetime.now().strftime('%Y%m%d_%H%M%S')}.png"
            self.create_coverage_chart(trend_data, str(chart_file))
        else:
            print("没有足够的历史数据生成趋势图")

        # 保存覆盖率数据为JSON
        json_file = output_dir / f"coverage_data_{datetime.now().strftime('%Y%m%d_%H%M%S')}.json"
        with open(json_file, 'w', encoding='utf-8') as f:
            json.dump(coverage_data, f, indent=2, ensure_ascii=False)

        print(f"覆盖率数据已保存: {json_file}")

        # 打印摘要
        summary = coverage_data["summary"]
        print("
=== 分析完成摘要 ===")
        print(".1f")
        print(".1f")
        print(".1f")
        print(f"分析的文件数: {len(coverage_data['files'])}")
        print(f"涉及的模块数: {len(coverage_data['modules'])}")

        print("
生成的文件:")
        print(f"- 模块报告: {module_report_file}")
        print(f"- 趋势报告: {trend_report_file}")
        print(f"- 未覆盖分析: {uncovered_report_file}")
        if trend_data:
            print(f"- 趋势图表: {chart_file}")
        print(f"- 数据文件: {json_file}")

        return True

def main():
    """主函数"""
    import argparse

    parser = argparse.ArgumentParser(description="SQLCC 增强覆盖率分析器")
    parser.add_argument("action", choices=["analyze", "trend", "uncovered", "chart"],
                       help="执行的操作")
    parser.add_argument("--input", type=str,
                       help="覆盖率数据文件路径")
    parser.add_argument("--output", type=str, default="test_reports",
                       help="输出目录")
    parser.add_argument("--days", type=int, default=30,
                       help="趋势分析天数")
    parser.add_argument("--chart-output", type=str,
                       help="图表输出文件路径")

    args = parser.parse_args()

    analyzer = EnhancedCoverageAnalyzer()

    if args.action == "analyze":
        success = analyzer.run_full_analysis(args.output)
        sys.exit(0 if success else 1)

    elif args.action == "trend":
        report = analyzer.generate_trend_report(args.days)
        print(report)

    elif args.action == "uncovered":
        # 需要先解析数据
        info_file = Path(args.input) if args.input else Path("bazel-out/_coverage/_coverage_report.dat")
        coverage_data = analyzer.parse_lcov_info(info_file)
        report = analyzer.analyze_uncovered_code(coverage_data)
        print(report)

    elif args.action == "chart":
        from test_status_tracker import TestStatusTracker
        tracker = TestStatusTracker()
        trend_data = tracker.get_coverage_trend(args.days)

        output_file = args.chart_output or f"coverage_trend_{datetime.now().strftime('%Y%m%d_%H%M%S')}.png"
        analyzer.create_coverage_chart(trend_data, output_file)

if __name__ == "__main__":
    main()
