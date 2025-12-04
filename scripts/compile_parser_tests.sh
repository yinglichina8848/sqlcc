#!/bin/bash

# SQL Parser重构测试编译脚本
# 编译新增的SQL Parser测试文件

echo "🔧 编译SQL Parser重构测试..."
echo "==================================="

# 设置变量
BUILD_DIR="/home/liying/sqlcc_qoder/build_parser_tests"
SOURCE_DIR="/home/liying/sqlcc_qoder"
INCLUDE_DIR="$SOURCE_DIR/include"
TEST_DIR="$SOURCE_DIR/tests/sql_parser"

# 清理并创建构建目录
echo "📁 创建构建目录..."
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

# 编译函数
compile_test() {
    local test_name=$1
    local source_file="$TEST_DIR/${test_name}.cpp"
    local output_file="$BUILD_DIR/$test_name"
    
    echo "🔨 编译测试: $test_name"
    
    if [ ! -f "$source_file" ]; then
        echo "❌ 源文件不存在: $source_file"
        return 1
    fi
    
    # 使用g++编译，添加必要的包含路径和库
    g++ -std=c++17 -I"$INCLUDE_DIR" \
        -I"$INCLUDE_DIR/sql_parser" \
        -I"$INCLUDE_DIR/core" \
        -I"$INCLUDE_DIR/sql" \
        -o "$output_file" \
        "$source_file" \
        -pthread 2>/dev/null
        
    if [ $? -eq 0 ]; then
        echo "✅ 编译成功: $test_name"
        echo "📂 输出文件: $output_file"
    else
        echo "❌ 编译失败: $test_name"
        # 尝试使用基础语法检查
        echo "🔍 进行语法检查..."
        g++ -std=c++17 -fsyntax-only \
            -I"$INCLUDE_DIR" \
            -I"$INCLUDE_DIR/sql_parser" \
            -I"$INCLUDE_DIR/core" \
            -I"$INCLUDE_DIR/sql" \
            "$source_file" 2>&1 | head -10
    fi
    
    echo "----------------------------------------"
}

# 编译所有新增的测试文件
echo "📋 测试文件列表:"
echo "- parser_new_unit_test.cpp"
echo "- lexer_new_unit_test.cpp" 
echo "- token_new_unit_test.cpp"
echo "- parser_new_integration_test.cpp"
echo ""

echo "🚀 开始编译测试..."

# 编译各个测试文件
compile_test "parser_new_unit_test"
compile_test "lexer_new_unit_test"
compile_test "token_new_unit_test"
compile_test "parser_new_integration_test"

echo ""
echo "📊 编译总结:"
echo "==================================="

# 列出生成的可执行文件
if [ -d "$BUILD_DIR" ]; then
    echo "📂 构建目录内容:"
    ls -la "$BUILD_DIR" 2>/dev/null || echo "构建目录为空"
    
    executable_count=$(ls -1 "$BUILD_DIR" 2>/dev/null | wc -l)
    echo "✅ 成功编译的测试文件数量: $executable_count"
else
    echo "❌ 构建目录创建失败"
fi

echo ""
echo "🎯 下一步:"
echo "1. 检查编译结果"
echo "2. 运行通过测试的单元测试"
echo "3. 集成到CMake构建系统"
echo "4. 加入总测试框架"

echo ""
echo "==================================="
echo "SQL Parser测试编译完成！"
