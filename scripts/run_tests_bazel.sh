#!/bin/bash

# SQLCC Bazel集成测试脚本
# 功能：使用Bazel构建系统运行所有测试

echo "====================================="
echo "SQLCC Bazel集成测试脚本"
echo "====================================="

# 定义颜色输出
GREEN="\033[0;32m"
YELLOW="\033[1;33m"
RED="\033[0;31m"
NC="\033[0m" # No Color
BLUE="\033[0;34m"

# 解析命令行参数
ENABLE_COVERAGE=false
RUN_ALL_TESTS=false
GENERATE_REPORT=false
PARALLEL_TESTS=false

for arg in "$@"; do
    case "$arg" in
        --coverage)
            ENABLE_COVERAGE=true
            echo -e "${YELLOW}代码覆盖率测试已启用${NC}"
            ;;
        --all)
            RUN_ALL_TESTS=true
            echo -e "${YELLOW}将运行所有测试${NC}"
            ;;
        --report)
            GENERATE_REPORT=true
            echo -e "${YELLOW}将生成测试报告${NC}"
            ;;
        --parallel)
            PARALLEL_TESTS=true
            echo -e "${YELLOW}并行测试执行已启用${NC}"
            ;;
        --help)
            echo "用法: $0 [选项]"
            echo "选项:"
            echo "  --coverage   启用代码覆盖率测试"
            echo "  --all        运行所有测试套件"
            echo "  --report     生成测试报告"
            echo "  --parallel   启用并行测试执行"
            echo "  --help       显示帮助信息"
            exit 0
            ;;
    esac
done

# 保存原始目录
ORIGINAL_DIR=$(pwd)

# 确保在项目根目录
if [ ! -f "WORKSPACE" ] || [ ! -f "BUILD.bazel" ]; then
    echo -e "${RED}错误：请在SQLCC项目根目录运行此脚本${NC}"
    exit 1
fi

echo -e "${BLUE}项目根目录: $(pwd)${NC}"

# 设置Bazel选项
BAZEL_OPTS=""
if [ "$PARALLEL_TESTS" = true ]; then
    BAZEL_OPTS="$BAZEL_OPTS --jobs=auto"
fi

if [ "$ENABLE_COVERAGE" = true ]; then
    BAZEL_OPTS="$BAZEL_OPTS --combined_report=lcov"
fi

# 运行测试函数
run_test_suite() {
    local suite_name=$1
    local target=$2
    
    echo -e "${BLUE}========================================${NC}"
    echo -e "${BLUE}运行测试套件: $suite_name${NC}"
    echo -e "${BLUE}目标: $target${NC}"
    echo -e "${BLUE}========================================${NC}"
    
    if [ "$ENABLE_COVERAGE" = true ]; then
        bazel coverage $BAZEL_OPTS $target
    else
        bazel test $BAZEL_OPTS $target
    fi
    
    local result=$?
    if [ $result -eq 0 ]; then
        echo -e "${GREEN}✓ $suite_name 测试通过${NC}"
    else
        echo -e "${RED}✗ $suite_name 测试失败${NC}"
    fi
    
    return $result
}

# 主要测试执行
echo -e "${BLUE}开始执行测试...${NC}"

TESTS_PASSED=0
TESTS_FAILED=0

# 运行核心测试
if run_test_suite "核心组件测试" "//:core_tests"; then
    ((TESTS_PASSED++))
else
    ((TESTS_FAILED++))
fi

# 运行解析器测试
if run_test_suite "解析器组件测试" "//:parser_tests"; then
    ((TESTS_PASSED++))
else
    ((TESTS_FAILED++))
fi

# 运行执行器测试
if run_test_suite "执行器组件测试" "//:executor_tests"; then
    ((TESTS_PASSED++))
else
    ((TESTS_FAILED++))
fi

# 运行存储测试
if run_test_suite "存储组件测试" "//:storage_tests"; then
    ((TESTS_PASSED++))
else
    ((TESTS_FAILED++))
fi

# 运行网络测试
if run_test_suite "网络组件测试" "//:network_tests"; then
    ((TESTS_PASSED++))
else
    ((TESTS_FAILED++))
fi

# 运行安全测试
if run_test_suite "安全组件测试" "//:security_tests"; then
    ((TESTS_PASSED++))
else
    ((TESTS_FAILED++))
fi

# 运行事务测试
if run_test_suite "事务组件测试" "//:transaction_tests"; then
    ((TESTS_PASSED++))
else
    ((TESTS_FAILED++))
fi

# 如果需要运行所有测试
if [ "$RUN_ALL_TESTS" = true ]; then
    # 运行调试测试
    if run_test_suite "调试组件测试" "//:debug_tests"; then
        ((TESTS_PASSED++))
    else
        ((TESTS_FAILED++))
    fi
    
    # 运行所有测试目标
    echo -e "${BLUE}========================================${NC}"
    echo -e "${BLUE}运行所有测试目标${NC}"
    echo -e "${BLUE}========================================${NC}"
    
    if [ "$ENABLE_COVERAGE" = true ]; then
        bazel coverage $BAZEL_OPTS //...
    else
        bazel test $BAZEL_OPTS //...
    fi
    
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}✓ 所有测试通过${NC}"
    else
        echo -e "${RED}✗ 部分测试失败${NC}"
    fi
fi

# 生成报告
if [ "$GENERATE_REPORT" = true ]; then
    echo -e "${BLUE}========================================${NC}"
    echo -e "${BLUE}生成测试报告${NC}"
    echo -e "${BLUE}========================================${NC}"
    
    # 创建报告目录
    mkdir -p test_reports
    
    # 生成测试总结
    cat > test_reports/test_summary.txt << EOF
SQLCC Bazel测试报告
生成时间: $(date)

测试统计:
- 通过的测试: $TESTS_PASSED
- 失败的测试: $TESTS_FAILED
- 总测试数: $((TESTS_PASSED + TESTS_FAILED))

测试配置:
- 覆盖率测试: $ENABLE_COVERAGE
- 并行执行: $PARALLEL_TESTS
- 运行所有测试: $RUN_ALL_TESTS

EOF
    
    echo -e "${GREEN}测试报告已生成: test_reports/test_summary.txt${NC}"
fi

# 显示总结
echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}测试执行完成${NC}"
echo -e "${BLUE}========================================${NC}"
echo -e "${GREEN}通过的测试: $TESTS_PASSED${NC}"
echo -e "${RED}失败的测试: $TESTS_FAILED${NC}"
echo -e "${BLUE}总测试数: $((TESTS_PASSED + TESTS_FAILED))${NC}"

# 返回结果
if [ $TESTS_FAILED -eq 0 ]; then
    echo -e "${GREEN}所有测试通过！${NC}"
    exit 0
else
    echo -e "${RED}部分测试失败${NC}"
    exit 1
fi