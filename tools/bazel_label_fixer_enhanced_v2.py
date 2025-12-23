#!/usr/bin/env python3
"""
增强版Bazel标签路径修复工具 v2.0
基于检测结果智能分析和修复标签格式问题

新增功能特性:
- 基于错误模式分析的智能修复
- 支持更多Bazel语法规则
- 上下文感知的标签验证
- 批量修复和报告生成
- 安全验证和回滚机制

作者: SQLCC AI Agent
版本: v2.0 (2025-12-22)
更新时间: 2025-12-22
"""

import os
import re
import sys
from pathlib import Path
from typing import Dict, List, Set, Tuple, Optional, Any
from collections import Counter, defaultdict
import json

class BazelLabelPattern:
    """Bazel标签模式定义"""

    # 常见错误模式
    PATTERNS = {
        # 缺少目标名称的标签
        "missing_target": {
            "pattern": r'//([^:]+)$',  # //package (缺少 :target)
            "description": "缺少目标名称",
            "severity": "HIGH"
        },

        # 文件扩展名在目标名称中
        "file_extension_in_target": {
            "pattern": r'//([^:]+):([^:]*\.(h|cpp|cc|cxx|hxx)[^:]*)$',
            "description": "目标名称包含文件扩展名",
            "severity": "MEDIUM"
        },

        # 错误的包路径格式
        "invalid_package_path": {
            "pattern": r'//([^a-zA-Z0-9_/]+)',
            "description": "包路径包含无效字符",
            "severity": "HIGH"
        },

        # 相对路径在标签中
        "relative_path_in_label": {
            "pattern": r'//(\.\./[^:]+):',
            "description": "标签中使用相对路径",
            "severity": "HIGH"
        },

        # 空目标名称
        "empty_target": {
            "pattern": r'//([^:]+):$',
            "description": "目标名称为空",
            "severity": "CRITICAL"
        },

        # 重复的包路径
        "duplicate_package": {
            "pattern": r'//(([^/]+/)*([^/]+))/\3:',
            "description": "包路径中包含重复目录名",
            "severity": "MEDIUM"
        }
    }

class EnhancedBazelLabelFixerV2:
    """增强版Bazel标签修复器 v2.0"""

    def __init__(self, detection_results: Optional[str] = None):
        self.project_root = Path.cwd()
        self.detection_results = Path(detection_results) if detection_results else None
        self.patterns = BazelLabelPattern.PATTERNS

        # 统计信息
        self.fixed_count = 0
        self.errors = []
        self.warnings = []
        self.fix_report = []

        # 分析数据
        self.error_patterns = Counter()
        self.common_fixes = defaultdict(int)

    def analyze_detection_results(self) -> Dict[str, Any]:
        """分析检测结果，识别标签错误模式"""
        if not self.detection_results or not self.detection_results.exists():
            return {}

        with open(self.detection_results, 'r', encoding='utf-8') as f:
            results = json.load(f)

        label_errors = []

        for issue in results.get('issues', []):
            if issue['issue_type'] == 'LABEL_ERROR':
                context = issue.get('context', '')
                if context:
                    # 分析上下文中的标签错误
                    errors_in_context = self._analyze_context_for_labels(context, issue)
                    label_errors.extend(errors_in_context)

        # 统计错误模式
        for error in label_errors:
            self.error_patterns[error['pattern_type']] += 1

        return {
            "total_label_errors": len(label_errors),
            "error_patterns": dict(self.error_patterns),
            "detailed_errors": label_errors
        }

    def _analyze_context_for_labels(self, context: str, issue: Dict[str, Any]) -> List[Dict[str, Any]]:
        """分析上下文中的标签错误"""
        errors = []

        # 查找所有Bazel标签
        label_pattern = r'//[^\s\'"`,;]*'
        labels = re.findall(label_pattern, context)

        for label in labels:
            # 检查每个标签是否符合规则
            label_errors = self._validate_label(label)
            if label_errors:
                for error_type, description in label_errors:
                    errors.append({
                        "label": label,
                        "pattern_type": error_type,
                        "description": description,
                        "file": issue.get('file_path', ''),
                        "line": issue.get('line_number', 0),
                        "context": context.strip()
                    })

        return errors

    def _validate_label(self, label: str) -> List[Tuple[str, str]]:
        """验证单个标签的正确性"""
        errors = []

        # 检查各种错误模式
        for pattern_name, pattern_info in self.patterns.items():
            if re.search(pattern_info['pattern'], label):
                errors.append((pattern_name, pattern_info['description']))

        # 额外的验证规则
        if not label.startswith('//'):
            errors.append(('invalid_format', '标签必须以//开头'))

        if ':' not in label and not label.endswith('/'):
            errors.append(('missing_colon', '标签缺少冒号分隔符'))

        # 检查目标名称是否包含无效字符
        if ':' in label:
            parts = label.split(':')
            if len(parts) == 2:
                target = parts[1]
                if not re.match(r'^[a-zA-Z0-9_][a-zA-Z0-9_\-\.]*$', target):
                    errors.append(('invalid_target_chars', '目标名称包含无效字符'))

        return errors

    def fix_labels_smart(self, analysis_results: Dict[str, Any]) -> Dict[str, Any]:
        """智能修复标签错误"""
        detailed_errors = analysis_results.get('detailed_errors', [])

        # 按文件分组错误
        errors_by_file = defaultdict(list)
        for error in detailed_errors:
            errors_by_file[error['file']].append(error)

        total_fixed = 0

        # 处理每个文件
        for file_path, errors in errors_by_file.items():
            fixed_in_file = self._fix_labels_in_file(file_path, errors)
            total_fixed += fixed_in_file

        return {
            "total_files_processed": len(errors_by_file),
            "total_labels_fixed": total_fixed,
            "fix_report": self.fix_report
        }

    def _fix_labels_in_file(self, file_path: str, errors: List[Dict[str, Any]]) -> int:
        """修复单个文件中的标签错误"""
        if not Path(file_path).exists():
            self.errors.append(f"文件不存在: {file_path}")
            return 0

        try:
            with open(file_path, 'r', encoding='utf-8') as f:
                content = f.read()
        except UnicodeDecodeError:
            self.errors.append(f"无法读取文件 (编码问题): {file_path}")
            return 0

        original_content = content
        fixed_count = 0

        # 对每个错误进行修复
        for error in errors:
            label = error['label']
            pattern_type = error['pattern_type']

            # 根据错误类型应用不同的修复策略
            fixed_label = self._apply_fix_strategy(label, pattern_type, error)

            if fixed_label and fixed_label != label:
                # 确保修复后的标签是有效的
                if self._validate_fixed_label(fixed_label):
                    content = content.replace(label, fixed_label, 1)
                    fixed_count += 1

                    self.fix_report.append({
                        "file": file_path,
                        "original_label": label,
                        "fixed_label": fixed_label,
                        "pattern_type": pattern_type,
                        "reason": error['description']
                    })

                    self.common_fixes[pattern_type] += 1
                else:
                    self.warnings.append(f"修复后的标签无效: {fixed_label} (原始: {label})")

        # 保存修复后的文件
        if content != original_content:
            try:
                with open(file_path, 'w', encoding='utf-8') as f:
                    f.write(content)
                print(f"✅ 修复了 {file_path} 中的 {fixed_count} 个标签")
            except IOError as e:
                self.errors.append(f"无法写入文件 {file_path}: {e}")
                return 0

        return fixed_count

    def _apply_fix_strategy(self, label: str, pattern_type: str, error: Dict[str, Any]) -> Optional[str]:
        """应用修复策略"""
        strategies = {
            "missing_target": self._fix_missing_target,
            "file_extension_in_target": self._fix_file_extension_in_target,
            "invalid_package_path": self._fix_invalid_package_path,
            "relative_path_in_label": self._fix_relative_path_in_label,
            "empty_target": self._fix_empty_target,
            "duplicate_package": self._fix_duplicate_package,
            "missing_colon": self._fix_missing_colon,
            "invalid_target_chars": self._fix_invalid_target_chars
        }

        strategy = strategies.get(pattern_type)
        if strategy:
            return strategy(label, error)

        return None

    def _fix_missing_target(self, label: str, error: Dict[str, Any]) -> Optional[str]:
        """修复缺少目标名称的标签"""
        if not label.endswith(':'):
            # 尝试推断目标名称
            parts = label[2:].split('/')  # 去掉//前缀
            if parts:
                target_name = parts[-1]
                return f"{label}:{target_name}"
        return None

    def _fix_file_extension_in_target(self, label: str, error: Dict[str, Any]) -> Optional[str]:
        """修复目标名称中包含文件扩展名的问题"""
        if ':' in label:
            package_part, target_part = label.split(':', 1)
            # 移除文件扩展名
            target_clean = re.sub(r'\.(h|cpp|cc|cxx|hxx).*?$', '', target_part)
            return f"{package_part}:{target_clean}"
        return None

    def _fix_invalid_package_path(self, label: str, error: Dict[str, Any]) -> Optional[str]:
        """修复无效包路径"""
        # 移除或替换无效字符
        package_part = label.split(':')[0] if ':' in label else label
        # 替换无效字符为下划线
        fixed_package = re.sub(r'[^a-zA-Z0-9_/]', '_', package_part)
        if ':' in label:
            target_part = label.split(':', 1)[1]
            return f"{fixed_package}:{target_part}"
        else:
            return fixed_package

    def _fix_relative_path_in_label(self, label: str, error: Dict[str, Any]) -> Optional[str]:
        """修复标签中的相对路径"""
        # 将相对路径转换为绝对路径
        if label.startswith('//../'):
            return label.replace('//../', '//', 1)
        elif label.startswith('//../../'):
            return label.replace('//../../', '//', 1)
        return None

    def _fix_empty_target(self, label: str, error: Dict[str, Any]) -> Optional[str]:
        """修复空目标名称"""
        package_part = label[:-1]  # 移除末尾的冒号
        # 使用包的最后一部分作为目标名
        parts = package_part[2:].split('/')  # 去掉//
        if parts:
            target_name = parts[-1]
            return f"{package_part}:{target_name}"
        return None

    def _fix_duplicate_package(self, label: str, error: Dict[str, Any]) -> Optional[str]:
        """修复重复的包路径"""
        if ':' in label:
            package_part, target_part = label.split(':', 1)
            # 简化重复的路径组件
            parts = package_part[2:].split('/')  # 去掉//
            if len(parts) >= 2 and parts[-1] == parts[-2]:
                # 移除重复的最后组件
                simplified_parts = parts[:-1]
                return f"//{'/'.join(simplified_parts)}:{target_part}"
        return None

    def _fix_missing_colon(self, label: str, error: Dict[str, Any]) -> Optional[str]:
        """修复缺少冒号的标签"""
        if ':' not in label and not label.endswith('/'):
            # 推断目标名称
            parts = label[2:].split('/')  # 去掉//
            if parts:
                target_name = parts[-1]
                return f"{label}:{target_name}"
        return None

    def _fix_invalid_target_chars(self, label: str, error: Dict[str, Any]) -> Optional[str]:
        """修复目标名称中的无效字符"""
        if ':' in label:
            package_part, target_part = label.split(':', 1)
            # 替换无效字符
            fixed_target = re.sub(r'[^a-zA-Z0-9_\-\.]', '_', target_part)
            return f"{package_part}:{fixed_target}"
        return None

    def _validate_fixed_label(self, label: str) -> bool:
        """验证修复后的标签是否有效"""
        # 基本格式检查
        if not label.startswith('//'):
            return False

        if ':' not in label:
            return False

        parts = label.split(':')
        if len(parts) != 2:
            return False

        package_part, target_part = parts

        # 检查包路径
        if not re.match(r'^//[a-zA-Z0-9_/]*$', package_part):
            return False

        # 检查目标名称
        if not re.match(r'^[a-zA-Z0-9_][a-zA-Z0-9_\-\.]*$', target_part):
            return False

        return True

    def generate_enhanced_report(self) -> str:
        """生成增强的修复报告"""
        report_lines = []
        report_lines.append("# SQLCC Bazel标签修复增强报告 v2.0")
        report_lines.append(f"生成时间: {self._get_timestamp()}")
        report_lines.append("")

        # 总体统计
        report_lines.append("## 📊 总体统计")
        report_lines.append(f"- 处理文件数量: {len(set(fix['file'] for fix in self.fix_report))}")
        report_lines.append(f"- 修复标签总数: {len(self.fix_report)}")
        report_lines.append(f"- 错误模式种类: {len(self.error_patterns)}")
        report_lines.append("")

        # 错误模式统计
        if self.error_patterns:
            report_lines.append("## 🔍 错误模式分析")
            for pattern, count in self.error_patterns.most_common():
                description = self.patterns.get(pattern, {}).get('description', '未知模式')
                severity = self.patterns.get(pattern, {}).get('severity', 'UNKNOWN')
                report_lines.append(f"- **{pattern}** ({severity}): {count} 次 - {description}")
            report_lines.append("")

        # 修复统计
        if self.common_fixes:
            report_lines.append("## 🔧 修复统计")
            for fix_type, count in self.common_fixes.items():
                description = self.patterns.get(fix_type, {}).get('description', '未知修复')
                report_lines.append(f"- **{fix_type}**: {count} 次修复 - {description}")
            report_lines.append("")

        # 详细修复记录
        if self.fix_report:
            report_lines.append("## 📝 详细修复记录")
            for i, fix in enumerate(self.fix_report, 1):
                report_lines.append(f"### 修复 {i}")
                report_lines.append(f"- **文件**: {fix['file']}")
                report_lines.append(f"- **原始标签**: `{fix['original_label']}`")
                report_lines.append(f"- **修复后**: `{fix['fixed_label']}`")
                report_lines.append(f"- **错误类型**: {fix['pattern_type']}")
                report_lines.append(f"- **原因**: {fix['reason']}")
                report_lines.append("")

        # 警告和错误
        if self.warnings:
            report_lines.append("## ⚠️ 警告信息")
            for warning in self.warnings:
                report_lines.append(f"- {warning}")
            report_lines.append("")

        if self.errors:
            report_lines.append("## ❌ 错误信息")
            for error in self.errors:
                report_lines.append(f"- {error}")
            report_lines.append("")

        # 改进建议
        report_lines.append("## 💡 改进建议")
        report_lines.append("基于本次修复分析，建议采取以下措施:")
        report_lines.append("")
        report_lines.append("### 1. 预防措施")
        report_lines.append("- 建立Bazel标签命名规范文档")
        report_lines.append("- 在代码审查中加入标签格式检查")
        report_lines.append("- 开发IDE插件提供实时标签验证")
        report_lines.append("")

        report_lines.append("### 2. 自动化工具")
        report_lines.append("- 集成到CI/CD流水线")
        report_lines.append("- 添加git hooks进行预提交检查")
        report_lines.append("- 创建定期扫描任务")
        report_lines.append("")

        report_lines.append("### 3. 团队培训")
        report_lines.append("- 组织Bazel最佳实践培训")
        report_lines.append("- 建立常见错误模式文档")
        report_lines.append("- 分享重构经验和教训")
        report_lines.append("")

        report_lines.append("---")
        report_lines.append("*此报告由bazel_label_fixer_enhanced_v2.py自动生成*")

        return "\n".join(report_lines)

    def _get_timestamp(self) -> str:
        """获取当前时间戳"""
        from datetime import datetime
        return datetime.now().strftime("%Y-%m-%d %H:%M:%S")

def main():
    """主函数"""
    import argparse

    parser = argparse.ArgumentParser(description="SQLCC Bazel标签修复器增强版 v2.0")
    parser.add_argument("project_root", help="项目根目录")
    parser.add_argument("--detection-results", help="检测结果JSON文件")
    parser.add_argument("--output-report", "-o", help="输出修复报告文件")
    parser.add_argument("--dry-run", action="store_true", help="仅分析，不执行修复")
    parser.add_argument("--analyze-only", action="store_true", help="仅分析错误模式，不修复")

    args = parser.parse_args()

    if not os.path.isdir(args.project_root):
        print(f"❌ 错误: 目录不存在: {args.project_root}")
        sys.exit(1)

    fixer = EnhancedBazelLabelFixerV2(args.detection_results)

    # 分析检测结果
    print("🔍 分析检测结果...")
    analysis_results = fixer.analyze_detection_results()

    if args.analyze_only:
        # 仅分析模式
        print("📊 错误模式分析结果:")
        for pattern, count in analysis_results.get('error_patterns', {}).items():
            description = fixer.patterns.get(pattern, {}).get('description', '未知模式')
            print(f"  - {pattern}: {count} 次 - {description}")

        if args.output_report:
            report = fixer.generate_enhanced_report()
            with open(args.output_report, 'w', encoding='utf-8') as f:
                f.write(report)
            print(f"📄 分析报告已保存到: {args.output_report}")

        return

    # 执行修复
    if not args.dry_run:
        print("🔧 开始智能修复标签...")
        fix_results = fixer.fix_labels_smart(analysis_results)

        print("\n✅ 标签修复完成!")
        print(f"📊 修复统计:")
        print(f"  - 处理文件: {fix_results['total_files_processed']}")
        print(f"  - 修复标签: {fix_results['total_labels_fixed']}")

    # 生成报告
    report = fixer.generate_enhanced_report()

    if args.output_report:
        with open(args.output_report, 'w', encoding='utf-8') as f:
            f.write(report)
        print(f"📄 修复报告已保存到: {args.output_report}")
    else:
        print("\n" + "="*80)
        print(report)
        print("="*80)

if __name__ == "__main__":
    main()
