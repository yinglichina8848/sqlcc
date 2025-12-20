#!/usr/bin/env python3
"""
Bazel代码质量检查工具

检查C++代码中的常见问题：
- 重复的函数定义
- 错误的单例模式实现
- 头文件依赖问题
"""

import re
import sys
from pathlib import Path
from typing import Dict, List, Set, Tuple


class BazelCodeChecker:
    """Bazel代码检查器"""

    def __init__(self):
        self.errors = []
        self.warnings = []

    def check_file(self, file_path: str) -> bool:
        """检查单个文件"""
        if not Path(file_path).exists():
            self.errors.append(f"文件不存在: {file_path}")
            return False

        try:
            with open(file_path, 'r', encoding='utf-8') as f:
                content = f.read()
        except UnicodeDecodeError:
            self.errors.append(f"无法读取文件 (编码问题): {file_path}")
            return False

        # 执行各种检查
        self._check_duplicate_functions(content, file_path)
        self._check_singleton_patterns(content, file_path)
        self._check_include_guards(content, file_path)
        self._check_template_specializations(content, file_path)

        return len(self.errors) == 0

    def _check_duplicate_functions(self, content: str, file_path: str):
        """检查重复的函数定义"""
        # 查找函数定义 (不包括声明)
        func_pattern = r'^(\w+(?:\s+\w+)*)\s+(\w+)\s*\([^)]*\)\s*(?:const)?\s*\{'
        functions = re.findall(func_pattern, content, re.MULTILINE)

        seen = {}
        for func in functions:
            return_type, func_name = func
            key = f"{return_type.strip()} {func_name}"

            if key in seen:
                self.errors.append(
                    f"重复的函数定义 '{key}' 在 {file_path}:{seen[key]} 和后续位置"
                )
            else:
                seen[key] = "multiple"

    def _check_singleton_patterns(self, content: str, file_path: str):
        """检查单例模式实现"""
        # 检查错误的智能指针单例
        bad_singleton_pattern = r'std::make_shared<(\w+)>\(\)'
        matches = re.findall(bad_singleton_pattern, content)
        for match in matches:
            if 'Singleton' in match or 'Manager' in match:
                self.warnings.append(
                    f"可能的单例模式问题: 使用std::make_shared创建 {match} "
                    f"可能需要public构造函数 ({file_path})"
                )

        # 检查正确的单例实现建议
        if 'static.*GetInstance()' in content and 'std::shared_ptr' in content:
            self.warnings.append(
                f"单例模式警告: 考虑使用原始指针而不是智能指针 "
                f"避免构造函数访问问题 ({file_path})"
            )

    def _check_include_guards(self, content: str, file_path: str):
        """检查头文件保护符"""
        if file_path.endswith('.h') or file_path.endswith('.hpp'):
            if not re.search(r'#ifndef\s+\w+', content):
                self.warnings.append(f"缺少头文件保护符: {file_path}")

    def _check_template_specializations(self, content: str, file_path: str):
        """检查模板特化"""
        # 查找模板显式实例化
        template_instantiation = r'template\s+\w+\s+\w+::\w+<'
        matches = re.finditer(template_instantiation, content)
        for match in matches:
            line_num = content[:match.start()].count('\n') + 1
            self.warnings.append(
                f"模板显式实例化可能导致链接问题 (行 {line_num}): {file_path}"
            )

    def check_duplicate_definitions_across_files(self, file_paths: List[str]):
        """检查跨文件的重复定义"""
        definitions = {}  # function_name -> [(file, line)]

        for file_path in file_paths:
            if not Path(file_path).exists():
                continue

            try:
                with open(file_path, 'r', encoding='utf-8') as f:
                    content = f.read()
            except UnicodeDecodeError:
                continue

            # 查找所有函数定义
            for match in re.finditer(
                r'^(\w+(?:\s+\w+)*)\s+(\w+)\s*\([^)]*\)\s*(?:const)?\s*\{',
                content,
                re.MULTILINE
            ):
                return_type = match.group(1).strip()
                func_name = match.group(2)
                line_num = content[:match.start()].count('\n') + 1

                key = f"{return_type} {func_name}"
                if key not in definitions:
                    definitions[key] = []
                definitions[key].append((file_path, line_num))

        # 检查重复定义
        for func_name, locations in definitions.items():
            if len(locations) > 1:
                self.errors.append(
                    f"函数 '{func_name}' 在多个文件中定义: " +
                    ", ".join(f"{f}:{l}" for f, l in locations)
                )

    def report(self):
        """生成报告"""
        if not self.errors and not self.warnings:
            print("✅ 代码检查通过")
            return True

        if self.errors:
            print("❌ 发现错误:")
            for error in self.errors:
                print(f"  - {error}")

        if self.warnings:
            print("⚠️  发现警告:")
            for warning in self.warnings:
                print(f"  - {warning}")

        return len(self.errors) == 0


def main():
    if len(sys.argv) < 2:
        print("用法: python bazel_code_checker.py <文件或目录> [--check-duplicates]")
        print("选项:")
        print("  --check-duplicates  检查跨文件的重复定义")
        sys.exit(1)

    checker = BazelCodeChecker()
    path = sys.argv[1]
    check_duplicates = "--check-duplicates" in sys.argv

    if Path(path).is_file():
        # 检查单个文件
        success = checker.check_file(path)
    elif Path(path).is_dir():
        # 检查目录中的所有C++文件
        cpp_files = []
        for ext in ['.cpp', '.cc', '.cxx', '.h', '.hpp', '.hxx']:
            cpp_files.extend(Path(path).rglob(f'*{ext}'))

        success = True
        for cpp_file in cpp_files:
            if not checker.check_file(str(cpp_file)):
                success = False

        if check_duplicates:
            checker.check_duplicate_definitions_across_files([str(f) for f in cpp_files])
    else:
        print(f"路径不存在: {path}")
        sys.exit(1)

    success = checker.report()
    sys.exit(0 if success else 1)


if __name__ == "__main__":
    main()
