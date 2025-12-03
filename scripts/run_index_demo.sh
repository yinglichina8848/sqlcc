#!/bin/bash

# 索引优化功能演示脚本
echo "开始运行索引优化功能演示..."

# 设置环境变量
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH

# 编译演示程序
echo "编译索引优化演示程序..."
g++ -std=c++17 \
    -Iinclude \
    examples/index_optimization_demo.cpp \
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
    -o index_demo

if [ $? -ne 0 ]; then
    echo "编译失败"
    exit 1
fi

# 运行演示
echo "运行索引优化演示..."
./index_demo

# 清理
rm -f index_demo
echo "演示完成"
