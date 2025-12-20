#!/usr/bin/env python3
"""
Bazel标签路径修复工具

自动修复Bazel BUILD文件中的标签路径问题：
- //include:xxx.h -> //include/xxx:xxx.h
- //src:xxx.cpp -> //src/xxx:xxx.cpp
"""

import re
import sys
from pathlib import Path


class BazelLabelFixer:
    """Bazel标签修复器"""

    def __init__(self):
        self.fixed_count = 0
        self.errors = []

    def fix_file(self, file_path: str, dry_run: bool = False) -> bool:
        """修复单个BUILD文件"""
        if not Path(file_path).exists():
            self.errors.append(f"文件不存在: {file_path}")
            return False

        try:
            with open(file_path, 'r', encoding='utf-8') as f:
                content = f.read()
        except UnicodeDecodeError:
            self.errors.append(f"无法读取文件 (编码问题): {file_path}")
            return False

        original_content = content

        # 修复include标签
        content = self._fix_include_labels(content)

        # 修复src标签
        content = self._fix_src_labels(content)

        # 修复其他可能的标签问题
        content = self._fix_other_labels(content)

        if content != original_content:
            if not dry_run:
                try:
                    with open(file_path, 'w', encoding='utf-8') as f:
                        f.write(content)
                    print(f"✅ 已修复: {file_path}")
                except IOError as e:
                    self.errors.append(f"无法写入文件 {file_path}: {e}")
                    return False
            else:
                print(f"🔍 [DRY RUN] 需要修复: {file_path}")

            self.fixed_count += 1
            return True
        else:
            if dry_run:
                print(f"✅ 无需修复: {file_path}")
            return True

    def _fix_include_labels(self, content: str) -> str:
        """修复include标签路径"""
        # 匹配 //include:xxx.h 模式
        pattern = r'//include:([^/\s]+\.h[^"\s]*)'
        replacement = r'//include/\1:\1'

        # 使用函数进行更复杂的替换
        def replace_func(match):
            filename = match.group(1)
            # 提取路径部分 (去掉.h扩展名)
            if '.h' in filename:
                path_part = filename.split('.h')[0]
                return f'//include/{path_part}:{filename}'
            return match.group(0)

        return re.sub(pattern, replace_func, content)

    def _fix_src_labels(self, content: str) -> str:
        """修复src标签路径"""
        # 匹配 //src:xxx.cpp 模式
        pattern = r'//src:([^/\s]+\.cpp[^"\s]*)'
        replacement = r'//src/\1:\1'

        def replace_func(match):
            filename = match.group(1)
            # 提取路径部分 (去掉.cpp扩展名)
            if '.cpp' in filename:
                path_part = filename.split('.cpp')[0]
                return f'//src/{path_part}:{filename}'
            return match.group(0)

        return re.sub(pattern, replace_func, content)

    def _fix_other_labels(self, content: str) -> str:
        """修复其他可能的标签问题"""
        # 这里可以添加更多修复规则
        return content

    def fix_directory(self, dir_path: str, dry_run: bool = False) -> bool:
        """修复目录中的所有BUILD文件"""
        build_files = Path(dir_path).rglob("BUILD.bazel")

        success = True
        for build_file in build_files:
            if not self.fix_file(str(build_file), dry_run):
                success = False

        return success

    def report(self):
        """生成报告"""
        if self.errors:
            print("❌ 修复过程中出现错误:")
            for error in self.errors:
                print(f"  - {error}")

        print(f"📊 修复统计: 修复了 {self.fixed_count} 个文件")

        if not self.errors and self.fixed_count > 0:
            print("✅ 标签修复完成")
            return True
        elif not self.errors and self.fixed_count == 0:
            print("✅ 所有标签都已正确")
            return True
        else:
            print("❌ 修复过程中出现错误")
            return False


def main():
    if len(sys.argv) < 2:
        print("用法: python bazel_label_fixer.py <BUILD文件或目录> [--dry-run]")
        print("选项:")
        print("  --dry-run  只显示需要修复的文件，不实际修改")
        sys.exit(1)

    path = sys.argv[1]
    dry_run = "--dry-run" in sys.argv

    fixer = BazelLabelFixer()

    if Path(path).is_file():
        # 修复单个文件
        success = fixer.fix_file(path, dry_run)
    elif Path(path).is_dir():
        # 修复目录中的所有BUILD文件
        success = fixer.fix_directory(path, dry_run)
    else:
        print(f"路径不存在: {path}")
        sys.exit(1)

    success = fixer.report()
    sys.exit(0 if success else 1)


if __name__ == "__main__":
    main()
