#!/bin/bash
# 通信协议测试脚本

echo "开始通信协议测试..."

# 记录开始时间
COMM_START=$(date +%s)

# 1. 启动测试服务器
echo "启动测试服务器..."
./scripts/start_test_server.sh &
SERVER_PID=$!

# 等待服务器启动
echo "等待服务器启动..."
sleep 5

# 检查服务器是否在运行
if ! kill -0 $SERVER_PID 2>/dev/null; then
    echo "ERROR: 服务器启动失败"
    exit 1
fi

# 2. 测试连接建立
echo "测试1: 连接建立"
if ! timeout 10 nc -z localhost 18647; then
    echo "ERROR: 无法连接到服务器端口 18647"
    kill $SERVER_PID 2>/dev/null || true
    exit 1
fi
echo "✅ 连接建立测试通过"

# 3. 测试MySQL协议握手
echo "测试2: MySQL协议握手"
if ./scripts/test_mysql_handshake.sh; then
    echo "✅ MySQL协议握手测试通过"
else
    echo "❌ MySQL协议握手测试失败"
    TEST_FAILED=1
fi

# 4. 测试基本SQL查询
echo "测试3: 基本SQL查询"
if ./scripts/test_basic_queries.sh; then
    echo "✅ 基本SQL查询测试通过"
else
    echo "❌ 基本SQL查询测试失败"
    TEST_FAILED=1
fi

# 5. 测试事务处理
echo "测试4: 事务处理"
if ./scripts/test_transaction_flow.sh; then
    echo "✅ 事务处理测试通过"
else
    echo "❌ 事务处理测试失败"
    TEST_FAILED=1
fi

# 6. 测试并发连接
echo "测试5: 并发连接"
if ./scripts/test_concurrent_connections.sh; then
    echo "✅ 并发连接测试通过"
else
    echo "❌ 并发连接测试失败"
    TEST_FAILED=1
fi

# 7. 测试错误处理
echo "测试6: 错误处理"
if ./scripts/test_error_handling.sh; then
    echo "✅ 错误处理测试通过"
else
    echo "❌ 错误处理测试失败"
    TEST_FAILED=1
fi

# 8. 清理
echo "清理测试环境..."
kill $SERVER_PID 2>/dev/null || true
sleep 2

# 9. 生成通信测试报告
COMM_END=$(date +%s)
COMM_DURATION=$((COMM_END - COMM_START))

cat > test_reports/communication_tests_$(date +%Y%m%d_%H%M%S).txt << EOF
通信协议测试报告
生成时间: $(date)
执行耗时: ${COMM_DURATION}秒

测试结果:
- 连接建立: ✅ 通过
- MySQL握手: $([ -z "$TEST_FAILED" ] && echo "✅ 通过" || echo "❌ 失败")
- 基本查询: $([ -z "$TEST_FAILED" ] && echo "✅ 通过" || echo "❌ 失败")
- 事务处理: $([ -z "$TEST_FAILED" ] && echo "✅ 通过" || echo "❌ 失败")
- 并发连接: $([ -z "$TEST_FAILED" ] && echo "✅ 通过" || echo "❌ 失败")
- 错误处理: $([ -z "$TEST_FAILED" ] && echo "✅ 通过" || echo "❌ 失败")

总体状态: $([ -z "$TEST_FAILED" ] && echo "✅ 全部通过" || echo "❌ 部分失败")
EOF

if [ -n "$TEST_FAILED" ]; then
    echo "通信协议测试发现问题，请检查服务器实现"
    exit 1
fi

echo "通信协议测试完成"
