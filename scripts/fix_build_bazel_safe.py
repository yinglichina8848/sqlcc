#!/usr/bin/env python3
"""
安全地修复 tests 目录下所有 BUILD.bazel 文件
添加标准的 copts 和 tags 配置，不破坏现有结构
"""

import os
import re
from pathlib import Path

# 标准编译选项
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
    return ["unit"]

def add_missing_copts(copts_block):
    """添加缺失的标准编译选项"""
    lines = copts_block.split('\n')
    new_lines = []
    added = set()

    # 先收集已有的copts
    for line in lines:
        new_lines.append(line)
        for opt in STANDARD_COPTS:
            if opt in line and '"' + opt + '"' in line:
                added.add(opt)

    # 添加缺失的copts
    if len(added) < len(STANDARD_COPTS):
        # 找到第一个copts行后插入缺失的选项
        result_lines = []
        first_copt_found = False
        for i, line in enumerate(lines):
            result_lines.append(line)
            if 'copts' in line and '[' in line and not first_copt_found:
                first_copt_found = True
                # 在下一行插入缺失的copts
                for opt in STANDARD_COPTS:
                    if opt not in added:
                        indent = '        '
                        result_lines.append(f'{indent}"{opt}",')
        return '\n'.join(result_lines)

    return copts_block

def add_missing_tags(tags_block, level_tags):
    """添加缺失的标签"""
    lines = tags_block.split('\n')
    new_lines = []
    added = set()

    # 收集已有的标签
    for line in lines:
        new_lines.append(line)
        for tag in ["coverage", "unit"] + level_tags:
            if f'"{tag}"' in line or f"'{tag}'" in line:
                added.add(tag)

    # 添加缺失的标签
    missing_tags = []
    if "coverage" not in added:
        missing_tags.append("coverage")
    if "unit" not in added:
        missing_tags.append("unit")
    for tag in level_tags:
        if tag not in added:
            missing_tags.append(tag)

    if missing_tags:
        # 找到第一个tags行后插入缺失的标签
        result_lines = []
        first_tag_found = False
        for i, line in enumerate(lines):
            result_lines.append(line)
            if 'tags' in line and '[' in line and not first_tag_found:
                first_tag_found = True
                indent = '        '
                for tag in missing_tags:
                    result_lines.append(f'{indent}"{tag}",')
        return '\n'.join(result_lines)

    return tags_block

def fix_cc_test_block(content, level_tags):
    """修复单个 cc_test 块"""
    # 查找 cc_test 开始和结束位置
    cc_test_start = content.find('cc_test(')
    if cc_test_start == -1:
        return content, False

    # 使用简单的括号计数找到匹配的结束位置
    paren_count = 0
    i = cc_test_start
    cc_test_end = -1
    in_string = False
    string_char = ''

    while i < len(content):
        char = content[i]

        # 处理字符串
        if char in ['"', "'"] and (i == 0 or content[i-1] != '\\'):
            if not in_string:
                in_string = True
                string_char = char
            elif char == string_char:
                in_string = False

        # 只在非字符串中计数括号
        if not in_string:
            if char == '(':
                paren_count += 1
            elif char == ')':
                paren_count -= 1
                if paren_count == 0:
                    cc_test_end = i + 1
                    break
        i += 1

    if cc_test_end == -1:
        return content, False

    cc_test_block = content[cc_test_start:cc_test_end]
    original_block = cc_test_block
    modified = False

    # 检查并修复 copts
    if 'copts' in cc_test_block:
        # 提取 copts 部分
        copts_match = re.search(r'copts\s*=\s*\[([^\]]*(?:\[[^\]]*\][^\]]*)*)\]', cc_test_block, re.DOTALL)
        if copts_match:
            copts_block = 'copts = [' + copts_match.group(1) + ']'
            new_copts_block = add_missing_copts(copts_block)
            if new_copts_block != copts_block:
                cc_test_block = cc_test_block.replace(copts_block, new_copts_block)
                modified = True
    else:
        # 添加 copts
        # 找到 srcs 或 deps 字段，在其后添加
        insert_pos = cc_test_block.find('deps = [')
        if insert_pos == -1:
            insert_pos = cc_test_block.find('srcs = [')
        if insert_pos == -1:
            insert_pos = cc_test_block.find(')')
        if insert_pos != -1:
            indent = '    '
            copts_addition = f'\n{indent}copts = [\n        "-g",\n        "-Wall",\n        "-Wextra",\n    ],'
            cc_test_block = cc_test_block[:insert_pos] + copts_addition + '\n    ' + cc_test_block[insert_pos:]
            modified = True

    # 检查并修复 tags
    if 'tags' in cc_test_block:
        # 提取 tags 部分
        tags_match = re.search(r'tags\s*=\s*\[([^\]]*(?:\[[^\]]*\][^\]]*)*)\]', cc_test_block, re.DOTALL)
        if tags_match:
            tags_block = 'tags = [' + tags_match.group(1) + ']'
            new_tags_block = add_missing_tags(tags_block, level_tags)
            if new_tags_block != tags_block:
                cc_test_block = cc_test_block.replace(tags_block, new_tags_block)
                modified = True
    else:
        # 添加 tags
        # 找到最后一个字段，在其后添加
        insert_pos = cc_test_block.rfind('],\n')
        if insert_pos != -1:
            indent = '    '
            tags_str = '    tags = [\n'
            tags_str += '        "coverage",\n'
            tags_str += '        "unit",\n'
            for tag in level_tags:
                tags_str += f'        "{tag}",\n'
            tags_str += '    ],'
            cc_test_block = cc_test_block[:insert_pos+2] + ',\n' + tags_str + cc_test_block[insert_pos+2:]
            modified = True

    # 检查并添加 testonly
    if 'testonly' not in cc_test_block:
        cc_test_block = cc_test_block.rstrip()
        if cc_test_block.endswith(')'):
            cc_test_block = cc_test_block[:-1].rstrip()
            cc_test_block += ',\n    testonly = True,\n)'
            modified = True

    if modified:
        return content[:cc_test_start] + cc_test_block + content[cc_test_end:], True

    return content, False

def fix_build_file(file_path):
    """修复单个 BUILD.bazel 文件"""
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()

        level_tags = determine_level_tags(file_path)
        original_content = content
        modified = False

        # 处理多个 cc_test 块
        max_iterations = 10  # 防止无限循环
        iteration = 0
        while iteration < max_iterations:
            new_content, block_modified = fix_cc_test_block(content, level_tags)
            if not block_modified:
                break
            content = new_content
            modified = True
            iteration += 1

        if modified:
            with open(file_path, 'w', encoding='utf-8') as f:
                f.write(content)
            print(f"✓ 已修复: {file_path}")
            return True
        else:
            print(f"- 无需修改: {file_path}")
            return False

    except Exception as e:
        print(f"✗ 错误处理文件 {file_path}: {e}")
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