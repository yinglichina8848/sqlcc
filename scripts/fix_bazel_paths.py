#!/usr/bin/env python3
"""
自动修复BUILD.bazel文件中的路径问题脚本
"""

import os
import re
from pathlib import Path

def fix_duplicate_paths(content):
    """修复重复路径前缀问题"""
    # 匹配类似 //include:include:include:include:include:... 的模式
    pattern = r'//include(?::include){2,}'
    def replace_duplicate(match):
        return "//include"
    
    content = re.sub(pattern, replace_duplicate, content)
    
    # 匹配类似 //include:include:include:include:include:.../path 的模式
    pattern_with_path = r'//include(?::include){2,}/([^"]+)'
    def replace_duplicate_with_path(match):
        return f"//include/{match.group(1)}"
    
    content = re.sub(pattern_with_path, replace_duplicate_with_path, content)
    
    return content

def fix_double_quotes(content):
    """修复双重引号问题"""
    # 匹配类似 ""file.h"" 的模式
    pattern = r'""([^"]+)""'
    def replace_double_quotes(match):
        return f'"{match.group(1)}"'
    
    content = re.sub(pattern, replace_double_quotes, content)
    return content

def fix_includes_paths(content):
    """修复includes路径问题"""
    # 将 //include 替换为 ../include
    content = re.sub(r'includes = \["//include"\]', 'includes = ["../include"]', content)
    # 将 includes = ["//include", ...] 中的 //include 替换为 ../include
    content = re.sub(r'"//include"(?=,|\s*\])', '"../include"', content)
    return content

def fix_deps_paths(content):
    """修复deps路径问题"""
    # 将 //include/storage_engine 替换为 //include:storage_engine
    content = re.sub(r'"//include/storage_engine"', '"//include:storage_engine"', content)
    # 处理其他类似的路径问题
    content = re.sub(r'"//include/([^:"][^"]*)"', r'"//include:\1"', content)
    return content

def process_build_file(file_path):
    """处理单个BUILD.bazel文件"""
    print(f"Processing {file_path}...")
    
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()
        
        original_content = content
        
        # 应用各种修复
        content = fix_duplicate_paths(content)
        content = fix_double_quotes(content)
        content = fix_includes_paths(content)
        content = fix_deps_paths(content)
        
        # 如果内容发生了变化，则写回文件
        if content != original_content:
            with open(file_path, 'w', encoding='utf-8') as f:
                f.write(content)
            print(f"  Fixed issues in {file_path}")
            return True
        else:
            print(f"  No issues found in {file_path}")
            return False
            
    except Exception as e:
        print(f"  Error processing {file_path}: {e}")
        return False

def find_build_files(root_dir):
    """查找所有BUILD.bazel文件"""
    build_files = []
    for root, dirs, files in os.walk(root_dir):
        for file in files:
            if file == "BUILD.bazel":
                build_files.append(os.path.join(root, file))
    return build_files

def main():
    """主函数"""
    root_dir = "/home/liying/sqlcc"
    print(f"Searching for BUILD.bazel files in {root_dir}...")
    
    build_files = find_build_files(root_dir)
    print(f"Found {len(build_files)} BUILD.bazel files")
    
    fixed_count = 0
    for build_file in build_files:
        if process_build_file(build_file):
            fixed_count += 1
    
    print(f"\nFixed {fixed_count} out of {len(build_files)} BUILD.bazel files")

if __name__ == "__main__":
    main()