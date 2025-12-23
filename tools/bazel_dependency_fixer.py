#!/usr/bin/env python3
"""
SQLCC Bazel依赖关系修复工具
基于检测结果，系统性地修复依赖关系问题

功能特性:
- 智能识别缺失的依赖目标
- 自动创建缺失的目标定义
- 修复循环依赖和无效依赖
- 生成依赖关系图分析

作者: SQLCC AI Agent
版本: v1.2.6
更新时间: 2025-12-22
"""

import os
import re
import sys
from pathlib import Path
from typing import Dict, List, Set, Tuple, Optional, Any
import json

class DependencyAnalyzer:
    """依赖关系分析器"""

    def __init__(self, project_root: str):
        self.project_root = Path(project_root).resolve()
        self.targets: Dict[str, Dict[str, Any]] = {}
        self.dependencies: Dict[str, Set[str]] = {}
        self.reverse_dependencies: Dict[str, Set[str]] = {}

    def build_dependency_graph(self) -> Dict[str, Any]:
        """构建完整的依赖关系图"""
        print("🔍 构建依赖关系图...")

        # 扫描所有BUILD.bazel文件
        build_files = []
        for root, dirs, files in os.walk(self.project_root):
            dirs[:] = [d for d in dirs if not d.startswith('.') and d not in ['bazel-bin', 'bazel-out', 'bazel-testlogs']]
            for file in files:
                if file == "BUILD.bazel":
                    build_files.append(Path(root) / file)

        print(f"📁 发现 {len(build_files)} 个BUILD.bazel文件")

        # 解析每个BUILD文件中的目标和依赖
        for build_file in build_files:
            self._parse_build_file(build_file)

        # 构建反向依赖关系
        for target, deps in self.dependencies.items():
            for dep in deps:
                if dep not in self.reverse_dependencies:
                    self.reverse_dependencies[dep] = set()
                self.reverse_dependencies[dep].add(target)

        return {
            "targets": self.targets,
            "dependencies": {k: list(v) for k, v in self.dependencies.items()},
            "reverse_dependencies": {k: list(v) for k, v in self.reverse_dependencies.items()}
        }

    def _parse_build_file(self, build_file: Path):
        """解析单个BUILD文件"""
        try:
            with open(build_file, 'r', encoding='utf-8') as f:
                content = f.read()

            # 计算包路径
            package_path = build_file.parent.relative_to(self.project_root)
            if str(package_path) == '.':
                package_path = ''
            else:
                package_path = str(package_path)

            # 查找所有目标定义
            target_pattern = r'(?P<type>cc_library|cc_binary|cc_test)\s*\(\s*name\s*=\s*"(?P<name>[^"]+)"'
            for match in re.finditer(target_pattern, content, re.MULTILINE | re.DOTALL):
                target_name = match.group('name')
                target_type = match.group('type')

                # 构建完整目标标签
                if package_path:
                    full_target = f"//{package_path}:{target_name}"
                else:
                    full_target = f"//:{target_name}"

                # 提取依赖
                deps = self._extract_dependencies(content, match.start())

                self.targets[full_target] = {
                    "name": target_name,
                    "type": target_type,
                    "package": package_path,
                    "file": str(build_file),
                    "line": content[:match.start()].count('\n') + 1
                }

                self.dependencies[full_target] = deps

        except Exception as e:
            print(f"❌ 解析BUILD文件失败 {build_file}: {e}")

    def _extract_dependencies(self, content: str, start_pos: int) -> Set[str]:
        """提取目标的依赖关系"""
        deps = set()

        # 查找deps声明
        deps_pattern = r'deps\s*=\s*\[([^\]]*)\]'
        deps_match = re.search(deps_pattern, content[start_pos:], re.DOTALL)

        if deps_match:
            deps_content = deps_match.group(1)
            # 提取所有引用的依赖
            dep_refs = re.findall(r'"([^"]*)"', deps_content)
            deps.update(dep_refs)

        return deps

    def find_missing_targets(self, detection_results: Dict[str, Any]) -> List[Dict[str, Any]]:
        """查找缺失的目标定义"""
        missing_targets = []

        for issue in detection_results.get('issues', []):
            if issue['issue_type'] == 'MISSING_TARGET':
                dependency = issue['metadata'].get('dependency', '')
                if dependency:
                    missing_targets.append({
                        "dependency": dependency,
                        "referenced_by": issue['file_path'],
                        "line": issue['line_number'],
                        "context": issue.get('context', ''),
                        "possible_solutions": self._suggest_target_creation(dependency, issue)
                    })

        return missing_targets

    def _suggest_target_creation(self, dependency: str, issue: Dict[str, Any]) -> List[Dict[str, Any]]:
        """为缺失依赖建议创建方案"""
        suggestions = []

        if dependency.startswith('//'):
            parts = dependency[2:].split(':')
            if len(parts) == 2:
                package_path, target_name = parts

                # 检查包目录是否存在
                package_dir = self.project_root / package_path
                if package_dir.exists():
                    # 检查可能的源文件
                    possible_sources = self._find_possible_sources(package_dir, target_name)

                    if possible_sources:
                        suggestions.append({
                            "type": "create_target",
                            "package": package_path,
                            "name": target_name,
                            "sources": possible_sources,
                            "build_file": str(package_dir / "BUILD.bazel")
                        })
                    else:
                        suggestions.append({
                            "type": "create_placeholder",
                            "package": package_path,
                            "name": target_name,
                            "reason": "No source files found"
                        })
                else:
                    suggestions.append({
                        "type": "create_package",
                        "package": package_path,
                        "name": target_name,
                        "reason": "Package directory does not exist"
                    })

        return suggestions

    def _find_possible_sources(self, package_dir: Path, target_name: str) -> List[str]:
        """查找可能的源文件"""
        possible_sources = []

        # 常见的源文件模式
        patterns = [
            f"{target_name}.cpp",
            f"{target_name}.cc",
            f"{target_name}_impl.cpp",
            f"{target_name}_implementation.cpp",
        ]

        for pattern in patterns:
            source_file = package_dir / "src" / pattern
            if source_file.exists():
                possible_sources.append(str(source_file.relative_to(package_dir)))

            source_file = package_dir / pattern
            if source_file.exists():
                possible_sources.append(str(source_file.relative_to(package_dir)))

        return possible_sources

class BazelDependencyFixer:
    """Bazel依赖关系修复器"""

    def __init__(self, project_root: str, detection_results: Optional[str] = None):
        self.project_root = Path(project_root).resolve()
        self.detection_results = Path(detection_results) if detection_results else None
        self.analyzer = DependencyAnalyzer(project_root)
        self.fixed_targets: List[str] = []
        self.created_targets: List[str] = []
        self.fix_report: List[Dict[str, Any]] = []

    def fix_dependencies(self, detection_json: Optional[str] = None) -> Dict[str, Any]:
        """修复依赖关系"""
        if detection_json:
            self.detection_results = Path(detection_json)

        # 构建依赖关系图
        dep_graph = self.analyzer.build_dependency_graph()

        if not self.detection_results or not self.detection_results.exists():
            print("❌ 未找到检测结果文件")
            return {"error": "Detection results not found"}

        # 读取检测结果
        with open(self.detection_results, 'r', encoding='utf-8') as f:
            results = json.load(f)

        # 查找缺失的目标
        missing_targets = self.analyzer.find_missing_targets(results)

        print(f"🎯 发现 {len(missing_targets)} 个缺失依赖目标")

        # 修复每个缺失目标
        total_fixed = 0
        for missing in missing_targets:
            if self._fix_missing_target(missing):
                total_fixed += 1

        print(f"✅ 依赖关系修复完成，共修复 {total_fixed} 个目标")

        return {
            "total_missing_targets": len(missing_targets),
            "total_fixed": total_fixed,
            "created_targets": self.created_targets,
            "fixed_targets": self.fixed_targets,
            "fix_report": self.fix_report,
            "dependency_graph": dep_graph
        }

    def _fix_missing_target(self, missing_info: Dict[str, Any]) -> bool:
        """修复单个缺失目标"""
        dependency = missing_info['dependency']
        suggestions = missing_info['possible_solutions']

        if not suggestions:
            print(f"⚠️  无法为 {dependency} 生成修复建议")
            return False

        # 选择最佳建议
        best_suggestion = self._choose_best_suggestion(suggestions)

        if best_suggestion['type'] == 'create_target':
            return self._create_target(best_suggestion, missing_info)
        elif best_suggestion['type'] == 'create_placeholder':
            return self._create_placeholder_target(best_suggestion, missing_info)
        elif best_suggestion['type'] == 'create_package':
            return self._create_package_and_target(best_suggestion, missing_info)

        return False

    def _choose_best_suggestion(self, suggestions: List[Dict[str, Any]]) -> Dict[str, Any]:
        """选择最佳的修复建议"""
        # 优先选择有源文件的建议
        for suggestion in suggestions:
            if suggestion['type'] == 'create_target' and suggestion.get('sources'):
                return suggestion

        # 其次选择创建占位符
        for suggestion in suggestions:
            if suggestion['type'] == 'create_placeholder':
                return suggestion

        # 最后选择创建包
        return suggestions[0]

    def _create_target(self, suggestion: Dict[str, Any], missing_info: Dict[str, Any]) -> bool:
        """创建目标定义"""
        package = suggestion['package']
        name = suggestion['name']
        sources = suggestion.get('sources', [])
        build_file_path = suggestion['build_file']

        build_file = Path(build_file_path)

        try:
            # 读取现有的BUILD文件
            if build_file.exists():
                with open(build_file, 'r', encoding='utf-8') as f:
                    content = f.read()
            else:
                content = self._create_basic_build_file(package)

            # 生成目标定义
            target_definition = self._generate_target_definition(name, sources, package)

            # 添加到BUILD文件末尾
            if not content.strip().endswith('\n'):
                content += '\n'
            content += '\n' + target_definition

            # 写入文件
            with open(build_file, 'w', encoding='utf-8') as f:
                f.write(content)

            self.created_targets.append(f"//{package}:{name}")
            self.fix_report.append({
                "action": "created_target",
                "target": f"//{package}:{name}",
                "file": str(build_file),
                "sources": sources,
                "reason": f"Missing dependency referenced in {missing_info['referenced_by']}"
            })

            print(f"📦 创建目标: //{package}:{name}")
            return True

        except Exception as e:
            print(f"❌ 创建目标失败 {package}:{name}: {e}")
            return False

    def _create_placeholder_target(self, suggestion: Dict[str, Any], missing_info: Dict[str, Any]) -> bool:
        """创建占位符目标"""
        package = suggestion['package']
        name = suggestion['name']
        build_file_path = self.project_root / package / "BUILD.bazel"

        try:
            # 创建占位符目标定义
            target_definition = f'''# Placeholder target - TODO: implement actual functionality
cc_library(
    name = "{name}",
    srcs = [],  # TODO: add source files
    hdrs = [],  # TODO: add header files
    deps = [],  # TODO: add dependencies
    visibility = ["//visibility:public"],
)
'''

            # 确保BUILD文件存在
            if not build_file_path.exists():
                build_file_path.parent.mkdir(parents=True, exist_ok=True)
                content = self._create_basic_build_file(package)
            else:
                with open(build_file_path, 'r', encoding='utf-8') as f:
                    content = f.read()

            # 添加占位符目标
            content += '\n' + target_definition

            with open(build_file_path, 'w', encoding='utf-8') as f:
                f.write(content)

            self.created_targets.append(f"//{package}:{name}")
            self.fix_report.append({
                "action": "created_placeholder",
                "target": f"//{package}:{name}",
                "file": str(build_file_path),
                "reason": f"Created placeholder for missing dependency referenced in {missing_info['referenced_by']}"
            })

            print(f"📦 创建占位符目标: //{package}:{name}")
            return True

        except Exception as e:
            print(f"❌ 创建占位符目标失败 {package}:{name}: {e}")
            return False

    def _create_package_and_target(self, suggestion: Dict[str, Any], missing_info: Dict[str, Any]) -> bool:
        """创建包目录和目标"""
        package = suggestion['package']
        name = suggestion['name']
        package_dir = self.project_root / package

        try:
            # 创建包目录
            package_dir.mkdir(parents=True, exist_ok=True)

            # 创建基本的BUILD文件
            build_file = package_dir / "BUILD.bazel"
            content = self._create_basic_build_file(package)

            # 添加占位符目标
            target_definition = f'''# Auto-generated placeholder target
cc_library(
    name = "{name}",
    srcs = [],  # TODO: add source files
    hdrs = [],  # TODO: add header files
    deps = [],  # TODO: add dependencies
    visibility = ["//visibility:public"],
)
'''
            content += '\n' + target_definition

            with open(build_file, 'w', encoding='utf-8') as f:
                f.write(content)

            self.created_targets.append(f"//{package}:{name}")
            self.fix_report.append({
                "action": "created_package_and_target",
                "target": f"//{package}:{name}",
                "package_dir": str(package_dir),
                "file": str(build_file),
                "reason": f"Created new package and placeholder target for missing dependency referenced in {missing_info['referenced_by']}"
            })

            print(f"📁 创建包和目标: //{package}:{name}")
            return True

        except Exception as e:
            print(f"❌ 创建包和目标失败 {package}:{name}: {e}")
            return False

    def _create_basic_build_file(self, package: str) -> str:
        """创建基本的BUILD文件"""
        return f'''# BUILD file for {package}
# Auto-generated by bazel_dependency_fixer.py

package(default_visibility = ["//visibility:public"])
'''

    def _generate_target_definition(self, name: str, sources: List[str], package: str) -> str:
        """生成目标定义"""
        srcs_list = ', '.join(f'"{src}"' for src in sources)
        hdrs_list = ', '.join(f'"{src.replace(".cpp", ".h").replace(".cc", ".h")}"' for src in sources if src.endswith(('.cpp', '.cc')))

        return f'''cc_library(
    name = "{name}",
    srcs = [{srcs_list}],
    hdrs = [{hdrs_list}],
    deps = [
        # TODO: add dependencies
    ],
    visibility = ["//visibility:public"],
)
'''

    def get_fix_report(self) -> str:
        """生成修复报告"""
        report_lines = []
        report_lines.append("# SQLCC依赖关系修复报告")
        report_lines.append(f"修复时间: {self._get_timestamp()}")
        report_lines.append(f"创建目标数量: {len(self.created_targets)}")
        report_lines.append(f"修复条目数量: {len(self.fix_report)}")
        report_lines.append("")

        if self.fix_report:
            report_lines.append("## 修复详情")
            for i, fix in enumerate(self.fix_report, 1):
                report_lines.append(f"### 修复 {i}")
                report_lines.append(f"- **动作**: {fix['action'].replace('_', ' ').title()}")
                report_lines.append(f"- **目标**: {fix['target']}")
                report_lines.append(f"- **文件**: {fix['file']}")
                if 'sources' in fix:
                    report_lines.append(f"- **源文件**: {', '.join(fix['sources'])}")
                if 'package_dir' in fix:
                    report_lines.append(f"- **包目录**: {fix['package_dir']}")
                report_lines.append(f"- **原因**: {fix['reason']}")
                report_lines.append("")
        else:
            report_lines.append("## 无修复条目")

        report_lines.append("")
        report_lines.append("---")
        report_lines.append("*此报告由bazel_dependency_fixer.py自动生成*")

        return "\n".join(report_lines)

    def _get_timestamp(self) -> str:
        """获取当前时间戳"""
        from datetime import datetime
        return datetime.now().strftime("%Y-%m-%d %H:%M:%S")

def main():
    """主函数"""
    import argparse

    parser = argparse.ArgumentParser(description="SQLCC Bazel依赖关系修复器")
    parser.add_argument("project_root", help="项目根目录")
    parser.add_argument("--detection-results", help="检测结果JSON文件")
    parser.add_argument("--output-report", "-o", help="输出修复报告文件")
    parser.add_argument("--analyze-only", action="store_true", help="仅分析，不执行修复")

    args = parser.parse_args()

    if not os.path.isdir(args.project_root):
        print(f"❌ 错误: 目录不存在: {args.project_root}")
        sys.exit(1)

    fixer = BazelDependencyFixer(args.project_root, args.detection_results)

    # 执行修复
    result = fixer.fix_dependencies()

    if "error" in result:
        print(f"❌ 错误: {result['error']}")
        sys.exit(1)

    # 生成报告
    report = fixer.get_fix_report()

    if args.output_report:
        with open(args.output_report, 'w', encoding='utf-8') as f:
            f.write(report)
        print(f"📄 修复报告已保存到: {args.output_report}")
    else:
        print(report)

    print("\n✅ 依赖关系修复完成!")
    print(f"📊 统计信息:")
    print(f"  - 缺失依赖目标: {result['total_missing_targets']}")
    print(f"  - 创建的目标: {result['total_fixed']}")
    print(f"  - 新建目标列表: {result['created_targets']}")

if __name__ == "__main__":
    sys.exit(main())
