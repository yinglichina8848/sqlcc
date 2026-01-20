#!/usr/bin/env python3
"""
SQLCC 依赖关系修复器
用于自动修复模块迁移后的依赖关系问题
"""

import os
import sys
import argparse
import re
from pathlib import Path
from typing import List, Dict, Set, Tuple


class DependencyFixer:
    def __init__(self, project_root: str):
        self.project_root = Path(project_root)
        self.include_dir = self.project_root / "include"
        self.src_dir = self.project_root / "src"

    def find_broken_includes(self) -> List[Tuple[Path, str]]:
        """查找所有损坏的include引用"""
        broken_includes = []

        print("扫描所有损坏的include引用...")

        for root_dir in [self.src_dir, self.include_dir]:
            if not root_dir.exists():
                continue

            for file_path in root_dir.rglob("*"):
                if file_path.suffix not in ['.cpp', '.h']:
                    continue

                try:
                    with open(file_path, 'r', encoding='utf-8') as f:
                        content = f.read()

                    # 查找#include <module/...> 模式
                    matches = re.findall(r'#include\s*<([^>]+)>', content)
                    for include in matches:
                        if "/" in include:
                            module_name = include.split("/")[0]
                            # 检查是否是已迁移的模块
                            module_path = self.src_dir / module_name
                            if module_path.exists() and (module_path / "BUILD.bazel").exists():
                                broken_includes.append((file_path, include))

                except Exception as e:
                    print(f"警告：无法读取文件 {file_path}: {e}")

        return broken_includes

    def fix_include_statements(self, broken_includes: List[Tuple[Path, str]]) -> bool:
        """修复include语句"""
        print(f"修复 {len(broken_includes)} 个损坏的include语句...")

        fixed_files = set()

        for file_path, include in broken_includes:
            try:
                with open(file_path, 'r', encoding='utf-8') as f:
                    content = f.read()

                # 将 <module/header.h> 改为 "module/header.h"
                old_pattern = f'#include <{include}>'
                new_pattern = f'#include "{include}"'

                if old_pattern in content:
                    new_content = content.replace(old_pattern, new_pattern)
                    with open(file_path, 'w', encoding='utf-8') as f:
                        f.write(new_content)
                    fixed_files.add(file_path)
                    print(f"修复: {file_path} - {include}")

            except Exception as e:
                print(f"错误：无法修复文件 {file_path}: {e}")
                return False

        print(f"成功修复 {len(fixed_files)} 个文件")
        return True

    def find_broken_build_deps(self) -> List[Tuple[Path, str]]:
        """查找损坏的BUILD文件依赖"""
        broken_deps = []

        print("扫描所有损坏的BUILD文件依赖...")

        for build_file in self.project_root.rglob("BUILD.bazel"):
            try:
                with open(build_file, 'r', encoding='utf-8') as f:
                    content = f.read()

                # 查找旧的include依赖
                matches = re.findall(r'//include/([^:\s]+)', content)
                for module_name in matches:
                    module_path = self.src_dir / module_name
                    if module_path.exists() and (module_path / "BUILD.bazel").exists():
                        broken_deps.append((build_file, f"//include/{module_name}"))

            except Exception as e:
                print(f"警告：无法读取BUILD文件 {build_file}: {e}")

        return broken_deps

    def fix_build_dependencies(self, broken_deps: List[Tuple[Path, str]]) -> bool:
        """修复BUILD文件依赖"""
        print(f"修复 {len(broken_deps)} 个损坏的BUILD依赖...")

        fixed_files = set()

        for build_file, old_dep in broken_deps:
            try:
                with open(build_file, 'r', encoding='utf-8') as f:
                    content = f.read()

                # 提取模块名
                module_name = old_dep.split('/')[-1]
                new_dep = f"//src/{module_name}:{module_name}"

                if old_dep in content:
                    new_content = content.replace(old_dep, new_dep)
                    with open(build_file, 'w', encoding='utf-8') as f:
                        f.write(new_content)
                    fixed_files.add(build_file)
                    print(f"修复BUILD: {build_file} - {old_dep} -> {new_dep}")

            except Exception as e:
                print(f"错误：无法修复BUILD文件 {build_file}: {e}")
                return False

        print(f"成功修复 {len(fixed_files)} 个BUILD文件")
        return True

    def validate_fixes(self) -> bool:
        """验证修复结果"""
        print("验证修复结果...")

        # 检查是否还有损坏的include
        remaining_broken = self.find_broken_includes()
        if remaining_broken:
            print(f"警告：仍存在 {len(remaining_broken)} 个损坏的include")
            for file_path, include in remaining_broken[:5]:  # 只显示前5个
                print(f"  {file_path}: {include}")
            return False

        # 检查是否还有损坏的BUILD依赖
        remaining_deps = self.find_broken_build_deps()
        if remaining_deps:
            print(f"警告：仍存在 {len(remaining_deps)} 个损坏的BUILD依赖")
            for build_file, dep in remaining_deps[:5]:  # 只显示前5个
                print(f"  {build_file}: {dep}")
            return False

        print("✓ 所有依赖关系修复完成")
        return True

    def generate_dependency_report(self) -> str:
        """生成依赖关系报告"""
        print("生成依赖关系报告...")

        report = []
        report.append("# SQLCC 依赖关系报告")
        report.append("")

        # 统计信息
        total_files = 0
        total_includes = 0

        for root_dir in [self.src_dir, self.include_dir]:
            if root_dir.exists():
                for file_path in root_dir.rglob("*"):
                    if file_path.suffix in ['.cpp', '.h']:
                        total_files += 1
                        try:
                            with open(file_path, 'r', encoding='utf-8') as f:
                                content = f.read()
                                includes = re.findall(r'#include\s*["<]([^">]+)[">]', content)
                                total_includes += len(includes)
                        except:
                            pass

        report.append(f"## 统计信息")
        report.append(f"- 总文件数: {total_files}")
        report.append(f"- 总include数: {total_includes}")
        report.append("")

        # 模块依赖图
        report.append("## 模块依赖关系")
        modules = []
        for module_dir in self.src_dir.iterdir():
            if module_dir.is_dir() and (module_dir / "BUILD.bazel").exists():
                modules.append(module_dir.name)

        for module in sorted(modules):
            deps = self._get_module_deps(module)
            if deps:
                report.append(f"- {module}: {', '.join(sorted(deps))}")
            else:
                report.append(f"- {module}: (无依赖)")

        return "\n".join(report)

    def _get_module_deps(self, module_name: str) -> Set[str]:
        """获取模块的依赖"""
        deps = set()
        build_file = self.src_dir / module_name / "BUILD.bazel"

        if build_file.exists():
            try:
                with open(build_file, 'r', encoding='utf-8') as f:
                    content = f.read()

                # 查找deps中的其他模块
                matches = re.findall(r'//src/([^:/]+):', content)
                for dep in matches:
                    if dep != module_name:  # 排除自依赖
                        deps.add(dep)
            except:
                pass

        return deps

    def fix_all_dependencies(self) -> bool:
        """修复所有依赖关系"""
        print("开始修复所有依赖关系...")
        print("=" * 50)

        steps = [
            ("查找损坏的include", lambda: (True, self.find_broken_includes())),
            ("修复include语句", lambda: (self.fix_include_statements(self.find_broken_includes()), None)),
            ("查找损坏的BUILD依赖", lambda: (True, self.find_broken_build_deps())),
            ("修复BUILD依赖", lambda: (self.fix_build_dependencies(self.find_broken_build_deps()), None)),
            ("验证修复结果", lambda: (self.validate_fixes(), None)),
        ]

        for step_name, step_func in steps:
            print(f"\n执行步骤: {step_name}")
            success, data = step_func()
            if not success:
                print(f"步骤 '{step_name}' 失败")
                return False

        print("\n✓ 所有依赖关系修复完成")
        return True


def main():
    parser = argparse.ArgumentParser(description="SQLCC 依赖关系修复器")
    parser.add_argument("--project-root", default=".", help="项目根目录")
    parser.add_argument("--generate-report", action="store_true", help="生成依赖关系报告")
    parser.add_argument("--validate-only", action="store_true", help="仅验证，不修复")

    args = parser.parse_args()

    fixer = DependencyFixer(args.project_root)

    if args.validate_only:
        success = fixer.validate_fixes()
        if success:
            print("\n🎉 所有依赖关系正常")
            sys.exit(0)
        else:
            print("\n❌ 发现依赖关系问题")
            sys.exit(1)

    if args.generate_report:
        report = fixer.generate_dependency_report()
        report_path = Path(args.project_root) / "dependency_report.md"
        with open(report_path, 'w', encoding='utf-8') as f:
            f.write(report)
        print(f"依赖关系报告已生成: {report_path}")
        return

    if fixer.fix_all_dependencies():
        print("\n🎉 依赖关系修复成功完成！")
        sys.exit(0)
    else:
        print("\n❌ 依赖关系修复失败！")
        sys.exit(1)


if __name__ == "__main__":
    main()
