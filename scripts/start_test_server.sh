#!/bin/bash
# 启动测试服务器脚本

echo "启动SQLCC测试服务器..."

# 设置测试环境变量
export SQLCC_TEST_MODE=1
export SQLCC_LOG_LEVEL=DEBUG
export SQLCC_TEST_DB_PATH="./data/sqlcc_test.db"

# 确保日志目录存在
mkdir -p test_reports

# 启动服务器（后台运行）
echo "启动服务器进程..."
./bazel-bin/sqlcc --port 18647 > test_reports/server_output.log 2>&1 &
SERVER_PID=$!

# 等待服务器启动
echo "等待服务器初始化..."
sleep 3

# 检查服务器是否成功启动
if kill -0 $SERVER_PID 2>/dev/null; then
    echo "服务器启动成功，PID: $SERVER_PID"
    # 保存PID以便后续清理
    echo $SERVER_PID > test_reports/server_pid.txt
    exit 0
else
    echo "ERROR: 服务器启动失败"
    cat test_reports/server_output.log
    exit 1
fi
