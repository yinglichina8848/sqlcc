#!/usr/bin/env python3
"""
SQLCC Include Path Fixer Script
根据header_index.md文档系统性地修复include路径问题
"""

import os
import re
import sys
from pathlib import Path

class IncludePathFixer:
    def __init__(self, project_root):
        self.project_root = Path(project_root)
        self.include_dir = self.project_root / "include"
        self.src_dir = self.project_root / "src"

        # 根据header_index.md定义的关键头文件映射
        self.header_mappings = {
            # AST相关
            "ast_node.h": "sql_parser/ast_node.h",
            "ast_nodes.h": "sql_parser/ast_nodes.h",
            "node_visitor.h": "sql_parser/node_visitor.h",

            # 数据类型相关
            "data_types.h": "sql_parser/data_types.h",
            "token.h": "sql_parser/token.h",

            # 执行引擎相关
            "execution_result.h": "core/execution_result.h",
            "sql_executor_interface.h": "core/sql_executor_interface.h",

            # 存储引擎相关
            "buffer_pool.h": "storage/buffer_pool.h",
            "b_plus_tree.h": "storage/b_plus_tree.h",

            # 网络相关
            "network.h": "network/network.h",

            # 工具类相关
            "config_manager.h": "utils/config_manager.h",
            "logger.h": "utils/logger.h",

            # 异常相关
            "exception.h": "exception.h",
        }

    def find_source_files(self):
        """查找所有需要检查的源文件"""
        source_files = []

        # 查找所有.cpp和.h文件
        for root, dirs, files in os.walk(self.project_root):
            # 跳过一些不需要的目录
            dirs[:] = [d for d in dirs if not d.startswith('.') and d not in ['bazel-bin', 'bazel-out', 'bazel-sqlcc']]

            for file in files:
                if file.endswith(('.cpp', '.h', '.cc')):
                    source_files.append(Path(root) / file)

        return source_files

    def fix_include_paths(self, file_path):
        """修复单个文件中的include路径"""
        try:
            with open(file_path, 'r', encoding='utf-8') as f:
                content = f.read()

            original_content = content
            modified = False

            # 修复常见的include路径问题
            lines = content.split('\n')
            for i, line in enumerate(lines):
                if line.strip().startswith('#include'):
                    # 检查是否是相对路径的include
                    match = re.match(r'#include\s+"([^"]+)"', line)
                    if match:
                        include_path = match.group(1)

                        # 检查是否需要修复路径
                        if include_path in self.header_mappings:
                            new_path = self.header_mappings[include_path]
                            if new_path != include_path:
                                lines[i] = line.replace(f'"{include_path}"', f'"{new_path}"')
                                print(f"Fixed {file_path}: {include_path} -> {new_path}")
                                modified = True

                        # 检查是否缺少include/前缀的系统头文件
                        elif not include_path.startswith('include/') and include_path in self.header_mappings.values():
                            # 这个include路径已经是正确的
                            pass

            if modified:
                new_content = '\n'.join(lines)
                with open(file_path, 'w', encoding='utf-8') as f:
                    f.write(new_content)
                return True

        except Exception as e:
            print(f"Error processing {file_path}: {e}")

        return False

    def add_missing_includes(self, file_path):
        """为文件添加缺失的include"""
        try:
            # 获取文件名（不含路径）
            file_name = file_path.name

            with open(file_path, 'r', encoding='utf-8') as f:
                content = f.read()

            # 检查文件内容来确定需要哪些include
            includes_to_add = []

            # 只有对源文件（.cpp）才添加include，对头文件不添加
            if file_path.suffix == '.cpp':
                # 如果文件使用了AST节点但没有包含相应头文件
                if re.search(r'\b(AstNode|Expression|Statement|BinaryExpression)\b', content):
                    if '#include "sql_parser/ast_node.h"' not in content and '#include "ast_node.h"' not in content:
                        includes_to_add.append('#include "sql_parser/ast_node.h"')

                if re.search(r'\b(ColumnDefinition|CreateStatement|SelectStatement)\b', content):
                    if '#include "sql_parser/ast_nodes.h"' not in content and '#include "ast_nodes.h"' not in content:
                        includes_to_add.append('#include "sql_parser/ast_nodes.h"')

                if re.search(r'\b(DataType|DataValue)\b', content):
                    if '#include "sql_parser/data_types.h"' not in content and '#include "data_types.h"' not in content:
                        includes_to_add.append('#include "sql_parser/data_types.h"')

                if re.search(r'\b(Token|Token::Type)\b', content):
                    if '#include "sql_parser/token.h"' not in content and '#include "token.h"' not in content:
                        includes_to_add.append('#include "sql_parser/token.h"')

            # 如果有需要添加的include
            if includes_to_add:
                lines = content.split('\n')
                insert_pos = 0

                # 找到第一个非空行和注释后的位置
                for i, line in enumerate(lines):
                    if line.strip() and not line.strip().startswith('//') and not line.strip().startswith('/*'):
                        insert_pos = i
                        break

                # 在适当位置插入include
                for include in reversed(includes_to_add):
                    lines.insert(insert_pos, include)

                new_content = '\n'.join(lines)
                with open(file_path, 'w', encoding='utf-8') as f:
                    f.write(new_content)

                print(f"Added missing includes to {file_path}: {includes_to_add}")
                return True

        except Exception as e:
            print(f"Error processing {file_path}: {e}")

        return False

    def process_all_files(self):
        """处理所有源文件"""
        source_files = self.find_source_files()
        fixed_count = 0
        added_count = 0

        print(f"Found {len(source_files)} source files to process")

        for file_path in source_files:
            # 跳过一些不需要处理的文件
            if 'bazel-' in str(file_path) or 'test_' in file_path.name:
                continue

            if self.fix_include_paths(file_path):
                fixed_count += 1

            if self.add_missing_includes(file_path):
                added_count += 1

        print(f"Processing complete: {fixed_count} files had include paths fixed, {added_count} files had includes added")

def main():
    if len(sys.argv) != 2:
        print("Usage: python fix_include_paths.py <project_root>")
        sys.exit(1)

    project_root = sys.argv[1]
    if not os.path.exists(project_root):
        print(f"Project root {project_root} does not exist")
        sys.exit(1)

    fixer = IncludePathFixer(project_root)
    fixer.process_all_files()

if __name__ == "__main__":
    main()
