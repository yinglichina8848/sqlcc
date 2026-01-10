#!/bin/bash

# SQLCC 大规模数据积累性能测试脚本
# 测试服务器端在大量数据情况下的性能表现

set -e

echo "=================================================================="
echo "SQLCC LARGE SCALE DATA ACCUMULATION PERFORMANCE TEST"
echo "=================================================================="
echo "Testing server performance under growing dataset conditions"
echo "Server thread pool: 8-16 threads"
echo "Client threads: 8-128 threads"
echo "Operations per thread: 10,000"
echo "Test mode: Data accumulation (no DELETE operations)"
echo ""

# 配置参数
SERVER_HOST="localhost"
SERVER_PORT=18647
SERVER_BINARY="//src/sqlcc_server:server_main"
CLIENT_BINARY="//src/network:true_crud_performance_test"

# 测试场景
SERVER_THREADS_LIST=(8 16)     # 服务器线程池大小
CLIENT_THREADS_LIST=(8 16 32)  # 客户端线程数
OPERATIONS_PER_THREAD=10000    # 每线程操作数

echo "Test Scenarios:"
echo "Server thread pools: ${SERVER_THREADS_LIST[*]}"
echo "Client thread counts: ${CLIENT_THREADS_LIST[*]}"
echo "Operations per client thread: $OPERATIONS_PER_THREAD"
echo ""

# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# 启动服务器
start_server() {
    local server_threads=$1
    log_info "Starting SQLCC server with $server_threads threads..."

    # 停止任何现有服务器
    pkill -f "server_main" || true
    sleep 3

    # 启动服务器
    bazel run "$SERVER_BINARY" -t "$server_threads" > server.log 2>&1 &
    SERVER_PID=$!

    # 等待服务器启动
    sleep 5

    # 检查服务器是否运行
    if ! pgrep -f "server_main" > /dev/null; then
        log_error "Server failed to start"
        cat server.log
        return 1
    fi

    log_success "Server started with PID $SERVER_PID (threads: $server_threads)"
    return 0
}

# 停止服务器
stop_server() {
    if [ -n "$SERVER_PID" ]; then
        log_info "Stopping server (PID: $SERVER_PID)..."
        kill "$SERVER_PID" 2>/dev/null || true
        wait "$SERVER_PID" 2>/dev/null || true
        SERVER_PID=""
    fi
}

# 运行数据积累测试
run_data_accumulation_test() {
    local server_threads=$1
    local client_threads=$2

    log_info "Running data accumulation test: Server(${server_threads}t) + Client(${client_threads}t)"

    # 运行客户端测试，启用数据积累模式
    local output
    output=$(bazel run "$CLIENT_BINARY" \
        -h "$SERVER_HOST" \
        -p "$SERVER_PORT" \
        -t "$client_threads" \
        -o "$OPERATIONS_PER_THREAD" \
        -a 2>&1)

    local exit_code=$?

    if [ $exit_code -ne 0 ]; then
        log_error "Client test failed"
        echo "$output"
        return 1
    fi

    log_success "Test completed: Server(${server_threads}t) + Client(${client_threads}t)"

    # 提取关键性能指标
    local final_throughput=$(echo "$output" | grep "Final throughput:" | sed 's/.*Final throughput: \([0-9.]*\) ops\/sec.*/\1/')
    local final_latency=$(echo "$output" | grep "Final latency:" | sed 's/.*Final latency: \([0-9.]*\) ms.*/\1/')
    local degradation=$(echo "$output" | grep "Performance degradation:" | sed 's/.*Performance degradation: \([0-9.]*\)%.*/\1/')
    local assessment=$(echo "$output" | grep "Assessment:" | sed 's/.*Assessment: \([A-Z]*\).*/\1/')

    echo "Performance Summary:"
    echo "  Final Throughput: ${final_throughput} ops/sec"
    echo "  Final Latency: ${final_latency}ms"
    echo "  Performance Degradation: ${degradation}%"
    echo "  Assessment: $assessment"
    echo ""

    return 0
}

# 主测试执行
main() {
    echo "Starting Large Scale Data Accumulation Tests..."
    echo "=================================================="
    echo ""

    local total_tests=$(( ${#SERVER_THREADS_LIST[@]} * ${#CLIENT_THREADS_LIST[@]} ))
    local test_count=0

    # 陷阱确保服务器被停止
    trap stop_server EXIT

    for server_threads in "${SERVER_THREADS_LIST[@]}"; do
        log_info "==============================================="
        log_info "TESTING SERVER CONFIGURATION: $server_threads threads"
        log_info "==============================================="

        # 启动服务器
        if ! start_server "$server_threads"; then
            log_error "Skipping server configuration $server_threads due to startup failure"
            continue
        fi

        for client_threads in "${CLIENT_THREADS_LIST[@]}"; do
            ((test_count++))
            log_info "Test $test_count/$total_tests: Server(${server_threads}t) + Client(${client_threads}t)"

            if run_data_accumulation_test "$server_threads" "$client_threads"; then
                log_success "Test $test_count completed successfully"
            else
                log_warning "Test $test_count failed"
            fi

            # 测试间短暂延迟
            sleep 3
        done

        # 停止服务器，为下一个服务器配置做准备
        stop_server
        sleep 5
    done

    echo ""
    echo "=================================================================="
    log_success "ALL LARGE SCALE TESTS COMPLETED"
    echo "=================================================================="
    echo ""
    echo "Key Findings:"
    echo "1. Data accumulation impact on query performance"
    echo "2. Server stability under growing datasets"
    echo "3. Optimal thread pool configuration for large data volumes"
    echo "4. Performance degradation patterns over time"
    echo ""
    echo "Check individual test outputs above for detailed analysis."
}

# 执行主函数
main "$@"