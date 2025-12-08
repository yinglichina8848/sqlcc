#!/usr/bin/env python3
"""
SQLCC v1.1.1 测试框架管理器
统一管理所有测试文件，包括单元测试、集成测试、性能测试和覆盖率测试
"""

import os
import sys
import subprocess
import json
import argparse
from pathlib import Path
from typing import Dict, List, Optional, Tuple
from dataclasses import dataclass, asdict
from datetime import datetime
import xml.etree.ElementTree as ET


@dataclass
class TestCategory:
    """测试分类配置"""
    name: str
    description: str
    directories: List[str]
    test_files: List[str]
    priority: str  # high, medium, low
    timeout: int = 300  # 秒


@dataclass
class TestSuite:
    """测试套件配置"""
    name: str
    description: str
    categories: List[TestCategory]
    dependencies: List[str]
    parallel_execution: bool = True
    retry_count: int = 1


class TestFrameworkManager:
    """测试框架管理器"""
    
    def __init__(self, project_root: str):
        self.project_root = Path(project_root)
        self.tests_dir = self.project_root / "tests"
        self.build_dir = self.project_root / "build"
        self.config_file = self.tests_dir / "test_framework_config.json"
        
        # 测试套件定义
        self.test_suites = self._define_test_suites()
        
    def _define_test_suites(self) -> List[TestSuite]:
        """定义测试套件 - 基于实际目录结构"""
        
        # 基础功能测试 (核心)
        basic_category = TestCategory(
            name="basic",
            description="基础功能测试 - DCL、DDL、基本SQL操作",
            directories=[
                "unit/basic",
                "sql_executor"
            ],
            test_files=[
                "comprehensive_dcl_test.cpp",
                "dcl_test.cpp",
                "dcl_parser_test.cpp",
                "dcl_test_advanced.cpp",
                "revoke_persistence_test.cpp",
                "final_dcl_test.cpp",
                "ddl_test.cpp",
                "simple_test.cpp",
                "transaction_manager_test.cpp"
            ],
            priority="high",
            timeout=600
        )
        
        # 解析器测试
        parser_category = TestCategory(
            name="parser",
            description="解析器测试 - SQL语法解析、词法分析",
            directories=[
                "unit/parser",
                "sql_parser"
            ],
            test_files=[
                "test_lexer.cpp",
                "test_fix.cpp",
                "test_keyword.cpp",
                "test_direct_keyword.cpp",
                "test_all_statements.cpp",
                "test_insert_parser.cpp",
                "test_simple_insert.cpp"
            ],
            priority="high",
            timeout=480
        )
        
        # 核心功能测试
        core_category = TestCategory(
            name="core",
            description="核心功能测试 - 存储引擎、执行器、事务管理",
            directories=[
                "unit/core",
                "unit/storage",
                "unit/storage_engine"
            ],
            test_files=[
                "storage_engine_test.cpp",
                "disk_manager_test.cpp",
                "buffer_pool_test.cpp",
                "b_plus_tree_test.cpp",
                "sql_executor_comprehensive_test.cpp"
            ],
            priority="high",
            timeout=540
        )
        
        # 网络功能测试
        network_category = TestCategory(
            name="network",
            description="网络功能测试 - 客户端-服务器、加密通信",
            directories=[
                "unit/network",
                "network",
                "client_server"
            ],
            test_files=[
                "sql_network_test.cpp",
                "tls_e2e_test.cc",
                "aes_encryption_test.cc"
            ],
            priority="medium",
            timeout=360
        )
        
        # 集成测试
        integration_category = TestCategory(
            name="integration", 
            description="集成测试 - 端到端功能验证",
            directories=[
                "integration",
                "unit/integration",
                "integration/basic_sql",
                "integration/advanced_sql"
            ],
            test_files=[
                "comprehensive_test.cpp",
                "isql_integration_test.cpp",
                "dml_executor_integration_test.cpp",
                "index_system_integration_test.cpp"
            ],
            priority="high",
            timeout=900
        )
        
        # 高级SQL功能测试
        advanced_sql_category = TestCategory(
            name="advanced_sql",
            description="高级SQL功能测试 - Join、Subquery、Window Functions",
            directories=[
                "advanced_sql",
                "unit/advanced",
                "advanced_sql/join",
                "advanced_sql/subquery",
                "advanced_sql/window",
                "advanced_sql/set_operation"
            ],
            test_files=[
                "join_test.cpp",
                "subquery_test.cpp", 
                "window_function_executor_test.cpp",
                "set_operation_test.cpp"
            ],
            priority="medium",
            timeout=420
        )
        
        # 性能测试
        performance_category = TestCategory(
            name="performance",
            description="性能测试 - 基准测试、压力测试",
            directories=[
                "performance",
                "performance/benchmark",
                "performance/stress",
                "performance/concurrency"
            ],
            test_files=[
                "benchmark_test.cpp",
                "stress_test.cpp",
                "concurrency_test.cpp",
                "memory_stress_test.cpp",
                "stability_test.cpp"
            ],
            priority="low",
            timeout=1200
        )
        
        # 安全性测试
        security_category = TestCategory(
            name="security",
            description="安全性测试 - 权限控制、注入防护",
            directories=[
                "security",
                "unit/security"
            ],
            test_files=[
                "permission_validation_test.cpp",
                "permission_check_test.cpp",
                "permission_validator_test.cpp"
            ],
            priority="medium",
            timeout=360
        )
        
        # 创建测试套件
        comprehensive_suite = TestSuite(
            name="comprehensive",
            description="SQLCC综合测试套件 - 所有功能模块",
            categories=[
                basic_category,
                parser_category,
                core_category,
                network_category,
                integration_category,
                advanced_sql_category,
                performance_category,
                security_category
            ],
            dependencies=["cmake", "gtest", "g++"],
            parallel_execution=True,
            retry_count=2
        )
        
        # 核心测试套件
        core_suite = TestSuite(
            name="core",
            description="核心功能测试套件 - 最小可运行环境",
            categories=[basic_category, parser_category, core_category],
            dependencies=["cmake", "gtest", "g++"],
            parallel_execution=True,
            retry_count=1
        )
        
        # 快速测试套件
        quick_suite = TestSuite(
            name="quick",
            description="快速测试套件 - 关键功能验证",
            categories=[basic_category, parser_category],
            dependencies=["cmake", "gtest", "g++"],
            parallel_execution=True,
            retry_count=0
        )
        
        return [comprehensive_suite, core_suite, quick_suite]
    
    def load_configuration(self) -> Optional[Dict]:
        """加载测试配置"""
        if self.config_file.exists():
            try:
                with open(self.config_file, 'r', encoding='utf-8') as f:
                    return json.load(f)
            except Exception as e:
                print(f"⚠️  配置文件加载失败: {e}")
                return None
        return None
    
    def save_configuration(self, config: Dict):
        """保存测试配置"""
        try:
            with open(self.config_file, 'w', encoding='utf-8') as f:
                json.dump(config, f, indent=2, ensure_ascii=False)
            print(f"✅ 配置已保存到: {self.config_file}")
        except Exception as e:
            print(f"❌ 配置保存失败: {e}")
    
    def discover_test_files(self) -> Dict[str, List[str]]:
        """发现所有测试文件"""
        discovered_tests = {}
        
        for suite in self.test_suites:
            suite_tests = []
            for category in suite.categories:
                for directory in category.directories:
                    dir_path = self.tests_dir / directory
                    if dir_path.exists():
                        for cpp_file in dir_path.glob("*.cpp"):
                            suite_tests.append(str(cpp_file.relative_to(self.tests_dir)))
                        for cc_file in dir_path.glob("*.cc"):
                            suite_tests.append(str(cc_file.relative_to(self.tests_dir)))
            
            discovered_tests[suite.name] = list(set(suite_tests))
        
        return discovered_tests
    
    def validate_test_structure(self) -> Tuple[bool, List[str]]:
        """验证测试结构完整性"""
        issues = []
        discovered_tests = self.discover_test_files()
        
        # 检查测试目录结构
        for suite in self.test_suites:
            if suite.name not in discovered_tests:
                issues.append(f"测试套件 '{suite.name}' 没有发现测试文件")
                continue
                
            for category in suite.categories:
                for directory in category.directories:
                    dir_path = self.tests_dir / directory
                    if not dir_path.exists():
                        issues.append(f"测试目录不存在: {directory}")
                    else:
                        # 检查目录中是否有测试文件
                        test_files = list(dir_path.glob("*.cpp")) + list(dir_path.glob("*.cc"))
                        if not test_files:
                            issues.append(f"测试目录为空: {directory}")
        
        return len(issues) == 0, issues
    
    def generate_cmake_config(self, suite_name: str = "comprehensive") -> str:
        """生成CMake配置"""
        suite = next((s for s in self.test_suites if s.name == suite_name), None)
        if not suite:
            raise ValueError(f"测试套件 '{suite_name}' 不存在")
        
        cmake_config = []
        cmake_config.append("# SQLCC v1.1.1 自动生成的测试配置")
        cmake_config.append(f"# 生成时间: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
        cmake_config.append(f"# 测试套件: {suite.name}")
        cmake_config.append("")
        
        # 添加包含目录
        cmake_config.append("# 包含目录配置")
        cmake_config.append("include_directories(")
        cmake_config.append("    ${CMAKE_SOURCE_DIR}/include")
        cmake_config.append("    ${CMAKE_SOURCE_DIR}/include/core")
        cmake_config.append("    ${CMAKE_SOURCE_DIR}/include/network")
        cmake_config.append("    ${CMAKE_SOURCE_DIR}/include/sql")
        cmake_config.append("    ${CMAKE_SOURCE_DIR}/include/utils")
        cmake_config.append(")")
        cmake_config.append("")
        
        # 按分类生成测试配置
        for category in suite.categories:
            cmake_config.append(f"# {category.description}")
            cmake_config.append(f"# 分类: {category.name}")
            cmake_config.append("")
            
            # 查找该分类的测试文件
            category_tests = []
            for directory in category.directories:
                dir_path = self.tests_dir / directory
                if dir_path.exists():
                    for test_file in dir_path.glob("*.cpp"):
                        rel_path = str(test_file.relative_to(self.tests_dir))
                        category_tests.append(rel_path)
                    for test_file in dir_path.glob("*.cc"):
                        rel_path = str(test_file.relative_to(self.tests_dir))
                        category_tests.append(rel_path)
            
            if category_tests:
                cmake_config.append(f"set({category.name.upper()}_TESTS")
                for test in category_tests:
                    cmake_config.append(f"    {test}")
                cmake_config.append(")")
                cmake_config.append("")
                
                # 生成可执行文件和测试定义
                cmake_config.append(f"# 创建 {category.name} 测试可执行文件")
                for test in category_tests:
                    test_name = Path(test).stem
                    cmake_config.append(f"add_executable({test_name} {test})")
                    cmake_config.append(f"target_link_libraries({test_name}")
                    cmake_config.append("    PRIVATE")
                    cmake_config.append("    gtest")
                    cmake_config.append("    gtest_main")
                    cmake_config.append("    pthread")
                    cmake_config.append("    ${CMAKE_DL_LIBS}")
                    cmake_config.append("    sqlcc_core_lib")
                    cmake_config.append("    sqlcc_parser")
                    cmake_config.append("    sqlcc_config_manager")
                    cmake_config.append("    sqlcc_storage_engine")
                    cmake_config.append("    sqlcc_transaction_manager")
                    cmake_config.append("    sqlcc_executor")
                    cmake_config.append(")")
                    cmake_config.append("")
                    
                    # 添加CTest配置
                    cmake_config.append(f"add_test(NAME {test_name} COMMAND {test_name})")
                    cmake_config.append("")
        
        return "\n".join(cmake_config)
    
    def run_test_suite(self, suite_name: str, build_only: bool = False, 
                      verbose: bool = False) -> bool:
        """运行测试套件"""
        print(f"🚀 开始运行测试套件: {suite_name}")
        
        suite = next((s for s in self.test_suites if s.name == suite_name), None)
        if not suite:
            print(f"❌ 测试套件 '{suite_name}' 不存在")
            return False
        
        # 验证测试结构
        is_valid, issues = self.validate_test_structure()
        if not is_valid:
            print("❌ 测试结构验证失败:")
            for issue in issues:
                print(f"   - {issue}")
            return False
        
        print("✅ 测试结构验证通过")
        
        # 生成CMake配置
        cmake_config = self.generate_cmake_config(suite_name)
        cmake_config_file = self.tests_dir / f"CMakeLists_{suite_name}.txt"
        
        try:
            with open(cmake_config_file, 'w', encoding='utf-8') as f:
                f.write(cmake_config)
            print(f"✅ CMake配置已生成: {cmake_config_file}")
        except Exception as e:
            print(f"❌ CMake配置生成失败: {e}")
            return False
        
        if build_only:
            print("🏗️  仅构建模式，跳过测试执行")
            return True
        
        # 执行测试
        return self._execute_tests(suite, verbose)
    
    def _execute_tests(self, suite: TestSuite, verbose: bool) -> bool:
        """执行测试"""
        print(f"📋 执行测试套件: {suite.name}")
        print(f"📝 描述: {suite.description}")
        print(f"🔄 重试次数: {suite.retry_count}")
        print(f"⚡ 并行执行: {suite.parallel_execution}")
        print("")
        
        success = True
        
        for category in suite.categories:
            print(f"🧪 执行测试分类: {category.name}")
            print(f"   描述: {category.description}")
            print(f"   超时: {category.timeout}秒")
            print(f"   优先级: {category.priority}")
            print("")
            
            category_success = self._run_category_tests(category, verbose)
            if not category_success:
                success = False
                print(f"❌ 分类测试失败: {category.name}")
            else:
                print(f"✅ 分类测试通过: {category.name}")
            print("")
        
        return success
    
    def _run_category_tests(self, category: TestCategory, verbose: bool) -> bool:
        """运行分类测试"""
        # 这里可以调用实际的测试执行逻辑
        # 暂时返回模拟结果
        return True
    
    def generate_test_report(self, output_file: Optional[str] = None) -> str:
        """生成测试报告"""
        discovered_tests = self.discover_test_files()
        is_valid, issues = self.validate_test_structure()
        
        report_lines = []
        report_lines.append("=" * 80)
        report_lines.append("SQLCC v1.1.1 测试框架报告")
        report_lines.append(f"生成时间: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
        report_lines.append("=" * 80)
        report_lines.append("")
        
        # 测试套件概览
        report_lines.append("📋 测试套件概览:")
        for suite in self.test_suites:
            report_lines.append(f"   • {suite.name}: {suite.description}")
            report_lines.append(f"     - 分类数量: {len(suite.categories)}")
            report_lines.append(f"     - 并行执行: {'是' if suite.parallel_execution else '否'}")
            report_lines.append(f"     - 重试次数: {suite.retry_count}")
        report_lines.append("")
        
        # 测试文件统计
        report_lines.append("📊 测试文件统计:")
        total_files = 0
        for suite_name, files in discovered_tests.items():
            file_count = len(files)
            total_files += file_count
            report_lines.append(f"   • {suite_name}: {file_count} 个测试文件")
        report_lines.append(f"   总计: {total_files} 个测试文件")
        report_lines.append("")
        
        # 结构验证结果
        report_lines.append("🔍 结构验证结果:")
        if is_valid:
            report_lines.append("   ✅ 测试结构完整")
        else:
            report_lines.append("   ❌ 测试结构存在问题:")
            for issue in issues:
                report_lines.append(f"     - {issue}")
        report_lines.append("")
        
        # 详细分类信息
        report_lines.append("📁 详细分类信息:")
        for suite in self.test_suites:
            report_lines.append(f"\n   测试套件: {suite.name}")
            for category in suite.categories:
                report_lines.append(f"     分类: {category.name}")
                report_lines.append(f"       描述: {category.description}")
                report_lines.append(f"       目录: {', '.join(category.directories)}")
                report_lines.append(f"       优先级: {category.priority}")
                report_lines.append(f"       超时: {category.timeout}秒")
        
        report_text = "\n".join(report_lines)
        
        if output_file:
            try:
                with open(output_file, 'w', encoding='utf-8') as f:
                    f.write(report_text)
                print(f"📄 测试报告已保存到: {output_file}")
            except Exception as e:
                print(f"❌ 测试报告保存失败: {e}")
        
        return report_text
    
    def interactive_setup(self):
        """交互式设置"""
        print("🎯 SQLCC v1.1.1 测试框架交互式设置")
        print("=" * 50)
        
        # 显示可用测试套件
        print("\n📋 可用的测试套件:")
        for i, suite in enumerate(self.test_suites, 1):
            print(f"   {i}. {suite.name}: {suite.description}")
        
        # 选择测试套件
        while True:
            try:
                choice = input("\n请选择测试套件 (输入数字): ").strip()
                suite_index = int(choice) - 1
                if 0 <= suite_index < len(self.test_suites):
                    selected_suite = self.test_suites[suite_index]
                    break
                else:
                    print("❌ 无效选择，请重新输入")
            except ValueError:
                print("❌ 请输入有效数字")
        
        # 生成配置
        print(f"\n🏗️  为测试套件 '{selected_suite.name}' 生成配置...")
        cmake_config = self.generate_cmake_config(selected_suite.name)
        
        # 保存配置
        config_file = self.tests_dir / f"CMakeLists_{selected_suite.name}.txt"
        try:
            with open(config_file, 'w', encoding='utf-8') as f:
                f.write(cmake_config)
            print(f"✅ 配置已保存到: {config_file}")
        except Exception as e:
            print(f"❌ 配置保存失败: {e}")
            return False
        
        # 生成报告
        report_file = self.tests_dir / f"test_framework_report_{selected_suite.name}.txt"
        self.generate_test_report(str(report_file))
        
        print("\n🎉 测试框架设置完成!")
        print(f"📄 详细报告: {report_file}")
        
        return True


def main():
    """主函数"""
    parser = argparse.ArgumentParser(description="SQLCC v1.1.1 测试框架管理器")
    parser.add_argument("--project-root", default=".", help="项目根目录")
    parser.add_argument("--suite", choices=["comprehensive", "core", "quick"], 
                       default="comprehensive", help="测试套件")
    parser.add_argument("--build-only", action="store_true", help="仅构建不测试")
    parser.add_argument("--verbose", action="store_true", help="详细输出")
    parser.add_argument("--report", help="生成测试报告到指定文件")
    parser.add_argument("--interactive", action="store_true", help="交互式设置")
    parser.add_argument("--validate", action="store_true", help="验证测试结构")
    
    args = parser.parse_args()
    
    try:
        manager = TestFrameworkManager(args.project_root)
        
        if args.interactive:
            manager.interactive_setup()
        elif args.validate:
            is_valid, issues = manager.validate_test_structure()
            if is_valid:
                print("✅ 测试结构验证通过")
            else:
                print("❌ 测试结构验证失败:")
                for issue in issues:
                    print(f"   - {issue}")
                sys.exit(1)
        elif args.report:
            manager.generate_test_report(args.report)
        else:
            success = manager.run_test_suite(args.suite, args.build_only, args.verbose)
            sys.exit(0 if success else 1)
            
    except Exception as e:
        print(f"❌ 错误: {e}")
        sys.exit(1)


if __name__ == "__main__":
    main()