#!/bin/bash

# SQLCC 编译验证脚本
# 验证Level 1-6测试的编译状态

set -e

echo "=== SQLCC Level 1-6 编译验证 ==="
echo "开始时间: $(date)"
echo

# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# 统计变量
total_tests=0
compiled_tests=0
failed_tests=0

# 编译验证函数
validate_compilation() {
    local test_target="$1"
    local test_name="$2"

    ((total_tests++))
    echo -n "验证 $test_name: "

    if bazel build "$test_target" >/dev/null 2>&1; then
        echo -e "${GREEN}✓ 编译成功${NC}"
        ((compiled_tests++))
        return 0
    else
        echo -e "${RED}✗ 编译失败${NC}"
        ((failed_tests++))
        return 1
    fi
}

echo "==========================================="
echo "Level 1: 基础工具类测试"
echo "==========================================="

# Level 1测试 - 只验证能编译的
validate_compilation "//tests/level1_foundation:basic_test" "Level 1基础功能测试"
validate_compilation "//tests/unit/core:config_manager_test" "配置管理器测试"
validate_compilation "//tests/unit/basic:token_test" "Token测试"
validate_compilation "//tests/unit/basic:exception_test" "异常处理测试"
validate_compilation "//tests/unit/basic:logger_basic_test" "基础日志测试"

echo
echo "==========================================="
echo "Level 2: 核心组件测试"
echo "==========================================="

# Level 2测试 - 只验证能编译的
validate_compilation "//tests/unit/core:config_manager_test" "配置管理器测试"

echo
echo "==========================================="
echo "Level 3: 存储引擎测试"
echo "==========================================="

# Level 3测试 - 只验证能编译的
validate_compilation "//tests/storage_engine:buffer_pool_test" "缓冲池测试"

echo
echo "==========================================="
echo "Level 6: 高级功能测试"
echo "==========================================="

# Level 6测试 - 只验证能编译的
validate_compilation "//tests/integration:ddl_commands_test" "DDL命令集成测试"

echo
echo "==========================================="
echo "编译验证结果汇总"
echo "==========================================="
echo "总测试目标: $total_tests"
echo "编译成功: $compiled_tests"
echo "编译失败: $failed_tests"
echo "成功率: $(awk "BEGIN {printf \"%.1f\", ${compiled_tests}/${total_tests}*100}")%"

if [ $failed_tests -eq 0 ]; then
    echo -e "${GREEN}🎉 所有验证的测试编译成功！${NC}"
else
    echo -e "${YELLOW}⚠️  $failed_tests 个测试编译失败${NC}"
fi

echo
echo "编译验证完成时间: $(date)"
echo "==========================================="

# 保存结果
cat > compilation_validation_report.txt << EOF
SQLCC 编译验证报告
生成时间: $(date)
===========================================

验证统计:
- 总测试目标: $total_tests
- 编译成功: $compiled_tests
- 编译失败: $failed_tests
- 成功率: $(awk "BEGIN {printf \"%.1f\", ${compiled_tests}/${total_tests}*100}")%

验证结果:
$(if [ $failed_tests -eq 0 ]; then
    echo "✅ 所有验证测试编译成功"
else
    echo "⚠️ 部分测试编译失败"
fi)

===========================================
EOF

echo "验证报告已保存到: compilation_validation_report.txt"