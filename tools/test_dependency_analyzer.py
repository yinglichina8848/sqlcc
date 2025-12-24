#!/usr/bin/env python3
"""
测试依赖分析工具 - 分析测试程序依赖关系

功能：
1. 分析测试文件的include依赖
2. 识别依赖层级
3. 生成修复建议
4. 输出文本分析报告
"""

import os
import re
import json
from pathlib import Path

class TestDependencyAnalyzer:
    def __init__(self, project_root: str):
        self.project_root = Path(project_root)
        self.include_pattern = re.compile(r'#include\s*["<]([^">]+)[">]')
        self.test_dependencies = {}  # 测试文件到依赖的映射
        self.layer_tests = {}        # 层级到测试文件的映射

    def analyze_test_file(self, test_file: Path) -> dict:
        """分析单个测试文件的依赖"""
        dependencies = {
            'includes': set(),
            'layers': set()
        }

        try:
            with open(test_file, 'r', encoding='utf-8') as f:
                content = f.read()

            # 提取include语句
            includes = self.include_pattern.findall(content)
            for include in includes:
                dependencies['includes'].add(include)

                # 判断依赖层级
                layer = self.classify_dependency_layer(include)
                if layer:
                    dependencies['layers'].add(layer)

        except Exception as e:
            print(f"Error analyzing {test_file}: {e}")

        return dependencies

    def classify_dependency_layer(self, include_path: str) -> str:
        """根据include路径分类依赖层级"""
        if include_path.startswith('gtest/') or include_path.startswith('gmock/'):
            return 'testing_framework'
        elif not include_path.startswith('sqlcc/') and not include_path.startswith('include/'):
            return 'stdlib'  # 标准库
        elif include_path.startswith('include/core/') or include_path.startswith('core/'):
            return 'core'
        elif include_path.startswith('include/sql_parser/') or include_path.startswith('sql_parser/'):
            return 'sql_parser'
        elif include_path.startswith('include/storage_engine/') or include_path.startswith('storage_engine/'):
            return 'storage_engine'
        elif include_path.startswith('include/execution/') or include_path.startswith('execution/'):
            return 'execution'
        elif include_path.startswith('include/network/') or include_path.startswith('network/'):
            return 'network'
        elif include_path.startswith('include/utils/') or include_path.startswith('utils/'):
            return 'utils'
        elif include_path.startswith('include/exception/') or include_path.startswith('exception/'):
            return 'exception'
        elif include_path.startswith('include/types/') or include_path.startswith('types/'):
            return 'types'
        else:
            return 'other'

    def build_dependency_graph(self, test_dir: str = "tests") -> dict:
        """构建测试依赖图"""
        test_path = self.project_root / test_dir

        print(f"Scanning directory: {test_path}")

        for root, dirs, files in os.walk(test_path):
            for file in files:
                if file.endswith('.cpp') or file.endswith('.cc'):
                    test_file = Path(root) / file
                    print(f"Analyzing: {test_file}")
                    deps = self.analyze_test_file(test_file)

                    # 保存依赖信息
                    test_name = str(test_file.relative_to(self.project_root))
                    self.test_dependencies[test_name] = deps

                    # 按层级分组
                    max_layer = self.get_max_dependency_layer(deps['layers'])
                    layer_key = f"layer_{max_layer}"
                    if layer_key not in self.layer_tests:
                        self.layer_tests[layer_key] = []
                    self.layer_tests[layer_key].append(test_name)

        print(f"Found {len(self.test_dependencies)} test files")
        return self.test_dependencies

    def get_max_dependency_layer(self, test_layers: set) -> int:
        """获取最高的依赖层级"""
        layer_order = {
            'stdlib': 0,
            'testing_framework': 1,
            'utils': 2, 'exception': 2, 'types': 2,
            'core': 3,
            'sql_parser': 4,
            'execution': 5, 'storage_engine': 5,
            'network': 6,
            'other': 7
        }

        max_layer = 0
        for layer in test_layers:
            if layer in layer_order:
                max_layer = max(max_layer, layer_order[layer])

        return max_layer

    def generate_fix_recommendations(self, layers: dict) -> dict:
        """生成修复建议"""
        recommendations = {
            'layer_0_stdlib': [
                "这些测试只依赖标准库，可以直接编译运行",
                "建议添加到独立的BUILD目标中"
            ],
            'layer_1_testing': [
                "这些测试依赖gtest等测试框架",
                "确保BUILD配置中包含测试框架依赖"
            ],
            'layer_2_basic': [
                "这些测试依赖基础组件 (utils, exception, types)",
                "需要确保include路径正确配置"
            ],
            'layer_3_core': [
                "这些测试依赖core模块",
                "可能需要重新组织依赖关系"
            ],
            'layer_4_parser': [
                "这些测试依赖sql_parser (第2层)",
                "当前被误认为是基础测试"
            ],
            'layer_5_engine': [
                "这些测试依赖高级引擎模块",
                "需要完整的项目构建环境"
            ],
            'layer_6_network': [
                "这些测试依赖网络模块",
                "需要网络组件构建环境"
            ],
            'layer_7_integration': [
                "集成测试，需要完整环境",
                "建议单独构建和运行"
            ]
        }

        return recommendations

    def export_analysis_report(self, output_file: str = "test_dependency_analysis.json"):
        """导出分析报告"""
        layers = self.layer_tests
        recommendations = self.generate_fix_recommendations(layers)

        report = {
            'summary': {
                'total_test_files': len(self.test_dependencies),
                'total_layers': len(layers)
            },
            'layer_analysis': layers,
            'recommendations': recommendations,
            'detailed_dependencies': self.test_dependencies
        }

        with open(output_file, 'w', encoding='utf-8') as f:
            json.dump(report, f, indent=2, ensure_ascii=False)

        print(f"Analysis report saved to: {output_file}")
        return report

    def run_full_analysis(self, test_dir: str = "tests", export_report: bool = True) -> dict:
        """运行完整分析"""
        print("🔍 Starting test dependency analysis...")

        # 构建依赖图
        print("📊 Building dependency graph...")
        self.build_dependency_graph(test_dir)

        # 输出层级统计
        print("\n📈 Dependency Layer Analysis:")
        for layer_name, tests in self.layer_tests.items():
            layer_num = layer_name.split('_')[1]
            layer_desc = {
                '0': 'Standard Library Only',
                '1': 'Testing Framework',
                '2': 'Basic Components',
                '3': 'Core Components',
                '4': 'SQL Parser',
                '5': 'Execution Engine',
                '6': 'Network Components',
                '7': 'Integration Tests'
            }.get(layer_num, 'Unknown')

            print(f"  {layer_name}: {len(tests)} tests ({layer_desc})")
            if len(tests) <= 5:  # 只显示少量测试的文件名
                for test in tests[:3]:
                    print(f"    - {Path(test).name}")
                if len(tests) > 3:
                    print(f"    ... and {len(tests) - 3} more")

        # 生成建议
        recommendations = self.generate_fix_recommendations(self.layer_tests)
        print("\n💡 Key Recommendations:")
        for layer, recs in recommendations.items():
            if layer in self.layer_tests and self.layer_tests[layer]:  # 只显示有测试的层级
                print(f"  {layer}:")
                for rec in recs:
                    print(f"    • {rec}")

        # 导出报告
        if export_report:
            report = self.export_analysis_report()
            print("✅ Analysis completed!")
            return report

        return {}

def main():
    """主函数"""
    import argparse

    parser = argparse.ArgumentParser(description="Test Dependency Analyzer")
    parser.add_argument("--project-root", default=".", help="Project root directory")
    parser.add_argument("--test-dir", default="tests", help="Test directory")
    parser.add_argument("--no-report", action="store_true", help="Skip report export")

    args = parser.parse_args()

    analyzer = TestDependencyAnalyzer(args.project_root)
    report = analyzer.run_full_analysis(
        test_dir=args.test_dir,
        export_report=not args.no_report
    )

    return report

if __name__ == "__main__":
    main()
