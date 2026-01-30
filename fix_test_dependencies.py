#!/usr/bin/env python3
"""
修复测试依赖中的 include/ 路径，将其更新为 src/ 路径
"""
import os
import re
from pathlib import Path

def fix_bazel_file(file_path):
    """修复单个BUILD.bazel文件中的依赖路径"""
    try:
        with open(file_path, 'rb') as f:
            content = f.read()

        # 尝试多种编码方式
        try:
            text_content = content.decode('utf-8')
        except UnicodeDecodeError:
            try:
                text_content = content.decode('latin-1')
            except UnicodeDecodeError:
                # 如果都失败，使用utf-8并忽略错误
                text_content = content.decode('utf-8', errors='ignore')

        original_content = text_content

        # 替换 //include/... 为 //src/...
        # 这个正则表达式匹配所有以 //include/ 开头的依赖路径
        text_content = re.sub(r'"//include/([^"]+)"', r'"//src/\1"', text_content)
        text_content = re.sub(r'"//include:([^"]+)"', r'"//src:\1"', text_content)

        # 特殊处理一些需要映射的模块
        mappings = {
            r'"//src/exception:base_exception"': '"//src/exception:exception"',
        }

        for old, new in mappings.items():
            text_content = text_content.replace(old, new)

        if text_content != original_content:
            with open(file_path, 'w', encoding='utf-8') as f:
                f.write(text_content)
            print(f"已修复: {file_path}")
            return True
        return False
    except Exception as e:
        print(f"处理文件时出错 {file_path}: {e}")
        return False

def find_and_fix_test_build_files():
    """查找并修复所有测试目录的BUILD.bazel文件"""
    tests_dir = Path('/home/liying/sqlcc/tests')

    if not tests_dir.exists():
        print(f"错误: 测试目录不存在: {tests_dir}")
        return

    fixed_count = 0

    # 递归查找所有BUILD.bazel文件
    for build_file in tests_dir.rglob('BUILD.bazel'):
        if fix_bazel_file(build_file):
            fixed_count += 1

    print(f"\n总共修复了 {fixed_count} 个BUILD.bazel文件")

if __name__ == '__main__':
    find_and_fix_test_build_files()