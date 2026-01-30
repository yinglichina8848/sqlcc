#!/bin/bash

# SQLCC Level 1-6 测试编译脚本
# 编译所有层次的测试用例

set -e

echo "=== SQLCC Level 1-6 测试编译脚本 ==="
echo "开始时间: $(date)"
echo

# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# 日志函数
log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# 编译结果统计
total_compiled=0
successful_compiles=0
failed_compiles=0

# 编译函数
compile_test() {
    local test_target="$1"
    local test_name="$2"

    log_info "编译测试: $test_name ($test_target)"

    if bazel build "$test_target" 2>/dev/null; then
        log_success "✓ $test_name 编译成功"
        ((successful_compiles++))
        return 0
    else
        log_error "✗ $test_name 编译失败"
        ((failed_compiles++))
        return 1
    fi
}

# Level 1: 基础工具类测试
echo "==========================================="
echo "Level 1: 基础工具类测试"
echo "==========================================="

level1_tests=(
    "//tests/unit/basic:token_test|Token基础功能测试"
    "//tests/unit/basic:exception_test|异常处理测试"
    "//tests/unit/basic:data_types_test|数据类型测试"
    "//tests/unit/basic:logger_basic_test|基础日志测试"
    "//tests/level1_foundation:basic_test|Level 1基础功能测试"
)

for test_info in "${level1_tests[@]}"; do
    IFS='|' read -r target name <<< "$test_info"
    compile_test "$target" "$name"
    ((total_compiled++))
done

# Level 2: 核心组件测试
echo
echo "==========================================="
echo "Level 2: 核心组件测试"
echo "==========================================="

level2_tests=(
    "//tests/unit/core:config_manager_test|配置管理器测试"
    "//tests/unit/core:database_manager_test|数据库管理器测试"
    "//tests/unit/core:execution_context_test|执行上下文测试"
    "//tests/unit/core:user_manager_test|用户管理器测试"
    "//tests/unit/core:system_database_test|系统数据库测试"
)

for test_info in "${level2_tests[@]}"; do
    IFS='|' read -r target name <<< "$test_info"
    compile_test "$target" "$name"
    ((total_compiled++))
done

# Level 3: 存储引擎测试
echo
echo "==========================================="
echo "Level 3: 存储引擎测试"
echo "==========================================="

level3_tests=(
    "//tests/storage_engine:buffer_pool_test|缓冲池测试"
    "//tests/storage_engine:disk_manager_test|磁盘管理器测试"
    "//tests/storage_engine:page_test|页面管理测试"
    "//tests/storage_engine:b_plus_tree_test|B+树索引测试"
    "//tests/storage_engine:table_storage_test|表存储测试"
)

for test_info in "${level3_tests[@]}"; do
    IFS='|' read -r target name <<< "$test_info"
    compile_test "$target" "$name"
    ((total_compiled++))
done

# Level 4: SQL解析器测试
echo
echo "==========================================="
echo "Level 4: SQL解析器测试"
echo "==========================================="

level4_tests=(
    "//tests/sql_parser:parser_test|SQL解析器测试"
    "//tests/sql_parser:lexer_test|词法分析器测试"
    "//tests/sql_parser:ast_test|抽象语法树测试"
    "//tests/sql_parser:select_parser_test|SELECT语句解析测试"
    "//tests/sql_parser:create_parser_test|CREATE语句解析测试"
)

for test_info in "${level4_tests[@]}"; do
    IFS='|' read -r target name <<< "$test_info"
    compile_test "$target" "$name"
    ((total_compiled++))
done

# Level 5: 执行引擎测试
echo
echo "==========================================="
echo "Level 5: 执行引擎测试"
echo "==========================================="

level5_tests=(
    "//tests/execution:executor_test|执行器测试"
    "//tests/execution:query_plan_test|查询计划测试"
    "//tests/execution:sql_executor_test|SQL执行器测试"
    "//tests/execution:unified_executor_test|统一执行器测试"
    "//tests/execution:transaction_test|事务执行测试"
)

for test_info in "${level5_tests[@]}"; do
    IFS='|' read -r target name <<< "$test_info"
    compile_test "$target" "$name"
    ((total_compiled++))
done

# Level 6: 高级功能测试
echo
echo "==========================================="
echo "Level 6: 高级功能测试"
echo "==========================================="

level6_tests=(
    "//tests/integration:ddl_commands_test|DDL命令集成测试"
    "//tests/integration:dml_commands_test|DML命令集成测试"
    "//tests/integration:advanced_sql_test|高级SQL测试"
    "//tests/integration:procedure_trigger_integration_test|存储过程触发器集成测试"
    "//tests/integration:set_operation_subquery_integration_test|集合操作子查询集成测试"
)

for test_info in "${level6_tests[@]}"; do
    IFS='|' read -r target name <<< "$test_info"
    compile_test "$target" "$name"
    ((total_compiled++))
done

# 生成编译报告
echo
echo "==========================================="
echo "编译结果汇总"
echo "==========================================="
echo "总编译目标: $total_compiled"
echo "编译成功: $successful_compiles"
echo "编译失败: $failed_compiles"
echo "成功率: $(awk "BEGIN {printf \"%.1f\", ${successful_compiles}/${total_compiled}*100}")%"

if [ $failed_compiles -eq 0 ]; then
    log_success "🎉 所有Level 1-6测试编译成功！"
else
    log_warning "⚠️  $failed_compiles 个测试编译失败，请检查错误信息"
fi

echo
echo "详细编译日志请查看上述输出"
echo "编译完成时间: $(date)"
echo "==========================================="

# 保存编译结果
cat > level1_to_level6_compilation_report.txt << EOF
SQLCC Level 1-6 测试编译报告
生成时间: $(date)
===========================================

编译统计:
- 总编译目标: $total_compiled
- 编译成功: $successful_compiles
- 编译失败: $failed_compiles
- 成功率: $(awk "BEGIN {printf \"%.1f\", ${successful_compiles}/${total_compiled}*100}")%

各Level编译结果:

Level 1 (基础工具类):
$(for test_info in "${level1_tests[@]}"; do
    IFS='|' read -r target name <<< "$test_info"
    echo "- $name: $(bazel build "$target" 2>/dev/null && echo "✓ 成功" || echo "✗ 失败")"
done)

Level 2 (核心组件):
$(for test_info in "${level2_tests[@]}"; do
    IFS='|' read -r target name <<< "$test_info"
    echo "- $name: $(bazel build "$target" 2>/dev/null && echo "✓ 成功" || echo "✗ 失败")"
done)

Level 3 (存储引擎):
$(for test_info in "${level3_tests[@]}"; do
    IFS='|' read -r target name <<< "$test_info"
    echo "- $name: $(bazel build "$target" 2>/dev/null && echo "✓ 成功" || echo "✗ 失败")"
done)

Level 4 (SQL解析器):
$(for test_info in "${level4_tests[@]}"; do
    IFS='|' read -r target name <<< "$test_info"
    echo "- $name: $(bazel build "$target" 2>/dev/null && echo "✓ 成功" || echo "✗ 失败")"
done)

Level 5 (执行引擎):
$(for test_info in "${level5_tests[@]}"; do
    IFS='|' read -r target name <<< "$test_info"
    echo "- $name: $(bazel build "$target" 2>/dev/null && echo "✓ 成功" || echo "✗ 失败")"
done)

Level 6 (高级功能):
$(for test_info in "${level6_tests[@]}"; do
    IFS='|' read -r target name <<< "$test_info"
    echo "- $name: $(bazel build "$target" 2>/dev/null && echo "✓ 成功" || echo "✗ 失败")"
done)

===========================================
编译完成时间: $(date)
EOF

log_success "编译报告已保存到: level1_to_level6_compilation_report.txt"

echo
echo "=== 编译脚本执行完毕 ==="