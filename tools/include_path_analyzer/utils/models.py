"""
SQLCC Include路径分析器数据模型
定义各种数据结构和类型
"""

from dataclasses import dataclass, field
from typing import Dict, List, Optional, Set, Tuple, Any
from pathlib import Path
from enum import Enum


class IssueSeverity(Enum):
    """问题严重程度"""
    LOW = "low"
    MEDIUM = "medium"
    HIGH = "high"
    CRITICAL = "critical"


class IssueType(Enum):
    """问题类型"""
    RELATIVE_PATH = "relative_path"
    DEPRECATED_HEADER = "deprecated_header"
    INCORRECT_MODULE_PATH = "incorrect_module_path"
    MISSING_HEADER = "missing_header"
    CIRCULAR_DEPENDENCY = "circular_dependency"
    CAN_NORMALIZE = "can_normalize"
    READ_ERROR = "read_error"
    BAZEL_COMPATIBILITY = "bazel_compatibility"


@dataclass
class IncludeIssue:
    """Include问题"""
    file_path: str
    line_number: int
    include_path: str
    issue_type: IssueType
    suggested_fix: str
    severity: IssueSeverity
    description: str = ""
    context: str = ""

    def to_dict(self) -> Dict[str, Any]:
        """转换为字典"""
        return {
            'file_path': self.file_path,
            'line_number': self.line_number,
            'include_path': self.include_path,
            'issue_type': self.issue_type.value,
            'suggested_fix': self.suggested_fix,
            'severity': self.severity.value,
            'description': self.description,
            'context': self.context
        }


@dataclass
class DependencyNode:
    """依赖关系节点"""
    file_path: str
    includes: Set[str] = field(default_factory=set)
    included_by: Set[str] = field(default_factory=set)
    include_depth: int = 0

    def to_dict(self) -> Dict[str, Any]:
        """转换为字典"""
        return {
            'file_path': self.file_path,
            'includes': list(self.includes),
            'included_by': list(self.included_by),
            'include_depth': self.include_depth
        }


@dataclass
class CircularDependency:
    """循环依赖"""
    cycle: List[str]
    description: str = ""
    severity: IssueSeverity = IssueSeverity.HIGH

    def to_dict(self) -> Dict[str, Any]:
        """转换为字典"""
        return {
            'cycle': self.cycle,
            'description': self.description,
            'severity': self.severity.value
        }


@dataclass
class AnalysisResult:
    """分析结果"""
    timestamp: str
    project_root: str
    total_files: int
    issues_by_file: Dict[str, List[IncludeIssue]] = field(default_factory=dict)
    dependency_graph: Dict[str, DependencyNode] = field(default_factory=dict)
    circular_dependencies: List[CircularDependency] = field(default_factory=list)
    statistics: Dict[str, Any] = field(default_factory=dict)

    @property
    def total_issues(self) -> int:
        """总问题数"""
        return sum(len(issues) for issues in self.issues_by_file.values())

    @property
    def issues_by_severity(self) -> Dict[str, int]:
        """按严重程度统计问题"""
        severity_count = {}
        for issues in self.issues_by_file.values():
            for issue in issues:
                severity = issue.severity.value
                severity_count[severity] = severity_count.get(severity, 0) + 1
        return severity_count

    @property
    def issues_by_type(self) -> Dict[str, int]:
        """按类型统计问题"""
        type_count = {}
        for issues in self.issues_by_file.values():
            for issue in issues:
                issue_type = issue.issue_type.value
                type_count[issue_type] = type_count.get(issue_type, 0) + 1
        return type_count

    def to_dict(self) -> Dict[str, Any]:
        """转换为字典"""
        return {
            'timestamp': self.timestamp,
            'project_root': self.project_root,
            'total_files': self.total_files,
            'total_issues': self.total_issues,
            'issues_by_file': {k: [issue.to_dict() for issue in v] for k, v in self.issues_by_file.items()},
            'dependency_graph': {k: v.to_dict() for k, v in self.dependency_graph.items()},
            'circular_dependencies': [cd.to_dict() for cd in self.circular_dependencies],
            'statistics': self.statistics,
            'issues_by_severity': self.issues_by_severity,
            'issues_by_type': self.issues_by_type
        }


@dataclass
class FixResult:
    """修复结果"""
    file_path: str
    original_content: str
    new_content: str
    fixes_applied: List[IncludeIssue]
    backup_created: bool = False
    success: bool = True
    error_message: str = ""

    def to_dict(self) -> Dict[str, Any]:
        """转换为字典"""
        return {
            'file_path': self.file_path,
            'fixes_applied': len(self.fixes_applied),
            'backup_created': self.backup_created,
            'success': self.success,
            'error_message': self.error_message,
            'fixes': [fix.to_dict() for fix in self.fixes_applied]
        }


@dataclass
class BatchFixResult:
    """批量修复结果"""
    total_files: int
    successful_fixes: int
    failed_fixes: int
    total_fixes_applied: int
    results: List[FixResult] = field(default_factory=list)
    errors: List[str] = field(default_factory=list)

    def to_dict(self) -> Dict[str, Any]:
        """转换为字典"""
        return {
            'total_files': self.total_files,
            'successful_fixes': self.successful_fixes,
            'failed_fixes': self.failed_fixes,
            'total_fixes_applied': self.total_fixes_applied,
            'results': [result.to_dict() for result in self.results],
            'errors': self.errors
        }


@dataclass
class Config:
    """配置类"""
    project_name: str = "SQLCC"
    project_root: str = "."
    include_dirs: List[str] = field(default_factory=lambda: ["include"])
    src_dirs: List[str] = field(default_factory=lambda: ["src", "tests"])
    max_include_depth: int = 10
    enable_circular_detection: bool = True
    check_bazel_compatibility: bool = True
    enable_auto_fix: bool = False
    output_formats: List[str] = field(default_factory=lambda: ["json", "html", "cli"])
    report_dir: str = "reports/include_analysis"
    enable_summary: bool = True
    enable_details: bool = True
    backup_files: bool = True
    dry_run: bool = False
    max_fixes_per_file: int = 10
    require_confirmation: bool = True

    # 模块映射
    module_mappings: Dict[str, str] = field(default_factory=dict)

    # 标准库列表
    standard_libraries: List[str] = field(default_factory=list)

    # 已弃用头文件列表
    deprecated_headers: List[str] = field(default_factory=list)

    def to_dict(self) -> Dict[str, Any]:
        """转换为字典"""
        return {
            'project_name': self.project_name,
            'project_root': self.project_root,
            'include_dirs': self.include_dirs,
            'src_dirs': self.src_dirs,
            'max_include_depth': self.max_include_depth,
            'enable_circular_detection': self.enable_circular_detection,
            'check_bazel_compatibility': self.check_bazel_compatibility,
            'enable_auto_fix': self.enable_auto_fix,
            'output_formats': self.output_formats,
            'report_dir': self.report_dir,
            'enable_summary': self.enable_summary,
            'enable_details': self.enable_details,
            'backup_files': self.backup_files,
            'dry_run': self.dry_run,
            'max_fixes_per_file': self.max_fixes_per_file,
            'require_confirmation': self.require_confirmation,
            'module_mappings_count': len(self.module_mappings),
            'standard_libraries_count': len(self.standard_libraries),
            'deprecated_headers_count': len(self.deprecated_headers)
        }
