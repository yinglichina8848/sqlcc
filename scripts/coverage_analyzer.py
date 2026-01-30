#!/usr/bin/env python3
"""
SQLCC 代码覆盖率分析器
基于源代码和测试文件结构分析，生成真实的覆盖率评估报告
"""

import os
import sys
import json
import subprocess
from pathlib import Path
from datetime import datetime
from typing import Dict, List, Tuple

class CoverageAnalyzer:
    def __init__(self, project_root: str):
        self.project_root = Path(project_root)
        self.src_dir = self.project_root / "src"
        self.tests_dir = self.project_root / "tests"
        self.report_dir = self.project_root / "coverage_results_real" / datetime.now().strftime("%Y%m%d_%H%M%S")
        
    def analyze(self) -> Dict:
        """执行完整的覆盖率分析"""
        print("=" * 60)
        print("SQLCC 真实代码覆盖率分析")
        print("=" * 60)
        print()
        
        # 创建报告目录
        self.report_dir.mkdir(parents=True, exist_ok=True)
        
        # 分析源代码
        src_stats = self._analyze_source_code()
        
        # 分析测试代码
        test_stats = self._analyze_test_code()
        
        # 计算覆盖率指标
        coverage_metrics = self._calculate_coverage(src_stats, test_stats)
        
        # 生成报告
        report = self._generate_report(src_stats, test_stats, coverage_metrics)
        
        # 保存报告
        self._save_report(report)
        
        return report
    
    def _analyze_source_code(self) -> Dict:
        """分析源代码结构和规模"""
        print("[1/4] 分析源代码结构...")
        
        stats = {
            "total_files": 0,
            "total_lines": 0,
            "modules": {}
        }
        
        modules = [
            ("sql_parser", "SQL解析器"),
            ("storage_engine", "存储引擎"),
            ("execution", "执行引擎"),
            ("network", "网络通信"),
            ("core", "核心组件"),
            ("transaction", "事务管理"),
            ("utils", "工具类"),
            ("exception", "异常处理"),
            ("logger", "日志系统"),
        ]
        
        for module_dir, module_name in modules:
            module_path = self.src_dir / module_dir
            if module_path.exists():
                files = list(module_path.rglob("*.cpp"))
                lines = sum(len(f.read_text().split('\n')) for f in files if f.exists())
                
                stats["modules"][module_name] = {
                    "files": len(files),
                    "lines": lines,
                    "path": str(module_path.relative_to(self.project_root))
                }
                stats["total_files"] += len(files)
                stats["total_lines"] += lines
        
        print(f"      发现 {stats['total_files']} 个源文件, {stats['total_lines']} 行代码")
        return stats
    
    def _analyze_test_code(self) -> Dict:
        """分析测试代码结构"""
        print("[2/4] 分析测试代码结构...")
        
        stats = {
            "total_test_files": 0,
            "total_test_lines": 0,
            "test_cases": 0,
            "modules": {}
        }
        
        if not self.tests_dir.exists():
            print("      警告: tests 目录不存在")
            return stats
        
        test_levels = [
            ("level1_foundation", "Level 1 - 基础"),
            ("level2_core_services", "Level 2 - 核心服务"),
            ("level2_storage_engine", "Level 2 - 存储引擎"),
            ("level3_transaction_manager", "Level 3 - 事务管理"),
            ("level4_sql_processing", "Level 4 - SQL处理"),
            ("level5_network", "Level 5 - 网络"),
            ("level6_enterprise", "Level 6 - 企业特性"),
            ("level6_integration", "Level 6 - 集成测试"),
            ("level7_integration", "Level 7 - 端到端"),
            ("temporary", "临时测试"),
        ]
        
        for level_dir, level_name in test_levels:
            level_path = self.tests_dir / level_dir
            if level_path.exists():
                test_files = list(level_path.rglob("*.cpp"))
                test_lines = 0
                test_cases = 0
                
                for f in test_files:
                    if f.exists():
                        content = f.read_text()
                        test_lines += len(content.split('\n'))
                        # 统计 TEST 和 TEST_F 宏
                        test_cases += content.count("TEST(") + content.count("TEST_F(")
                
                stats["modules"][level_name] = {
                    "files": len(test_files),
                    "lines": test_lines,
                    "test_cases": test_cases,
                    "path": str(level_path.relative_to(self.project_root))
                }
                stats["total_test_files"] += len(test_files)
                stats["total_test_lines"] += test_lines
                stats["test_cases"] += test_cases
        
        print(f"      发现 {stats['total_test_files']} 个测试文件, {stats['test_cases']} 个测试用例")
        return stats
    
    def _calculate_coverage(self, src_stats: Dict, test_stats: Dict) -> Dict:
        """计算覆盖率指标"""
        print("[3/4] 计算覆盖率指标...")
        
        metrics = {
            "test_to_code_ratio": 0.0,
            "estimated_coverage": 0.0,
            "modules": {}
        }
        
        if src_stats["total_lines"] > 0:
            metrics["test_to_code_ratio"] = test_stats["total_test_lines"] / src_stats["total_lines"]
        
        # 基于测试代码比例的估算覆盖率
        # 这是一个保守估计，实际覆盖率通常低于测试代码比例
        metrics["estimated_coverage"] = min(metrics["test_to_code_ratio"] * 0.4, 0.95)
        
        # 计算每个模块的指标
        for module_name, src_module in src_stats["modules"].items():
            # 查找对应的测试模块
            test_module = None
            for test_name, test_data in test_stats["modules"].items():
                if module_name.lower() in test_name.lower() or test_name.lower() in module_name.lower():
                    test_module = test_data
                    break
            
            if test_module:
                ratio = test_module["lines"] / src_module["lines"] if src_module["lines"] > 0 else 0
                estimated = min(ratio * 0.4, 0.95)
            else:
                ratio = 0
                estimated = 0
            
            metrics["modules"][module_name] = {
                "test_code_ratio": ratio,
                "estimated_coverage": estimated,
                "test_cases": test_module["test_cases"] if test_module else 0
            }
        
        print(f"      测试/源码比例: {metrics['test_to_code_ratio']:.2%}")
        print(f"      估算覆盖率: {metrics['estimated_coverage']:.1%}")
        return metrics
    
    def _generate_report(self, src_stats: Dict, test_stats: Dict, metrics: Dict) -> Dict:
        """生成完整报告"""
        print("[4/4] 生成覆盖率报告...")
        
        report = {
            "metadata": {
                "generated_at": datetime.now().isoformat(),
                "project_version": "v1.3.8",
                "analyzer_version": "1.0.0"
            },
            "summary": {
                "source_files": src_stats["total_files"],
                "source_lines": src_stats["total_lines"],
                "test_files": test_stats["total_test_files"],
                "test_lines": test_stats["total_test_lines"],
                "test_cases": test_stats["test_cases"],
                "test_to_code_ratio": f"{metrics['test_to_code_ratio']:.2%}",
                "estimated_coverage": f"{metrics['estimated_coverage']:.1%}"
            },
            "modules": [],
            "recommendations": []
        }
        
        # 模块详情
        for module_name, module_data in src_stats["modules"].items():
            coverage_data = metrics["modules"].get(module_name, {})
            report["modules"].append({
                "name": module_name,
                "source_files": module_data["files"],
                "source_lines": module_data["lines"],
                "test_cases": coverage_data.get("test_cases", 0),
                "estimated_coverage": coverage_data.get("estimated_coverage", 0),
                "status": "✅ 良好" if coverage_data.get("estimated_coverage", 0) > 0.5 else "⚠️ 需改进"
            })
        
        # 生成改进建议
        low_coverage_modules = [
            m for m in report["modules"]
            if isinstance(m.get("estimated_coverage"), (int, float)) and m["estimated_coverage"] < 0.3
        ]
        
        if low_coverage_modules:
            report["recommendations"].append(
                f"优先补充以下模块的测试: {', '.join(m['name'] for m in low_coverage_modules[:3])}"
            )
        
        if metrics["estimated_coverage"] < 0.5:
            report["recommendations"].append("整体测试覆盖率偏低，建议增加单元测试")
        
        report["recommendations"].append("建立自动化测试执行流程")
        report["recommendations"].append("使用 llvm-cov 收集真实覆盖率数据")
        
        return report
    
    def _save_report(self, report: Dict):
        """保存报告到文件"""
        # JSON 格式
        json_path = self.report_dir / "coverage_report.json"
        with open(json_path, 'w') as f:
            json.dump(report, f, indent=2)
        
        # Markdown 格式
        md_path = self.report_dir / "coverage_report.md"
        with open(md_path, 'w') as f:
            f.write(self._format_markdown(report))
        
        # 创建最新报告链接
        latest_link = self.project_root / "coverage_results_real" / "latest"
        if latest_link.exists():
            latest_link.unlink()
        latest_link.symlink_to(self.report_dir, target_is_directory=True)
        
        print(f"\n报告已保存到: {self.report_dir}")
        print(f"  - JSON: {json_path}")
        print(f"  - Markdown: {md_path}")
        print(f"  - 最新链接: {latest_link}")
    
    def _format_markdown(self, report: Dict) -> str:
        """格式化为 Markdown"""
        md = f"""# SQLCC 代码覆盖率分析报告

**生成时间**: {report['metadata']['generated_at']}  
**项目版本**: {report['metadata']['project_version']}  
**分析器版本**: {report['metadata']['analyzer_version']}

---

## 📊 概览

| 指标 | 数值 |
|------|------|
| 源文件数 | {report['summary']['source_files']} |
| 源码行数 | {report['summary']['source_lines']:,} |
| 测试文件数 | {report['summary']['test_files']} |
| 测试代码行数 | {report['summary']['test_lines']:,} |
| 测试用例数 | {report['summary']['test_cases']} |
| 测试/源码比例 | {report['summary']['test_to_code_ratio']} |
| **估算覆盖率** | **{report['summary']['estimated_coverage']}** |

---

## 📁 模块详情

| 模块 | 源文件 | 源码行数 | 测试用例 | 估算覆盖率 | 状态 |
|------|--------|----------|----------|------------|------|
"""
        
        for module in report['modules']:
            coverage = module['estimated_coverage']
            if isinstance(coverage, float):
                coverage_str = f"{coverage:.1%}"
            else:
                coverage_str = str(coverage)
            
            md += f"| {module['name']} | {module['source_files']} | {module['source_lines']:,} | {module['test_cases']} | {coverage_str} | {module['status']} |\n"
        
        md += """
---

## 💡 改进建议

"""
        for i, rec in enumerate(report['recommendations'], 1):
            md += f"{i}. {rec}\n"
        
        md += """
---

## 📈 覆盖率说明

**重要提示**: 本报告的覆盖率是基于测试代码比例估算的，**不是**通过实际测试执行收集的真实覆盖率。

要获取真实覆盖率数据，需要：
1. 修复编译环境问题
2. 成功编译测试目标
3. 使用 `bazel coverage` 或 `llvm-cov` 收集运行时覆盖率数据
4. 生成详细的行级覆盖率报告

---

*报告生成: 自动代码覆盖率分析器*
"""
        
        return md


def main():
    project_root = os.environ.get("PROJECT_ROOT", "/home/liying/sqlcc")
    
    analyzer = CoverageAnalyzer(project_root)
    report = analyzer.analyze()
    
    print()
    print("=" * 60)
    print("分析完成!")
    print("=" * 60)


if __name__ == "__main__":
    main()
