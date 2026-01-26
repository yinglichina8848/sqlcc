#!/usr/bin/env python3
"""
SQLCC 项目文档-代码一致性保守修复脚本

此脚本采用保守方式修复文档与代码的一致性问题，
重点解决核心问题而不影响其他内容
"""

import os
import re
import sys
from pathlib import Path
from typing import List, Dict, Tuple, Set


def find_potential_file_references():
    """查找文档中可能的文件引用"""
    potential_refs = []
    
    for doc_file in Path('docs').rglob("*.md"):
        with open(doc_file, 'r', encoding='utf-8') as f:
            content = f.read()
        
        # 查找形如 [text](path/to/file.ext) 或 `path/to/file.ext` 的引用
        # 但排除明显不是文件路径的内容
        bracket_refs = re.findall(r'\[([^\]]+)\]\(([^)]+\.(?:h|hpp|cpp|cc|cxx))\)', content)
        for text, path in bracket_refs:
            potential_refs.append((str(doc_file), path, f"[{text}]({path})"))
        
        # 只处理看起来像真实路径的反引号内容（包含斜杠）
        backtick_refs = re.findall(r'`([a-zA-Z0-9_/-]+/\w+\.(?:h|hpp|cpp|cc|cxx))`', content)
        for path in backtick_refs:
            potential_refs.append((str(doc_file), path, f"`{path}`"))
    
    return potential_refs


def validate_file_exists(path):
    """检查文件是否真实存在"""
    # 尝试各种可能的前缀
    prefixes = ['src/', 'include/', 'examples/', 'tests/', 'scripts/']
    
    for prefix in prefixes:
        full_path = os.path.join(prefix, path)
        if os.path.exists(full_path):
            return full_path
    
    # 如果路径本身就是绝对路径
    if os.path.exists(path):
        return path
    
    # 如果路径以 /home 开头，提取后面的路径部分
    if path.startswith('/home/'):
        rel_path = '/'.join(path.split('/')[3:])  # 跳过 /home/user/ 部分
        for prefix in prefixes:
            full_path = os.path.join(prefix, rel_path)
            if os.path.exists(full_path):
                return full_path
    
    return None


def main():
    """主函数，执行保守修复"""
    print("开始执行SQLCC文档-代码一致性保守修复...")
    
    print("\n1. 扫描文档中的文件引用...")
    all_refs = find_potential_file_references()
    print(f"   找到 {len(all_refs)} 个潜在文件引用")
    
    print("\n2. 验证文件引用的有效性...")
    invalid_refs = []
    valid_refs = []
    
    for doc_file, path, full_ref in all_refs:
        actual_path = validate_file_exists(path)
        if actual_path:
            valid_refs.append((doc_file, path, actual_path))
        else:
            # 检查是否是明显无效的路径（如包含特殊字符或命令行片段）
            if not any(x in path for x in ['/...', '/*', '>&1', '2>&1', 'bazel']):
                invalid_refs.append((doc_file, path, full_ref))
    
    print(f"   有效引用: {len(valid_refs)}")
    print(f"   无效引用: {len(invalid_refs)}")
    
    print("\n3. 生成修复建议...")
    
    # 创建修复报告
    report_dir = Path('docs/consistency_reports')
    report_dir.mkdir(exist_ok=True)
    
    with open(report_dir / 'conservative_fix_report.md', 'w', encoding='utf-8') as f:
        f.write("# 文档-代码一致性保守修复报告\n\n")
        f.write("## 无效的文件引用清单\n\n")
        f.write("以下是在文档中引用但不存在的代码文件:\n\n")
        
        # 按文档分组
        refs_by_doc = {}
        for doc, path, full_ref in invalid_refs:
            if doc not in refs_by_doc:
                refs_by_doc[doc] = []
            refs_by_doc[doc].append((path, full_ref))
        
        for doc, refs in refs_by_doc.items():
            f.write(f"### {doc}\n\n")
            for path, full_ref in refs:
                f.write(f"- `{path}` - 引用形式: `{full_ref}`\n")
            f.write("\n")
        
        f.write("\n## 修复建议\n\n")
        f.write("1. 删除或更正这些无效的文件引用\n")
        f.write("2. 使用正确的路径替换这些引用\n")
        f.write("3. 如果引用是示例或占位符，考虑使用删除线标记\n\n")
        
        f.write("## 有效引用统计\n\n")
        f.write(f"发现 {len(valid_refs)} 个有效引用，这些不需要修复。\n")
    
    print(f"   修复建议已保存到: docs/consistency_reports/conservative_fix_report.md")
    
    print("\n4. 总结")
    print(f"   - 发现 {len(invalid_refs)} 个无效引用")
    print(f"   - 发现 {len(valid_refs)} 个有效引用")
    print("   - 请根据报告手动修复无效引用")
    
    return 0


if __name__ == "__main__":
    sys.exit(main())