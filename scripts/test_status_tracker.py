#!/usr/bin/env python3
# SQLCC 测试状态跟踪器
# 提供测试执行状态管理和历史记录功能

import json
import os
import time
from datetime import datetime
from typing import Dict, List, Any, Optional
from pathlib import Path

class TestStatusTracker:
    """测试状态跟踪器"""

    def __init__(self, status_file: str = "test_reports/test_status.json"):
        self.status_file = Path(status_file)
        self.status_file.parent.mkdir(parents=True, exist_ok=True)
        self.status_data = self._load_status()

    def _load_status(self) -> Dict[str, Any]:
        """加载状态数据"""
        if self.status_file.exists():
            try:
                with open(self.status_file, 'r', encoding='utf-8') as f:
                    return json.load(f)
            except (json.JSONDecodeError, IOError) as e:
                print(f"警告: 无法加载状态文件 {self.status_file}: {e}")
                return self._create_default_status()
        else:
            return self._create_default_status()

    def _create_default_status(self) -> Dict[str, Any]:
        """创建默认状态数据"""
        return {
            "version": "1.0",
            "created_at": datetime.now().isoformat(),
            "last_updated": datetime.now().isoformat(),
            "overall_stats": {
                "total_runs": 0,
                "successful_runs": 0,
                "failed_runs": 0,
                "last_successful_run": None,
                "average_duration": 0,
                "best_coverage": {
                    "line": 0.0,
                    "branch": 0.0,
                    "function": 0.0
                }
            },
            "module_stats": {},
            "test_history": [],
            "current_run": None
        }

    def _save_status(self):
        """保存状态数据"""
        self.status_data["last_updated"] = datetime.now().isoformat()
        try:
            with open(self.status_file, 'w', encoding='utf-8') as f:
                json.dump(self.status_data, f, indent=2, ensure_ascii=False)
        except IOError as e:
            print(f"错误: 无法保存状态文件 {self.status_file}: {e}")

    def start_run(self, run_id: str, mode: str = "full", metadata: Dict[str, Any] = None):
        """开始新的测试运行"""
        run_data = {
            "run_id": run_id,
            "start_time": datetime.now().isoformat(),
            "mode": mode,
            "status": "running",
            "metadata": metadata or {},
            "phases": {},
            "coverage": {},
            "errors": [],
            "end_time": None,
            "duration": None
        }

        self.status_data["current_run"] = run_data
        self._save_status()

    def update_phase(self, phase: str, status: str, details: Dict[str, Any] = None):
        """更新测试阶段状态"""
        if self.status_data["current_run"] is None:
            return

        phase_data = {
            "status": status,
            "timestamp": datetime.now().isoformat(),
            "details": details or {}
        }

        self.status_data["current_run"]["phases"][phase] = phase_data
        self._save_status()

    def record_error(self, phase: str, error_type: str, message: str, details: Dict[str, Any] = None):
        """记录错误"""
        if self.status_data["current_run"] is None:
            return

        error_data = {
            "phase": phase,
            "type": error_type,
            "message": message,
            "timestamp": datetime.now().isoformat(),
            "details": details or {}
        }

        self.status_data["current_run"]["errors"].append(error_data)
        self._save_status()

    def update_coverage(self, coverage_data: Dict[str, float]):
        """更新覆盖率数据"""
        if self.status_data["current_run"] is None:
            return

        self.status_data["current_run"]["coverage"] = coverage_data

        # 更新最佳覆盖率记录
        best_coverage = self.status_data["overall_stats"]["best_coverage"]
        updated = False

        for metric in ["line", "branch", "function"]:
            if metric in coverage_data:
                current_value = coverage_data[metric]
                if current_value > best_coverage.get(metric, 0.0):
                    best_coverage[metric] = current_value
                    updated = True

        if updated:
            self._save_status()

    def finish_run(self, final_status: str, duration: int):
        """完成测试运行"""
        if self.status_data["current_run"] is None:
            return

        current_run = self.status_data["current_run"]
        current_run["status"] = final_status
        current_run["end_time"] = datetime.now().isoformat()
        current_run["duration"] = duration

        # 更新总体统计
        overall_stats = self.status_data["overall_stats"]
        overall_stats["total_runs"] += 1

        if final_status == "success":
            overall_stats["successful_runs"] += 1
            overall_stats["last_successful_run"] = current_run["end_time"]
        else:
            overall_stats["failed_runs"] += 1

        # 更新平均时长
        total_duration = overall_stats["average_duration"] * (overall_stats["total_runs"] - 1) + duration
        overall_stats["average_duration"] = total_duration / overall_stats["total_runs"]

        # 添加到历史记录
        self.status_data["test_history"].append(current_run)

        # 限制历史记录数量（保留最近100次）
        if len(self.status_data["test_history"]) > 100:
            self.status_data["test_history"] = self.status_data["test_history"][-100:]

        # 清空当前运行
        self.status_data["current_run"] = None

        self._save_status()

    def get_status_summary(self) -> Dict[str, Any]:
        """获取状态摘要"""
        stats = self.status_data["overall_stats"]
        current = self.status_data.get("current_run")

        return {
            "total_runs": stats["total_runs"],
            "success_rate": (stats["successful_runs"] / stats["total_runs"] * 100) if stats["total_runs"] > 0 else 0,
            "last_successful_run": stats["last_successful_run"],
            "average_duration": stats["average_duration"],
            "best_coverage": stats["best_coverage"],
            "current_run": {
                "active": current is not None,
                "run_id": current["run_id"] if current else None,
                "status": current["status"] if current else None,
                "mode": current["mode"] if current else None
            } if current else None
        }

    def get_recent_runs(self, limit: int = 10) -> List[Dict[str, Any]]:
        """获取最近的测试运行记录"""
        history = self.status_data["test_history"]
        return history[-limit:] if len(history) >= limit else history

    def get_module_stats(self, module: str) -> Dict[str, Any]:
        """获取模块统计信息"""
        return self.status_data["module_stats"].get(module, {})

    def update_module_stats(self, module: str, stats: Dict[str, Any]):
        """更新模块统计信息"""
        self.status_data["module_stats"][module] = stats
        self._save_status()

    def get_coverage_trend(self, days: int = 30) -> List[Dict[str, Any]]:
        """获取覆盖率趋势数据"""
        history = self.status_data["test_history"]
        cutoff_time = time.time() - (days * 24 * 60 * 60)

        trend_data = []
        for run in history:
            if run.get("status") == "success" and run.get("coverage"):
                run_time = datetime.fromisoformat(run["end_time"]).timestamp()
                if run_time >= cutoff_time:
                    trend_data.append({
                        "timestamp": run["end_time"],
                        "coverage": run["coverage"],
                        "duration": run["duration"]
                    })

        return sorted(trend_data, key=lambda x: x["timestamp"])

    def generate_report(self) -> str:
        """生成状态报告"""
        summary = self.get_status_summary()
        recent_runs = self.get_recent_runs(5)

        report = f"""================================================================================
                         SQLCC 测试状态跟踪报告
================================================================================

生成时间: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}

================================================================================
总体统计
================================================================================

总运行次数: {summary['total_runs']}
成功率: {summary['success_rate']:.1f}%
平均时长: {summary['average_duration']:.1f}秒
最后成功运行: {summary['last_successful_run'] or '无'}

最佳覆盖率:
- 行覆盖率: {summary['best_coverage']['line']:.1f}%
- 分支覆盖率: {summary['best_coverage']['branch']:.1f}%
- 函数覆盖率: {summary['best_coverage']['function']:.1f}%

当前运行状态: {'运行中' if summary['current_run'] and summary['current_run']['active'] else '空闲'}
"""

        if summary['current_run'] and summary['current_run']['active']:
            current = summary['current_run']
            report += f"""
当前运行ID: {current['run_id']}
当前运行模式: {current['mode']}
当前状态: {current['status']}
"""

        if recent_runs:
            report += """

================================================================================
最近运行记录
================================================================================

"""

            for i, run in enumerate(reversed(recent_runs), 1):
                status_icon = "✅" if run["status"] == "success" else "❌"
                duration = run.get("duration", 0)
                coverage = run.get("coverage", {})
                line_cov = coverage.get("line", 0)

                report += f"{i}. {status_icon} {run['run_id']} - {run['mode']}模式\n"
                report += f"   时间: {run['start_time'][:19]}\n"
                report += f"   时长: {duration}秒\n"
                if coverage:
                    report += f"   覆盖率: 行{line_cov:.1f}%\n"
                report += "\n"

        # 覆盖率趋势
        trend = self.get_coverage_trend(7)  # 最近7天
        if trend:
            report += """================================================================================
覆盖率趋势 (最近7天)
================================================================================

"""

            for entry in trend[-7:]:  # 最多显示7条
                timestamp = entry["timestamp"][:19]
                coverage = entry["coverage"]
                line_cov = coverage.get("line", 0)
                report += f"{timestamp}: 行覆盖率 {line_cov:.1f}%\n"

        report += """
================================================================================
模块状态
================================================================================

"""

        for module, stats in self.status_data["module_stats"].items():
            report += f"{module}:\n"
            if "last_test" in stats:
                report += f"  最后测试: {stats['last_test']}\n"
            if "test_count" in stats:
                report += f"  测试次数: {stats['test_count']}\n"
            if "success_rate" in stats:
                report += f"  成功率: {stats['success_rate']:.1f}%\n"
            report += "\n"

        report += """
================================================================================
状态文件位置: test_reports/test_status.json
================================================================================
"""

        return report

def main():
    """主函数 - 命令行接口"""
    import argparse

    parser = argparse.ArgumentParser(description="SQLCC 测试状态跟踪器")
    parser.add_argument("action", choices=["status", "report", "history", "trend"],
                       help="执行的操作")
    parser.add_argument("--limit", type=int, default=10,
                       help="历史记录数量限制")
    parser.add_argument("--days", type=int, default=30,
                       help="趋势分析天数")
    parser.add_argument("--output", type=str,
                       help="输出文件路径")

    args = parser.parse_args()

    tracker = TestStatusTracker()

    if args.action == "status":
        summary = tracker.get_status_summary()
        print(json.dumps(summary, indent=2, ensure_ascii=False))

    elif args.action == "report":
        report = tracker.generate_report()
        if args.output:
            with open(args.output, 'w', encoding='utf-8') as f:
                f.write(report)
            print(f"报告已保存到: {args.output}")
        else:
            print(report)

    elif args.action == "history":
        history = tracker.get_recent_runs(args.limit)
        print(json.dumps(history, indent=2, ensure_ascii=False))

    elif args.action == "trend":
        trend = tracker.get_coverage_trend(args.days)
        print(json.dumps(trend, indent=2, ensure_ascii=False))

if __name__ == "__main__":
    main()
