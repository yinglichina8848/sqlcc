#!/usr/bin/env python3
"""
修复测试BUILD.bazel文件中直接引用头文件的问题，替换为正确的Bazel库目标
"""

import os
import re
from pathlib import Path

def fix_build_file(file_path):
    """修复单个BUILD.bazel文件中的依赖"""
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()

        original_content = content

        # 创建映射表，将头文件引用替换为对应的库目标
        replacements = {
            # 核心模块
            r'"//src/core/core_database_manager.h"': r'"//src/core:core"',
            r'"//src/core/execution_context.h"': r'"//src/core:core"',
            r'"//src/core/execution_result.h"': r'"//src/core:core"',
            r'"//src/core/permission_validator.h"': r'"//src/core:core"',
            r'"//src/core/system_database.h"': r'"//src/core:core"',
            r'"//src/core/user_manager.h"': r'"//src/core:core"',
            r'"//src/core/execution_strategy.h"': r'"//src/core:core"',

            # 执行模块
            r'"//src/execution/execution_strategy.h"': r'"//src/execution:execution"',
            r'"//src/execution/dml_execution_strategy.h"': r'"//src/execution:execution"',
            r'"//src/execution/ddl_execution_strategy.h"': r'"//src/execution:execution"',
            r'"//src/execution/dcl_execution_strategy.h"': r'"//src/execution:execution"',
            r'"//src/execution/utility_execution_strategy.h"': r'"//src/execution:execution"',
            r'"//src/execution/aggregate_engine.h"': r'"//src/execution:execution"',
            r'"//src/execution/cost_estimator.h"': r'"//src/execution:execution"',
            r'"//src/execution/comprehensive_task_executor.h"': r'"//src/execution:execution"',
            r'"//src/execution/function_executor.h"': r'"//src/execution:execution"',
            r'"//src/execution/group_by_executor.h"': r'"//src/execution:execution"',
            r'"//src/execution/join_executor.h"': r'"//src/execution:execution"',
            r'"//src/execution/load_data_executor.h"': r'"//src/execution:execution"',
            r'"//src/execution/query_plan_factory.h"': r'"//src/execution:execution"',
            r'"//src/execution/recursive_query_executor.h"': r'"//src/execution:execution"',
            r'"//src/execution/standalone_test.h"': r'"//src/execution:execution"',
            r'"//src/execution/task_executor.h"': r'"//src/execution:execution"',
            r'"//src/execution/task_executor_simple.h"': r'"//src/execution:execution"',
            r'"//src/execution/task_result.h"': r'"//src/execution:execution"',
            r'"//src/execution/test_runner.h"': r'"//src/execution:execution"',
            r'"//src/execution/unified_executor.h"': r'"//src/execution:execution"',
            r'"//src/execution/unified_query_plan.h"': r'"//src/execution:execution"',
            r'"//src/execution/window_function_executor.h"': r'"//src/execution:execution"',

            # SQL执行器模块
            r'"//src/sql_executor/domain_manager.h"': r'"//src/sql_executor:sql_executor"',
            r'"//src/sql_executor/enhanced_alter_table_manager.h"': r'"//src/sql_executor:sql_executor"',
            r'"//src/sql_executor/enhanced_trigger_manager.h"': r'"//src/sql_executor:sql_executor"',
            r'"//src/sql_executor/procedure_executor.h"': r'"//src/sql_executor:sql_executor"',
            r'"//src/sql_executor/procedure_function_manager.h"': r'"//src/sql_executor:sql_executor"',
            r'"//src/sql_executor/transaction_control_manager.h"': r'"//src/sql_executor:sql_executor"',

            # 事务管理
            r'"//src/transaction_manager.h"': r'"//src/transaction_manager:transaction_manager"',
            r'"//src/transaction_context.h"': r'"//src/transaction:transaction"',
            r'"//src/transaction_context_impl.h"': r'"//src/transaction:transaction"',
            r'"//src/transaction/savepoint_manager.h"': r'"//src/transaction:transaction"',

            # 数据库管理器
            r'"//src/database_manager/BUILD.bazel"': r'"//src/database_manager:database_manager"',
            r'"//src/database_manager.h"': r'"//src/database_manager:database_manager"',

            # 工具模块
            r'"//src/utils/config_manager.h"': r'"//src/utils:config_manager"',
            r'"//src/utils/smart_config_manager.h"': r'"//src/utils:config_manager"',
            r'"//src/utils/config_lifecycle.h"': r'"//src/utils:config_manager"',
            r'"//src/utils/config_snapshot.h"': r'"//src/utils:config_manager"',
            r'"//src/utils/logger.h"': r'"//src/utils:logger"',
        }

        # 执行替换
        for old, new in replacements.items():
            content = content.replace(old, new)

        # 替换所有其他可能的头文件引用
        # 匹配模式："//src/[模块]/[文件名].h"
        header_pattern = re.compile(r'"//src/([^/]+)/([^/]+)\.h"')

        def replace_header(match):
            module = match.group(1)
            filename = match.group(2)

            # 根据模块返回对应的库目标
            module_map = {
                'core': '//src/core:core',
                'execution': '//src/execution:execution',
                'sql_parser': '//src/sql_parser:sql_parser',
                'sql_executor': '//src/sql_executor:sql_executor',
                'storage_engine': '//src/storage_engine:storage_engine',
                'exception': '//src/exception:exception',
                'utils': '//src/utils:utils',
                'database_manager': '//src/database_manager:database_manager',
                'transaction_manager': '//src/transaction_manager:transaction_manager',
                'transaction': '//src/transaction:transaction',
                'page': '//src/page:page',
                'mocks': '//src/mocks:mocks',
                'monitoring': '//src/monitoring:monitoring',
                'security': '//src/security:security',
                'types': '//src/types:types',
                'procedure': '//src/procedure:procedure',
                'trigger': '//src/trigger:trigger',
                'network': '//src/network:network',
                'isql_network': '//src/isql_network:isql_network',
            }

            if module in module_map:
                return f'"{module_map[module]}"'
            else:
                return match.group(0)  # 未找到匹配的模块，保留原样

        content = header_pattern.sub(replace_header, content)

        # 修复可能的重复目标引用
        seen = set()
        lines = content.split('\n')
        new_lines = []
        for line in lines:
            if 'deps' in line:
                new_lines.append(line)
                continue
            if ']' in line and 'deps' in new_lines[-1]:
                new_lines.append(line)
                continue

            # 移除重复的依赖
            if '"//' in line and line.strip() in seen:
                continue
            if '"//' in line:
                seen.add(line.strip())
            new_lines.append(line)

        content = '\n'.join(new_lines)

        if content != original_content:
            with open(file_path, 'w', encoding='utf-8') as f:
                f.write(content)
            print(f"已修复: {file_path}")
            return True
        return False

    except Exception as e:
        print(f"处理文件时出错 {file_path}: {e}")
        return False

def find_and_fix_all_build_files():
    """查找并修复所有测试目录的BUILD.bazel文件"""
    tests_dir = Path('/home/liying/sqlcc/tests')

    if not tests_dir.exists():
        print(f"错误: 测试目录不存在: {tests_dir}")
        return

    fixed_count = 0

    # 递归查找所有BUILD.bazel文件
    for build_file in tests_dir.rglob('BUILD.bazel'):
        if fix_build_file(build_file):
            fixed_count += 1

    print(f"\n总共修复了 {fixed_count} 个BUILD.bazel文件")

if __name__ == '__main__':
    find_and_fix_all_build_files()
