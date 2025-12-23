#!/usr/bin/env python3
"""
SQLCC 预编译头文件分析工具
分析头文件使用频率，确定预编译头文件策略
"""

import os
import json
import re
from pathlib import Path
from typing import Dict, List, Tuple, Set
from collections import defaultdict, Counter


class PrecompiledHeaderAnalyzer:
    """预编译头文件分析器"""

    def __init__(self, project_root: str):
        self.project_root = Path(project_root)
        self.include_dir = self.project_root / 'include'
        self.src_dir = self.project_root / 'src'

        # 标准库头文件（不应放入预编译头文件）
        self.standard_headers = self._load_standard_headers()

        # 系统头文件（不应放入预编译头文件）
        self.system_headers = self._load_system_headers()

        # 分析结果
        self.header_usage = Counter()
        self.file_includes = defaultdict(set)
        self.include_frequency = defaultdict(int)

    def _load_standard_headers(self) -> Set[str]:
        """加载标准库头文件列表"""
        return {
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

            # C标准库的C++版本
            'cassert', 'cctype', 'cerrno', 'cfloat', 'climits', 'clocale',
            'cmath', 'csetjmp', 'csignal', 'cstdarg', 'cstddef', 'cstdio',
            'cstdlib', 'cstring', 'ctime',
        }

    def _load_system_headers(self) -> Set[str]:
        """加载系统头文件列表"""
        return {
            # 系统调用
            'fcntl.h', 'unistd.h', 'sys/types.h', 'sys/stat.h', 'sys/socket.h',
            'sys/epoll.h', 'netinet/in.h', 'netdb.h', 'arpa/inet.h', 'cerrno',

            # 第三方库
            'gtest/gtest.h', 'gmock/gmock.h', 'spdlog/spdlog.h',
            'spdlog/sinks/basic_file_sink.h', 'spdlog/sinks/stdout_color_sinks.h',
            'openssl/ssl.h', 'openssl/err.h', 'readline/history.h', 'readline/readline.h',
            'sys/resource.h', 'sys/wait.h', 'sys/types.h', 'sys/socket.h',
            'netinet/in.h', 'arpa/inet.h', 'fcntl.h', 'unistd.h', 'cerrno',
            'sys/epoll.h', 'netdb.h', 'csignal'
        }

    def analyze_project_headers(self, analysis_file: str = None) -> Dict[str, any]:
        """分析项目头文件使用情况"""
        print("🔍 开始分析项目头文件使用情况...")

        if analysis_file and Path(analysis_file).exists():
            # 从现有的分析文件加载
            with open(analysis_file, 'r', encoding='utf-8') as f:
                data = json.load(f)
                analysis_results = data.get('analysis_results', {})
        else:
            # 重新分析项目
            analysis_results = self._analyze_all_files()

        # 统计头文件使用频率
        self._calculate_usage_statistics_from_analysis(analysis_results)

        # 生成预编译头文件建议
        recommendations = self._generate_pch_recommendations()

        # 生成报告
        report = {
            'header_usage_statistics': dict(self.header_usage.most_common(50)),
            'total_files_analyzed': len(analysis_results),
            'total_includes_found': sum(len(includes) for includes in analysis_results.values()),
            'precompiled_header_candidates': recommendations,
            'analysis_timestamp': str(self.project_root.stat().st_mtime)
        }

        return report

    def _analyze_all_files(self) -> Dict[str, List[Dict]]:
        """分析所有源文件的include情况"""
        results = {}

        # 分析src目录
        for cpp_file in self.src_dir.rglob('*.cpp'):
            if cpp_file.name.endswith('_test.cpp') or cpp_file.name.endswith('.cpp'):
                includes = self._analyze_file_includes(cpp_file)
                if includes:
                    results[str(cpp_file.relative_to(self.project_root))] = includes

        # 分析tests目录的部分文件（重点分析）
        tests_dir = self.project_root / 'tests'
        if tests_dir.exists():
            for cpp_file in tests_dir.rglob('*.cpp'):
                if cpp_file.name.endswith('_test.cpp') or cpp_file.name.endswith('.cpp'):
                    includes = self._analyze_file_includes(cpp_file)
                    if includes:
                        results[str(cpp_file.relative_to(self.project_root))] = includes

        return results

    def _analyze_file_includes(self, file_path: Path) -> List[Dict]:
        """分析单个文件的include情况"""
        includes = []

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
                includes.append({
                    'line': i,
                    'path': include_path,
                    'type': '<>' if '<' in line else '""'
                })

        except Exception as e:
            print(f"Error analyzing {file_path}: {e}")

        return includes

    def _calculate_usage_statistics_from_analysis(self, analysis_results: Dict[str, List[Dict]]):
        """从分析结果计算头文件使用统计"""
        for file_path, includes in analysis_results.items():
            for include in includes:
                include_path = include['include_path']

                # 只统计项目内部头文件，排除标准库和系统头文件
                if (include_path not in self.standard_headers and
                    include_path not in self.system_headers and
                    not include_path.startswith('include/') and  # 相对路径已处理
                    not include_path.startswith('../')):       # 相对路径已处理

                    # 标准化路径
                    normalized_path = self._normalize_include_path(include_path)
                    if normalized_path:
                        self.header_usage[normalized_path] += 1
                        self.file_includes[file_path].add(normalized_path)
                        self.include_frequency[normalized_path] += 1

    def _normalize_include_path(self, include_path: str) -> str:
        """标准化include路径"""
        # 处理一些常见映射
        path_mappings = {
            'database_manager.h': 'core/database_manager.h',
            'sql_executor.h': 'core/sql_executor.h',
            'user_manager.h': 'core/user_manager.h',
            'permission_validator.h': 'core/permission_validator.h',
            'error_handler.h': 'core/error_handler.h',
            'storage_engine.h': 'storage_engine.h',
            'buffer_pool.h': 'storage/buffer_pool.h',
            'b_plus_tree.h': 'storage/b_plus_tree.h',
            'index_manager.h': 'storage/index_manager.h',
            'disk_manager.h': 'storage/disk_manager.h',
            'table_storage.h': 'storage/table_storage.h',
            'network.h': 'network/network.h',
            'connection_state.h': 'network/connection_state.h',
            'encryption.h': 'network/encryption.h',
            'session.h': 'network/session.h',
            'session_manager.h': 'network/session_manager.h',
            'logger.h': 'utils/logger.h',
            'config_manager.h': 'utils/config_manager.h',
            'smart_config_manager.h': 'utils/smart_config_manager.h',
            'config_snapshot.h': 'utils/config_snapshot.h',
            'config_lifecycle.h': 'utils/config_lifecycle.h',
        }

        return path_mappings.get(include_path, include_path)

    def _generate_pch_recommendations(self) -> Dict[str, any]:
        """生成预编译头文件建议"""
        # 获取使用频率最高的头文件
        top_headers = self.header_usage.most_common(30)

        # 按模块分类
        categorized_headers = self._categorize_headers(top_headers)

        # 生成预编译头文件内容
        pch_content = self._generate_pch_content(categorized_headers)

        # 计算预期收益
        estimated_benefit = self._calculate_estimated_benefit(top_headers)

        return {
            'top_used_headers': top_headers,
            'categorized_headers': categorized_headers,
            'precompiled_header_content': pch_content,
            'estimated_benefit': estimated_benefit,
            'recommendations': self._generate_implementation_recommendations()
        }

    def _categorize_headers(self, top_headers: List[Tuple[str, int]]) -> Dict[str, List[Tuple[str, int]]]:
        """按模块分类头文件"""
        categories = {
            'core': [],
            'storage': [],
            'network': [],
            'utils': [],
            'sql_parser': [],
            'execution': [],
            'other': []
        }

        for header, count in top_headers:
            if header.startswith('core/'):
                categories['core'].append((header, count))
            elif header.startswith('storage/') or header.startswith('storage_engine'):
                categories['storage'].append((header, count))
            elif header.startswith('network/'):
                categories['network'].append((header, count))
            elif header.startswith('utils/'):
                categories['utils'].append((header, count))
            elif header.startswith('sql_parser/'):
                categories['sql_parser'].append((header, count))
            elif header.startswith('execution/'):
                categories['execution'].append((header, count))
            else:
                categories['other'].append((header, count))

        return categories

    def _generate_pch_content(self, categorized_headers: Dict[str, List[Tuple[str, int]]]) -> str:
        """生成预编译头文件内容"""
        lines = [
            "// SQLCC Precompiled Header File",
            "// Generated automatically - DO NOT EDIT",
            "",
            "#pragma once",
            "",
            "// Standard library headers",
            "#include <iostream>",
            "#include <string>",
            "#include <vector>",
            "#include <map>",
            "#include <set>",
            "#include <memory>",
            "#include <functional>",
            "#include <algorithm>",
            "#include <exception>",
            "#include <stdexcept>",
            "",
            "// C standard library",
            "#include <cstring>",
            "#include <cctype>",
            "#include <ctime>",
            "#include <cmath>",
            "",
        ]

        # 按类别添加项目头文件
        for category, headers in categorized_headers.items():
            if not headers:
                continue

            lines.append(f"// {category.upper()} module headers")
            for header, _ in headers:
                lines.append(f'#include "{header}"')
            lines.append("")

        return '\n'.join(lines)

    def _calculate_estimated_benefit(self, top_headers: List[Tuple[str, int]]) -> Dict[str, any]:
        """计算预编译头文件的预期收益"""
        total_includes = sum(count for _, count in top_headers)
        total_files = len(self.file_includes)

        # 估算编译时间节省
        # 假设每个include节省5ms的编译时间
        estimated_time_saved_ms = total_includes * 5
        estimated_time_saved_sec = estimated_time_saved_ms / 1000

        return {
            'total_includes_covered': total_includes,
            'estimated_compile_time_saved_sec': estimated_time_saved_sec,
            'estimated_compile_time_saved_min': estimated_time_saved_sec / 60,
            'coverage_percentage': (total_includes / sum(self.include_frequency.values())) * 100,
            'files_that_would_benefit': total_files
        }

    def _generate_implementation_recommendations(self) -> List[str]:
        """生成实施建议"""
        return [
            "1. 创建 'include/sqlcc_pch.h' 作为主预编译头文件",
            "2. 在Bazel BUILD文件中添加预编译头文件配置",
            "3. 在主要的源文件中包含预编译头文件",
            "4. 测试编译时间改进效果",
            "5. 根据实际效果调整预编译头文件内容",
            "6. 考虑为不同模块创建专门的预编译头文件",
            "7. 监控预编译头文件的维护成本"
        ]

    def save_analysis_report(self, report: Dict[str, any], output_file: str):
        """保存分析报告"""
        with open(output_file, 'w', encoding='utf-8') as f:
            json.dump(report, f, indent=2, ensure_ascii=False)

        print(f"💾 预编译头文件分析报告已保存到: {output_file}")

    def generate_pch_file(self, report: Dict[str, any], output_file: str):
        """生成预编译头文件"""
        content = report['precompiled_header_candidates']['precompiled_header_content']

        with open(output_file, 'w', encoding='utf-8') as f:
            f.write(content)

        print(f"📝 预编译头文件已生成: {output_file}")


def main():
    import argparse

    parser = argparse.ArgumentParser(description='SQLCC 预编译头文件分析工具')
    parser.add_argument('--project-root', default='.', help='项目根目录')
    parser.add_argument('--analysis-file', help='现有的include分析文件')
    parser.add_argument('--analyze', action='store_true', help='分析头文件使用情况')
    parser.add_argument('--generate-pch', action='store_true', help='生成预编译头文件')
    parser.add_argument('--output', default='pch_analysis_report.json', help='输出文件')
    parser.add_argument('--pch-file', default='include/sqlcc_pch.h', help='预编译头文件路径')

    args = parser.parse_args()

    analyzer = PrecompiledHeaderAnalyzer(args.project_root)

    if args.analyze:
        print("🔍 分析预编译头文件使用情况...")
        report = analyzer.analyze_project_headers(args.analysis_file)

        analyzer.save_analysis_report(report, args.output)

        # 打印关键统计信息
        stats = report['precompiled_header_candidates']['estimated_benefit']
        print("📊 分析结果:")
        print(f"  - 覆盖的include数量: {stats['total_includes_covered']}")
        print(f"  - 预计节省编译时间: {stats['estimated_compile_time_saved_sec']:.1f}秒")
        print(f"  - 预计节省编译时间: {stats['estimated_compile_time_saved_min']:.1f}分钟")
        print(f"  - 覆盖率百分比: {stats['coverage_percentage']:.1f}%")
        print(f"  - 受益文件数: {stats['files_that_would_benefit']}")

    if args.generate_pch:
        print("📝 生成预编译头文件...")
        # 先分析再生成
        report = analyzer.analyze_project_headers(args.analysis_file)
        analyzer.generate_pch_file(report, args.pch_file)


if __name__ == '__main__':
    main()
