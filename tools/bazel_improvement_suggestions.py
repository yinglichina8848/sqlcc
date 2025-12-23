#!/usr/bin/env python3
"""
SQLCC Bazel构建配置改进建议工具
基于修复结果和剩余问题，生成具体的改进建议

功能特性:
- 分析剩余问题的模式和趋势
- 生成针对性的改进建议
- 提供优先级排序的行动计划
- 智能推荐工具增强方案

作者: SQLCC AI Agent
版本: v1.2.6
更新时间: 2025-12-22
"""

import os
import sys
import json
from pathlib import Path
from typing import Dict, List, Any, Tuple
from collections import Counter, defaultdict

class BazelImprovementAnalyzer:
    """Bazel改进分析器"""

    def __init__(self, project_root: str):
        self.project_root = Path(project_root).resolve()
        self.analysis_results = {}

    def analyze_remaining_issues(self, detection_results: Dict[str, Any]) -> Dict[str, Any]:
        """分析剩余问题"""
        issues = detection_results.get('issues', [])

        # 问题分类统计
        issue_types = Counter()
        issue_patterns = defaultdict(list)
        file_patterns = defaultdict(list)

        for issue in issues:
            issue_type = issue['issue_type']
            issue_types[issue_type] += 1

            # 收集问题模式
            context = issue.get('context', '')
            if context:
                issue_patterns[issue_type].append(context)

            # 收集文件模式
            file_path = issue.get('file_path', '')
            if file_path:
                file_patterns[issue_type].append(file_path)

        return {
            "issue_statistics": dict(issue_types),
            "issue_patterns": dict(issue_patterns),
            "file_patterns": dict(file_patterns),
            "total_issues": len(issues)
        }

    def generate_improvement_suggestions(self, analysis_results: Dict[str, Any]) -> List[Dict[str, Any]]:
        """生成改进建议"""
        suggestions = []

        # 基于问题类型的建议
        issue_stats = analysis_results['issue_statistics']

        # LABEL_ERROR改进建议
        if issue_stats.get('LABEL_ERROR', 0) > 0:
            suggestions.extend(self._generate_label_error_suggestions(
                issue_stats['LABEL_ERROR'],
                analysis_results['issue_patterns'].get('LABEL_ERROR', [])
            ))

        # MISSING_FILE改进建议
        if issue_stats.get('MISSING_FILE', 0) > 0:
            suggestions.extend(self._generate_missing_file_suggestions(
                issue_stats['MISSING_FILE'],
                analysis_results['issue_patterns'].get('MISSING_FILE', [])
            ))

        # MISSING_TARGET改进建议
        if issue_stats.get('MISSING_TARGET', 0) > 0:
            suggestions.extend(self._generate_missing_target_suggestions(
                issue_stats['MISSING_TARGET'],
                analysis_results['issue_patterns'].get('MISSING_TARGET', [])
            ))

        # 工具链改进建议
        suggestions.extend(self._generate_toolchain_improvements())

        # CI/CD集成建议
        suggestions.extend(self._generate_ci_cd_suggestions())

        return suggestions

    def _generate_label_error_suggestions(self, count: int, patterns: List[str]) -> List[Dict[str, Any]]:
        """生成标签错误改进建议"""
        suggestions = []

        # 分析常见错误模式
        common_errors = self._analyze_label_patterns(patterns)

        suggestions.append({
            "priority": "HIGH",
            "category": "LABEL_ERROR",
            "title": "增强标签格式校验规则",
            "description": f"发现{count}个标签格式错误，需增强校验规则",
            "current_impact": f"影响{count}个构建目标的正确性",
            "proposed_solution": "扩展bazel_label_fixer.py的校验规则",
            "implementation_steps": [
                "分析常见错误模式",
                "添加新的校验规则",
                "实现自动修复逻辑",
                "添加单元测试"
            ],
            "estimated_effort": "2-3天",
            "expected_benefit": f"减少80%的标签格式错误",
            "common_patterns": common_errors[:5]  # 前5个常见模式
        })

        if any('deps' in pattern for pattern in patterns):
            suggestions.append({
                "priority": "MEDIUM",
                "category": "LABEL_ERROR",
                "title": "依赖声明规范化",
                "description": "标准化依赖声明的格式和结构",
                "proposed_solution": "创建依赖声明格式化工具",
                "implementation_steps": [
                    "定义标准依赖格式",
                    "实现自动格式化",
                    "集成到现有工具链"
                ],
                "estimated_effort": "1天",
                "expected_benefit": "提高依赖声明的一致性"
            })

        return suggestions

    def _generate_missing_file_suggestions(self, count: int, patterns: List[str]) -> List[Dict[str, Any]]:
        """生成缺失文件改进建议"""
        suggestions = []

        suggestions.append({
            "priority": "HIGH",
            "category": "MISSING_FILE",
            "title": "智能Include路径解析",
            "description": f"剩余{count}个include路径问题需要更智能的解析",
            "proposed_solution": "增强include_fixer.py的路径解析能力",
            "implementation_steps": [
                "添加条件编译支持 (#ifdef)",
                "支持间接依赖分析",
                "实现平台相关路径处理",
                "添加上下文感知分析"
            ],
            "estimated_effort": "3-4天",
            "expected_benefit": f"解决剩余{count}个复杂include问题"
        })

        # 检查是否需要预编译头文件优化
        suggestions.append({
            "priority": "MEDIUM",
            "category": "MISSING_FILE",
            "title": "预编译头文件策略优化",
            "description": "基于include使用频率优化预编译头文件",
            "proposed_solution": "实现智能的PCH策略生成器",
            "implementation_steps": [
                "分析include使用频率",
                "生成优化的PCH配置",
                "验证编译时间改善"
            ],
            "estimated_effort": "2天",
            "expected_benefit": "提升编译速度40%"
        })

        return suggestions

    def _generate_missing_target_suggestions(self, count: int, patterns: List[str]) -> List[Dict[str, Any]]:
        """生成缺失目标改进建议"""
        suggestions = []

        suggestions.append({
            "priority": "HIGH",
            "category": "MISSING_TARGET",
            "title": "依赖关系智能推断",
            "description": f"剩余{count}个缺失目标需要更智能的创建逻辑",
            "proposed_solution": "增强dependency_fixer.py的推断能力",
            "implementation_steps": [
                "分析目标命名模式",
                "实现基于上下文的推断",
                "添加循环依赖检测",
                "优化占位符目标生成"
            ],
            "estimated_effort": "3天",
            "expected_benefit": f"自动化解决90%的缺失目标问题"
        })

        return suggestions

    def _generate_toolchain_improvements(self) -> List[Dict[str, Any]]:
        """生成工具链改进建议"""
        suggestions = []

        suggestions.append({
            "priority": "HIGH",
            "category": "TOOLCHAIN",
            "title": "集成修复工作流",
            "description": "创建一键执行的完整修复工作流",
            "proposed_solution": "开发bazel_fix_all.sh集成脚本",
            "implementation_steps": [
                "设计工作流编排逻辑",
                "实现错误处理和回滚",
                "添加进度显示和报告",
                "集成所有现有工具"
            ],
            "estimated_effort": "2天",
            "expected_benefit": "简化修复流程，提高效率"
        })

        suggestions.append({
            "priority": "MEDIUM",
            "category": "TOOLCHAIN",
            "title": "性能监控和优化",
            "description": "添加工具执行时间和成功率监控",
            "proposed_solution": "实现工具性能分析和优化",
            "implementation_steps": [
                "添加性能指标收集",
                "优化慢速操作",
                "生成性能报告",
                "持续性能监控"
            ],
            "estimated_effort": "1-2天",
            "expected_benefit": "提升工具执行效率20%"
        })

        return suggestions

    def _generate_ci_cd_suggestions(self) -> List[Dict[str, Any]]:
        """生成CI/CD集成建议"""
        suggestions = []

        suggestions.append({
            "priority": "MEDIUM",
            "category": "CI_CD",
            "title": "GitHub Actions集成",
            "description": "将构建配置检查集成到CI/CD流程",
            "proposed_solution": "创建自动化检查工作流",
            "implementation_steps": [
                "编写GitHub Actions配置",
                "设置自动修复和提交",
                "配置通知机制",
                "添加分支保护规则"
            ],
            "estimated_effort": "1天",
            "expected_benefit": "防止构建配置回归"
        })

        return suggestions

    def _analyze_label_patterns(self, patterns: List[str]) -> List[Dict[str, Any]]:
        """分析标签错误模式"""
        pattern_counts = Counter()

        for pattern in patterns:
            # 提取错误特征
            if 'deps = [' in pattern:
                if ':' not in pattern and '.h' in pattern:
                    pattern_counts['missing_target_name'] += 1
                elif '.h' in pattern:
                    pattern_counts['file_extension_in_target'] += 1
            elif 'srcs = [' in pattern:
                if '.h' in pattern:
                    pattern_counts['header_in_srcs'] += 1

        # 转换为结构化结果
        result = []
        for pattern, count in pattern_counts.most_common():
            result.append({
                "pattern": pattern,
                "count": count,
                "description": self._get_pattern_description(pattern)
            })

        return result

    def _get_pattern_description(self, pattern: str) -> str:
        """获取模式描述"""
        descriptions = {
            "missing_target_name": "依赖声明缺少目标名称",
            "file_extension_in_target": "目标名称包含文件扩展名",
            "header_in_srcs": "头文件出现在srcs声明中"
        }
        return descriptions.get(pattern, "未知模式")

    def generate_implementation_plan(self, suggestions: List[Dict[str, Any]]) -> Dict[str, Any]:
        """生成实施计划"""
        # 按优先级排序
        priority_order = {"CRITICAL": 0, "HIGH": 1, "MEDIUM": 2, "LOW": 3}

        sorted_suggestions = sorted(
            suggestions,
            key=lambda x: priority_order.get(x['priority'], 999)
        )

        # 分阶段规划
        phases = {
            "phase1_critical": [],  # 立即执行
            "phase2_high": [],      # 本周完成
            "phase3_medium": [],    # 下周完成
            "phase4_future": []     # 长期规划
        }

        for suggestion in sorted_suggestions:
            priority = suggestion['priority']
            if priority == "CRITICAL":
                phases["phase1_critical"].append(suggestion)
            elif priority == "HIGH":
                phases["phase2_high"].append(suggestion)
            elif priority == "MEDIUM":
                phases["phase3_medium"].append(suggestion)
            else:
                phases["phase4_future"].append(suggestion)

        # 计算总工作量
        total_effort = sum(
            self._parse_effort(s['estimated_effort'])
            for phase in phases.values()
            for s in phase
        )

        return {
            "phases": phases,
            "total_suggestions": len(suggestions),
            "total_effort_days": total_effort,
            "implementation_timeline": self._generate_timeline(phases)
        }

    def _parse_effort(self, effort_str: str) -> float:
        """解析工作量字符串"""
        if '-' in effort_str:
            # 处理范围，如 "2-3天"
            parts = effort_str.replace('天', '').split('-')
            return (float(parts[0]) + float(parts[1])) / 2
        else:
            # 处理单个数字，如 "2天"
            return float(effort_str.replace('天', ''))

    def _generate_timeline(self, phases: Dict[str, List]) -> Dict[str, str]:
        """生成时间线"""
        return {
            "phase1_critical": "立即开始 (今天)",
            "phase2_high": "本周完成",
            "phase3_medium": "下周完成",
            "phase4_future": "后续规划"
        }

def main():
    """主函数"""
    import argparse

    parser = argparse.ArgumentParser(description="SQLCC Bazel改进建议生成器")
    parser.add_argument("project_root", help="项目根目录")
    parser.add_argument("--detection-results", help="检测结果JSON文件")
    parser.add_argument("--output-plan", "-o", help="输出改进计划文件")

    args = parser.parse_args()

    if not os.path.isdir(args.project_root):
        print(f"❌ 错误: 目录不存在: {args.project_root}")
        sys.exit(1)

    analyzer = BazelImprovementAnalyzer(args.project_root)

    # 读取检测结果
    if args.detection_results and os.path.exists(args.detection_results):
        with open(args.detection_results, 'r', encoding='utf-8') as f:
            detection_results = json.load(f)
    else:
        print("❌ 需要提供有效的检测结果文件")
        sys.exit(1)

    # 分析剩余问题
    analysis_results = analyzer.analyze_remaining_issues(detection_results)

    # 生成改进建议
    suggestions = analyzer.generate_improvement_suggestions(analysis_results)

    # 生成实施计划
    implementation_plan = analyzer.generate_implementation_plan(suggestions)

    # 输出结果
    print("🔍 SQLCC Bazel构建配置改进建议")
    print("=" * 50)
    print(f"剩余问题总数: {analysis_results['total_issues']}")
    print(f"生成建议数量: {len(suggestions)}")
    print(f"预计总工作量: {implementation_plan['total_effort_days']}天")
    print()

    # 按阶段输出建议
    for phase_name, phase_suggestions in implementation_plan['phases'].items():
        if phase_suggestions:
            phase_title = phase_name.replace('_', ' ').title()
            timeline = implementation_plan['implementation_timeline'][phase_name]

            print(f"📅 {phase_title} ({timeline})")
            print("-" * 40)

            for i, suggestion in enumerate(phase_suggestions, 1):
                print(f"{i}. 🔧 {suggestion['title']}")
                print(f"   优先级: {suggestion['priority']}")
                print(f"   类别: {suggestion['category']}")
                print(f"   工作量: {suggestion['estimated_effort']}")
                print(f"   预期收益: {suggestion['expected_benefit']}")
                print()

    # 保存详细计划
    if args.output_plan:
        plan_data = {
            "analysis_results": analysis_results,
            "suggestions": suggestions,
            "implementation_plan": implementation_plan
        }

        with open(args.output_plan, 'w', encoding='utf-8') as f:
            json.dump(plan_data, f, ensure_ascii=False, indent=2)

        print(f"📄 详细改进计划已保存到: {args.output_plan}")

    print("✅ 改进建议生成完成!")
    print("\n💡 建议优先实施phase1_critical的改进项，以快速解决最关键的问题。")

if __name__ == "__main__":
    sys.exit(main())
