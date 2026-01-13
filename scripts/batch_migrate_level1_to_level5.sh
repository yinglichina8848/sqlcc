#!/bin/bash
# 批量迁移层次1到层次5的测试文件脚本

echo "=== 批量迁移层次1→层次5测试文件 ==="

# 需要迁移的测试文件列表（基于Bazel依赖分析）
MIGRATION_TARGETS=(
    "sql_executor_core_test"    # 依赖: core, database_manager, execution_context, sql_executor
    "json_operations_test"      # 依赖: sql_executor, sql_parser
    "stored_procedure_test"     # 依赖: sql_executor, sql_parser
    "trigger_test"              # 依赖: sql_executor, sql_parser
)

MIGRATION_LOG="batch_migration_level1_to_level5.log"
> "$MIGRATION_LOG"

echo "迁移开始时间: $(date)" >> "$MIGRATION_LOG"
echo "迁移类型: 层次1 → 层次5 (基础工具 → 执行引擎)" >> "$MIGRATION_LOG"
echo "" >> "$MIGRATION_LOG"

# 迁移函数
migrate_test_file() {
    local test_name="$1"
    local source_dir="tests/unit/basic"
    local target_dir="tests/level5_execution/${test_name//_test/}"
    local source_file="$source_dir/${test_name}.cpp"
    local target_file="$target_dir/${test_name}.cpp"

    echo "迁移测试: $test_name"
    echo "  源位置: $source_file"
    echo "  目标位置: $target_file"

    # 检查源文件是否存在
    if [ ! -f "$source_file" ]; then
        echo "  ❌ 错误: 源文件不存在: $source_file"
        echo "$(date): ERROR - Source file not found: $source_file" >> "$MIGRATION_LOG"
        return 1
    fi

    # 创建目标目录
    mkdir -p "$target_dir"

    # 复制文件
    if cp "$source_file" "$target_file"; then
        echo "  ✅ 文件复制成功"

        # 创建BUILD.bazel配置
        create_build_config "$test_name" "$target_dir"

        # 备份原配置
        backup_original_config "$test_name"

        echo "$(date): SUCCESS - Migrated $test_name from level1 to level5" >> "$MIGRATION_LOG"
        echo "  Source: $source_file" >> "$MIGRATION_LOG"
        echo "  Target: $target_file" >> "$MIGRATION_LOG"
        echo "" >> "$MIGRATION_LOG"

        return 0
    else
        echo "  ❌ 错误: 文件复制失败"
        echo "$(date): ERROR - Failed to copy $source_file" >> "$MIGRATION_LOG"
        return 1
    fi
}

# 创建BUILD.bazel配置
create_build_config() {
    local test_name="$1"
    local target_dir="$2"
    local build_file="$target_dir/BUILD.bazel"

    # 根据测试类型确定依赖
    local deps=""
    case "$test_name" in
        "sql_executor_core_test")
            deps='"//src/core:core",
        "//src/core:database_manager",
        "//src:execution_context",
        "//src/sql_executor:sql_executor",
        "@com_google_googletest//:gtest_main"'
            ;;
        "json_operations_test")
            deps='"//src/sql_executor:sql_executor",
        "//src/sql_parser:sql_parser",
        "@com_google_googletest//:gtest_main"'
            ;;
        "stored_procedure_test")
            deps='"//src/sql_executor:sql_executor",
        "//src/sql_parser:sql_parser",
        "@com_google_googletest//:gtest_main"'
            ;;
        "trigger_test")
            deps='"//src/sql_executor:sql_executor",
        "//src/sql_parser:sql_parser",
        "@com_google_googletest//:gtest_main"'
            ;;
        *)
            deps='"@com_google_googletest//:gtest_main"'
            ;;
    esac

    cat > "$build_file" << EOF
load("@rules_cc//cc:defs.bzl", "cc_test")

cc_test(
    name = "${test_name}",
    srcs = ["${test_name}.cpp"],
    deps = [
        ${deps}
    ],
    copts = [
        "-std=c++20",
        "-stdlib=libc++",
        "-Wall",
        "-Wextra",
    ],
    linkopts = [
        "-stdlib=libc++",
        "-lc++abi",
    ],
    features = ["cpp20_modules"],
)
EOF

    echo "  ✅ BUILD.bazel配置已创建: $build_file"
}

# 备份原配置
backup_original_config() {
    local test_name="$1"
    local original_build="tests/unit/basic/BUILD.bazel"

    if [ -f "$original_build" ]; then
        # 注释掉相关配置行
        sed -i "s|cc_test(|# MIGRATED: cc_test(|g" "$original_build"
        sed -i "s|name = \"$test_name\"|# MIGRATED: name = \"$test_name\"|g" "$original_build"
        echo "  ✅ 原配置已备份"
    fi
}

# 执行迁移
echo "开始迁移层次1→层次5的测试文件..."
echo ""

total_targets=${#MIGRATION_TARGETS[@]}
successful_migrations=0
failed_migrations=0

for test_name in "${MIGRATION_TARGETS[@]}"; do
    echo "----------------------------------------"
    if migrate_test_file "$test_name"; then
        ((successful_migrations++))
    else
        ((failed_migrations++))
    fi
    echo ""
done

# 生成总结报告
echo "=== 迁移完成总结 ==="
echo "总目标文件数: $total_targets"
echo "成功迁移: $successful_migrations"
echo "失败迁移: $failed_migrations"
echo ""
echo "迁移日志已保存到: $MIGRATION_LOG"
echo ""

if [ $successful_migrations -gt 0 ]; then
    echo "✅ 成功迁移的测试文件:"
    for test_name in "${MIGRATION_TARGETS[@]}"; do
        if [ -d "tests/level5_execution/${test_name//_test/}" ]; then
            echo "  - $test_name → tests/level5_execution/${test_name//_test/}/"
        fi
    done
    echo ""
fi

if [ $failed_migrations -gt 0 ]; then
    echo "❌ 迁移失败的测试文件:"
    for test_name in "${MIGRATION_TARGETS[@]}"; do
        if [ ! -d "tests/level5_execution/${test_name//_test/}" ]; then
            echo "  - $test_name"
        fi
    done
    echo ""
fi

echo "注意: 原文件仍保留，可在验证后删除"
echo "建议: 运行编译验证确保新配置正确"