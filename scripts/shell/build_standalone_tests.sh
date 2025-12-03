#!/bin/bash

# 独立测试构建脚本
# 避免主项目链接错误，专门编译和运行测试

echo "=== 构建独立测试 ==="

# 创建构建目录
BUILD_DIR="build_standalone"
if [ -d "$BUILD_DIR" ]; then
    echo "清理现有构建目录..."
    rm -rf "$BUILD_DIR"
fi

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# 配置CMake，仅构建测试相关目标
echo "配置CMake..."
cmake .. -DBUILD_TESTS=ON -DBUILD_EXAMPLES=ON -DCMAKE_BUILD_TYPE=Release

if [ $? -ne 0 ]; then
    echo "CMake配置失败"
    exit 1
fi

# 编译高级SQL测试
echo "编译高级SQL测试..."
make advanced_sql_test -j$(nproc)

if [ $? -ne 0 ]; then
    echo "高级SQL测试编译失败"
    exit 1
fi

# 编译CRUD性能测试
echo "编译CRUD性能测试..."
make crud_performance_test_standalone -j$(nproc)

if [ $? -ne 0 ]; then
    echo "CRUD性能测试编译失败"
    exit 1
fi

# 运行高级SQL测试
echo "运行高级SQL测试..."
./bin/advanced_sql_test

if [ $? -ne 0 ]; then
    echo "高级SQL测试运行失败"
    exit 1
fi

# 运行CRUD性能测试
echo "运行CRUD性能测试..."
./bin/crud_performance_test_standalone --quick --verbose

if [ $? -ne 0 ]; then
    echo "CRUD性能测试运行失败"
    exit 1
fi

echo "=== 所有测试运行完成 ==="
echo "测试结果保存在: $BUILD_DIR/performance_results/"

# 显示测试摘要
echo ""
echo "=== 测试摘要 ==="
echo "✓ 高级SQL测试 - 完成"
echo "✓ CRUD性能测试 - 完成"
echo "✓ 所有测试已成功集成到性能测试部分"