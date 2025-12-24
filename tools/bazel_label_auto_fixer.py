#!/usr/bin/env python3
"""
Bazel Label Auto Fixer - 自动检测和修复Bazel标签引用错误

功能特性:
- 自动扫描所有BUILD.bazel文件
- 检测错误的Bazel标签格式
- 自动修复标签引用错误
- 生成详细的修复报告
- 支持dry-run模式预览修复
- 支持批量处理和单文件处理

作者: SQLCC AI Assistant
版本: 1.0.0
日期: 2025-12-24
"""

import os
import re
import argparse
import logging
from pathlib import Path
from typing import Dict, List, Tuple, Set
from dataclasses import dataclass, field

# 配置日志
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)

@dataclass
class FixResult:
    """修复结果数据类"""
    file_path: str
    original_line: str
    fixed_line: str
    line_number: int
    error_type: str
    description: str

@dataclass
class FixReport:
    """修复报告数据类"""
    total_files: int = 0
    processed_files: int = 0
    total_fixes: int = 0
    fixes_by_type: Dict[str, int] = field(default_factory=dict)
    fixes_by_file: Dict[str, List[FixResult]] = field(default_factory=dict)
    errors: List[str] = field(default_factory=list)

class BazelLabelAutoFixer:
    """Bazel标签自动修复器"""

    def __init__(self, root_path: str = "."):
        self.root_path = Path(root_path).resolve()
        self.report = FixReport()

        # Bazel标签格式正则表达式
        self.patterns = {
            'invalid_external_ref': re.compile(r'"//([^:]+):([^/]+/[^"]*)"'),
            'invalid_package_path': re.compile(r'"//([^:]+/[^:]+):([^"]*)"'),
            'missing_quotes': re.compile(r'\b//([^:\s]+):([^\s,]+)'),
            'relative_path_error': re.compile(r'"([^"]*)\.\./([^"]*)"'),
        }

        # 错误类型映射
        self.error_types = {
            'invalid_external_ref': '无效的外部引用格式',
            'invalid_package_path': '无效的包路径格式',
            'missing_quotes': '缺少引号',
            'relative_path_error': '相对路径错误',
        }

    def find_build_files(self) -> List[Path]:
        """查找所有BUILD.bazel文件"""
        build_files = []
        for root, dirs, files in os.walk(self.root_path):
            # 跳过一些不需要的目录
            dirs[:] = [d for d in dirs if not d.startswith('.') and d not in ['bazel-bin', 'bazel-out', 'bazel-sqlcc']]

            for file in files:
                if file in ['BUILD.bazel', 'BUILD']:
                    build_files.append(Path(root) / file)

        return sorted(build_files)

    def analyze_line(self, line: str, line_num: int, file_path: str) -> List[FixResult]:
        """分析单行代码，返回修复结果"""
        fixes = []

        # 检测各种错误模式
        for pattern_name, pattern in self.patterns.items():
            matches = pattern.findall(line)
            if matches:
                for match in matches:
                    if pattern_name == 'invalid_external_ref':
                        # 修复 //package:subpackage/file.cpp 格式
                        package_part, file_part = match
                        if '/' in file_part:
                            # 这是错误的格式，需要修复
                            fixed_line = line.replace(f'//{package_part}:{file_part}', f'//{package_part}/{file_part.split("/")[0]}:{file_part}')
                            fixes.append(FixResult(
                                file_path=str(file_path),
                                original_line=line.strip(),
                                fixed_line=fixed_line,
                                line_number=line_num,
                                error_type=pattern_name,
                                description=f'修复外部引用格式: //{package_part}:{file_part} -> //{package_part}/{file_part.split("/")[0]}:{file_part}'
                            ))

                    elif pattern_name == 'invalid_package_path':
                        # 修复包路径错误
                        full_path, target = match
                        if ':' in full_path and '/' in full_path:
                            parts = full_path.split(':')
                            if len(parts) == 2:
                                package_path = parts[0]
                                if '/' in package_path:
                                    # 可能是错误的格式
                                    fixed_package = package_path.replace('/', '_')
                                    fixed_line = line.replace(f'//{full_path}:{target}', f'//{fixed_package}:{target}')
                                    fixes.append(FixResult(
                                        file_path=str(file_path),
                                        original_line=line.strip(),
                                        fixed_line=fixed_line,
                                        line_number=line_num,
                                        error_type=pattern_name,
                                        description=f'修复包路径格式: //{full_path}:{target} -> //{fixed_package}:{target}'
                                    ))

                    elif pattern_name == 'missing_quotes':
                        # 添加缺失的引号
                        package_part, target_part = match
                        fixed_line = line.replace(f'//{package_part}:{target_part}', f'"//{package_part}:{target_part}"')
                        fixes.append(FixResult(
                            file_path=str(file_path),
                            original_line=line.strip(),
                            fixed_line=fixed_line,
                            line_number=line_num,
                            error_type=pattern_name,
                            description=f'添加缺失的引号: //{package_part}:{target_part} -> "//{package_part}:{target_part}"'
                        ))

                    elif pattern_name == 'relative_path_error':
                        # 修复相对路径
                        prefix, suffix = match
                        # 简化相对路径（这里可以根据具体需求调整）
                        if '../' in prefix:
                            simplified = prefix.replace('../', '')
                            fixed_line = line.replace(f'"{prefix}"', f'"{simplified}"')
                            fixes.append(FixResult(
                                file_path=str(file_path),
                                original_line=line.strip(),
                                fixed_line=fixed_line,
                                line_number=line_num,
                                error_type=pattern_name,
                                description=f'简化相对路径: {prefix} -> {simplified}'
                            ))

        return fixes

    def fix_file(self, file_path: Path, dry_run: bool = True) -> List[FixResult]:
        """修复单个文件"""
        fixes = []

        try:
            with open(file_path, 'r', encoding='utf-8') as f:
                lines = f.readlines()

            modified_lines = []
            for line_num, line in enumerate(lines, 1):
                line_fixes = self.analyze_line(line, line_num, file_path)
                if line_fixes:
                    fixes.extend(line_fixes)
                    # 应用第一个修复（简化处理，实际可以处理多个）
                    if line_fixes and not dry_run:
                        modified_lines.append(line_fixes[0].fixed_line + '\n')
                    else:
                        modified_lines.append(line)
                else:
                    modified_lines.append(line)

            # 写入修复后的文件
            if not dry_run and fixes:
                with open(file_path, 'w', encoding='utf-8') as f:
                    f.writelines(modified_lines)
                logger.info(f"已修复文件: {file_path} ({len(fixes)} 个修复)")

        except Exception as e:
            error_msg = f"处理文件 {file_path} 时出错: {str(e)}"
            logger.error(error_msg)
            self.report.errors.append(error_msg)

        return fixes

    def fix_all_files(self, dry_run: bool = True, file_filter: str = None) -> FixReport:
        """修复所有BUILD文件"""
        logger.info("开始扫描BUILD文件...")

        build_files = self.find_build_files()
        self.report.total_files = len(build_files)

        logger.info(f"发现 {len(build_files)} 个BUILD文件")

        # 应用文件过滤器
        if file_filter:
            build_files = [f for f in build_files if file_filter in str(f)]
            logger.info(f"应用过滤器后剩余 {len(build_files)} 个文件")

        for file_path in build_files:
            logger.debug(f"处理文件: {file_path}")
            fixes = self.fix_file(file_path, dry_run)
            self.report.processed_files += 1

            if fixes:
                self.report.fixes_by_file[str(file_path)] = fixes
                self.report.total_fixes += len(fixes)

                # 统计错误类型
                for fix in fixes:
                    self.report.fixes_by_type[fix.error_type] = self.report.fixes_by_type.get(fix.error_type, 0) + 1

                logger.info(f"文件 {file_path} 发现 {len(fixes)} 个需要修复的问题")

        return self.report

    def generate_report(self, output_file: str = None) -> str:
        """生成修复报告"""
        report_lines = []
        report_lines.append("# Bazel标签自动修复报告")
        report_lines.append(f"生成时间: {Path(output_file).stem.split('_')[-1] if output_file else '实时'}")
        report_lines.append("")

        report_lines.append("## 统计摘要")
        report_lines.append(f"- 处理文件总数: {self.report.total_files}")
        report_lines.append(f"- 实际处理文件: {self.report.processed_files}")
        report_lines.append(f"- 发现问题总数: {self.report.total_fixes}")
        report_lines.append("")

        if self.report.fixes_by_type:
            report_lines.append("## 问题类型统计")
            for error_type, count in sorted(self.report.fixes_by_type.items()):
                report_lines.append(f"- {self.error_types.get(error_type, error_type)}: {count} 个")
            report_lines.append("")

        if self.report.fixes_by_file:
            report_lines.append("## 详细修复记录")
            for file_path, fixes in sorted(self.report.fixes_by_file.items()):
                report_lines.append(f"### {file_path} ({len(fixes)} 个修复)")
                for i, fix in enumerate(fixes, 1):
                    report_lines.append(f"#### 修复 {i}")
                    report_lines.append(f"- **行号**: {fix.line_number}")
                    report_lines.append(f"- **错误类型**: {self.error_types.get(fix.error_type, fix.error_type)}")
                    report_lines.append(f"- **原始代码**: `{fix.original_line}`")
                    report_lines.append(f"- **修复后**: `{fix.fixed_line.strip()}`")
                    report_lines.append(f"- **描述**: {fix.description}")
                    report_lines.append("")
                report_lines.append("")

        if self.report.errors:
            report_lines.append("## 处理错误")
            for error in self.report.errors:
                report_lines.append(f"- {error}")
            report_lines.append("")

        report_content = "\n".join(report_lines)

        if output_file:
            with open(output_file, 'w', encoding='utf-8') as f:
                f.write(report_content)
            logger.info(f"报告已保存到: {output_file}")

        return report_content

    def print_summary(self):
        """打印摘要信息"""
        print("\n" + "="*60)
        print("Bazel标签自动修复器 - 执行摘要")
        print("="*60)
        print(f"处理文件总数: {self.report.total_files}")
        print(f"实际处理文件: {self.report.processed_files}")
        print(f"发现问题总数: {self.report.total_fixes}")

        if self.report.fixes_by_type:
            print("\n问题类型分布:")
            for error_type, count in sorted(self.report.fixes_by_type.items()):
                print(f"  {self.error_types.get(error_type, error_type)}: {count} 个")

        if self.report.errors:
            print(f"\n处理错误: {len(self.report.errors)} 个")
            for error in self.report.errors[:5]:  # 只显示前5个错误
                print(f"  - {error}")
            if len(self.report.errors) > 5:
                print(f"  ... 还有 {len(self.report.errors) - 5} 个错误")

        print("="*60)

def main():
    """主函数"""
    parser = argparse.ArgumentParser(
        description="Bazel标签自动修复工具",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
使用示例:
  # 预览模式（推荐先运行）
  python bazel_label_auto_fixer.py --dry-run

  # 实际修复
  python bazel_label_auto_fixer.py --fix

  # 修复特定文件
  python bazel_label_auto_fixer.py --fix --filter "src/BUILD.bazel"

  # 生成详细报告
  python bazel_label_auto_fixer.py --dry-run --report bazel_fix_report.md
        """
    )

    parser.add_argument(
        '--root', '-r',
        default='.',
        help='项目根目录路径 (默认: 当前目录)'
    )

    parser.add_argument(
        '--dry-run', '-d',
        action='store_true',
        help='预览模式，只显示需要修复的问题，不实际修改文件'
    )

    parser.add_argument(
        '--fix', '-f',
        action='store_true',
        help='修复模式，实际修改文件'
    )

    parser.add_argument(
        '--filter',
        help='文件过滤器，只处理包含指定字符串的文件路径'
    )

    parser.add_argument(
        '--report', '-o',
        help='生成详细报告文件路径'
    )

    parser.add_argument(
        '--verbose', '-v',
        action='store_true',
        help='详细输出模式'
    )

    args = parser.parse_args()

    # 参数验证
    if not args.dry_run and not args.fix:
        print("错误: 必须指定 --dry-run 或 --fix 模式")
        print("建议先运行 --dry-run 预览修复内容")
        return 1

    if args.dry_run and args.fix:
        print("错误: 不能同时指定 --dry-run 和 --fix")
        return 1

    # 设置日志级别
    if args.verbose:
        logging.getLogger().setLevel(logging.DEBUG)

    # 创建修复器实例
    fixer = BazelLabelAutoFixer(args.root)

    # 执行修复
    dry_run = not args.fix
    mode_desc = "预览模式" if dry_run else "修复模式"

    logger.info(f"启动Bazel标签自动修复器 - {mode_desc}")
    logger.info(f"项目根目录: {args.root}")

    if args.filter:
        logger.info(f"文件过滤器: {args.filter}")

    # 执行修复
    report = fixer.fix_all_files(dry_run=dry_run, file_filter=args.filter)

    # 生成报告
    if args.report:
        fixer.generate_report(args.report)

    # 打印摘要
    fixer.print_summary()

    # 输出建议
    if dry_run and report.total_fixes > 0:
        print("\n建议:")
        print("1. 审查上述修复内容是否正确")
        print("2. 如确认无误，运行以下命令进行实际修复:")
        print(f"   python {__file__} --fix" + (f" --filter {args.filter}" if args.filter else ""))

    return 0

if __name__ == "__main__":
    exit(main())
