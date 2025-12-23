#!/usr/bin/env python3
"""
SQLCC Bazel Include路径修复工具
基于检测结果，系统性地修复头文件的include路径

功能特性:
- 智能识别头文件的新位置
- 批量修复所有引用这些头文件的include语句
- 支持相对路径和绝对路径的转换
- 生成修复报告

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

class IncludePathMapper:
    """Include路径映射器"""

    def __init__(self, project_root: str):
        self.project_root = Path(project_root).resolve()
        self.header_mapping: Dict[str, str] = {}
        self.reverse_mapping: Dict[str, List[str]] = {}

    def build_header_mapping(self) -> Dict[str, str]:
        """构建头文件的完整映射"""
        print("🔍 构建头文件映射...")

        # 扫描所有头文件
        header_files = []
        for root, dirs, files in os.walk(self.project_root):
            # 跳过构建目录
            dirs[:] = [d for d in dirs if not d.startswith('.') and d not in ['bazel-bin', 'bazel-out', 'bazel-testlogs']]

            for file in files:
                if file.endswith(('.h', '.hpp', '.hxx')):
                    header_files.append(Path(root) / file)

        print(f"📁 发现 {len(header_files)} 个头文件")

        # 构建映射：文件名 -> 完整路径
        for header_file in header_files:
            file_name = header_file.name
            relative_path = header_file.relative_to(self.project_root)

            if file_name not in self.header_mapping:
                self.header_mapping[file_name] = str(relative_path)
            else:
                # 如果有多个同名文件，取第一个（通常是最新的）
                pass

        return self.header_mapping

    def find_correct_include_path(self, old_include: str, source_file: Path) -> Optional[str]:
        """找到正确的include路径"""
        # 解析旧的include路径
        if old_include.startswith('"') and old_include.endswith('"'):
            old_include = old_include[1:-1]
        elif old_include.startswith('<') and old_include.endswith('>'):
            old_include = old_include[1:-1]

        # 如果是相对路径，尝试解析
        if old_include.startswith('../') or old_include.startswith('./'):
            # 计算相对于源文件的路径
            try:
                resolved = (source_file.parent / old_include).resolve()
                if resolved.exists() and resolved.relative_to(self.project_root):
                    return str(resolved.relative_to(self.project_root))
            except:
                pass

        # 如果是文件名，查找映射
        file_name = Path(old_include).name
        if file_name in self.header_mapping:
            correct_path = self.header_mapping[file_name]
            # 计算相对路径
            try:
                source_dir = source_file.parent
                target_file = self.project_root / correct_path
                relative_path = os.path.relpath(str(target_file), str(source_dir))
                return relative_path
            except:
                return correct_path

        return None

class BazelIncludeFixer:
    """Bazel Include路径修复器"""

    def __init__(self, project_root: str, detection_results: Optional[str] = None):
        self.project_root = Path(project_root).resolve()
        self.detection_results = Path(detection_results) if detection_results else None
        self.path_mapper = IncludePathMapper(project_root)
        self.fixed_files: List[str] = []
        self.fix_report: List[Dict[str, Any]] = []

    def fix_include_paths(self, detection_json: Optional[str] = None) -> Dict[str, Any]:
        """修复include路径"""
        if detection_json:
            self.detection_results = Path(detection_json)

        # 构建头文件映射
        header_mapping = self.path_mapper.build_header_mapping()

        if not self.detection_results or not self.detection_results.exists():
            print("❌ 未找到检测结果文件")
            return {"error": "Detection results not found"}

        # 读取检测结果
        with open(self.detection_results, 'r', encoding='utf-8') as f:
            results = json.load(f)

        # 获取所有MISSING_FILE问题
        missing_file_issues = [
            issue for issue in results['issues']
            if issue['issue_type'] == 'MISSING_FILE'
        ]

        print(f"📁 发现 {len(missing_file_issues)} 个缺失文件问题")

        # 按文件分组处理
        files_to_fix = {}
        for issue in missing_file_issues:
            file_path = issue['metadata'].get('file_ref', '')
            if file_path:
                build_file = issue['file_path']
                if build_file not in files_to_fix:
                    files_to_fix[build_file] = []
                files_to_fix[build_file].append(file_path)

        # 修复每个文件
        total_fixed = 0
        for build_file, missing_files in files_to_fix.items():
            fixed_count = self._fix_build_file(build_file, missing_files)
            total_fixed += fixed_count

        print(f"✅ 修复完成，共修复 {total_fixed} 个include路径")

        return {
            "total_missing_files": len(missing_file_issues),
            "files_processed": len(files_to_fix),
            "total_fixed": total_fixed,
            "fix_report": self.fix_report
        }

    def _fix_build_file(self, build_file_path: str, missing_files: List[str]) -> int:
        """修复单个BUILD文件"""
        build_file = Path(build_file_path)

        if not build_file.exists():
            print(f"⚠️  BUILD文件不存在: {build_file_path}")
            return 0

        try:
            with open(build_file, 'r', encoding='utf-8') as f:
                content = f.read()

            original_content = content
            fixed_count = 0

            # 处理每个缺失的文件
            for missing_file in missing_files:
                # 在BUILD文件中查找对应的标签引用
                # 例如：//include/utils:logger.h -> 需要修复为正确的路径
                if ':' in missing_file:
                    # 这是Bazel标签，需要找到在BUILD文件中的引用
                    label_pattern = re.escape(missing_file)
                    matches = re.finditer(label_pattern, content)

                    for match in matches:
                        # 找到正确的include路径
                        correct_path = self._find_correct_path_for_label(missing_file, build_file)
                        if correct_path:
                            # 替换标签
                            old_label = match.group(0)
                            new_label = f'"{correct_path}"'

                            content = content.replace(old_label, new_label, 1)
                            fixed_count += 1

                            self.fix_report.append({
                                "file": build_file_path,
                                "old_include": old_label,
                                "new_include": new_label,
                                "reason": "Header file location changed during refactoring"
                            })

            # 保存修复后的文件
            if content != original_content:
                with open(build_file, 'w', encoding='utf-8') as f:
                    f.write(content)

                self.fixed_files.append(build_file_path)
                print(f"🔧 修复了 {build_file_path} 中的 {fixed_count} 个include路径")

            return fixed_count

        except Exception as e:
            print(f"❌ 处理文件失败 {build_file_path}: {e}")
            return 0

    def _find_correct_path_for_label(self, label: str, build_file: Path) -> Optional[str]:
        """为Bazel标签找到正确的路径"""
        # 解析Bazel标签
        # 例如：//include/utils:logger.h
        if label.startswith('//'):
            parts = label[2:].split(':')
            if len(parts) == 2:
                package_path = parts[0]
                target_name = parts[1]

                # 查找正确的头文件位置
                possible_paths = [
                    f"{package_path}/{target_name}",
                    f"{package_path}/include/{target_name}",
                    f"include/{package_path}/{target_name}",
                ]

                for possible_path in possible_paths:
                    full_path = self.project_root / possible_path
                    if full_path.exists():
                        # 计算相对于BUILD文件的相对路径
                        try:
                            relative_path = os.path.relpath(str(full_path), str(build_file.parent))
                            return relative_path
                        except:
                            return str(full_path.relative_to(self.project_root))

        return None

    def get_fix_report(self) -> str:
        """生成修复报告"""
        report_lines = []
        report_lines.append("# SQLCC Include路径修复报告")
        report_lines.append(f"修复时间: {self._get_timestamp()}")
        report_lines.append(f"修复文件数量: {len(self.fixed_files)}")
        report_lines.append(f"修复条目数量: {len(self.fix_report)}")
        report_lines.append("")

        if self.fix_report:
            report_lines.append("## 修复详情")
            for i, fix in enumerate(self.fix_report, 1):
                report_lines.append(f"### 修复 {i}")
                report_lines.append(f"- **文件**: {fix['file']}")
                report_lines.append(f"- **旧路径**: {fix['old_include']}")
                report_lines.append(f"- **新路径**: {fix['new_include']}")
                report_lines.append(f"- **原因**: {fix['reason']}")
                report_lines.append("")
        else:
            report_lines.append("## 无修复条目")

        report_lines.append("")
        report_lines.append("---")
        report_lines.append("*此报告由bazel_include_fixer.py自动生成*")

        return "\n".join(report_lines)

    def _get_timestamp(self) -> str:
        """获取当前时间戳"""
        from datetime import datetime
        return datetime.now().strftime("%Y-%m-%d %H:%M:%S")

def main():
    """主函数"""
    import argparse

    parser = argparse.ArgumentParser(description="SQLCC Bazel Include路径修复器")
    parser.add_argument("project_root", help="项目根目录")
    parser.add_argument("--detection-results", help="检测结果JSON文件")
    parser.add_argument("--output-report", "-o", help="输出修复报告文件")

    args = parser.parse_args()

    if not os.path.isdir(args.project_root):
        print(f"❌ 错误: 目录不存在: {args.project_root}")
        sys.exit(1)

    fixer = BazelIncludeFixer(args.project_root, args.detection_results)

    # 执行修复
    result = fixer.fix_include_paths()

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

    print("\n✅ Include路径修复完成!")
    print(f"📊 统计信息:")
    print(f"  - 处理的缺失文件: {result['total_missing_files']}")
    print(f"  - 处理的BUILD文件: {result['files_processed']}")
    print(f"  - 修复的路径: {result['total_fixed']}")

if __name__ == "__main__":
    sys.exit(main())
