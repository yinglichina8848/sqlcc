#!/usr/bin/env python3
"""
SQLCC现代化测试框架执行器

采用类似Maven的生命周期管理，支持声明式配置和插件扩展。
每个测试只写一次编译定义，通过依赖直接集成。
"""

import os
import sys
import yaml
import json
import time
import subprocess
import threading
from pathlib import Path
from typing import Dict, List, Optional, Any, Callable
from dataclasses import dataclass, field
from enum import Enum
from concurrent.futures import ThreadPoolExecutor, as_completed

class TestType(Enum):
    """测试类型枚举"""
    UNIT = "unit"
    INTEGRATION = "integration"
    PERFORMANCE = "performance"
    SECURITY = "security"

class TestStatus(Enum):
    """测试状态枚举"""
    PENDING = "pending"
    RUNNING = "running"
    PASSED = "passed"
    FAILED = "failed"
    SKIPPED = "skipped"
    TIMEOUT = "timeout"

@dataclass
class TestResult:
    """测试结果数据类"""
    test_name: str
    component: str
    test_type: TestType
    status: TestStatus
    duration: float
    error_message: str = ""
    stdout: str = ""
    stderr: str = ""

@dataclass
class ComponentConfig:
    """组件配置数据类"""
    name: str
    description: str
    source_dir: Path
    test_dir: Path
    dependencies: List[str]
    tests: Dict[str, Any]
    coverage: Dict[str, Any]
    resources: Dict[str, Any]

@dataclass
class TestSuiteConfig:
    """测试套件配置数据类"""
    name: str
    description: str
    components: List[str]
    test_types: Dict[str, bool]
    timeout_multiplier: float
    parallel_execution: bool
    coverage_thresholds: Dict[str, float]

class TestLifecycle:
    """测试生命周期管理类"""
    
    def __init__(self, project_root: Path):
        self.project_root = project_root
        self.config = self._load_config()
        self.components = self._load_components()
        self.test_suites = self._load_test_suites()
        
        # 插件系统
        self.plugins = self._load_plugins()
        
        # 执行统计
        self.execution_stats = {
            'total_tests': 0,
            'passed_tests': 0,
            'failed_tests': 0,
            'skipped_tests': 0,
            'total_duration': 0.0
        }
    
    def _load_config(self) -> Dict:
        """加载主配置文件"""
        config_path = self.project_root / "config" / "test_framework.yaml"
        
        if not config_path.exists():
            raise FileNotFoundError(f"配置文件不存在: {config_path}")
        
        with open(config_path, 'r') as f:
            return yaml.safe_load(f)
    
    def _load_components(self) -> Dict[str, ComponentConfig]:
        """加载组件配置"""
        components = {}
        
        for comp_name, comp_config in self.config['components'].items():
            components[comp_name] = ComponentConfig(
                name=comp_name,
                description=comp_config['description'],
                source_dir=self.project_root / comp_config['source_dir'],
                test_dir=self.project_root / comp_config['test_dir'],
                dependencies=comp_config.get('dependencies', []),
                tests=comp_config.get('tests', {}),
                coverage=comp_config.get('coverage', {}),
                resources=comp_config.get('resources', {})
            )
        
        return components
    
    def _load_test_suites(self) -> Dict[str, TestSuiteConfig]:
        """加载测试套件配置"""
        suites = {}
        
        for suite_name, suite_config in self.config['test_suites'].items():
            suites[suite_name] = TestSuiteConfig(
                name=suite_name,
                description=suite_config['description'],
                components=suite_config['components'] if suite_config['components'] != 'all' else list(self.components.keys()),
                test_types=suite_config['test_types'],
                timeout_multiplier=suite_config['timeout_multiplier'],
                parallel_execution=suite_config['parallel_execution'],
                coverage_thresholds=suite_config['coverage_thresholds']
            )
        
        return suites
    
    def _load_plugins(self) -> Dict[str, Any]:
        """加载插件系统"""
        # 简化实现，实际应该动态加载插件
        return {
            'coverage': CoveragePlugin(self.project_root),
            'reporting': ReportingPlugin(self.project_root),
            'static_analysis': StaticAnalysisPlugin(self.project_root)
        }
    
    def execute_suite(self, suite_name: str, dry_run: bool = False) -> bool:
        """执行测试套件"""
        if suite_name not in self.test_suites:
            print(f"❌ 未知测试套件: {suite_name}")
            return False
        
        suite = self.test_suites[suite_name]
        print(f"🚀 执行测试套件: {suite_name}")
        print(f"📋 描述: {suite.description}")
        print(f"🔧 组件: {', '.join(suite.components)}")
        
        # 执行前置生命周期
        self._execute_lifecycle('pre_test', suite)
        
        # 执行测试
        results = self._execute_tests(suite, dry_run)
        
        # 执行后置生命周期
        self._execute_lifecycle('post_test', suite, results)
        
        # 生成报告
        self._generate_reports(suite, results)
        
        # 检查质量门限
        success = self._check_quality_gates(suite, results)
        
        return success
    
    def _execute_lifecycle(self, phase: str, suite: TestSuiteConfig, results: List[TestResult] = None):
        """执行生命周期阶段"""
        print(f"🔧 执行生命周期阶段: {phase}")
        
        if phase == 'pre_test':
            # 环境准备
            self._prepare_environment()
            
            # 构建测试目标
            self._build_test_targets(suite)
            
            # 插件前置处理
            for plugin_name, plugin in self.plugins.items():
                plugin.pre_test(suite)
        
        elif phase == 'post_test' and results is not None:
            # 插件后置处理
            for plugin_name, plugin in self.plugins.items():
                plugin.post_test(suite, results)
            
            # 清理环境
            self._cleanup_environment()
    
    def _prepare_environment(self):
        """准备测试环境"""
        print("🔧 准备测试环境")
        
        env_config = self.config.get('environment', {})
        
        # 创建临时目录
        temp_dir = Path(env_config.get('runtime', {}).get('database', {}).get('temp_dir', '/tmp/sqlcc_tests'))
        temp_dir.mkdir(parents=True, exist_ok=True)
        
        # 设置环境变量
        os.environ['SQLCC_TEST_TEMP_DIR'] = str(temp_dir)
        
        # 其他环境准备...
    
    def _build_test_targets(self, suite: TestSuiteConfig):
        """构建测试目标"""
        print("🔨 构建测试目标")
        
        for component_name in suite.components:
            if component_name not in self.components:
                print(f"⚠️  跳过未知组件: {component_name}")
                continue
            
            component = self.components[component_name]
            
            # 使用Bazel构建测试目标
            build_targets = []
            
            for test_type, enabled in suite.test_types.items():
                if enabled:
                    # 构建该类型的所有测试
                    build_targets.append(f"//{component.test_dir.relative_to(self.project_root)}:{test_type}_tests")
            
            if build_targets:
                try:
                    cmd = ["bazel", "build"] + build_targets
                    result = subprocess.run(cmd, cwd=self.project_root, capture_output=True, text=True)
                    
                    if result.returncode == 0:
                        print(f"✅ 构建成功: {component_name}")
                    else:
                        print(f"❌ 构建失败: {component_name}")
                        print(result.stderr)
                        
                except Exception as e:
                    print(f"❌ 构建异常: {component_name} - {e}")
    
    def _execute_tests(self, suite: TestSuiteConfig, dry_run: bool) -> List[TestResult]:
        """执行测试"""
        print("🧪 执行测试")
        
        all_results = []
        
        if suite.parallel_execution:
            # 并行执行
            all_results = self._execute_tests_parallel(suite, dry_run)
        else:
            # 串行执行
            all_results = self._execute_tests_serial(suite, dry_run)
        
        return all_results
    
    def _execute_tests_serial(self, suite: TestSuiteConfig, dry_run: bool) -> List[TestResult]:
        """串行执行测试"""
        results = []
        
        for component_name in suite.components:
            if component_name not in self.components:
                continue
            
            component = self.components[component_name]
            
            for test_type, enabled in suite.test_types.items():
                if not enabled:
                    continue
                
                # 执行该组件的该类型测试
                component_results = self._execute_component_tests(component, test_type, suite, dry_run)
                results.extend(component_results)
        
        return results
    
    def _execute_tests_parallel(self, suite: TestSuiteConfig, dry_run: bool) -> List[TestResult]:
        """并行执行测试"""
        results = []
        execution_config = self.config.get('execution', {})
        max_workers = execution_config.get('parallel', {}).get('max_workers', 4)
        
        with ThreadPoolExecutor(max_workers=max_workers) as executor:
            # 提交所有测试任务
            future_to_test = {}
            
            for component_name in suite.components:
                if component_name not in self.components:
                    continue
                
                component = self.components[component_name]
                
                for test_type, enabled in suite.test_types.items():
                    if not enabled:
                        continue
                    
                    # 提交测试执行任务
                    future = executor.submit(
                        self._execute_component_tests, 
                        component, test_type, suite, dry_run
                    )
                    future_to_test[future] = (component_name, test_type)
            
            # 收集结果
            for future in as_completed(future_to_test):
                component_name, test_type = future_to_test[future]
                
                try:
                    component_results = future.result()
                    results.extend(component_results)
                    print(f"✅ 完成并行测试: {component_name}.{test_type}")
                    
                except Exception as e:
                    print(f"❌ 并行测试失败: {component_name}.{test_type} - {e}")
        
        return results
    
    def _execute_component_tests(self, component: ComponentConfig, test_type: str, 
                                suite: TestSuiteConfig, dry_run: bool) -> List[TestResult]:
        """执行组件测试"""
        results = []
        
        # 构建测试目标
        test_target = f"//{component.test_dir.relative_to(self.project_root)}:{test_type}_tests"
        
        if dry_run:
            print(f"[DRY RUN] 将执行测试: {test_target}")
            
            # 模拟测试结果
            mock_result = TestResult(
                test_name=f"mock_{test_type}_test",
                component=component.name,
                test_type=TestType(test_type.upper()),
                status=TestStatus.PASSED,
                duration=0.1
            )
            results.append(mock_result)
            
            return results
        
        # 计算超时时间
        timeout_config = self.config.get('execution', {}).get('timeout', {})
        base_timeout = timeout_config.get('default_seconds', 300)
        multiplier = timeout_config.get('multipliers', {}).get(test_type, 1.0)
        suite_multiplier = suite.timeout_multiplier
        
        timeout_seconds = int(base_timeout * multiplier * suite_multiplier)
        
        try:
            # 执行测试
            cmd = [
                "bazel", "test", test_target,
                "--test_output=all",
                "--test_timeout", str(timeout_seconds)
            ]
            
            start_time = time.time()
            result = subprocess.run(cmd, cwd=self.project_root, capture_output=True, text=True, timeout=timeout_seconds)
            duration = time.time() - start_time
            
            # 解析测试结果
            test_result = TestResult(
                test_name=test_target,
                component=component.name,
                test_type=TestType(test_type.upper()),
                status=TestStatus.PASSED if result.returncode == 0 else TestStatus.FAILED,
                duration=duration,
                stdout=result.stdout,
                stderr=result.stderr
            )
            
            results.append(test_result)
            
            # 更新统计信息
            self._update_stats(test_result)
            
        except subprocess.TimeoutExpired:
            # 超时处理
            timeout_result = TestResult(
                test_name=test_target,
                component=component.name,
                test_type=TestType(test_type.upper()),
                status=TestStatus.TIMEOUT,
                duration=timeout_seconds,
                error_message=f"测试超时 ({timeout_seconds}秒)"
            )
            results.append(timeout_result)
            self._update_stats(timeout_result)
            
        except Exception as e:
            # 异常处理
            error_result = TestResult(
                test_name=test_target,
                component=component.name,
                test_type=TestType(test_type.upper()),
                status=TestStatus.FAILED,
                duration=0.0,
                error_message=str(e)
            )
            results.append(error_result)
            self._update_stats(error_result)
        
        return results
    
    def _update_stats(self, result: TestResult):
        """更新执行统计"""
        self.execution_stats['total_tests'] += 1
        self.execution_stats['total_duration'] += result.duration
        
        if result.status == TestStatus.PASSED:
            self.execution_stats['passed_tests'] += 1
        elif result.status == TestStatus.FAILED or result.status == TestStatus.TIMEOUT:
            self.execution_stats['failed_tests'] += 1
        elif result.status == TestStatus.SKIPPED:
            self.execution_stats['skipped_tests'] += 1
    
    def _generate_reports(self, suite: TestSuiteConfig, results: List[TestResult]):
        """生成测试报告"""
        print("📊 生成测试报告")
        
        # 调用报告插件
        if 'reporting' in self.plugins:
            self.plugins['reporting'].generate_report(suite, results, self.execution_stats)
        
        # 控制台报告
        self._print_console_report(results)
    
    def _print_console_report(self, results: List[TestResult]):
        """打印控制台报告"""
        print("\n" + "="*60)
        print("📋 测试执行报告")
        print("="*60)
        
        # 按状态分组
        passed = [r for r in results if r.status == TestStatus.PASSED]
        failed = [r for r in results if r.status == TestStatus.FAILED]
        timeout = [r for r in results if r.status == TestStatus.TIMEOUT]
        
        print(f"总测试数: {self.execution_stats['total_tests']}")
        print(f"通过: {len(passed)}")
        print(f"失败: {len(failed)}")
        print(f"超时: {len(timeout)}")
        print(f"总耗时: {self.execution_stats['total_duration']:.2f}秒")
        
        if failed or timeout:
            print("\n❌ 失败的测试:")
            for result in failed + timeout:
                print(f"  - {result.component}.{result.test_type}: {result.error_message}")
        
        print("="*60)
    
    def _check_quality_gates(self, suite: TestSuiteConfig, results: List[TestResult]) -> bool:
        """检查质量门限"""
        print("🔍 检查质量门限")
        
        # 检查测试通过率
        total_tests = self.execution_stats['total_tests']
        passed_tests = self.execution_stats['passed_tests']
        
        if total_tests == 0:
            print("⚠️  没有执行任何测试")
            return False
        
        success_rate = passed_tests / total_tests
        
        # 简单的通过率检查（可以扩展为更复杂的质量门限）
        min_success_rate = 0.95  # 95%的最低通过率
        
        if success_rate < min_success_rate:
            print(f"❌ 质量门限检查失败: 通过率 {success_rate:.1%} < {min_success_rate:.1%}")
            return False
        
        print(f"✅ 质量门限检查通过: 通过率 {success_rate:.1%}")
        return True
    
    def _cleanup_environment(self):
        """清理测试环境"""
        print("🧹 清理测试环境")
        
        # 清理临时文件
        temp_dir = os.environ.get('SQLCC_TEST_TEMP_DIR')
        if temp_dir and Path(temp_dir).exists():
            import shutil
            shutil.rmtree(temp_dir, ignore_errors=True)

# 插件基类
class BasePlugin:
    """插件基类"""
    
    def __init__(self, project_root: Path):
        self.project_root = project_root
    
    def pre_test(self, suite: TestSuiteConfig):
        """测试前置处理"""
        pass
    
    def post_test(self, suite: TestSuiteConfig, results: List[TestResult]):
        """测试后置处理"""
        pass

class CoveragePlugin(BasePlugin):
    """覆盖率插件"""
    
    def pre_test(self, suite: TestSuiteConfig):
        """启用覆盖率编译"""
        if any(suite.coverage_thresholds.values()):  # 如果有覆盖率要求
            print("📊 启用覆盖率编译")
            # 设置覆盖率编译标志
            os.environ['COVERAGE_ENABLED'] = 'true'
    
    def post_test(self, suite: TestSuiteConfig, results: List[TestResult]):
        """生成覆盖率报告"""
        if any(suite.coverage_thresholds.values()):
            print("📊 生成覆盖率报告")
            
            # 使用gcovr生成报告
            try:
                cmd = [
                    "gcovr",
                    "--root", str(self.project_root),
                    "--exclude", "tests/.*",
                    "--html", "--html-detail",
                    "--output", "coverage_report.html"
                ]
                
                subprocess.run(cmd, cwd=self.project_root, capture_output=True)
                print("✅ 覆盖率报告生成完成")
                
            except Exception as e:
                print(f"❌ 覆盖率报告生成失败: {e}")

class ReportingPlugin(BasePlugin):
    """报告插件"""
    
    def generate_report(self, suite: TestSuiteConfig, results: List[TestResult], stats: Dict):
        """生成详细报告"""
        report = {
            'suite': suite.name,
            'timestamp': time.time(),
            'stats': stats,
            'results': [
                {
                    'test_name': r.test_name,
                    'component': r.component,
                    'test_type': r.test_type.value,
                    'status': r.status.value,
                    'duration': r.duration,
                    'error_message': r.error_message
                }
                for r in results
            ]
        }
        
        # 保存JSON报告
        report_path = self.project_root / "test_report.json"
        with open(report_path, 'w') as f:
            json.dump(report, f, indent=2)
        
        print(f"📄 详细报告已保存: {report_path}")

class StaticAnalysisPlugin(BasePlugin):
    """静态分析插件"""
    
    def pre_test(self, suite: TestSuiteConfig):
        """运行静态分析"""
        print("🔍 运行静态分析")
        
        # 简化实现，实际应该调用cppcheck/clang-tidy等工具
        try:
            # 示例：运行基本的代码检查
            cmd = ["find", str(self.project_root / "src"), "-name", "*.cpp", "-exec", "grep", "-n", "TODO", "{}", ";"]
            result = subprocess.run(cmd, capture_output=True, text=True)
            
            if result.stdout:
                print("⚠️  发现TODO注释:")
                print(result.stdout)
            
        except Exception as e:
            print(f"❌ 静态分析失败: {e}")

def main():
    """主函数"""
    import argparse
    
    parser = argparse.ArgumentParser(description="SQLCC现代化测试框架执行器")
    parser.add_argument("suite", help="要执行的测试套件名称")
    parser.add_argument("--dry-run", action="store_true", 
                       help="干运行模式，不实际执行测试")
    parser.add_argument("--project-root", default=".",
                       help="项目根目录路径")
    
    args = parser.parse_args()
    
    project_root = Path(args.project_root).resolve()
    
    if not project_root.exists():
        print(f"❌ 项目根目录不存在: {project_root}")
        sys.exit(1)
    
    try:
        # 创建测试生命周期管理器
        lifecycle = TestLifecycle(project_root)
        
        # 执行测试套件
        success = lifecycle.execute_suite(args.suite, args.dry_run)
        
        if success:
            print(f"\n🎉 测试套件执行{'模拟' if args.dry_run else ''}成功: {args.suite}")
            sys.exit(0)
        else:
            print(f"\n💥 测试套件执行失败: {args.suite}")
            sys.exit(1)
            
    except Exception as e:
        print(f"💥 测试框架执行异常: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)

if __name__ == "__main__":
    main()