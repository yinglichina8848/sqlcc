#!/bin/bash

echo "Starting SqlCC Simple Encryption Testing"

# 启动服务器
echo "Starting server on port 18646..."
./sqlcc_server -p 18646 &
SERVER_PID=$!

# 等待服务器启动
sleep 2

# 检查服务器是否正在运行
if ps -p $SERVER_PID > /dev/null; then
    echo "Server started successfully"
else
    echo "Failed to start server"
    exit 1
fi

# 运行客户端测试
echo "Running client test..."
./isql_network -h localhost -p 18646 -u testuser -P testpass << EOF
SELECT 1;
quit
EOF

# 停止服务器
echo "Stopping server..."
kill $SERVER_PID 2>/dev/null
wait $SERVER_PID 2>/dev/null

echo "Simple encryption testing completed"