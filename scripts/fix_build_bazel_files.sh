#!/bin/bash
# 批量修复 tests 目录下的 BUILD.bazel 文件
# 添加标准的 copts 和 tags 配置

set -e

PROJECT_ROOT="/home/liying/sqlcc"
TESTS_DIR="${PROJECT_ROOT}/tests"

echo "开始修复 tests 目录下的 BUILD.bazel 文件..."
echo "================================================"

# 查找所有 BUILD.bazel 文件
BUILD_FILES=$(find "${TESTS_DIR}" -name "BUILD.bazel" -type f)

TOTAL_FILES=$(echo "${BUILD_FILES}" | wc -l)
echo "找到 ${TOTAL_FILES} 个 BUILD.bazel 文件"
echo ""

FIXED_COUNT=0

for file in ${BUILD_FILES}; do
    echo "处理: ${file}"

    # 获取文件所在的相对路径，用于确定 level 标签
    REL_PATH=${file#${TESTS_DIR}/}

    # 确定正确的 level 标签
    LEVEL_TAG=""
    if [[ ${REL_PATH} == level1_foundation/* ]]; then
        LEVEL_TAG="level1"
    elif [[ ${REL_PATH} == level2_core/* ]]; then
        LEVEL_TAG="level2"
    elif [[ ${REL_PATH} == level2_core_services/* ]]; then
        LEVEL_TAG="level2"
    elif [[ ${REL_PATH} == level2_storage_engine/* ]]; then
        LEVEL_TAG="level2"
    elif [[ ${REL_PATH} == level3_transaction_manager/* ]]; then
        LEVEL_TAG="level3"
    elif [[ ${REL_PATH} == level4_sql_processing/* ]]; then
        LEVEL_TAG="level4"
    elif [[ ${REL_PATH} == level5_network/* ]]; then
        LEVEL_TAG="level5"
    elif [[ ${REL_PATH} == level6_integration/* ]] || [[ ${REL_PATH} == level6_enterprise/* ]]; then
        LEVEL_TAG="level6"
    elif [[ ${REL_PATH} == level7_integration/* ]]; then
        LEVEL_TAG="level7"
    elif [[ ${REL_PATH} == sql_parser/* ]]; then
        LEVEL_TAG="level2"
    elif [[ ${REL_PATH} == unit/* ]]; then
        LEVEL_TAG="unit"
    else
        LEVEL_TAG="unit"
    fi

    # 使用 Python 脚本来处理每个文件
    python3 << PYTHON_SCRIPT
import sys
import re

file_path = "${file}"
level_tag = "${LEVEL_TAG}"

try:
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()

    original_content = content
    modified = False

    # 查找所有 cc_test 块
    cc_test_pattern = r'(cc_test\s*\([^)]*\n(?:[^()]*\n)*?\))'
    matches = list(re.finditer(cc_test_pattern, content, re.DOTALL))

    for match in reversed(matches):
        test_block = match.group(1)
        original_block = test_block

        # 处理 copts - 检查是否需要添加标准 copts
        if 'copts' in test_block:
            # 检查是否包含 -g
            if '-g' not in test_block:
                test_block = re.sub(
                    r'(copts\s*=\s*\[)',
                    r'\1\n        "-g",',
                    test_block
                )
            # 检查是否包含 -Wall
            if '-Wall' not in test_block:
                test_block = re.sub(
                    r'(copts\s*=\s*\[)',
                    r'\1\n        "-Wall",',
                    test_block
                )
            # 检查是否包含 -Wextra
            if '-Wextra' not in test_block:
                test_block = re.sub(
                    r'(copts\s*=\s*\[)',
                    r'\1\n        "-Wextra",',
                    test_block
                )
        else:
            # 添加 copts 字段
            test_block = test_block.replace(
                'srcs = ',
                'srcs = \n    copts = [\n        "-g",\n        "-Wall",\n        "-Wextra",\n    ],\n    '
            )

        # 处理 tags - 检查是否需要添加 coverage 和 level 标签
        if 'tags' in test_block:
            # 检查是否包含 coverage
            if '"coverage"' not in test_block and "'coverage'" not in test_block:
                test_block = re.sub(
                    r'(tags\s*=\s*\[)',
                    r'\1\n        "coverage",',
                    test_block
                )
            # 检查是否包含 level 标签
            if f'"{level_tag}"' not in test_block and f"'{level_tag}'" not in test_block:
                test_block = re.sub(
                    r'(tags\s*=\s*\[)',
                    fr'\1\n        "{level_tag}",',
                    test_block
                )
            # 检查是否包含 unit 标签
            if '"unit"' not in test_block and "'unit'" not in test_block:
                test_block = re.sub(
                    r'(tags\s*=\s*\[)',
                    r'\1\n        "unit",',
                    test_block
                )
        else:
            # 添加 tags 字段
            test_block = test_block.replace(
                'deps = ',
                f'deps = \n    tags = [\n        "coverage",\n        "{level_tag}",\n        "unit",\n    ],\n    '
            )

        # 检查是否需要添加 testonly
        if 'testonly' not in test_block:
            test_block = test_block.rstrip(')\n') + ',\n    testonly = True,\n)'

        # 如果有修改，替换原内容
        if test_block != original_block:
            content = content[:match.start()] + test_block + content[match.end():]
            modified = True

    if modified:
        with open(file_path, 'w', encoding='utf-8') as f:
            f.write(content)
        print(f"  ✓ 已修复: {file_path}")
        sys.exit(0)
    else:
        print(f"  - 无需修改: {file_path}")
        sys.exit(1)

except Exception as e:
    print(f"  ✗ 错误: {e}")
    sys.exit(2)
PYTHON_SCRIPT

    if [ $? -eq 0 ]; then
        ((FIXED_COUNT++))
    fi

    echo ""
done

echo "================================================"
echo "完成! 共处理 ${TOTAL_FILES} 个文件，修复了 ${FIXED_COUNT} 个文件"
echo ""

exit 0