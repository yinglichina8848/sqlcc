#!/bin/bash

# SQLCC 综合覆盖率测试脚本
# 整合所有成功的覆盖率测试方法
# 使用 Clang++ 20 和 LLVM-20 工具进行全面覆盖率分析

set -e

echo "================================================================="
echo "SQLCC 综合覆盖率测试脚本 - 整合所有成功测试方法"
echo "使用 Clang++ 20 + LLVM-20 工具链"
echo "================================================================="
echo ""

# 全局配置
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
COVERAGE_DIR="${PROJECT_ROOT}/coverage_data"
COMBINED_DIR="${COVERAGE_DIR}/combined"

# 成功运行的测试方法列表
SUCCESSFUL_METHODS=(
    "layer1_test:基础工具类测试"
    "storage_engine_test:存储引擎测试"
    "bplus_tree_test:B+树索引测试"
    "crud_operations_test:CRUD操作测试"
    "performance_test:性能测试"
)

# 工具检查
check_tools() {
    echo "🔧 检查必需工具..."
    local tools=("clang++-20" "llvm-profdata-20" "llvm-cov-20")
    for tool in "${tools[@]}"; do
        if ! command -v "$tool" &> /dev/null; then
            echo "❌ 错误: $tool 未安装"
            exit 1
        fi
    done
    echo "✅ 工具检查通过"
}

# 创建目录结构
setup_directories() {
    echo "📁 创建覆盖率数据目录..."
    mkdir -p "${COMBINED_DIR}"
    mkdir -p "${COVERAGE_DIR}/layer1"
    mkdir -p "${COVERAGE_DIR}/storage"
    mkdir -p "${COVERAGE_DIR}/bplus"
    mkdir -p "${COVERAGE_DIR}/crud"
    mkdir -p "${COVERAGE_DIR}/performance"
    echo "✅ 目录创建完成"
}

# 运行基础工具类测试 (Layer 1)
run_layer1_test() {
    echo ""
    echo "🧪 运行基础工具类测试 (Layer 1)..."

    cd "${PROJECT_ROOT}"

    # 编译
    clang++-20 -std=c++20 \
        -fprofile-instr-generate \
        -fcoverage-mapping \
        -I. -Iinclude -I/usr/include \
        -g \
        coverage_data/layer1/simple_test_runner.cpp \
        src/logger/logger.cpp \
        -lgtest -lgtest_main -lgmock -pthread \
        -o coverage_data/layer1/layer1_test

    # 运行测试
    export LLVM_PROFILE_FILE="coverage_data/layer1/layer1.profraw"
    ./coverage_data/layer1/layer1_test

    # 生成报告
    llvm-profdata-20 merge coverage_data/layer1/layer1.profraw \
        -o coverage_data/layer1/layer1.profdata

    llvm-cov-20 report coverage_data/layer1/layer1_test \
        --instr-profile=coverage_data/layer1/layer1.profdata \
        > coverage_data/layer1/layer1_report.txt

    echo "✅ Layer 1 测试完成"
}

# 运行存储引擎测试
run_storage_engine_test() {
    echo ""
    echo "🗄️  运行存储引擎测试..."

    cd "${PROJECT_ROOT}"

    # 编译存储引擎测试
    clang++-20 -std=c++20 \
        -fprofile-instr-generate \
        -fcoverage-mapping \
        -I. -Iinclude -I/usr/include \
        -g \
        tests/storage_engine/storage_engine_comprehensive_test.cpp \
        tests/storage_engine/buffer_pool_test.cpp \
        tests/storage_engine/page_allocator_test.cpp \
        -lgtest -lgtest_main -lgmock -pthread \
        -o coverage_data/storage/storage_test

    # 运行测试
    export LLVM_PROFILE_FILE="coverage_data/storage/storage.profraw"
    ./coverage_data/storage/storage_test

    # 生成报告
    llvm-profdata-20 merge coverage_data/storage/storage.profraw \
        -o coverage_data/storage/storage.profdata

    llvm-cov-20 report coverage_data/storage/storage_test \
        --instr-profile=coverage_data/storage/storage.profdata \
        > coverage_data/storage/storage_report.txt

    echo "✅ 存储引擎测试完成"
}

# 运行B+树测试
run_bplus_tree_test() {
    echo ""
    echo "🌳 运行B+树索引测试..."

    cd "${PROJECT_ROOT}"

    # 编译B+树测试
    clang++-20 -std=c++20 \
        -fprofile-instr-generate \
        -fcoverage-mapping \
        -I. -Iinclude -I/usr/include \
        -g \
        tests/storage_engine/b_plus_tree_core_test.cpp \
        tests/storage_engine/test_bplus_tree_fix.cpp \
        -lgtest -lgtest_main -lgmock -pthread \
        -o coverage_data/bplus/bplus_test

    # 运行测试
    export LLVM_PROFILE_FILE="coverage_data/bplus/bplus.profraw"
    ./coverage_data/bplus/bplus_test

    # 生成报告
    llvm-profdata-20 merge coverage_data/bplus/bplus.profraw \
        -o coverage_data/bplus/bplus.profdata

    llvm-cov-20 report coverage_data/bplus/bplus_test \
        --instr-profile=coverage_data/bplus/bplus.profdata \
        > coverage_data/bplus/bplus_report.txt

    echo "✅ B+树测试完成"
}

# 运行CRUD操作测试
run_crud_test() {
    echo ""
    echo "🔄 运行CRUD操作测试..."

    cd "${PROJECT_ROOT}"

    # 编译CRUD测试
    clang++-20 -std=c++20 \
        -fprofile-instr-generate \
        -fcoverage-mapping \
        -I. -Iinclude -I/usr/include \
        -g \
        tests/crud_test.cpp \
        -lgtest -lgtest_main -lgmock -pthread \
        -o coverage_data/crud/crud_test

    # 运行测试
    export LLVM_PROFILE_FILE="coverage_data/crud/crud.profraw"
    ./coverage_data/crud/crud_test

    # 生成报告
    llvm-profdata-20 merge coverage_data/crud/crud.profraw \
        -o coverage_data/crud/crud.profdata

    llvm-cov-20 report coverage_data/crud/crud_test \
        --instr-profile=coverage_data/crud/crud.profdata \
        > coverage_data/crud/crud_report.txt

    echo "✅ CRUD测试完成"
}

# 运行性能测试
run_performance_test() {
    echo ""
    echo "⚡ 运行性能测试..."

    cd "${PROJECT_ROOT}"

    # 编译性能测试
    clang++-20 -std=c++20 \
        -fprofile-instr-generate \
        -fcoverage-mapping \
        -I. -Iinclude -I/usr/include \
        -g \
        tests/performance/basic/performance_test_base.cc \
        tests/performance/crud/real_crud_performance_test.cpp \
        -lgtest -lgtest_main -lgmock -pthread \
        -o coverage_data/performance/perf_test

    # 运行测试
    export LLVM_PROFILE_FILE="coverage_data/performance/perf.profraw"
    ./coverage_data/performance/perf_test

    # 生成报告
    llvm-profdata-20 merge coverage_data/performance/perf.profraw \
        -o coverage_data/performance/perf.profdata

    llvm-cov-20 report coverage_data/performance/perf_test \
        --instr-profile=coverage_data/performance/perf.profdata \
        > coverage_data/performance/perf_report.txt

    echo "✅ 性能测试完成"
}

# 合并所有覆盖率数据
merge_all_coverage() {
    echo ""
    echo "🔀 合并所有覆盖率数据..."

    # 收集所有profdata文件
    local profdata_files=()
    for dir in layer1 storage bplus crud performance; do
        if [ -f "${COVERAGE_DIR}/${dir}/${dir}.profdata" ]; then
            profdata_files+=("${COVERAGE_DIR}/${dir}/${dir}.profdata")
        fi
    done

    if [ ${#profdata_files[@]} -eq 0 ]; then
        echo "⚠️  没有找到profdata文件"
        return 1
    fi

    # 合并所有profdata
    llvm-profdata-20 merge "${profdata_files[@]}" \
        -o "${COMBINED_DIR}/combined.profdata"

    echo "✅ 覆盖率数据合并完成"
}

# 生成综合报告
generate_combined_report() {
    echo ""
    echo "📊 生成综合覆盖率报告..."

    # 找到一个可用的测试可执行文件作为基准
    local test_executable=""
    for dir in layer1 storage bplus crud performance; do
        if [ -f "${COVERAGE_DIR}/${dir}/${dir}_test" ]; then
            test_executable="${COVERAGE_DIR}/${dir}/${dir}_test"
            break
        fi
    done

    if [ -z "$test_executable" ]; then
        echo "⚠️  没有找到测试可执行文件"
        return 1
    fi

    # 生成综合文本报告
    llvm-cov-20 report "$test_executable" \
        --instr-profile="${COMBINED_DIR}/combined.profdata" \
        > "${COMBINED_DIR}/combined_report.txt"

    # 生成HTML报告
    llvm-cov-20 show "$test_executable" \
        --instr-profile="${COMBINED_DIR}/combined.profdata" \
        --format=html \
        --output-dir="${COMBINED_DIR}/html"

    # 生成LCOV格式报告
    llvm-cov-20 export "$test_executable" \
        --instr-profile="${COMBINED_DIR}/combined.profdata" \
        --format=lcov \
        > "${COMBINED_DIR}/combined.lcov"

    echo "✅ 综合报告生成完成"
}

# 显示结果摘要
show_summary() {
    echo ""
    echo "================================================================="
    echo "🎯 SQLCC 综合覆盖率测试结果摘要"
    echo "================================================================="

    if [ -f "${COMBINED_DIR}/combined_report.txt" ]; then
        echo "📈 总体覆盖率统计:"
        head -10 "${COMBINED_DIR}/combined_report.txt"
        echo ""
    fi

    echo "📁 生成的报告文件:"
    echo "  - 文本报告: ${COMBINED_DIR}/combined_report.txt"
    echo "  - HTML报告: ${COMBINED_DIR}/html/index.html"
    echo "  - LCOV报告: ${COMBINED_DIR}/combined.lcov"
    echo ""

    echo "📋 测试方法执行情况:"
    echo "  ✅ 基础工具类测试 (Layer 1)"
    echo "  ✅ 存储引擎测试"
    echo "  ✅ B+树索引测试"
    echo "  ✅ CRUD操作测试"
    echo "  ✅ 性能测试"
    echo ""

    echo "🛠️  使用的工具链:"
    echo "  - 编译器: Clang++ 20"
    echo "  - 覆盖率工具: LLVM-20 (llvm-profdata-20, llvm-cov-20)"
    echo ""

    echo "================================================================="
    echo "🎉 综合覆盖率测试完成！"
    echo "================================================================="
}

# 主函数
main() {
    echo "🚀 开始SQLCC综合覆盖率测试..."
    echo "时间: $(date)"
    echo ""

    # 执行所有步骤
    check_tools
    setup_directories

    # 运行所有测试
    run_layer1_test
    run_storage_engine_test
    run_bplus_tree_test
    run_crud_test
    run_performance_test

    # 合并和生成报告
    merge_all_coverage
    generate_combined_report
    show_summary

    echo ""
    echo "时间: $(date)"
    echo "🎯 测试完成！"
}

# 如果脚本被直接执行，运行主函数
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    main "$@"
fi
