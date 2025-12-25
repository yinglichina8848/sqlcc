#!/bin/bash
# 测试编译检查脚本 - 识别可以编译和运行的测试

echo "=== 开始测试编译检查 ==="
echo "检查日期: $(date)"

# 输出文件
COMPILE_LOG="compile_check_results.log"
WORKING_TESTS="working_tests.list"
FAILED_TESTS="failed_tests.list"

echo "编译检查结果 - $(date)" > "$COMPILE_LOG"
echo "" >> "$COMPILE_LOG"

> "$WORKING_TESTS"
> "$FAILED_TESTS"

# 获取所有测试目标（只获取前几个主要测试，避免过多输出）
echo "获取测试目标列表..."
ALL_TESTS=$(bazel query 'kind(cc_test, //tests/...)' 2>/dev/null | head -10)

if [ -z "$ALL_TESTS" ]; then
    echo "无法获取测试目标列表，尝试其他方法..."
    # 手动指定一些已知的测试目标
    ALL_TESTS="
//tests/unit/basic:logger_basic_test
//tests/unit/executor:task_executor_test
//tests/unit/storage:buffer_pool_smart_pointer_test
"
fi

echo "发现测试目标:"
echo "$ALL_TESTS"

TOTAL_TESTS=$(echo "$ALL_TESTS" | wc -l)
CURRENT_TEST=0

echo "开始逐个编译测试..."
while IFS= read -r test_target; do
    # 跳过空行
    [ -z "$test_target" ] && continue
    
    CURRENT_TEST=$((CURRENT_TEST + 1))
    echo "[$CURRENT_TEST] 检查测试: $test_target"
    
    # 清理之前的构建产物
    bazel clean --expunge >/dev/null 2>&1
    
    # 尝试编译测试
    if timeout 60 bazel build "$test_target" --test_output=errors >/dev/null 2>&1; then
        echo "✅ $test_target - 编译成功"
        echo "$test_target" >> "$WORKING_TESTS"
        echo "✅ $test_target - 编译成功" >> "$COMPILE_LOG"
    else
        echo "❌ $test_target - 编译失败"
        echo "$test_target" >> "$FAILED_TESTS"
        echo "❌ $test_target - 编译失败" >> "$COMPILE_LOG"
    fi
done <<< "$ALL_TESTS"

# 统计结果
WORKING_COUNT=$(wc -l < "$WORKING_TESTS")
FAILED_COUNT=$(wc -l < "$FAILED_TESTS")

echo "" >> "$COMPILE_LOG"
echo "=== 编译检查统计 ===" >> "$COMPILE_LOG"
echo "总测试数: $TOTAL_TESTS" >> "$COMPILE_LOG"
echo "编译成功: $WORKING_COUNT" >> "$COMPILE_LOG"
echo "编译失败: $FAILED_COUNT" >> "$COMPILE_LOG"

echo ""
echo "=== 编译检查统计 ==="
echo "总测试数: $TOTAL_TESTS"
echo "编译成功: $WORKING_COUNT"
echo "编译失败: $FAILED_COUNT"

if [ $WORKING_COUNT -gt 0 ]; then
    echo ""
    echo "可工作的测试列表已保存到: $WORKING_TESTS"
    echo "详细日志已保存到: $COMPILE_LOG"
else
    echo ""
    echo "没有找到可以编译的测试"
fi

echo "编译检查完成"
