#!/usr/bin/env python3
"""
SQLCC 模块化迁移执行器
用于自动执行从include/src到模块化结构的迁移
"""

import os
import sys
import argparse
import shutil
import subprocess
from pathlib import Path
from typing import List, Dict, Set
import re


class MigrationExecutor:
    def __init__(self, project_root: str):
        self.project_root = Path(project_root)
        self.include_dir = self.project_root / "include"
        self.src_dir = self.project_root / "src"
        self.tools_dir = self.project_root / "tools"

    def analyze_dependencies(self, module_name: str) -> Dict[str, List[str]]:
        """分析模块的依赖关系"""
        print(f"分析模块 {module_name} 的依赖关系...")

        module_path = self.src_dir / module_name
        if not module_path.exists():
            print(f"错误：模块 {module_name} 不存在")
            return {}

        # 收集所有#include语句
        includes = []
        for cpp_file in module_path.rglob("*.cpp"):
            try:
                with open(cpp_file, 'r', encoding='utf-8') as f:
                    content = f.read()
                    # 查找#include语句
                    matches = re.findall(r'#include\s*["<]([^">]+)[">]', content)
                    includes.extend(matches)
            except Exception as e:
                print(f"警告：无法读取文件 {cpp_file}: {e}")

        for h_file in module_path.rglob("*.h"):
            try:
                with open(h_file, 'r', encoding='utf-8') as f:
                    content = f.read()
                    matches = re.findall(r'#include\s*["<]([^">]+)[">]', content)
                    includes.extend(matches)
            except Exception as e:
                print(f"警告：无法读取文件 {h_file}: {e}")

        # 分类依赖
        external_deps = []

        # 标准库头文件列表（常见C++标准库头文件）
        std_headers = {
            'algorithm', 'array', 'atomic', 'bitset', 'chrono', 'codecvt', 'complex',
            'condition_variable', 'deque', 'exception', 'execution', 'filesystem',
            'forward_list', 'fstream', 'functional', 'future', 'initializer_list',
            'iomanip', 'ios', 'iosfwd', 'iostream', 'istream', 'iterator', 'limits',
            'list', 'locale', 'map', 'memory', 'mutex', 'new', 'numeric', 'optional',
            'ostream', 'queue', 'random', 'ratio', 'regex', 'scoped_allocator',
            'set', 'shared_mutex', 'sstream', 'stack', 'stdexcept', 'streambuf',
            'string', 'string_view', 'strstream', 'system_error', 'thread', 'tuple',
            'type_traits', 'typeindex', 'typeinfo', 'unordered_map', 'unordered_set',
            'utility', 'valarray', 'variant', 'vector', 'any', 'barrier', 'bit',
            'charconv', 'compare', 'concepts', 'coroutine', 'format', 'latch',
            'numbers', 'ranges', 'semaphore', 'source_location', 'span', 'stop_token',
            'syncstream', 'version', 'cassert', 'cctype', 'cerrno', 'cfenv', 'cfloat',
            'cinttypes', 'ciso646', 'climits', 'clocale', 'cmath', 'csetjmp', 'csignal',
            'cstdalign', 'cstdarg', 'cstdbool', 'cstdint', 'cstdio', 'cstdlib', 'cstring',
            'ctgmath', 'ctime', 'cuchar', 'cwchar', 'cwctype'
        }

        for include in includes:
            if include.startswith(module_name + "/"):
                continue  # 内部依赖，忽略
            elif "/" in include:
                # 可能是其他模块的头文件
                dep_module = include.split("/")[0]
                if (self.src_dir / dep_module).exists():
                    external_deps.append(dep_module)
            # 忽略标准库头文件和全局头文件

        return {
            "internal": [],
            "external": list(set(external_deps))
        }

    def create_module_structure(self, module_name: str) -> bool:
        """创建模块的标准目录结构"""
        print(f"为模块 {module_name} 创建标准目录结构...")

        module_path = self.src_dir / module_name
        include_path = module_path / "include" / module_name
        src_path = module_path / "src"

        try:
            include_path.mkdir(parents=True, exist_ok=True)
            src_path.mkdir(parents=True, exist_ok=True)
            return True
        except Exception as e:
            print(f"错误：创建目录结构失败: {e}")
            return False

    def move_files(self, module_name: str) -> bool:
        """移动文件到新的模块结构"""
        print(f"移动模块 {module_name} 的文件...")

        # 源位置
        old_include_dir = self.include_dir / module_name
        old_src_dir = self.src_dir / module_name

        # 目标位置
        new_include_dir = self.src_dir / module_name / "include" / module_name
        new_src_dir = self.src_dir / module_name / "src"

        try:
            # 移动头文件
            if old_include_dir.exists():
                for header_file in old_include_dir.rglob("*.h"):
                    rel_path = header_file.relative_to(old_include_dir)
                    target_file = new_include_dir / rel_path
                    target_file.parent.mkdir(parents=True, exist_ok=True)
                    shutil.move(str(header_file), str(target_file))
                    print(f"移动头文件: {header_file} -> {target_file}")

            # 移动源文件（如果在src/module_name下且不是src/module_name/src/）
            if old_src_dir.exists():
                for src_file in old_src_dir.rglob("*.cpp"):
                    # 跳过已经在src/module_name/src/下的文件
                    if "src" + os.sep + module_name + os.sep + "src" in str(src_file):
                        continue

                    rel_path = src_file.relative_to(old_src_dir)
                    target_file = new_src_dir / rel_path
                    target_file.parent.mkdir(parents=True, exist_ok=True)
                    shutil.move(str(src_file), str(target_file))
                    print(f"移动源文件: {src_file} -> {target_file}")

            return True
        except Exception as e:
            print(f"错误：移动文件失败: {e}")
            return False

    def generate_build_file(self, module_name: str) -> bool:
        """生成模块的BUILD.bazel文件"""
        print(f"为模块 {module_name} 生成BUILD.bazel文件...")

        deps = self.analyze_dependencies(module_name)

        # 转换依赖为Bazel格式
        bazel_deps = []
        for dep in deps.get("external", []):
            if "/" in dep:
                dep_module = dep.split("/")[0]
                if (self.src_dir / dep_module).exists():
                    bazel_deps.append(f"//src/{dep_module}:{dep_module}")
            # 忽略全局依赖，因为我们现在是模块化结构

        all_deps = bazel_deps

        build_content = f'''cc_library(
    name = "{module_name}",
    srcs = glob(["src/**/*.cpp"]),
    hdrs = glob(["include/{module_name}/**/*.h"]),
    visibility = ["//visibility:public"],
    deps = [
{chr(10).join(f'        "{dep}",' for dep in all_deps)}
    ],
)
'''

        build_path = self.src_dir / module_name / "BUILD.bazel"
        try:
            with open(build_path, 'w', encoding='utf-8') as f:
                f.write(build_content)
            print(f"生成BUILD文件: {build_path}")
            return True
        except Exception as e:
            print(f"错误：生成BUILD文件失败: {e}")
            return False

    def update_references(self, module_name: str) -> bool:
        """更新代码中的引用"""
        print(f"更新模块 {module_name} 的引用...")

        # 更新#include语句
        old_pattern = f'#include <{module_name}/'
        new_pattern = f'#include "{module_name}/'

        updated_files = []

        # 搜索所有可能引用该模块的文件
        for root_dir in [self.src_dir, self.include_dir]:
            if root_dir.exists():
                for file_path in root_dir.rglob("*"):
                    if file_path.suffix in ['.cpp', '.h']:
                        try:
                            with open(file_path, 'r', encoding='utf-8') as f:
                                content = f.read()

                            if old_pattern in content:
                                new_content = content.replace(old_pattern, new_pattern)
                                with open(file_path, 'w', encoding='utf-8') as f:
                                    f.write(new_content)
                                updated_files.append(file_path)
                                print(f"更新文件: {file_path}")
                        except Exception as e:
                            print(f"警告：无法更新文件 {file_path}: {e}")

        # 更新BUILD文件中的依赖
        old_dep = f"//include:{module_name}"
        new_dep = f"//src/{module_name}:{module_name}"

        for build_file in self.project_root.rglob("BUILD.bazel"):
            try:
                with open(build_file, 'r', encoding='utf-8') as f:
                    content = f.read()

                if old_dep in content:
                    new_content = content.replace(old_dep, new_dep)
                    with open(build_file, 'w', encoding='utf-8') as f:
                        f.write(new_content)
                    updated_files.append(build_file)
                    print(f"更新BUILD文件: {build_file}")
            except Exception as e:
                print(f"警告：无法更新BUILD文件 {build_file}: {e}")

        return True

    def validate_migration(self, module_name: str) -> bool:
        """验证迁移结果"""
        print(f"验证模块 {module_name} 的迁移结果...")

        module_path = self.src_dir / module_name

        # 检查目录结构
        if not (module_path / "BUILD.bazel").exists():
            print("错误：BUILD.bazel文件不存在")
            return False

        if not (module_path / "include" / module_name).exists():
            print("错误：include目录结构不正确")
            return False

        if not (module_path / "src").exists():
            print("错误：src目录不存在")
            return False

        # 尝试编译
        try:
            result = subprocess.run(
                ["bazel", "build", f"//src/{module_name}:{module_name}"],
                cwd=self.project_root,
                capture_output=True,
                text=True,
                timeout=300
            )

            if result.returncode == 0:
                print("✓ 编译成功")
                return True
            else:
                print("✗ 编译失败")
                print("错误输出:", result.stderr)
                return False
        except subprocess.TimeoutExpired:
            print("✗ 编译超时")
            return False
        except Exception as e:
            print(f"✗ 编译执行失败: {e}")
            return False

    def migrate_module(self, module_name: str) -> bool:
        """执行完整的模块迁移"""
        print(f"\n开始迁移模块: {module_name}")
        print("=" * 50)

        steps = [
            ("创建目录结构", self.create_module_structure),
            ("移动文件", self.move_files),
            ("生成BUILD文件", self.generate_build_file),
            ("更新引用", self.update_references),
            ("验证迁移", self.validate_migration),
        ]

        for step_name, step_func in steps:
            print(f"\n执行步骤: {step_name}")
            if not step_func(module_name):
                print(f"步骤 '{step_name}' 失败，中止迁移")
                return False

        print(f"\n✓ 模块 {module_name} 迁移完成")
        return True


def main():
    parser = argparse.ArgumentParser(description="SQLCC 模块化迁移执行器")
    parser.add_argument("module", help="要迁移的模块名称")
    parser.add_argument("--project-root", default=".", help="项目根目录")

    args = parser.parse_args()

    executor = MigrationExecutor(args.project_root)

    if executor.migrate_module(args.module):
        print(f"\n🎉 模块 {args.module} 迁移成功完成！")
        sys.exit(0)
    else:
        print(f"\n❌ 模块 {args.module} 迁移失败！")
        sys.exit(1)


if __name__ == "__main__":
    main()
