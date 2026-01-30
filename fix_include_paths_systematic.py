#!/usr/bin/env python3
"""
系统性修复include路径问题的脚本
将绝对路径include转换为相对路径include
"""

import os
import re
from pathlib import Path

def get_relative_path(from_file, to_file):
    """
    获取从from_file到to_file的相对路径
    """
    from_dir = Path(from_file).parent
    to_file_path = Path(to_file)
    
    try:
        relative_path = os.path.relpath(to_file_path, from_dir)
        return relative_path
    except ValueError:
        # 在不同驱动器上，返回绝对路径
        return str(to_file_path)

def find_header_file(header_name, src_root):
    """
    在src目录中查找头文件
    """
    header_name = header_name.strip('"')
    
    # 在src目录中搜索
    for root, dirs, files in os.walk(src_root):
        if header_name in files:
            return os.path.join(root, header_name)
    
    return None

def fix_include_in_file(file_path, src_root):
    """
    修复单个文件中的include路径
    """
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    original_content = content
    
    # 匹配 #include "src/xxx/yyy.h" 格式
    pattern = r'#include\s+"(src/[^"]+)"'
    
    def replace_include(match):
        old_include = match.group(1)
        header_path = os.path.join(src_root, old_include)
        
        if os.path.exists(header_path):
            # 获取相对路径
            relative_path = get_relative_path(file_path, header_path)
            return f'#include "{relative_path}"'
        else:
            # 文件不存在，尝试查找
            header_name = os.path.basename(old_include)
            found_path = find_header_file(header_name, src_root)
            if found_path:
                relative_path = get_relative_path(file_path, found_path)
                return f'#include "{relative_path}"'
        
        return match.group(0)
    
    # 替换include语句
    content = re.sub(pattern, replace_include, content)
    
    if content != original_content:
        with open(file_path, 'w', encoding='utf-8') as f:
            f.write(content)
        return True
    
    return False

def main():
    src_root = '/home/liying/sqlcc/src'
    
    # 查找所有.cpp和.h文件
    cpp_files = []
    h_files = []
    
    for root, dirs, files in os.walk(src_root):
        for file in files:
            file_path = os.path.join(root, file)
            if file.endswith('.cpp'):
                cpp_files.append(file_path)
            elif file.endswith('.h'):
                h_files.append(file_path)
    
    print(f"找到 {len(cpp_files)} 个 .cpp 文件")
    print(f"找到 {len(h_files)} 个 .h 文件")
    
    fixed_count = 0
    
    # 修复.cpp文件
    for file_path in cpp_files:
        if fix_include_in_file(file_path, src_root):
            fixed_count += 1
            print(f"已修复: {file_path}")
    
    # 修复.h文件
    for file_path in h_files:
        if fix_include_in_file(file_path, src_root):
            fixed_count += 1
            print(f"已修复: {file_path}")
    
    print(f"\n总计修复了 {fixed_count} 个文件")

if __name__ == '__main__':
    main()