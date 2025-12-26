"""
SQLCC Include路径分析器
负责分析和检测include路径问题
"""

import re
from pathlib import Path
from typing import Dict, List, Optional, Set, Tuple
from ..utils.models import (
    IncludeIssue, IssueType, IssueSeverity,
    Config, AnalysisResult, DependencyNode, CircularDependency
)


class IncludePathAnalyzer:
    """Include路径分析器"""

    def __init__(self, config: Config):
        self.config = config
        self.project_root = Path(config.project_root)
        self.include_dirs = [self.project_root / d for d in config.include_dirs]

        # 编译正则表达式用于检测问题
        self.relative_path_pattern = re.compile(r'^(\.\./)+')
        self._compiled_patterns = {}

        # 缓存
        self._header_cache: Dict[str, Path] = {}
        self._analysis_cache: Dict[str, List[IncludeIssue]] = {}

    def analyze_file(self, file_path: Path) -> List[IncludeIssue]:
        """
        分析单个文件的include问题
        """
        if str(file_path) in self._analysis_cache:
            return self._analysis_cache[str(file_path)]

        issues = []
        relative_file_path = file_path.relative_to(self.project_root)

        try:
            content = file_path.read_text(encoding='utf-8')
            lines = content.split('\n')

            for line_num, line in enumerate(lines, 1):
                line = line.strip()
                if not line.startswith('#include'):
                    continue

                include_path = self._extract_include_path(line)
                if not include_path:
                    continue

                # 分析include路径问题
                issue = self._analyze_include_path(include_path, file_path, line_num, line)
                if issue:
                    issues.append(issue)

        except Exception as e:
            issues.append(IncludeIssue(
                file_path=str(relative_file_path),
                line_number=0,
                include_path='',
                issue_type=IssueType.READ_ERROR,
                suggested_fix='',
                severity=IssueSeverity.HIGH,
                description=f"文件读取错误: {e}"
            ))

        self._analysis_cache[str(file_path)] = issues
        return issues

    def analyze_project(self, directories: Optional[List[str]] = None) -> AnalysisResult:
        """
        分析整个项目的include问题
        """
        if directories is None:
            directories = self.config.src_dirs

        timestamp = self._get_timestamp()
        issues_by_file = {}
        dependency_graph = {}

        total_files = 0

        for dir_name in directories:
            dir_path = self.project_root / dir_name
            if not dir_path.exists():
                continue

            for file_path in dir_path.rglob('*.cpp'):
                if self._should_analyze_file(file_path):
                    total_files += 1
                    issues = self.analyze_file(file_path)
                    if issues:
                        relative_path = str(file_path.relative_to(self.project_root))
                        issues_by_file[relative_path] = issues

                    # 构建依赖关系图
                    self._build_dependency_graph(file_path, dependency_graph)

        # 检测循环依赖
        circular_dependencies = self._detect_circular_dependencies(dependency_graph)

        # 计算统计信息
        statistics = self._calculate_statistics(issues_by_file, dependency_graph)

        return AnalysisResult(
            timestamp=timestamp,
            project_root=self.config.project_root,
            total_files=total_files,
            issues_by_file=issues_by_file,
            dependency_graph=dependency_graph,
            circular_dependencies=circular_dependencies,
            statistics=statistics
        )

    def _extract_include_path(self, line: str) -> Optional[str]:
        """从include行中提取路径"""
        # 匹配 #include "path" 或 #include <path>
        match = re.match(r'#include\s*[<"]([^>"]+)[>"]', line)
        return match.group(1) if match else None

    def _analyze_include_path(self, include_path: str, source_file: Path,
                            line_num: int, context: str) -> Optional[IncludeIssue]:
        """分析单个include路径"""
        relative_source_path = str(source_file.relative_to(self.project_root))

        # 1. 检查相对路径问题
        if self.relative_path_pattern.match(include_path):
            return IncludeIssue(
                file_path=relative_source_path,
                line_number=line_num,
                include_path=include_path,
                issue_type=IssueType.RELATIVE_PATH,
                suggested_fix=self._resolve_relative_path(include_path, source_file),
                severity=IssueSeverity.HIGH,
                description="使用相对路径引用include目录",
                context=context
            )

        # 2. 检查已弃用的头文件
        if include_path in self.config.deprecated_headers:
            return IncludeIssue(
                file_path=relative_source_path,
                line_number=line_num,
                include_path=include_path,
                issue_type=IssueType.DEPRECATED_HEADER,
                suggested_fix=self._suggest_deprecated_replacement(include_path),
                severity=IssueSeverity.MEDIUM,
                description="使用已弃用的头文件",
                context=context
            )

        # 3. 检查模块路径映射
        if include_path in self.config.module_mappings:
            mapped_path = self.config.module_mappings[include_path]
            if mapped_path != include_path:
                return IncludeIssue(
                    file_path=relative_source_path,
                    line_number=line_num,
                    include_path=include_path,
                    issue_type=IssueType.INCORRECT_MODULE_PATH,
                    suggested_fix=mapped_path,
                    severity=IssueSeverity.MEDIUM,
                    description="模块路径不正确",
                    context=context
                )

        # 4. 检查头文件是否存在
        resolved_path = self._resolve_include_path(include_path, source_file)
        if not resolved_path or not resolved_path.exists():
            return IncludeIssue(
                file_path=relative_source_path,
                line_number=line_num,
                include_path=include_path,
                issue_type=IssueType.MISSING_HEADER,
                suggested_fix=self._suggest_missing_header_fix(include_path),
                severity=IssueSeverity.HIGH,
                description="引用的头文件不存在",
                context=context
            )

        # 5. 检查是否可以规范化路径
        normalized_path = self._normalize_include_path(include_path, source_file)
        if normalized_path and normalized_path != include_path:
            return IncludeIssue(
                file_path=relative_source_path,
                line_number=line_num,
                include_path=include_path,
                issue_type=IssueType.CAN_NORMALIZE,
                suggested_fix=normalized_path,
                severity=IssueSeverity.LOW,
                description="路径可以规范化",
                context=context
            )

        return None

    def _resolve_relative_path(self, include_path: str, source_file: Path) -> Optional[str]:
        """解析相对路径"""
        try:
            # 计算相对路径对应的绝对路径
            resolved_path = (source_file.parent / include_path).resolve()

            # 检查是否在include目录中
            for include_dir in self.include_dirs:
                if resolved_path.is_relative_to(include_dir):
                    return str(resolved_path.relative_to(include_dir))

        except (ValueError, RuntimeError):
            pass

        return None

    def _resolve_include_path(self, include_path: str, source_file: Path) -> Optional[Path]:
        """解析include路径为实际文件路径"""
        # 1. 检查标准库
        if include_path in self.config.standard_libraries:
            return None  # 标准库文件不需要解析

        # 2. 检查include目录
        for include_dir in self.include_dirs:
            candidate = include_dir / include_path
            if candidate.exists():
                return candidate

        # 3. 尝试从模块映射解析
        if include_path in self.config.module_mappings:
            mapped_path = self.config.module_mappings[include_path]
            for include_dir in self.include_dirs:
                candidate = include_dir / mapped_path
                if candidate.exists():
                    return candidate

        return None

    def _normalize_include_path(self, include_path: str, source_file: Path) -> Optional[str]:
        """规范化include路径"""
        resolved = self._resolve_include_path(include_path, source_file)
        if resolved:
            for include_dir in self.include_dirs:
                if resolved.is_relative_to(include_dir):
                    return str(resolved.relative_to(include_dir))

        return None

    def _suggest_deprecated_replacement(self, deprecated_header: str) -> str:
        """建议已弃用头文件的替代方案"""
        # 简单的映射替换
        replacements = {
            "storage/buffer_pool_v2.h": "storage/buffer_pool.h",
            "storage/buffer_pool_fixed.h": "storage/buffer_pool.h"
        }
        return replacements.get(deprecated_header, deprecated_header)

    def _suggest_missing_header_fix(self, missing_header: str) -> str:
        """建议缺失头文件的修复方案"""
        # 检查是否有相似的头文件
        for include_dir in self.include_dirs:
            if include_dir.exists():
                for root, dirs, files in os.walk(include_dir):
                    for file in files:
                        if file.endswith('.h') and missing_header.split('/')[-1] in file:
                            rel_path = Path(root).relative_to(include_dir) / file
                            return str(rel_path)

        return f"检查路径或创建文件: {missing_header}"

    def _build_dependency_graph(self, file_path: Path, dependency_graph: Dict[str, DependencyNode]):
        """构建依赖关系图"""
        try:
            content = file_path.read_text(encoding='utf-8')
            lines = content.split('\n')

            file_key = str(file_path.relative_to(self.project_root))
            includes = set()

            for line in lines:
                line = line.strip()
                if not line.startswith('#include'):
                    continue

                include_path = self._extract_include_path(line)
                if include_path and include_path not in self.config.standard_libraries:
                    resolved = self._resolve_include_path(include_path, file_path)
                    if resolved:
                        resolved_key = str(resolved.relative_to(self.project_root))
                        includes.add(resolved_key)

            # 计算include深度
            include_depth = self._calculate_include_depth(includes, dependency_graph)

            dependency_graph[file_key] = DependencyNode(
                file_path=file_key,
                includes=includes,
                include_depth=include_depth
            )

            # 更新反向依赖
            for included_file in includes:
                if included_file not in dependency_graph:
                    dependency_graph[included_file] = DependencyNode(
                        file_path=included_file,
                        includes=set(),
                        include_depth=0
                    )
                dependency_graph[included_file].included_by.add(file_key)

        except Exception:
            pass

    def _calculate_include_depth(self, includes: Set[str], dependency_graph: Dict[str, DependencyNode]) -> int:
        """计算include深度"""
        if not includes:
            return 0

        max_depth = 0
        for included_file in includes:
            if included_file in dependency_graph:
                depth = dependency_graph[included_file].include_depth + 1
            else:
                depth = 1
            max_depth = max(max_depth, depth)

        return max_depth

    def _detect_circular_dependencies(self, dependency_graph: Dict[str, DependencyNode]) -> List[CircularDependency]:
        """检测循环依赖"""
        from ..utils.models import CircularDependency, IssueSeverity

        cycles = []
        visited = set()
        rec_stack = set()

        def dfs(node: str, path: List[str]) -> bool:
            visited.add(node)
            rec_stack.add(node)
            path.append(node)

            for neighbor in dependency_graph.get(node, DependencyNode(node)).includes:
                if neighbor not in visited:
                    if dfs(neighbor, path):
                        return True
                elif neighbor in rec_stack:
                    # 找到循环
                    cycle_start = path.index(neighbor)
                    cycle = path[cycle_start:] + [neighbor]
                    cycles.append(CircularDependency(
                        cycle=cycle,
                        description=f"循环依赖: {' -> '.join(cycle[:-1])}",
                        severity=IssueSeverity.HIGH
                    ))
                    return True

            path.pop()
            rec_stack.remove(node)
            return False

        for node in dependency_graph:
            if node not in visited:
                dfs(node, [])

        return cycles

    def _calculate_statistics(self, issues_by_file: Dict[str, List[IncludeIssue]],
                            dependency_graph: Dict[str, DependencyNode]) -> Dict[str, any]:
        """计算统计信息"""
        total_issues = sum(len(issues) for issues in issues_by_file.values())

        # 按类型统计
        type_stats = {}
        severity_stats = {}
        depth_stats = {'max': 0, 'avg': 0.0, 'distribution': {}}

        for issues in issues_by_file.values():
            for issue in issues:
                # 类型统计
                issue_type = issue.issue_type.value
                type_stats[issue_type] = type_stats.get(issue_type, 0) + 1

                # 严重程度统计
                severity = issue.severity.value
                severity_stats[severity] = severity_stats.get(severity, 0) + 1

        # 深度统计
        if dependency_graph:
            depths = [node.include_depth for node in dependency_graph.values()]
            depth_stats['max'] = max(depths) if depths else 0
            depth_stats['avg'] = sum(depths) / len(depths) if depths else 0

            # 深度分布
            for depth in range(depth_stats['max'] + 1):
                count = sum(1 for d in depths if d == depth)
                if count > 0:
                    depth_stats['distribution'][str(depth)] = count

        return {
            'total_issues': total_issues,
            'issues_by_type': type_stats,
            'issues_by_severity': severity_stats,
            'dependency_stats': {
                'total_files': len(dependency_graph),
                'depth_stats': depth_stats
            }
        }

    def _should_analyze_file(self, file_path: Path) -> bool:
        """判断是否应该分析文件"""
        # 只分析C++源文件
        if not file_path.suffix in ['.cpp', '.cc', '.cxx']:
            return False

        # 跳过测试文件（如果不需要）
        if 'test' in file_path.name.lower():
            return True

        return True

    def _get_timestamp(self) -> str:
        """获取当前时间戳"""
        from datetime import datetime
        return datetime.now().strftime('%Y-%m-%d %H:%M:%S')

    def clear_cache(self):
        """清除缓存"""
        self._header_cache.clear()
        self._analysis_cache.clear()


# 导入os模块（需要在函数内部使用时导入）
import os
