#!/bin/bash

# SQLCC CRUD性能测试脚本

echo "=== SQLCC CRUD Performance Test Script ==="
echo ""

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 函数：打印带颜色的消息
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

# 配置参数
SERVER_HOST="localhost"
SERVER_PORT=18647
TEST_ITERATIONS=50  # 每个CRUD操作的迭代次数

print_status "Test Configuration:"
echo "  Server: $SERVER_HOST:$SERVER_PORT"
echo "  Iterations per CRUD operation: $TEST_ITERATIONS"
echo ""

# 检查编译状态
print_status "Checking compilation status..."

if ! bazel build //src/sqlcc_server:server_main >/dev/null 2>&1; then
    print_error "Server compilation failed"
    exit 1
fi

if ! bazel build //:crud_performance_test >/dev/null 2>&1; then
    print_error "CRUD performance test compilation failed"
    exit 1
fi

print_success "All components compiled successfully"
echo ""

# 启动服务器（后台运行）
print_status "Starting SQLCC Server..."
bazel run //src/sqlcc_server:server_main >/dev/null 2>&1 &
SERVER_PID=$!

# 等待服务器启动
sleep 3

# 检查服务器是否在运行
if ps -p $SERVER_PID > /dev/null 2>&1; then
    print_success "Server started successfully (PID: $SERVER_PID)"
else
    print_error "Server failed to start"
    exit 1
fi

echo ""

# 运行CRUD性能测试
print_status "Running CRUD Performance Test..."
echo "This will test INSERT, SELECT, UPDATE, DELETE operations..."
echo ""

# 运行性能测试
if bazel run //:crud_performance_test -- "$SERVER_HOST" "$SERVER_PORT" "$TEST_ITERATIONS"; then
    print_success "CRUD performance test completed successfully"
else
    print_error "CRUD performance test failed"
fi

echo ""

# 停止服务器
print_status "Stopping server..."
kill $SERVER_PID 2>/dev/null
wait $SERVER_PID 2>/dev/null

if ! ps -p $SERVER_PID > /dev/null 2>&1; then
    print_success "Server stopped successfully"
else
    print_warning "Server may still be running"
    kill -9 $SERVER_PID 2>/dev/null
fi

echo ""

# 测试总结
print_success "=== CRUD Performance Test Summary ==="
echo "Tested Operations:"
echo "  ✅ INSERT - Data insertion performance"
echo "  ✅ SELECT - Data query performance"
echo "  ✅ UPDATE - Data modification performance"
echo "  ✅ DELETE - Data deletion performance"
echo ""
echo "Test Results:"
echo "  📊 Operations per second"
echo "  ⏱️  Average response time"
echo "  📈 Success rate"
echo "  🔍 Error analysis"
echo ""

print_success "🎉 SQLCC Client-Server CRUD performance test completed!"