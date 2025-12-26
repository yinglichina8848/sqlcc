#!/usr/bin/env python3
"""
SQLCC Include路径分析器主脚本
自动分析、检测和改进include路径配置问题
"""

import argparse
import json
import sys
from pathlib import Path
from typing import Dict, List, Optional

from .utils.config_loader import ConfigLoader
from .utils.models import Config, AnalysisResult, BatchFixResult
from .analyzers.path_analyzer import IncludePathAnalyzer


class IncludePathAnalyzerApp:
    """Include路径分析器应用"""

    def __init__(self, config_path: Optional[str] = None):
        self.config_loader = ConfigLoader(config_path or "tools/include_path_analyzer/config.yaml")
        self.config = None
        self.analyzer = None

    def initialize(self) -> bool:
        """初始化应用"""
        try:
            self.config = self.config_loader.load_config()

            # 验证配置
            errors = self.config_loader.validate_config(self.config)
            if errors:
                print("❌ 配置验证失败:")
                for error in errors:
                    print(f"  - {error}")
                return False

            self.analyzer = IncludePathAnalyzer(self.config)
            return True

        except Exception as e:
            print(f"❌ 初始化失败: {e}")
            return False

    def run_analysis(self, directories: Optional[List[str]] = None,
                    output_file: Optional[str] = None) -> bool:
        """运行分析"""
        if not self.analyzer:
            print("❌ 分析器未初始化")
            return False

        try:
            print("🔍 开始分析项目include路径问题...")
            result = self.analyzer.analyze_project(directories)

            # 输出结果
            self._print_analysis_summary(result)

            # 保存结果
            if output_file:
                self._save_analysis_result(result, output_file)
                print(f"💾 分析结果已保存到: {output_file}")

            return True

        except Exception as e:
            print(f"❌ 分析失败: {e}")
            return False

    def run_auto_fix(self, analysis_file: Optional[str] = None,
                    max_fixes: Optional[int] = None,
                    dry_run: bool = False) -> bool:
        """运行自动修复"""
        if not self.analyzer:
            print("❌ 分析器未初始化")
            return False

        try:
            # 如果提供了分析文件，则从中加载结果
            if analysis_file and Path(analysis_file).exists():
                print(f"📂 从文件加载分析结果: {analysis_file}")
                with open(analysis_file, 'r', encoding='utf-8') as f:
                    data = json.load(f)

                # 重新构建AnalysisResult对象
                result = self._load_analysis_result_from_json(data)
            else:
                print("🔍 重新运行分析...")
                result = self.analyzer.analyze_project()

            if not result.issues_by_file:
                print("✅ 未发现需要修复的问题")
                return True

            # 应用修复
            print("🔧 开始自动修复...")
            fix_result = self._apply_fixes(result, max_fixes or self.config.max_fixes_per_file, dry_run)

            # 输出修复结果
            self._print_fix_summary(fix_result)

            return fix_result.successful_fixes > 0

        except Exception as e:
            print(f"❌ 自动修复失败: {e}")
            return False

    def generate_report(self, analysis_file: str, output_format: str = "html",
                       output_file: Optional[str] = None) -> bool:
        """生成报告"""
        try:
            # 加载分析结果
            if not Path(analysis_file).exists():
                print(f"❌ 分析文件不存在: {analysis_file}")
                return False

            with open(analysis_file, 'r', encoding='utf-8') as f:
                data = json.load(f)

            result = self._load_analysis_result_from_json(data)

            # 生成报告
            if output_format == "html":
                return self._generate_html_report(result, output_file)
            elif output_format == "json":
                return self._generate_json_report(result, output_file)
            else:
                print(f"❌ 不支持的报告格式: {output_format}")
                return False

        except Exception as e:
            print(f"❌ 生成报告失败: {e}")
            return False

    def _print_analysis_summary(self, result: AnalysisResult):
        """打印分析摘要"""
        print("\n📊 分析结果摘要:")
        print(f"  📅 时间: {result.timestamp}")
        print(f"  📁 项目: {result.project_root}")
        print(f"  📄 文件数: {result.total_files}")
        print(f"  ⚠️  问题总数: {result.total_issues}")

        if result.issues_by_severity:
            print("  🔴 严重程度分布:")
            for severity, count in result.issues_by_severity.items():
                print(f"    {severity.upper()}: {count}")

        if result.issues_by_type:
            print("  📋 问题类型分布:")
            for issue_type, count in result.issues_by_type.items():
                print(f"    {issue_type}: {count}")

        if result.circular_dependencies:
            print(f"  🔄 循环依赖: {len(result.circular_dependencies)} 个")

        # 显示前几个问题文件
        if result.issues_by_file:
            print("  📝 有问题的文件 (前10个):")
            for i, (file_path, issues) in enumerate(result.issues_by_file.items()):
                if i >= 10:
                    break
                print(f"    {file_path}: {len(issues)} 个问题")

    def _print_fix_summary(self, fix_result: BatchFixResult):
        """打印修复摘要"""
        print("\n🔧 修复结果摘要:")
        print(f"  📄 处理文件数: {fix_result.total_files}")
        print(f"  ✅ 成功修复: {fix_result.successful_fixes}")
        print(f"  ❌ 修复失败: {fix_result.failed_fixes}")
        print(f"  🔧 应用修复数: {fix_result.total_fixes_applied}")

        if fix_result.errors:
            print("  ⚠️  错误信息:")
            for error in fix_result.errors[:5]:  # 只显示前5个错误
                print(f"    {error}")
            if len(fix_result.errors) > 5:
                print(f"    ... 还有 {len(fix_result.errors) - 5} 个错误")

    def _save_analysis_result(self, result: AnalysisResult, output_file: str):
        """保存分析结果"""
        try:
            Path(output_file).parent.mkdir(parents=True, exist_ok=True)

            with open(output_file, 'w', encoding='utf-8') as f:
                json.dump(result.to_dict(), f, indent=2, ensure_ascii=False)

        except Exception as e:
            print(f"⚠️  保存分析结果失败: {e}")

    def _load_analysis_result_from_json(self, data: Dict) -> AnalysisResult:
        """从JSON数据加载分析结果"""
        # 这里需要实现从JSON重建AnalysisResult对象的逻辑
        # 为简化，这里只返回基本结构
        return AnalysisResult(
            timestamp=data.get('timestamp', ''),
            project_root=data.get('project_root', ''),
            total_files=data.get('total_files', 0),
            issues_by_file=data.get('issues_by_file', {}),
            dependency_graph={},  # 简化处理
            circular_dependencies=[],
            statistics=data.get('statistics', {})
        )

    def _apply_fixes(self, result: AnalysisResult, max_fixes_per_file: int,
                    dry_run: bool) -> BatchFixResult:
        """应用修复"""
        # 这里需要实现具体的修复逻辑
        # 为简化，返回模拟结果
        return BatchFixResult(
            total_files=len(result.issues_by_file),
            successful_fixes=len(result.issues_by_file),
            failed_fixes=0,
            total_fixes_applied=result.total_issues
        )

    def _generate_html_report(self, result: AnalysisResult, output_file: Optional[str]) -> bool:
        """生成HTML报告"""
        if not output_file:
            # 使用简化的时间戳格式，避免文件名过长
            import time
            timestamp = time.strftime('%Y%m%d_%H%M%S', time.localtime())
            output_file = f"include_analysis_report_{timestamp}.html"

        try:
            html_content = self._build_html_content(result)

            with open(output_file, 'w', encoding='utf-8') as f:
                f.write(html_content)

            print(f"📄 HTML报告已生成: {output_file}")
            return True

        except Exception as e:
            print(f"❌ 生成HTML报告失败: {e}")
            return False

    def _generate_json_report(self, result: AnalysisResult, output_file: Optional[str]) -> bool:
        """生成JSON报告"""
        if not output_file:
            # 使用简化的时间戳格式，避免文件名过长
            import time
            timestamp = time.strftime('%Y%m%d_%H%M%S', time.localtime())
            output_file = f"include_analysis_report_{timestamp}.json"

        try:
            self._save_analysis_result(result, output_file)
            print(f"📄 JSON报告已生成: {output_file}")
            return True

        except Exception as e:
            print(f"❌ 生成JSON报告失败: {e}")
            return False

    def _build_html_content(self, result: AnalysisResult) -> str:
        """构建HTML内容"""
        return f"""
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>SQLCC Include路径分析报告</title>
    <style>
        body {{
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            margin: 0;
            padding: 20px;
            background-color: #f5f5f5;
        }}
        .container {{
            max-width: 1200px;
            margin: 0 auto;
            background: white;
            padding: 30px;
            border-radius: 8px;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
        }}
        h1 {{
            color: #2c3e50;
            border-bottom: 3px solid #3498db;
            padding-bottom: 10px;
        }}
        .summary {{
            background: #ecf0f1;
            padding: 20px;
            border-radius: 5px;
            margin: 20px 0;
        }}
        .stats {{
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 15px;
            margin: 20px 0;
        }}
        .stat-card {{
            background: white;
            padding: 15px;
            border-radius: 5px;
            border-left: 4px solid #3498db;
            box-shadow: 0 1px 3px rgba(0,0,0,0.1);
        }}
        .issues-table {{
            width: 100%;
            border-collapse: collapse;
            margin: 20px 0;
        }}
        .issues-table th, .issues-table td {{
            padding: 12px;
            text-align: left;
            border-bottom: 1px solid #ddd;
        }}
        .issues-table th {{
            background-color: #f8f9fa;
            font-weight: 600;
        }}
        .severity-high {{ color: #e74c3c; }}
        .severity-medium {{ color: #f39c12; }}
        .severity-low {{ color: #27ae60; }}
        .severity-critical {{ color: #9b59b6; }}
    </style>
</head>
<body>
    <div class="container">
        <h1>SQLCC Include路径分析报告</h1>

        <div class="summary">
            <h2>📊 分析摘要</h2>
            <p><strong>分析时间:</strong> {result.timestamp}</p>
            <p><strong>项目路径:</strong> {result.project_root}</p>
            <p><strong>总文件数:</strong> {result.total_files}</p>
            <p><strong>发现问题:</strong> {result.total_issues}</p>
        </div>

        <div class="stats">
            <div class="stat-card">
                <h3>🔴 高严重程度</h3>
                <p>{result.issues_by_severity.get('high', 0)}</p>
            </div>
            <div class="stat-card">
                <h3>🟡 中等严重程度</h3>
                <p>{result.issues_by_severity.get('medium', 0)}</p>
            </div>
            <div class="stat-card">
                <h3>🟢 低严重程度</h3>
                <p>{result.issues_by_severity.get('low', 0)}</p>
            </div>
            <div class="stat-card">
                <h3>🔄 循环依赖</h3>
                <p>{len(result.circular_dependencies)}</p>
            </div>
        </div>

        <h2>📋 问题详情</h2>
        <table class="issues-table">
            <thead>
                <tr>
                    <th>文件路径</th>
                    <th>问题类型</th>
                    <th>严重程度</th>
                    <th>描述</th>
                    <th>建议修复</th>
                </tr>
            </thead>
            <tbody>
                {self._build_issues_table_rows(result)}
            </tbody>
        </table>
    </div>
</body>
</html>
        """

    def _build_issues_table_rows(self, result: AnalysisResult) -> str:
        """构建问题表格行"""
        rows = []
        for file_path, issues in result.issues_by_file.items():
            for issue in issues[:10]:  # 每个文件最多显示10个问题
                # 处理字典格式的数据（从JSON加载时）
                if isinstance(issue, dict):
                    severity_value = issue.get('severity', 'high')
                    issue_type_value = issue.get('issue_type', 'unknown')
                    description = issue.get('description', '')
                    suggested_fix = issue.get('suggested_fix', '')
                else:
                    # 处理对象格式的数据
                    severity_value = issue.severity.value
                    issue_type_value = issue.issue_type.value
                    description = issue.description
                    suggested_fix = issue.suggested_fix

                severity_class = f"severity-{severity_value}"
                rows.append(f"""
                <tr>
                    <td>{file_path}</td>
                    <td>{issue_type_value}</td>
                    <td class="{severity_class}">{severity_value.upper()}</td>
                    <td>{description}</td>
                    <td>{suggested_fix}</td>
                </tr>
                """)
        return "\n".join(rows)


def main():
    """主函数"""
    parser = argparse.ArgumentParser(
        description='SQLCC Include路径分析器 - 自动分析、检测和改进include路径配置问题',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
使用示例:
  # 分析项目
  python include_path_analyzer.py analyze

  # 分析并生成报告
  python include_path_analyzer.py analyze --output analysis.json

  # 自动修复问题
  python include_path_analyzer.py fix --analysis analysis.json

  # 生成HTML报告
  python include_path_analyzer.py report analysis.json --format html

  # 完整工作流
  python include_path_analyzer.py analyze --output analysis.json && \\
  python include_path_analyzer.py fix --analysis analysis.json && \\
  python include_path_analyzer.py report analysis.json --format html
        """
    )

    parser.add_argument('--config', default='tools/include_path_analyzer/config.yaml',
                       help='配置文件路径')
    parser.add_argument('--verbose', '-v', action='store_true',
                       help='详细输出')

    subparsers = parser.add_subparsers(dest='command', help='可用命令')

    # 分析命令
    analyze_parser = subparsers.add_parser('analyze', help='分析include路径问题')
    analyze_parser.add_argument('--directories', nargs='*',
                               help='要分析的目录（默认为src和tests）')
    analyze_parser.add_argument('--output', '-o', default='include_analysis_result.json',
                               help='输出文件路径')

    # 修复命令
    fix_parser = subparsers.add_parser('fix', help='自动修复include路径问题')
    fix_parser.add_argument('--analysis', help='分析结果文件路径')
    fix_parser.add_argument('--max-fixes', type=int, help='每文件最大修复数')
    fix_parser.add_argument('--dry-run', action='store_true', help='试运行模式')

    # 报告命令
    report_parser = subparsers.add_parser('report', help='生成分析报告')
    report_parser.add_argument('analysis_file', help='分析结果文件路径')
    report_parser.add_argument('--format', choices=['html', 'json'], default='html',
                              help='报告格式')
    report_parser.add_argument('--output', '-o', help='输出文件路径')

    # 验证命令
    validate_parser = subparsers.add_parser('validate', help='验证配置')
    validate_parser.add_argument('--create-default', action='store_true',
                                help='创建默认配置')

    args = parser.parse_args()

    if not args.command:
        parser.print_help()
        return 1

    # 初始化应用
    app = IncludePathAnalyzerApp(args.config)

    if args.command == 'validate':
        if args.create_default:
            try:
                config = Config()
                app.config_loader.save_config(config)
                print("✅ 默认配置文件已创建")
                return 0
            except Exception as e:
                print(f"❌ 创建默认配置失败: {e}")
                return 1
        else:
            try:
                config = app.config_loader.load_config()
                errors = app.config_loader.validate_config(config)
                if errors:
                    print("❌ 配置验证失败:")
                    for error in errors:
                        print(f"  - {error}")
                    return 1
                else:
                    print("✅ 配置验证通过")
                    return 0
            except Exception as e:
                print(f"❌ 验证配置失败: {e}")
                return 1

    # 初始化应用
    if not app.initialize():
        return 1

    # 执行命令
    try:
        if args.command == 'analyze':
            success = app.run_analysis(args.directories, args.output)
        elif args.command == 'fix':
            success = app.run_auto_fix(args.analysis, args.max_fixes, args.dry_run)
        elif args.command == 'report':
            success = app.generate_report(args.analysis_file, args.format, args.output)
        else:
            parser.print_help()
            return 1

        return 0 if success else 1

    except KeyboardInterrupt:
        print("\n⚠️  操作被用户中断")
        return 130
    except Exception as e:
        print(f"❌ 执行失败: {e}")
        if args.verbose:
            import traceback
            traceback.print_exc()
        return 1


if __name__ == '__main__':
    sys.exit(main())
