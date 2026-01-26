#!/usr/bin/env python3
"""
SQLCC 项目组件交互分析和文档生成脚本

此脚本分析SQLCC项目中不同组件之间的交互关系，并生成组件交互文档
"""

import os
import re
import sys
from pathlib import Path
from typing import List, Dict, Tuple, Set


def analyze_component_interactions():
    """分析组件间的交互关系"""
    print("分析组件间的交互关系...")
    
    # 定义组件及其对应的目录
    components = {
        'Storage Engine': ['src/storage_engine', 'include/storage_engine'],
        'Transaction Manager': ['src/transaction', 'include/transaction'], 
        'Index System': ['src/index', 'include/index', 'src/btree', 'src/b_plus_tree'],
        'Network Module': ['src/network', 'include/network'],
        'SQL Parser': ['src/sql_parser', 'include/sql_parser'],
        'SQL Executor': ['src/sql_executor', 'include/sql_executor'],
        'Config Manager': ['src/config_manager', 'include/config_manager']
    }
    
    interactions = {}
    
    # 为每个组件查找与其他组件的交互
    for comp_name, comp_dirs in components.items():
        interactions[comp_name] = {}
        
        # 遍历当前组件的文件
        for comp_dir in comp_dirs:
            dir_path = Path(comp_dir)
            if not dir_path.exists():
                continue
                
            # 修复这一行：将map转换为list再拼接
            all_files = list(dir_path.rglob('*.h')) + list(dir_path.rglob('*.cpp'))
            for file_path in all_files:
                with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
                    content = f.read()
                
                # 检查当前组件的文件中是否引用了其他组件的文件
                for other_comp, other_dirs in components.items():
                    if comp_name == other_comp:
                        continue
                    
                    # 检查是否包含其他组件的头文件
                    for other_dir in other_dirs:
                        short_dir = other_dir.split('/')[-1] if '/' in other_dir else other_dir
                        if f'#include "{other_dir}' in content or f'#include <{other_dir}' in content:
                            if other_comp not in interactions[comp_name]:
                                interactions[comp_name][other_comp] = set()
                            interactions[comp_name][other_comp].add(f"Include: {other_dir}")
                        
                        # 检查是否有对其他组件类的引用
                        # 例如：StorageEngine*，TransactionManager::method()等
                        pattern = rf'({other_comp.replace(" ", "").replace("-", "")}\s*::\s*\w+|{other_comp.replace(" ", "").replace("-", "")}\s*\*|\b{other_comp.replace(" ", "").replace("-", "")}\s*&|\b{other_comp.replace(" ", "").replace("-", "")}\s*\()'
                        matches = re.findall(pattern, content)
                        if matches:
                            if other_comp not in interactions[comp_name]:
                                interactions[comp_name][other_comp] = set()
                            for match in matches:
                                interactions[comp_name][other_comp].add(f"Usage: {match}")
    
    # 过滤掉没有交互的组件
    filtered_interactions = {k: v for k, v in interactions.items() if v}
    
    print(f"发现 {len(filtered_interactions)} 个组件存在交互关系")
    return filtered_interactions


def generate_interaction_diagram(interactions):
    """生成Mermaid格式的组件交互图"""
    print("生成组件交互图...")
    
    diagram = "```mermaid\ngraph TD\n"
    
    # 添加所有组件节点
    all_components = set()
    for comp, deps in interactions.items():
        all_components.add(comp)
        for dep in deps.keys():
            all_components.add(dep)
    
    for i, comp in enumerate(sorted(all_components)):
        node_id = f"C{i}"
        diagram += f"    {node_id}[{comp}]\n"
    
    # 添加交互关系边
    comp_to_id = {comp: f"C{i}" for i, comp in enumerate(sorted(all_components))}
    
    for comp, deps in interactions.items():
        comp_id = comp_to_id[comp]
        for dep in deps.keys():
            dep_id = comp_to_id[dep]
            diagram += f"    {comp_id} --> {dep_id}\n"
    
    diagram += "```\n"
    
    return diagram


def generate_interaction_documentation(interactions):
    """生成组件交互文档"""
    print("生成组件交互文档...")
    
    doc_dir = Path('docs/architecture')
    doc_dir.mkdir(exist_ok=True)
    
    with open(doc_dir / 'component_interactions.md', 'w', encoding='utf-8') as f:
        f.write("# SQLCC 组件交互关系文档\n\n")
        f.write("本文档描述了SQLCC项目中各组件之间的交互关系。\n\n")
        
        # 生成交互图
        interaction_diagram = generate_interaction_diagram(interactions)
        f.write("## 组件交互图\n\n")
        f.write(interaction_diagram)
        f.write("\n")
        
        # 详细描述交互关系
        f.write("## 交互详情\n\n")
        
        for comp, deps in interactions.items():
            f.write(f"### {comp}\n\n")
            f.write(f"{comp} 与以下组件存在交互关系：\n\n")
            
            for dep, details in deps.items():
                f.write(f"#### 与 {dep} 的交互\n\n")
                for detail in list(details)[:5]:  # 只显示前5个交互详情
                    f.write(f"- {detail}\n")
                if len(details) > 5:
                    f.write(f"- ... 还有 {len(details)-5} 个交互详情\n")
                f.write("\n")
            
            f.write("---\n\n")
        
        f.write("## 总结\n\n")
        f.write("SQLCC系统采用模块化设计，各组件间通过明确定义的接口进行交互。这种设计使得系统具有良好的可维护性和扩展性。")
    
    print(f"组件交互文档已保存到: {doc_dir}/component_interactions.md")
    return str(doc_dir / 'component_interactions.md')


def create_api_usage_examples():
    """创建API使用示例文档"""
    print("创建API使用示例文档...")
    
    examples_dir = Path('docs/api_examples')
    examples_dir.mkdir(exist_ok=True)
    
    # 示例API使用文档
    api_examples = {
        'storage_engine_example.md': """
# 存储引擎API使用示例

## 基本用法

```cpp
#include "storage_engine/storage_engine.h"

// 初始化存储引擎
auto storage = std::make_shared<StorageEngine>();
storage->Initialize();

// 创建表
TableSchema schema;
schema.SetName("users");
// ... 设置表结构 ...

storage->CreateTable(schema);

// 插入数据
Record record;
// ... 设置记录数据 ...
storage->Insert("users", record);
```

## 高级用法

```cpp
// 批量操作
std::vector<Record> records;
// ... 填充记录 ...
storage->BatchInsert("users", records);

// 查询操作
Query query;
query.SetTable("users");
query.AddCondition("age > 18");

auto results = storage->Select(query);
```
""",
        'transaction_example.md': """
# 事务管理API使用示例

## 基本事务操作

```cpp
#include "transaction_manager/transaction_manager.h"

// 创建事务管理器实例
auto tm = std::make_shared<TransactionManager>();

// 开始事务
auto txn = tm->Begin();
try {
    // 执行事务操作
    txn->Update("table1", key1, value1);
    txn->Update("table2", key2, value2);
    
    // 提交事务
    tm->Commit(txn);
} catch (...) {
    // 回滚事务
    tm->Abort(txn);
}
```

## 事务隔离级别

```cpp
// 设置隔离级别
auto txn = tm->Begin(IsolationLevel::READ_COMMITTED);

// 或者使用快照隔离
auto snapshot_txn = tm->Begin(IsolationLevel::SNAPSHOT_ISOLATION);
```
""",
        'index_example.md': """
# 索引系统API使用示例

## B+树索引操作

```cpp
#include "index/b_plus_tree.h"

// 创建B+树索引
auto index = std::make_shared<BPlusTreeIndex<int, RID>>("idx_users_age", 4096);

// 插入索引项
index->Insert(25, RID{1, 100});
index->Insert(30, RID{2, 200});

// 查找操作
std::vector<RID> results;
index->GetValue(25, results);

// 范围查询
auto range_results = index->Range(20, 35);
```

## 复合索引

```cpp
// 创建复合索引
auto composite_idx = std::make_shared<CompositeIndex>("idx_users_name_age");
composite_idx->AddField("name", DataType::VARCHAR);
composite_idx->AddField("age", DataType::INTEGER);

// 使用复合索引
composite_idx->Insert({"John", 25}, RID{1, 100});
```
"""
    }
    
    created_examples = []
    for filename, content in api_examples.items():
        filepath = examples_dir / filename
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(content.strip() + "\n")
        created_examples.append(str(filepath))
    
    print(f"创建了 {len(created_examples)} 个API使用示例")
    return created_examples


def main():
    """主函数"""
    print("开始执行SQLCC项目组件交互分析和API示例生成...")
    
    # 分析组件交互
    interactions = analyze_component_interactions()
    
    # 生成交互文档
    interaction_doc = generate_interaction_documentation(interactions)
    
    # 创建API使用示例
    api_examples = create_api_usage_examples()
    
    # 生成总结报告
    report_dir = Path('docs/interaction_analysis_reports')
    report_dir.mkdir(exist_ok=True)
    
    with open(report_dir / 'interaction_analysis_summary.md', 'w', encoding='utf-8') as f:
        f.write("# SQLCC 组件交互分析总结报告\n\n")
        
        f.write("## 分析摘要\n\n")
        f.write(f"- 发现 {len(interactions)} 个组件存在交互关系\n")
        f.write(f"- 生成了 {len(api_examples)} 个API使用示例\n")
        f.write(f"- 创建了组件交互文档: {interaction_doc}\n\n")
        
        f.write("## 组件交互概览\n\n")
        for comp, deps in interactions.items():
            f.write(f"- **{comp}** 与 {len(deps)} 个其他组件交互\n")
        
        f.write("\n## API示例列表\n\n")
        for example in api_examples:
            f.write(f"- {example}\n")
        
        f.write("\n## 后续建议\n\n")
        f.write("1. 根据实际需要完善更多API示例\n")
        f.write("2. 定期更新组件交互关系文档\n")
        f.write("3. 为关键API添加性能特征说明\n")
        f.write("4. 记录组件交互的最佳实践\n\n")
    
    print(f"分析总结报告已保存到: {report_dir}/interaction_analysis_summary.md")
    print("组件交互分析和API示例生成完成！")
    
    return 0


if __name__ == "__main__":
    sys.exit(main())