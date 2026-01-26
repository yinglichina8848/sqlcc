#!/usr/bin/env python3
"""
SQLCC 项目全面部件分析脚本

此脚本分析SQLCC项目的所有主要部件，包括存储引擎、事务管理器、
索引系统、网络模块、SQL解析器、执行器等
"""

import os
import re
import sys
from pathlib import Path
from typing import List, Dict, Tuple, Set


def analyze_storage_engine():
    """分析存储引擎组件"""
    print("分析存储引擎组件...")
    
    storage_files = list(Path('src/storage_engine').rglob('*.cpp')) + list(Path('src/storage_engine').rglob('*.h'))
    storage_files += list(Path('include/storage_engine').rglob('*.h'))
    
    classes = set()
    functions = set()
    
    for file_path in storage_files:
        with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()
            
        # 查找类定义
        class_matches = re.findall(r'class\s+(\w+)|struct\s+(\w+)', content)
        for match in class_matches:
            cls_name = next(name for name in match if name)
            if cls_name and len(cls_name) > 1:
                classes.add(cls_name)
                
        # 查找函数定义
        func_matches = re.findall(r'(\w+)\s+[\w~]+\s*\([^;]*\)\s*(const)?\s*[;{]', content)
        for match in func_matches:
            func_name = match[0]
            if func_name and len(func_name) > 1 and func_name not in ['return', 'const', 'virtual', 'static', 'void', 'int', 'bool', 'double', 'char', 'if', 'else', 'while', 'for']:
                functions.add(func_name)
    
    print(f"存储引擎组件: {len(classes)} 个类, {len(functions)} 个函数")
    return {
        'files': storage_files,
        'classes': classes,
        'functions': functions
    }


def analyze_transaction_manager():
    """分析事务管理器组件"""
    print("分析事务管理器组件...")
    
    transaction_files = list(Path('src/transaction').rglob('*.cpp')) + list(Path('src/transaction').rglob('*.h'))
    transaction_files += list(Path('include/transaction').rglob('*.h'))
    
    classes = set()
    functions = set()
    
    for file_path in transaction_files:
        with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()
            
        # 查找类定义
        class_matches = re.findall(r'class\s+(\w+)|struct\s+(\w+)', content)
        for match in class_matches:
            cls_name = next(name for name in match if name)
            if cls_name and len(cls_name) > 1:
                classes.add(cls_name)
                
        # 查找函数定义
        func_matches = re.findall(r'(\w+)\s+[\w~]+\s*\([^;]*\)\s*(const)?\s*[;{]', content)
        for match in func_matches:
            func_name = match[0]
            if func_name and len(func_name) > 1 and func_name not in ['return', 'const', 'virtual', 'static', 'void', 'int', 'bool', 'double', 'char', 'if', 'else', 'while', 'for']:
                functions.add(func_name)
    
    print(f"事务管理器组件: {len(classes)} 个类, {len(functions)} 个函数")
    return {
        'files': transaction_files,
        'classes': classes,
        'functions': functions
    }


def analyze_index_system():
    """分析索引系统组件"""
    print("分析索引系统组件...")
    
    index_files = []
    for ext in ["*.h", "*.hpp", "*.cpp", "*.cc", "*.cxx"]:
        for file_path in Path('.').rglob(ext):
            if any(keyword in str(file_path).lower() for keyword in 
                   ['index', 'btree', 'bplustree', 'b_plus', 'b-plus', 'tree']):
                full_path = str(file_path).replace('./', '')
                if full_path.startswith(('src/', 'include/')):
                    index_files.append(file_path)
    
    classes = set()
    functions = set()
    
    for file_path in index_files:
        with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()
            
        # 查找类定义
        class_matches = re.findall(r'class\s+(\w+)|struct\s+(\w+)', content)
        for match in class_matches:
            cls_name = next(name for name in match if name)
            if cls_name and len(cls_name) > 1:
                classes.add(cls_name)
                
        # 查找函数定义
        func_matches = re.findall(r'(\w+)\s+[\w~]+\s*\([^;]*\)\s*(const)?\s*[;{]', content)
        for match in func_matches:
            func_name = match[0]
            if func_name and len(func_name) > 1 and func_name not in ['return', 'const', 'virtual', 'static', 'void', 'int', 'bool', 'double', 'char', 'if', 'else', 'while', 'for']:
                functions.add(func_name)
    
    print(f"索引系统组件: {len(classes)} 个类, {len(functions)} 个函数")
    return {
        'files': index_files,
        'classes': classes,
        'functions': functions
    }


def analyze_network_module():
    """分析网络模块组件"""
    print("分析网络模块组件...")
    
    network_files = list(Path('src/network').rglob('*.cpp')) + list(Path('src/network').rglob('*.h'))
    network_files += list(Path('include/network').rglob('*.h'))
    
    classes = set()
    functions = set()
    
    for file_path in network_files:
        with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()
            
        # 查找类定义
        class_matches = re.findall(r'class\s+(\w+)|struct\s+(\w+)', content)
        for match in class_matches:
            cls_name = next(name for name in match if name)
            if cls_name and len(cls_name) > 1:
                classes.add(cls_name)
                
        # 查找函数定义
        func_matches = re.findall(r'(\w+)\s+[\w~]+\s*\([^;]*\)\s*(const)?\s*[;{]', content)
        for match in func_matches:
            func_name = match[0]
            if func_name and len(func_name) > 1 and func_name not in ['return', 'const', 'virtual', 'static', 'void', 'int', 'bool', 'double', 'char', 'if', 'else', 'while', 'for']:
                functions.add(func_name)
    
    print(f"网络模块组件: {len(classes)} 个类, {len(functions)} 个函数")
    return {
        'files': network_files,
        'classes': classes,
        'functions': functions
    }


def analyze_sql_parser():
    """分析SQL解析器组件"""
    print("分析SQL解析器组件...")
    
    parser_files = list(Path('src/sql_parser').rglob('*.cpp')) + list(Path('src/sql_parser').rglob('*.h'))
    parser_files += list(Path('include/sql_parser').rglob('*.h'))
    
    classes = set()
    functions = set()
    
    for file_path in parser_files:
        with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()
            
        # 查找类定义
        class_matches = re.findall(r'class\s+(\w+)|struct\s+(\w+)', content)
        for match in class_matches:
            cls_name = next(name for name in match if name)
            if cls_name and len(cls_name) > 1:
                classes.add(cls_name)
                
        # 查找函数定义
        func_matches = re.findall(r'(\w+)\s+[\w~]+\s*\([^;]*\)\s*(const)?\s*[;{]', content)
        for match in func_matches:
            func_name = match[0]
            if func_name and len(func_name) > 1 and func_name not in ['return', 'const', 'virtual', 'static', 'void', 'int', 'bool', 'double', 'char', 'if', 'else', 'while', 'for']:
                functions.add(func_name)
    
    print(f"SQL解析器组件: {len(classes)} 个类, {len(functions)} 个函数")
    return {
        'files': parser_files,
        'classes': classes,
        'functions': functions
    }


def analyze_sql_executor():
    """分析SQL执行器组件"""
    print("分析SQL执行器组件...")
    
    executor_files = list(Path('src/sql_executor').rglob('*.cpp')) + list(Path('src/sql_executor').rglob('*.h'))
    executor_files += list(Path('include/sql_executor').rglob('*.h'))
    
    classes = set()
    functions = set()
    
    for file_path in executor_files:
        with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()
            
        # 查找类定义
        class_matches = re.findall(r'class\s+(\w+)|struct\s+(\w+)', content)
        for match in class_matches:
            cls_name = next(name for name in match if name)
            if cls_name and len(cls_name) > 1:
                classes.add(cls_name)
                
        # 查找函数定义
        func_matches = re.findall(r'(\w+)\s+[\w~]+\s*\([^;]*\)\s*(const)?\s*[;{]', content)
        for match in func_matches:
            func_name = match[0]
            if func_name and len(func_name) > 1 and func_name not in ['return', 'const', 'virtual', 'static', 'void', 'int', 'bool', 'double', 'char', 'if', 'else', 'while', 'for']:
                functions.add(func_name)
    
    print(f"SQL执行器组件: {len(classes)} 个类, {len(functions)} 个函数")
    return {
        'files': executor_files,
        'classes': classes,
        'functions': functions
    }


def analyze_config_manager():
    """分析配置管理器组件"""
    print("分析配置管理器组件...")
    
    config_files = list(Path('src/config_manager').rglob('*.cpp')) + list(Path('src/config_manager').rglob('*.h'))
    config_files += list(Path('include/config_manager').rglob('*.h'))
    
    classes = set()
    functions = set()
    
    for file_path in config_files:
        with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()
            
        # 查找类定义
        class_matches = re.findall(r'class\s+(\w+)|struct\s+(\w+)', content)
        for match in class_matches:
            cls_name = next(name for name in match if name)
            if cls_name and len(cls_name) > 1:
                classes.add(cls_name)
                
        # 查找函数定义
        func_matches = re.findall(r'(\w+)\s+[\w~]+\s*\([^;]*\)\s*(const)?\s*[;{]', content)
        for match in func_matches:
            func_name = match[0]
            if func_name and len(func_name) > 1 and func_name not in ['return', 'const', 'virtual', 'static', 'void', 'int', 'bool', 'double', 'char', 'if', 'else', 'while', 'for']:
                functions.add(func_name)
    
    print(f"配置管理器组件: {len(classes)} 个类, {len(functions)} 个函数")
    return {
        'files': config_files,
        'classes': classes,
        'functions': functions
    }


def find_documentation_for_components(components_data):
    """查找各组件的文档"""
    print("\n查找各组件的文档...")
    
    docs_dir = Path('docs')
    component_docs = {}
    
    # 检查每个组件是否有对应的文档
    for comp_name, comp_data in components_data.items():
        doc_files = []
        comp_lower = comp_name.lower().replace(' ', '_')
        
        for doc_file in docs_dir.rglob('*.md'):
            with open(doc_file, 'r', encoding='utf-8') as f:
                content = f.read()
                
            # 检查文档是否提及该组件的类或函数
            mentions = 0
            for cls in comp_data['classes']:
                if cls.lower() in content.lower():
                    mentions += 1
            for func in comp_data['functions']:
                if func.lower() in content.lower():
                    mentions += 1
                    
            if mentions > 0:
                doc_files.append((str(doc_file), mentions))
        
        component_docs[comp_name] = doc_files
        print(f"{comp_name}: {len(doc_files)} 个相关文档")
    
    return component_docs


def main():
    """主函数"""
    print("开始执行SQLCC项目全面部件分析...")
    
    # 分析各个组件
    components = {
        'Storage Engine': analyze_storage_engine(),
        'Transaction Manager': analyze_transaction_manager(),
        'Index System': analyze_index_system(),
        'Network Module': analyze_network_module(),
        'SQL Parser': analyze_sql_parser(),
        'SQL Executor': analyze_sql_executor(),
        'Config Manager': analyze_config_manager()
    }
    
    # 查找各组件的文档
    component_docs = find_documentation_for_components(components)
    
    # 生成综合报告
    report_dir = Path('docs/comprehensive_analysis_reports')
    report_dir.mkdir(exist_ok=True)
    
    with open(report_dir / 'comprehensive_component_analysis_report.md', 'w', encoding='utf-8') as f:
        f.write("# SQLCC 项目全面部件分析报告\n\n")
        
        f.write("## 1. 分析摘要\n\n")
        f.write("| 组件 | 文件数 | 类数 | 函数数 |\n")
        f.write("|------|--------|------|--------|\n")
        
        total_files = 0
        total_classes = 0
        total_functions = 0
        
        for comp_name, comp_data in components.items():
            file_count = len(comp_data['files'])
            class_count = len(comp_data['classes'])
            func_count = len(comp_data['functions'])
            
            total_files += file_count
            total_classes += class_count
            total_functions += func_count
            
            f.write(f"| {comp_name} | {file_count} | {class_count} | {func_count} |\n")
        
        f.write(f"\n| 总计 | {total_files} | {total_classes} | {total_functions} |\n\n")
        
        f.write("## 2. 各组件详细信息\n\n")
        
        for comp_name, comp_data in components.items():
            f.write(f"### {comp_name}\n\n")
            f.write(f"**文件列表:**\n\n")
            for file_path in comp_data['files'][:10]:  # 只列出前10个
                f.write(f"- {file_path}\n")
            if len(comp_data['files']) > 10:
                f.write(f"... 还有 {len(comp_data['files']) - 10} 个文件\n")
            f.write("\n")
            
            f.write(f"**类列表:**\n\n")
            sorted_classes = sorted(list(comp_data['classes']))[:15]  # 只列出前15个
            for cls in sorted_classes:
                f.write(f"- `{cls}`\n")
            if len(comp_data['classes']) > 15:
                f.write(f"... 还有 {len(comp_data['classes']) - 15} 个类\n")
            f.write("\n")
            
            f.write(f"**函数列表:**\n\n")
            sorted_functions = sorted(list(comp_data['functions']))[:15]  # 只列出前15个
            for func in sorted_functions:
                f.write(f"- `{func}`\n")
            if len(comp_data['functions']) > 15:
                f.write(f"... 还有 {len(comp_data['functions']) - 15} 个函数\n")
            f.write("\n")
        
        f.write("## 3. 文档关联情况\n\n")
        for comp_name, docs in component_docs.items():
            f.write(f"### {comp_name}\n\n")
            if docs:
                for doc_path, mentions in docs:
                    f.write(f"- [{doc_path}] - 提及 {mentions} 次\n")
            else:
                f.write(f"未找到与 {comp_name} 相关的文档\n")
            f.write("\n")
        
        f.write("## 4. 分析结论与建议\n\n")
        f.write("### 4.1 文档覆盖情况\n\n")
        for comp_name, docs in component_docs.items():
            if not docs:
                f.write(f"- **{comp_name}**: 缺少专门的文档，建议创建设计文档\n")
        
        f.write("\n### 4.2 修复建议\n\n")
        f.write("1. 为缺少文档的组件创建专门的设计文档\n")
        f.write("2. 补充各组件的API文档\n")
        f.write("3. 创建组件间交互关系的文档\n")
        f.write("4. 整理各组件的关键类和函数说明\n\n")
        
        f.write("### 4.3 优先级排序\n\n")
        f.write("1. **高优先级**: 为存储引擎、事务管理器、索引系统创建详细设计文档\n")
        f.write("2. **中优先级**: 完善SQL解析器和执行器的API文档\n")
        f.write("3. **低优先级**: 补充网络模块和配置管理器的文档\n\n")
    
    print(f"\n综合分析报告已保存到: {report_dir}/comprehensive_component_analysis_report.md")
    print("分析完成！")
    
    return 0


if __name__ == "__main__":
    sys.exit(main())