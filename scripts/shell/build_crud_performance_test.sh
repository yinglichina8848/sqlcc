#!/bin/bash

# CRUD性能测试构建脚本
# 编译和运行CRUD性能测试，验证单操作<5ms的性能要求

echo "=== CRUD性能测试构建 ==="

# 检查必要的头文件
echo "检查头文件..."
if [ ! -f "./include/sqlcc/sqlcc.h" ]; then
    echo "错误: 缺少sqlcc.h头文件"
    exit 1
fi

if [ ! -f "./tests/performance/crud_performance_test.h" ]; then
    echo "错误: 缺少crud_performance_test.h头文件"
    exit 1
fi

if [ ! -f "./tests/performance/crud_performance_test.cc" ]; then
    echo "错误: 缺少crud_performance_test.cc源文件"
    exit 1
fi

# 编译CRUD性能测试
echo "编译CRUD性能测试..."
cd /home/liying/sqlcc_qoder/tests/performance

g++ -std=c++17 -I../../include -I. \
    crud_performance_test.cc crud_performance_test_main.cc \
    -lpthread -o crud_performance_test

if [ $? -ne 0 ]; then
    echo "CRUD性能测试编译失败"
    exit 1
fi

# 运行CRUD性能测试
echo "运行CRUD性能测试..."
./crud_performance_test --quick

if [ $? -ne 0 ]; then
    echo "CRUD性能测试运行失败"
    exit 1
fi

echo "=== CRUD性能测试运行完成 ==="
echo "✓ 测试程序编译成功"
echo "✓ 性能测试执行完成"
echo "✓ 单操作延迟验证进行中"

echo ""
echo "=== 性能测试摘要 ==="
echo "✓ CRUD性能测试 - 编译和运行成功"
echo "✓ 测试包含INSERT、SELECT、UPDATE、DELETE操作"
echo "✓ 验证1-10万行数据下的性能要求"
echo "✓ 单操作延迟目标: <5ms (SSD环境)"