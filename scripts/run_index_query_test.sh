#!/bin/bash

# 索引查询测试运行脚本
echo "开始运行索引查询测试..."

# 设置环境变量
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH

# 编译测试
echo "编译测试文件..."
g++ -std=c++17 \
    -Iinclude \
    -lgtest \
    -lgtest_main \
    -pthread \
    tests/index_query_test.cpp \
    src/execution_engine.cpp \
    src/sql_parser/ast_nodes.cpp \
    src/sql_parser/parser.cpp \
    src/sql_parser/lexer.cpp \
    src/sql_parser/token.cpp \
    src/database_manager.cpp \
    src/table_storage.cpp \
    src/storage_engine.cpp \
    src/disk_manager.cpp \
    src/error_handler.cpp \
    src/logger.cpp \
    src/user_manager.cpp \
    src/system_database.cpp \
    src/b_plus_tree.cpp \
    -o index_query_test

if [ $? -ne 0 ]; then
    echo "编译失败"
    exit 1
fi

# 运行测试
echo "运行索引查询测试..."
./index_query_test

# 清理
rm -f index_query_test
echo "测试完成"
