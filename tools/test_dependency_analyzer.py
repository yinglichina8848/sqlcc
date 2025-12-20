#!/usr/bin/env python3
"""
测试代码依赖分析工具
用于检测测试代码中的include和包依赖问题，生成系统性的重构计划
"""

import os
import re
import json
import subprocess
from pathlib import Path
from typing import Dict, List, Set, Tuple, Optional
from dataclasses import dataclass, asdict
import argparse

@dataclass
class IncludeIssue:
    """Include问题"""
    file_path: str
    line_number: int
    include_path: str
    issue_type: str  # 'missing', 'incorrect', 'deprecated'
    suggested_fix: str

@dataclass
class DependencyIssue:
    """依赖问题"""
    test_target: str
    current_deps: List[str]
    missing_deps: List[str]
    unused_deps: List[str]
    circular_deps: List[str]

@dataclass
class TestAnalysisResult:
    """测试分析结果"""
    test_file: str
    include_issues: List[IncludeIssue]
    dependency_issues: Optional[DependencyIssue]
    build_status: str  # 'success', 'failure', 'not_tested'
    error_message: Optional[str] = None

class TestDependencyAnalyzer:
    """测试依赖分析器"""

    def __init__(self, project_root: str):
        self.project_root = Path(project_root)
        self.include_patterns = {
            # 标准库头文件
            'gtest': ['gtest/gtest.h', 'gmock/gmock.h'],
            'memory': ['memory', 'unique_ptr', 'shared_ptr'],
            'vector': ['vector', 'array', 'deque'],
            'string': ['string', 'cstring'],
            'iostream': ['iostream', 'fstream', 'sstream'],
            'algorithm': ['algorithm', 'functional'],
            'mutex': ['mutex', 'thread', 'atomic'],
            'variant': ['variant', 'optional', 'any'],

            # 项目头文件模式
            'sql_parser': [
                'sql_parser/lexer.h', 'sql_parser/parser.h', 'sql_parser/ast_node.h',
                'sql_parser/token.h', 'sql_parser/constraint.h', 'sql_parser/function_ast.h'
            ],
            'storage': [
                'storage/buffer_pool.h', 'storage/b_plus_tree.h', 'storage/storage_engine.h',
                'storage/index_manager.h', 'storage/disk_manager.h'
            ],
            'core': [
                'core/user_manager.h', 'core/config_manager.h', 'core/database_manager.h',
                'core/sql_executor.h', 'core/permission_validator.h'
            ],
            'utils': [
                'utils/logger.h', 'utils/config_manager.h', 'utils/smart_config_manager.h',
                'utils/data_type.h', 'utils/version.h'
            ],
            'network': [
                'network/network.h', 'network/connection_state.h', 'network/encryption.h'
            ],
            'execution': [
                'execution/join_executor.h', 'execution/function_executor.h',
                'execution/window_function_executor.h', 'execution/load_data_executor.h'
            ]
        }

        self.deprecated_includes = {
            'sql_parser/lexer_new.h': 'sql_parser/lexer.h',
            'sql_parser/parser_new.h': 'sql_parser/parser.h',
            'storage/buffer_pool_v2.h': 'storage/buffer_pool.h',
            'storage/buffer_pool_fixed.h': 'storage/buffer_pool.h',
        }

    def analyze_test_file(self, test_file: Path) -> TestAnalysisResult:
        """分析单个测试文件"""
        result = TestAnalysisResult(
            test_file=str(test_file.relative_to(self.project_root)),
            include_issues=[],
            dependency_issues=None,
            build_status='not_tested'
        )

        try:
            # 读取文件内容
            content = test_file.read_text(encoding='utf-8')

            # 分析include语句
            result.include_issues = self._analyze_includes(content, test_file)

            # 分析依赖关系
            result.dependency_issues = self._analyze_dependencies(test_file)

            # 测试构建状态
            result.build_status = self._test_build_status(test_file)

        except Exception as e:
            result.error_message = str(e)
            result.build_status = 'error'

        return result

    def _analyze_includes(self, content: str, file_path: Path) -> List[IncludeIssue]:
        """分析include语句"""
        issues = []
        lines = content.split('\n')

        for i, line in enumerate(lines, 1):
            line = line.strip()
            if not line.startswith('#include'):
                continue

            # 提取include路径
            match = re.match(r'#include\s*[<"]([^>"]+)[>"]', line)
            if not match:
                continue

            include_path = match.group(1)

            # 检查include问题
            issue = self._check_include_issue(include_path, file_path, i)
            if issue:
                issues.append(issue)

        return issues

    def _check_include_issue(self, include_path: str, file_path: Path, line_number: int) -> Optional[IncludeIssue]:
        """检查单个include是否有问题"""

        # 检查是否为弃用的include
        if include_path in self.deprecated_includes:
            return IncludeIssue(
                file_path=str(file_path),
                line_number=line_number,
                include_path=include_path,
                issue_type='deprecated',
                suggested_fix=self.deprecated_includes[include_path]
            )

        # 检查include路径是否正确
        if not self._is_valid_include_path(include_path):
            return IncludeIssue(
                file_path=str(file_path),
                line_number=line_number,
                include_path=include_path,
                issue_type='incorrect',
                suggested_fix=self._suggest_correct_include(include_path, file_path)
            )

        # 检查头文件是否存在
        if not self._include_file_exists(include_path):
            return IncludeIssue(
                file_path=str(file_path),
                line_number=line_number,
                include_path=include_path,
                issue_type='missing',
                suggested_fix=self._suggest_missing_include(include_path, file_path)
            )

        return None

    def _is_valid_include_path(self, include_path: str) -> bool:
        """检查include路径是否有效"""
        # 标准库头文件
        if not include_path.startswith(('sqlcc/', 'gtest/', 'gmock/')):
            return True

        # 项目头文件应该有正确的命名空间前缀
        expected_patterns = [
            r'^sql_parser/',
            r'^storage/',
            r'^core/',
            r'^utils/',
            r'^network/',
            r'^execution/',
            r'^transaction/',
            r'^types/',
            r'^procedure/',
            r'^trigger/'
        ]

        return any(re.match(pattern, include_path) for pattern in expected_patterns)

    def _include_file_exists(self, include_path: str) -> bool:
        """检查include文件是否存在"""
        # 对于标准库头文件，假设存在
        if not include_path.startswith(('sqlcc/', 'gtest/', 'gmock/')):
            return True

        # 检查项目头文件
        include_file = self.project_root / 'include' / include_path
        return include_file.exists()

    def _suggest_correct_include(self, include_path: str, file_path: Path) -> str:
        """建议正确的include路径"""
        # 移除不正确的路径前缀
        if include_path.startswith('sqlcc/'):
            include_path = include_path[6:]

        # 添加正确的include目录前缀（如果需要）
        if not include_path.startswith(('sql_parser/', 'storage/', 'core/', 'utils/',
                                     'network/', 'execution/', 'transaction/', 'types/',
                                     'procedure/', 'trigger/')):
            # 尝试推断正确的模块
            if 'lexer' in include_path or 'parser' in include_path or 'token' in include_path:
                return f'sql_parser/{include_path}'
            elif 'buffer' in include_path or 'tree' in include_path or 'storage' in include_path:
                return f'storage/{include_path}'
            elif 'config' in include_path or 'logger' in include_path:
                return f'utils/{include_path}'

        return include_path

    def _suggest_missing_include(self, include_path: str, file_path: Path) -> str:
        """建议缺失的include文件"""
        # 查找相似的文件
        include_dir = self.project_root / 'include'
        if include_dir.exists():
            for root, dirs, files in os.walk(include_dir):
                for file in files:
                    if file.endswith('.h') and include_path.split('/')[-1] in file:
                        rel_path = Path(root).relative_to(include_dir) / file
                        return str(rel_path)

        return f"创建文件: {include_path}"

    def _analyze_dependencies(self, test_file: Path) -> Optional[DependencyIssue]:
        """分析测试文件的依赖关系"""
        # 查找对应的BUILD文件
        build_file = self._find_build_file(test_file)
        if not build_file:
            return None

        try:
            content = build_file.read_text()
            current_deps = self._extract_deps_from_build(content)
            required_deps = self._infer_required_deps(test_file)

            missing_deps = [dep for dep in required_deps if dep not in current_deps]
            unused_deps = [dep for dep in current_deps if dep not in required_deps]

            return DependencyIssue(
                test_target=self._get_test_target_name(test_file),
                current_deps=current_deps,
                missing_deps=missing_deps,
                unused_deps=unused_deps,
                circular_deps=[]  # 暂时不检测循环依赖
            )

        except Exception:
            return None

    def _find_build_file(self, test_file: Path) -> Optional[Path]:
        """查找对应的BUILD文件"""
        current_dir = test_file.parent

        while current_dir != self.project_root:
            build_file = current_dir / 'BUILD.bazel'
            if build_file.exists():
                return build_file
            current_dir = current_dir.parent

        return None

    def _extract_deps_from_build(self, content: str) -> List[str]:
        """从BUILD文件中提取依赖"""
        deps = []
        # 简单的正则表达式提取deps
        dep_matches = re.findall(r'deps\s*=\s*\[([^\]]+)\]', content, re.DOTALL)
        for match in dep_matches:
            # 提取引号内的依赖
            dep_items = re.findall(r'"([^"]+)"', match)
            deps.extend(dep_items)

        return deps

    def _infer_required_deps(self, test_file: Path) -> List[str]:
        """推断需要的依赖"""
        content = test_file.read_text()
        required_deps = []

        # 基于include语句推断依赖
        includes = re.findall(r'#include\s*[<"]([^>"]+)[>"]', content)

        dep_map = {
            'sql_parser': ['//src/sql_parser:sql_parser'],
            'storage': ['//src/storage_engine:storage_engine'],
            'core': ['//src/core:core'],
            'utils': ['//src/utils:utils'],
            'network': ['//src/network:network'],
            'execution': ['//src/execution:execution'],
            'gtest': ['@com_google_googletest//:gtest_main'],
            'gmock': ['@com_google_googletest//:gtest_main']
        }

        for include in includes:
            for module, deps in dep_map.items():
                if module in include:
                    required_deps.extend(deps)
                    break

        return list(set(required_deps))  # 去重

    def _get_test_target_name(self, test_file: Path) -> str:
        """获取测试目标名称"""
        return f"//{test_file.parent.relative_to(self.project_root)}:{test_file.stem}"

    def _test_build_status(self, test_file: Path) -> str:
        """测试构建状态"""
        try:
            # 简单的bazel build测试
            target_name = self._get_test_target_name(test_file)
            result = subprocess.run(
                ['bazel', 'build', target_name],
                cwd=self.project_root,
                capture_output=True,
                text=True,
                timeout=30
            )
            return 'success' if result.returncode == 0 else 'failure'
        except subprocess.TimeoutExpired:
            return 'timeout'
        except Exception:
            return 'error'

    def generate_refactor_plan(self, analysis_results: List[TestAnalysisResult]) -> Dict:
        """生成重构计划"""

        # 按优先级分类问题
        critical_issues = []
        major_issues = []
        minor_issues = []

        for result in analysis_results:
            if result.build_status == 'failure':
                critical_issues.append(result)
            elif result.include_issues or result.dependency_issues:
                major_issues.append(result)
            else:
                minor_issues.append(result)

        # 生成分阶段计划
        plan = {
            'summary': {
                'total_tests': len(analysis_results),
                'critical_issues': len(critical_issues),
                'major_issues': len(major_issues),
                'minor_issues': len(minor_issues)
            },
            'phases': [
                {
                    'name': '紧急修复阶段',
                    'duration': '1-2天',
                    'targets': [r.test_file for r in critical_issues],
                    'tasks': [
                        '修复构建失败的测试',
                        '解决缺失的依赖',
                        '修复关键的include路径错误'
                    ]
                },
                {
                    'name': '系统性重构阶段',
                    'duration': '3-5天',
                    'targets': [r.test_file for r in major_issues],
                    'tasks': [
                        '标准化include路径',
                        '优化BUILD.bazel依赖',
                        '移除弃用的include',
                        '统一代码风格'
                    ]
                },
                {
                    'name': '优化完善阶段',
                    'duration': '2-3天',
                    'targets': [r.test_file for r in minor_issues],
                    'tasks': [
                        '性能优化',
                        '代码清理',
                        '文档更新'
                    ]
                }
            ],
            'detailed_fixes': self._generate_detailed_fixes(analysis_results)
        }

        return plan

    def _generate_detailed_fixes(self, results: List[TestAnalysisResult]) -> List[Dict]:
        """生成详细的修复建议"""
        fixes = []

        for result in results:
            if not result.include_issues and not result.dependency_issues:
                continue

            fix = {
                'file': result.test_file,
                'build_status': result.build_status,
                'include_fixes': [],
                'dependency_fixes': {}
            }

            # Include修复
            for issue in result.include_issues:
                fix['include_fixes'].append({
                    'line': issue.line_number,
                    'original': issue.include_path,
                    'suggested': issue.suggested_fix,
                    'type': issue.issue_type
                })

            # 依赖修复
            if result.dependency_issues:
                fix['dependency_fixes'] = {
                    'missing': result.dependency_issues.missing_deps,
                    'unused': result.dependency_issues.unused_deps,
                    'target': result.dependency_issues.test_target
                }

            fixes.append(fix)

        return fixes

    def save_report(self, results: List[TestAnalysisResult], plan: Dict, output_file: str):
        """保存分析报告"""
        report = {
            'timestamp': str(self.project_root.stat().st_mtime),
            'analysis_results': [asdict(r) for r in results],
            'refactor_plan': plan
        }

        with open(output_file, 'w', encoding='utf-8') as f:
            json.dump(report, f, indent=2, ensure_ascii=False)

def main():
    parser = argparse.ArgumentParser(description='测试代码依赖分析工具')
    parser.add_argument('--project-root', default='.', help='项目根目录')
    parser.add_argument('--output', default='test_dependency_report.json', help='输出文件')
    parser.add_argument('--test-dir', default='tests', help='测试目录')

    args = parser.parse_args()

    analyzer = TestDependencyAnalyzer(args.project_root)
    test_dir = Path(args.project_root) / args.test_dir

    print(f"🔍 分析测试目录: {test_dir}")

    results = []
    if test_dir.exists():
        for test_file in test_dir.rglob('*.cpp'):
            if test_file.name.endswith('_test.cpp'):
                print(f"📋 分析文件: {test_file.relative_to(Path(args.project_root))}")
                result = analyzer.analyze_test_file(test_file)
                results.append(result)

    print(f"📊 分析完成，共处理 {len(results)} 个测试文件")

    # 生成重构计划
    plan = analyzer.generate_refactor_plan(results)
    print(f"📝 生成重构计划: {plan['summary']}")

    # 保存报告
    analyzer.save_report(results, plan, args.output)
    print(f"💾 报告已保存到: {args.output}")

    # 输出关键统计
    critical = plan['summary']['critical_issues']
    if critical > 0:
        print(f"🚨 发现 {critical} 个紧急问题需要立即修复")

if __name__ == '__main__':
    main()
