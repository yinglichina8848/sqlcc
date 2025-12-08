#!/bin/bash

# 生成覆盖率报告脚本
set -e

BUILD_DIR="/home/liying/sqlcc/build"
REPORTS_DIR="/home/liying/sqlcc/docs/coverage_reports"
HTML_DIR="$REPORTS_DIR/html"

echo "=== SQLCC 覆盖率报告生成器 ==="

# 创建输出目录
mkdir -p "$HTML_DIR"
mkdir -p "$REPORTS_DIR"

# 清理之前的覆盖率数据
echo "清理之前的覆盖率数据..."
find "$BUILD_DIR" -name "*.gcda" -delete 2>/dev/null || true
find "$BUILD_DIR" -name "*.gcno" -delete 2>/dev/null || true

# 重新编译并运行基本覆盖率测试
echo "重新编译覆盖率测试..."
cd "$BUILD_DIR"
make database_manager_coverage_test

echo "运行基本覆盖率测试..."
./tests/coverage/database_manager_coverage_test --gtest_brief=1 > /dev/null 2>&1

# 收集覆盖率数据
echo "收集覆盖率数据..."
cd "$BUILD_DIR"

# 检查是否有覆盖率数据
GCD_COUNT=$(find . -name "*.gcda" | wc -l)
if [ "$GCD_COUNT" -eq 0 ]; then
    echo "警告：未找到.gcda覆盖率数据文件"
    exit 1
fi

echo "找到 $GCD_COUNT 个覆盖率数据文件"

# 使用lcov生成HTML报告（忽略错误）
echo "生成HTML覆盖率报告..."
lcov --capture --directory . --output-file coverage_info.info --ignore-errors mismatch

# 过滤掉测试文件和构建文件，只保留源代码
lcov --remove coverage_info.info "*/tests/*" "*/build/*" "*.cpp.gcda" --output-file coverage_filtered.info

# 检查是否有源代码覆盖率数据
if [ ! -s "coverage_filtered.info" ]; then
    echo "警告：没有源代码覆盖率数据，尝试从所有数据生成报告..."
    lcov --capture --directory . --output-file coverage_all.info --ignore-errors mismatch
    genhtml coverage_all.info --output-directory "$HTML_DIR" --ignore-errors mismatch
else
    # 生成HTML报告
    genhtml coverage_filtered.info --output-directory "$HTML_DIR" --ignore-errors mismatch
fi

# 复制报告到文档目录
cp -r "$HTML_DIR"/* "$REPORTS_DIR/" 2>/dev/null || true

# 生成文本格式的覆盖率摘要
echo "生成覆盖率摘要..."
if [ -f "coverage_filtered.info" ]; then
    lcov --summary coverage_filtered.info > "$REPORTS_DIR/coverage_summary.txt" 2>/dev/null || true
else
    lcov --summary coverage_all.info > "$REPORTS_DIR/coverage_summary.txt" 2>/dev/null || true
fi

echo "=== 覆盖率报告生成完成 ==="
echo "HTML报告位置：$HTML_DIR"
echo "摘要报告：$REPORTS_DIR/coverage_summary.txt"

# 显示覆盖率摘要
if [ -f "$REPORTS_DIR/coverage_summary.txt" ]; then
    echo "=== 覆盖率摘要 ==="
    cat "$REPORTS_DIR/coverage_summary.txt"
fi