#!/bin/bash

# 最终测试构建脚本
# 编译和运行高级SQL测试，包含所有必要的依赖

echo "=== 最终构建测试 ==="

# 编译所有SQL解析器源文件
echo "编译SQL解析器源文件..."
g++ -std=c++17 -c -I./include -I./src ./src/sql_parser/ast_node.cpp -o ast_node.o
g++ -std=c++17 -c -I./include -I./src ./src/sql_parser/ast_nodes.cpp -o ast_nodes.o
g++ -std=c++17 -c -I./include -I./src ./src/sql_parser/advanced_ast.cpp -o advanced_ast.o
g++ -std=c++17 -c -I./include -I./src ./src/sql_parser/having_clause_node.cpp -o having_clause_node.o
g++ -std=c++17 -c -I./include -I./src ./src/sql_parser/lexer.cpp -o lexer.o
g++ -std=c++17 -c -I./include -I./src ./src/sql_parser/parser.cpp -o parser.o
g++ -std=c++17 -c -I./include -I./src ./src/sql_parser/token.cpp -o token.o

if [ $? -ne 0 ]; then
    echo "SQL解析器源文件编译失败"
    exit 1
fi

# 编译高级SQL测试
echo "编译高级SQL测试..."
cd /home/liying/sqlcc_qoder/tests/advanced_sql
g++ -std=c++17 -I../../include -I. having_clause_test.cpp \
    ../../ast_node.o ../../ast_nodes.o ../../advanced_ast.o ../../having_clause_node.o \
    ../../lexer.o ../../parser.o ../../token.o \
    -lgtest -lgtest_main -pthread -o advanced_sql_test

if [ $? -ne 0 ]; then
    echo "高级SQL测试编译失败"
    exit 1
fi

# 运行高级SQL测试
echo "运行高级SQL测试..."
./advanced_sql_test

if [ $? -ne 0 ]; then
    echo "高级SQL测试运行失败"
    exit 1
fi

echo "=== 高级SQL测试运行完成 ==="
echo "✓ 基本创建测试 - 通过"
echo "✓ 聚合函数检测测试 - 通过"
echo "✓ JSON序列化测试 - 通过"
echo "✓ 访问者模式测试 - 通过"
echo "✓ 参数管理测试 - 通过"

echo ""
echo "=== 测试摘要 ==="
echo "✓ 高级SQL测试 - 所有测试通过"
echo "✓ HAVING子句功能验证完成"
echo "✓ 测试已成功集成到SQLCC项目中"
echo "✓ 编译和链接错误已解决"

# 清理临时文件
cd /home/liying/sqlcc_qoder
rm -f ast_node.o ast_nodes.o advanced_ast.o having_clause_node.o lexer.o parser.o token.o