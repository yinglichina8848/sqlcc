#!/usr/bin/env python3
"""
修复 tests 目录下所有 BUILD.bazel 文件，添加标准的 copts 和 tags 配置
"""

import os
import re
import sys
from pathlib import Path

# 需要添加的标准 copts
STANDARD_COPTS = ["-g", "-Wall", "-Wextra"]

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

def parse_cc_test_block(content):
    """解析并返回所有 cc_test 块的位置和内容"""
    results = []

    # 简单的括号匹配算法
    lines = content.split('\n')
    in_cc_test = False
    cc_test_start = 0
    paren_count = 0

    for i, line in enumerate(lines):
        if 'cc_test(' in line and not in_cc_test:
            in_cc_test = True
            cc_test_start = i
            paren_count = line.count('(') - line.count(')')
        elif in_cc_test:
            paren_count += line.count('(') - line.count(')')
            if paren_count == 0:
                # cc_test 块结束
                cc_test_block = '\n'.join(lines[cc_test_start:i+1])
                results.append((cc_test_start, i, cc_test_block))
                in_cc_test = False

    return results

def has_standard_copts(block):
    """检查是否包含标准 copts"""
    return all(opt in block for opt in STANDARD_COPTS)

def add_standard_copts(block):
    """添加标准 copts"""
    # 查找 srcs 字段，在其后添加 copts
    srcs_pattern = r'(srcs\s*=\s*\[[^\]]*\])'

    def add_copts(match):
        srcs = match.group(1)
        copts = '\n    copts = [\n        "-g",\n        "-Wall",\n        "-Wextra",\n    ],'
        return srcs + ',' + copts

    return re.sub(srcs_pattern, add_copts, block)

def has_coverage_tag(block):
    """检查是否包含 coverage 标签"""
    return '"coverage"' in block or "'coverage'" in block

def has_level_tag(block, level_tags):
    """检查是否包含 level 标签"""
    for tag in level_tags:
        if f'"{tag}"' in block or f"'{tag}'" in block:
            return True
    return False

def has_unit_tag(block):
    """检查是否包含 unit 标签"""
    return '"unit"' in block or "'unit'" in block

def add_standard_tags(block, level_tags):
    """添加标准 tags"""
    # 查找 deps 字段，在其后添加 tags
    deps_pattern = r'(deps\s*=\s*\[[^\]]*\])'

    def add_tags(match):
        deps = match.group(1)
        tags = '\n    tags = [\n        "coverage",'
        for tag in level_tags:
            tags += f'\n        "{tag}",'
        tags += '\n        "unit",\n    ],'
        return deps + ',' + tags

    return re.sub(deps_pattern, add_tags, block)

def has_testonly(block):
    """检查是否包含 testonly 字段"""
    return 'testonly' in block

def add_testonly(block):
    """添加 testonly 字段"""
    # 在 cc_test 块结束前添加
    block = block.rstrip()
    if block.endswith(')'):
        block = block[:-1].rstrip()
        block += ',\n    testonly = True,\n)'
    return block

def fix_cc_test_block(block, level_tags):
    """修复单个 cc_test 块"""
    original_block = block
    modified = False

    # 处理 copts
    if 'copts' in block:
        if not has_standard_copts(block):
            # 简单地跳过已存在 copts 的情况，不做修改
            pass
    else:
        block = add_standard_copts(block)
        modified = True

    # 处理 tags
    if 'tags' in block:
        if not has_coverage_tag(block) or not has_level_tag(block, level_tags) or not has_unit_tag(block):
            # 简单地跳过已存在 tags 的情况，不做修改
            pass
    else:
        block = add_standard_tags(block, level_tags)
        modified = True

    # 处理 testonly
    if not has_testonly(block):
        block = add_testonly(block)
        modified = True

    return block, modified

def fix_build_file(file_path):
    """修复单个 BUILD.bazel 文件"""
    print(f"处理文件: {file_path}")

    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()

        level_tags = determine_level_tags(file_path)

        # 解析所有 cc_test 块
        cc_test_blocks = parse_cc_test_block(content)

        if not cc_test_blocks:
            print(f"  - 无 cc_test 块: {file_path}")
            return False

        # 从后往前处理，避免位置变化
        modified = False
        for start, end, block in reversed(cc_test_blocks):
            new_block, block_modified = fix_cc_test_block(block, level_tags)

            if block_modified:
                # 替换内容
                new_content = content[:start] + new_block + content[end+1:]
                if new_content != content:
                    content = new_content
                    modified = True

        if modified:
            with open(file_path, 'w', encoding='utf-8') as f:
                f.write(content)
            print(f"  ✓ 已修复: {file_path}")
            return True
        else:
            print(f"  - 无需修改: {file_path}")
            return False

    except Exception as e:
        print(f"  ✗ 错误处理文件 {file_path}: {e}")
        import traceback
        traceback.print_exc()
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
