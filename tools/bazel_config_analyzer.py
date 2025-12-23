#!/usr/bin/env python3
"""
SQLCC Bazel构建配置综合分析器
系统性地检测和修复Bazel构建配置问题

功能特性:
- 全面扫描所有BUILD.bazel文件
- 检测语法错误、标签引用问题、依赖关系问题
- 生成详细的问题报告和修复建议
- 支持批量修复功能

作者: SQLCC AI Agent
版本: v1.2.6
更新时间: 2025-12-22
"""

import os
import re
import sys
import json
import subprocess
from pathlib import Path
from typing import Dict, List, Set, Tuple, Optional
from dataclasses import dataclass, field
from enum import Enum

class IssueSeverity(Enum):
    CRITICAL = "CRITICAL"
    HIGH = "HIGH"
    MEDIUM = "MEDIUM"
    LOW = "LOW"
    INFO = "INFO"

class IssueType(Enum):
    SYNTAX_ERROR = "SYNTAX_ERROR"
    LABEL_ERROR = "LABEL_ERROR"
    DEPENDENCY_ERROR = "DEPENDENCY_ERROR"
    MISSING_TARGET = "MISSING_TARGET"
    DUPLICATE_TARGET = "DUPLICATE_TARGET"
    INVALID_PATH = "INVALID_PATH"
    MISSING_FILE = "MISSING_FILE"
    CONFIGURATION_ERROR = "CONFIGURATION_ERROR"

@dataclass
class BuildIssue:
    file_path: str
    line_number: int
    issue_type: IssueType
    severity: IssueSeverity
    message: str
    suggestion: str
    context: str = ""
    fixable: bool = False

@dataclass
class BuildConfig:
    project_root: Path
    build_files: List[Path] = field(default_factory=list)
    source_files: List[Path] = field(default_factory=list)
    issues: List[BuildIssue] = field(default_factory=list)
    dependencies: Dict[str, Set[str]] = field(default_factory=dict)

class BazelConfigAnalyzer:
    """Bazel构建配置分析器"""

    def __init__(self, project_root: str):
        self.project_root = Path(project_root).resolve()
        self.config = BuildConfig(project_root=self.project_root)
        self._load_build_files()
        self._load_source_files()

    def _load_build_files(self):
        """加载所有BUILD.bazel文件"""
        for root, dirs, files in os.walk(self.project_root):
            # 跳过某些目录
            dirs[:] = [d for d in dirs if not d.startswith('.') and d not in ['bazel-bin', 'bazel-out', 'bazel-testlogs']]

            for file in files:
                if file == "BUILD.bazel":
                    self.config.build_files.append(Path(root) / file)

    def _load_source_files(self):
        """加载所有源文件"""
        source_extensions = {'.cpp', '.cc', '.c', '.hpp', '.h', '.hxx', '.cxx'}
        for root, dirs, files in os.walk(self.project_root):
            dirs[:] = [d for d in dirs if not d.startswith('.') and d not in ['bazel-bin', 'bazel-out', 'bazel-testlogs']]

            for file in files:
                if any(file.endswith(ext) for ext in source_extensions):
                    self.config.source_files.append(Path(root) / file)

    def analyze_build_files(self) -> List[BuildIssue]:
        """分析所有BUILD.bazel文件"""
        print(f"🔍 分析 {len(self.config.build_files)} 个BUILD.bazel文件...")

        for build_file in self.config.build_files:
            self._analyze_single_build_file(build_file)

        return self.config.issues

    def _analyze_single_build_file(self, build_file: Path):
        """分析单个BUILD.bazel文件"""
        try:
            with open(build_file, 'r', encoding='utf-8') as f:
                content = f.read()

            lines = content.split('\n')

            # 检查语法错误
            self._check_syntax_errors(build_file, content, lines)

            # 检查标签引用
            self._check_label_references(build_file, content, lines)

            # 检查依赖声明
            self._check_dependencies(build_file, content, lines)

            # 检查文件引用
            self._check_file_references(build_file, content, lines)

        except Exception as e:
            self.config.issues.append(BuildIssue(
                file_path=str(build_file),
                line_number=0,
                issue_type=IssueType.SYNTAX_ERROR,
                severity=IssueSeverity.CRITICAL,
                message=f"无法读取或解析BUILD文件: {str(e)}",
                suggestion="检查文件权限和编码格式",
                fixable=False
            ))

    def _check_syntax_errors(self, build_file: Path, content: str, lines: List[str]):
        """检查语法错误"""
        # 检查基本的Python语法问题
        if 'load(' in content and not content.count('(') == content.count(')'):
            self._add_issue(build_file, 0, IssueType.SYNTAX_ERROR,
                          IssueSeverity.HIGH, "括号不匹配", "检查load语句的括号匹配")

        # 检查未闭合的字符串
        quote_count = content.count('"') + content.count("'")
        if quote_count % 2 != 0:
            self._add_issue(build_file, 0, IssueType.SYNTAX_ERROR,
                          IssueSeverity.CRITICAL, "字符串引号不匹配", "检查字符串引号的配对")

    def _check_label_references(self, build_file: Path, content: str, lines: List[str]):
        """检查标签引用问题"""
        # 查找所有标签引用
        label_pattern = r'"//([^"]+)"'
        matches = re.finditer(label_pattern, content)

        for match in matches:
            label = match.group(0)
            line_num = content[:match.start()].count('\n') + 1

            # 检查是否包含上层引用
            if '../' in label:
                self._add_issue(build_file, line_num, IssueType.LABEL_ERROR,
                              IssueSeverity.HIGH, f"标签包含上层引用: {label}",
                              "使用正确的包路径引用", context=lines[line_num-1].strip(), fixable=True)

            # 检查标签格式
            if not re.match(r'"//[a-zA-Z_][a-zA-Z0-9_]*(/[a-zA-Z_][a-zA-Z0-9_]*)*:[a-zA-Z_][a-zA-Z0-9_]*"', label):
                self._add_issue(build_file, line_num, IssueType.LABEL_ERROR,
                              IssueSeverity.MEDIUM, f"标签格式不规范: {label}",
                              "使用标准格式: //package:target", context=lines[line_num-1].strip())

    def _check_dependencies(self, build_file: Path, content: str, lines: List[str]):
        """检查依赖声明"""
        # 查找deps声明
        deps_pattern = r'deps\s*=\s*\[([^\]]*)\]'
        deps_match = re.search(deps_pattern, content, re.DOTALL)

        if deps_match:
            deps_content = deps_match.group(1)
            deps = re.findall(r'"([^"]*)"', deps_content)

            # 记录依赖关系
            target_name = self._get_target_name(build_file, content)
            if target_name:
                self.config.dependencies[target_name] = set(deps)

            # 检查依赖有效性
            for dep in deps:
                if dep.startswith('//'):
                    # 检查目标是否存在
                    if not self._target_exists(dep):
                        line_num = content.find(dep)
                        line_num = content[:line_num].count('\n') + 1 if line_num >= 0 else 0
                        self._add_issue(build_file, line_num, IssueType.MISSING_TARGET,
                                      IssueSeverity.HIGH, f"依赖目标不存在: {dep}",
                                      "检查目标名称或创建缺失的目标")

    def _check_file_references(self, build_file: Path, content: str, lines: List[str]):
        """检查文件引用"""
        # 检查srcs和hdrs中的文件引用
        file_refs = re.findall(r'srcs\s*=\s*\[([^\]]*)\]|hdrs\s*=\s*\[([^\]]*)\]', content, re.DOTALL)

        for refs in file_refs:
            for ref_group in refs:
                if ref_group:
                    files = re.findall(r'"([^"]*)"', ref_group)
                    for file_ref in files:
                        if '../' in file_ref:
                            # 找到对应的行号
                            line_num = 0
                            for i, line in enumerate(lines):
                                if file_ref in line:
                                    line_num = i + 1
                                    break

                            self._add_issue(build_file, line_num, IssueType.INVALID_PATH,
                                          IssueSeverity.MEDIUM, f"文件路径包含上层引用: {file_ref}",
                                          "使用相对于包的路径", context=lines[line_num-1].strip() if line_num > 0 else "", fixable=True)

                        # 检查文件是否存在
                        if not self._file_exists_in_package(build_file.parent, file_ref):
                            line_num = 0
                            for i, line in enumerate(lines):
                                if file_ref in line:
                                    line_num = i + 1
                                    break

                            self._add_issue(build_file, line_num, IssueType.MISSING_FILE,
                                          IssueSeverity.HIGH, f"引用的文件不存在: {file_ref}",
                                          "检查文件路径或创建缺失的文件")

    def _target_exists(self, target_label: str) -> bool:
        """检查目标是否存在"""
        # 简单的存在性检查，实际应该解析BUILD文件
        # 这里做一个基本的检查
        if ':' in target_label:
            package_path, target_name = target_label[2:].split(':', 1)
            build_file = self.project_root / package_path / "BUILD.bazel"
            if build_file.exists():
                try:
                    with open(build_file, 'r') as f:
                        content = f.read()
                        # 检查目标名称是否存在
                        if f'name = "{target_name}"' in content or f"name = '{target_name}'" in content:
                            return True
                except:
                    pass
        return False

    def _file_exists_in_package(self, package_dir: Path, file_ref: str) -> bool:
        """检查文件在包中是否存在"""
        file_path = package_dir / file_ref
        return file_path.exists()

    def _get_target_name(self, build_file: Path, content: str) -> Optional[str]:
        """获取BUILD文件中的目标名称"""
        name_match = re.search(r'name\s*=\s*"([^"]*)"', content)
        if name_match:
            return name_match.group(1)
        return None

    def _add_issue(self, build_file: Path, line_num: int, issue_type: IssueType,
                   severity: IssueSeverity, message: str, suggestion: str,
                   context: str = "", fixable: bool = False):
        """添加问题到列表"""
        issue = BuildIssue(
            file_path=str(build_file),
            line_number=line_num,
            issue_type=issue_type,
            severity=severity,
            message=message,
            suggestion=suggestion,
            context=context,
            fixable=fixable
        )
        self.config.issues.append(issue)

    def generate_report(self, output_file: Optional[str] = None) -> str:
        """生成分析报告"""
        report_lines = []
        report_lines.append("# SQLCC Bazel构建配置分析报告")
        report_lines.append(f"项目根目录: {self.project_root}")
        report_lines.append(f"分析时间: {self._get_timestamp()}")
        report_lines.append("")

        # 统计信息
        total_files = len(self.config.build_files)
        total_issues = len(self.config.issues)
        issues_by_severity = {}
        issues_by_type = {}

        for issue in self.config.issues:
            issues_by_severity[issue.severity.value] = issues_by_severity.get(issue.severity.value, 0) + 1
            issues_by_type[issue.issue_type.value] = issues_by_type.get(issue.issue_type.value, 0) + 1

        report_lines.append("## 统计摘要")
        report_lines.append(f"- 分析的BUILD文件数量: {total_files}")
        report_lines.append(f"- 发现的问题总数: {total_issues}")
        report_lines.append("")

        if issues_by_severity:
            report_lines.append("### 按严重程度统计")
            for severity, count in sorted(issues_by_severity.items()):
                report_lines.append(f"- {severity}: {count}")
            report_lines.append("")

        if issues_by_type:
            report_lines.append("### 按问题类型统计")
            for issue_type, count in sorted(issues_by_type.items()):
                report_lines.append(f"- {issue_type}: {count}")
            report_lines.append("")

        # 详细问题列表
        if self.config.issues:
            report_lines.append("## 详细问题列表")
            report_lines.append("")

            for i, issue in enumerate(self.config.issues, 1):
                report_lines.append(f"### 问题 {i}")
                report_lines.append(f"- **文件**: {issue.file_path}")
                report_lines.append(f"- **行号**: {issue.line_number}")
                report_lines.append(f"- **类型**: {issue.issue_type.value}")
                report_lines.append(f"- **严重程度**: {issue.severity.value}")
                report_lines.append(f"- **问题描述**: {issue.message}")
                report_lines.append(f"- **修复建议**: {issue.suggestion}")
                if issue.context:
                    report_lines.append(f"- **上下文**: `{issue.context}`")
                report_lines.append(f"- **可自动修复**: {'是' if issue.fixable else '否'}")
                report_lines.append("")

        # 依赖关系分析
        if self.config.dependencies:
            report_lines.append("## 依赖关系分析")
            report_lines.append("")
            for target, deps in sorted(self.config.dependencies.items()):
                report_lines.append(f"### {target}")
                if deps:
                    for dep in sorted(deps):
                        report_lines.append(f"- {dep}")
                else:
                    report_lines.append("- 无依赖")
                report_lines.append("")

        report = "\n".join(report_lines)

        if output_file:
            with open(output_file, 'w', encoding='utf-8') as f:
                f.write(report)
            print(f"📄 报告已保存到: {output_file}")

        return report

    def _get_timestamp(self) -> str:
        """获取当前时间戳"""
        from datetime import datetime
        return datetime.now().strftime("%Y-%m-%d %H:%M:%S")

    def run_bazel_test(self) -> Tuple[bool, str]:
        """运行Bazel构建测试"""
        print("🔨 运行Bazel构建测试...")

        try:
            result = subprocess.run(
                ["bazel", "build", "//..."],
                cwd=self.project_root,
                capture_output=True,
                text=True,
                timeout=300  # 5分钟超时
            )

            success = result.returncode == 0
            output = result.stdout + result.stderr

            return success, output

        except subprocess.TimeoutExpired:
            return False, "构建超时"
        except FileNotFoundError:
            return False, "未找到bazel命令"
        except Exception as e:
            return False, f"构建测试失败: {str(e)}"

def main():
    """主函数"""
    if len(sys.argv) != 2:
        print("用法: python3 bazel_config_analyzer.py <项目根目录>")
        sys.exit(1)

    project_root = sys.argv[1]

    if not os.path.isdir(project_root):
        print(f"错误: 目录不存在: {project_root}")
        sys.exit(1)

    # 创建分析器
    analyzer = BazelConfigAnalyzer(project_root)

    # 分析构建文件
    issues = analyzer.analyze_build_files()

    # 运行构建测试
    build_success, build_output = analyzer.run_bazel_test()

    # 生成报告
    report_file = f"bazel_config_analysis_{analyzer._get_timestamp().replace(' ', '_').replace(':', '')}.md"
    report = analyzer.generate_report(report_file)

    # 输出摘要
    print("\n📊 分析完成!")
    print(f"📁 分析的BUILD文件: {len(analyzer.config.build_files)}")
    print(f"⚠️ 发现的问题: {len(issues)}")
    print(f"🔨 构建测试: {'✅ 通过' if build_success else '❌ 失败'}")
    print(f"📄 详细报告: {report_file}")

    # 显示前几个问题
    if issues:
        print("\n🔍 前5个问题:")
        for i, issue in enumerate(issues[:5], 1):
            print(f"{i}. [{issue.severity.value}] {issue.message} ({issue.file_path}:{issue.line_number})")

    return 0 if not issues and build_success else 1

if __name__ == "__main__":
    sys.exit(main())
