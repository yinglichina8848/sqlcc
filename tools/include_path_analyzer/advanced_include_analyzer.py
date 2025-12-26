#!/usr/bin/env python3
"""
SQLCC 高级Include路径分析和修复脚本
基于header_index.md文档进行系统性include路径分析和修复
"""

import os
import re
import sys
import json
from pathlib import Path
from typing import Dict, List, Set, Tuple
from dataclasses import dataclass
from collections import defaultdict

@dataclass
class IncludeIssue:
    file_path: str
    line_number: int
    current_include: str
    correct_include: str
    issue_type: str  # 'wrong_path', 'missing_include', 'unused_include'
    severity: str   # 'error', 'warning', 'info'

@dataclass
class HeaderInfo:
    path: str
    module: str
    category: str
    dependencies: List[str]
    classes: List[str]

class AdvancedIncludeAnalyzer:
    def __init__(self, project_root: str):
        self.project_root = Path(project_root)
        self.include_dir = self.project_root / "include"
        self.src_dir = self.project_root / "src"
        self.header_index = self.load_header_index()
        self.issues: List[IncludeIssue] = []
        self.analysis_results = {}

    def load_header_index(self) -> Dict[str, HeaderInfo]:
        """从header_index.md加载头文件索引信息"""
        header_index_path = self.project_root / "docs/project/header_index.md"
        header_map = {}

        if not header_index_path.exists():
            print(f"Warning: header_index.md not found at {header_index_path}")
            return header_map

        try:
            with open(header_index_path, 'r', encoding='utf-8') as f:
                content = f.read()

            # 解析各个模块的头文件信息
            sections = re.split(r'^### \d+\.', content, flags=re.MULTILINE)

            for section in sections[1:]:  # 跳过前言部分
                lines = section.strip().split('\n')
                if not lines:
                    continue

                # 提取模块信息
                module_match = re.search(r'#### \*\*(\w+)\*\*', section)
                if not module_match:
                    continue

                module_name = module_match.group(1).lower()

                # 解析表格内容
                table_lines = []
                in_table = False
                for line in lines:
                    if '| 文件名 |' in line:
                        in_table = True
                        continue
                    elif in_table and line.startswith('| ---'):
                        continue
                    elif in_table and line.startswith('| '):
                        table_lines.append(line)
                    elif in_table and not line.startswith('| '):
                        break

                # 解析表格行
                for line in table_lines:
                    parts = [p.strip() for p in line.split('|')[1:-1]]
                    if len(parts) >= 2:
                        header_name = parts[0].replace('`', '')
                        description = parts[1]

                        # 构建HeaderInfo
                        header_info = HeaderInfo(
                            path=f"include/{module_name}/{header_name}",
                            module=module_name,
                            category=self.infer_category(module_name),
                            dependencies=self.extract_dependencies(description),
                            classes=self.extract_classes(description)
                        )

                        # 添加到映射
                        header_map[header_name] = header_info
                        # 也支持模块前缀的形式
                        header_map[f"{module_name}/{header_name}"] = header_info

        except Exception as e:
            print(f"Error loading header index: {e}")

        return header_map

    def infer_category(self, module: str) -> str:
        """根据模块名推断类别"""
        category_map = {
            'core': '核心组件',
            'sql_parser': 'SQL解析器',
            'execution': '执行引擎',
            'storage': '存储接口',
            'storage_engine': '存储引擎',
            'network': '网络通信',
            'transaction': '事务管理',
            'procedure': '存储过程',
            'trigger': '触发器',
            'exception': '异常处理',
            'utils': '工具类',
            'types': '数据类型',
            'security': '安全模块'
        }
        return category_map.get(module, '其他')

    def extract_dependencies(self, description: str) -> List[str]:
        """从描述中提取依赖关系"""
        deps = []
        # 查找形如 "exception.h", "ast_node.h" 的依赖
        dep_patterns = [
            r'`([^`]+\.h)`',  # `file.h` 格式
            r'(\w+\.h)',     # file.h 格式
        ]

        for pattern in dep_patterns:
            matches = re.findall(pattern, description)
            deps.extend(matches)

        return list(set(deps))

    def extract_classes(self, description: str) -> List[str]:
        """从描述中提取类名"""
        classes = []
        # 查找主要的类名
        class_patterns = [
            r'`([A-Z]\w+)`',  # `ClassName` 格式
            r'\b([A-Z]\w+(?:Exception|Manager|Validator|Executor|Interface))\b'
        ]

        for pattern in class_patterns:
            matches = re.findall(pattern, description)
            classes.extend(matches)

        return list(set(classes))

    def analyze_file_includes(self, file_path: Path) -> List[IncludeIssue]:
        """分析单个文件的include问题"""
        issues = []

        try:
            with open(file_path, 'r', encoding='utf-8') as f:
                lines = f.readlines()

            # 收集所有include语句
            includes = []
            for i, line in enumerate(lines, 1):
                line = line.strip()
                if line.startswith('#include'):
                    includes.append((i, line))

            # 分析include语句
            for line_num, include_line in includes:
                issue = self.analyze_include_line(file_path, line_num, include_line)
                if issue:
                    issues.append(issue)

            # 检查缺失的include
            missing_issues = self.check_missing_includes(file_path, lines, includes)
            issues.extend(missing_issues)

        except Exception as e:
            print(f"Error analyzing {file_path}: {e}")

        return issues

    def analyze_include_line(self, file_path: Path, line_num: int, include_line: str) -> IncludeIssue:
        """分析单个include语句"""
        # 提取include路径
        match = re.match(r'#include\s+["<]([^">]+)[">]', include_line)
        if not match:
            return None

        include_path = match.group(1)

        # 检查是否是相对路径引用
        if include_path.startswith('../') or include_path.startswith('./'):
            # 计算正确的绝对路径
            file_dir = file_path.parent
            resolved_path = (file_dir / include_path).resolve()
            relative_to_include = os.path.relpath(resolved_path, self.include_dir)

            if relative_to_include in self.header_index:
                return IncludeIssue(
                    file_path=str(file_path),
                    line_number=line_num,
                    current_include=include_path,
                    correct_include=relative_to_include,
                    issue_type='wrong_path',
                    severity='warning'
                )

        # 检查include路径是否在索引中但路径不正确
        include_name = Path(include_path).name
        if include_name in self.header_index:
            correct_path = self.header_index[include_name].path
            if correct_path != include_path:
                return IncludeIssue(
                    file_path=str(file_path),
                    line_number=line_num,
                    current_include=include_path,
                    correct_include=correct_path,
                    issue_type='wrong_path',
                    severity='error'
                )

        return None

    def check_missing_includes(self, file_path: Path, lines: List[str], existing_includes: List[Tuple[int, str]]) -> List[IncludeIssue]:
        """检查缺失的include"""
        issues = []

        # 获取文件内容
        content = ''.join(lines)

        # 提取现有include的文件名
        existing_headers = set()
        for _, include_line in existing_includes:
            match = re.match(r'#include\s+["<]([^">]+)[">]', include_line)
            if match:
                header_path = match.group(1)
                header_name = Path(header_path).name
                existing_headers.add(header_name)

        # 检查是否使用了某个模块的类但没有包含相应头文件
        for header_name, header_info in self.header_index.items():
            if header_name in existing_headers:
                continue

            # 检查是否使用了该头文件中的类
            classes_used = False
            for class_name in header_info.classes:
                if re.search(rf'\b{class_name}\b', content):
                    classes_used = True
                    break

            if classes_used:
                # 检查是否应该包含这个头文件
                should_include = self.should_include_header(file_path, header_info)
                if should_include:
                    issues.append(IncludeIssue(
                        file_path=str(file_path),
                        line_number=0,  # 位置未知
                        current_include='',
                        correct_include=header_info.path,
                        issue_type='missing_include',
                        severity='warning'
                    ))

        return issues

    def should_include_header(self, file_path: Path, header_info: HeaderInfo) -> bool:
        """判断是否应该包含某个头文件"""
        # 根据文件位置和模块关系判断
        file_module = self.get_file_module(file_path)

        # 同模块内的文件通常不需要显式include
        if file_module == header_info.module:
            return False

        # 核心模块通常被广泛使用
        if header_info.module in ['core', 'exception', 'utils']:
            return True

        # 根据依赖关系判断
        return header_info.module in ['sql_parser', 'execution', 'storage']

    def get_file_module(self, file_path: Path) -> str:
        """获取文件所属的模块"""
        parts = file_path.parts
        if 'src' in parts:
            src_index = parts.index('src')
            if src_index + 1 < len(parts):
                return parts[src_index + 1]
        return 'unknown'

    def generate_fixes(self, issues: List[IncludeIssue]) -> Dict[str, List[str]]:
        """生成修复建议"""
        fixes = defaultdict(list)

        for issue in issues:
            file_path = issue.file_path

            if issue.issue_type == 'wrong_path':
                fixes[file_path].append(f"将 '{issue.current_include}' 改为 '{issue.correct_include}' (第{issue.line_number}行)")
            elif issue.issue_type == 'missing_include':
                fixes[file_path].append(f"添加缺失的include: '{issue.correct_include}'")

        return dict(fixes)

    def apply_fixes(self, fixes: Dict[str, List[str]], dry_run: bool = True) -> Dict[str, int]:
        """应用修复"""
        stats = {'fixed': 0, 'added': 0, 'errors': 0}

        for file_path, fix_list in fixes.items():
            try:
                with open(file_path, 'r', encoding='utf-8') as f:
                    content = f.read()

                original_content = content

                for fix in fix_list:
                    if '改为' in fix:
                        # 路径修正
                        parts = fix.split("'")
                        if len(parts) >= 4:
                            old_path = parts[1]
                            new_path = parts[3]
                            content = content.replace(f'#include "{old_path}"', f'#include "{new_path}"')
                            content = content.replace(f'#include <{old_path}>', f'#include <{new_path}>')
                            stats['fixed'] += 1
                    elif '添加缺失的include' in fix:
                        # 添加include
                        include_match = re.search(r"'([^']+)'", fix)
                        if include_match:
                            include_path = include_match.group(1)
                            include_line = f'#include "{include_path}"\n'

                            # 找到合适的位置插入
                            lines = content.split('\n')
                            insert_pos = 0
                            for i, line in enumerate(lines):
                                if line.strip().startswith('#include'):
                                    insert_pos = i + 1
                                elif line.strip() and not line.strip().startswith('//'):
                                    break

                            lines.insert(insert_pos, include_line)
                            content = '\n'.join(lines)
                            stats['added'] += 1

                if not dry_run and content != original_content:
                    with open(file_path, 'w', encoding='utf-8') as f:
                        f.write(content)
                    print(f"Applied fixes to {file_path}")

            except Exception as e:
                print(f"Error applying fixes to {file_path}: {e}")
                stats['errors'] += 1

        return stats

    def analyze_project(self) -> Dict:
        """分析整个项目的include问题"""
        print("开始分析项目include路径...")

        all_issues = []
        source_files = self.find_source_files()

        print(f"找到 {len(source_files)} 个源文件待分析")

        for file_path in source_files:
            if self.should_skip_file(file_path):
                continue

            issues = self.analyze_file_includes(file_path)
            all_issues.extend(issues)

        # 按类型和严重程度统计
        stats = {
            'total_files': len(source_files),
            'total_issues': len(all_issues),
            'issues_by_type': defaultdict(int),
            'issues_by_severity': defaultdict(int),
            'issues_by_module': defaultdict(int)
        }

        for issue in all_issues:
            stats['issues_by_type'][issue.issue_type] += 1
            stats['issues_by_severity'][issue.severity] += 1

            module = self.get_file_module(Path(issue.file_path))
            stats['issues_by_module'][module] += 1

        self.analysis_results = {
            'statistics': stats,
            'issues': [vars(issue) for issue in all_issues],
            'fixes': self.generate_fixes(all_issues)
        }

        return self.analysis_results

    def find_source_files(self) -> List[Path]:
        """查找所有源文件"""
        source_files = []
        extensions = ['.cpp', '.cc', '.h', '.hpp']

        for root, dirs, files in os.walk(self.project_root):
            # 跳过不需要的目录
            dirs[:] = [d for d in dirs if not d.startswith('.') and d not in [
                'bazel-bin', 'bazel-out', 'bazel-sqlcc', 'build', '.git'
            ]]

            for file in files:
                if any(file.endswith(ext) for ext in extensions):
                    source_files.append(Path(root) / file)

        return source_files

    def should_skip_file(self, file_path: Path) -> bool:
        """判断是否应该跳过某个文件"""
        # 跳过测试文件、构建文件等
        skip_patterns = [
            'test_', '_test.', 'BUILD', 'WORKSPACE',
            'bazel-', '.pb.', '.grpc.'
        ]

        file_str = str(file_path)
        return any(pattern in file_str for pattern in skip_patterns)

    def generate_report(self, output_file: str = None) -> str:
        """生成分析报告"""
        if not self.analysis_results:
            return "未进行分析，请先调用 analyze_project()"

        stats = self.analysis_results['statistics']

        report = []
        report.append("# SQLCC Include路径分析报告")
        report.append(f"生成时间: {os.popen('date').read().strip()}")
        report.append("")

        report.append("## 📊 统计信息")
        report.append(f"- 总文件数: {stats['total_files']}")
        report.append(f"- 总问题数: {stats['total_issues']}")
        report.append("")

        report.append("## 🔍 问题分类")
        for issue_type, count in stats['issues_by_type'].items():
            type_name = {
                'wrong_path': '路径错误',
                'missing_include': '缺失include',
                'unused_include': '未使用的include'
            }.get(issue_type, issue_type)
            report.append(f"- {type_name}: {count}")
        report.append("")

        report.append("## ⚠️ 严重程度分布")
        for severity, count in stats['issues_by_severity'].items():
            severity_name = {
                'error': '错误',
                'warning': '警告',
                'info': '信息'
            }.get(severity, severity)
            report.append(f"- {severity_name}: {count}")
        report.append("")

        report.append("## 📁 模块分布")
        for module, count in sorted(stats['issues_by_module'].items()):
            report.append(f"- {module}: {count}")
        report.append("")

        # 详细修复建议
        fixes = self.analysis_results.get('fixes', {})
        if fixes:
            report.append("## 🔧 修复建议")
            for file_path, fix_list in fixes.items():
                report.append(f"### {file_path}")
                for fix in fix_list:
                    report.append(f"- {fix}")
                report.append("")

        report_text = '\n'.join(report)

        if output_file:
            with open(output_file, 'w', encoding='utf-8') as f:
                f.write(report_text)
            print(f"报告已保存到: {output_file}")

        return report_text

def main():
    if len(sys.argv) < 2:
        print("Usage: python advanced_include_analyzer.py <project_root> [--fix] [--output <file>]")
        sys.exit(1)

    project_root = sys.argv[1]
    apply_fixes = '--fix' in sys.argv
    output_file = None

    if '--output' in sys.argv:
        idx = sys.argv.index('--output')
        if idx + 1 < len(sys.argv):
            output_file = sys.argv[idx + 1]

    analyzer = AdvancedIncludeAnalyzer(project_root)

    print("正在分析项目include路径...")
    results = analyzer.analyze_project()

    print("\n分析完成！")
    print(f"发现问题: {results['statistics']['total_issues']}")

    if apply_fixes:
        print("正在应用修复...")
        stats = analyzer.apply_fixes(results.get('fixes', {}), dry_run=False)
        print(f"修复完成: {stats}")

    # 生成报告
    report = analyzer.generate_report(output_file)
    print("\n" + "="*50)
    print(report)

if __name__ == "__main__":
    main()
