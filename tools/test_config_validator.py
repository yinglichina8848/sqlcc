#!/usr/bin/env python3
"""
测试配置验证器 (Test Config Validator)
用于验证和修复Bazel BUILD.bazel配置文件的标准化问题

功能特性:
- BUILD.bazel配置标准化检查
- 依赖完整性验证
- 循环依赖检测
- 自动化修复建议

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


class IssueSeverity(Enum):
    """问题严重程度"""
    ERROR = "error"
    WARNING = "warning"
    INFO = "info"


class IssueType(Enum):
    """问题类型"""
    DEPENDENCY_MISSING = "dependency_missing"
    DEPENDENCY_UNUSED = "dependency_unused"
    DEPENDENCY_DUPLICATE = "dependency_duplicate"
    DEPENDENCY_CIRCULAR = "dependency_circular"
    CONFIG_INCONSISTENT = "config_inconsistent"
    STANDARD_VIOLATION = "standard_violation"


@dataclass
class ConfigIssue:
    """配置问题"""
    file_path: str
    line_number: int
    issue_type: IssueType
    severity: IssueSeverity
    message: str
    suggestion: str
    auto_fixable: bool = False


@dataclass
class ValidationResult:
    """验证结果"""
    file_path: str
    issues: List[ConfigIssue]
    is_valid: bool
    summary: Dict[str, int]


class StandardConfig:
    """标准配置模板"""

    # 标准测试依赖项
    STANDARD_TEST_DEPS = [
        "@com_google_googletest//:gtest_main",
    ]

    # 标准编译选项
    STANDARD_COPTS = [
        "-std=c++20",
        "-stdlib=libc++",
    ]

    # 标准链接选项
    STANDARD_LINKOPTS = [
        "-stdlib=libc++",
        "-lc++abi",
    ]

    # 标准特性
    STANDARD_FEATURES = [
        "cpp20_modules",
    ]


class TestConfigValidator:
    """测试配置验证器"""

    def __init__(self, project_root: str):
        self.project_root = Path(project_root)
        self.standard_config = StandardConfig()
        self.dependency_graph = {}
        self.reverse_graph = {}

    def validate_file(self, file_path: Path) -> ValidationResult:
        """验证单个BUILD文件"""
        issues = []
        content = file_path.read_text()

        # 解析依赖关系
        deps_info = self._parse_dependencies(content)

        # 验证依赖项
        issues.extend(self._validate_dependencies(file_path, deps_info))

        # 验证配置标准化
        issues.extend(self._validate_standardization(file_path, content))

        # 检查循环依赖
        issues.extend(self._check_circular_dependencies(file_path, deps_info))

        # 生成摘要
        summary = self._generate_summary(issues)
        is_valid = not any(issue.severity == IssueSeverity.ERROR for issue in issues)

        # 计算相对路径，如果不在项目根目录下则使用绝对路径
        try:
            relative_path = file_path.relative_to(self.project_root)
        except ValueError:
            relative_path = file_path

        return ValidationResult(
            file_path=str(relative_path),
            issues=issues,
            is_valid=is_valid,
            summary=summary
        )

    def _parse_dependencies(self, content: str) -> Dict[str, List[str]]:
        """解析BUILD文件中的依赖关系"""
        deps_info = {}

        # 找到所有的cc_test规则
        test_pattern = r'cc_test\(\s*name\s*=\s*"([^"]+)"(.*?)\)'

        for match in re.finditer(test_pattern, content, re.DOTALL):
            test_name = match.group(1)
            test_content = match.group(2)

            # 提取deps
            deps_match = re.search(r'deps\s*=\s*\[([^\]]+)\]', test_content, re.DOTALL)
            if deps_match:
                deps_str = deps_match.group(1)
                deps = re.findall(r'"([^"]+)"', deps_str)
                deps_info[test_name] = deps

        return deps_info

    def _validate_dependencies(self, file_path: Path, deps_info: Dict[str, List[str]]) -> List[ConfigIssue]:
        """验证依赖项"""
        issues = []

        for test_name, deps in deps_info.items():
            # 检查重复依赖
            seen_deps = set()
            for i, dep in enumerate(deps):
                if dep in seen_deps:
                    issues.append(ConfigIssue(
                        file_path=str(file_path),
                        line_number=0,  # TODO: 计算实际行号
                        issue_type=IssueType.DEPENDENCY_DUPLICATE,
                        severity=IssueSeverity.WARNING,
                        message=f"测试 '{test_name}' 中存在重复依赖: {dep}",
                        suggestion=f"移除重复的依赖项 {dep}",
                        auto_fixable=True
                    ))
                else:
                    seen_deps.add(dep)

            # 检查缺失的标准依赖
            for std_dep in self.standard_config.STANDARD_TEST_DEPS:
                if std_dep not in deps:
                    issues.append(ConfigIssue(
                        file_path=str(file_path),
                        line_number=0,
                        issue_type=IssueType.DEPENDENCY_MISSING,
                        severity=IssueSeverity.ERROR,
                        message=f"测试 '{test_name}' 缺少标准依赖: {std_dep}",
                        suggestion=f"添加依赖: {std_dep}",
                        auto_fixable=True
                    ))

        return issues

    def _validate_standardization(self, file_path: Path, content: str) -> List[ConfigIssue]:
        """验证配置标准化"""
        issues = []

        # 检查copts标准化
        if '"-std=c++20"' not in content or '"-stdlib=libc++"' not in content:
            issues.append(ConfigIssue(
                file_path=str(file_path),
                line_number=0,
                issue_type=IssueType.STANDARD_VIOLATION,
                severity=IssueSeverity.WARNING,
                message="编译选项不符合标准配置",
                suggestion="使用标准copts配置",
                auto_fixable=True
            ))

        # 检查linkopts标准化
        if '"-stdlib=libc++"' not in content or '"-lc++abi"' not in content:
            issues.append(ConfigIssue(
                file_path=str(file_path),
                line_number=0,
                issue_type=IssueType.STANDARD_VIOLATION,
                severity=IssueSeverity.WARNING,
                message="链接选项不符合标准配置",
                suggestion="使用标准linkopts配置",
                auto_fixable=True
            ))

        return issues

    def _check_circular_dependencies(self, file_path: Path, deps_info: Dict[str, List[str]]) -> List[ConfigIssue]:
        """检查循环依赖"""
        issues = []

        # 构建依赖图
        for test_name, deps in deps_info.items():
            self.dependency_graph[test_name] = []
            for dep in deps:
                # 只考虑内部依赖
                if dep.startswith('//'):
                    dep_name = dep.split(':')[-1]
                    self.dependency_graph[test_name].append(dep_name)

        # 检查循环依赖 (简化实现)
        visited = set()
        recursion_stack = set()

        def has_cycle(node: str) -> bool:
            visited.add(node)
            recursion_stack.add(node)

            for neighbor in self.dependency_graph.get(node, []):
                if neighbor not in visited:
                    if has_cycle(neighbor):
                        return True
                elif neighbor in recursion_stack:
                    return True

            recursion_stack.remove(node)
            return False

        for node in self.dependency_graph:
            if node not in visited:
                if has_cycle(node):
                    issues.append(ConfigIssue(
                        file_path=str(file_path),
                        line_number=0,
                        issue_type=IssueType.DEPENDENCY_CIRCULAR,
                        severity=IssueSeverity.ERROR,
                        message=f"检测到循环依赖，涉及节点: {node}",
                        suggestion="重构依赖关系以消除循环",
                        auto_fixable=False
                    ))

        return issues

    def _generate_summary(self, issues: List[ConfigIssue]) -> Dict[str, int]:
        """生成摘要统计"""
        summary = {
            'total': len(issues),
            'errors': len([i for i in issues if i.severity == IssueSeverity.ERROR]),
            'warnings': len([i for i in issues if i.severity == IssueSeverity.WARNING]),
            'info': len([i for i in issues if i.severity == IssueSeverity.INFO]),
        }
        return summary

    def generate_report(self, results: List[ValidationResult]) -> Dict[str, Any]:
        """生成完整报告"""
        total_files = len(results)
        total_issues = sum(r.summary['total'] for r in results)
        total_errors = sum(r.summary['errors'] for r in results)
        total_warnings = sum(r.summary['warnings'] for r in results)

        # 按类型统计问题
        issues_by_type = {}
        for result in results:
            for issue in result.issues:
                issue_type = issue.issue_type.value
                if issue_type not in issues_by_type:
                    issues_by_type[issue_type] = 0
                issues_by_type[issue_type] += 1

        return {
            'summary': {
                'total_files': total_files,
                'total_issues': total_issues,
                'total_errors': total_errors,
                'total_warnings': total_warnings,
                'valid_files': len([r for r in results if r.is_valid]),
                'invalid_files': len([r for r in results if not r.is_valid]),
            },
            'issues_by_type': issues_by_type,
            'detailed_results': [asdict(r) for r in results],
            'recommendations': self._generate_recommendations(results)
        }

    def _generate_recommendations(self, results: List[ValidationResult]) -> List[str]:
        """生成修复建议"""
        recommendations = []

        total_errors = sum(r.summary['errors'] for r in results)
        total_warnings = sum(r.summary['warnings'] for r in results)

        if total_errors > 0:
            recommendations.append(f"🔴 发现 {total_errors} 个错误需要立即修复")
            recommendations.append("   建议优先修复DEPENDENCY_MISSING和DEPENDENCY_CIRCULAR问题")

        if total_warnings > 0:
            recommendations.append(f"🟡 发现 {total_warnings} 个警告建议修复")
            recommendations.append("   可以考虑修复DEPENDENCY_DUPLICATE和STANDARD_VIOLATION问题")

        # 具体建议
        has_duplicates = any(any(i.issue_type == IssueType.DEPENDENCY_DUPLICATE for i in r.issues) for r in results)
        if has_duplicates:
            recommendations.append("💡 自动修复: 运行 --fix 选项自动移除重复依赖")

        has_standard_violations = any(any(i.issue_type == IssueType.STANDARD_VIOLATION for i in r.issues) for r in results)
        if has_standard_violations:
            recommendations.append("💡 标准化: 运行 --standardize 选项应用标准配置")

        return recommendations

    def apply_fixes(self, results: List[ValidationResult]) -> bool:
        """应用自动修复"""
        success = True

        for result in results:
            if not result.issues:
                continue

            file_path = self.project_root / result.file_path
            try:
                content = file_path.read_text()

                # 应用修复
                for issue in result.issues:
                    if issue.auto_fixable:
                        content = self._apply_fix(content, issue)

                # 写回文件
                file_path.write_text(content)
                print(f"✅ 已修复: {result.file_path}")

            except Exception as e:
                print(f"❌ 修复失败 {result.file_path}: {e}")
                success = False

        return success

    def _apply_fix(self, content: str, issue: ConfigIssue) -> str:
        """应用单个修复"""
        import re

        if issue.issue_type == IssueType.DEPENDENCY_DUPLICATE:
            # 移除重复依赖 - 改进实现

            # 找到所有cc_test规则并处理重复依赖
            def remove_duplicate_deps(match):
                test_content = match.group(2)
                # 找到deps部分
                deps_match = re.search(r'deps\s*=\s*\[([^\]]+)\]', test_content, re.DOTALL)
                if deps_match:
                    deps_str = deps_match.group(1)
                    # 解析依赖项列表
                    deps = []
                    for line in deps_str.split('\n'):
                        line = line.strip()
                        if line.startswith('"') and line.endswith('",'):
                            dep = line.strip('",')
                            if dep and dep not in deps:  # 只保留第一次出现的依赖
                                deps.append(dep)

                    # 重新构建deps字符串
                    new_deps_str = '\n'.join(f'        "{dep}",' for dep in deps)
                    new_test_content = test_content.replace(deps_match.group(0), f'deps = [\n{new_deps_str}\n    ]')
                    return f'cc_test(\n    name = {match.group(1)},{new_test_content}'
                return match.group(0)

            # 应用到所有cc_test规则
            content = re.sub(r'cc_test\(\s*name\s*=\s*"([^"]+)"(.*?)\)', remove_duplicate_deps, content, flags=re.DOTALL)

        elif issue.issue_type == IssueType.DEPENDENCY_MISSING:
            # 添加缺失依赖
            if '"@com_google_googletest//:gtest_main"' in issue.suggestion:
                # 在deps列表开头添加
                content = re.sub(
                    r'(deps\s*=\s*\[)',
                    r'\1\n        "@com_google_googletest//:gtest_main",',
                    content
                )

        elif issue.issue_type == IssueType.STANDARD_VIOLATION:
            # 应用标准配置 - 改进实现
            if "编译选项不符合标准配置" in issue.message:
                # 检查是否已有copts，如果没有则添加
                if 'copts = [' not in content:
                    # 在合适的位置添加copts
                    content = re.sub(
                        r'(cc_test\(\s*name\s*=\s*"[^"]+"[^\)]+\))',
                        r'\1\n    copts = [\n        "-std=c++20",\n        "-stdlib=libc++",\n    ],',
                        content,
                        flags=re.DOTALL
                    )

            if "链接选项不符合标准配置" in issue.message:
                # 检查是否已有linkopts，如果没有则添加
                if 'linkopts = [' not in content:
                    # 在合适的位置添加linkopts
                    content = re.sub(
                        r'(cc_test\(\s*name\s*=\s*"[^"]+"[^\)]+\))',
                        r'\1\n    linkopts = [\n        "-stdlib=libc++",\n        "-lc++abi",\n    ],',
                        content,
                        flags=re.DOTALL
                    )

        return content


def main():
    parser = argparse.ArgumentParser(description='测试配置验证器')
    parser.add_argument('--project-root', default='.', help='项目根目录')
    parser.add_argument('--file', help='指定要验证的文件')
    parser.add_argument('--output', default='config_validation_report.json', help='输出报告文件')
    parser.add_argument('--fix', action='store_true', help='自动应用修复')
    parser.add_argument('--standardize', action='store_true', help='标准化配置')

    args = parser.parse_args()

    validator = TestConfigValidator(args.project_root)
    results = []

    if args.file:
        # 验证单个文件
        file_path = Path(args.project_root) / args.file
        if file_path.exists():
            result = validator.validate_file(file_path)
            results.append(result)
        else:
            print(f"文件不存在: {args.file}")
            sys.exit(1)
    else:
        # 验证所有BUILD文件
        for build_file in Path(args.project_root).rglob('BUILD.bazel'):
            if 'tests' in str(build_file):  # 只验证测试相关的BUILD文件
                result = validator.validate_file(build_file)
                results.append(result)

        # 生成报告
        report = validator.generate_report(results)
        print("📊 配置验证报告:")
        print(f"   总文件数: {report['summary']['total_files']}")
        print(f"   有效文件: {report['summary']['valid_files']}")
        print(f"   无效文件: {report['summary']['invalid_files']}")
        print(f"   总问题数: {report['summary']['total_issues']}")
        print(f"   错误数: {report['summary']['total_errors']}")
        print(f"   警告数: {report['summary']['total_warnings']}")

        if report['recommendations']:
            print("\n💡 修复建议:")
            for rec in report['recommendations']:
                print(f"   {rec}")

        # 保存详细报告 (修复JSON序列化问题)
        def serialize_obj(obj):
            if isinstance(obj, IssueType):
                return obj.value
            elif isinstance(obj, IssueSeverity):
                return obj.value
            raise TypeError(f"Object of type {obj.__class__.__name__} is not JSON serializable")

        with open(args.output, 'w', encoding='utf-8') as f:
            json.dump(report, f, indent=2, ensure_ascii=False, default=serialize_obj)
    print(f"\n💾 详细报告已保存到: {args.output}")

    # 应用自动修复
    if args.fix:
        print("\n🔧 应用自动修复...")
        success = validator.apply_fixes(results)
        if success:
            print("✅ 自动修复完成")
        else:
            print("❌ 部分修复失败，请检查输出")

    # 检查是否成功
    has_errors = report['summary']['total_errors'] > 0
    if has_errors and not args.fix:
        print("\n❌ 发现配置错误，建议运行 --fix 选项自动修复")
        sys.exit(1)


if __name__ == '__main__':
    main()
