#!/bin/bash

# SQLCC v1.3.4 快速覆盖率测试脚本
# 专门用于测试 Phase 3 事务处理增强功能

set -e

echo "================================================================="
echo "SQLCC v1.3.4 快速覆盖率测试 - Phase 3 事务处理增强验证"
echo "================================================================="
echo "开始时间: $(date)"
echo ""

# 配置参数
COVERAGE_DIR="/tmp/coverage_phase3_$(date +%Y%m%d_%H%M%S)"
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
COVERAGE_REPORT_DIR="${PROJECT_ROOT}/coverage_phase3_report"

# 创建目录
mkdir -p "$COVERAGE_DIR"
mkdir -p "$COVERAGE_REPORT_DIR"

echo "覆盖率数据目录: $COVERAGE_DIR"
echo "覆盖率报告目录: $COVERAGE_REPORT_DIR"
echo ""

# 检查工具
check_tools() {
    echo "检查必需工具..."

    if ! command -v bazel &> /dev/null; then
        echo "❌ Bazel 未安装"
        exit 1
    fi
    echo "✅ Bazel 验证通过"

    if ! command -v llvm-cov-18 &> /dev/null; then
        echo "❌ llvm-cov-18 未安装"
        exit 1
    fi
    echo "✅ LLVM 覆盖率工具验证通过"

    if ! command -v llvm-profdata-18 &> /dev/null; then
        echo "❌ llvm-profdata-18 未安装"
        exit 1
    fi
    echo "✅ LLVM profdata 工具验证通过"

    echo ""
}

# 编译事务处理增强测试（带覆盖率）
build_with_coverage() {
    echo "=========================================="
    echo "编译事务处理增强测试（带覆盖率）"
    echo "=========================================="

    cd "$PROJECT_ROOT"

    # 设置覆盖率编译选项
    export BAZEL_CXXOPTS="-fprofile-instr-generate -fcoverage-mapping"
    export BAZEL_LDFLAGS="-fprofile-instr-generate"

    echo "编译事务处理增强测试..."
    if ! bazel build //:test_transaction_enhancements --jobs=2; then
        echo "❌ 编译失败"
        exit 1
    fi

    echo "✅ 编译成功"
    echo ""
}

# 执行事务处理增强测试
run_tests() {
    echo "=========================================="
    echo "执行事务处理增强测试"
    echo "=========================================="

    cd "$PROJECT_ROOT"

    # 设置覆盖率环境变量
    export LLVM_PROFILE_FILE="$COVERAGE_DIR/test_%p.profraw"

    echo "运行测试..."
    if bazel test //:test_transaction_enhancements \
        --test_output=summary \
        --test_timeout=120 \
        --jobs=1; then
        echo "✅ 测试通过"
    else
        echo "❌ 测试失败"
        exit 1
    fi

    echo ""
}

# 生成覆盖率报告
generate_reports() {
    echo "=========================================="
    echo "生成覆盖率报告"
    echo "=========================================="

    # 查找覆盖率文件
    COVERAGE_FILES=("$COVERAGE_DIR"/*.profraw)

    if [ ${#COVERAGE_FILES[@]} -eq 0 ]; then
        echo "⚠️  没有找到覆盖率数据文件"
        return 1
    fi

    echo "找到 ${#COVERAGE_FILES[@]} 个覆盖率数据文件"

    # 合并覆盖率数据
    echo "合并覆盖率数据..."
    llvm-profdata-18 merge "${COVERAGE_FILES[@]}" -o "$COVERAGE_DIR/merged.profdata"

    # 生成文本报告 - 重点关注事务处理相关代码
    echo "生成文本覆盖率报告..."
    llvm-cov-18 report \
        --instr-profile="$COVERAGE_DIR/merged.profdata" \
        --ignore-filename-regex=".*test.*" \
        --ignore-filename-regex=".*Test.*" \
        src/transaction_manager/ src/transaction/ src/storage_engine/ include/transaction_manager.h include/transaction/ include/storage/ > "$COVERAGE_REPORT_DIR/coverage_report.txt"

    # 生成HTML报告
    echo "生成HTML覆盖率报告..."
    llvm-cov-18 show \
        --instr-profile="$COVERAGE_DIR/merged.profdata" \
        --ignore-filename-regex=".*test.*" \
        --ignore-filename-regex=".*Test.*" \
        --format=html \
        --output-dir="$COVERAGE_REPORT_DIR/html" \
        src/transaction_manager/ src/transaction/ src/storage_engine/ include/transaction_manager.h include/transaction/ include/storage/

    echo "✅ 覆盖率报告生成完成"
    echo ""
}

# 显示结果
show_results() {
    echo "=========================================="
    echo "测试结果摘要"
    echo "=========================================="

    if [ -f "$COVERAGE_REPORT_DIR/coverage_report.txt" ]; then
        echo "覆盖率报告摘要:"
        echo "----------------------------------------"
        head -20 "$COVERAGE_REPORT_DIR/coverage_report.txt"
        echo ""
    fi

    echo "报告文件位置:"
    echo "- 覆盖率数据: $COVERAGE_DIR"
    echo "- 文本报告: $COVERAGE_REPORT_DIR/coverage_report.txt"
    echo "- HTML报告: $COVERAGE_REPORT_DIR/html/index.html"
    echo ""

    # 检查事务管理器覆盖率
    if grep -q "transaction_manager" "$COVERAGE_REPORT_DIR/coverage_report.txt"; then
        echo "✅ 事务管理器覆盖率数据已生成"
    else
        echo "⚠️  未找到事务管理器覆盖率数据"
    fi

    echo ""
}

# 主函数
main() {
    check_tools
    build_with_coverage
    run_tests
    generate_reports
    show_results

    # 清理环境变量
    unset LLVM_PROFILE_FILE
    unset BAZEL_CXXOPTS
    unset BAZEL_LDFLAGS

    echo "================================================================="
    echo "快速覆盖率测试完成"
    echo "================================================================="
    echo "执行完成时间: $(date)"
    echo "================================================================="

    echo "🎉 Phase 3 事务处理增强功能验证完成！"
}

# 执行主函数
main "$@"