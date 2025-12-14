#!/bin/bash
# 构建验证脚本

echo "开始构建验证..."

# 记录开始时间
BUILD_START=$(date +%s)

# 1. 验证Bazel状态（快速模式）
echo "验证Bazel状态..."
bazel version

# 2. 跳过完整清理以节省时间

# 3. 只构建核心目标（快速模式）
echo "构建核心目标..."
if ! bazel build //:sqlcc //:isql //tests/unit/basic:permission_validator_test --keep_going; then
    echo "ERROR: 核心目标构建失败"
    exit 1
fi

# 4. 验证关键可执行文件
echo "验证关键可执行文件..."
if [ ! -f "bazel-bin/sqlcc" ]; then
    echo "ERROR: sqlcc 可执行文件未生成"
    exit 1
fi

if [ ! -f "bazel-bin/isql" ]; then
    echo "ERROR: isql 可执行文件未生成"
    exit 1
fi

# 6. 检查构建产物大小
echo "检查构建产物大小..."
SQLCC_SIZE=$(stat -c%s bazel-bin/sqlcc 2>/dev/null || stat -f%z bazel-bin/sqlcc 2>/dev/null || echo "0")
ISQL_SIZE=$(stat -c%s bazel-bin/isql 2>/dev/null || stat -f%z bazel-bin/isql 2>/dev/null || echo "0")

echo "sqlcc 大小: $SQLCC_SIZE bytes"
echo "isql 大小: $ISQL_SIZE bytes"

# 7. 验证构建时间
BUILD_END=$(date +%s)
BUILD_DURATION=$((BUILD_END - BUILD_START))
echo "构建耗时: ${BUILD_DURATION}秒"

# 8. 生成构建报告
cat > test_reports/build_validation_$(date +%Y%m%d_%H%M%S).txt << EOF
构建验证报告
生成时间: $(date)
构建耗时: ${BUILD_DURATION}秒

可执行文件:
- sqlcc: ${SQLCC_SIZE} bytes
- isql: ${ISQL_SIZE} bytes

构建状态: ✅ 成功
EOF

echo "构建验证完成"
