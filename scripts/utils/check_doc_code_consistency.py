#!/usr/bin/env python3
"""
SQLCC 项目文档-代码一致性检查脚本

此脚本用于验证文档与代码实现之间的一致性，
确保文档内容与实际代码实现相符。
"""

import os
import re
import sys
from pathlib import Path
from typing import List, Dict, Tuple

def find_documented_classes(docs_dir: str) -> Dict[str, str]:
    """查找文档中描述的类"""
    documented_classes = {}
    
    for doc_file in Path(docs_dir).rglob("*.md"):
        with open(doc_file, 'r', encoding='utf-8') as f:
            content = f.read()
            
            # 查找文档中定义的类
            class_matches = re.findall(r'class\s+([A-Za-z_][A-Za-z0-9_]*)', content)
            for cls in class_matches:
                documented_classes[cls] = str(doc_file)
                
    return documented_classes

def find_implemented_classes(src_dir: str) -> Dict[str, str]:
    """查找代码中实现的类"""
    implemented_classes = {}
    
    for src_file in Path(src_dir).rglob("*.[ch]pp"):
        with open(src_file, 'r', encoding='utf-8') as f:
            content = f.read()
            
            # 查找代码中定义的类
            class_matches = re.findall(r'\bclass\s+([A-Za-z_][A-Za-z0-9_]*)', content)
            for cls in class_matches:
                implemented_classes[cls] = str(src_file)
                
    return implemented_classes

def find_documented_functions(docs_dir: str) -> Dict[str, str]:
    """查找文档中描述的函数"""
    documented_functions = {}
    
    for doc_file in Path(docs_dir).rglob("*.md"):
        with open(doc_file, 'r', encoding='utf-8') as f:
            content = f.read()
            
            # 查找文档中的函数定义（通常在代码块中）
            func_matches = re.findall(r'([A-Za-z_][A-Za-z0-9_]*)\s*\([^)]*\)\s*(?:const)?\s*(?:= 0|= delete)?', content)
            for func in func_matches:
                if func not in ['if', 'for', 'while', 'return', 'int', 'bool', 'void', 'char', 'double', 'float', 'long', 'short', 'unsigned', 'signed']:
                    documented_functions[func] = str(doc_file)
                
    return documented_functions

def find_implemented_functions(src_dir: str) -> Dict[str, str]:
    """查找代码中实现的函数"""
    implemented_functions = {}
    
    for src_file in Path(src_dir).rglob("*.[ch]pp"):
        with open(src_file, 'r', encoding='utf-8') as f:
            content = f.read()
            
            # 查找函数定义
            func_matches = re.findall(r'([A-Za-z_][A-Za-z0-9_]*)\s*::\s*([A-Za-z_][A-Za-z0-9_]*)\s*\([^)]*\)', content)
            for _, func in func_matches:
                if func not in ['if', 'for', 'while', 'return', 'int', 'bool', 'void', 'char', 'double', 'float', 'long', 'short', 'unsigned', 'signed']:
                    implemented_functions[func] = str(src_file)
                
    return implemented_functions

def check_referenced_docs_exist():
    """检查代码中引用的文档是否存在"""
    missing_docs = []
    
    # 在源代码中搜索文档引用
    for src_file in Path('src').rglob("*.cpp"):
        with open(src_file, 'r', encoding='utf-8') as f:
            content = f.read()
            
            # 查找文档引用，通常是类似 docs/design/...md 的路径
            doc_refs = re.findall(r'docs/[^\s")\']*\.md', content)
            for doc_ref in doc_refs:
                if not os.path.exists(doc_ref):
                    missing_docs.append((str(src_file), doc_ref))
                    
    return missing_docs

def check_referenced_code_exists():
    """检查文档中引用的代码文件是否存在"""
    missing_code = []
    
    for doc_file in Path('docs').rglob("*.md"):
        with open(doc_file, 'r', encoding='utf-8') as f:
            content = f.read()
            
            # 查找代码文件引用，通常是类似 src/... 或 include/... 的路径
            code_refs = re.findall(r'[^(]*\.(?:h|hpp|cpp|cc|cxx)[^)\s]*', content)
            for code_ref in code_refs:
                # 清理路径
                path = code_ref.strip().strip('`')
                if path.startswith('/'):
                    path = path[1:]
                else:
                    continue  # 跳过相对路径引用
                    
                # 尝试在不同目录中查找文件
                full_path = None
                for base in ['src/', 'include/', 'examples/', 'tests/']:
                    candidate = base + path
                    if os.path.exists(candidate):
                        full_path = candidate
                        break
                        
                if not full_path:
                    missing_code.append((str(doc_file), path))
                    
    return missing_code

def main():
    """主函数，执行一致性检查"""
    print("开始执行SQLCC文档-代码一致性检查...")
    
    # 检查代码中引用的文档是否存在
    print("\n1. 检查代码中引用的文档是否存在...")
    missing_docs = check_referenced_docs_exist()
    if missing_docs:
        print(f"  ❌ 发现 {len(missing_docs)} 个缺失的文档引用:")
        for src_file, doc_path in missing_docs:
            print(f"     - {src_file}: {doc_path}")
    else:
        print("  ✅ 所有代码中引用的文档都存在")
    
    # 检查文档中引用的代码文件是否存在
    print("\n2. 检查文档中引用的代码文件是否存在...")
    missing_code = check_referenced_code_exists()
    if missing_code:
        print(f"  ❌ 发现 {len(missing_code)} 个缺失的代码文件引用:")
        for doc_file, code_path in missing_code:
            print(f"     - {doc_file}: {code_path}")
    else:
        print("  ✅ 所有文档中引用的代码文件都存在")
    
    # 检查类定义的一致性
    print("\n3. 检查类定义的一致性...")
    documented_classes = find_documented_classes('docs')
    implemented_classes = find_implemented_classes('src')
    
    undocumented_classes = set(implemented_classes.keys()) - set(documented_classes.keys())
    unimplemented_classes = set(documented_classes.keys()) - set(implemented_classes.keys())
    
    if undocumented_classes:
        print(f"  ⚠️  发现 {len(undocumented_classes)} 个未文档化的类:")
        for cls in list(undocumented_classes)[:10]:  # 只显示前10个
            print(f"     - {cls} (在 {implemented_classes[cls]} 中实现)")
        if len(undocumented_classes) > 10:
            print(f"     ... 还有 {len(undocumented_classes) - 10} 个")
    
    if unimplemented_classes:
        print(f"  ⚠️  发现 {len(unimplemented_classes)} 个文档中描述但未实现的类:")
        for cls in list(unimplemented_classes)[:10]:  # 只显示前10个
            print(f"     - {cls} (在 {documented_classes[cls]} 中描述)")
        if len(unimplemented_classes) > 10:
            print(f"     ... 还有 {len(unimplemented_classes) - 10} 个")
    
    if not undocumented_classes and not unimplemented_classes:
        print("  ✅ 所有类定义都已正确文档化")
    
    # 检查函数定义的一致性
    print("\n4. 检查函数定义的一致性...")
    documented_funcs = find_documented_functions('docs')
    implemented_funcs = find_implemented_functions('src')
    
    undocumented_funcs = set(implemented_funcs.keys()) - set(documented_funcs.keys())
    unimplemented_funcs = set(documented_funcs.keys()) - set(implemented_funcs.keys())
    
    if undocumented_funcs:
        print(f"  ⚠️  发现 {len(undocumented_funcs)} 个未文档化的函数:")
        for func in list(undocumented_funcs)[:10]:  # 只显示前10个
            print(f"     - {func} (在 {implemented_funcs[func]} 中实现)")
        if len(undocumented_funcs) > 10:
            print(f"     ... 还有 {len(undocumented_funcs) - 10} 个")
    
    if unimplemented_funcs:
        print(f"  ⚠️  发现 {len(unimplemented_funcs)} 个文档中描述但未实现的函数:")
        for func in list(unimplemented_funcs)[:10]:  # 只显示前10个
            print(f"     - {func} (在 {documented_funcs[func]} 中描述)")
        if len(unimplemented_funcs) > 10:
            print(f"     ... 还有 {len(unimplemented_funcs) - 10} 个")
    
    print("\n5. 检查总结:")
    total_issues = len(missing_docs) + len(missing_code) + len(undocumented_classes) + len(unimplemented_classes) + len(undocumented_funcs) + len(unimplemented_funcs)
    
    if total_issues == 0:
        print("  🎉 恭喜! 文档与代码实现完全一致!")
        return 0
    else:
        print(f"  📋 检查完成，发现 {total_issues} 个需要注意的问题")
        return 1

if __name__ == "__main__":
    sys.exit(main())