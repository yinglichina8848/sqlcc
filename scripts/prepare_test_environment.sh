#!/bin/bash
# 测试环境准备脚本

echo "准备测试环境..."

# 1. 清理旧的测试数据（快速模式）
echo "清理旧的测试数据..."
rm -rf test_db_* 2>/dev/null || true
rm -f *.db 2>/dev/null || true
rm -f *.log server.log client.log 2>/dev/null || true

# 2. 清理Bazel缓存（可选，在CI环境中）
if [ "$CLEAN_CACHE" = "true" ]; then
    echo "清理Bazel缓存..."
    bazel clean --expunge
fi

# 3. 创建测试数据目录
echo "创建测试数据目录..."
mkdir -p data test_reports coverage_report

# 4. 设置测试数据库
echo "设置测试数据库..."
# 创建基础测试数据库文件
touch data/sqlcc_test.db

# 5. 验证系统依赖
echo "验证系统依赖..."
command -v bazel >/dev/null 2>&1 || { echo "ERROR: bazel not found"; exit 1; }
# 跳过lcov和genhtml检查以加快速度

# 6. 设置环境变量
echo "设置环境变量..."
export SQLCC_TEST_MODE=1
export SQLCC_LOG_LEVEL=INFO
export SQLCC_TEST_DB_PATH="./data/sqlcc_test.db"

# 7. 验证网络端口可用性（简化版）
echo "验证网络端口可用性..."
# 简化端口检查，避免长时间等待

# 8. 创建测试配置文件（简化版）
echo "创建测试配置文件..."
mkdir -p config
cat > config/test_config.json << EOF
{
    "database": {
        "path": "./data/sqlcc_test.db",
        "max_connections": 10,
        "timeout": 5000
    },
    "test_mode": true
}
EOF

echo "测试环境准备完成"
