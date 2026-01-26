#!/usr/bin/env python3
"""
SQLCC 项目文档补全工具

此脚本分析并补全项目中缺少的文档，特别是针对之前分析中发现的文档不足的组件
"""

import os
import re
import sys
from pathlib import Path
from typing import List, Dict, Tuple, Set


def extract_detailed_info_from_source(component_name, source_files):
    """从源代码中提取详细信息"""
    print(f"从 {component_name} 的源代码中提取详细信息...")
    
    detailed_info = {
        'classes': {},
        'functions': {},
        'enums': {},
        'structs': {}
    }
    
    for file_path in source_files:
        with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()
        
        # 提取类的详细信息
        class_pattern = r'(?:^|\n)(?:\s*\/\/[^\n]*\n)*(?:\s*\/\*.*?\*\/\s*)?(\s*class\s+(\w+)[^{]*\{((?:[^{}]|{[^{}]*})*)\};?)'
        class_matches = re.finditer(class_pattern, content, re.DOTALL | re.MULTILINE)
        
        for match in class_matches:
            class_name = match.group(2)
            class_body = match.group(3)
            
            # 提取构造函数、析构函数、公有方法
            constructors = re.findall(r'(\w+)\s*\([^;]*\)\s*;', class_body)
            destructors = re.findall(r'~(\w+)\s*\([^;]*\)\s*;', class_body)
            public_methods = re.findall(r'(?:public:|private:|protected:)?\s*(\w+(?:\s+\w+)?\s+\w+)\s*\([^;]*\)\s*;', class_body)
            
            detailed_info['classes'][class_name] = {
                'file': str(file_path),
                'constructors': constructors,
                'destructors': destructors,
                'methods': public_methods,
                'definition': match.group(1)[:200] + "..." if len(match.group(1)) > 200 else match.group(1)
            }
        
        # 提取函数定义
        func_pattern = r'(?:^|\n)(?:\s*\/\/[^\n]*\n)*(?:\s*\/\*.*?\*\/\s*)?(\s*(?:\w+\s+)*\**\s*(\w+)\s*\([^;]*\)\s*\{)'
        func_matches = re.finditer(func_pattern, content, re.MULTILINE)
        
        for match in func_matches:
            func_name = match.group(2)
            if func_name not in ['if', 'while', 'for', 'switch', 'return']:
                detailed_info['functions'][func_name] = {
                    'file': str(file_path),
                    'signature': match.group(1)[:100] + "..." if len(match.group(1)) > 100 else match.group(1)
                }
    
    return detailed_info


def create_component_documentation(component_name, detailed_info):
    """为指定组件创建详细文档"""
    print(f"为 {component_name} 创建详细文档...")
    
    # 创建组件文档目录
    doc_dir = Path(f'docs/design/{component_name.lower().replace(" ", "_")}')
    doc_dir.mkdir(exist_ok=True)
    
    # 生成组件概述文档
    overview_file = doc_dir / f'{component_name.lower().replace(" ", "_")}_overview.md'
    with open(overview_file, 'w', encoding='utf-8') as f:
        f.write(f"# {component_name} 概述\n\n")
        f.write(f"{component_name} 是 SQLCC 数据库系统中的核心组件之一，负责...\n\n")
        f.write("## 主要职责\n\n")
        f.write("- 职责1\n")
        f.write("- 职责2\n")
        f.write("- 职责3\n\n")
        f.write("## 架构设计\n\n")
        f.write("组件采用分层架构设计，主要包括...\n\n")
        f.write("## 性能特点\n\n")
        f.write("- 特点1\n")
        f.write("- 特点2\n")
        f.write("- 特点3\n\n")
    
    # 生成类文档
    class_doc_file = doc_dir / f'{component_name.lower().replace(" ", "_")}_classes.md'
    with open(class_doc_file, 'w', encoding='utf-8') as f:
        f.write(f"# {component_name} 类设计文档\n\n")
        
        if detailed_info['classes']:
            f.write("## 类列表\n\n")
            for class_name, info in detailed_info['classes'].items():
                f.write(f"### {class_name}\n\n")
                f.write(f"**定义位置**: `{info['file']}`\n\n")
                f.write(f"**定义**:\n```cpp\n{info['definition']}\n```\n\n")
                
                if info['constructors']:
                    f.write("**构造函数**:\n")
                    for ctor in info['constructors']:
                        f.write(f"- `{ctor}`\n")
                    f.write("\n")
                
                if info['destructors']:
                    f.write("**析构函数**:\n")
                    for dtor in info['destructors']:
                        f.write(f"- `{dtor}`\n")
                    f.write("\n")
                
                if info['methods']:
                    f.write("**公有方法**:\n")
                    for method in info['methods']:
                        f.write(f"- `{method}`\n")
                    f.write("\n")
                
                f.write("---\n\n")
        else:
            f.write("该组件没有定义任何类。\n\n")
    
    # 生成函数文档
    func_doc_file = doc_dir / f'{component_name.lower().replace(" ", "_")}_functions.md'
    with open(func_doc_file, 'w', encoding='utf-8') as f:
        f.write(f"# {component_name} 函数设计文档\n\n")
        
        if detailed_info['functions']:
            f.write("## 函数列表\n\n")
            for func_name, info in detailed_info['functions'].items():
                f.write(f"### {func_name}\n\n")
                f.write(f"**定义位置**: `{info['file']}`\n\n")
                f.write(f"**签名**:\n```cpp\n{info['signature']}\n```\n\n")
                f.write("---\n\n")
        else:
            f.write("该组件没有定义任何函数。\n\n")
    
    print(f"为 {component_name} 创建了 {len(detailed_info['classes'])} 个类文档和 {len(detailed_info['functions'])} 个函数文档")
    
    return {
        'overview': str(overview_file),
        'classes': str(class_doc_file),
        'functions': str(func_doc_file)
    }


def load_component_data():
    """加载之前分析的组件数据"""
    # 这里模拟加载之前分析的数据
    # 在实际应用中，这里应该从文件或其他持久化存储中加载
    components = {
        'Storage Engine': list(Path('src/storage_engine').rglob('*.cpp')) + list(Path('src/storage_engine').rglob('*.h')) + list(Path('include/storage_engine').rglob('*.h')),
        'Transaction Manager': list(Path('src/transaction').rglob('*.cpp')) + list(Path('src/transaction').rglob('*.h')) + list(Path('include/transaction').rglob('*.h')),
        'Index System': list(Path('src').rglob('*index*.cpp')) + list(Path('src').rglob('*index*.h')) + list(Path('include').rglob('*index*.h')),
        'Network Module': list(Path('src/network').rglob('*.cpp')) + list(Path('src/network').rglob('*.h')) + list(Path('include/network').rglob('*.h')),
        'SQL Parser': list(Path('src/sql_parser').rglob('*.cpp')) + list(Path('src/sql_parser').rglob('*.h')) + list(Path('include/sql_parser').rglob('*.h')),
        'SQL Executor': list(Path('src/sql_executor').rglob('*.cpp')) + list(Path('src/sql_executor').rglob('*.h')) + list(Path('include/sql_executor').rglob('*.h')),
        'Config Manager': list(Path('src/config_manager').rglob('*.cpp')) + list(Path('src/config_manager').rglob('*.h')) + list(Path('include/config_manager').rglob('*.h'))
    }
    
    return components


def main():
    """主函数"""
    print("开始执行SQLCC项目文档补全工作...")
    
    # 加载组件数据
    components = load_component_data()
    
    # 为每个组件创建详细文档
    created_docs = {}
    
    for comp_name, source_files in components.items():
        print(f"\n处理 {comp_name} 组件...")
        
        # 从源代码提取详细信息
        detailed_info = extract_detailed_info_from_source(comp_name, source_files)
        
        # 创建文档
        doc_files = create_component_documentation(comp_name, detailed_info)
        created_docs[comp_name] = doc_files
    
    # 生成补全报告
    report_dir = Path('docs/document_completion_reports')
    report_dir.mkdir(exist_ok=True)
    
    with open(report_dir / 'document_completion_report.md', 'w', encoding='utf-8') as f:
        f.write("# SQLCC 项目文档补全报告\n\n")
        
        f.write("## 1. 补全摘要\n\n")
        f.write("| 组件 | 概述文档 | 类文档 | 函数文档 |\n")
        f.write("|------|----------|--------|---------|\n")
        
        for comp_name, doc_files in created_docs.items():
            f.write(f"| {comp_name} | [{Path(doc_files['overview']).name}]({doc_files['overview']}) | [{Path(doc_files['classes']).name}]({doc_files['classes']}) | [{Path(doc_files['functions']).name}]({doc_files['functions']}) |\n")
        
        f.write("\n## 2. 补全详情\n\n")
        for comp_name, doc_files in created_docs.items():
            f.write(f"### {comp_name}\n\n")
            f.write(f"为 {comp_name} 组件创建了以下文档：\n\n")
            f.write(f"- 概述文档: {doc_files['overview']}\n")
            f.write(f"- 类设计文档: {doc_files['classes']}\n")
            f.write(f"- 函数设计文档: {doc_files['functions']}\n\n")
    
    print(f"\n文档补全报告已保存到: {report_dir}/document_completion_report.md")
    print("文档补全工作完成！")
    
    return 0


if __name__ == "__main__":
    sys.exit(main())