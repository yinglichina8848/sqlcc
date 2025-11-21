#!/bin/bash

# SQLCC 综合测试运行脚本
# 支持分类执行：unit, coverage, performance

set -e

# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 帮助信息
show_help() {
    echo "用法: $0 [选项] [测试分类]"
    echo ""
    echo "测试分类:"
    echo "  unit        - 运行单元测试"
    echo "  coverage    - 运行覆盖率测试"
    echo "  performance - 运行性能测试"
    echo "  all         - 运行所有测试分类"
    echo ""
    echo "选项:"
    echo "  -h, --help  - 显示帮助信息"
    echo "  -v, --verbose - 详细输出"
    echo "  -j, --jobs N  - 并行编译任务数 (默认: auto)"
    echo ""
    echo "示例:"
    echo "  $0 unit                    # 运行单元测试"
    echo "  $0 coverage -v             # 运行覆盖率测试，详细输出"
    echo "  $0 performance -j 4        # 运行性能测试，4个并行任务"
    echo "  $0 all                     # 运行所有测试"
}

# 默认参数
VERBOSE=0
JOBS=""
TEST_TYPE=""

# 解析参数
while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            show_help
            exit 0
            ;;
        -v|--verbose)
            VERBOSE=1
            shift
            ;;
        -j|--jobs)
            JOBS="--parallel $2"
            shift 2
            ;;
        unit|coverage|performance|all)
            TEST_TYPE=$1
            shift
            ;;
        *)
            echo -e "${RED}错误: 未知参数 $1${NC}"
            show_help
            exit 1
            ;;
    esac
done

# 检查测试类型
if [[ -z "$TEST_TYPE" ]]; then
    echo -e "${RED}错误: 请指定测试分类${NC}"
    show_help
    exit 1
fi

# 检查构建目录
BUILD_DIR="/home/liying/sqlcc/build"
if [[ ! -d "$BUILD_DIR" ]]; then
    echo -e "${BLUE}创建构建目录...${NC}"
    mkdir -p "$BUILD_DIR"
fi

# 进入构建目录
cd "$BUILD_DIR"

# 配置CMake
echo -e "${BLUE}配置构建系统...${NC}"
if [[ $VERBOSE -eq 1 ]]; then
    cmake .. -DCMAKE_BUILD_TYPE=Debug
else
    cmake .. -DCMAKE_BUILD_TYPE=Debug > /dev/null 2>&1
fi

# 运行测试的函数
run_unit_tests() {
    echo -e "${GREEN}=== 运行单元测试 ===${NC}"
    if [[ $VERBOSE -eq 1 ]]; then
        make unit_tests $JOBS
    else
        make unit_tests $JOBS > /dev/null 2>&1
    fi
    echo -e "${GREEN}✓ 单元测试完成${NC}"
}

run_coverage_tests() {
    echo -e "${GREEN}=== 运行覆盖率测试 ===${NC}"
    if [[ $VERBOSE -eq 1 ]]; then
        make coverage $JOBS
    else
        make coverage $JOBS > /dev/null 2>&1
    fi
    echo -e "${GREEN}✓ 覆盖率测试完成${NC}"
    echo -e "${YELLOW}覆盖率报告已生成到 build/ 目录${NC}"
}

run_performance_tests() {
    echo -e "${GREEN}=== 运行性能测试 ===${NC}"
    if [[ $VERBOSE -eq 1 ]]; then
        make performance_core $JOBS
    else
        make performance_core $JOBS > /dev/null 2>&1
    fi
    echo -e "${GREEN}✓ 性能测试完成${NC}"
    echo -e "${YELLOW}性能测试结果已保存到 build/performance_results/ 目录${NC}"
}

# 根据测试类型执行
case $TEST_TYPE in
    unit)
        run_unit_tests
        ;;
    coverage)
        run_coverage_tests
        ;;
    performance)
        run_performance_tests
        ;;
    all)
        run_unit_tests
        echo ""
        run_coverage_tests
        echo ""
        run_performance_tests
        echo ""
        echo -e "${GREEN}=== 所有测试分类已完成 ===${NC}"
        ;;
esac

echo -e "${BLUE}测试运行完成！${NC}"

# 显示结果摘要
echo ""
echo -e "${BLUE}=== 测试摘要 ===${NC}"
echo -e "测试分类: ${GREEN}$TEST_TYPE${NC}"
echo -e "构建目录: ${YELLOW}$BUILD_DIR${NC}"
echo -e "详细模式: $([[ $VERBOSE -eq 1 ]] && echo "${GREEN}开启${NC}" || echo "${RED}关闭${NC}")"
if [[ -n "$JOBS" ]]; then
    echo -e "并行任务: ${YELLOW}$JOBS${NC}"
fi

# 显示后续操作建议
echo ""
echo -e "${BLUE}后续操作建议:${NC}"
case $TEST_TYPE in
    coverage)
        echo "- 查看覆盖率报告: find $BUILD_DIR -name '*.gcov' -type f"
        echo "- 生成HTML报告: gcovr -r /home/liying/sqlcc --html-details coverage.html"
        ;;
    performance)
        echo "- 查看性能结果: ls -la $BUILD_DIR/performance_results/"
        echo "- 分析性能日志: tail -f $BUILD_DIR/performance_results/*.log"
        ;;
    unit)
        echo "- 重新运行特定测试: make run_<test_name>_test"
        echo "- 查看测试详细输出: ctest -V"
        ;;
    all)
        echo "- 查看所有测试结果: find $BUILD_DIR -name '*.log' -o -name '*.gcov' -o -name '*.html'"
        echo "- 清理构建文件: make clean"
        ;;
esac

exit 0