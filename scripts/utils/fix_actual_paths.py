#!/usr/bin/env python3
"""
SQLCC 项目文档路径修复脚本

此脚本会扫描项目中的实际文件，并尝试修复文档中的路径引用
"""

import os
import re
import sys
from pathlib import Path
from typing import Dict, List, Tuple


def build_file_map() -> Dict[str, str]:
    """
    构建一个映射，从文件名到实际存在的完整路径
    """
    file_map = {}
    
    # 扫描所有源码文件
    for ext in ["*.h", "*.hpp", "*.cpp", "*.cc", "*.cxx"]:
        for file_path in Path('.').rglob(ext):
            # 获取文件名
            filename = file_path.name
            full_path = str(file_path)
            
            # 如果文件名已经存在映射，添加更具体的路径作为备选
            if filename in file_map:
                # 如果已有映射不够具体，用新的替换
                if len(full_path) < len(file_map[filename]):
                    file_map[f"{file_path.parent.name}/{filename}"] = full_path
                else:
                    file_map[f"{file_path.parent.name}/{filename}"] = full_path
            else:
                file_map[filename] = full_path
    
    return file_map


def find_most_similar_path(filename: str, file_map: Dict[str, str]) -> str:
    """
    根据文件名找到最相似的实际路径
    """
    if filename in file_map:
        return file_map[filename]
    
    # 尝试在file_map中找到相似的路径
    for mapped_filename, actual_path in file_map.items():
        if mapped_filename == filename or mapped_filename.split('/')[-1] == filename:
            return actual_path
    
    # 如果找不到精确匹配，尝试模糊匹配
    for mapped_filename, actual_path in file_map.items():
        if filename in mapped_filename or mapped_filename in filename:
            return actual_path
    
    return None


def fix_document_paths():
    """
    修复文档中的路径引用
    """
    file_map = build_file_map()
    fixes_applied = 0
    
    # 遍历所有文档文件
    for doc_file in Path('docs').rglob("*.md"):
        with open(doc_file, 'r', encoding='utf-8') as f:
            original_content = f.read()
        
        fixed_content = original_content
        
        # 找到所有可能的文件引用
        # 匹配 [text](path/to/file.ext) 和 `path/to/file.ext` 格式
        bracket_refs = re.findall(r'\[([^\]]+)\]\(([^)]+\.(?:h|hpp|cpp|cc|cxx))\)', original_content)
        backtick_refs = re.findall(r'`([a-zA-Z0-9_/-]+/\w+\.(?:h|hpp|cpp|cc|cxx))`', original_content)
        
        # 处理括号引用
        for text, path in bracket_refs:
            filename = path.split('/')[-1]
            actual_path = find_most_similar_path(filename, file_map)
            
            if actual_path and actual_path != path:
                # 更新链接
                old_link = f"[{text}]({path})"
                new_link = f"[{text}]({actual_path})"
                fixed_content = fixed_content.replace(old_link, new_link)
                print(f"Fixed: {path} -> {actual_path} in {doc_file}")
                fixes_applied += 1
        
        # 处理反引号引用
        for path in backtick_refs:
            filename = path.split('/')[-1]
            actual_path = find_most_similar_path(filename, file_map)
            
            if actual_path and actual_path != path:
                # 更新引用
                old_ref = f"`{path}`"
                new_ref = f"`{actual_path}`"
                fixed_content = fixed_content.replace(old_ref, new_ref)
                print(f"Fixed: {path} -> {actual_path} in {doc_file}")
                fixes_applied += 1
    
        # 如果内容被修改，写回文件
        if fixed_content != original_content:
            with open(doc_file, 'w', encoding='utf-8') as f:
                f.write(fixed_content)
    
    return fixes_applied


def main():
    print("开始修复SQLCC文档中的文件路径引用...")
    
    fixes_count = fix_document_paths()
    
    print(f"\n完成！共修复了 {fixes_count} 个文件路径引用")
    
    return 0


if __name__ == "__main__":
    sys.exit(main())