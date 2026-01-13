#!/bin/bash
# 迁移 execution_context_test.cpp 到正确层次

echo "=== 迁移 execution_context_test.cpp ==="

SOURCE_FILE="tests/unit/basic/execution_context_test.cpp"
TARGET_DIR="tests/level5_execution/execution_context"
TARGET_FILE="${TARGET_DIR}/execution_context_test.cpp"

# 检查源文件是否存在
if [ ! -f "$SOURCE_FILE" ]; then
    echo "错误: 源文件不存在: $SOURCE_FILE"
    exit 1
fi

# 创建目标目录
mkdir -p "$TARGET_DIR"

# 复制文件
echo "复制文件: $SOURCE_FILE -> $TARGET_FILE"
cp "$SOURCE_FILE" "$TARGET_FILE"

# 验证复制成功
if [ -f "$TARGET_FILE" ]; then
    echo "文件复制成功"

    # 显示文件大小对比
    SOURCE_SIZE=$(stat -c%s "$SOURCE_FILE")
    TARGET_SIZE=$(stat -c%s "$TARGET_FILE")
    echo "源文件大小: $SOURCE_SIZE bytes"
    echo "目标文件大小: $TARGET_SIZE bytes"

    if [ "$SOURCE_SIZE" -eq "$TARGET_SIZE" ]; then
        echo "文件完整性验证: 通过"
    else
        echo "警告: 文件大小不匹配，可能存在问题"
    fi
else
    echo "错误: 文件复制失败"
    exit 1
fi

# 更新目标文件的CMakeLists.txt
CMAKE_FILE="${TARGET_DIR}/CMakeLists.txt"
cat >> "$CMAKE_FILE" << 'EOF'

# execution_context_test.cpp
add_executable(execution_context_test execution_context_test.cpp)
target_link_libraries(execution_context_test
    PRIVATE
    gtest
    gtest_main
    pthread
    ${CMAKE_DL_LIBS}
    sqlcc_core_lib
    sqlcc_parser
    sqlcc_config_manager
    sqlcc_storage_engine
    sqlcc_transaction_manager
    sqlcc_executor
)
add_test(NAME execution_context_test COMMAND execution_context_test)
EOF

echo "CMakeLists.txt已更新"

# 备份并注释掉原文件的CMakeLists.txt中的相关配置
ORIGINAL_CMAKE="tests/CMakeLists_new.txt"
if [ -f "$ORIGINAL_CMAKE" ]; then
    echo "备份原CMakeLists.txt配置..."
    cp "$ORIGINAL_CMAKE" "${ORIGINAL_CMAKE}.backup_execution_context"

    # 注释掉execution_context_test相关的行
    sed -i 's|add_executable(execution_context_test |# MIGRATED: add_executable(execution_context_test |g' "$ORIGINAL_CMAKE"
    sed -i 's|target_link_libraries(execution_context_test|# MIGRATED: target_link_libraries(execution_context_test|g' "$ORIGINAL_CMAKE"
    sed -i 's|add_test(NAME execution_context_test |# MIGRATED: add_test(NAME execution_context_test |g' "$ORIGINAL_CMAKE"

    echo "原CMakeLists.txt已备份并更新"
fi

# 创建迁移记录
MIGRATION_LOG="test_migration_log.txt"
echo "$(date): MIGRATED execution_context_test.cpp from tests/unit/basic/ to tests/level5_execution/execution_context/" >> "$MIGRATION_LOG"
echo "Source: $SOURCE_FILE" >> "$MIGRATION_LOG"
echo "Target: $TARGET_FILE" >> "$MIGRATION_LOG"
echo "Reason: File was incorrectly classified as level 1 but depends on level 5 execution engine components" >> "$MIGRATION_LOG"
echo "---" >> "$MIGRATION_LOG"

echo "迁移记录已保存到: $MIGRATION_LOG"

# 验证迁移后的文件是否可以编译
echo "验证新位置文件的编译配置..."

# 检查新的CMakeLists.txt是否正确
if grep -q "execution_context_test" "$CMAKE_FILE"; then
    echo "新CMakeLists.txt配置验证: 通过"
else
    echo "警告: 新CMakeLists.txt可能有问题"
fi

echo ""
echo "=== execution_context_test.cpp 迁移完成 ==="
echo "📁 新位置: $TARGET_FILE"
echo "🔧 CMake配置已更新"
echo "📋 迁移记录已保存"
echo ""
echo "注意: 原文件仍保留，可在验证无误后删除"