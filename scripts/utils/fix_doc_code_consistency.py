#!/usr/bin/env python3
"""
SQLCC 项目文档-代码一致性修复脚本

此脚本用于检测并修复文档与代码实现之间的一致性问题，
确保文档内容与实际代码实现相符。
"""

import os
import re
import sys
import shutil
from pathlib import Path
from typing import List, Dict, Tuple, Set


def find_documented_classes(docs_dir: str) -> Dict[str, str]:
    """查找文档中描述的类"""
    documented_classes = {}
    
    for doc_file in Path(docs_dir).rglob("*.md"):
        with open(doc_file, 'r', encoding='utf-8') as f:
            content = f.read()
            
            # 查找文档中定义的类
            class_matches = re.findall(r'class\s+([A-Za-z_][A-Za-z0-9_]*)', content)
            for cls in class_matches:
                if cls not in ['if', 'for', 'while', 'return', 'int', 'bool', 'void', 'char', 'double', 'float', 'long', 'short', 'unsigned', 'signed', 'struct']:
                    documented_classes[cls] = str(doc_file)
                
    return documented_classes


def find_implemented_classes(src_dir: str) -> Dict[str, str]:
    """查找代码中实现的类"""
    implemented_classes = {}
    
    for src_file in Path(src_dir).rglob("*.[ch]pp"):
        with open(src_file, 'r', encoding='utf-8') as f:
            content = f.read()
            
            # 查找代码中定义的类
            class_matches = re.findall(r'\b(?:class|struct)\s+([A-Za-z_][A-Za-z0-9_]*)', content)
            for cls in class_matches:
                implemented_classes[cls] = str(src_file)
                
    return implemented_classes


def find_documented_functions(docs_dir: str) -> Dict[str, str]:
    """查找文档中描述的函数"""
    documented_functions = {}
    
    for doc_file in Path(docs_dir).rglob("*.md"):
        with open(doc_file, 'r', encoding='utf-8') as f:
            content = f.read()
            
            # 查找文档中的函数定义（更精确的正则表达式）
            # 匹配 function_name(...) 或 function_name (...) 格式
            func_matches = re.findall(r'\b([A-Za-z_][A-Za-z0-9_]*)\s*\([^)]*\)', content)
            for func in func_matches:
                if func not in ['if', 'for', 'while', 'return', 'int', 'bool', 'void', 'char', 'double', 'float', 'long', 'short', 'unsigned', 'signed', 'auto', 'case', 'switch', 'break', 'continue', 'default', 'do', 'else', 'goto', 'sizeof', 'static_cast', 'dynamic_cast', 'const_cast', 'reinterpret_cast', 'new', 'delete', 'public', 'protected', 'private', 'virtual', 'explicit', 'friend', 'operator', 'template', 'typename', 'namespace', 'using', 'try', 'catch', 'throw', 'true', 'false', 'nullptr', 'this']:
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
                if func not in ['if', 'for', 'while', 'return', 'int', 'bool', 'void', 'char', 'double', 'float', 'long', 'short', 'unsigned', 'signed', 'auto', 'case', 'switch', 'break', 'continue', 'default', 'do', 'else', 'goto', 'sizeof', 'static_cast', 'dynamic_cast', 'const_cast', 'reinterpret_cast', 'new', 'delete', 'public', 'protected', 'private', 'virtual', 'explicit', 'friend', 'operator', 'template', 'typename', 'namespace', 'using', 'try', 'catch', 'throw', 'true', 'false', 'nullptr', 'this']:
                    implemented_functions[func] = str(src_file)
                    
            # 查找独立函数定义
            standalone_func_matches = re.findall(r'\b([A-Za-z_][A-Za-z0-9_]*)\s*\([^)]*\)\s*(?:const)?\s*(?:= 0|= delete)?\s*[{]', content)
            for func in standalone_func_matches:
                if func not in ['if', 'for', 'while', 'return', 'int', 'bool', 'void', 'char', 'double', 'float', 'long', 'short', 'unsigned', 'signed', 'auto', 'case', 'switch', 'break', 'continue', 'default', 'do', 'else', 'goto', 'sizeof', 'static_cast', 'dynamic_cast', 'const_cast', 'reinterpret_cast', 'new', 'delete', 'public', 'protected', 'private', 'virtual', 'explicit', 'friend', 'operator', 'template', 'typename', 'namespace', 'using', 'try', 'catch', 'throw', 'true', 'false', 'nullptr', 'this']:
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
            
            # 更精确地查找代码文件引用，避免误判
            # 仅查找以特定路径开头的文件引用
            code_refs = re.findall(r'([a-zA-Z0-9_/-]+/\w+\.(?:h|hpp|cpp|cc|cxx))', content)
            for code_ref in code_refs:
                # 尝试在不同目录中查找文件
                full_path = None
                for base in ['src/', 'include/', 'examples/', 'tests/', 'scripts/']:
                    candidate = os.path.join(base, code_ref)
                    if os.path.exists(candidate):
                        full_path = candidate
                        break
                        
                if not full_path and os.path.exists(code_ref):
                    full_path = code_ref
                        
                if not full_path:
                    missing_code.append((str(doc_file), code_ref))
                    
    return missing_code


def fix_documented_code_references():
    """修复文档中引用的代码文件路径"""
    fixed_count = 0
    
    # 获取所有有效的源代码文件列表
    valid_src_files = set()
    for ext in ["*.cpp", "*.h", "*.hpp", "*.cc", "*.cxx"]:
        for file_path in Path('src').rglob(ext):
            valid_src_files.add(str(file_path.relative_to('src')))
    for ext in ["*.cpp", "*.h", "*.hpp", "*.cc", "*.cxx"]:
        for file_path in Path('include').rglob(ext):
            valid_src_files.add(str(file_path.relative_to('include')))
    
    # 遍历文档文件，尝试修复错误的路径
    for doc_file in Path('docs').rglob("*.md"):
        with open(doc_file, 'r', encoding='utf-8') as f:
            original_content = f.read()
        
        # 查找可能的错误路径并尝试修复
        # 这里我们只修复一些常见的错误模式
        fixed_content = original_content
        
        # 搜索文档中可能的错误路径
        code_refs = re.findall(r'`([a-zA-Z0-9_/]+\.(?:h|hpp|cpp|cc|cxx))`', original_content)
        for code_ref in code_refs:
            # 如果找到的路径在有效文件中找不到，则尝试修复
            if code_ref not in valid_src_files:
                # 尝试添加 src/ 或 include/ 前缀
                for prefix in ['src/', 'include/']:
                    corrected_path = f"{prefix}{code_ref}"
                    if os.path.exists(corrected_path):
                        fixed_content = fixed_content.replace(f"`{code_ref}`", f"`{corrected_path}`")
                        print(f"Fixed reference in {doc_file}: {code_ref} -> {corrected_path}")
                        fixed_count += 1
                        break
    
        # 如果内容被修改，写回文件
        if fixed_content != original_content:
            with open(doc_file, 'w', encoding='utf-8') as f:
                f.write(fixed_content)
    
    return fixed_count


def create_class_documentation():
    """为未文档化的类生成基本文档"""
    documented_classes = find_documented_classes('docs')
    implemented_classes = find_implemented_classes('src')
    
    undocumented_classes = set(implemented_classes.keys()) - set(documented_classes.keys())
    
    # 创建一个汇总文档，列出未文档化的类
    doc_dir = Path('docs/generated')
    doc_dir.mkdir(exist_ok=True)
    
    with open(doc_dir / 'undocumented_classes.md', 'w', encoding='utf-8') as f:
        f.write("# 未文档化的类\n\n")
        f.write("以下是在代码中实现但尚未文档化的类:\n\n")
        
        for cls in sorted(undocumented_classes):
            f.write(f"- `{cls}` - 在 [{implemented_classes[cls]}]({implemented_classes[cls]}) 中实现\n")
    
    return len(undocumented_classes)


def create_function_documentation():
    """为未文档化的函数生成基本文档"""
    documented_funcs = find_documented_functions('docs')
    implemented_funcs = find_implemented_functions('src')
    
    undocumented_funcs = set(implemented_funcs.keys()) - set(documented_funcs.keys())
    
    # 创建一个汇总文档，列出未文档化的函数
    doc_dir = Path('docs/generated')
    doc_dir.mkdir(exist_ok=True)
    
    with open(doc_dir / 'undocumented_functions.md', 'w', encoding='utf-8') as f:
        f.write("# 未文档化的函数\n\n")
        f.write("以下是在代码中实现但尚未文档化的函数:\n\n")
        
        for func in sorted(list(undocumented_funcs)[:100]):  # 只列出前100个
            f.write(f"- `{func}` - 在 [{implemented_funcs[func]}]({implemented_funcs[func]}) 中实现\n")
    
    return min(len(undocumented_funcs), 100)


def remove_invalid_code_references():
    """移除文档中无效的代码引用"""
    removed_count = 0
    
    for doc_file in Path('docs').rglob("*.md"):
        with open(doc_file, 'r', encoding='utf-8') as f:
            original_content = f.read()
        
        # 查找无效的代码引用并移除它们
        fixed_content = original_content
        invalid_refs = []
        
        # 搜索文档中的代码引用 - 更精确的正则表达式
        # 只匹配像 [filename.ext](path/to/filename.ext) 或 `path/to/filename.ext` 这样的引用
        code_refs = re.findall(r'\[([a-zA-Z0-9_/]+\.(?:h|hpp|cpp|cc|cxx))\]\(([^)]+)\)|`([a-zA-Z0-9_/]+\.(?:h|hpp|cpp|cc|cxx))`', original_content)
        
        for groups in code_refs:
            # groups 是一个三元组，其中匹配的部分会是具体值，未匹配的是空字符串
            if groups[0]:  # 匹配的是 [filename.ext](path/to/filename.ext) 格式
                filepath = groups[1]  # 完整路径
                filename = groups[0]
            elif groups[2]:  # 匹配的是 `filename.ext` 格式
                filepath = groups[2]
                filename = groups[2]
            else:
                continue
                
            # 检查文件是否存在
            full_path = None
            for base in ['src/', 'include/', 'examples/', 'tests/', 'scripts/']:
                candidate = os.path.join(base, filepath)
                if os.path.exists(candidate):
                    full_path = candidate
                    break
            
            # 检查完整路径
            if not full_path and os.path.exists(filepath):
                full_path = filepath
            
            if not full_path:
                # 如果文件不存在，标记为需要移除或清理
                # 但我们只对那些看起来像是真实文件路径的进行处理
                if '/' in filepath or os.path.sep in filepath or any(part in filepath for part in ['src', 'include', 'test', 'example']):
                    # 这些看起来像是真正的文件路径，标记为无效
                    invalid_refs.append((filepath, f"[{filename}]({filepath})" if groups[0] else f"`{filename}`"))
        
        # 从内容中移除无效引用
        for filepath, ref_text in invalid_refs:
            # 替换为删除线格式
            replacement = f"~~{ref_text}~~"
            fixed_content = fixed_content.replace(ref_text, replacement)
            removed_count += 1
    
        # 如果内容被修改，写回文件
        if fixed_content != original_content:
            with open(doc_file, 'w', encoding='utf-8') as f:
                f.write(fixed_content)
    
    return removed_count


def main():
    """主函数，执行一致性修复"""
    print("开始执行SQLCC文档-代码一致性修复...")
    
    # 修复文档中引用的代码文件路径
    print("\n1. 修复文档中引用的代码文件路径...")
    fixed_refs = fix_documented_code_references()
    print(f"  ✅ 修复了 {fixed_refs} 个错误的代码引用路径")
    
    # 移除无效的代码引用
    print("\n2. 移除文档中无效的代码引用...")
    removed_refs = remove_invalid_code_references()
    print(f"  ✅ 移除了 {removed_refs} 个无效的代码引用")
    
    # 生成未文档化类的文档
    print("\n3. 生成未文档化类的基本文档...")
    undocumented_classes_count = create_class_documentation()
    print(f"  ✅ 记录了 {undocumented_classes_count} 个未文档化的类")
    
    # 生成未文档化函数的文档
    print("\n4. 生成未文档化函数的基本文档...")
    undocumented_funcs_count = create_function_documentation()
    print(f"  ✅ 记录了 {undocumented_funcs_count} 个未文档化的函数")
    
    print(f"\n5. 修复总结:")
    total_fixed = fixed_refs + removed_refs
    total_recorded = undocumented_classes_count + undocumented_funcs_count
    print(f"  🎉 成功修复了 {fixed_refs} 个引用问题，标记了 {removed_refs} 个无效引用")
    print(f"  📁 记录了 {total_recorded} 个未文档化元素")
    print(f"  📁 未文档化元素已保存到 docs/generated/ 目录中")
    
    return 0


if __name__ == "__main__":
    sys.exit(main())