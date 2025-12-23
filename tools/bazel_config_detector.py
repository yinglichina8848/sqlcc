#!/usr/bin/env python3
"""
SQLCC Bazel构建配置检测器
专门用于检测Bazel构建配置中的各种问题

功能特性:
- 全面扫描所有BUILD.bazel文件
- 检测标签引用问题、文件存在性、依赖关系等
- 生成结构化的检测结果
- 支持增量检测和缓存

作者: SQLCC AI Agent
版本: v1.2.6
更新时间: 2025-12-22
"""

import os
import re
import sys
import json
import hashlib
from pathlib import Path
from typing import Dict, List, Set, Tuple, Optional, Any
from dataclasses import dataclass, field, asdict
from enum import Enum
import time

class IssueType(Enum):
    LABEL_ERROR = "LABEL_ERROR"
    MISSING_FILE = "MISSING_FILE"
    MISSING_TARGET = "MISSING_TARGET"
    INVALID_PATH = "INVALID_PATH"
    DEPENDENCY_ERROR = "DEPENDENCY_ERROR"
    SYNTAX_ERROR = "SYNTAX_ERROR"
    CONFIGURATION_ERROR = "CONFIGURATION_ERROR"

class IssueSeverity(Enum):
    CRITICAL = "CRITICAL"
    HIGH = "HIGH"
    MEDIUM = "MEDIUM"
    LOW = "LOW"
    INFO = "INFO"

@dataclass
class DetectionIssue:
    file_path: str
    line_number: int
    issue_type: IssueType
    severity: IssueSeverity
    message: str
    context: str = ""
    suggestion: str = ""
    fixable: bool = False
    metadata: Dict[str, Any] = field(default_factory=dict)

@dataclass
class DetectionResult:
    project_root: Path
    scan_time: float
    total_files: int
    issues: List[DetectionIssue] = field(default_factory=list)
    file_hashes: Dict[str, str] = field(default_factory=dict)
    statistics: Dict[str, Any] = field(default_factory=dict)

    def to_dict(self) -> Dict[str, Any]:
        """转换为字典格式用于JSON序列化"""
        def serialize_issue(issue: DetectionIssue) -> Dict[str, Any]:
            return {
                "file_path": issue.file_path,
                "line_number": issue.line_number,
                "issue_type": issue.issue_type.value,
                "severity": issue.severity.value,
                "message": issue.message,
                "context": issue.context,
                "suggestion": issue.suggestion,
                "fixable": issue.fixable,
                "metadata": issue.metadata
            }

        return {
            "project_root": str(self.project_root),
            "scan_time": self.scan_time,
            "total_files": self.total_files,
            "issues": [serialize_issue(issue) for issue in self.issues],
            "file_hashes": self.file_hashes,
            "statistics": self.statistics
        }

    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> 'DetectionResult':
        """从字典创建实例"""
        issues = [DetectionIssue(**issue) for issue in data["issues"]]
        return cls(
            project_root=Path(data["project_root"]),
            scan_time=data["scan_time"],
            total_files=data["total_files"],
            issues=issues,
            file_hashes=data.get("file_hashes", {}),
            statistics=data.get("statistics", {})
        )

class BazelConfigDetector:
    """Bazel构建配置检测器"""

    def __init__(self, project_root: str, cache_file: Optional[str] = None):
        self.project_root = Path(project_root).resolve()
        self.cache_file = Path(cache_file) if cache_file else self.project_root / ".bazel_detection_cache.json"
        self.result = DetectionResult(
            project_root=self.project_root,
            scan_time=0.0,
            total_files=0
        )

    def detect_all(self, force_full_scan: bool = False) -> DetectionResult:
        """执行全面检测"""
        start_time = time.time()

        print("🔍 开始Bazel配置检测...")

        # 加载缓存
        cached_result = self._load_cache()
        if not force_full_scan and cached_result:
            print("📋 使用缓存结果，执行增量检测...")
            self.result = cached_result
            self._incremental_scan()
        else:
            print("🔄 执行完整检测...")
            self._full_scan()

        # 计算统计信息
        self._calculate_statistics()

        # 保存缓存
        self.result.scan_time = time.time() - start_time
        self._save_cache()

        print(f"✅ 检测完成，发现 {len(self.result.issues)} 个问题")
        return self.result

    def _full_scan(self):
        """执行完整扫描"""
        build_files = self._find_build_files()
        self.result.total_files = len(build_files)

        print(f"📁 发现 {len(build_files)} 个BUILD.bazel文件")

        for build_file in build_files:
            self._detect_file(build_file)

    def _incremental_scan(self):
        """执行增量扫描"""
        build_files = self._find_build_files()
        current_hashes = self._calculate_file_hashes(build_files)

        # 检查哪些文件发生了变化
        changed_files = []
        for file_path, current_hash in current_hashes.items():
            cached_hash = self.result.file_hashes.get(file_path)
            if cached_hash != current_hash:
                changed_files.append(Path(file_path))

        print(f"🔄 {len(changed_files)} 个文件发生变化，需要重新检测")

        # 移除旧的问题
        changed_file_strs = [str(f) for f in changed_files]
        self.result.issues = [
            issue for issue in self.result.issues
            if issue.file_path not in changed_file_strs
        ]

        # 重新检测变更的文件
        for build_file in changed_files:
            self._detect_file(build_file)

        # 更新哈希
        self.result.file_hashes = current_hashes
        self.result.total_files = len(build_files)

    def _find_build_files(self) -> List[Path]:
        """查找所有BUILD.bazel文件"""
        build_files = []
        for root, dirs, files in os.walk(self.project_root):
            # 跳过某些目录
            dirs[:] = [d for d in dirs if not d.startswith('.') and d not in ['bazel-bin', 'bazel-out', 'bazel-testlogs']]

            for file in files:
                if file == "BUILD.bazel":
                    build_files.append(Path(root) / file)
        return build_files

    def _detect_file(self, build_file: Path):
        """检测单个BUILD文件"""
        try:
            with open(build_file, 'r', encoding='utf-8') as f:
                content = f.read()

            lines = content.split('\n')

            # 执行各种检测
            self._detect_label_issues(build_file, content, lines)
            self._detect_file_issues(build_file, content, lines)
            self._detect_dependency_issues(build_file, content, lines)
            self._detect_syntax_issues(build_file, content, lines)
            self._detect_config_issues(build_file, content, lines)

        except Exception as e:
            # 文件读取错误
            self._add_issue(build_file, 0, IssueType.SYNTAX_ERROR,
                          IssueSeverity.CRITICAL,
                          f"无法读取BUILD文件: {str(e)}",
                          "检查文件权限和编码格式",
                          fixable=False)

    def _detect_label_issues(self, build_file: Path, content: str, lines: List[str]):
        """检测标签引用问题"""
        # 查找所有标签引用
        label_pattern = r'"//([^"]+)"'
        matches = re.finditer(label_pattern, content)

        for match in matches:
            label = match.group(0)
            line_num = content[:match.start()].count('\n') + 1

            # 检查是否包含上层引用
            if '../' in label:
                self._add_issue(build_file, line_num, IssueType.LABEL_ERROR,
                              IssueSeverity.MEDIUM,
                              f"标签包含上层引用: {label}",
                              "使用正确的包路径引用",
                              context=lines[line_num-1].strip(),
                              fixable=True,
                              metadata={"label": label, "type": "relative_path"})

            # 检查标签格式
            if not re.match(r'"//[a-zA-Z_][a-zA-Z0-9_]*(/[a-zA-Z_][a-zA-Z0-9_]*)*:[a-zA-Z_][a-zA-Z0-9_]*"', label):
                self._add_issue(build_file, line_num, IssueType.LABEL_ERROR,
                              IssueSeverity.MEDIUM,
                              f"标签格式不规范: {label}",
                              "使用标准格式: //package:target",
                              context=lines[line_num-1].strip(),
                              metadata={"label": label, "type": "format_error"})

    def _detect_file_issues(self, build_file: Path, content: str, lines: List[str]):
        """检测文件存在性问题"""
        # 检查srcs和hdrs中的文件引用
        file_refs = re.findall(r'srcs\s*=\s*\[([^\]]*)\]|hdrs\s*=\s*\[([^\]]*)\]', content, re.DOTALL)

        for refs in file_refs:
            for ref_group in refs:
                if ref_group:
                    files = re.findall(r'"([^"]*)"', ref_group)
                    for file_ref in files:
                        if '../' in file_ref:
                            # 找到对应的行号
                            line_num = 0
                            for i, line in enumerate(lines):
                                if file_ref in line:
                                    line_num = i + 1
                                    break

                            self._add_issue(build_file, line_num, IssueType.INVALID_PATH,
                                          IssueSeverity.MEDIUM,
                                          f"文件路径包含上层引用: {file_ref}",
                                          "使用相对于包的路径",
                                          context=lines[line_num-1].strip() if line_num > 0 else "",
                                          fixable=True,
                                          metadata={"file_ref": file_ref, "type": "relative_path"})

                        # 检查文件是否存在
                        if not self._file_exists_in_package(build_file.parent, file_ref):
                            line_num = 0
                            for i, line in enumerate(lines):
                                if file_ref in line:
                                    line_num = i + 1
                                    break

                            self._add_issue(build_file, line_num, IssueType.MISSING_FILE,
                                          IssueSeverity.HIGH,
                                          f"引用的文件不存在: {file_ref}",
                                          "检查文件路径或创建缺失的文件",
                                          fixable=False,
                                          metadata={"file_ref": file_ref, "type": "missing_file"})

    def _detect_dependency_issues(self, build_file: Path, content: str, lines: List[str]):
        """检测依赖关系问题"""
        # 查找deps声明
        deps_pattern = r'deps\s*=\s*\[([^\]]*)\]'
        deps_match = re.search(deps_pattern, content, re.DOTALL)

        if deps_match:
            deps_content = deps_match.group(1)
            deps = re.findall(r'"([^"]*)"', deps_content)

            for dep in deps:
                if dep.startswith('//'):
                    # 检查目标是否存在
                    if not self._target_exists(dep):
                        line_num = content.find(dep)
                        line_num = content[:line_num].count('\n') + 1 if line_num >= 0 else 0
                        self._add_issue(build_file, line_num, IssueType.MISSING_TARGET,
                                      IssueSeverity.HIGH,
                                      f"依赖目标不存在: {dep}",
                                      "检查目标名称或创建缺失的目标",
                                      fixable=False,
                                      metadata={"dependency": dep, "type": "missing_target"})

    def _detect_syntax_issues(self, build_file: Path, content: str, lines: List[str]):
        """检测语法问题"""
        # 检查基本的Python语法问题
        if 'load(' in content and not content.count('(') == content.count(')'):
            self._add_issue(build_file, 0, IssueType.SYNTAX_ERROR,
                          IssueSeverity.HIGH,
                          "括号不匹配",
                          "检查load语句的括号匹配",
                          fixable=False,
                          metadata={"type": "bracket_mismatch"})

        # 检查未闭合的字符串
        quote_count = content.count('"') + content.count("'")
        if quote_count % 2 != 0:
            self._add_issue(build_file, 0, IssueType.SYNTAX_ERROR,
                          IssueSeverity.CRITICAL,
                          "字符串引号不匹配",
                          "检查字符串引号的配对",
                          fixable=False,
                          metadata={"type": "quote_mismatch"})

    def _detect_config_issues(self, build_file: Path, content: str, lines: List[str]):
        """检测配置问题"""
        # 检查includes路径
        includes_pattern = r'includes\s*=\s*\[([^\]]*)\]'
        includes_match = re.search(includes_pattern, content, re.DOTALL)

        if includes_match:
            includes_content = includes_match.group(1)
            includes = re.findall(r'"([^"]*)"', includes_content)

            for include_path in includes:
                if include_path == "//include":
                    line_num = content.find(include_path)
                    line_num = content[:line_num].count('\n') + 1 if line_num >= 0 else 0
                    self._add_issue(build_file, line_num, IssueType.CONFIGURATION_ERROR,
                                  IssueSeverity.MEDIUM,
                                  f"不正确的include路径: {include_path}",
                                  "使用相对路径: '../../include'",
                                  context=lines[line_num-1].strip() if line_num > 0 else "",
                                  fixable=True,
                                  metadata={"include_path": include_path, "type": "invalid_include"})

    def _target_exists(self, target_label: str) -> bool:
        """检查目标是否存在"""
        # 简单的存在性检查
        if ':' in target_label:
            package_path, target_name = target_label[2:].split(':', 1)
            build_file = self.project_root / package_path / "BUILD.bazel"
            if build_file.exists():
                try:
                    with open(build_file, 'r') as f:
                        content = f.read()
                        # 检查目标名称是否存在
                        if f'name = "{target_name}"' in content or f"name = '{target_name}'" in content:
                            return True
                except:
                    pass
        return False

    def _file_exists_in_package(self, package_dir: Path, file_ref: str) -> bool:
        """检查文件在包中是否存在"""
        file_path = package_dir / file_ref
        return file_path.exists()

    def _add_issue(self, build_file: Path, line_num: int, issue_type: IssueType,
                   severity: IssueSeverity, message: str, suggestion: str = "",
                   context: str = "", fixable: bool = False, metadata: Optional[Dict[str, Any]] = None):
        """添加问题到结果"""
        issue = DetectionIssue(
            file_path=str(build_file),
            line_number=line_num,
            issue_type=issue_type,
            severity=severity,
            message=message,
            context=context,
            suggestion=suggestion,
            fixable=fixable,
            metadata=metadata or {}
        )
        self.result.issues.append(issue)

    def _calculate_file_hashes(self, files: List[Path]) -> Dict[str, str]:
        """计算文件哈希"""
        hashes = {}
        for file_path in files:
            try:
                with open(file_path, 'rb') as f:
                    content = f.read()
                    hashes[str(file_path)] = hashlib.md5(content).hexdigest()
            except:
                hashes[str(file_path)] = ""
        return hashes

    def _calculate_statistics(self):
        """计算统计信息"""
        stats = {
            "total_issues": len(self.result.issues),
            "issues_by_type": {},
            "issues_by_severity": {},
            "fixable_issues": 0,
            "critical_issues": 0
        }

        for issue in self.result.issues:
            # 按类型统计
            type_key = issue.issue_type.value
            stats["issues_by_type"][type_key] = stats["issues_by_type"].get(type_key, 0) + 1

            # 按严重程度统计
            severity_key = issue.severity.value
            stats["issues_by_severity"][severity_key] = stats["issues_by_severity"].get(severity_key, 0) + 1

            # 可修复问题计数
            if issue.fixable:
                stats["fixable_issues"] += 1

            # 严重问题计数
            if issue.severity in [IssueSeverity.CRITICAL, IssueSeverity.HIGH]:
                stats["critical_issues"] += 1

        self.result.statistics = stats

    def _save_cache(self):
        """保存检测结果到缓存"""
        try:
            cache_data = self.result.to_dict()
            with open(self.cache_file, 'w', encoding='utf-8') as f:
                json.dump(cache_data, f, indent=2, ensure_ascii=False)
        except Exception as e:
            print(f"⚠️  保存缓存失败: {e}")

    def _load_cache(self) -> Optional[DetectionResult]:
        """从缓存加载检测结果"""
        if not self.cache_file.exists():
            return None

        try:
            with open(self.cache_file, 'r', encoding='utf-8') as f:
                cache_data = json.load(f)
            return DetectionResult.from_dict(cache_data)
        except Exception as e:
            print(f"⚠️  加载缓存失败: {e}")
            return None

def main():
    """主函数"""
    import argparse

    parser = argparse.ArgumentParser(description="SQLCC Bazel配置检测器")
    parser.add_argument("project_root", help="项目根目录")
    parser.add_argument("--force", action="store_true", help="强制执行完整检测")
    parser.add_argument("--output", "-o", help="输出文件路径")
    parser.add_argument("--format", choices=["json", "text"], default="text", help="输出格式")

    args = parser.parse_args()

    if not os.path.isdir(args.project_root):
        print(f"❌ 错误: 目录不存在: {args.project_root}")
        sys.exit(1)

    # 创建检测器
    detector = BazelConfigDetector(args.project_root)

    # 执行检测
    result = detector.detect_all(force_full_scan=args.force)

    # 输出结果
    if args.format == "json":
        output_data = result.to_dict()
        if args.output:
            with open(args.output, 'w', encoding='utf-8') as f:
                json.dump(output_data, f, indent=2, ensure_ascii=False)
            print(f"📄 检测结果已保存到: {args.output}")
        else:
            print(json.dumps(output_data, indent=2, ensure_ascii=False))
    else:
        # 文本格式输出
        print(f"\n📊 检测结果摘要")
        print(f"项目根目录: {result.project_root}")
        print(f"扫描耗时: {result.scan_time:.2f}秒")
        print(f"BUILD文件数量: {result.total_files}")
        print(f"发现问题总数: {result.statistics['total_issues']}")

        if result.statistics.get('issues_by_severity'):
            print(f"\n按严重程度统计:")
            for severity, count in result.statistics['issues_by_severity'].items():
                print(f"  {severity}: {count}")

        if result.statistics.get('issues_by_type'):
            print(f"\n按问题类型统计:")
            for issue_type, count in result.statistics['issues_by_type'].items():
                print(f"  {issue_type}: {count}")

        print(f"\n可修复问题: {result.statistics.get('fixable_issues', 0)}")
        print(f"严重问题: {result.statistics.get('critical_issues', 0)}")

        # 显示前几个问题
        if result.issues:
            print(f"\n🔍 前5个问题:")
            for i, issue in enumerate(result.issues[:5], 1):
                print(f"{i}. [{issue.severity.value}] {issue.message}")
                print(f"   文件: {issue.file_path}:{issue.line_number}")
                if issue.suggestion:
                    print(f"   建议: {issue.suggestion}")
                print()

    return 0

if __name__ == "__main__":
    sys.exit(main())
