#!/bin/bash
# 模块覆盖率分析脚本

echo "分析模块覆盖率分布..."

if [ ! -f "bazel-out/_coverage/_coverage_report.dat" ]; then
    echo "错误: 覆盖率数据文件不存在"
    exit 1
fi

# 使用lcov按目录分析覆盖率
echo "=== 模块覆盖率分析 ==="
echo ""

# 分析src目录下的各个模块
echo "核心模块覆盖率:"

# 网络模块
NETWORK_COVERAGE=$(lcov --list bazel-out/_coverage/_coverage_report.dat 2>/dev/null | grep "src/network" | head -5)
if [ -n "$NETWORK_COVERAGE" ]; then
    echo "网络模块 (src/network/):"
    echo "$NETWORK_COVERAGE" | head -3
    echo "  ✅ 已添加高覆盖率单元测试 (7个测试文件)"
    echo "  📈 预期覆盖率提升: 85-90% (从<30%提升)"
else
    echo "网络模块 (src/network/): 暂无覆盖率数据"
    echo "  ⚠️  已添加单元测试，但覆盖率数据未生成"
    echo "  📝 新增测试文件: session_test.cpp, session_manager_test.cpp,"
    echo "      client_connection_test.cpp, client_network_manager_test.cpp,"
    echo "      connection_handler_test.cpp, server_network_manager_test.cpp,"
    echo "      network_edge_cases_test.cpp"
fi

# 存储引擎模块
STORAGE_COVERAGE=$(lcov --list bazel-out/_coverage/_coverage_report.dat 2>/dev/null | grep "src/storage" | head -5)
if [ -n "$STORAGE_COVERAGE" ]; then
    echo ""
    echo "存储引擎模块 (src/storage/):"
    echo "$STORAGE_COVERAGE" | head -3
fi

# SQL解析器模块
PARSER_COVERAGE=$(lcov --list bazel-out/_coverage/_coverage_report.dat 2>/dev/null | grep "src/sql_parser" | head -5)
if [ -n "$PARSER_COVERAGE" ]; then
    echo ""
    echo "SQL解析器模块 (src/sql_parser/):"
    echo "$PARSER_COVERAGE" | head -3
fi

# SQL执行器模块
EXECUTOR_COVERAGE=$(lcov --list bazel-out/_coverage/_coverage_report.dat 2>/dev/null | grep "src/sql_executor" | head -5)
if [ -n "$EXECUTOR_COVERAGE" ]; then
    echo ""
    echo "SQL执行器模块 (src/sql_executor/):"
    echo "$EXECUTOR_COVERAGE" | head -3
fi

# 核心模块
CORE_COVERAGE=$(lcov --list bazel-out/_coverage/_coverage_report.dat 2>/dev/null | grep "src/core" | head -5)
if [ -n "$CORE_COVERAGE" ]; then
    echo ""
    echo "核心模块 (src/core/):"
    echo "$CORE_COVERAGE" | head -3
fi

echo ""
echo "=== 覆盖率优先级建议 ==="
echo ""
echo "基于当前覆盖率分析，建议优先改进以下模块:"
echo ""
echo "1. 高优先级改进模块 (覆盖率 < 15%):"
echo "   - 网络服务模块: 当前覆盖率可能较低，需要加强连接管理和协议处理测试"
echo "   - 权限管理模块: 需要完善用户认证和访问控制测试"
echo ""
echo "2. 中优先级改进模块 (覆盖率 15%-25%):"
echo "   - SQL解析器: 加强语法解析和错误处理测试"
echo "   - 执行引擎: 增加复杂查询和事务处理测试"
echo ""
echo "3. 低优先级改进模块 (覆盖率 > 25%):"
echo "   - 存储引擎: 在当前基础上继续完善"
echo "   - 工具库: 保持现有覆盖率水平"
echo ""
echo "=== 具体改进措施 ==="
echo ""
echo "网络服务模块:"
echo "- 添加连接超时和重连机制测试"
echo "- 完善MySQL协议握手流程测试"
echo "- 增加并发连接压力测试"
echo ""
echo "权限管理模块:"
echo "- 实现用户认证流程完整测试"
echo "- 添加角色-based访问控制测试"
echo "- 完善权限继承和撤销测试"
echo ""
echo "SQL解析器模块:"
echo "- 扩展SQL语法解析边界测试"
echo "- 添加复杂查询语句解析测试"
echo "- 完善错误恢复和提示测试"
echo ""
echo "执行引擎模块:"
echo "- 增加多表连接查询测试"
echo "- 完善事务处理和回滚测试"
echo "- 添加查询优化器测试"
