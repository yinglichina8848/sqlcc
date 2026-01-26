#!/usr/bin/env python3
"""
SQLCC 项目文档-代码一致性精准修复脚本

此脚本专门针对文档中引用的代码文件进行精确修复，
避免误伤正常的文档内容。
"""

import os
import re
import sys
from pathlib import Path
from typing import List, Dict, Tuple, Set


def check_referenced_code_exists():
    """检查文档中引用的代码文件是否存在"""
    missing_code = []
    
    for doc_file in Path('docs').rglob("*.md"):
        with open(doc_file, 'r', encoding='utf-8') as f:
            content = f.read()
            
        # 更精确的正则表达式，只匹配特定格式的代码文件引用
        # 匹配格式如: `src/some/file.cpp`, `[some_file.cpp](src/some/file.cpp)` 等
        potential_refs = []
        
        # 匹配 [text](path/to/file.ext) 格式
        bracket_refs = re.findall(r'\[([^\]]+)\]\(([^)]+\.(?:h|hpp|cpp|cc|cxx))\)', content)
        for text, path in bracket_refs:
            potential_refs.append((path, str(doc_file)))
        
        # 匹配 `path/to/file.ext` 格式（仅当路径包含目录部分时）
        backtick_refs = re.findall(r'`([a-zA-Z0-9_/-]+/\w+\.(?:h|hpp|cpp|cc|cxx))`', content)
        for path in backtick_refs:
            potential_refs.append((path, str(doc_file)))
        
        # 检查每个潜在引用
        for path, doc_path in potential_refs:
            full_path = None
            
            # 尝试常见路径前缀
            prefixes = ['', 'src/', 'include/', 'examples/', 'tests/', 'scripts/']
            for prefix in prefixes:
                candidate = os.path.join(prefix, path).replace('\\', '/')
                if os.path.exists(candidate):
                    full_path = candidate
                    break
            
            # 如果仍然没有找到，可能是绝对路径形式
            if not full_path and path.startswith('/') and os.path.exists(path[1:]):  # Remove leading slash
                full_path = path[1:]
            
            if not full_path:
                # 特殊处理某些可能的误报
                if path.startswith('home/') or path.startswith('liying@') or path.startswith('/...'):
                    # 这些不是真实的文件路径，跳过
                    continue
                missing_code.append((str(doc_path), path))
    
    return missing_code


def analyze_problematic_refs():
    """分析问题引用的模式"""
    print("分析问题引用模式...")
    
    missing_code = check_referenced_code_exists()
    
    # 按文档分组
    refs_by_doc = {}
    for doc, path in missing_code:
        if doc not in refs_by_doc:
            refs_by_doc[doc] = []
        refs_by_doc[doc].append(path)
    
    print(f"发现 {len(missing_code)} 个疑似问题引用，分布在 {len(refs_by_doc)} 个文档中")
    
    for doc, paths in refs_by_doc.items():
        print(f"\n文档: {doc}")
        for path in paths[:5]:  # 只显示前5个
            print(f"  - {path}")
        if len(paths) > 5:
            print(f"  ... 还有 {len(paths)-5} 个")
    
    return missing_code


def create_summary_report(missing_refs):
    """创建问题摘要报告"""
    report_dir = Path('docs/consistency_reports')
    report_dir.mkdir(exist_ok=True)
    
    with open(report_dir / 'problematic_refs_summary.md', 'w', encoding='utf-8') as f:
        f.write("# 文档-代码一致性问题摘要\n\n")
        f.write("以下是在文档中引用但不存在的代码文件:\n\n")
        
        # 按文档分组
        refs_by_doc = {}
        for doc, path in missing_refs:
            if doc not in refs_by_doc:
                refs_by_doc[doc] = []
            refs_by_doc[doc].append(path)
        
        for doc, paths in refs_by_doc.items():
            f.write(f"## {doc}\n")
            for path in paths:
                f.write(f"- `{path}`\n")
            f.write("\n")
    
    print(f"问题摘要已保存到 {report_dir}/problematic_refs_summary.md")


def main():
    """主函数，执行精准修复分析"""
    print("开始执行SQLCC文档-代码一致性精准分析...")
    
    # 分析问题引用
    missing_refs = analyze_problematic_refs()
    
    if missing_refs:
        # 创建摘要报告
        create_summary_report(missing_refs)
        
        print(f"\n发现了 {len(missing_refs)} 个疑似问题引用")
        print("注意：这些引用不一定都是真正的错误，有些可能是示例或占位符")
        print("请人工检查 docs/consistency_reports/problematic_refs_summary.md 文件中的问题")
    else:
        print("未发现明显的文档-代码一致性问题")
    
    return 0


if __name__ == "__main__":
    sys.exit(main())