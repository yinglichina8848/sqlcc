#!/bin/bash
# 构建验证脚本

echo "开始构建验证..."

# 记录开始时间
BUILD_START=$(date +%s)

# 1. 验证Bazel状态（快速模式）
echo "验证Bazel状态..."
bazel version

# 2. 跳过完整清理以节省时间

# 3. 只构建核心库（快速模式）
echo "构建核心库..."
if ! bazel build //src/sql_parser:sqlcc_parser //src/core:sqlcc_core_lib //src/storage_engine:sqlcc_storage_engine --keep_going; then
    echo "ERROR: 核心库构建失败"
    exit 1
fi

# 4. 验证关键库文件
echo "验证关键库文件..."
if [ ! -f "bazel-bin/src/sql_parser/libsqlcc_parser.so" ] && [ ! -f "bazel-bin/src/sql_parser/libsqlcc_parser.a" ]; then
    echo "ERROR: sqlcc_parser 库未生成"
    exit 1
fi

if [ ! -f "bazel-bin/src/core/libsqlcc_core_lib.so" ] && [ ! -f "bazel-bin/src/core/libsqlcc_core_lib.a" ]; then
    echo "ERROR: sqlcc_core_lib 库未生成"
    exit 1
fi

# 6. 检查构建产物大小
echo "检查构建产物大小..."
PARSER_SIZE=$(stat -c%s bazel-bin/src/sql_parser/libsqlcc_parser.so 2>/dev/null || stat -c%s bazel-bin/src/sql_parser/libsqlcc_parser.a 2>/dev/null || stat -f%z bazel-bin/src/sql_parser/libsqlcc_parser.so 2>/dev/null || stat -f%z bazel-bin/src/sql_parser/libsqlcc_parser.a 2>/dev/null || echo "0")
CORE_SIZE=$(stat -c%s bazel-bin/src/core/libsqlcc_core_lib.so 2>/dev/null || stat -c%s bazel-bin/src/core/libsqlcc_core_lib.a 2>/dev/null || stat -f%z bazel-bin/src/core/libsqlcc_core_lib.so 2>/dev/null || stat -f%z bazel-bin/src/core/libsqlcc_core_lib.a 2>/dev/null || echo "0")

echo "sqlcc_parser 大小: $PARSER_SIZE bytes"
echo "sqlcc_core_lib 大小: $CORE_SIZE bytes"

# 7. 验证构建时间
BUILD_END=$(date +%s)
BUILD_DURATION=$((BUILD_END - BUILD_START))
echo "构建耗时: ${BUILD_DURATION}秒"

# 8. 生成构建报告
cat > test_reports/build_validation_$(date +%Y%m%d_%H%M%S).txt << EOF
构建验证报告
生成时间: $(date)
构建耗时: ${BUILD_DURATION}秒

库文件:
- sqlcc_parser: ${PARSER_SIZE} bytes
- sqlcc_core_lib: ${CORE_SIZE} bytes

构建状态: ✅ 成功
EOF

echo "构建验证完成"
