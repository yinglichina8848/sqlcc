#!/bin/bash

# 网络加密功能测试脚本

BASE_DIR="/home/liying/sqlcc"
BUILD_DIR="$BASE_DIR/build"

# 确保脚本可执行
chmod +x "$0"

echo "Starting SqlCC Network Encryption Testing"

# 编译 sqlcc_server
echo "Compiling sqlcc_server..."
cd "$BUILD_DIR"
g++ -std=c++11 -o sqlcc_server "$BASE_DIR/src/sqlcc_server/server_main.cpp" -I"$BASE_DIR/include" -L"$BUILD_DIR/src" -lsqlcc_network -pthread -lgcov

# 编译 isql_network
echo "Compiling isql_network..."
g++ -std=c++11 -o isql_network "$BASE_DIR/src/isql_network/client_main.cpp" -I"$BASE_DIR/include" -L"$BUILD_DIR/src" -lsqlcc_network -pthread -lgcov

# 启动服务器
echo "Starting server on port 18645..."
"$BUILD_DIR/sqlcc_server" -p 18645 &
SERVER_PID=$!

# 等待服务器启动
sleep 2

# 检查服务器是否启动成功
if kill -0 $SERVER_PID 2>/dev/null; then
    echo "Server started successfully"
else
    echo "Failed to start server"
    exit 1
fi

# 运行客户端测试
echo "Running client test..."
timeout 10s "$BUILD_DIR/isql_network" -h localhost -p 18645 -u admin -P password

# 停止服务器
echo "Stopping server..."
kill $SERVER_PID 2>/dev/null || true
sleep 1 # 等待服务器完全停止

echo "Network encryption testing completed"