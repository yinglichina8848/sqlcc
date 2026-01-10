#!/bin/bash

# SQLCC MySQL Protocol Cross Performance Test Script
# Tests various combinations of server thread pool sizes and client configurations

set -e

# Configuration
SERVER_HOST="localhost"
SERVER_PORT=18647
SERVER_BINARY="//src/network:server_main"
CLIENT_BINARY="//src/network:true_crud_performance_test"

# Test configurations
SERVER_THREADS=(4 8 16 32 64 128)  # Server thread pool sizes
CLIENT_THREADS=(1 2 4 8 16 32 64)  # Client thread counts
CRUD_OPERATIONS=(100 200 400 800 1000)  # Operations per client thread

# Results file
RESULTS_FILE="cross_test_results_$(date +%Y%m%d_%H%M%S).csv"
echo "Server_Threads,Client_Threads,Operations_Per_Thread,Total_Time_ms,Operations_Per_Sec,Avg_Latency_ms,Success_Rate,Total_Inserts,Total_Selects,Total_Updates,Total_Deletes,Total_Errors" > "$RESULTS_FILE"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

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

# Function to start server
start_server() {
    local server_threads=$1
    log_info "Starting server with $server_threads threads..."

    # Kill any existing server
    pkill -f "server_main" || true
    sleep 2

    # Start server in background
    bazel run "$SERVER_BINARY" -t "$server_threads" > server.log 2>&1 &
    SERVER_PID=$!

    # Wait for server to start
    sleep 3

    # Check if server is running
    if ! pgrep -f "server_main" > /dev/null; then
        log_error "Server failed to start"
        cat server.log
        return 1
    fi

    log_success "Server started with PID $SERVER_PID"
    return 0
}

# Function to stop server
stop_server() {
    if [ -n "$SERVER_PID" ]; then
        log_info "Stopping server (PID: $SERVER_PID)..."
        kill "$SERVER_PID" 2>/dev/null || true
        wait "$SERVER_PID" 2>/dev/null || true
        SERVER_PID=""
    fi
}

# Function to run client test
run_client_test() {
    local client_threads=$1
    local operations=$2
    local server_threads=$3

    log_info "Running client test: $client_threads threads, $operations ops/thread (Server: $server_threads threads)"

    # Run client test and capture output
    local output
    output=$(bazel run "$CLIENT_BINARY" -h "$SERVER_HOST" -p "$SERVER_PORT" -t "$client_threads" -o "$operations" 2>&1)

    if [ $? -ne 0 ]; then
        log_error "Client test failed"
        echo "$output"
        return 1
    fi

    # Parse results from output
    local total_time=$(echo "$output" | grep "Total time:" | sed 's/.*Total time: \([0-9.]*\) ms.*/\1/')
    local ops_per_sec=$(echo "$output" | grep "Throughput:" | sed 's/.*Throughput: \([0-9.]*\) ops\/sec.*/\1/')
    local avg_latency=$(echo "$output" | grep "Avg latency:" | sed 's/.*Avg latency: \([0-9.]*\) ms.*/\1/')
    local success_rate=$(echo "$output" | grep "Success rate:" | sed 's/.*Success rate: \([0-9.]*\)%.*/\1/')

    # Parse CRUD operations
    local inserts=$(echo "$output" | grep "INSERT:" | sed 's/.*INSERT: \([0-9]*\).*/\1/')
    local selects=$(echo "$output" | grep "SELECT:" | sed 's/.*SELECT: \([0-9]*\).*/\1/')
    local updates=$(echo "$output" | grep "UPDATE:" | sed 's/.*UPDATE: \([0-9]*\).*/\1/')
    local deletes=$(echo "$output" | grep "DELETE:" | sed 's/.*DELETE: \([0-9]*\).*/\1/')

    # Calculate total errors
    local total_operations=$((client_threads * operations * 4))
    local total_successful=$((inserts + selects + updates + deletes))
    local total_errors=$((total_operations - total_successful))

    # Save results to CSV
    echo "$server_threads,$client_threads,$operations,$total_time,$ops_per_sec,$avg_latency,$success_rate,$inserts,$selects,$updates,$deletes,$total_errors" >> "$RESULTS_FILE"

    log_success "Test completed: ${ops_per_sec} ops/sec, ${success_rate}% success rate"
    return 0
}

# Function to run all tests for a server configuration
run_server_tests() {
    local server_threads=$1

    if ! start_server "$server_threads"; then
        return 1
    fi

    for client_threads in "${CLIENT_THREADS[@]}"; do
        for operations in "${CRUD_OPERATIONS[@]}"; do
            if run_client_test "$client_threads" "$operations" "$server_threads"; then
                log_info "Completed: Server($server_threads) + Client($client_threads) + Ops($operations)"
            else
                log_warning "Failed: Server($server_threads) + Client($client_threads) + Ops($operations)"
            fi

            # Small delay between tests
            sleep 2
        done
    done

    stop_server
}

# Main test execution
log_info "Starting SQLCC Cross Performance Test"
log_info "Results will be saved to: $RESULTS_FILE"
log_info "Server thread pool sizes: ${SERVER_THREADS[*]}"
log_info "Client thread counts: ${CLIENT_THREADS[*]}"
log_info "CRUD operations per thread: ${CRUD_OPERATIONS[*]}"
log_info "Total test combinations: $(( ${#SERVER_THREADS[@]} * ${#CLIENT_THREADS[@]} * ${#CRUD_OPERATIONS[@]} ))"

# Trap to ensure server is stopped on exit
trap stop_server EXIT

# Run all server configurations
for server_threads in "${SERVER_THREADS[@]}"; do
    log_info "========================================"
    log_info "Testing Server Configuration: $server_threads threads"
    log_info "========================================"

    if run_server_tests "$server_threads"; then
        log_success "Server configuration $server_threads completed"
    else
        log_error "Server configuration $server_threads failed"
    fi

    # Longer delay between server configurations
    sleep 5
done

log_info "========================================"
log_info "Cross Performance Test Completed"
log_info "========================================"
log_info "Results saved to: $RESULTS_FILE"

# Generate summary report
log_info "Generating summary report..."
python3 -c "
import pandas as pd
import sys

try:
    df = pd.read_csv('$RESULTS_FILE')

    print('=== CROSS PERFORMANCE TEST SUMMARY ===')
    print()

    # Best configurations by throughput
    print('TOP 10 CONFIGURATIONS BY THROUGHPUT:')
    top_throughput = df.nlargest(10, 'Operations_Per_Sec')[['Server_Threads', 'Client_Threads', 'Operations_Per_Thread', 'Operations_Per_Sec', 'Success_Rate']]
    print(top_throughput.to_string(index=False))
    print()

    # Best configurations by success rate
    print('TOP 10 CONFIGURATIONS BY SUCCESS RATE:')
    top_success = df.nlargest(10, 'Success_Rate')[['Server_Threads', 'Client_Threads', 'Operations_Per_Thread', 'Operations_Per_Sec', 'Success_Rate']]
    print(top_success.to_string(index=False))
    print()

    # Server thread pool analysis
    print('AVERAGE PERFORMANCE BY SERVER THREAD POOL SIZE:')
    server_analysis = df.groupby('Server_Threads').agg({
        'Operations_Per_Sec': 'mean',
        'Success_Rate': 'mean',
        'Avg_Latency_ms': 'mean'
    }).round(2)
    print(server_analysis.to_string())
    print()

    # Find optimal server thread pool size
    optimal_server = server_analysis['Operations_Per_Sec'].idxmax()
    print(f'RECOMMENDED SERVER THREAD POOL SIZE: {optimal_server} threads')
    print(f'Achieves {server_analysis.loc[optimal_server, \"Operations_Per_Sec\"]:.2f} ops/sec average throughput')

except Exception as e:
    print(f'Error generating summary: {e}', file=sys.stderr)
    sys.exit(1)
"

log_success "All tests completed successfully!"