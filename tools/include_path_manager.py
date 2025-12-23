#!/usr/bin/env python3
"""
SQLCC Include路径管理器
用于规范化include路径，统一include依赖管理
"""

import os
import re
import json
from pathlib import Path
from typing import Dict, List, Set, Tuple, Optional, Pattern
from dataclasses import dataclass, asdict
from enum import Enum


@dataclass
class IncludePathMapping:
    """Include路径映射"""
    old_path: str
    new_path: str
    category: str  # 'relative', 'deprecated', 'incorrect', 'regex'
    reason: str
    is_regex: bool = False
    compiled_pattern: Optional[Pattern] = None


@dataclass
class IncludeIssue:
    """Include问题"""
    file_path: str
    line_number: int
    include_path: str
    issue_type: str
    suggested_fix: str
    severity: str  # 'high', 'medium', 'low'


class IncludeDependencyAnalyzer:
    """Include依赖分析器"""

    def __init__(self):
        self.dependency_graph = {}  # file -> set of included files
        self.reverse_graph = {}    # file -> set of files that include it
        self.cycles = []           # 检测到的循环依赖

    def add_dependency(self, from_file: str, to_file: str):
        """添加依赖关系"""
        if from_file not in self.dependency_graph:
            self.dependency_graph[from_file] = set()
        if to_file not in self.reverse_graph:
            self.reverse_graph[to_file] = set()

        self.dependency_graph[from_file].add(to_file)
        self.reverse_graph[to_file].add(from_file)

    def detect_cycles(self) -> List[List[str]]:
        """检测循环依赖"""
        cycles = []
        visited = set()
        rec_stack = set()

        def dfs(node, path):
            visited.add(node)
            rec_stack.add(node)
            path.append(node)

            for neighbor in self.dependency_graph.get(node, set()):
                if neighbor not in visited:
                    if dfs(neighbor, path):
                        return True
                elif neighbor in rec_stack:
                    # 找到循环
                    cycle_start = path.index(neighbor)
                    cycles.append(path[cycle_start:] + [neighbor])
                    return True

            path.pop()
            rec_stack.remove(node)
            return False

        for node in self.dependency_graph:
            if node not in visited:
                dfs(node, [])

        self.cycles = cycles
        return cycles

    def find_cycle_breaking_suggestions(self, cycles: List[List[str]]) -> Dict[str, List[str]]:
        """为循环依赖提供破环建议"""
        suggestions = {}

        for cycle in cycles:
            cycle_str = " -> ".join(cycle[:-1])  # 移除重复的最后一个元素

            # 分析循环中的每个文件，建议破环点
            breaking_points = []

            for i, file in enumerate(cycle[:-1]):  # 不处理最后一个重复元素
                next_file = cycle[(i + 1) % (len(cycle) - 1)]

                # 检查是否可以通过前向声明破环
                if self._can_break_with_forward_declaration(file, next_file):
                    breaking_points.append(f"文件 {file} 可以通过前向声明避免包含 {next_file}")

                # 检查是否可以提取接口
                if self._can_break_with_interface_extraction(file, next_file):
                    breaking_points.append(f"文件 {file} 和 {next_file} 可以考虑提取公共接口")

                # 检查是否可以反转依赖方向
                if self._can_break_with_dependency_inversion(file, next_file):
                    breaking_points.append(f"可以考虑反转 {file} 到 {next_file} 的依赖方向")

            suggestions[cycle_str] = breaking_points

        return suggestions

    def _can_break_with_forward_declaration(self, from_file: str, to_file: str) -> bool:
        """检查是否可以通过前向声明破环依赖"""
        try:
            # 读取文件内容
            from_path = self.project_root / from_file
            to_path = self.project_root / to_file

            if not from_path.exists() or not to_path.exists():
                return False

            from_content = from_path.read_text(encoding='utf-8')
            to_content = to_path.read_text(encoding='utf-8')

            # 检查to_file中是否只有类声明，没有实现
            # 简化检查：如果to_file主要是声明性的，可以考虑前向声明
            lines_of_code = len([line for line in to_content.split('\n') if line.strip() and not line.strip().startswith('//')])
            include_count = to_content.count('#include')

            # 如果文件较小且include不多，可能是声明性文件
            return lines_of_code < 100 and include_count < 5

        except Exception:
            return False

    def _can_break_with_interface_extraction(self, file1: str, file2: str) -> bool:
        """检查是否可以通过接口提取破环依赖"""
        try:
            # 检查两个文件是否有共同的依赖模式
            file1_path = self.project_root / file1
            file2_path = self.project_root / file2

            if not file1_path.exists() or not file2_path.exists():
                return False

            file1_content = file1_path.read_text(encoding='utf-8')
            file2_content = file2_path.read_text(encoding='utf-8')

            # 检查是否都有类似的类定义
            file1_classes = self._extract_class_names(file1_content)
            file2_classes = self._extract_class_names(file2_content)

            # 如果有相似的类命名模式，建议提取接口
            return bool(file1_classes and file2_classes and
                       any(cls1.split('_')[0] == cls2.split('_')[0] for cls1 in file1_classes for cls2 in file2_classes))

        except Exception:
            return False

    def _can_break_with_dependency_inversion(self, from_file: str, to_file: str) -> bool:
        """检查是否可以通过依赖反转破环依赖"""
        try:
            # 检查是否可以通过回调或观察者模式反转依赖
            from_path = self.project_root / from_file
            to_path = self.project_root / to_file

            if not from_path.exists() or not to_path.exists():
                return False

            from_content = from_path.read_text(encoding='utf-8')
            to_content = to_path.read_text(encoding='utf-8')

            # 检查from_file是否主要使用to_file的接口
            from_includes = len(re.findall(r'#include\s*[<"]([^>"]*{})[<"]'.format(re.escape(to_file.split('/')[-1].replace('.cpp', '.h'))), from_content))

            # 如果from_file只是为了调用to_file的方法，可以考虑依赖反转
            return from_includes > 0 and 'callback' in from_content.lower()

        except Exception:
            return False

    def _extract_class_names(self, content: str) -> List[str]:
        """从代码中提取类名"""
        class_pattern = r'class\s+(\w+)'
        matches = re.findall(class_pattern, content)
        return matches

    def get_include_depth(self, file_path: str) -> int:
        """计算include深度"""
        def calculate_depth(node, visited=None):
            if visited is None:
                visited = set()

            if node in visited:
                return 0  # 防止无限递归

            visited.add(node)
            max_depth = 0

            for dep in self.dependency_graph.get(node, set()):
                depth = calculate_depth(dep, visited.copy()) + 1
                max_depth = max(max_depth, depth)

            return max_depth

        return calculate_depth(file_path)


class IncludePathManager:
    """Include路径管理器"""

    def __init__(self, project_root: str):
        self.project_root = Path(project_root)
        self.include_dir = self.project_root / 'include'
        self.src_dir = self.project_root / 'src'

        # 路径映射规则
        self.path_mappings = self._load_path_mappings()

        # 模块映射 - 扩展版本
        self.module_mappings = self._load_module_mappings()

        # 依赖分析器
        self.dependency_analyzer = IncludeDependencyAnalyzer()

        # 缓存
        self._include_cache = {}
        self._analysis_cache = {}

        # 统计信息
        self.stats = {
            'files_analyzed': 0,
            'issues_found': 0,
            'cycles_detected': 0,
            'auto_fixes_applied': 0,
            'bazel_files_checked': 0,
            'bazel_issues_found': 0,
            'dependencies_analyzed': 0,
            'reports_generated': 0
        }

    def _load_module_mappings(self) -> Dict[str, str]:
        """加载模块映射配置"""
        mappings = {}

        # 核心模块映射
        core_mappings = {
            'database_manager.h': 'core/database_manager.h',
            'sql_executor.h': 'core/sql_executor.h',
            'user_manager.h': 'core/user_manager.h',
            'permission_validator.h': 'core/permission_validator.h',
            'error_handler.h': 'core/error_handler.h',
        }

        # 存储引擎模块映射
        storage_mappings = {
            'storage_engine.h': 'storage_engine.h',
            'buffer_pool.h': 'storage/buffer_pool.h',
            'b_plus_tree.h': 'storage/b_plus_tree.h',
            'index_manager.h': 'storage/index_manager.h',
            'disk_manager.h': 'storage/disk_manager.h',
            'table_storage.h': 'storage/table_storage.h',
            'wal_writer.h': 'storage/wal_writer.h',
            'wal_buffer.h': 'storage/wal_buffer.h',
            'checkpoint.h': 'storage/checkpoint.h',
        }

        # SQL解析器模块映射
        sql_parser_mappings = {
            'sql_parser.h': 'sql_parser/parser.h',
            'lexer.h': 'sql_parser/lexer.h',
            'parser.h': 'sql_parser/parser.h',
            'ast_node.h': 'sql_parser/ast_node.h',
            'token.h': 'sql_parser/token.h',
            'constraint.h': 'sql_parser/constraint.h',
            'function_ast.h': 'sql_parser/function_ast.h',
            'set_operation.h': 'sql_parser/set_operation.h',
            'window_function.h': 'sql_parser/window_function.h',
            'recursive_query.h': 'sql_parser/recursive_query.h',
            'json.h': 'sql_parser/json.h',
            'datetime.h': 'sql_parser/datetime.h',
            'decimal.h': 'sql_parser/decimal.h',
        }

        # 工具模块映射
        utils_mappings = {
            'config_manager.h': 'utils/config_manager.h',
            'logger.h': 'utils/logger.h',
            'smart_config_manager.h': 'utils/smart_config_manager.h',
            'config_snapshot.h': 'utils/config_snapshot.h',
            'config_lifecycle.h': 'utils/config_lifecycle.h',
        }

        # 网络模块映射
        network_mappings = {
            'network.h': 'network/network.h',
            'connection_state.h': 'network/connection_state.h',
            'encryption.h': 'network/encryption.h',
            'session.h': 'network/session.h',
            'session_manager.h': 'network/session_manager.h',
            'client_connection.h': 'network/client_connection.h',
            'client_network_manager.h': 'network/client_network_manager.h',
            'server_network_manager.h': 'network/server_network_manager.h',
            'connection_handler.h': 'network/connection_handler.h',
            'connection_state_machine.h': 'network/connection_state_machine.h',
            'network_exception.h': 'network/network_exception.h',
            'message_processor.h': 'network/message_processor.h',
            'key_rotation_policy.h': 'network/key_rotation_policy.h',
            'network_monitor.h': 'network/network_monitor.h',
            'network_stability_guard.h': 'network/network_stability_guard.h',
            'data_transmission_validator.h': 'network/data_transmission_validator.h',
            'mysql_protocol.h': 'network/mysql_protocol.h',
        }

        # 执行引擎模块映射
        execution_mappings = {
            'join_executor.h': 'execution/join_executor.h',
            'function_executor.h': 'execution/function_executor.h',
            'window_function_executor.h': 'execution/window_function_executor.h',
            'load_data_executor.h': 'execution/load_data_executor.h',
            'set_operation_executor.h': 'execution/set_operation_executor.h',
            'recursive_query_executor.h': 'execution/recursive_query_executor.h',
            'dml_execution_strategy.h': 'execution/dml_execution_strategy.h',
            'execution_strategy.h': 'execution/execution_strategy.h',
            'aggregate_engine.h': 'execution/aggregate_engine.h',
            'group_by_executor.h': 'execution/group_by_executor.h',
            'unified_executor.h': 'execution/unified_executor.h',
            'ddl_execution_strategy.h': 'execution/ddl_execution_strategy.h',
            'task_executor.h': 'execution/task_executor.h',
            'comprehensive_task_executor.h': 'execution/comprehensive_task_executor.h',
            'test_runner.h': 'execution/test_runner.h',
            'standalone_test.h': 'execution/standalone_test.h',
        }

        # 触发器模块映射
        trigger_mappings = {
            'trigger_manager.h': 'trigger/trigger_manager.h',
            'sql_trigger_executor.h': 'trigger/sql_trigger_executor.h',
            'trigger_definition.h': 'trigger/trigger_definition.h',
            'trigger_executor.h': 'trigger/trigger_executor.h',
            'recursion_guard.h': 'trigger/recursion_guard.h',
        }

        # 存储过程模块映射
        procedure_mappings = {
            'procedure_parser.h': 'procedure/procedure_parser.h',
            'procedure_vm.h': 'procedure/procedure_vm.h',
            'procedure_trigger_executor.h': 'procedure/procedure_trigger_executor.h',
            'procedure_trigger_task.h': 'execution/procedure_trigger_task.h',
        }

        # 事务模块映射
        transaction_mappings = {
            'savepoint_manager.h': 'transaction/savepoint_manager.h',
        }

        # 类型模块映射
        types_mappings = {
            'domain_manager.h': 'types/domain_manager.h',
        }

        # 合并所有映射
        mappings.update(core_mappings)
        mappings.update(storage_mappings)
        mappings.update(sql_parser_mappings)
        mappings.update(utils_mappings)
        mappings.update(network_mappings)
        mappings.update(execution_mappings)
        mappings.update(trigger_mappings)
        mappings.update(procedure_mappings)
        mappings.update(transaction_mappings)
        mappings.update(types_mappings)

        return mappings

    def _load_path_mappings(self) -> Dict[str, IncludePathMapping]:
        """加载路径映射配置"""
        mappings = {}

        # 相对路径映射
        mappings.update({
            '../include/database_manager.h': IncludePathMapping(
                old_path='../include/database_manager.h',
                new_path='core/database_manager.h',
                category='relative',
                reason='使用相对路径引用include目录'
            ),
            '../include/sql_executor.h': IncludePathMapping(
                old_path='../include/sql_executor.h',
                new_path='core/sql_executor.h',
                category='relative',
                reason='使用相对路径引用include目录'
            ),
        })

        # 弃用路径映射
        mappings.update({
            'storage/buffer_pool_v2.h': IncludePathMapping(
                old_path='storage/buffer_pool_v2.h',
                new_path='storage/buffer_pool.h',
                category='deprecated',
                reason='v2版本已弃用，使用新版本'
            ),
            'storage/buffer_pool_fixed.h': IncludePathMapping(
                old_path='storage/buffer_pool_fixed.h',
                new_path='storage/buffer_pool.h',
                category='deprecated',
                reason='已弃用的固定版本'
            ),
        })

        # 正则表达式映射 - 处理批量路径问题
        regex_mappings = [
            # 存储引擎相关正则映射
            IncludePathMapping(
                old_path=r'storage_engine/(.+)\.h',
                new_path=r'storage_engine/\1.h',
                category='regex',
                reason='存储引擎模块路径标准化',
                is_regex=True
            ),
            # 网络模块路径映射
            IncludePathMapping(
                old_path=r'network/(.+)\.h',
                new_path=r'network/\1.h',
                category='regex',
                reason='网络模块路径标准化',
                is_regex=True
            ),
            # 执行引擎路径映射
            IncludePathMapping(
                old_path=r'execution/(.+)\.h',
                new_path=r'execution/\1.h',
                category='regex',
                reason='执行引擎模块路径标准化',
                is_regex=True
            ),
        ]

        # 编译正则表达式并添加到映射
        for mapping in regex_mappings:
            if mapping.is_regex:
                try:
                    mapping.compiled_pattern = re.compile(mapping.old_path)
                    # 使用模式对象的字符串表示作为键
                    mappings[str(mapping.compiled_pattern.pattern)] = mapping
                except re.error as e:
                    print(f"Warning: Invalid regex pattern '{mapping.old_path}': {e}")

        return mappings

    def normalize_include_path(self, include_path: str, source_file: Path) -> Tuple[str, Optional[str]]:
        """
        规范化include路径
        返回: (normalized_path, error_message)
        """
        # 移除引号
        include_path = include_path.strip('"<>')

        # 检查是否已有映射
        if include_path in self.path_mappings:
            mapping = self.path_mappings[include_path]
            return mapping.new_path, None

        # 处理相对路径
        if include_path.startswith('../'):
            normalized = self._resolve_relative_path(include_path, source_file)
            if normalized:
                return normalized, None

        # 检查是否为标准库或第三方库
        if self._is_standard_library(include_path):
            return include_path, None

        # 检查include文件是否存在
        if self._include_file_exists(include_path):
            return include_path, None

        # 尝试自动修正
        suggested = self._suggest_correct_path(include_path, source_file)
        if suggested:
            return suggested, f"建议使用: {suggested}"

        return include_path, f"无法识别的include路径: {include_path}"

    def _resolve_relative_path(self, include_path: str, source_file: Path) -> Optional[str]:
        """解析相对路径"""
        try:
            # 计算相对路径的绝对位置
            resolved_path = (source_file.parent / include_path).resolve()

            # 检查是否在include目录内
            if resolved_path.is_relative_to(self.include_dir):
                relative_path = resolved_path.relative_to(self.include_dir)
                return str(relative_path)

        except (ValueError, RuntimeError):
            pass

        return None

    def _is_standard_library(self, include_path: str) -> bool:
        """检查是否为标准库头文件"""
        standard_headers = {
            # C标准库
            'assert.h', 'ctype.h', 'errno.h', 'float.h', 'limits.h', 'locale.h',
            'math.h', 'setjmp.h', 'signal.h', 'stdarg.h', 'stddef.h', 'stdio.h',
            'stdlib.h', 'string.h', 'time.h',

            # C++标准库
            'algorithm', 'array', 'atomic', 'bitset', 'chrono', 'complex',
            'deque', 'exception', 'filesystem', 'fstream', 'functional',
            'future', 'initializer_list', 'iomanip', 'ios', 'iosfwd',
            'iostream', 'istream', 'iterator', 'limits', 'list', 'locale',
            'map', 'memory', 'mutex', 'new', 'numeric', 'optional', 'ostream',
            'queue', 'random', 'ratio', 'regex', 'set', 'sstream', 'stack',
            'stdexcept', 'streambuf', 'string', 'string_view', 'system_error',
            'thread', 'tuple', 'type_traits', 'unordered_map', 'unordered_set',
            'utility', 'valarray', 'variant', 'vector',

            # C++17及以上
            'any', 'execution', 'memory_resource', 'shared_mutex', 'typeindex',

            # 第三方库
            'gtest/gtest.h', 'gmock/gmock.h', 'spdlog/spdlog.h', 'spdlog/sinks/basic_file_sink.h',
            'spdlog/sinks/stdout_color_sinks.h', 'openssl/ssl.h', 'openssl/err.h'
        }

        return include_path in standard_headers or include_path.startswith(('gtest/', 'gmock/', 'spdlog/', 'openssl/'))

    def _include_file_exists(self, include_path: str) -> bool:
        """检查include文件是否存在"""
        include_file = self.include_dir / include_path
        return include_file.exists()

    def _suggest_correct_path(self, include_path: str, source_file: Path) -> Optional[str]:
        """建议正确的include路径"""

        # 检查模块映射
        if include_path in self.module_mappings:
            return self.module_mappings[include_path]

        # 尝试模糊匹配
        for key, value in self.module_mappings.items():
            if key.split('.')[0] in include_path:
                return value

        # 检查include目录下的相似文件
        if self.include_dir.exists():
            for root, dirs, files in os.walk(self.include_dir):
                for file in files:
                    if file.endswith('.h') and include_path.split('/')[-1] in file:
                        rel_path = Path(root).relative_to(self.include_dir) / file
                        return str(rel_path)

        return None

    def analyze_file_includes(self, file_path: Path) -> List[IncludeIssue]:
        """分析文件的include问题"""
        issues = []

        try:
            content = file_path.read_text(encoding='utf-8')
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

                # 规范化路径
                normalized, error_msg = self.normalize_include_path(include_path, file_path)

                if error_msg:
                    issues.append(IncludeIssue(
                        file_path=str(file_path.relative_to(self.project_root)),
                        line_number=i,
                        include_path=include_path,
                        issue_type='incorrect_path',
                        suggested_fix=normalized,
                        severity='medium'
                    ))
                elif normalized != include_path:
                    issues.append(IncludeIssue(
                        file_path=str(file_path.relative_to(self.project_root)),
                        line_number=i,
                        include_path=include_path,
                        issue_type='can_normalize',
                        suggested_fix=normalized,
                        severity='low'
                    ))

        except Exception as e:
            issues.append(IncludeIssue(
                file_path=str(file_path.relative_to(self.project_root)),
                line_number=0,
                include_path='',
                issue_type='read_error',
                suggested_fix='',
                severity='high'
            ))

        return issues

    def scan_project_includes(self, directories: List[str] = None) -> Dict[str, List[IncludeIssue]]:
        """扫描项目中的include问题"""
        if directories is None:
            directories = ['src', 'include', 'tests']

        results = {}

        for dir_name in directories:
            dir_path = self.project_root / dir_name
            if not dir_path.exists():
                continue

            for file_path in dir_path.rglob('*.cpp'):
                if file_path.name.endswith('_test.cpp') or file_path.name.endswith('.cpp'):
                    issues = self.analyze_file_includes(file_path)
                    if issues:
                        results[str(file_path.relative_to(self.project_root))] = issues

        return results

    def generate_fix_script(self, analysis_results: Dict[str, List[IncludeIssue]]) -> str:
        """生成修复脚本"""
        script_lines = ['#!/bin/bash', '# SQLCC Include路径修复脚本', '']

        for file_path, issues in analysis_results.items():
            script_lines.append(f'# 修复文件: {file_path}')

            for issue in issues:
                if issue.issue_type in ['incorrect_path', 'can_normalize']:
                    # 生成sed命令进行替换
                    old_include = f'#include "{issue.include_path}"'
                    new_include = f'#include "{issue.suggested_fix}"'

                    # 处理尖括号格式
                    old_include_angle = f'#include <{issue.include_path}>'
                    new_include_angle = f'#include <{issue.suggested_fix}>'

                    script_lines.extend([
                        f'sed -i \'s|{old_include}|{new_include}|g\' "{file_path}"',
                        f'sed -i \'s|{old_include_angle}|{new_include_angle}|g\' "{file_path}"'
                    ])

            script_lines.append('')

        return '\n'.join(script_lines)

    def save_analysis_report(self, analysis_results: Dict[str, List[IncludeIssue]], output_file: str):
        """保存分析报告"""
        # 将IncludeIssue对象转换为字典
        serializable_results = {}
        for file_path, issues in analysis_results.items():
            serializable_results[file_path] = [asdict(issue) for issue in issues]

        report = {
            'timestamp': str(self.project_root.stat().st_mtime),
            'analysis_results': serializable_results,
            'summary': {
                'total_files': len(analysis_results),
                'total_issues': sum(len(issues) for issues in analysis_results.values()),
                'severity_breakdown': self._calculate_severity_breakdown(analysis_results)
            }
        }

        with open(output_file, 'w', encoding='utf-8') as f:
            json.dump(report, f, indent=2, ensure_ascii=False)

    def _calculate_severity_breakdown(self, analysis_results: Dict[str, List[IncludeIssue]]) -> Dict[str, int]:
        """计算严重程度分布"""
        breakdown = {'high': 0, 'medium': 0, 'low': 0}

        for issues in analysis_results.values():
            for issue in issues:
                breakdown[issue.severity] += 1

        return breakdown

    def analyze_project_dependencies(self, directories: List[str] = None) -> Dict[str, any]:
        """分析整个项目的依赖关系"""
        if directories is None:
            directories = ['src', 'include', 'tests']

        print("🔗 构建项目依赖关系图...")

        # 收集所有include关系
        for dir_name in directories:
            dir_path = self.project_root / dir_name
            if not dir_path.exists():
                continue

            for file_path in dir_path.rglob('*.cpp'):
                if file_path.name.endswith('_test.cpp') or file_path.name.endswith('.cpp'):
                    self._analyze_file_dependencies(file_path)

        # 检测循环依赖
        cycles = self.dependency_analyzer.detect_cycles()

        # 计算统计信息
        stats = self._calculate_dependency_stats()

        return {
            'dependency_graph': self.dependency_analyzer.dependency_graph,
            'reverse_graph': self.dependency_analyzer.reverse_graph,
            'cycles': cycles,
            'stats': stats
        }

    def _analyze_file_dependencies(self, file_path: Path):
        """分析单个文件的依赖关系"""
        try:
            content = file_path.read_text(encoding='utf-8')
            lines = content.split('\n')

            file_key = str(file_path.relative_to(self.project_root))

            for line in lines:
                line = line.strip()
                if not line.startswith('#include'):
                    continue

                # 提取include路径
                match = re.match(r'#include\s*[<"]([^>"]+)[>"]', line)
                if not match:
                    continue

                include_path = match.group(1)

                # 解析实际的文件路径
                resolved_path = self._resolve_include_to_file(include_path, file_path)
                if resolved_path:
                    resolved_key = str(resolved_path.relative_to(self.project_root))
                    self.dependency_analyzer.add_dependency(file_key, resolved_key)

        except Exception as e:
            print(f"Error analyzing {file_path}: {e}")

    def _resolve_include_to_file(self, include_path: str, source_file: Path) -> Optional[Path]:
        """将include路径解析为实际文件路径"""
        # 标准化include路径
        normalized, _ = self.normalize_include_path(include_path, source_file)

        if normalized:
            # 尝试在include目录中查找
            include_file = self.include_dir / normalized
            if include_file.exists():
                return include_file

        return None

    def _calculate_dependency_stats(self) -> Dict[str, any]:
        """计算依赖关系统计信息"""
        graph = self.dependency_analyzer.dependency_graph

        # 计算各种统计信息
        total_files = len(graph)
        total_dependencies = sum(len(deps) for deps in graph.values())

        # 计算include深度
        depths = {}
        for file in graph:
            depths[file] = self.dependency_analyzer.get_include_depth(file)

        max_depth = max(depths.values()) if depths else 0
        avg_depth = sum(depths.values()) / len(depths) if depths else 0

        # 计算最依赖的文件
        most_depended = {}
        for deps in graph.values():
            for dep in deps:
                most_depended[dep] = most_depended.get(dep, 0) + 1

        top_depended = sorted(most_depended.items(), key=lambda x: x[1], reverse=True)[:10]

        return {
            'total_files': total_files,
            'total_dependencies': total_dependencies,
            'max_include_depth': max_depth,
            'avg_include_depth': avg_depth,
            'cycles_count': len(self.dependency_analyzer.cycles),
            'most_depended_files': top_depended
        }

    def generate_dependency_report(self, analysis_result: Dict[str, any], output_file: str):
        """生成依赖关系分析报告"""
        report = {
            'timestamp': str(self.project_root.stat().st_mtime),
            'cycles': analysis_result['cycles'],
            'stats': analysis_result['stats'],
            'recommendations': self._generate_dependency_recommendations(analysis_result)
        }

        with open(output_file, 'w', encoding='utf-8') as f:
            json.dump(report, f, indent=2, ensure_ascii=False)

        print(f"📊 依赖关系分析报告已保存到: {output_file}")

    def _generate_dependency_recommendations(self, analysis_result: Dict[str, any]) -> List[str]:
        """生成依赖关系优化建议"""
        recommendations = []

        # 循环依赖警告
        cycles = analysis_result['cycles']
        if cycles:
            recommendations.append(f"⚠️  发现 {len(cycles)} 个循环依赖，需要立即处理")

        # 深度警告
        stats = analysis_result['stats']
        if stats['max_include_depth'] > 5:
            recommendations.append(f"⚠️  最大include深度为 {stats['max_include_depth']}，建议优化")

        # 高依赖文件警告
        most_depended = stats['most_depended_files']
        if most_depended and most_depended[0][1] > 10:
            top_file, count = most_depended[0]
            recommendations.append(f"📈 文件 {top_file} 被 {count} 个文件依赖，考虑模块化")

        return recommendations

    def auto_fix_includes(self, analysis_results: Dict[str, List[IncludeIssue]]) -> Dict[str, int]:
        """自动修复include问题"""
        fixes_applied = {}

        for file_path, issues in analysis_results.items():
            fixes_count = 0

            for issue in issues:
                if issue.issue_type in ['incorrect_path', 'can_normalize']:
                    try:
                        self._apply_fix_to_file(file_path, issue)
                        fixes_count += 1
                        self.stats['auto_fixes_applied'] += 1
                    except Exception as e:
                        print(f"Error fixing {file_path}:{issue.line_number}: {e}")

            if fixes_count > 0:
                fixes_applied[file_path] = fixes_count

        return fixes_applied

    def _apply_fix_to_file(self, file_path: str, issue: IncludeIssue):
        """对单个文件应用修复"""
        full_path = self.project_root / file_path

        try:
            with open(full_path, 'r', encoding='utf-8') as f:
                lines = f.readlines()

            # 修改指定行
            if 1 <= issue.line_number <= len(lines):
                old_line = lines[issue.line_number - 1]
                new_line = old_line.replace(
                    f'#include "{issue.include_path}"',
                    f'#include "{issue.suggested_fix}"'
                ).replace(
                    f'#include <{issue.include_path}>',
                    f'#include <{issue.suggested_fix}>'
                )

                lines[issue.line_number - 1] = new_line

                # 写回文件
                with open(full_path, 'w', encoding='utf-8') as f:
                    f.writelines(lines)

        except Exception as e:
            raise Exception(f"Failed to apply fix: {e}")

    def validate_build_compatibility(self) -> Dict[str, any]:
        """验证构建兼容性"""
        issues = []

        # 检查Bazel文件
        bazel_files = list(self.project_root.rglob('BUILD.bazel'))
        bazel_files.extend(self.project_root.rglob('BUILD'))

        for bazel_file in bazel_files:
            try:
                content = bazel_file.read_text(encoding='utf-8')

                # 检查相对路径include
                if '../include/' in content:
                    issues.append({
                        'file': str(bazel_file.relative_to(self.project_root)),
                        'type': 'relative_include_in_bazel',
                        'description': 'Bazel文件中包含相对路径include'
                    })

            except Exception as e:
                issues.append({
                    'file': str(bazel_file.relative_to(self.project_root)),
                    'type': 'read_error',
                    'description': f'无法读取Bazel文件: {e}'
                })

        return {
            'compatibility_issues': issues,
            'total_bazel_files': len(bazel_files),
            'issues_count': len(issues)
        }


def create_bazel_integration(manager: IncludePathManager) -> str:
    """创建Bazel集成配置"""
    bazel_content = '''
# SQLCC Include路径检查规则

def _include_path_checker_impl(ctx):
    """Include路径检查器实现"""
    output = ctx.actions.declare_file(ctx.label.name + "_report.json")

    args = ctx.actions.args()
    args.add("--project-root", ctx.workspace_root)
    args.add("--analyze")
    args.add("--output", output.path)

    ctx.actions.run(
        outputs = [output],
        executable = ctx.executable._checker,
        arguments = [args],
    )

    return [DefaultInfo(files = depset([output]))]

include_path_checker = rule(
    implementation = _include_path_checker_impl,
    attrs = {
        "_checker": attr.label(
            executable = True,
            cfg = "exec",
            default = Label("//tools:include_path_manager"),
        ),
    },
)
'''

    return bazel_content.strip()


def generate_build_integration_script(manager: IncludePathManager) -> str:
    """生成构建集成脚本"""
    script = '''#!/bin/bash
# SQLCC 构建时Include检查脚本

set -e

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
INCLUDE_CHECKER="$PROJECT_ROOT/tools/include_path_manager.py"

echo "🔍 构建时Include路径检查..."

# 运行include检查
python3 "$INCLUDE_CHECKER" --project-root "$PROJECT_ROOT" --analyze

# 检查返回值
if [ $? -eq 0 ]; then
    echo "✅ Include路径检查通过"
else
    echo "❌ Include路径检查失败，请修复问题后重新构建"
    exit 1
fi
'''

    return script


def main():
    import argparse

    parser = argparse.ArgumentParser(description='SQLCC Include路径管理器')
    parser.add_argument('--project-root', default='.', help='项目根目录')
    parser.add_argument('--analyze', action='store_true', help='分析include问题')
    parser.add_argument('--fix-script', action='store_true', help='生成修复脚本')
    parser.add_argument('--dependencies', action='store_true', help='分析依赖关系')
    parser.add_argument('--bazel-integration', action='store_true', help='生成Bazel集成配置')
    parser.add_argument('--build-integration', action='store_true', help='生成构建集成脚本')
    parser.add_argument('--output', default='include_analysis_report.json', help='输出文件')

    args = parser.parse_args()

    manager = IncludePathManager(args.project_root)

    if args.analyze:
        print("🔍 分析项目include路径问题...")
        results = manager.scan_project_includes()

        print(f"📊 发现 {len(results)} 个有问题的文件")
        total_issues = sum(len(issues) for issues in results.values())
        print(f"📋 共发现 {total_issues} 个include问题")

        manager.save_analysis_report(results, args.output)
        print(f"💾 分析报告已保存到: {args.output}")

    if args.fix_script:
        print("🔧 生成修复脚本...")
        # 先分析再生成脚本
        results = manager.scan_project_includes()
        script_content = manager.generate_fix_script(results)

        script_file = 'fix_include_paths.sh'
        with open(script_file, 'w', encoding='utf-8') as f:
            f.write(script_content)

        # 给脚本添加执行权限
        os.chmod(script_file, 0o755)

        print(f"📝 修复脚本已生成: {script_file}")

    if args.dependencies:
        print("🔗 分析项目依赖关系...")
        analysis_result = manager.analyze_project_dependencies()

        print("📊 依赖分析统计:")
        print(f"  - 总文件数: {analysis_result['stats']['total_files']}")
        print(f"  - 总依赖数: {analysis_result['stats']['total_dependencies']}")
        print(f"  - 最大include深度: {analysis_result['stats']['max_include_depth']}")
        print(f"  - 循环依赖数: {len(analysis_result['cycles'])}")

        if analysis_result['cycles']:
            print(f"⚠️  发现 {len(analysis_result['cycles'])} 个循环依赖")

        dep_report = 'dependency_analysis_report.json'
        manager.generate_dependency_report(analysis_result, dep_report)
        print(f"💾 依赖分析报告已保存到: {dep_report}")

    if args.bazel_integration:
        print("📦 生成Bazel集成配置...")
        bazel_config = create_bazel_integration(manager)

        bazel_file = 'include_checker.bzl'
        with open(bazel_file, 'w', encoding='utf-8') as f:
            f.write(bazel_config)

        print(f"📝 Bazel集成配置已生成: {bazel_file}")

    if args.build_integration:
        print("🔨 生成构建集成脚本...")
        build_script = generate_build_integration_script(manager)

        script_file = 'check_includes_build.sh'
        with open(script_file, 'w', encoding='utf-8') as f:
            f.write(build_script)

        # 给脚本添加执行权限
        os.chmod(script_file, 0o755)

        print(f"📝 构建集成脚本已生成: {script_file}")

    # 打印统计信息
    if any([args.analyze, args.fix_script, args.dependencies]):
        print("\n📈 统计信息:")
        print(f"  - 分析文件数: {manager.stats['files_analyzed']}")
        print(f"  - 发现问题数: {manager.stats['issues_found']}")
        print(f"  - 检测循环数: {manager.stats['cycles_detected']}")
        print(f"  - 自动修复数: {manager.stats['auto_fixes_applied']}")
        print(f"  - Bazel文件数: {manager.stats['bazel_files_checked']}")
        print(f"  - Bazel问题数: {manager.stats['bazel_issues_found']}")
        print(f"  - 依赖分析数: {manager.stats['dependencies_analyzed']}")
        print(f"  - 报告生成数: {manager.stats['reports_generated']}")


if __name__ == '__main__':
    main()
