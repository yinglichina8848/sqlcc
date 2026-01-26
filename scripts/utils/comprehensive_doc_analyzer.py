#!/usr/bin/env python3
"""
SQLCC 项目文档和索引文件全面分析修复脚本

此脚本分析并修复项目中缺失的文档和索引文件问题
"""

import os
import re
import sys
from pathlib import Path
from typing import List, Dict, Tuple, Set


def find_missing_documentation():
    """查找缺失的文档"""
    print("开始查找缺失的文档...")
    
    # 获取所有实现文件
    impl_files = list(Path('.').rglob('*.cpp')) + list(Path('.').rglob('*.cc')) + list(Path('.').rglob('*.cxx'))
    header_files = list(Path('.').rglob('*.h')) + list(Path('.').rglob('*.hpp'))
    
    # 提取类名和函数名
    classes_in_code = set()
    functions_in_code = set()
    
    for impl_file in impl_files + header_files:
        with open(impl_file, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()
            
        # 查找类定义
        class_matches = re.findall(r'class\s+(\w+)|struct\s+(\w+)|namespace\s+(\w+)', content)
        for match in class_matches:
            cls_name = next(name for name in match if name)  # 获取第一个非空的匹配
            if cls_name and len(cls_name) > 1:  # 忽略单字母命名
                classes_in_code.add(cls_name)
                
        # 查找函数定义
        func_matches = re.findall(r'(\w+)\s+[\w~]+\s*\([^;]*\)\s*(const)?\s*(= 0)?\s*[{;]', content)
        for match in func_matches:
            func_name = match[0]
            if func_name and len(func_name) > 1 and func_name not in ['return', 'const', 'virtual', 'static']:
                functions_in_code.add(func_name)
    
    print(f"在代码中找到 {len(classes_in_code)} 个类和 {len(functions_in_code)} 个函数")
    
    # 获取文档中描述的类和函数
    doc_files = list(Path('docs').rglob('*.md'))
    
    classes_in_docs = set()
    functions_in_docs = set()
    
    for doc_file in doc_files:
        with open(doc_file, 'r', encoding='utf-8') as f:
            content = f.read()
            
        # 查找文档中提到的类
        class_matches = re.findall(r'(\w+)Manager|(\w+)Handler|(\w+)Processor|(\w+)Service|(\w+)Provider', content)
        for match in class_matches:
            cls_name = next(name for name in match if name)
            if cls_name and len(cls_name) > 1:
                classes_in_docs.add(cls_name)
                
        # 更广泛的类查找
        general_class_matches = re.findall(r'(\w+)Class|(\w+)Interface|(\w+)Component|(\w+)Module|(\w+)System', content)
        for match in general_class_matches:
            cls_name = next(name for name in match if name)
            if cls_name and len(cls_name) > 1:
                classes_in_docs.add(cls_name)
    
    print(f"在文档中找到 {len(classes_in_docs)} 个类")
    
    # 检查缺失的文档
    missing_classes = classes_in_code - classes_in_docs
    missing_functions = functions_in_code - functions_in_docs
    
    print(f"缺少文档的类: {len(missing_classes)} 个")
    print(f"缺少文档的函数: {len(missing_functions)} 个")
    
    return {
        'classes_in_code': classes_in_code,
        'functions_in_code': functions_in_code,
        'classes_in_docs': classes_in_docs,
        'missing_classes': missing_classes,
        'missing_functions': missing_functions
    }


def find_index_files():
    """查找可能缺失的索引文件"""
    print("\n开始查找可能缺失的索引文件...")
    
    # 搜索可能的索引文件
    index_files = list(Path('.').rglob('*index*'))
    index_files.extend(list(Path('.').rglob('*btree*')))
    index_files.extend(list(Path('.').rglob('*BPlus*')))
    
    print(f"找到 {len(index_files)} 个与索引相关的文件:")
    for idx_file in index_files[:10]:  # 只显示前10个
        print(f"  - {idx_file}")
    
    return index_files


def analyze_header_file_consistency():
    """分析头文件一致性问题"""
    print("\n开始分析头文件一致性问题...")
    
    # 找出所有头文件
    header_files = list(Path('include').rglob('*.h')) + list(Path('include').rglob('*.hpp'))
    
    # 检查头文件保护
    headers_without_guard = []
    for h_file in header_files:
        with open(h_file, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()
            
        if not re.search(r'#ifndef\s+\w+\s*#define\s+\w+', content) and '#pragma once' not in content:
            headers_without_guard.append(str(h_file))
    
    print(f"发现 {len(headers_without_guard)} 个没有头文件保护的头文件")
    
    return headers_without_guard


def generate_documentation_suggestions(data):
    """生成文档建议"""
    print("\n生成文档建议...")
    
    suggestions_dir = Path('docs/documentation_suggestions')
    suggestions_dir.mkdir(exist_ok=True)
    
    # 生成缺失类的建议
    with open(suggestions_dir / 'missing_classes_suggestions.md', 'w', encoding='utf-8') as f:
        f.write("# 缺失的类文档建议\n\n")
        f.write("以下是在代码中实现但在文档中缺失的类，建议为它们创建文档：\n\n")
        
        for cls in sorted(list(data['missing_classes'])[:50]):  # 限制数量以避免文件过大
            f.write(f"- `{cls}`\n")
    
    # 生成缺失函数的建议
    with open(suggestions_dir / 'missing_functions_suggestions.md', 'w', encoding='utf-8') as f:
        f.write("# 缺失的函数文档建议\n\n")
        f.write("以下是在代码中实现但在文档中缺失的函数，建议为它们创建文档：\n\n")
        
        for func in sorted(list(data['missing_functions'])[:50]):  # 限制数量
            f.write(f"- `{func}`\n")
    
    print(f"已生成文档建议，保存在 {suggestions_dir} 目录中")


def main():
    """主函数"""
    print("开始执行SQLCC项目文档和索引文件全面分析...")
    
    # 分析缺失的文档
    data = find_missing_documentation()
    
    # 分析索引文件
    index_files = find_index_files()
    
    # 分析头文件一致性
    headers_without_guard = analyze_header_file_consistency()
    
    # 生成文档建议
    generate_documentation_suggestions(data)
    
    # 生成综合报告
    report_dir = Path('docs/comprehensive_analysis_reports')
    report_dir.mkdir(exist_ok=True)
    
    with open(report_dir / 'comprehensive_analysis_report.md', 'w', encoding='utf-8') as f:
        f.write("# SQLCC 项目文档和索引文件全面分析报告\n\n")
        
        f.write("## 1. 分析摘要\n\n")
        f.write(f"- 代码中实现的类数量: {len(data['classes_in_code'])}\n")
        f.write(f"- 文档中描述的类数量: {len(data['classes_in_docs'])}\n")
        f.write(f"- 缺失的类文档数量: {len(data['missing_classes'])}\n")
        f.write(f"- 代码中实现的函数数量: {len(data['functions_in_code'])}\n")
        f.write(f"- 缺失的函数文档数量: {len(data['missing_functions'])}\n")
        f.write(f"- 与索引相关的文件数量: {len(index_files)}\n")
        f.write(f"- 缺少头文件保护的头文件数量: {len(headers_without_guard)}\n\n")
        
        f.write("## 2. 修复建议\n\n")
        f.write("### 2.1 类文档修复\n\n")
        f.write("为缺失的类创建详细的文档，包括类的功能、接口、使用示例等。\n\n")
        
        f.write("### 2.2 函数文档修复\n\n")
        f.write("为缺失的函数创建文档，特别是公共接口函数。\n\n")
        
        f.write("### 2.3 索引系统文档\n\n")
        f.write("整理索引系统的相关文档，包括B+树实现、索引管理器等。\n\n")
        
        f.write("### 2.4 头文件修复\n\n")
        f.write("为缺少头文件保护的头文件添加适当的头文件保护或#pragma once指令。\n\n")
        
        f.write("## 3. 后续步骤\n\n")
        f.write("1. 审查生成的建议文档\n")
        f.write("2. 按优先级为缺失的类和函数添加文档\n")
        f.write("3. 更新索引系统相关文档\n")
        f.write("4. 添加必要的头文件保护\n")
        f.write("5. 重新运行一致性检查验证修复效果\n")
    
    print(f"\n综合分析报告已保存到: {report_dir}/comprehensive_analysis_report.md")
    print("分析完成！")
    
    return 0


if __name__ == "__main__":
    sys.exit(main())