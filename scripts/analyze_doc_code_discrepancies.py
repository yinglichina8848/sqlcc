#!/usr/bin/env python3
"""
深度分析文档-代码不一致性问题

分析哪些文档中的内容是过时的，哪些是确实需要实现的
"""

import os
import re
from pathlib import Path
from typing import List, Dict, Set

def analyze_unimplemented_classes():
    """分析文档中描述但未实现的类"""
    print("分析未实现类的具体情况...")
    
    # 获取所有源代码文件
    src_files = {}
    
    # 查找cpp文件
    for src_file in Path('src').rglob("*.cpp"):
        try:
            with open(src_file, 'r', encoding='utf-8', errors='ignore') as f:
                content = f.read()
                
                # 查找类定义
                class_patterns = [
                    r'class\s+([A-Za-z_][A-Za-z0-9_]*)\s*(?:[:{])',
                    r'struct\s+([A-Za-z_][A-Za-z0-9_]*)\s*(?:[:{])'
                ]
                
                for pattern in class_patterns:
                    matches = re.findall(pattern, content)
                    for cls_name in matches:
                        if cls_name not in src_files:
                            src_files[cls_name] = str(src_file)
        except:
            continue
    
    # 查找头文件
    for src_file in Path('src').rglob("*.h"):
        try:
            with open(src_file, 'r', encoding='utf-8', errors='ignore') as f:
                content = f.read()
                
                # 查找类定义
                class_patterns = [
                    r'class\s+([A-Za-z_][A-Za-z0-9_]*)\s*(?:[:{])',
                    r'struct\s+([A-Za-z_][A-Za-z0-9_]*)\s*(?:[:{])'
                ]
                
                for pattern in class_patterns:
                    matches = re.findall(pattern, content)
                    for cls_name in matches:
                        if cls_name not in src_files:
                            src_files[cls_name] = str(src_file)
        except:
            continue
    
    print(f"在源代码中找到了 {len(src_files)} 个类")
    
    # 分析文档中提到的类
    doc_classes = {}
    for doc_file in Path('docs').rglob("*.md"):
        try:
            with open(doc_file, 'r', encoding='utf-8', errors='ignore') as f:
                content = f.read()
                
                # 查找类名引用
                class_matches = re.findall(r'\b([A-Z][A-Za-z0-9_]*)\b', content)
                for cls in class_matches:
                    if cls not in doc_classes:
                        doc_classes[cls] = []
                    doc_classes[cls].append(str(doc_file))
        except:
            continue
    
    print(f"在文档中找到了 {len(doc_classes)} 个不同的类名")
    
    # 找出文档中提到的但源代码中不存在的类
    unimplemented_classes = {}
    for cls, files in doc_classes.items():
        if cls not in src_files:
            unimplemented_classes[cls] = files
    
    return unimplemented_classes, src_files

def categorize_classes(unimplemented_classes: Dict[str, List[str]]):
    """对未实现的类进行分类"""
    categories = {
        "obsolete_terms": set(),  # 明显过时的术语
        "planned_features": set(),  # 规划中的功能
        "required_implementations": set(),  # 需要实现的类
        "documentation_issues": set()  # 文档问题
    }
    
    obsolete_keywords = [
        'ConfigManager', 'Bazel', 'Analyzer', 'Learner', 'Experience', 'Privacy',
        'Adaptive', 'Txn', 'Base'
    ]
    
    planned_features = [
        'Hierarchical', 'Advanced', 'Enhanced', 'Unified', 'Composite',
        'TriggerDefinition', 'ErrorCollector'
    ]
    
    for cls in unimplemented_classes.keys():
        # 检查是否是明显的过时术语
        if any(keyword.lower() in cls.lower() for keyword in obsolete_keywords):
            categories["obsolete_terms"].add(cls)
        # 检查是否是规划中的功能
        elif any(keyword.lower() in cls.lower() for keyword in planned_features):
            categories["planned_features"].add(cls)
        # 检查是否看起来是需要实现的核心类
        elif re.match(r'^[A-Z][a-z]+(Manager|Executor|Handler|Validator|Parser|Engine)$', cls):
            categories["required_implementations"].add(cls)
        else:
            categories["documentation_issues"].add(cls)
    
    return categories

def main():
    """主分析函数"""
    print("=== SQLCC 文档-代码不一致性深度分析 ===\n")
    
    unimplemented_classes, src_classes = analyze_unimplemented_classes()
    
    print(f"\n发现 {len(unimplemented_classes)} 个文档中描述但未实现的类")
    print(f"实际源代码中有 {len(src_classes)} 个已实现的类\n")
    
    # 分类分析
    categories = categorize_classes(unimplemented_classes)
    
    print("\n=== 分类分析结果 ===")
    for category, classes in categories.items():
        print(f"\n{category}: {len(classes)} 个")
        if classes:
            for cls in list(classes)[:10]:  # 显示前10个
                print(f"  - {cls}")
            if len(classes) > 10:
                print(f"  ... 还有 {len(classes) - 10} 个")
    
    # 生成建议
    print("\n=== 处理建议 ===")
    total_issues = sum(len(classes) for classes in categories.values())
    
    if categories["obsolete_terms"]:
        print(f"1. 清理过时术语 ({len(categories['obsolete_terms'])} 个):")
        print("   建议删除或更新相关文档内容")
        print("   示例术语:", list(categories["obsolete_terms"])[:5])
    
    if categories["planned_features"]:
        print(f"\n2. 规划中功能 ({len(categories['planned_features'])} 个):")
        print("   建议更新文档状态为'规划中'或制定实现计划")
        print("   示例:", list(categories["planned_features"])[:5])
    
    if categories["required_implementations"]:
        print(f"\n3. 需要实现的类 ({len(categories['required_implementations'])} 个):")
        print("   建议排期实现或明确废弃")
        print("   优先级示例:", list(categories["required_implementations"])[:5])
    
    if categories["documentation_issues"]:
        print(f"\n4. 文档质量问题 ({len(categories['documentation_issues'])} 个):")
        print("   建议审查和清理不规范的文档内容")
    
    print(f"\n总问题数: {total_issues}")
    
    return categories

if __name__ == "__main__":
    categories = main()