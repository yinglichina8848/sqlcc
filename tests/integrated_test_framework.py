#!/usr/bin/env python3
"""
SQLCC v1.1.1 集成测试框架
基于实际测试文件分布的智能测试管理系统
"""

import os
import sys
import subprocess
import json
import argparse
from pathlib import Path
from typing import Dict, List, Optional, Tuple, Set
from dataclasses import dataclass, asdict
from datetime import datetime
import glob


@dataclass
class TestFile:
    """测试文件信息"""
    name: str
    path: str
    category: str
    priority: str
    timeout: int
    dependencies: List[str]
    description: str


@dataclass
class TestCategory:
    """测试分类"""
    name: str
    description: str
    files: List[TestFile]
    timeout: int = 300
    parallel: bool = True


class IntegratedTestFramework:
    """集成测试框架"""
    
    def __init__(self, project_root: str):
        self.project_root = Path(project_root)
        self.tests_dir = self.project_root / "tests"
        self.build_dir = self.project_root / "build"
        
        # 加载测试文件映射
        self.test_files = self._discover_test_files()
        self.categories = self._organize_categories()
        
    def _discover_test_files(self) -> List[TestFile]:
        """发现并分类所有测试文件"""
        test_files = []
        
        # 定义测试文件分类规则
        file_patterns = {
            # DCL相关测试 (数据控制语言)
            'dcl': {
                'patterns': ['*dcl*', '*grant*', '*revoke*', '*permission*'],
                'category': 'dcl',
                'priority': 'high',
                'timeout': 600,
                'description': '数据控制语言测试 - GRANT、REVOKE、权限管理'
            },
            
            # 解析器测试
            'parser': {
                'patterns': ['*parser*', '*lexer*', '*token*', '*keyword*', 'test_*'],
                'category': 'parser',
                'priority': 'high',
                'timeout': 480,
                'description': 'SQL解析器测试 - 词法分析、语法分析'
            },
            
            # 执行器测试
            'executor': {
                'patterns': ['*executor*', '*sql_executor*'],
                'category': 'executor',
                'priority': 'high',
                'timeout': 540,
                'description': 'SQL执行器测试 - 查询执行、结果处理'
            },
            
            # 事务管理测试
            'transaction': {
                'patterns': ['*transaction*', '*deadlock*', '*lock*'],
                'category': 'transaction',
                'priority': 'high',
                'timeout': 600,
                'description': '事务管理测试 - ACID特性、并发控制'
            },
            
            # 存储引擎测试
            'storage': {
                'patterns': ['*storage*', '*disk*', '*buffer*', '*b_plus*', '*page*'],
                'category': 'storage',
                'priority': 'high',
                'timeout': 480,
                'description': '存储引擎测试 - 页面管理、索引结构'
            },
            
            # 网络功能测试
            'network': {
                'patterns': ['*network*', '*client*', '*server*', '*tls*', '*aes*'],
                'category': 'network',
                'priority': 'medium',
                'timeout': 360,
                'description': '网络功能测试 - 客户端-服务器通信、加密'
            },
            
            # 集成测试
            'integration': {
                'patterns': ['*integration*', '*comprehensive*', '*e2e*', '*end*'],
                'category': 'integration',
                'priority': 'high',
                'timeout': 900,
                'description': '集成测试 - 端到端功能验证'
            },
            
            # 高级SQL功能测试
            'advanced': {
                'patterns': ['*join*', '*subquery*', '*window*', '*advanced*'],
                'category': 'advanced',
                'priority': 'medium',
                'timeout': 420,
                'description': '高级SQL功能测试 - JOIN、子查询、窗口函数'
            },
            
            # 性能测试
            'performance': {
                'patterns': ['*performance*', '*benchmark*', '*stress*', '*load*'],
                'category': 'performance',
                'priority': 'low',
                'timeout': 1200,
                'description': '性能测试 - 基准测试、压力测试'
            },
            
            # 基础功能测试
            'basic': {
                'patterns': ['*test*', '*simple*', '*basic*', '*create*', '*insert*'],
                'category': 'basic',
                'priority': 'medium',
                'timeout': 300,
                'description': '基础功能测试 - CRUD操作、基本语法'
            }
        }
        
        # 调试信息
        print(f"🔍 扫描目录: {self.tests_dir}")
        print(f"🔍 项目根目录: {self.project_root}")
        
        # 扫描所有测试文件
        cpp_files = list(self.tests_dir.rglob("*.cpp"))
        print(f"🔍 发现 .cpp 文件: {len(cpp_files)}")
        
        for cpp_file in cpp_files:
            if "__pycache__" in str(cpp_file):
                continue
                
            file_name = cpp_file.name
            file_path = str(cpp_file.relative_to(self.project_root))  # 相对于项目根目录
            
            print(f"📄 处理文件: {file_name} ({file_path})")
            
            # 根据文件名模式分类
            assigned_category = None
            for cat_name, config in file_patterns.items():
                for pattern in config['patterns']:
                    if any(pat in file_name.lower() for pat in pattern.replace('*', '').split()):
                        assigned_category = cat_name
                        print(f"   🔗 匹配模式: {pattern} -> {cat_name}")
                        break
                if assigned_category:
                    break
            
            # 默认归类为基础测试
            if not assigned_category:
                assigned_category = 'basic'
                print(f"   📋 默认分类: basic")
            
            # 获取对应配置
            config = file_patterns.get(assigned_category, file_patterns['basic'])
            
            test_file = TestFile(
                name=file_name,
                path=file_path,
                category=config['category'],
                priority=config['priority'],
                timeout=config['timeout'],
                dependencies=[],
                description=config['description']
            )
            
            test_files.append(test_file)
        
        # 添加.cc文件
        cc_files = list(self.tests_dir.rglob("*.cc"))
        print(f"🔍 发现 .cc 文件: {len(cc_files)}")
        
        for cc_file in cc_files:
            if "__pycache__" in str(cc_file):
                continue
                
            file_name = cc_file.name
            file_path = str(cc_file.relative_to(self.project_root))
            
            test_file = TestFile(
                name=file_name,
                path=file_path,
                category='network',  # .cc文件主要是网络测试
                priority='medium',
                timeout=360,
                dependencies=[],
                description='网络功能测试 - C++源文件'
            )
            
            test_files.append(test_file)
            
        print(f"✅ 总计发现测试文件: {len(test_files)}")
        
        return test_files
    
    def _organize_categories(self) -> Dict[str, TestCategory]:
        """组织测试分类"""
        categories = {}
        
        # 按分类组织测试文件
        for test_file in self.test_files:
            if test_file.category not in categories:
                categories[test_file.category] = TestCategory(
                    name=test_file.category,
                    description=test_file.description,
                    files=[],
                    timeout=max([f.timeout for f in self.test_files if f.category == test_file.category]),
                    parallel=True
                )
            
            categories[test_file.category].files.append(test_file)
        
        return categories
    
    def generate_unified_cmake(self, output_file: Optional[str] = None) -> str:
        """生成统一的CMake配置"""
        cmake_lines = []
        cmake_lines.append("# SQLCC v1.1.1 集成测试框架自动生成")
        cmake_lines.append(f"# 生成时间: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
        cmake_lines.append("# 基于实际测试文件分布的智能配置")
        cmake_lines.append("")
        
        # 基本配置
        cmake_lines.append("# 基本包含目录")
        cmake_lines.append("include_directories(")
        cmake_lines.append("    ${CMAKE_SOURCE_DIR}/include")
        cmake_lines.append("    ${CMAKE_SOURCE_DIR}/include/core")
        cmake_lines.append("    ${CMAKE_SOURCE_DIR}/include/network")
        cmake_lines.append("    ${CMAKE_SOURCE_DIR}/include/sql")
        cmake_lines.append("    ${CMAKE_SOURCE_DIR}/include/utils")
        cmake_lines.append(")")
        cmake_lines.append("")
        
        # 按分类生成测试配置
        priority_order = ['high', 'medium', 'low']
        for priority in priority_order:
            cmake_lines.append(f"# ===== {priority.upper()} 优先级测试 =====")
            cmake_lines.append("")
            
            for category_name, category in sorted(self.categories.items()):
                # 筛选该优先级的测试
                priority_tests = [f for f in category.files if f.priority == priority]
                
                if not priority_tests:
                    continue
                
                cmake_lines.append(f"# {category.description}")
                cmake_lines.append(f"# 分类: {category.name} ({priority} 优先级)")
                cmake_lines.append("")
                
                # 为每个测试文件生成配置
                for test_file in priority_tests:
                    test_name = Path(test_file.path).stem
                    
                    cmake_lines.append(f"# 测试文件: {test_file.name}")
                    cmake_lines.append(f"add_executable({test_name} {test_file.path})")
                    cmake_lines.append(f"target_link_libraries({test_name}")
                    cmake_lines.append("    PRIVATE")
                    cmake_lines.append("    gtest")
                    cmake_lines.append("    gtest_main")
                    cmake_lines.append("    pthread")
                    cmake_lines.append("    ${CMAKE_DL_LIBS}")
                    cmake_lines.append("    sqlcc_core_lib")
                    cmake_lines.append("    sqlcc_parser")
                    cmake_lines.append("    sqlcc_config_manager")
                    cmake_lines.append("    sqlcc_storage_engine")
                    cmake_lines.append("    sqlcc_transaction_manager")
                    cmake_lines.append("    sqlcc_executor")
                    
                    # 根据测试类型添加特定库
                    if test_file.category == 'network':
                        cmake_lines.append("    sqlcc_network")
                    
                    cmake_lines.append(")")
                    cmake_lines.append("")
                    
                    # 添加CTest配置
                    cmake_lines.append(f"add_test(NAME {test_name} COMMAND {test_name})")
                    cmake_lines.append(f"set_tests_properties({test_name} PROPERTIES TIMEOUT {test_file.timeout})")
                    cmake_lines.append("")
                
                cmake_lines.append("")
        
        # 测试套件定义
        cmake_lines.append("# ===== 测试套件定义 =====")
        cmake_lines.append("")
        
        # 核心测试套件
        cmake_lines.append("# 核心功能测试套件")
        cmake_lines.append("add_test(NAME core_suite COMMAND ${CMAKE_CTEST_COMMAND} -R \"(dcl|parser|executor|transaction|storage)_test\")")
        cmake_lines.append("")
        
        # 快速测试套件
        cmake_lines.append("# 快速测试套件")
        cmake_lines.append("add_test(NAME quick_suite COMMAND ${CMAKE_CTEST_COMMAND} -R \"(dcl|parser)_test\" --output-on-failure)")
        cmake_lines.append("")
        
        # 完整测试套件
        cmake_lines.append("# 完整测试套件")
        cmake_lines.append("add_test(NAME full_suite COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure)")
        cmake_lines.append("")
        
        # 并行测试配置
        cmake_lines.append("# 启用并行测试")
        cmake_lines.append("set_property(TEST core_suite quick_suite full_suite PROPERTY PARALLEL TRUE)")
        cmake_lines.append("")
        
        cmake_text = "\n".join(cmake_lines)
        
        if output_file:
            try:
                with open(output_file, 'w', encoding='utf-8') as f:
                    f.write(cmake_text)
                print(f"✅ CMake配置已生成: {output_file}")
            except Exception as e:
                print(f"❌ CMake配置生成失败: {e}")
        
        return cmake_text
    
    def generate_test_runner_script(self, output_file: Optional[str] = None) -> str:
        """生成测试运行脚本"""
        script_lines = []
        script_lines.append("#!/bin/bash")
        script_lines.append("# SQLCC v1.1.1 集成测试运行脚本")
        script_lines.append(f"# 生成时间: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
        script_lines.append("")
        
        script_lines.append("set -e")
        script_lines.append("")
        
        script_lines.append("# 配置参数")
        script_lines.append('BUILD_DIR="${1:-./build}"')
        script_lines.append('TEST_SUITE="${2:-full_suite}"')
        script_lines.append('VERBOSE="${3:-false}"')
        script_lines.append("")
        
        script_lines.append("echo \"🚀 SQLCC v1.1.1 集成测试框架\"")
        script_lines.append(f'echo "📅 测试时间: $(date)"')
        script_lines.append('echo "🏗️  构建目录: $BUILD_DIR"')
        script_lines.append('echo "📋 测试套件: $TEST_SUITE"')
        script_lines.append("")
        
        script_lines.append("# 检查构建目录")
        script_lines.append('if [ ! -d "$BUILD_DIR" ]; then')
        script_lines.append('    echo "❌ 构建目录不存在: $BUILD_DIR"')
        script_lines.append('    echo "💡 请先运行: mkdir -p $BUILD_DIR && cd $BUILD_DIR && cmake .. && make"')
        script_lines.append('    exit 1')
        script_lines.append("fi")
        script_lines.append("")
        
        script_lines.append("# 切换到构建目录")
        script_lines.append('cd "$BUILD_DIR"')
        script_lines.append("")
        
        script_lines.append("# 运行测试")
        script_lines.append('if [ "$VERBOSE" = "true" ]; then')
        script_lines.append('    echo "🔍 详细模式运行测试..."')
        script_lines.append('    ctest -V -R "$TEST_SUITE"')
        script_lines.append('else')
        script_lines.append('    echo "⚡ 标准模式运行测试..."')
        script_lines.append('    ctest -R "$TEST_SUITE" --output-on-failure')
        script_lines.append('fi')
        script_lines.append("")
        
        script_lines.append('echo "✅ 测试完成"')
        
        script_text = "\n".join(script_lines)
        
        if output_file:
            try:
                with open(output_file, 'w', encoding='utf-8') as f:
                    f.write(script_text)
                # 添加执行权限
                os.chmod(output_file, 0o755)
                print(f"✅ 测试运行脚本已生成: {output_file}")
            except Exception as e:
                print(f"❌ 测试运行脚本生成失败: {e}")
        
        return script_text
    
    def generate_framework_report(self, output_file: Optional[str] = None) -> str:
        """生成框架报告"""
        report_lines = []
        report_lines.append("=" * 80)
        report_lines.append("SQLCC v1.1.1 集成测试框架报告")
        report_lines.append(f"生成时间: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
        report_lines.append("=" * 80)
        report_lines.append("")
        
        # 总体统计
        total_files = len(self.test_files)
        total_categories = len(self.categories)
        
        report_lines.append("📊 总体统计:")
        report_lines.append(f"   • 测试文件总数: {total_files}")
        report_lines.append(f"   • 测试分类数: {total_categories}")
        report_lines.append(f"   • 项目目录: {self.project_root}")
        report_lines.append(f"   • 测试目录: {self.tests_dir}")
        report_lines.append("")
        
        # 分类统计
        report_lines.append("📁 分类统计:")
        for category_name, category in sorted(self.categories.items()):
            file_count = len(category.files)
            high_priority = len([f for f in category.files if f.priority == 'high'])
            medium_priority = len([f for f in category.files if f.priority == 'medium'])
            low_priority = len([f for f in category.files if f.priority == 'low'])
            
            report_lines.append(f"   • {category.name} ({category.description})")
            report_lines.append(f"     - 文件数: {file_count}")
            report_lines.append(f"     - 优先级分布: 高({high_priority}) 中({medium_priority}) 低({low_priority})")
            report_lines.append(f"     - 超时设置: {category.timeout}秒")
            report_lines.append("")
        
        # 优先级分布
        priority_stats = {}
        for test_file in self.test_files:
            priority = test_file.priority
            priority_stats[priority] = priority_stats.get(priority, 0) + 1
        
        report_lines.append("🎯 优先级分布:")
        for priority in ['high', 'medium', 'low']:
            count = priority_stats.get(priority, 0)
            percentage = (count / total_files * 100) if total_files > 0 else 0
            report_lines.append(f"   • {priority.upper()}: {count} 个文件 ({percentage:.1f}%)")
        report_lines.append("")
        
        # 详细文件列表
        report_lines.append("📄 详细文件列表:")
        for category_name, category in sorted(self.categories.items()):
            report_lines.append(f"\n   分类: {category.name}")
            for test_file in sorted(category.files, key=lambda x: (x.priority != 'high', x.priority != 'medium', x.name)):
                report_lines.append(f"     • {test_file.name}")
                report_lines.append(f"       路径: {test_file.path}")
                report_lines.append(f"       优先级: {test_file.priority}")
                report_lines.append(f"       超时: {test_file.timeout}秒")
        
        report_text = "\n".join(report_lines)
        
        if output_file:
            try:
                with open(output_file, 'w', encoding='utf-8') as f:
                    f.write(report_text)
                print(f"📄 框架报告已保存到: {output_file}")
            except Exception as e:
                print(f"❌ 框架报告保存失败: {e}")
        
        return report_text
    
    def interactive_setup(self):
        """交互式设置"""
        print("🎯 SQLCC v1.1.1 集成测试框架设置")
        print("=" * 50)
        
        # 显示统计信息
        total_files = len(self.test_files)
        total_categories = len(self.categories)
        print(f"\n📊 发现 {total_files} 个测试文件，分布在 {total_categories} 个分类中")
        
        # 显示分类概览
        print("\n📁 测试分类概览:")
        for category_name, category in sorted(self.categories.items()):
            file_count = len(category.files)
            print(f"   • {category.name}: {file_count} 个文件")
        
        # 生成配置选项
        print("\n🔧 可用的配置生成选项:")
        print("   1. 生成统一 CMakeLists.txt")
        print("   2. 生成测试运行脚本")
        print("   3. 生成框架报告")
        print("   4. 生成所有配置")
        
        while True:
            try:
                choice = input("\n请选择操作 (输入数字): ").strip()
                
                if choice == "1":
                    output_file = self.tests_dir / "CMakeLists_integrated.txt"
                    self.generate_unified_cmake(str(output_file))
                    
                elif choice == "2":
                    output_file = self.tests_dir / "run_tests.sh"
                    self.generate_test_runner_script(str(output_file))
                    
                elif choice == "3":
                    output_file = self.tests_dir / "test_framework_report.txt"
                    self.generate_framework_report(str(output_file))
                    
                elif choice == "4":
                    print("\n🔄 生成所有配置...")
                    
                    cmake_file = self.tests_dir / "CMakeLists_integrated.txt"
                    script_file = self.tests_dir / "run_tests.sh"
                    report_file = self.tests_dir / "test_framework_report.txt"
                    
                    self.generate_unified_cmake(str(cmake_file))
                    self.generate_test_runner_script(str(script_file))
                    self.generate_framework_report(str(report_file))
                    
                    print(f"\n✅ 所有配置已生成:")
                    print(f"   • CMake配置: {cmake_file}")
                    print(f"   • 运行脚本: {script_file}")
                    print(f"   • 框架报告: {report_file}")
                    
                else:
                    print("❌ 无效选择，请重新输入")
                    continue
                
                break
                
            except ValueError:
                print("❌ 请输入有效数字")
            except KeyboardInterrupt:
                print("\n👋 操作已取消")
                return
        
        print("\n🎉 集成测试框架设置完成!")
        return True


def main():
    """主函数"""
    parser = argparse.ArgumentParser(description="SQLCC v1.1.1 集成测试框架")
    parser.add_argument("--project-root", default=".", help="项目根目录")
    parser.add_argument("--cmake", help="生成CMake配置到指定文件")
    parser.add_argument("--script", help="生成测试脚本到指定文件")
    parser.add_argument("--report", help="生成报告到指定文件")
    parser.add_argument("--interactive", action="store_true", help="交互式设置")
    parser.add_argument("--stats", action="store_true", help="显示统计信息")
    
    args = parser.parse_args()
    
    try:
        framework = IntegratedTestFramework(args.project_root)
        
        if args.interactive:
            framework.interactive_setup()
        elif args.stats:
            framework.generate_framework_report()
        elif args.cmake:
            framework.generate_unified_cmake(args.cmake)
        elif args.script:
            framework.generate_test_runner_script(args.script)
        elif args.report:
            framework.generate_framework_report(args.report)
        else:
            # 默认生成所有配置
            framework.interactive_setup()
            
    except Exception as e:
        print(f"❌ 错误: {e}")
        sys.exit(1)


if __name__ == "__main__":
    main()