#!/usr/bin/env python3
"""
修复 tests 目录下所有 BUILD.bazel 文件，添加标准的 copts 和 tags 配置
"""

import os
import re
from pathlib import Path

# 需要添加的标准 copts
STANDARD_COPTS = ["-g", "-Wall", "-Wextra"]

# 需要添加的标准 tags
STANDARD_TAGS = ["coverage"]

# 根据目录路径确定 level 标签
LEVEL_TAGS = {
    "level1_foundation": ["level1"],
    "level2_core": ["level2"],
    "level2_core_services": ["level2"],
    "level2_storage_engine": ["level2"],
    "level3_transaction_manager": ["level3"],
    "level4_sql_processing": ["level4"],
    "level5_network": ["level5"],
    "level6_integration": ["level6"],
    "level6_enterprise": ["level6"],
    "level7_integration": ["level7"],
    "unit": ["unit"],
    "sql_parser": ["level2"],
    "temporary": ["temporary"],
}

def determine_level_tags(file_path):
    """根据文件路径确定 level 标签"""
    path_parts = file_path.parts
    for dir_name, tags in LEVEL_TAGS.items():
        if dir_name in path_parts:
            return tags
    return ["unit"]  # 默认

def fix_cc_test(content, file_path):
    """修复单个 cc_test 配置块"""
    level_tags = determine_level_tags(file_path)

    # 查找所有 cc_test 块
    pattern = r'(cc_test\s*\([^)]+\n(?:[^()]*\n)*?\))'
    matches = list(re.finditer(pattern, content, re.DOTALL))

    for match in reversed(matches):
        test_block = match.group(1)
        original_block = test_block

        # 处理 copts
        if 'copts' in test_block:
            # 检查是否已包含标准 copts
            for opt in STANDARD_COPTS:
                if opt not in test_block:
                    # 在 copts 列表中添加缺失的选项
                    test_block = re.sub(
                        r'(copts\s*=\s*\[)([^]]*)',
                        lambda m: f'{m.group(1)}"{opt}", {m.group(2)}',
                        test_block
                    )
        else:
            # 在 cc_test 中添加 copts 字段
            # 找到 name 行之后的位置插入
            copts_str = '\n    copts = [\n'
            for opt in STANDARD_COPTS:
                copts_str += f'        "{opt}",\n'
            copts_str += '    ],'
            test_block = test_block.replace(
                'srcs = ',
                f'srcs = {copts_str}\n    '
            )

        # 处理 tags
        if 'tags' in test_block:
            # 检查是否已包含 coverage 标签
            if 'coverage' not in test_block:
                # 在 tags 列表中添加 coverage
                test_block = re.sub(
                    r'(tags\s*=\s*\[)([^]]*)',
                    lambda m: f'{m.group(1)}"coverage", {m.group(2)}',
                    test_block
                )

            # 检查是否需要添加 level 标签
            for tag in level_tags:
                if f'"{tag}"' not in test_block and f"'{tag}'" not in test_block:
                    test_block = re.sub(
                        r'(tags\s*=\s*\[)([^]]*)',
                        lambda m: f'{m.group(1)}"{tag}", {m.group(2)}',
                        test_block
                    )
        else:
            # 添加 tags 字段
            tags_str = '\n    tags = [\n'
            tags_str += '        "coverage",\n'
            for tag in level_tags:
                tags_str += f'        "{tag}",\n'
            tags_str += '    ],'
            # 找到 deps 行之后的位置插入
            test_block = test_block.replace(
                'deps = ',
                f'deps = {tags_str}\n    '
            )

        # 如果有修改，替换原内容
        if test_block != original_block:
            content = content[:match.start()] + test_block + content[match.end():]

    return content

def fix_build_file(file_path):
    """修复单个 BUILD.bazel 文件"""
    print(f"处理文件: {file_path}")

    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()

        modified_content = fix_cc_test(content, file_path)

        if modified_content != content:
            with open(file_path, 'w', encoding='utf-8') as f:
                f.write(modified_content)
            print(f"  ✓ 已修复: {file_path}")
            return True
        else:
            print(f"  - 无需修改: {file_path}")
            return False

    except Exception as e:
        print(f"  ✗ 错误处理文件 {file_path}: {e}")
        return False

def main():
    """主函数"""
    tests_dir = Path('/home/liying/sqlcc/tests')

    if not tests_dir.exists():
        print(f"错误: 测试目录不存在: {tests_dir}")
        return 1

    # 查找所有 BUILD.bazel 文件
    build_files = list(tests_dir.rglob('BUILD.bazel'))

    print(f"找到 {len(build_files)} 个 BUILD.bazel 文件")
    print("=" * 60)

    modified_count = 0
    for build_file in build_files:
        if fix_build_file(build_file):
            modified_count += 1

    print("=" * 60)
    print(f"完成! 共处理 {len(build_files)} 个文件，修改了 {modified_count} 个文件")
    return 0

if __name__ == '__main__':
    exit(main())