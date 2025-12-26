#!/bin/bash

# SQLCC v1.2.10 完整测试覆盖率执行脚本
# 使用Clang++ 18.0 + LLVM覆盖率工具链
# 按照7层测试层次结构执行所有测试，收集覆盖率数据，生成报告

set -e  # 遇到错误立即退出

echo "================================================================="
echo "SQLCC v1.2.10 完整测试覆盖率执行脚本 (Clang++ 18.0 + LLVM)"
echo "================================================================="
echo "开始时间: $(date)"
echo ""

# 配置参数
COVERAGE_DIR="/tmp/coverage_full_$(date +%Y%m%d_%H%M%S)"
COMPILE_MODE="clang18"  # clang18 或 gcc15
GENERATE_HTML=true
GENERATE_LCOV=true
MAX_PARALLEL_JOBS=4

# 创建覆盖率数据目录
mkdir -p "$COVERAGE_DIR"
echo "覆盖率数据目录: $COVERAGE_DIR"

# 7层测试层次结构定义
declare -A TEST_LAYERS

# 第1层：基础工具类 (7个文件)
TEST_LAYERS[1]="tests/unit/basic:logger_basic_test tests/unit/basic:data_types_test"

# 第2层：存储引擎基础 (8个文件)
TEST_LAYERS[2]="tests/storage_engine:b_plus_tree_core_test tests/storage_engine:page_allocator_test tests/storage_engine:data_integrity_test tests/storage_engine:disk_manager_test tests/storage_engine:concurrency_control_test"

# 第3层：索引系统 (12个文件)
TEST_LAYERS[3]="tests/storage_engine:index_manager_test tests/storage_engine:wal_system_test tests/storage_engine:buffer_pool_test tests/storage_engine:storage_engine_boundary_test"

# 第4层：SQL解析器 (10个文件)
TEST_LAYERS[4]="tests/unit/parser:sql_parser_high_coverage_test tests/unit/parser:constraint_test tests/unit/parser:window_function_test"

# 第5层：执行引擎 (15个文件)
TEST_LAYERS[5]="tests/unit/executor:task_executor_test tests/unit/executor:task_executor_comprehensive_test tests/unit/executor:function_executor_test tests/unit/executor:join_executor_boundary_test tests/unit/executor:set_operation_boundary_test"

# 第6层：网络通信 (8个文件)
TEST_LAYERS[6]="tests/unit/network:network_connection_test tests/integration:client_server_integration_test"

# 第7层：高层功能 (集成、性能、安全测试)
TEST_LAYERS[7]="tests/integration:simple_sql_test tests/integration:sql_92_comprehensive_test tests/integration:constraint_advanced_test tests/demo:expression_test tests/validate:user_manager_test"

# 全局统计变量
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0
TOTAL_COVERAGE_FILES=()

# 检查工具函数
check_tools() {
    echo "检查必需工具..."

    if [[ "$COMPILE_MODE" == "clang18" ]]; then
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
        echo "✅ Clang++ 18.0 + LLVM工具链验证通过"
    else
        if ! command -v g++ &> /dev/null; then
            echo "错误: g++ 未安装"
            exit 1
        fi
        echo "✅ GCC工具链验证通过"
    fi

    if ! command -v bazel &> /dev/null; then
        echo "错误: bazel 未安装"
        exit 1
    fi
    echo "✅ Bazel构建工具验证通过"
    echo ""
}

# 执行单层测试函数
run_layer_tests() {
    local layer_num=$1
    local test_targets=$2
    local layer_name

    case $layer_num in
        1) layer_name="基础工具类" ;;
        2) layer_name="存储引擎基础" ;;
        3) layer_name="索引系统" ;;
        4) layer_name="SQL解析器" ;;
        5) layer_name="执行引擎" ;;
        6) layer_name="网络通信" ;;
        7) layer_name="高层功能" ;;
        *) layer_name="未知层" ;;
    esac

    echo "=========================================="
    echo "执行第${layer_num}层测试: ${layer_name}"
    echo "=========================================="
    echo "测试目标: $test_targets"
    echo ""

    local layer_passed=0
    local layer_failed=0
    local layer_coverage_files=()

    # 分割测试目标并执行
    for target in $test_targets; do
        echo "执行测试: $target"

        # 设置覆盖率环境变量
        export LLVM_PROFILE_FILE="$COVERAGE_DIR/coverage_layer${layer_num}_$(basename $target).profraw"

        # 执行测试（使用coverage模式重新编译核心组件）
        if bazel coverage "$target" --test_timeout=120 --test_output=summary --jobs=$MAX_PARALLEL_JOBS; then
            echo "✅ $target - PASSED"
            ((layer_passed++))
            ((PASSED_TESTS++))
            layer_coverage_files+=("$COVERAGE_DIR/coverage_layer${layer_num}_$(basename $target).profraw")
        else
            echo "❌ $target - FAILED"
            ((layer_failed++))
            ((FAILED_TESTS++))
        fi

        echo ""
    done

    echo "第${layer_num}层测试结果统计:"
    echo "  通过: $layer_passed"
    echo "  失败: $layer_failed"
    echo "  总计: $((layer_passed + layer_failed))"
    echo ""

    # 将覆盖率文件添加到全局列表
    TOTAL_COVERAGE_FILES+=("${layer_coverage_files[@]}")

    return $layer_failed
}

# 生成覆盖率报告函数
generate_coverage_report() {
    echo "=========================================="
    echo "生成覆盖率报告"
    echo "=========================================="

    # 检查是否有覆盖率数据
    if [ ${#TOTAL_COVERAGE_FILES[@]} -eq 0 ]; then
        echo "⚠️  没有找到覆盖率数据文件"
        return 1
    fi

    echo "找到 ${#TOTAL_COVERAGE_FILES[@]} 个覆盖率数据文件"

    # 合并覆盖率数据
    echo "合并覆盖率数据..."
    llvm-profdata-18 merge "${TOTAL_COVERAGE_FILES[@]}" -o "$COVERAGE_DIR/coverage.profdata"

    # 获取所有测试对象的列表（排除测试程序自身的覆盖率）
    OBJECT_FILES=""
    for target in "${TEST_LAYERS[1]}" "${TEST_LAYERS[2]}" "${TEST_LAYERS[3]}" "${TEST_LAYERS[4]}" "${TEST_LAYERS[5]}" "${TEST_LAYERS[6]}" "${TEST_LAYERS[7]}"; do
        # 转换Bazel目标为对象文件路径
        target_clean=$(echo "$target" | sed 's/tests\///; s/:/\//')
        OBJECT_FILES="$OBJECT_FILES --object=bazel-bin/$target_clean"
    done

    # 生成文本报告 - 只包含include/和src/目录的SQLCC项目代码
    echo "生成文本覆盖率报告 (只包含SQLCC项目代码)..."
    llvm-cov-18 report \
        --instr-profile="$COVERAGE_DIR/coverage.profdata" \
        --ignore-filename-regex=".*test.*" \
        --ignore-filename-regex=".*Test.*" \
        --ignore-filename-regex=".*_test.*" \
        --ignore-filename-regex=".*_Test.*" \
        $OBJECT_FILES \
        src/ include/ > "$COVERAGE_DIR/coverage_report.txt"

    # 生成详细HTML报告 - 只包含include/和src/目录的SQLCC项目代码
    if [[ "$GENERATE_HTML" == true ]]; then
        echo "生成HTML覆盖率报告 (只包含SQLCC项目代码)..."
        llvm-cov-18 show \
            --instr-profile="$COVERAGE_DIR/coverage.profdata" \
            --ignore-filename-regex=".*test.*" \
            --ignore-filename-regex=".*Test.*" \
            --ignore-filename-regex=".*_test.*" \
            --ignore-filename-regex=".*_Test.*" \
            --format=html \
            --output-dir="$COVERAGE_DIR/coverage_html" \
            $OBJECT_FILES \
            src/ include/
    fi

    # 生成LCOV格式报告 - 只包含include/和src/目录的SQLCC项目代码
    if [[ "$GENERATE_LCOV" == true ]]; then
        echo "生成LCOV格式报告 (只包含SQLCC项目代码)..."
        llvm-cov-18 export \
            --instr-profile="$COVERAGE_DIR/coverage.profdata" \
            --ignore-filename-regex=".*test.*" \
            --ignore-filename-regex=".*Test.*" \
            --ignore-filename-regex=".*_test.*" \
            --ignore-filename-regex=".*_Test.*" \
            --format=lcov \
            $OBJECT_FILES \
            src/ include/ > "$COVERAGE_DIR/coverage.lcov"
    fi

    echo "覆盖率报告生成完成 (已过滤测试程序代码)"
    echo ""
}

# 显示结果函数
show_results() {
    echo "================================================================="
    echo "完整测试执行结果统计"
    echo "================================================================="

    TOTAL_TESTS=$((PASSED_TESTS + FAILED_TESTS))

    echo "总体统计:"
    echo "  总测试数: $TOTAL_TESTS"
    echo "  通过测试: $PASSED_TESTS"
    echo "  失败测试: $FAILED_TESTS"
    echo "  通过率: $((PASSED_TESTS * 100 / TOTAL_TESTS))%"
    echo ""

    # 显示覆盖率摘要
    if [ -f "$COVERAGE_DIR/coverage_report.txt" ]; then
        echo "覆盖率摘要:"
        echo "----------------------------------------"
        tail -20 "$COVERAGE_DIR/coverage_report.txt"
        echo ""
    fi

    # 显示报告位置
    echo "报告文件位置:"
    echo "----------------------------------------"
    echo "覆盖率数据目录: $COVERAGE_DIR"
    echo "文本报告: $COVERAGE_DIR/coverage_report.txt"

    if [[ "$GENERATE_HTML" == true ]] && [ -d "$COVERAGE_DIR/coverage_html" ]; then
        echo "HTML报告: $COVERAGE_DIR/coverage_html/index.html"
    fi

    if [[ "$GENERATE_LCOV" == true ]] && [ -f "$COVERAGE_DIR/coverage.lcov" ]; then
        echo "LCOV报告: $COVERAGE_DIR/coverage.lcov"
    fi

    echo ""
    echo "================================================================="
    echo "执行完成时间: $(date)"
    echo "================================================================="

    # 清理环境变量
    unset LLVM_PROFILE_FILE
}

# 主函数
main() {
    check_tools

    local failed_layers=0

    # 按层执行测试
    for layer in {1..7}; do
        if [[ -n "${TEST_LAYERS[$layer]}" ]]; then
            if ! run_layer_tests $layer "${TEST_LAYERS[$layer]}"; then
                ((failed_layers++))
            fi
        fi
    done

    # 生成覆盖率报告
    generate_coverage_report

    # 显示最终结果
    show_results

    echo ""
    if [ $failed_layers -gt 0 ]; then
        echo "⚠️  有 $failed_layers 层测试出现失败，请检查上述输出"
        exit 1
    else
        echo "🎉 所有测试层执行完成！"
        exit 0
    fi
}

# 如果脚本被直接执行，运行主函数
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    main "$@"
fi
