#!/bin/bash
# 迁移下一批测试文件脚本

echo "=== 迁移下一批测试文件 ==="

# 基于Bazel分析结果，需要迁移的测试文件
# 1. permission_validator_test: 层次1 → 层次5 (依赖//src/execution:execution)
# 2. 其他需要确认的测试文件

MIGRATION_LOG="batch_migration_phase2.log"
> "$MIGRATION_LOG"

echo "迁移开始时间: $(date)" >> "$MIGRATION_LOG"

# 迁移permission_validator_test
echo "迁移 permission_validator_test..."

SOURCE_DIR="tests/unit/basic"
TARGET_DIR="tests/level5_execution/permission_validator"
SOURCE_FILE="$SOURCE_DIR/permission_validator_test.cpp"
TARGET_FILE="$TARGET_DIR/permission_validator_test.cpp"

# 检查源文件是否存在
if [ ! -f "$SOURCE_FILE" ]; then
    echo "错误: 源文件不存在: $SOURCE_FILE"
    exit 1
fi

# 创建目标目录
mkdir -p "$TARGET_DIR"

# 复制文件
echo "复制 $SOURCE_FILE -> $TARGET_FILE"
cp "$SOURCE_FILE" "$TARGET_FILE"

# 创建BUILD.bazel配置
cat > "$TARGET_DIR/BUILD.bazel" << 'EOF'
load("@rules_cc//cc:defs.bzl", "cc_test")

cc_test(
    name = "permission_validator_test",
    srcs = ["permission_validator_test.cpp"],
    deps = [
        "//src/core:core",
        "//src/execution:execution",
        "//src/sql_parser:sql_parser",
        "@com_google_googletest//:gtest_main",
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

echo "permission_validator_test 迁移完成" >> "$MIGRATION_LOG"
echo "  源位置: $SOURCE_FILE" >> "$MIGRATION_LOG"
echo "  目标位置: $TARGET_FILE" >> "$MIGRATION_LOG"
echo "  原因: 依赖//src/execution:execution，属于层次5执行引擎测试" >> "$MIGRATION_LOG"

# 备份原配置（注释掉）
ORIGINAL_BUILD="tests/unit/basic/BUILD.bazel"
if [ -f "$ORIGINAL_BUILD" ]; then
    sed -i 's|cc_test(|# MIGRATED: cc_test(|g' "$ORIGINAL_BUILD"
    sed -i 's|name = "permission_validator_test"|# MIGRATED: name = "permission_validator_test"|g' "$ORIGINAL_BUILD"
    echo "原配置已备份" >> "$MIGRATION_LOG"
fi

echo ""
echo "=== 迁移完成总结 ==="
echo "迁移的测试文件:"
echo "  - permission_validator_test (层次1 → 层次5)"
echo ""
echo "迁移日志已保存到: $MIGRATION_LOG"
echo ""
echo "注意: 原文件仍保留，可在验证后删除"