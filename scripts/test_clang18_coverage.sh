#!/bin/bash

# SQLCC Clang 18 覆盖率测试脚本
# 使用 Clang++ 18 和 LLVM 覆盖率工具进行测试

set -e

echo "=== SQLCC Clang 18 覆盖率测试脚本 ==="

# 检查工具是否存在
check_tools() {
    echo "检查必需工具..."

    if ! command -v clang++-18 &> /dev/null; then
        echo "错误: clang++-18 未安装"
        exit 1
    fi

    if ! command -v llvm-profdata-18 &> /dev/null; then
        echo "错误: llvm-profdata-18 未安装"
        exit 1
    fi

    if ! command -v llvm-cov-18 &> /dev/null; then
        echo "错误: llvm-cov-18 未安装"
        exit 1
    fi

    echo "工具检查通过"
}

# 编译测试程序
compile_test() {
    echo "编译测试程序..."

    # 创建输出目录
    mkdir -p coverage_data/layer1

    # 编译选项：C++20 + 覆盖率，使用系统googletest
    clang++-18 -std=c++20 \
        -fprofile-instr-generate \
        -fcoverage-mapping \
        -I. \
        -Iinclude \
        -I/usr/include \
        -g \
        coverage_data/layer1/simple_test_runner.cpp \
        src/logger/logger.cpp \
        -lgtest \
        -lgtest_main \
        -lgmock \
        -pthread \
        -o coverage_data/layer1/layer1_test

    echo "编译完成"
}

# 运行测试并收集覆盖率
run_test() {
    echo "运行测试并收集覆盖率..."

    # 设置环境变量
    export LLVM_PROFILE_FILE="coverage_data/layer1/coverage.profraw"

    # 运行测试（从项目根目录）
    ./coverage_data/layer1/layer1_test

    echo "测试执行完成"
}

# 生成覆盖率报告
generate_report() {
    echo "生成覆盖率报告..."

    # 合并profile数据（在项目根目录）
    llvm-profdata-18 merge coverage_data/layer1/coverage.profraw -o coverage_data/layer1/coverage.profdata

    # 生成文本报告
    echo "生成文本覆盖率报告..."
    llvm-cov-18 report \
        coverage_data/layer1/layer1_test \
        --instr-profile=coverage_data/layer1/coverage.profdata \
        > coverage_data/layer1/coverage_report.txt

    # 生成详细的HTML报告
    echo "生成HTML覆盖率报告..."
    llvm-cov-18 show \
        coverage_data/layer1/layer1_test \
        --instr-profile=coverage_data/layer1/coverage.profdata \
        --format=html \
        --output-dir=coverage_data/layer1/coverage_html

    # 生成LCOV格式报告
    echo "生成LCOV格式报告..."
    llvm-cov-18 export \
        coverage_data/layer1/layer1_test \
        --instr-profile=coverage_data/layer1/coverage.profdata \
        --format=lcov \
        > coverage_data/layer1/coverage.lcov

    echo "报告生成完成"
}

# 显示结果
show_results() {
    echo "=== 覆盖率测试结果 ==="

    if [ -f "coverage_data/layer1/coverage_report.txt" ]; then
        echo "覆盖率摘要:"
        cat coverage_data/layer1/coverage_report.txt | head -20
        echo ""
    fi

    if [ -f "coverage_data/layer1/coverage.lcov" ]; then
        echo "LCOV文件已生成: coverage_data/layer1/coverage.lcov"
    fi

    if [ -d "coverage_data/layer1/coverage_html" ]; then
        echo "HTML报告已生成: coverage_data/layer1/coverage_html/index.html"
    fi

    echo "覆盖率测试完成！"
}

# 主函数
main() {
    check_tools
    compile_test
    run_test
    generate_report
    show_results
}

# 如果脚本被直接执行，运行主函数
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    main "$@"
fi
