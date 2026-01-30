#!/usr/bin/env python3
"""
批量修复头文件引用路径的脚本
将非 src/ 前缀的头文件引用更新为 src/ 前缀
"""

import os
import re
from pathlib import Path

def find_file(target_name, base_path):
    """查找目标文件的位置"""
    # 先在 src 目录下查找
    src_path = base_path / "src"
    for root, dirs, files in os.walk(src_path):
        if target_name in files:
            rel_path = os.path.relpath(os.path.join(root, target_name), base_path)
            return rel_path
    return None

def fix_header_paths(file_path, base_path):
    """修复单个文件中的头文件引用"""
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()

    original_content = content
    modified = False

    # 匹配 #include "xxx" 格式的引用
    pattern = r'^#include\s+"([^"]+)"'
    matches = re.findall(pattern, content, re.MULTILINE)

    for include_path in matches:
        # 跳过已经使用 src/ 前缀的引用
        if include_path.startswith("src/"):
            continue

        # 跳过相对路径引用（以 ../ 开头）
        if include_path.startswith("../"):
            continue

        # 跳过标准库和第三方库引用（以 .h 结尾但不在 src 目录下）
        if include_path.endswith(".h"):
            # 尝试查找文件
            target_file = find_file(include_path, base_path)
            if target_file:
                # 更新引用路径
                old_include = f'#include "{include_path}"'
                new_include = f'#include "{target_file}"'
                content = content.replace(old_include, new_include)
                modified = True
                print(f"  Fixed: {include_path} -> {target_file}")

    if modified:
        with open(file_path, 'w', encoding='utf-8') as f:
            f.write(content)
        return True
    return False

def main():
    base_path = Path("/home/liying/sqlcc")
    src_path = base_path / "src"

    if not src_path.exists():
        print(f"Error: {src_path} does not exist")
        return

    # 遍历所有 .h 和 .cpp 文件
    modified_count = 0
    total_count = 0

    for root, dirs, files in os.walk(src_path):
        for file in files:
            if file.endswith('.h') or file.endswith('.cpp'):
                file_path = os.path.join(root, file)
                total_count += 1
                if fix_header_paths(file_path, base_path):
                    modified_count += 1

    print(f"\nTotal files scanned: {total_count}")
    print(f"Files modified: {modified_count}")

if __name__ == "__main__":
    main()