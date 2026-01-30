#!/usr/bin/env python3
"""
SQLCC 代码覆盖率分析器 v2
基于测试 Level 层次结构分析，正确映射测试到源码模块
"""

import os
import sys
import json
from pathlib import Path
from datetime import datetime
from typing import Dict, List, Tuple

class CoverageAnalyzerV2:
    """基于 SQL 组件依赖层次的覆盖率分析器"""
    
    # 测试 Level 与源码模块的映射关系
    LEVEL_TO_SOURCE_MAPPING = {
        "Level 1 - 基础": {
            "test_path": "tests/level1_foundation",
            "source_modules": ["utils", "exception", "logger", "types"],
            "description": "基础工具类"
        },
        "Level 2 - 核心服务": {
            "test_path": "tests/level2_core_services",
            "source_modules": ["core", "sql_parser", "config_manager"],
            "description": "核心服务和SQL解析器"
        },
        "Level 2 - 存储引擎": {
            "test_path": "tests/level2_storage_engine",
            "source_modules": ["storage_engine"],
            "description": "存储引擎"
        },
        "Level 3 - 事务管理": {
            "test_path": "tests/level3_transaction_manager",
            "source_modules": ["transaction", "transaction_manager", "execution"],
            "description": "事务管理和执行引擎"
        },
        "Level 4 - SQL处理": {
            "test_path": "tests/level4_sql_processing",
            "source_modules": ["sql_executor", "trigger", "procedure"],
            "description": "SQL执行器和触发器"
        },
        "Level 5 - 网络": {
            "test_path": "tests/level5_network",
            "source_modules": ["network"],
            "description": "网络通信"
        },
        "Level 6 - 企业特性": {
            "test_path": "tests/level6_enterprise",
            "source_modules": ["security"],
            "description": "企业级安全特性"
        },
        "Level 6 - 集成测试": {
            "test_path": "tests/level6_integration",
            "source_modules": ["integration"],  # 跨模块测试
            "description": "系统集成测试"
        },
        "Level 7 - 端到端": {
            "test_path": "tests/level7_integration",
            "source_modules": ["end_to_end"],  # 端到端测试
            "description": "端到端集成测试"
        },
        "临时测试": {
            "test_path": "tests/temporary",
            "source_modules": [],  # 不直接对应源码
            "description": "临时调试测试"
        }
    }
    
    def __init__(self, project_root: str):
        self.project_root = Path(project_root)
        self.src_dir = self.project_root / "src"
        self.tests_dir = self.project_root / "tests"
        self.report_dir = self.project_root / "coverage_results_real" / datetime.now().strftime("%Y%m%d_%H%M%S")
        
    def analyze(self) -> Dict:
        """执行完整的覆盖率分析"""
        print("=" * 70)
        print("SQLCC 真实代码覆盖率分析 v2")
        print("基于 SQL 组件依赖层次结构")
        print("=" * 70)
        print()
        
        # 创建报告目录
        self.report_dir.mkdir(parents=True, exist_ok=True)
        
        # 分析源码结构
        src_stats = self._analyze_source_code()
        
        # 按 Level 分析测试
        level_stats = self._analyze_tests_by_level()
        
        # 计算覆盖率指标
        coverage_metrics = self._calculate_coverage_by_level(src_stats, level_stats)
        
        # 生成报告
        report = self._generate_report(src_stats, level_stats, coverage_metrics)
        
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
        
        # 遍历 src 目录下的所有模块
        for module_dir in self.src_dir.iterdir():
            if module_dir.is_dir() and not module_dir.name.startswith('.'):
                cpp_files = list(module_dir.rglob("*.cpp"))
                total_lines = 0
                
                for f in cpp_files:
                    if f.exists():
                        try:
                            total_lines += len(f.read_text().split('\n'))
                        except:
                            pass
                
                if cpp_files:  # 只记录有代码文件的模块
                    stats["modules"][module_dir.name] = {
                        "files": len(cpp_files),
                        "lines": total_lines,
                        "path": str(module_dir.relative_to(self.project_root))
                    }
                    stats["total_files"] += len(cpp_files)
                    stats["total_lines"] += total_lines
        
        print(f"      发现 {stats['total_files']} 个源文件, {stats['total_lines']:,} 行代码")
        print(f"      源码模块: {', '.join(stats['modules'].keys())}")
        return stats
    
    def _analyze_tests_by_level(self) -> Dict:
        """按 Level 分析测试代码"""
        print("[2/4] 按 Level 分析测试代码...")
        
        stats = {
            "total_test_files": 0,
            "total_test_lines": 0,
            "total_test_cases": 0,
            "levels": {}
        }
        
        for level_name, level_info in self.LEVEL_TO_SOURCE_MAPPING.items():
            test_path = self.project_root / level_info["test_path"]
            
            if not test_path.exists():
                continue
            
            # 统计该 Level 的测试文件
            test_files = list(test_path.rglob("*.cpp"))
            test_lines = 0
            test_cases = 0
            
            for f in test_files:
                if f.exists():
                    try:
                        content = f.read_text()
                        test_lines += len(content.split('\n'))
                        # 统计 TEST 和 TEST_F 宏
                        test_cases += content.count("TEST(") + content.count("TEST_F(")
                    except:
                        pass
            
            stats["levels"][level_name] = {
                "test_files": len(test_files),
                "test_lines": test_lines,
                "test_cases": test_cases,
                "source_modules": level_info["source_modules"],
                "description": level_info["description"],
                "test_path": level_info["test_path"]
            }
            stats["total_test_files"] += len(test_files)
            stats["total_test_lines"] += test_lines
            stats["total_test_cases"] += test_cases
        
        print(f"      发现 {stats['total_test_files']} 个测试文件")
        print(f"      发现 {stats['total_test_cases']} 个测试用例")
        return stats
    
    def _calculate_coverage_by_level(self, src_stats: Dict, level_stats: Dict) -> Dict:
        """按 Level 计算覆盖率指标"""
        print("[3/4] 计算覆盖率指标...")
        
        metrics = {
            "test_to_code_ratio": 0.0,
            "estimated_coverage": 0.0,
            "levels": {}
        }
        
        if src_stats["total_lines"] > 0:
            metrics["test_to_code_ratio"] = level_stats["total_test_lines"] / src_stats["total_lines"]
        
        # 基于测试代码比例的估算覆盖率
        # 考虑：测试代码比例 * 转换系数 (0.4) * 质量系数
        metrics["estimated_coverage"] = min(metrics["test_to_code_ratio"] * 0.4, 0.95)
        
        # 计算每个 Level 的指标
        for level_name, level_data in level_stats["levels"].items():
            # 计算该 Level 对应的源码规模
            source_lines = 0
            source_files = 0
            
            for module_name in level_data["source_modules"]:
                if module_name in src_stats["modules"]:
                    source_lines += src_stats["modules"][module_name]["lines"]
                    source_files += src_stats["modules"][module_name]["files"]
            
            # 计算指标
            if source_lines > 0:
                ratio = level_data["test_lines"] / source_lines
                # 估算覆盖率：测试代码比例 * 质量系数
                # 质量系数根据测试用例数量调整
                quality_factor = min(level_data["test_cases"] / 100, 1.0) * 0.5 + 0.3
                estimated = min(ratio * quality_factor, 0.95)
            else:
                ratio = 0
                estimated = 0
            
            metrics["levels"][level_name] = {
                "source_files": source_files,
                "source_lines": source_lines,
                "test_files": level_data["test_files"],
                "test_lines": level_data["test_lines"],
                "test_cases": level_data["test_cases"],
                "test_code_ratio": ratio,
                "estimated_coverage": estimated,
                "source_modules": level_data["source_modules"]
            }
        
        print(f"      测试/源码比例: {metrics['test_to_code_ratio']:.2%}")
        print(f"      估算整体覆盖率: {metrics['estimated_coverage']:.1%}")
        return metrics
    
    def _generate_report(self, src_stats: Dict, level_stats: Dict, metrics: Dict) -> Dict:
        """生成完整报告"""
        print("[4/4] 生成覆盖率报告...")
        
        report = {
            "metadata": {
                "generated_at": datetime.now().isoformat(),
                "project_version": "v1.3.8",
                "analyzer_version": "2.0.0",
                "methodology": "基于SQL组件依赖层次分析"
            },
            "summary": {
                "source_files": src_stats["total_files"],
                "source_lines": src_stats["total_lines"],
                "test_files": level_stats["total_test_files"],
                "test_lines": level_stats["total_test_lines"],
                "test_cases": level_stats["total_test_cases"],
                "test_to_code_ratio": f"{metrics['test_to_code_ratio']:.2%}",
                "estimated_coverage": f"{metrics['estimated_coverage']:.1%}"
            },
            "levels": [],
            "module_mapping": {},
            "recommendations": []
        }
        
        # Level 详情 - 按依赖层次排序
        level_order = [
            "Level 1 - 基础",
            "Level 2 - 核心服务",
            "Level 2 - 存储引擎",
            "Level 3 - 事务管理",
            "Level 4 - SQL处理",
            "Level 5 - 网络",
            "Level 6 - 企业特性",
            "Level 6 - 集成测试",
            "Level 7 - 端到端"
        ]
        
        for level_name in level_order:
            if level_name in metrics["levels"]:
                level_data = metrics["levels"][level_name]
                coverage = level_data["estimated_coverage"]
                
                # 状态判断
                if coverage >= 0.5:
                    status = "✅ 良好"
                elif coverage >= 0.3:
                    status = "🟡 一般"
                elif coverage > 0:
                    status = "⚠️ 需改进"
                else:
                    status = "🔴 严重不足"
                
                report["levels"].append({
                    "name": level_name,
                    "source_modules": level_data["source_modules"],
                    "source_files": level_data["source_files"],
                    "source_lines": level_data["source_lines"],
                    "test_files": level_data["test_files"],
                    "test_cases": level_data["test_cases"],
                    "estimated_coverage": coverage,
                    "status": status
                })
        
        # 生成改进建议
        low_coverage_levels = [
            l for l in report["levels"]
            if isinstance(l.get("estimated_coverage"), (int, float)) 
            and l["estimated_coverage"] < 0.3 and l["test_cases"] > 0
        ]
        
        if low_coverage_levels:
            report["recommendations"].append(
                f"优先改进以下Level的测试覆盖: {', '.join(l['name'] for l in low_coverage_levels[:3])}"
            )
        
        zero_coverage = [l for l in report["levels"] if l["test_cases"] == 0]
        if zero_coverage:
            report["recommendations"].append(
                f"以下Level完全无测试，需紧急补充: {', '.join(l['name'] for l in zero_coverage)}"
            )
        
        if metrics["estimated_coverage"] < 0.5:
            report["recommendations"].append("整体测试覆盖率偏低，建议增加核心模块单元测试")
        
        report["recommendations"].append("建立自动化测试执行和覆盖率收集流程")
        report["recommendations"].append("使用 bazel coverage + llvm-cov 收集真实运行时覆盖率")
        
        return report
    
    def _save_report(self, report: Dict):
        """保存报告到文件"""
        # JSON 格式
        json_path = self.report_dir / "coverage_report_v2.json"
        with open(json_path, 'w') as f:
            json.dump(report, f, indent=2, ensure_ascii=False)
        
        # Markdown 格式
        md_path = self.report_dir / "coverage_report_v2.md"
        with open(md_path, 'w') as f:
            f.write(self._format_markdown(report))
        
        # 创建最新报告链接
        latest_link = self.project_root / "coverage_results_real" / "latest_v2"
        if latest_link.exists():
            latest_link.unlink()
        latest_link.symlink_to(self.report_dir, target_is_directory=True)
        
        print(f"\n报告已保存到: {self.report_dir}")
        print(f"  - JSON: {json_path}")
        print(f"  - Markdown: {md_path}")
        print(f"  - 最新链接: {latest_link}")
    
    def _format_markdown(self, report: Dict) -> str:
        """格式化为 Markdown"""
        md = f"""# SQLCC 代码覆盖率分析报告 v2

**生成时间**: {report['metadata']['generated_at']}  
**项目版本**: {report['metadata']['project_version']}  
**分析器版本**: {report['metadata']['analyzer_version']}  
**分析方法**: {report['metadata']['methodology']}

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

## 📁 按 Level 分析

| Level | 源码模块 | 源文件 | 源码行数 | 测试文件 | 测试用例 | 估算覆盖率 | 状态 |
|-------|----------|--------|----------|----------|----------|------------|------|
"""
        
        for level in report['levels']:
            modules_str = ', '.join(level['source_modules']) if level['source_modules'] else '-'
            coverage = level['estimated_coverage']
            if isinstance(coverage, float):
                coverage_str = f"{coverage:.1%}"
            else:
                coverage_str = str(coverage)
            
            md += f"| {level['name']} | {modules_str} | {level['source_files']} | {level['source_lines']:,} | {level['test_files']} | {level['test_cases']} | {coverage_str} | {level['status']} |\n"
        
        md += """
---

## 🗺️ SQL 组件依赖层次

```
Level 1 (Foundation)
├── utils/         - 工具类
├── exception/     - 异常处理
├── logger/        - 日志系统
└── types/         - 类型系统

Level 2 (Core Services)
├── core/          - 核心组件
├── sql_parser/    - SQL解析器 ⭐ 核心
└── config_manager/ - 配置管理

Level 2 (Storage Engine)
└── storage_engine/ - 存储引擎 ⭐ 核心

Level 3 (Transaction Manager)
├── transaction/        - 事务管理
├── transaction_manager/ - 事务管理器
└── execution/          - 执行引擎 ⭐ 核心

Level 4 (SQL Processing)
├── sql_executor/ - SQL执行器
├── trigger/      - 触发器
└── procedure/    - 存储过程

Level 5 (Network)
└── network/ - 网络通信

Level 6 (Enterprise & Integration)
├── security/    - 企业安全
└── integration/ - 集成测试

Level 7 (End-to-End)
└── end_to_end/ - 端到端测试
```

---

## 💡 关键发现

### 测试覆盖分布

"""
        
        # 找出高覆盖和低覆盖的 Level
        good_levels = [l for l in report['levels'] if '✅' in l['status']]
        bad_levels = [l for l in report['levels'] if '🔴' in l['status'] or '⚠️' in l['status']]
        
        if good_levels:
            md += "**覆盖良好的 Level**:\\n"
            for level in good_levels:
                md += f"- {level['name']}: {level['estimated_coverage']:.1%} ({level['test_cases']} 个测试用例)\\n"
            md += "\\n"
        
        if bad_levels:
            md += "**需要改进的 Level**:\\n"
            for level in bad_levels:
                md += f"- {level['name']}: {level['estimated_coverage']:.1%} ({level['test_cases']} 个测试用例)\\n"
            md += "\\n"
        
        md += """---

## 💡 改进建议

"""
        for i, rec in enumerate(report['recommendations'], 1):
            md += f"{i}. {rec}\\n"
        
        md += """
---

## 📈 覆盖率说明

**重要提示**: 本报告的覆盖率是基于测试代码比例和组件层次结构估算的，**不是**通过实际测试执行收集的真实覆盖率。

要获取真实覆盖率数据，需要：
1. 修复编译环境问题（`common_ast` target 缺失）
2. 成功编译并运行测试
3. 使用 `bazel coverage` 或 `llvm-cov` 收集运行时覆盖率数据
4. 生成详细的行级覆盖率报告

---

**测试层次结构说明**: 
- SQLCC 采用 7 层测试架构，对应 SQL 组件的依赖层次
- Level 1-2 测试基础组件，不依赖其他 Level
- Level 3+ 测试依赖下层组件的高级功能
- 这种结构确保测试的层次性和可维护性

---

*报告生成: SQLCC 代码覆盖率分析器 v2*
"""
        
        return md


def main():
    project_root = os.environ.get("PROJECT_ROOT", "/home/liying/sqlcc")
    
    analyzer = CoverageAnalyzerV2(project_root)
    report = analyzer.analyze()
    
    print()
    print("=" * 70)
    print("分析完成!")
    print("=" * 70)


if __name__ == "__main__":
    main()
