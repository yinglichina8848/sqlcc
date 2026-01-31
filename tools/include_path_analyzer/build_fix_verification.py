#!/usr/bin/env python3
"""
SQLCC BUILD配置修复效果验证脚本
验证include路径分析器对项目BUILD配置的修复效果
"""

import json
import os
from pathlib import Path
from collections import defaultdict


def analyze_json_results(json_file):
    """分析JSON结果文件"""
    print("🔍 分析BUILD配置修复效果...")
    print(f"📂 结果文件: {json_file}")

    with open(json_file, 'r', encoding='utf-8') as f:
        data = json.load(f)

    # 基本信息
    total_files = data.get('total_files', 0)
    total_issues = data.get('total_issues', 0)
    issues_by_severity = data.get('issues_by_severity', {})
    issues_by_type = data.get('issues_by_type', {})
    issues_by_file = data.get('issues_by_file', {})

    print(f"📊 分析结果:")
    print(f"  📄 总文件数: {total_files}")
    print(f"  ⚠️  发现问题数: {total_issues}")

    print("  🔴 严重程度分布:")
    for severity, count in issues_by_severity.items():
        print(f"    {severity.upper()}: {count}")

    print("  📋 问题类型分布:")
    for issue_type, count in issues_by_type.items():
        print(f"    {issue_type}: {count}")

    # 分析问题最多的文件
    file_issue_counts = [(file, len(issues)) for file, issues in issues_by_file.items()]
    file_issue_counts.sort(key=lambda x: x[1], reverse=True)

    print("  📝 问题最多的文件 (Top 10):")
    for file_path, count in file_issue_counts[:10]:
        print(f"    {file_path}: {count} 个问题")

    # 分析问题类型分布
    print("\n🔧 问题类型详细分析:")
    for issue_type, count in sorted(issues_by_type.items(), key=lambda x: x[1], reverse=True):
        percentage = (count / total_issues * 100) if total_issues > 0 else 0
        print(f"    {issue_type}: {count} ({percentage:.1f}%)")
    # 分析严重程度
    high_severity = issues_by_severity.get('high', 0)
    medium_severity = issues_by_severity.get('medium', 0)
    low_severity = issues_by_severity.get('low', 0)

    print("\n⚠️  严重程度评估:")
    print(f"  🔴 高严重程度: {high_severity} ({high_severity/total_issues*100:.1f}%)" if total_issues > 0 else "  🔴 高严重程度: 0 (0.0%)")
    print(f"  🟡 中等严重程度: {medium_severity} ({medium_severity/total_issues*100:.1f}%)" if total_issues > 0 else "  🟡 中等严重程度: 0 (0.0%)")
    print(f"  🟢 低严重程度: {low_severity} ({low_severity/total_issues*100:.1f}%)" if total_issues > 0 else "  🟢 低严重程度: 0 (0.0%)")

    return {
        'total_files': total_files,
        'total_issues': total_issues,
        'issues_by_severity': issues_by_severity,
        'issues_by_type': issues_by_type,
        'top_problem_files': file_issue_counts[:10]
    }


def generate_build_fix_report(analysis_results):
    """生成BUILD修复报告"""
    print("\n📋 生成BUILD配置修复效果报告...")

    total_issues = analysis_results['total_issues']
    issues_by_type = analysis_results['issues_by_type']

    # 评估修复效果
    missing_headers = issues_by_type.get('missing_header', 0)
    incorrect_paths = issues_by_type.get('incorrect_module_path', 0)
    relative_paths = issues_by_type.get('relative_path', 0)

    print("🎯 BUILD配置修复效果评估:")
    print(f"  🔍 总问题数: {total_issues}")

    # 主要问题类型分析
    print("\n📊 主要问题类型:")
    print(f"  ❌ 缺失头文件: {missing_headers} ({missing_headers/total_issues*100:.1f}%)" if total_issues > 0 else "  ❌ 缺失头文件: 0 (0.0%)")
    print(f"  🔄 模块路径错误: {incorrect_paths} ({incorrect_paths/total_issues*100:.1f}%)" if total_issues > 0 else "  🔄 模块路径错误: 0 (0.0%)")
    print(f"  📁 相对路径问题: {relative_paths} ({relative_paths/total_issues*100:.1f}%)" if total_issues > 0 else "  📁 相对路径问题: 0 (0.0%)")

    # 修复建议
    print("
💡 修复建议:"    if missing_headers > 0:
        print(f"  🔧 需要添加/创建 {missing_headers} 个缺失的头文件")
    if incorrect_paths > 0:
        print(f"  🔧 需要修正 {incorrect_paths} 个模块路径映射")
    if relative_paths > 0:
        print(f"  🔧 需要替换 {relative_paths} 个相对路径引用")

    # 影响评估
    high_severity = analysis_results['issues_by_severity'].get('high', 0)
    if high_severity > total_issues * 0.5:
        print("  ⚠️  高严重程度问题占比超过50%，建议优先修复")
    elif total_issues < 100:
        print("  ✅ 问题数量相对较少，修复难度适中")
    else:
        print("  ⚠️  问题数量较多，建议分批次修复")

    return True


def main():
    """主函数"""
    json_file = "build_fix_verification.json"

    if not Path(json_file).exists():
        print(f"❌ 结果文件不存在: {json_file}")
        print("请先运行分析命令: python3 -m tools.include_path_analyzer.include_path_analyzer analyze --output build_fix_verification.json")
        return 1

    print("🚀 SQLCC BUILD配置修复效果验证")
    print("=" * 60)

    try:
        # 分析结果
        results = analyze_json_results(json_file)

        # 生成修复报告
        generate_build_fix_report(results)

        print("\n" + "=" * 60)
        print("✅ BUILD配置修复效果验证完成")

        # 总结
        total_issues = results['total_issues']
        if total_issues == 0:
            print("🎉 恭喜！项目中没有发现include路径配置问题")
        elif total_issues < 50:
            print(f"📈 发现 {total_issues} 个问题，修复工作量适中")
        elif total_issues < 200:
            print(f"📈 发现 {total_issues} 个问题，需要系统性修复")
        else:
            print(f"📈 发现 {total_issues} 个问题，建议分阶段进行修复")

        return 0

    except Exception as e:
        print(f"❌ 验证失败: {e}")
        import traceback
        traceback.print_exc()
        return 1


if __name__ == '__main__':
    exit(main())
