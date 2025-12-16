
生成时间: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}
报告ID: {timestamp}

测试系统状态总览

总运行次数: {status_summary['total_runs']}
成功率: {status_summary['success_rate']:.1f}%
平均时长: {status_summary['average_duration']:.1f}秒
最后成功运行: {status_summary['last_successful_run'] or '无'}

最佳覆盖率记录:
- 行覆盖率: {status_summary['best_coverage']['line']:.1f}%
- 分支覆盖率: {status_summary['best_coverage']['branch']:.1f}%
- 函数覆盖率: {status_summary['best_coverage']['function']:.1f}%

详细测试结果

"""

        # 包含最新的测试报告内容
        if latest_incremental and latest_incremental.exists():
            report_content += f"\n=== 最新测试执行报告 ({latest_incremental.name}) ===\n\n"
            try:
                with open(latest_incremental, 'r', encoding='utf-8') as f:
                    # 只包含关键部分
                    content = f.read()
                    # 提取测试执行摘要部分
                    lines = content.split('\n')
                    in_summary = False
                    for line in lines:
                        if '测试执行摘要' in line:
                            in_summary = True
                        elif in_summary and line.startswith('==='):
                            break
                        elif in_summary:
                            report_content += line + '\n'
            except Exception as e:
                report_content += f"无法读取测试报告: {e}\n"

        if latest_coverage and latest_coverage.exists():
            report_content += f"\n=== 最新覆盖率分析报告 ({latest_coverage.name}) ===\n\n"
            try:
                with open(latest_coverage, 'r', encoding='utf-8') as f:
                    content = f.read()
                    # 提取总体覆盖率汇总部分
                    lines = content.split('\n')
                    in_summary = False
                    for line in lines:
                        if '总体覆盖率汇总' in line:
                            in_summary = True
                        elif in_summary and line.startswith('==='):
                            if '模块覆盖率详情' not in line:
                                break
                        elif in_summary:
                            report_content += line + '\n'
            except Exception as e:
                report_content += f"无法读取覆盖率报告: {e}\n"

        # 添加质量评估
        report_content += """

质量评估与建议

"""

        # 基于覆盖率和测试结果进行评估
        overall_quality = "unknown"
        recommendations = []

        if status_summary['total_runs'] > 0:
            success_rate = status_summary['success_rate']

            if success_rate >= 90:
                overall_quality = "优秀"
                report_content += "✅ 测试质量: 优秀\n"
            elif success_rate >= 75:
                overall_quality = "良好"
                report_content += "⚠️  测试质量: 良好\n"
                recommendations.append("- 关注失败的测试用例，提升成功率")
            else:
                overall_quality = "需改进"
                report_content += "❌ 测试质量: 需改进\n"
                recommendations.append("- 优先修复编译和运行时错误")
                recommendations.append("- 检查测试环境配置")

        # 覆盖率评估
        line_coverage = status_summary['best_coverage']['line']
        if line_coverage >= 50:
            report_content += "✅ 覆盖率水平: 良好\n"
        elif line_coverage >= 20:
            report_content += "⚠️  覆盖率水平: 一般\n"
            recommendations.append("- 增加单元测试覆盖高风险模块")
        else:
            report_content += "❌ 覆盖率水平: 严重不足\n"
            recommendations.append("- 紧急提升测试覆盖率，目标30%+")

        # 生成建议
        if recommendations:
            report_content += "\n改进建议:\n"
            for rec in recommendations:
                report_content += f"{rec}\n"

        # 技术债务分析
        report_content += """

技术债务分析:
- 测试自动化程度: 高 (CI/CD集成完善)
- 测试维护成本: 中 (需要定期更新测试用例)
- 覆盖率监控: 已实现 (趋势分析和报告)
- 失败原因跟踪: 已实现 (详细错误日志)

"""

        # 保存报告
        with open(report_file, 'w', encoding='utf-8') as f:
            f.write(report_content)

        print(f"✅ 综合报告已生成: {report_file}")

        # 显示关键指标
        print("
=== 关键指标摘要 ===")
        print(f"测试成功率: {status_summary['success_rate']:.1f}%")
        print(f"最佳行覆盖率: {status_summary['best_coverage']['line']:.1f}%")
        print(f"测试运行次数: {status_summary['total_runs']}")
        print(f"平均测试时长: {status_summary['average_duration']:.1f}秒")

        return str(report_file)

    def run_full_test_cycle(self, mode: str = "full", include_coverage: bool = True,
                          verbose: bool = False) -> bool:
        """运行完整的测试周期"""
测试模式: {mode}
覆盖率分析: {include_coverage}
详细输出: {verbose}
开始时间: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}
".format(
            mode=mode,
            include_coverage=include_coverage,
            verbose=verbose
        ))
        print(f"""================================================================================
                    SQLCC 完整测试周期开始
测试模式: {mode}
覆盖率分析: {include_coverage}
详细输出: {verbose}
开始时间: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}
""")
测试模式: {mode}
覆盖率分析: {include_coverage}
详细输出: {verbose}
开始时间: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}
".format(
            mode=mode,
            include_coverage=include_coverage,
            verbose=verbose
        ))

        run_id = f"full_cycle_{datetime.now().strftime('%Y%m%d_%H%M%S')}"

        # 开始测试运行跟踪
        self.tracker.start_run(run_id, mode)

        success = True
        start_time = datetime.now()

        try:
            # 步骤1: 构建检查
            print("\n[步骤1/4] 构建检查")
            self.tracker.update_phase("build_check", "running")

            if not self.run_build_checks():
                self.tracker.update_phase("build_check", "failed")
                self.tracker.record_error("build_check", "build_failure", "构建验证失败")
                if not verbose:  # 非详细模式下失败时退出
                    success = False
            else:
                self.tracker.update_phase("build_check", "completed")

            # 步骤2: 逐步递进测试
            if success or verbose:  # 详细模式下继续执行
                print("\n[步骤2/4] 逐步递进测试")
                self.tracker.update_phase("incremental_tests", "running")

                if not self.run_incremental_tests(mode, verbose, False):  # 不使用fail-fast
                    self.tracker.update_phase("incremental_tests", "failed")
                    success = False
                else:
                    self.tracker.update_phase("incremental_tests", "completed")

            # 步骤3: 覆盖率分析
            if include_coverage and (success or verbose):
                print("\n[步骤3/4] 覆盖率分析")
                self.tracker.update_phase("coverage_analysis", "running")

                if self.run_coverage_analysis():
                    self.tracker.update_phase("coverage_analysis", "completed")

                    # 更新覆盖率数据到跟踪器
                    # 这里可以从最新报告中提取覆盖率数据
                    try:
                        coverage_reports = list(self.test_reports_dir.glob("coverage_data_*.json"))
                        if coverage_reports:
                            latest_coverage_file = max(coverage_reports)
                            with open(latest_coverage_file, 'r', encoding='utf-8') as f:
                                coverage_data = json.load(f)
                                summary = coverage_data.get("summary", {})
                                self.tracker.update_coverage({
                                    "line": summary.get("line_coverage", 0),
                                    "branch": summary.get("branch_coverage", 0),
                                    "function": summary.get("function_coverage", 0)
                                })
                    except Exception as e:
                        print(f"警告: 无法更新覆盖率数据: {e}")

                else:
                    self.tracker.update_phase("coverage_analysis", "failed")
                    success = False

            # 步骤4: 生成最终报告
            print("\n[步骤4/4] 生成综合报告")
            self.tracker.update_phase("report_generation", "running")

            report_file = self.generate_final_report()
            self.tracker.update_phase("report_generation", "completed",
                                    {"report_file": report_file})

        except Exception as e:
            print(f"\n❌ 测试周期执行异常: {e}")
            success = False
            self.tracker.record_error("system", "execution_error", str(e))

        # 完成测试运行
        end_time = datetime.now()
        duration = int((end_time - start_time).total_seconds())

        final_status = "success" if success else "failed"
        self.tracker.finish_run(final_status, duration)

        print("
测试周期完成
状态: {final_status.upper()}
总时长: {duration}秒
报告位置: test_reports/
".format(final_status=final_status))

        return success

def main():
    """主函数"""
    parser = argparse.ArgumentParser(
        description="SQLCC 统一测试系统",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
使用示例:
  # 运行完整测试周期
  python sqlcc_test_system.py run-full

  # 只运行构建检查
  python sqlcc_test_system.py build-check

  # 生成测试状态报告
  python sqlcc_test_system.py report

  # 运行覆盖率分析
  python sqlcc_test_system.py coverage

  # 查看测试历史
  python sqlcc_test_system.py history
        """
    )

    parser.add_argument("action", choices=[
        "run-full", "build-check", "test-only", "coverage", "report",
        "status", "history", "trend"
    ], help="执行的操作")

    parser.add_argument("--mode", choices=["unit_only", "integration", "full", "performance"],
                       default="full", help="测试模式 (默认: full)")

    parser.add_argument("--no-coverage", action="store_true",
                       help="跳过覆盖率分析")

    parser.add_argument("--verbose", action="store_true",
                       help="详细输出模式")

    parser.add_argument("--fail-fast", action="store_true",
                       help="遇到失败立即停止")

    args = parser.parse_args()

    system = SQLCCTestSystem()

    if args.action == "run-full":
        include_coverage = not args.no_coverage
        success = system.run_full_test_cycle(args.mode, include_coverage, args.verbose)
        sys.exit(0 if success else 1)

    elif args.action == "build-check":
        success = system.run_build_checks()
        sys.exit(0 if success else 1)

    elif args.action == "test-only":
        success = system.run_incremental_tests(args.mode, args.verbose, args.fail_fast)
        sys.exit(0 if success else 1)

    elif args.action == "coverage":
        success = system.run_coverage_analysis()
        sys.exit(0 if success else 1)

    elif args.action == "report":
        report_file = system.generate_final_report()
        print(f"综合报告已生成: {report_file}")

    elif args.action == "status":
        summary = system.tracker.get_status_summary()
        print(json.dumps(summary, indent=2, ensure_ascii=False))

    elif args.action == "history":
        history = system.tracker.get_recent_runs(10)
        for run in history:
            status_icon = "✅" if run["status"] == "success" else "❌"
            duration = run.get("duration", 0)
            print(f"{run['run_id']}: {status_icon} {run['status']} ({duration}s)")

    elif args.action == "trend":
        trend = system.tracker.get_coverage_trend(7)
        if trend:
            print("最近7天覆盖率趋势:")
            for entry in trend:
                timestamp = entry["timestamp"][:19]
                coverage = entry["coverage"]
                line_cov = coverage.get("line", 0)
                print(f"  {timestamp}: 行覆盖率 {line_cov:.1f}%")
        else:
            print("没有找到覆盖率趋势数据")

if __name__ == "__main__":
    main()
#!/usr/bin/env python3
# SQLCC 统一测试系统
# 提供完整的测试执行、状态跟踪和报告生成功能

import os
import sys
import json
import subprocess
import argparse
from datetime import datetime
from pathlib import Path

# 添加脚本目录到Python路径
script_dir = Path(__file__).parent
sys.path.insert(0, str(script_dir))

try:
    from test_status_tracker import TestStatusTracker
    from enhanced_coverage_analyzer import EnhancedCoverageAnalyzer
    TRACKER_AVAILABLE = True
except ImportError:
    TRACKER_AVAILABLE = False

class SQLCCTestSystem:
    """SQLCC 统一测试系统"""

    def __init__(self):
        self.project_root = Path.cwd()
        self.test_reports_dir = self.project_root / "test_reports"
        self.test_reports_dir.mkdir(exist_ok=True)

        if TRACKER_AVAILABLE:
            self.tracker = TestStatusTracker()
            self.analyzer = EnhancedCoverageAnalyzer()
        else:
            self.tracker = None
            self.analyzer = None

    def run_build_checks(self):
        """运行构建检查"""
        print("=== 构建检查 ===")

        try:
            result = subprocess.run(
                ["./scripts/validate_build.sh"],
                capture_output=True,
                text=True,
                cwd=self.project_root
            )

            if result.returncode == 0:
                print("✅ 构建验证通过")
                return True
            else:
                print("❌ 构建验证失败")
                print("错误输出:")
                print(result.stderr)
                return False

        except Exception as e:
            print(f"❌ 构建检查执行失败: {e}")
            return False

    def run_incremental_tests(self, mode="full", verbose=False, fail_fast=False):
        """运行逐步递进测试"""
        print(f"=== 运行逐步递进测试 (模式: {mode}) ===")

        cmd = [str(script_dir / "run_incremental_tests.sh"), f"--mode={mode}"]

        if verbose:
            cmd.append("--verbose")
        if fail_fast:
            cmd.append("--fail-fast")

        try:
            result = subprocess.run(cmd, cwd=self.project_root)
            return result.returncode == 0

        except Exception as e:
            print(f"❌ 测试执行失败: {e}")
            return False

    def run_coverage_analysis(self):
        """运行覆盖率分析"""
        print("=== 运行覆盖率分析 ===")

        try:
            cmd = [str(script_dir / "enhanced_coverage_analyzer.py"), "analyze"]
            result = subprocess.run(cmd, cwd=self.project_root)
            return result.returncode == 0

        except Exception as e:
            print(f"❌ 覆盖率分析失败: {e}")
            return False

    def generate_final_report(self):
        """生成最终综合报告"""
        print("=== 生成最终综合报告 ===")

        timestamp = datetime.now().strftime('%Y%m%d_%H%M%S')
        report_file = self.test_reports_dir / f"comprehensive_test_report_{timestamp}.txt"

        # 获取最新测试报告
        incremental_reports = list(self.test_reports_dir.glob("incremental_test_report_*.txt"))
        coverage_reports = list(self.test_reports_dir.glob("module_coverage_report_*.txt"))

        latest_incremental = max(incremental_reports) if incremental_reports else None
        latest_coverage = max(coverage_reports) if coverage_reports else None

        # 生成综合报告
        report_content = f"""================================================================================
                         SQLCC 综合测试报告

生成时间: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}
报告ID: {timestamp}

测试执行结果汇总

"""

        # 包含最新的测试报告内容
        if latest_incremental and latest_incremental.exists():
            report_content += f"\n=== 最新测试执行报告 ({latest_incremental.name}) ===\n\n"
            try:
                with open(latest_incremental, 'r', encoding='utf-8') as f:
                    content = f.read()
                    # 查找关键摘要部分
                    if "测试执行摘要" in content:
                        start = content.find("测试执行摘要")
                        end = content.find("================================================================================", start + 1)
                        if end > start:
                            report_content += content[start:end]
            except Exception as e:
                report_content += f"无法读取测试报告: {e}\n"

        if latest_coverage and latest_coverage.exists():
            report_content += f"\n=== 最新覆盖率分析报告 ({latest_coverage.name}) ===\n\n"
            try:
                with open(latest_coverage, 'r', encoding='utf-8') as f:
                    content = f.read()
                    # 查找覆盖率汇总部分
                    if "总体覆盖率汇总" in content:
                        start = content.find("总体覆盖率汇总")
                        end = content.find("================================================================================", start + 1)
                        if end > start:
                            report_content += content[start:end]
            except Exception as e:
                report_content += f"无法读取覆盖率报告: {e}\n"

        report_content += """

测试系统状态

- 构建检查: 已实现 (validate_build.sh)
- 逐步测试: 已实现 (run_incremental_tests.sh)
- 覆盖率分析: 已实现 (enhanced_coverage_analyzer.py)
- 状态跟踪: 已实现 (test_status_tracker.py)
- 报告生成: 已实现 (本报告)

使用指南

# 运行完整测试周期
python scripts/sqlcc_test_system.py run-full

# 只运行构建检查
python scripts/sqlcc_test_system.py build-check

# 只运行测试 (不含覆盖率)
python scripts/sqlcc_test_system.py run-full --no-coverage

# 生成测试报告
python scripts/sqlcc_test_system.py report

# 查看测试状态
python scripts/sqlcc_test_system.py status

报告位置: test_reports/
"""

        # 保存报告
        with open(report_file, 'w', encoding='utf-8') as f:
            f.write(report_content)

        print(f"✅ 综合报告已生成: {report_file}")
        return str(report_file)

    def run_full_test_cycle(self, mode="full", include_coverage=True, verbose=False):
        """运行完整的测试周期"""
        print(f"""================================================================================
                    SQLCC 完整测试周期开始
测试模式: {mode}
覆盖率分析: {include_coverage}
详细输出: {verbose}
开始时间: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}
""")

        success = True
        start_time = datetime.now()

        try:
            # 步骤1: 构建检查
            print("\n[步骤1/4] 构建检查")
            if not self.run_build_checks():
                print("构建检查失败")
                if not verbose:
                    success = False

            # 步骤2: 逐步递进测试
            if success or verbose:
                print("\n[步骤2/4] 逐步递进测试")
                if not self.run_incremental_tests(mode, verbose, False):
                    print("测试执行失败")
                    success = False

            # 步骤3: 覆盖率分析
            if include_coverage and (success or verbose):
                print("\n[步骤3/4] 覆盖率分析")
                if not self.run_coverage_analysis():
                    print("覆盖率分析失败")
                    success = False

            # 步骤4: 生成最终报告
            print("\n[步骤4/4] 生成综合报告")
            report_file = self.generate_final_report()

        except Exception as e:
            print(f"\n❌ 测试周期执行异常: {e}")
            success = False

        # 计算总时长
        end_time = datetime.now()
        duration = int((end_time - start_time).total_seconds())

        status = "SUCCESS" if success else "FAILED"
        print(f"""
测试周期完成
状态: {status}
总时长: {duration}秒
报告位置: test_reports/
""")

        return success

def main():
    """主函数"""
    parser = argparse.ArgumentParser(
        description="SQLCC 统一测试系统",
        epilog="""
使用示例:
  python scripts/sqlcc_test_system.py run-full          # 完整测试周期
  python scripts/sqlcc_test_system.py build-check       # 构建检查
  python scripts/sqlcc_test_system.py report            # 生成报告
  python scripts/sqlcc_test_system.py status            # 查看状态
        """
    )

    parser.add_argument("action", choices=[
        "run-full", "build-check", "test-only", "coverage", "report",
        "status", "history", "trend"
    ], help="执行的操作")

    parser.add_argument("--mode", choices=["unit_only", "integration", "full", "performance"],
                       default="full", help="测试模式")

    parser.add_argument("--no-coverage", action="store_true",
                       help="跳过覆盖率分析")

    parser.add_argument("--verbose", action="store_true",
                       help="详细输出")

    args = parser.parse_args()

    system = SQLCCTestSystem()

    if args.action == "run-full":
        include_coverage = not args.no_coverage
        success = system.run_full_test_cycle(args.mode, include_coverage, args.verbose)
        sys.exit(0 if success else 1)

    elif args.action == "build-check":
        success = system.run_build_checks()
        sys.exit(0 if success else 1)

    elif args.action == "test-only":
        success = system.run_incremental_tests(args.mode, args.verbose, True)
        sys.exit(0 if success else 1)

    elif args.action == "coverage":
        success = system.run_coverage_analysis()
        sys.exit(0 if success else 1)

    elif args.action == "report":
        report_file = system.generate_final_report()
        print(f"综合报告已生成: {report_file}")

    elif args.action == "status":
        if TRACKER_AVAILABLE:
            summary = system.tracker.get_status_summary()
            print(json.dumps(summary, indent=2, ensure_ascii=False))
        else:
            print("状态跟踪功能不可用")

    elif args.action == "history":
        if TRACKER_AVAILABLE:
            history = system.tracker.get_recent_runs(10)
            for run in history:
                status_icon = "✅" if run["status"] == "success" else "❌"
                duration = run.get("duration", 0)
                print(f"{run['run_id']}: {status_icon} {run['status']} ({duration}s)")
        else:
            print("历史记录功能不可用")

    elif args.action == "trend":
        if TRACKER_AVAILABLE:
            trend = system.tracker.get_coverage_trend(7)
            if trend:
                print("最近7天覆盖率趋势:")
                for entry in trend:
                    timestamp = entry["timestamp"][:19]
                    coverage = entry["coverage"]
                    line_cov = coverage.get("line", 0)
                    print(f"  {timestamp}: 行覆盖率 {line_cov:.1f}%")
            else:
                print("没有找到覆盖率趋势数据")
        else:
            print("趋势分析功能不可用")

if __name__ == "__main__":
    main()
================================================================================

生成时间: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}
报告ID: {timestamp}

================================================================================
测试系统状态总览
================================================================================

总运行次数: {status_summary['total_runs']}
成功率: {status_summary['success_rate']:.1f}%
平均时长: {status_summary['average_duration']:.1f}秒
最后成功运行: {status_summary['last_successful_run'] or '无'}

最佳覆盖率记录:
- 行覆盖率: {status_summary['best_coverage']['line']:.1f}%
- 分支覆盖率: {status_summary['best_coverage']['branch']:.1f}%
- 函数覆盖率: {status_summary['best_coverage']['function']:.1f}%

================================================================================
详细测试结果
================================================================================

"""

        # 包含最新的测试报告内容
        if latest_incremental and latest_incremental.exists():
            report_content += f"\n=== 最新测试执行报告 ({latest_incremental.name}) ===\n\n"
            try:
                with open(latest_incremental, 'r', encoding='utf-8') as f:
                    # 只包含关键部分
                    content = f.read()
                    # 提取测试执行摘要部分
                    lines = content.split('\n')
                    in_summary = False
                    for line in lines:
                        if '测试执行摘要' in line:
                            in_summary = True
                        elif in_summary and line.startswith('==='):
                            break
                        elif in_summary:
                            report_content += line + '\n'
            except Exception as e:
                report_content += f"无法读取测试报告: {e}\n"

        if latest_coverage and latest_coverage.exists():
            report_content += f"\n=== 最新覆盖率分析报告 ({latest_coverage.name}) ===\n\n"
            try:
                with open(latest_coverage, 'r', encoding='utf-8') as f:
                    content = f.read()
                    # 提取总体覆盖率汇总部分
                    lines = content.split('\n')
                    in_summary = False
                    for line in lines:
                        if '总体覆盖率汇总' in line:
                            in_summary = True
                        elif in_summary and line.startswith('==='):
                            if '模块覆盖率详情' not in line:
                                break
                        elif in_summary:
                            report_content += line + '\n'
            except Exception as e:
                report_content += f"无法读取覆盖率报告: {e}\n"

        # 添加质量评估
        report_content += """

================================================================================
质量评估与建议
================================================================================

"""

        # 基于覆盖率和测试结果进行评估
        overall_quality = "unknown"
        recommendations = []

        if status_summary['total_runs'] > 0:
            success_rate = status_summary['success_rate']

            if success_rate >= 90:
                overall_quality = "优秀"
                report_content += "✅ 测试质量: 优秀\n"
            elif success_rate >= 75:
                overall_quality = "良好"
                report_content += "⚠️  测试质量: 良好\n"
                recommendations.append("- 关注失败的测试用例，提升成功率")
            else:
                overall_quality = "需改进"
                report_content += "❌ 测试质量: 需改进\n"
                recommendations.append("- 优先修复编译和运行时错误")
                recommendations.append("- 检查测试环境配置")

        # 覆盖率评估
        line_coverage = status_summary['best_coverage']['line']
        if line_coverage >= 50:
            report_content += "✅ 覆盖率水平: 良好\n"
        elif line_coverage >= 20:
            report_content += "⚠️  覆盖率水平: 一般\n"
            recommendations.append("- 增加单元测试覆盖高风险模块")
        else:
            report_content += "❌ 覆盖率水平: 严重不足\n"
            recommendations.append("- 紧急提升测试覆盖率，目标30%+")

        # 生成建议
        if recommendations:
            report_content += "\n改进建议:\n"
            for rec in recommendations:
                report_content += f"{rec}\n"

        # 技术债务分析
        report_content += """

技术债务分析:
- 测试自动化程度: 高 (CI/CD集成完善)
- 测试维护成本: 中 (需要定期更新测试用例)
- 覆盖率监控: 已实现 (趋势分析和报告)
- 失败原因跟踪: 已实现 (详细错误日志)

"""

        # 保存报告
        with open(report_file, 'w', encoding='utf-8') as f:
            f.write(report_content)

        print(f"✅ 综合报告已生成: {report_file}")

        # 显示关键指标
        print("
=== 关键指标摘要 ===")
        print(f"测试成功率: {status_summary['success_rate']:.1f}%")
        print(f"最佳行覆盖率: {status_summary['best_coverage']['line']:.1f}%")
        print(f"测试运行次数: {status_summary['total_runs']}")
        print(f"平均测试时长: {status_summary['average_duration']:.1f}秒")

        return str(report_file)

    def run_full_test_cycle(self, mode: str = "full", include_coverage: bool = True,
                          verbose: bool = False) -> bool:
        """运行完整的测试周期"""
测试模式: {mode}
覆盖率分析: {include_coverage}
详细输出: {verbose}
开始时间: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}
".format(
            mode=mode,
            include_coverage=include_coverage,
            verbose=verbose
        ))
测试模式: {mode}
覆盖率分析: {include_coverage}
详细输出: {verbose}
开始时间: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}
""")
测试模式: {mode}
覆盖率分析: {include_coverage}
详细输出: {verbose}
开始时间: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}
".format(
            mode=mode,
            include_coverage=include_coverage,
            verbose=verbose
        ))
        print(f"""================================================================================
                    SQLCC 完整测试周期开始
测试模式: {mode}
覆盖率分析: {include_coverage}
详细输出: {verbose}
开始时间: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}
""")
测试模式: {mode}
覆盖率分析: {include_coverage}
详细输出: {verbose}
开始时间: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}
""")
================================================================================
测试模式: {mode}
覆盖率分析: {include_coverage}
详细输出: {verbose}
开始时间: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}
================================================================================
".format(
            mode=mode,
            include_coverage=include_coverage,
            verbose=verbose
        ))

        run_id = f"full_cycle_{datetime.now().strftime('%Y%m%d_%H%M%S')}"

        # 开始测试运行跟踪
        self.tracker.start_run(run_id, mode)

        success = True
        start_time = datetime.now()

        try:
            # 步骤1: 构建检查
            print("\n[步骤1/4] 构建检查")
            self.tracker.update_phase("build_check", "running")

            if not self.run_build_checks():
                self.tracker.update_phase("build_check", "failed")
                self.tracker.record_error("build_check", "build_failure", "构建验证失败")
                if not verbose:  # 非详细模式下失败时退出
                    success = False
            else:
                self.tracker.update_phase("build_check", "completed")

            # 步骤2: 逐步递进测试
            if success or verbose:  # 详细模式下继续执行
                print("\n[步骤2/4] 逐步递进测试")
                self.tracker.update_phase("incremental_tests", "running")

                if not self.run_incremental_tests(mode, verbose, False):  # 不使用fail-fast
                    self.tracker.update_phase("incremental_tests", "failed")
                    success = False
                else:
                    self.tracker.update_phase("incremental_tests", "completed")

            # 步骤3: 覆盖率分析
            if include_coverage and (success or verbose):
                print("\n[步骤3/4] 覆盖率分析")
                self.tracker.update_phase("coverage_analysis", "running")

                if self.run_coverage_analysis():
                    self.tracker.update_phase("coverage_analysis", "completed")

                    # 更新覆盖率数据到跟踪器
                    # 这里可以从最新报告中提取覆盖率数据
                    try:
                        coverage_reports = list(self.test_reports_dir.glob("coverage_data_*.json"))
                        if coverage_reports:
                            latest_coverage_file = max(coverage_reports)
                            with open(latest_coverage_file, 'r', encoding='utf-8') as f:
                                coverage_data = json.load(f)
                                summary = coverage_data.get("summary", {})
                                self.tracker.update_coverage({
                                    "line": summary.get("line_coverage", 0),
                                    "branch": summary.get("branch_coverage", 0),
                                    "function": summary.get("function_coverage", 0)
                                })
                    except Exception as e:
                        print(f"警告: 无法更新覆盖率数据: {e}")

                else:
                    self.tracker.update_phase("coverage_analysis", "failed")
                    success = False

            # 步骤4: 生成最终报告
            print("\n[步骤4/4] 生成综合报告")
            self.tracker.update_phase("report_generation", "running")

            report_file = self.generate_final_report()
            self.tracker.update_phase("report_generation", "completed",
                                    {"report_file": report_file})

        except Exception as e:
            print(f"\n❌ 测试周期执行异常: {e}")
            success = False
            self.tracker.record_error("system", "execution_error", str(e))

        # 完成测试运行
        end_time = datetime.now()
        duration = int((end_time - start_time).total_seconds())

        final_status = "success" if success else "failed"
        self.tracker.finish_run(final_status, duration)

        print("
================================================================================
测试周期完成
================================================================================
状态: {final_status.upper()}
总时长: {duration}秒
报告位置: test_reports/
================================================================================
".format(final_status=final_status))

        return success

def main():
    """主函数"""
    parser = argparse.ArgumentParser(
        description="SQLCC 统一测试系统",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
使用示例:
  # 运行完整测试周期
  python sqlcc_test_system.py run-full

  # 只运行构建检查
  python sqlcc_test_system.py build-check

  # 生成测试状态报告
  python sqlcc_test_system.py report

  # 运行覆盖率分析
  python sqlcc_test_system.py coverage

  # 查看测试历史
  python sqlcc_test_system.py history
        """
    )

    parser.add_argument("action", choices=[
        "run-full", "build-check", "test-only", "coverage", "report",
        "status", "history", "trend"
    ], help="执行的操作")

    parser.add_argument("--mode", choices=["unit_only", "integration", "full", "performance"],
                       default="full", help="测试模式 (默认: full)")

    parser.add_argument("--no-coverage", action="store_true",
                       help="跳过覆盖率分析")

    parser.add_argument("--verbose", action="store_true",
                       help="详细输出模式")

    parser.add_argument("--fail-fast", action="store_true",
                       help="遇到失败立即停止")

    args = parser.parse_args()

    system = SQLCCTestSystem()

    if args.action == "run-full":
        include_coverage = not args.no_coverage
        success = system.run_full_test_cycle(args.mode, include_coverage, args.verbose)
        sys.exit(0 if success else 1)

    elif args.action == "build-check":
        success = system.run_build_checks()
        sys.exit(0 if success else 1)

    elif args.action == "test-only":
        success = system.run_incremental_tests(args.mode, args.verbose, args.fail_fast)
        sys.exit(0 if success else 1)

    elif args.action == "coverage":
        success = system.run_coverage_analysis()
        sys.exit(0 if success else 1)

    elif args.action == "report":
        report_file = system.generate_final_report()
        print(f"综合报告已生成: {report_file}")

    elif args.action == "status":
        summary = system.tracker.get_status_summary()
        print(json.dumps(summary, indent=2, ensure_ascii=False))

    elif args.action == "history":
        history = system.tracker.get_recent_runs(10)
        for run in history:
            status_icon = "✅" if run["status"] == "success" else "❌"
            duration = run.get("duration", 0)
            print(f"{run['run_id']}: {status_icon} {run['status']} ({duration}s)")

    elif args.action == "trend":
        trend = system.tracker.get_coverage_trend(7)
        if trend:
            print("最近7天覆盖率趋势:")
            for entry in trend:
                timestamp = entry["timestamp"][:19]
                coverage = entry["coverage"]
                line_cov = coverage.get("line", 0)
                print(f"  {timestamp}: 行覆盖率 {line_cov:.1f}%")
        else:
            print("没有找到覆盖率趋势数据")

if __name__ == "__main__":
    main()
