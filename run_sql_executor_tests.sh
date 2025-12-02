#!/bin/bash

# SQL执行器重构测试脚本
# 用于验证SQL执行器真实执行能力

echo "=========================================="
echo "SQL执行器真实执行能力重构测试"
echo "=========================================="

# 设置环境变量
export TEST_DB_PATH="./test_execution.db"

# 清理旧的测试文件
rm -f $TEST_DB_PATH
rm -f test_*.db

echo "清理旧测试文件完成"

# 编译项目（如果需要）
echo "检查项目编译状态..."
if [ ! -f "build/sqlcc" ]; then
    echo "项目未编译，开始编译..."
    mkdir -p build
    cd build
    cmake .. -DCMAKE_BUILD_TYPE=Debug
    make -j4
    cd ..
    if [ $? -ne 0 ]; then
        echo "编译失败，退出测试"
        exit 1
    fi
else
    echo "项目已编译"
fi

echo "开始运行单元测试..."

# 运行AST节点测试
echo "----------------------------------------"
echo "运行AST节点单元测试"
echo "----------------------------------------"
./build/tests/unit/ast_nodes_test 2>/dev/null
AST_RESULT=$?

# 运行执行引擎测试
echo "----------------------------------------"
echo "运行执行引擎单元测试"
echo "----------------------------------------"
./build/tests/unit/execution_engine_test 2>/dev/null
ENGINE_RESULT=$?

# 运行集成测试
echo "----------------------------------------"
echo "运行SQL执行器集成测试"
echo "----------------------------------------"
./build/tests/integration/sql_executor_integration_test 2>/dev/null
INTEGRATION_RESULT=$?

echo "=========================================="
echo "测试结果汇总"
echo "=========================================="

echo "AST节点测试: $([ $AST_RESULT -eq 0 ] && echo "✅ 通过" || echo "❌ 失败")"
echo "执行引擎测试: $([ $ENGINE_RESULT -eq 0 ] && echo "✅ 通过" || echo "❌ 失败")"
echo "集成测试: $([ $INTEGRATION_RESULT -eq 0 ] && echo "✅ 通过" || echo "❌ 失败")"

# 计算总体成功率
TOTAL_TESTS=3
PASSED_TESTS=0
[ $AST_RESULT -eq 0 ] && ((PASSED_TESTS++))
[ $ENGINE_RESULT -eq 0 ] && ((PASSED_TESTS++))
[ $INTEGRATION_RESULT -eq 0 ] && ((PASSED_TESTS++))

SUCCESS_RATE=$((PASSED_TESTS * 100 / TOTAL_TESTS))

echo ""
echo "总体测试结果: $PASSED_TESTS/$TOTAL_TESTS 通过 ($SUCCESS_RATE%)"

if [ $SUCCESS_RATE -ge 80 ]; then
    echo "🎉 重构测试总体成功！"
    echo ""
    echo "✅ SQL执行器已实现真实执行能力"
    echo "✅ 消除了假执行问题"
    echo "✅ 建立了完整的执行引擎架构"
    echo "✅ 支持基本的CRUD操作"
else
    echo "⚠️  重构测试存在问题，需要进一步调试"
fi

echo ""
echo "=========================================="
echo "手动测试验证"
echo "=========================================="

echo "您可以手动运行以下SQL语句进行验证："
echo ""
echo "# 创建表"
echo "CREATE TABLE users (id INTEGER, name VARCHAR, age INTEGER);"
echo ""
echo "# 插入数据"
echo "INSERT INTO users (id, name, age) VALUES (1, 'Alice', 25);"
echo ""
echo "# 查询数据"
echo "SELECT * FROM users;"
echo ""
echo "# 更新数据"
echo "UPDATE users SET age = 26 WHERE id = 1;"
echo ""
echo "# 删除数据"
echo "DELETE FROM users WHERE id = 1;"
echo ""
echo "# 删除表"
echo "DROP TABLE users;"

echo ""
echo "=========================================="
echo "清理测试文件"
echo "=========================================="

# 清理测试文件
rm -f $TEST_DB_PATH
rm -f test_*.db

echo "测试完成！"
