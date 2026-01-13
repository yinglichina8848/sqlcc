#!/bin/bash
# 基于Bazel BUILD.bazel文件分析测试层次结构的脚本

echo "=== 基于Bazel BUILD.bazel分析测试层次结构 ==="

# 输出文件
BAZEL_ANALYSIS_FILE="bazel_test_hierarchy_analysis.txt"
HIERARCHY_MAPPING_FILE="bazel_hierarchy_mapping.txt"
MIGRATION_PLAN_FILE="bazel_migration_plan.md"

# 清空输出文件
> "$BAZEL_ANALYSIS_FILE"
> "$HIERARCHY_MAPPING_FILE"
> "$MIGRATION_PLAN_FILE"

echo "分析时间: $(date)" >> "$BAZEL_ANALYSIS_FILE"
echo "分析时间: $(date)" >> "$HIERARCHY_MAPPING_FILE"
echo "分析时间: $(date)" >> "$MIGRATION_PLAN_FILE"

echo "" >> "$BAZEL_ANALYSIS_FILE"
echo "" >> "$HIERARCHY_MAPPING_FILE"
echo "" >> "$MIGRATION_PLAN_FILE"

# 层次映射定义（基于BUILD.bazel依赖分析）
declare -A DEPENDENCY_TO_LEVEL

# 层次1: 基础工具 (utils, logger, config, basic types)
DEPENDENCY_TO_LEVEL["//src/utils:"]="1"
DEPENDENCY_TO_LEVEL["//src/logger:"]="1"
DEPENDENCY_TO_LEVEL["//include/utils:"]="1"
DEPENDENCY_TO_LEVEL["//src/config_manager:"]="1"

# 层次2: 核心组件 (database_manager, user_manager, system_db)
DEPENDENCY_TO_LEVEL["//src/core:"]="2"
DEPENDENCY_TO_LEVEL["//src/core:core"]="2"
DEPENDENCY_TO_LEVEL["//src/core:database_manager"]="2"
DEPENDENCY_TO_LEVEL["//src/core:user_manager"]="2"
DEPENDENCY_TO_LEVEL["//src/core:system_database"]="2"

# 层次3: 存储引擎 (storage_engine, buffer_pool, b_plus_tree, disk_manager)
DEPENDENCY_TO_LEVEL["//src/storage_engine:"]="3"
DEPENDENCY_TO_LEVEL["//src/storage_engine:storage_engine"]="3"

# 层次4: SQL解析器 (sql_parser, lexer, parser, ast)
DEPENDENCY_TO_LEVEL["//src/sql_parser:"]="4"
DEPENDENCY_TO_LEVEL["//src/sql_parser:sql_parser"]="4"

# 层次5: 执行引擎 (execution, transaction, sql_executor)
DEPENDENCY_TO_LEVEL["//src/execution:"]="5"
DEPENDENCY_TO_LEVEL["//src/sql_executor:"]="5"
DEPENDENCY_TO_LEVEL["//src:execution_context"]="5"
DEPENDENCY_TO_LEVEL["//src/transaction:"]="5"
DEPENDENCY_TO_LEVEL["//src:execution"]="5"

# 层次6: 网络通信 (network, security, encryption)
DEPENDENCY_TO_LEVEL["//src/network:"]="6"

# 层次7: 企业级特性 (procedure, trigger)
DEPENDENCY_TO_LEVEL["//src/procedure:"]="7"
DEPENDENCY_TO_LEVEL["//src/trigger:"]="7"

# 层次8: 系统集成 (end-to-end, performance)
# 集成测试通常没有特定的src依赖，或者依赖多个层次

# 查找所有BUILD.bazel文件
echo "查找所有BUILD.bazel文件..."
find tests -name "BUILD.bazel" | sort > bazel_files.txt

TOTAL_FILES=$(wc -l < bazel_files.txt)
echo "发现 $TOTAL_FILES 个BUILD.bazel文件"

echo "=== Bazel BUILD文件分析 ===" >> "$BAZEL_ANALYSIS_FILE"
echo "总BUILD文件数: $TOTAL_FILES" >> "$BAZEL_ANALYSIS_FILE"
echo "" >> "$BAZEL_ANALYSIS_FILE"

# 存储测试到层次的映射
declare -A TEST_TO_LEVEL
declare -A TEST_DEPENDENCIES

# 分析每个BUILD.bazel文件
while IFS= read -r bazel_file; do
    echo "分析: $bazel_file"

    # 提取相对路径
    relative_path="${bazel_file#tests/}"
    relative_path="${relative_path%/BUILD.bazel}"

    echo "文件: $relative_path/BUILD.bazel" >> "$BAZEL_ANALYSIS_FILE"
    echo "路径: $bazel_file" >> "$BAZEL_ANALYSIS_FILE"

    # 提取所有cc_test目标
    cc_tests=$(grep -n "cc_test(" "$bazel_file" | sed 's/:.*//')

    if [ -n "$cc_tests" ]; then
        echo "发现的cc_test目标:" >> "$BAZEL_ANALYSIS_FILE"

        # 处理每个cc_test
        for line_num in $cc_tests; do
            # 提取测试名称 (name字段)
            test_name=$(sed -n "${line_num},/^)/p" "$bazel_file" | grep 'name = "' | head -1 | sed 's/.*name = "\([^"]*\)".*/\1/')

            if [ -n "$test_name" ]; then
                echo "  测试: $test_name" >> "$BAZEL_ANALYSIS_FILE"

                # 提取依赖项
                deps_block=$(sed -n "${line_num},/^)/p" "$bazel_file" | grep -A 50 "deps = \[" | grep -B 50 "\]" | grep -v "deps = \[" | grep -v "\]")
                dependencies=$(echo "$deps_block" | grep -o '"//[^"]*"' | tr -d '"' | sort | uniq)

                TEST_DEPENDENCIES["$test_name"]="$dependencies"

                # 基于依赖确定层次
                max_level="1"
                actual_dependencies=""

                for dep in $dependencies; do
                    for prefix in "${!DEPENDENCY_TO_LEVEL[@]}"; do
                        if [[ "$dep" == "$prefix"* ]]; then
                            level="${DEPENDENCY_TO_LEVEL[$prefix]}"
                            if [ "$level" -gt "$max_level" ]; then
                                max_level="$level"
                            fi
                            actual_dependencies="$actual_dependencies $dep"
                        fi
                    done
                done

                TEST_TO_LEVEL["$test_name"]="$max_level"

                echo "    依赖: $dependencies" >> "$BAZEL_ANALYSIS_FILE"
                echo "    建议层次: $max_level (基于依赖: $actual_dependencies)" >> "$BAZEL_ANALYSIS_FILE"
                echo "" >> "$BAZEL_ANALYSIS_FILE"
            fi
        done
    else
        echo "  (无cc_test目标)" >> "$BAZEL_ANALYSIS_FILE"
    fi

    echo "" >> "$BAZEL_ANALYSIS_FILE"

done < bazel_files.txt

# 生成层次映射报告
echo "=== 测试层次映射 ===" >> "$HIERARCHY_MAPPING_FILE"

declare -A LEVEL_NAMES
LEVEL_NAMES["1"]="level1_foundation (基础工具测试)"
LEVEL_NAMES["2"]="level2_core (核心组件测试)"
LEVEL_NAMES["3"]="level3_storage (存储引擎测试)"
LEVEL_NAMES["4"]="level4_parser (SQL解析器测试)"
LEVEL_NAMES["5"]="level5_execution (执行引擎测试)"
LEVEL_NAMES["6"]="level6_network (网络通信测试)"
LEVEL_NAMES["7"]="level7_enterprise (企业级特性测试)"
LEVEL_NAMES["8"]="level8_integration (系统集成测试)"

for level in {1..8}; do
    echo "层次 $level: ${LEVEL_NAMES[$level]}" >> "$HIERARCHY_MAPPING_FILE"
    echo "测试列表:" >> "$HIERARCHY_MAPPING_FILE"

    found_tests=""
    for test_name in "${!TEST_TO_LEVEL[@]}"; do
        if [ "${TEST_TO_LEVEL[$test_name]}" = "$level" ]; then
            found_tests="$found_tests $test_name"
            echo "  - $test_name (依赖: ${TEST_DEPENDENCIES[$test_name]})" >> "$HIERARCHY_MAPPING_FILE"
        fi
    done

    if [ -z "$found_tests" ]; then
        echo "  (无测试)" >> "$HIERARCHY_MAPPING_FILE"
    fi
    echo "" >> "$HIERARCHY_MAPPING_FILE"
done

# 生成迁移计划
echo "# Bazel测试层次重构迁移计划" >> "$MIGRATION_PLAN_FILE"
echo "" >> "$MIGRATION_PLAN_FILE"
echo "## 概述" >> "$MIGRATION_PLAN_FILE"
echo "基于Bazel BUILD.bazel文件的依赖关系分析，重新确定测试层次分类。" >> "$MIGRATION_PLAN_FILE"
echo "" >> "$MIGRATION_PLAN_FILE"
echo "## 当前分析结果" >> "$MIGRATION_PLAN_FILE"
echo "- 分析BUILD文件数: $TOTAL_FILES" >> "$MIGRATION_PLAN_FILE"
echo "- 发现测试目标数: ${#TEST_TO_LEVEL[@]}" >> "$MIGRATION_PLAN_FILE"
echo "" >> "$MIGRATION_PLAN_FILE"

echo "## 层次分布" >> "$MIGRATION_PLAN_FILE"
for level in {1..8}; do
    level_count=0
    for test_name in "${!TEST_TO_LEVEL[@]}"; do
        if [ "${TEST_TO_LEVEL[$test_name]}" = "$level" ]; then
            ((level_count++))
        fi
    done
    echo "- 层次$level (${LEVEL_NAMES[$level]}): $level_count 个测试" >> "$MIGRATION_PLAN_FILE"
done
echo "" >> "$MIGRATION_PLAN_FILE"

echo "## 关键发现" >> "$MIGRATION_PLAN_FILE"

# 特别关注execution_context_test
if [ -n "${TEST_TO_LEVEL['execution_context_test']}" ]; then
    echo "### execution_context_test 分析" >> "$MIGRATION_PLAN_FILE"
    echo "- 当前层次: 1 (unit/basic)" >> "$MIGRATION_PLAN_FILE"
    echo "- 建议层次: ${TEST_TO_LEVEL['execution_context_test']} (level${TEST_TO_LEVEL['execution_context_test']}_*)" >> "$MIGRATION_PLAN_FILE"
    echo "- 依赖分析: ${TEST_DEPENDENCIES['execution_context_test']}" >> "$MIGRATION_PLAN_FILE"
    echo "- 结论: 需要迁移到层次${TEST_TO_LEVEL['execution_context_test']} ✅" >> "$MIGRATION_PLAN_FILE"
    echo "" >> "$MIGRATION_PLAN_FILE"
fi

echo "## 迁移策略" >> "$MIGRATION_PLAN_FILE"
echo "1. **基于依赖的自动分类**: 使用依赖关系分析确定层次" >> "$MIGRATION_PLAN_FILE"
echo "2. **层次隔离**: 确保低层次测试不依赖高层次组件" >> "$MIGRATION_PLAN_FILE"
echo "3. **渐进迁移**: 分批次迁移，避免大面积修改" >> "$MIGRATION_PLAN_FILE"
echo "" >> "$MIGRATION_PLAN_FILE"

echo "## 实施步骤" >> "$MIGRATION_PLAN_FILE"
echo "1. 备份当前BUILD.bazel配置" >> "$MIGRATION_PLAN_FILE"
echo "2. 创建新的层次目录结构" >> "$MIGRATION_PLAN_FILE"
echo "3. 按层次迁移cc_test目标" >> "$MIGRATION_PLAN_FILE"
echo "4. 更新依赖路径" >> "$MIGRATION_PLAN_FILE"
echo "5. 验证编译和测试执行" >> "$MIGRATION_PLAN_FILE"
echo "" >> "$MIGRATION_PLAN_FILE"

# 清理临时文件
rm bazel_files.txt

echo "分析完成!"
echo "结果已保存到:"
echo "  - $BAZEL_ANALYSIS_FILE (详细分析)"
echo "  - $HIERARCHY_MAPPING_FILE (层次映射)"
echo "  - $MIGRATION_PLAN_FILE (迁移计划)"