#!/bin/bash

# SQLCC CRUD测试覆盖率数据生成脚本
# 专门用于生成CRUD性能测试的覆盖率数据

set -e

echo "=========================================="
echo "SQLCC CRUD测试覆盖率数据生成"
echo "=========================================="
echo "开始时间: $(date)"
echo ""

# 创建覆盖率数据目录
COVERAGE_DIR="/tmp/crud_coverage_data"
mkdir -p "$COVERAGE_DIR"

echo "将为CRUD测试生成覆盖率数据"
echo ""

# 设置覆盖率环境变量
export LLVM_PROFILE_FILE="$COVERAGE_DIR/crud_coverage.profraw"

echo "----------------------------------------"
echo "执行CRUD测试并收集覆盖率数据"
echo "----------------------------------------"

# 执行CRUD测试
if ./bazel-bin/tests/crud_test; then
    echo "✅ CRUD测试执行成功"
else
    echo "❌ CRUD测试执行失败"
    exit 1
fi

echo ""
echo "----------------------------------------"
echo "生成覆盖率报告"
echo "----------------------------------------"

# 检查是否有覆盖率数据
if [ -f "$COVERAGE_DIR/crud_coverage.profraw" ]; then
    echo "发现覆盖率数据文件，正在生成报告..."

    # 合并覆盖率数据
    llvm-profdata merge "$COVERAGE_DIR/crud_coverage.profraw" -o "$COVERAGE_DIR/crud_coverage.profdata"
    echo "覆盖率数据合并完成"

    # 生成文本覆盖率报告
    echo "生成文本覆盖率报告..."
    llvm-cov show \
        --instr-profile="$COVERAGE_DIR/crud_coverage.profdata" \
        --object=bazel-bin/tests/crud_test \
        --format=text \
        --output-dir="$COVERAGE_DIR/text" \
        src/database_manager.cpp > "$COVERAGE_DIR/crud_coverage_text_report.txt"

    # 生成HTML覆盖率报告
    echo "生成HTML覆盖率报告..."
    llvm-cov show \
        --instr-profile="$COVERAGE_DIR/crud_coverage.profdata" \
        --object=bazel-bin/tests/crud_test \
        --format=html \
        --output-dir="$COVERAGE_DIR/html" \
        src/database_manager.cpp

    # 生成覆盖率摘要
    echo "生成覆盖率摘要..."
    llvm-cov report \
        --instr-profile="$COVERAGE_DIR/crud_coverage.profdata" \
        --object=bazel-bin/tests/crud_test \
        src/database_manager.cpp > "$COVERAGE_DIR/crud_coverage_summary.txt"

    echo ""
    echo "=========================================="
    echo "CRUD覆盖率报告生成完成"
    echo "=========================================="
    echo "覆盖率数据目录: $COVERAGE_DIR"
    echo "HTML报告: $COVERAGE_DIR/html/index.html"
    echo "文本摘要: $COVERAGE_DIR/crud_coverage_summary.txt"
    echo "详细文本报告: $COVERAGE_DIR/crud_coverage_text_report.txt"

    # 显示覆盖率摘要
    echo ""
    echo "=========================================="
    echo "覆盖率摘要"
    echo "=========================================="
    cat "$COVERAGE_DIR/crud_coverage_summary.txt"

else
    echo "❌ 未找到覆盖率数据文件"
    echo "可能的原因:"
    echo "  - 测试执行失败"
    echo "  - LLVM_PROFILE_FILE环境变量未正确设置"
    echo "  - Clang编译器未启用覆盖率支持"
fi

# 清理环境变量
unset LLVM_PROFILE_FILE

echo ""
echo "=========================================="
echo "执行完成"
echo "=========================================="
echo "结束时间: $(date)"

exit 0
