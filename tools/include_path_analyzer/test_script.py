#!/usr/bin/env python3
"""
SQLCC Include路径分析器测试脚本
用于验证工具的基本功能
"""

import sys
import os
from pathlib import Path

# 添加项目根目录到Python路径
project_root = Path(__file__).parent.parent.parent
sys.path.insert(0, str(project_root))

try:
    from tools.include_path_analyzer import IncludePathAnalyzerApp
    print("✅ 模块导入成功")
except ImportError as e:
    print(f"❌ 模块导入失败: {e}")
    sys.exit(1)

def test_config_loading():
    """测试配置加载"""
    print("\n🔧 测试配置加载...")
    try:
        app = IncludePathAnalyzerApp()
        if app.initialize():
            print("✅ 配置加载成功")
            print(f"  项目名称: {app.config.project_name}")
            print(f"  项目根目录: {app.config.project_root}")
            print(f"  Include目录: {app.config.include_dirs}")
            print(f"  源代码目录: {app.config.src_dirs}")
            return True
        else:
            print("❌ 配置加载失败")
            return False
    except Exception as e:
        print(f"❌ 配置加载异常: {e}")
        return False

def test_basic_analysis():
    """测试基本分析功能"""
    print("\n🔍 测试基本分析功能...")
    try:
        app = IncludePathAnalyzerApp()

        if not app.initialize():
            print("❌ 初始化失败")
            return False

        # 分析一个小目录作为测试
        test_dirs = ["src/sql_parser"]  # 只分析一个小的目录

        result = app.analyzer.analyze_project(test_dirs)

        print("✅ 分析完成")
        print(f"  分析文件数: {result.total_files}")
        print(f"  发现问题数: {result.total_issues}")

        if result.issues_by_severity:
            print("  问题严重程度分布:")
            for severity, count in result.issues_by_severity.items():
                print(f"    {severity}: {count}")

        if result.issues_by_file:
            print("  有问题的文件:")
            for file_path, issues in list(result.issues_by_file.items())[:3]:  # 只显示前3个
                print(f"    {file_path}: {len(issues)} 个问题")

        return True

    except Exception as e:
        print(f"❌ 分析测试异常: {e}")
        import traceback
        traceback.print_exc()
        return False

def test_report_generation():
    """测试报告生成功能"""
    print("\n📄 测试报告生成功能...")
    try:
        app = IncludePathAnalyzerApp()

        if not app.initialize():
            print("❌ 初始化失败")
            return False

        # 生成一个简单的分析结果进行测试
        result = app.analyzer.analyze_project(["src/sql_parser"])

        # 先保存分析结果到文件
        json_file = "test_include_analysis.json"
        app._save_analysis_result(result, json_file)

        # 然后生成JSON报告
        success = app.generate_report(json_file, "json", f"report_{json_file}")

        if success:
            print(f"✅ JSON报告生成成功")
            # 清理测试文件
            if os.path.exists(json_file):
                os.remove(json_file)
            report_file = f"report_{json_file}"
            if os.path.exists(report_file):
                os.remove(report_file)
            return True
        else:
            print("❌ JSON报告生成失败")
            return False

    except Exception as e:
        print(f"❌ 报告生成测试异常: {e}")
        return False

def main():
    """主测试函数"""
    print("🚀 SQLCC Include路径分析器测试开始")
    print("=" * 50)

    tests = [
        ("配置加载", test_config_loading),
        ("基本分析", test_basic_analysis),
        ("报告生成", test_report_generation),
    ]

    passed = 0
    total = len(tests)

    for test_name, test_func in tests:
        print(f"\n📋 执行测试: {test_name}")
        print("-" * 30)
        if test_func():
            passed += 1
            print(f"✅ {test_name} 测试通过")
        else:
            print(f"❌ {test_name} 测试失败")

    print("\n" + "=" * 50)
    print(f"🎯 测试结果: {passed}/{total} 通过")

    if passed == total:
        print("🎉 所有测试通过！Include路径分析器工作正常。")
        return 0
    else:
        print("⚠️  部分测试失败，请检查配置和代码。")
        return 1

if __name__ == '__main__':
    sys.exit(main())
