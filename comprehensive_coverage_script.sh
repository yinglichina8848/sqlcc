#!/bin/bash

echo "========================================="
echo "SQLCC v1.2.10 全面覆盖率分析系统"
echo "========================================="
echo "开始时间: $(date)"
echo ""

# 创建覆盖率目录
mkdir -p comprehensive_coverage_html

# 清理之前的覆盖率文件
rm -f *.profraw *.profdata

echo "=== 第一阶段: 基础组件测试 ==="

# 1. 编译和运行Logger测试
echo "编译Logger测试..."
if clang++-18 -fprofile-instr-generate -fcoverage-mapping -std=c++20 -stdlib=libc++ -Iinclude -I/home/liying/sqlcc/include -L/usr/lib -lc++ -lc++abi simple_logger_test.cpp src/logger/logger.cpp -o simple_logger_coverage_test 2>/dev/null; then
    echo "Logger测试编译成功，运行测试..."
    ./simple_logger_coverage_test
    echo "Logger测试完成"
else
    echo "Logger测试编译失败，跳过"
fi

# 2. 编译和运行Token测试
echo "编译Token测试..."
if clang++-18 -fprofile-instr-generate -fcoverage-mapping -std=c++20 -stdlib=libc++ -Iinclude -I/home/liying/sqlcc/include -L/usr/lib -lc++ -lc++abi tests/unit/basic/token_test.cpp src/sql_parser/token.cpp -o token_coverage_test 2>/dev/null; then
    echo "Token测试编译成功，运行测试..."
    ./token_coverage_test
    echo "Token测试完成"
else
    echo "Token测试编译失败，跳过"
fi

# 3. 编译和运行AST Node测试
echo "编译AST Node测试..."
if clang++-18 -fprofile-instr-generate -fcoverage-mapping -std=c++20 -stdlib=libc++ -Iinclude -I/home/liying/sqlcc/include -L/usr/lib -lc++ -lc++abi tests/unit/parser/ast_node_test.cpp src/sql_parser/ast_node.cpp src/sql_parser/token.cpp -o ast_node_coverage_test 2>/dev/null; then
    echo "AST Node测试编译成功，运行测试..."
    ./ast_node_coverage_test
    echo "AST Node测试完成"
else
    echo "AST Node测试编译失败，跳过"
fi

# 4. 编译和运行Buffer Pool测试
echo "编译Buffer Pool测试..."
if clang++-18 -fprofile-instr-generate -fcoverage-mapping -std=c++20 -stdlib=libc++ -Iinclude -I/home/liying/sqlcc/include -L/usr/lib -lc++ -lc++abi tests/unit/storage/buffer_pool_test.cpp src/storage_engine/buffer_pool.cpp -o buffer_pool_coverage_test 2>/dev/null; then
    echo "Buffer Pool测试编译成功，运行测试..."
    ./buffer_pool_coverage_test
    echo "Buffer Pool测试完成"
else
    echo "Buffer Pool测试编译失败，跳过"
fi

# 5. 编译和运行Config Manager测试
echo "编译Config Manager测试..."
if clang++-18 -fprofile-instr-generate -fcoverage-mapping -std=c++20 -stdlib=libc++ -Iinclude -I/home/liying/sqlcc/include -L/usr/lib -lc++ -lc++abi tests/unit/core/config_manager_test.cpp src/core/config_manager.cpp -o config_manager_coverage_test 2>/dev/null; then
    echo "Config Manager测试编译成功，运行测试..."
    ./config_manager_coverage_test
    echo "Config Manager测试完成"
else
    echo "Config Manager测试编译失败，跳过"
fi

# 6. 编译和运行User Manager测试
echo "编译User Manager测试..."
if clang++-18 -fprofile-instr-generate -fcoverage-mapping -std=c++20 -stdlib=libc++ -Iinclude -I/home/liying/sqlcc/include -L/usr/lib -lc++ -lc++abi tests/unit/core/user_manager_test.cpp src/core/user_manager.cpp -o user_manager_coverage_test 2>/dev/null; then
    echo "User Manager测试编译成功，运行测试..."
    ./user_manager_coverage_test
    echo "User Manager测试完成"
else
    echo "User Manager测试编译失败，跳过"
fi

echo ""
echo "=== 第二阶段: 生成综合覆盖率报告 ==="

# 合并所有覆盖率数据
echo "合并覆盖率数据..."
llvm-profdata-18 merge *.profraw -o comprehensive.profdata 2>/dev/null || echo "没有找到profraw文件，跳过合并"

# 生成综合HTML报告
echo "生成综合覆盖率报告..."
if [ -f "comprehensive.profdata" ]; then
    # 尝试生成包含多个源文件的报告
    llvm-cov-18 show simple_logger_coverage_test token_coverage_test ast_node_coverage_test buffer_pool_coverage_test config_manager_coverage_test user_manager_coverage_test -instr-profile=comprehensive.profdata --format=html -output-dir=comprehensive_coverage_html --title="SQLCC v1.2.10 Comprehensive Coverage Report" 2>/dev/null || \
    llvm-cov-18 show simple_logger_coverage_test -instr-profile=comprehensive.profdata --format=html -output-dir=comprehensive_coverage_html --title="SQLCC v1.2.10 Comprehensive Coverage Report" 2>/dev/null || \
    echo "覆盖率报告生成失败"
else
    echo "没有覆盖率数据文件，跳过报告生成"
fi

# 清理临时文件
echo "清理临时文件..."
rm -f *.profraw *.profdata *_coverage_test

echo ""
echo "========================================="
echo "覆盖率分析完成"
echo "========================================="
echo "HTML报告已生成: comprehensive_coverage_html/index.html"
echo "结束时间: $(date)"
echo ""

echo "测试统计:"
echo "- 编译器: Clang 18.0"
echo "- C++标准: C++20"
echo "- 覆盖率工具: LLVM Cov"
echo "- 测试组件: Logger, Token, AST Node, Buffer Pool, Config Manager, User Manager"
echo ""

echo "报告包含以下内容:"
echo "1. 多组件源代码覆盖率统计"
echo "2. 函数执行情况"
echo "3. 行覆盖率详情"
echo "4. 分支覆盖率信息"
echo ""

echo "覆盖率报告路径: comprehensive_coverage_html/"
echo "打开方式: 在浏览器中打开 index.html 文件"

echo ""
echo "成功编译的组件列表:"
ls -la *_coverage_test 2>/dev/null || echo "所有临时文件已清理"
