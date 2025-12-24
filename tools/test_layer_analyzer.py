#!/usr/bin/env python3
"""
测试分层依赖分析器 (Test Layer Dependency Analyzer)
用于分析测试包的分层依赖关系，从0层到7层

功能特性:
- 分层依赖分析 (Layer 0-7)
- 包含文件依赖检测
- 构建配置问题识别
- 重新生成配置评估

作者: SQLCC AI Agent
版本: v1.0.0
"""

import os
import re
import json
import sys
from pathlib import Path
from typing import Dict, List, Set, Tuple, Optional, Any
from dataclasses import dataclass, asdict
import argparse
from enum import Enum


class TestLayer(Enum):
    """测试层级定义"""
    LAYER_0_INFRASTRUCTURE = "layer_0_infrastructure"  # 基础设施层
    LAYER_1_BASIC = "layer_1_basic"                   # 基础功能层
    LAYER_2_CORE = "layer_2_core"                     # 核心服务层
    LAYER_3_STORAGE = "layer_3_storage"               # 数据存储层
    LAYER_4_PARSER = "layer_4_parser"                 # 解析层
    LAYER_5_EXECUTOR = "layer_5_executor"             # 执行层
    LAYER_6_NETWORK = "layer_6_network"               # 网络层
    LAYER_7_INTEGRATION = "layer_7_integration"       # 集成测试层


@dataclass
class LayerInfo:
    """层级信息"""
    layer: TestLayer
    name: str
    description: str
    build_files: List[str]
    dependencies: Set[str]
    include_deps: Set[str]
    issues: List[str]


@dataclass
class DependencyAnalysis:
    """依赖分析结果"""
    layer: TestLayer
    file_path: str
    direct_deps: List[str]
    indirect_deps: List[str]
    include_deps: List[str]
    missing_deps: List[str]
    circular_deps: List[str]


class TestLayerAnalyzer:
    """测试分层分析器"""

    def __init__(self, project_root: str):
        self.project_root = Path(project_root)
        self.layers = self._define_layers()
        self.dependency_graph = {}
        self.include_graph = {}

    def _define_layers(self) -> Dict[TestLayer, LayerInfo]:
        """定义测试层级结构"""
        return {
            TestLayer.LAYER_0_INFRASTRUCTURE: LayerInfo(
                layer=TestLayer.LAYER_0_INFRASTRUCTURE,
                name="基础设施层",
                description="基础组件和工具测试",
                build_files=[],
                dependencies=set(),
                include_deps=set(),
                issues=[]
            ),
            TestLayer.LAYER_1_BASIC: LayerInfo(
                layer=TestLayer.LAYER_1_BASIC,
                name="基础功能层",
                description="基本数据类型和工具函数测试",
                build_files=[],
                dependencies=set(),
                include_deps=set(),
                issues=[]
            ),
            TestLayer.LAYER_2_CORE: LayerInfo(
                layer=TestLayer.LAYER_2_CORE,
                name="核心服务层",
                description="配置管理、日志、错误处理等核心服务测试",
                build_files=[],
                dependencies=set(),
                include_deps=set(),
                issues=[]
            ),
            TestLayer.LAYER_3_STORAGE: LayerInfo(
                layer=TestLayer.LAYER_3_STORAGE,
                name="数据存储层",
                description="存储引擎、缓冲池、索引等存储组件测试",
                build_files=[],
                dependencies=set(),
                include_deps=set(),
                issues=[]
            ),
            TestLayer.LAYER_4_PARSER: LayerInfo(
                layer=TestLayer.LAYER_4_PARSER,
                name="解析层",
                description="SQL解析器、词法分析等解析组件测试",
                build_files=[],
                dependencies=set(),
                include_deps=set(),
                issues=[]
            ),
            TestLayer.LAYER_5_EXECUTOR: LayerInfo(
                layer=TestLayer.LAYER_5_EXECUTOR,
                name="执行层",
                description="查询执行器、事务管理等执行组件测试",
                build_files=[],
                dependencies=set(),
                include_deps=set(),
                issues=[]
            ),
            TestLayer.LAYER_6_NETWORK: LayerInfo(
                layer=TestLayer.LAYER_6_NETWORK,
                name="网络层",
                description="网络通信、连接管理等网络组件测试",
                build_files=[],
                dependencies=set(),
                include_deps=set(),
                issues=[]
            ),
            TestLayer.LAYER_7_INTEGRATION: LayerInfo(
                layer=TestLayer.LAYER_7_INTEGRATION,
                name="集成测试层",
                description="端到端集成测试和性能测试",
                build_files=[],
                dependencies=set(),
                include_deps=set(),
                issues=[]
            ),
        }

    def _classify_build_file(self, file_path: Path) -> TestLayer:
        """根据文件路径分类到对应的层级"""
        path_str = str(file_path)

        # Layer 0: 基础设施
        if any(pattern in path_str for pattern in ['scripts', 'stubs', 'debug/src']):
            return TestLayer.LAYER_0_INFRASTRUCTURE

        # Layer 1: 基础功能
        elif 'unit/basic' in path_str:
            return TestLayer.LAYER_1_BASIC

        # Layer 2: 核心服务
        elif any(pattern in path_str for pattern in ['unit/core', 'components/core']):
            return TestLayer.LAYER_2_CORE

        # Layer 3: 数据存储
        elif any(pattern in path_str for pattern in ['storage_engine', 'unit/storage', 'components/storage']):
            return TestLayer.LAYER_3_STORAGE

        # Layer 4: 解析层
        elif any(pattern in path_str for pattern in ['sql_parser', 'unit/parser', 'components/parser']):
            return TestLayer.LAYER_4_PARSER

        # Layer 5: 执行层
        elif any(pattern in path_str for pattern in ['executor', 'sql_executor', 'unit/executor']):
            return TestLayer.LAYER_5_EXECUTOR

        # Layer 6: 网络层
        elif any(pattern in path_str for pattern in ['network', 'unit/network', 'components/network']):
            return TestLayer.LAYER_6_NETWORK

        # Layer 7: 集成测试
        elif any(pattern in path_str for pattern in ['integration', 'performance', 'demo']):
            return TestLayer.LAYER_7_INTEGRATION

        # 默认归类到基础设施层
        else:
            return TestLayer.LAYER_0_INFRASTRUCTURE

    def analyze_layer_dependencies(self) -> Dict[str, Any]:
        """分析分层依赖关系"""
        print("🔍 开始分层依赖分析...")

        # 收集所有BUILD文件
        build_files = list(self.project_root.rglob('tests/**/BUILD.bazel'))

        # 按层级分类
        for build_file in build_files:
            layer = self._classify_build_file(build_file)
            self.layers[layer].build_files.append(str(build_file.relative_to(self.project_root)))

        # 分析每个层级的依赖
        layer_analysis = {}
        for layer, info in self.layers.items():
            if info.build_files:  # 只分析有文件的层级
                analysis = self._analyze_layer_dependencies(layer, info)
                layer_analysis[layer.value] = analysis

        return {
            'summary': self._generate_summary(layer_analysis),
            'layer_analysis': layer_analysis,
            'recommendations': self._generate_recommendations(layer_analysis)
        }

    def _analyze_layer_dependencies(self, layer: TestLayer, info: LayerInfo) -> Dict[str, Any]:
        """分析单个层级的依赖关系"""
        print(f"📊 分析 {info.name} ({layer.value})...")

        all_deps = set()
        all_includes = set()
        all_issues = []

        for build_file in info.build_files:
            file_path = self.project_root / build_file

            # 分析构建依赖
            deps, issues = self._analyze_build_dependencies(file_path)
            all_deps.update(deps)
            all_issues.extend(issues)

            # 分析包含依赖
            includes = self._analyze_include_dependencies(file_path)
            all_includes.update(includes)

        # 存储到层级信息
        info.dependencies = all_deps
        info.include_deps = all_includes
        info.issues = all_issues

        return {
            'layer_info': asdict(info),
            'dependency_analysis': {
                'total_dependencies': len(all_deps),
                'total_includes': len(all_includes),
                'unique_dependencies': sorted(list(all_deps)),
                'unique_includes': sorted(list(all_includes)),
            },
            'issues': all_issues,
            'regeneration_assessment': self._assess_regeneration_feasibility(layer, all_deps, all_includes)
        }

    def _analyze_build_dependencies(self, file_path: Path) -> Tuple[Set[str], List[str]]:
        """分析构建依赖"""
        deps = set()
        issues = []

        try:
            content = file_path.read_text()

            # 提取deps
            deps_match = re.findall(r'"([^"]+)"', content)
            for dep in deps_match:
                if dep.startswith('//'):  # 只关心内部依赖
                    deps.add(dep)

            # 检查配置问题
            issues.extend(self._check_build_issues(content, file_path))

        except Exception as e:
            issues.append(f"无法分析文件 {file_path}: {e}")

        return deps, issues

    def _analyze_include_dependencies(self, file_path: Path) -> Set[str]:
        """分析包含依赖"""
        includes = set()

        try:
            # 查找同目录下的源文件
            dir_path = file_path.parent
            source_files = list(dir_path.glob('*.cpp')) + list(dir_path.glob('*.cc'))

            for source_file in source_files:
                try:
                    content = source_file.read_text()
                    # 提取#include语句
                    include_matches = re.findall(r'#include\s*["<]([^">]+)[">]', content)
                    for include in include_matches:
                        includes.add(include)
                except Exception:
                    continue

        except Exception as e:
            print(f"警告: 无法分析包含依赖 {file_path}: {e}")

        return includes

    def _check_build_issues(self, content: str, file_path: Path) -> List[str]:
        """检查构建配置问题"""
        issues = []

        # 检查编译选项
        if 'cc_test(' in content or 'cc_binary(' in content:
            if '"-std=c++20"' not in content and '"-stdlib=libc++"' not in content:
                issues.append("缺少标准编译选项")

        # 检查语法错误
        if 'name = ' in content and ',,' in content:
            issues.append("语法错误：多余的逗号")

        # 检查依赖格式
        if '"//' in content and not re.search(r'"//[^"]+":', content):
            issues.append("依赖格式可能不正确")

        return issues

    def _assess_regeneration_feasibility(self, layer: TestLayer, deps: Set[str], includes: Set[str]) -> Dict[str, Any]:
        """评估重新生成配置的可行性"""
        feasibility_score = 0
        reasons = []

        # 评估依赖复杂度
        if len(deps) < 5:
            feasibility_score += 30
            reasons.append("依赖关系简单，易于重新生成")
        elif len(deps) < 10:
            feasibility_score += 20
            reasons.append("依赖关系中等复杂度")
        else:
            feasibility_score += 10
            reasons.append("依赖关系复杂，需要仔细分析")

        # 评估包含文件复杂度
        if len(includes) < 10:
            feasibility_score += 30
            reasons.append("包含文件数量少，易于管理")
        elif len(includes) < 20:
            feasibility_score += 20
            reasons.append("包含文件数量中等")
        else:
            feasibility_score += 10
            reasons.append("包含文件数量多，重新生成复杂度高")

        # 评估层级特点
        if layer in [TestLayer.LAYER_0_INFRASTRUCTURE, TestLayer.LAYER_1_BASIC]:
            feasibility_score += 20
            reasons.append("基础层级，重新生成风险低")
        elif layer in [TestLayer.LAYER_6_NETWORK, TestLayer.LAYER_7_INTEGRATION]:
            feasibility_score += 15
            reasons.append("高层级，依赖关系复杂")

        # 评估现有配置质量
        build_files_count = len(self.layers[layer].build_files)
        if build_files_count == 1:
            feasibility_score += 15
            reasons.append("单文件配置，易于重新生成")
        elif build_files_count <= 3:
            feasibility_score += 10
            reasons.append("少量配置文件，便于管理")

        recommendation = "建议重新生成" if feasibility_score >= 60 else "建议修复现有配置"

        return {
            'feasibility_score': feasibility_score,
            'recommendation': recommendation,
            'reasons': reasons,
            'estimated_effort': '低' if feasibility_score >= 70 else '中' if feasibility_score >= 50 else '高'
        }

    def _generate_summary(self, layer_analysis: Dict[str, Any]) -> Dict[str, Any]:
        """生成总体摘要"""
        total_files = sum(len(analysis['layer_info']['build_files']) for analysis in layer_analysis.values())
        total_deps = sum(analysis['dependency_analysis']['total_dependencies'] for analysis in layer_analysis.values())
        total_issues = sum(len(analysis['issues']) for analysis in layer_analysis.values())

        regeneration_candidates = []
        for layer_name, analysis in layer_analysis.items():
            if analysis['regeneration_assessment']['feasibility_score'] >= 60:
                regeneration_candidates.append(layer_name)

        return {
            'total_layers': len([l for l in self.layers.keys() if self.layers[l].build_files]),
            'total_build_files': total_files,
            'total_dependencies': total_deps,
            'total_issues': total_issues,
            'regeneration_candidates': regeneration_candidates,
            'layer_distribution': {
                layer_name: len(analysis['layer_info']['build_files'])
                for layer_name, analysis in layer_analysis.items()
            }
        }

    def _generate_recommendations(self, layer_analysis: Dict[str, Any]) -> List[str]:
        """生成修复建议"""
        recommendations = []

        # 重新生成建议
        regeneration_layers = [
            layer for layer, analysis in layer_analysis.items()
            if analysis['regeneration_assessment']['recommendation'] == '建议重新生成'
        ]

        if regeneration_layers:
            recommendations.append(f"🔄 建议重新生成配置的层级: {', '.join(regeneration_layers)}")
            recommendations.append("   重新生成可以确保配置标准化和依赖完整性")

        # 修复建议
        fix_layers = [
            layer for layer, analysis in layer_analysis.items()
            if analysis['regeneration_assessment']['recommendation'] == '建议修复现有配置'
        ]

        if fix_layers:
            recommendations.append(f"🔧 建议修复现有配置的层级: {', '.join(fix_layers)}")
            recommendations.append("   现有配置有一定价值，可以通过修复达到标准")

        # 优先级建议
        high_priority_issues = sum(
            len(analysis['issues']) for analysis in layer_analysis.values()
            if any('语法错误' in issue for issue in analysis['issues'])
        )

        if high_priority_issues > 0:
            recommendations.append(f"🚨 发现 {high_priority_issues} 个语法错误需要立即修复")
            recommendations.append("   语法错误会阻塞构建，应优先处理")

        return recommendations

    def generate_comprehensive_report(self, output_file: str = None) -> Dict[str, Any]:
        """生成综合报告"""
        analysis = self.analyze_layer_dependencies()

        report = {
            'title': 'SQLCC 测试分层依赖分析报告',
            'version': 'v1.0.0',
            'generated_at': '2025-12-24',
            'analysis': analysis,
            'layer_definitions': {
                layer.value: {
                    'name': info.name,
                    'description': info.description,
                    'level': layer.value.split('_')[1]  # 提取层级数字
                }
                for layer, info in self.layers.items()
            }
        }

        if output_file:
            # 自定义JSON编码器处理枚举和集合
            class CustomJSONEncoder(json.JSONEncoder):
                def default(self, obj):
                    if isinstance(obj, TestLayer):
                        return obj.value
                    elif isinstance(obj, set):
                        return list(obj)
                    return super().default(obj)

            with open(output_file, 'w', encoding='utf-8') as f:
                json.dump(report, f, indent=2, ensure_ascii=False, cls=CustomJSONEncoder)
            print(f"💾 详细报告已保存到: {output_file}")

        return report


def main():
    parser = argparse.ArgumentParser(description='测试分层依赖分析器')
    parser.add_argument('--project-root', default='.', help='项目根目录')
    parser.add_argument('--output', default='test_layer_analysis_report.json', help='输出报告文件')
    parser.add_argument('--layer', help='指定分析特定层级')

    args = parser.parse_args()

    analyzer = TestLayerAnalyzer(args.project_root)
    report = analyzer.generate_comprehensive_report(args.output)

    # 打印摘要
    summary = report['analysis']['summary']
    print("\n" + "="*60)
    print("📊 SQLCC 测试分层依赖分析报告")
    print("="*60)
    print(f"总层级数: {summary['total_layers']}")
    print(f"总BUILD文件数: {summary['total_build_files']}")
    print(f"总依赖数: {summary['total_dependencies']}")
    print(f"总问题数: {summary['total_issues']}")
    print(f"建议重新生成的层级: {len(summary['regeneration_candidates'])}个")

    if summary['regeneration_candidates']:
        print(f"  - {', '.join(summary['regeneration_candidates'])}")

    print("\n层级文件分布:")
    for layer, count in summary['layer_distribution'].items():
        if count > 0:
            print(f"  {layer}: {count} 个文件")

    print("\n💡 修复建议:")
    for rec in report['analysis']['recommendations']:
        print(f"   {rec}")

    print(f"\n💾 详细报告已保存到: {args.output}")


if __name__ == '__main__':
    main()
