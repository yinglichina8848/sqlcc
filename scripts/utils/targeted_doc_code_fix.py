#!/usr/bin/env python3
"""
SQLCC 项目文档-代码一致性针对性修复脚本

此脚本专注于解决文档与代码实现之间的核心一致性问题
"""

import os
import re
import sys
from pathlib import Path
from typing import List, Dict, Tuple, Set


def create_file_mapping():
    """创建一个从常用名称到实际路径的映射"""
    mapping = {}
    
    # 遍历源代码目录，建立文件名到路径的映射
    for ext in ["*.h", "*.hpp", "*.cpp", "*.cc", "*.cxx"]:
        for file_path in Path('.').rglob(ext):
            filename = file_path.name
            full_path = str(file_path).replace('./', '')
            
            # 仅处理存在于src或include目录下的文件
            if full_path.startswith(('src/', 'include/')):
                # 用文件名作为键，完整路径作为值
                if filename not in mapping:
                    mapping[filename] = full_path
                else:
                    # 如果已经有相同文件名的映射，选择更具体的路径
                    if len(full_path) < len(mapping[filename]):
                        mapping[filename] = full_path
    
    return mapping


def fix_obvious_path_errors():
    """修复明显的路径错误"""
    file_mapping = create_file_mapping()
    fixes_count = 0
    
    # 定义一些常见的错误模式和修复规则
    fix_patterns = [
        # 修复 include/ 下的头文件
        (r'\b(include/[a-zA-Z0-9_]+\.h(?:pp)?)\b', lambda m: file_mapping.get(m.group(1).split('/')[-1], m.group(1))),
        # 修复 src/ 下的源文件
        (r'\b(src/[a-zA-Z0-9_]+\.cpp)\b', lambda m: file_mapping.get(m.group(1).split('/')[-1], m.group(1))),
        # 修复 /home/user/path 格式到相对路径
        (r'/home/[^/]+/[^/]+/(src/.+)', lambda m: m.group(1)),
        (r'/home/[^/]+/[^/]+/(include/.+)', lambda m: m.group(1)),
    ]
    
    # 处理文档文件
    for doc_file in Path('docs').rglob("*.md"):
        with open(doc_file, 'r', encoding='utf-8') as f:
            original_content = f.read()
        
        fixed_content = original_content
        
        # 应用修复模式
        for pattern, replacer in fix_patterns:
            matches = re.finditer(pattern, fixed_content)
            for match in matches:
                matched_path = match.group(1)
                if matched_path in file_mapping:
                    actual_path = file_mapping[matched_path]
                    # 替换整个匹配项
                    fixed_content = re.sub(
                        pattern,
                        lambda m: m.group(0).replace(matched_path, actual_path),
                        fixed_content
                    )
                    fixes_count += 1
    
        # 也处理括号和反引号格式的链接
        for doc_file in Path('docs').rglob("*.md"):
            with open(doc_file, 'r', encoding='utf-8') as f:
                original_content = f.read()
            
            fixed_content = original_content
            
            # 处理 [text](path) 格式的链接
            bracket_links = re.findall(r'\[([^\]]+)\]\(([^)]+)\)', original_content)
            for text, path in bracket_links:
                # 检查路径是否指向不存在的文件
                if path.endswith(('.h', '.hpp', '.cpp', '.cc', '.cxx')):
                    filename = path.split('/')[-1]
                    if filename in file_mapping and path != file_mapping[filename]:
                        old_link = f"[{text}]({path})"
                        new_link = f"[{text}]({file_mapping[filename]})"
                        fixed_content = fixed_content.replace(old_link, new_link)
                        fixes_count += 1
            
            # 如果内容有变化，写回文件
            if fixed_content != original_content:
                with open(doc_file, 'w', encoding='utf-8') as f:
                    f.write(fixed_content)
    
    return fixes_count


def main():
    """主函数，执行针对性修复"""
    print("开始执行SQLCC文档-代码一致性针对性修复...")
    
    print("\n正在修复明显的路径错误...")
    fixes_count = fix_obvious_path_errors()
    
    print(f"已完成修复，共修正了 {fixes_count} 个路径引用")
    
    # 创建一个简短的总结报告
    report_dir = Path('docs/consistency_reports')
    report_dir.mkdir(exist_ok=True)
    
    with open(report_dir / 'targeted_fix_summary.md', 'w', encoding='utf-8') as f:
        f.write("# 文档-代码一致性针对性修复总结\n\n")
        f.write(f"本次修复共修正了 {fixes_count} 个明显的路径错误。\n\n")
        f.write("修复主要包括：\n")
        f.write("- 将错误的 include/ 文件路径修正为实际存在的路径\n")
        f.write("- 将错误的 src/ 文件路径修正为实际存在的路径\n")
        f.write("- 将绝对路径转换为相对路径\n")
        f.write("- 修正 [text](path) 格式的链接到正确路径\n\n")
        f.write("这解决了文档中引用代码文件路径不一致的主要问题。\n")
    
    print(f"修复总结已保存到: {report_dir}/targeted_fix_summary.md")
    
    return 0


if __name__ == "__main__":
    sys.exit(main())