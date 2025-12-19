#!/bin/bash

# SQLCC v1.2.3 全面测试执行脚本
# 逐个编译和执行测试，记录失败和通过的情况

TEST_LOG="/home/liying/sqlcc/docs/项目进展/v1.2.3/测试执行日志.md"
RESULTS_DIR="/home/liying/sqlcc/test_results"

# 清空之前的测试结果
> "$TEST_LOG"

echo "# SQLCC v1.2.3 测试执行日志" > "$TEST_LOG"
echo "" >> "$TEST_LOG"
echo "**测试开始时间**: $(date)" >> "$TEST_LOG"
echo "**测试范围**: 基于测试源码详细整理报告的所有测试文件" >> "$TEST_LOG"
echo "**测试目标**: 编译并执行所有测试，记录通过/失败情况，为改进计划提供依据" >> "$TEST_LOG"
echo "" >> "$TEST_LOG"

# 初始化计数器
total_tests=0
passed_tests=0
failed_tests=0
compile_failed=0

echo "## 测试执行记录" >> "$TEST_LOG"
echo "" >> "$TEST_LOG"

# 获取所有测试文件列表
find /home/liying/sqlcc/tests -name "*.cpp" -o -name "*.cc" | sort > /tmp/all_tests.txt
find /home/liying/sqlcc -maxdepth 1 -name "*test*.cpp" | sort >> /tmp/all_tests.txt

# 逐个编译和执行测试
while IFS= read -r test_file; do
    ((total_tests++))
    echo "正在测试: $test_file"
    
    # 提取文件名作为测试名称
    test_name=$(basename "$test_file")
    
    echo "### $total_tests. $test_file" >> "$TEST_LOG"
    echo "- 开始时间: $(date)" >> "$TEST_LOG"
    
    # 尝试编译测试
    if bazel build "$test_file" 2>/tmp/compile_error.log; then
        echo "- 编译状态: 通过" >> "$TEST_LOG"
        
        # 尝试执行测试
        if bazel run "$test_file" 2>/tmp/run_error.log; then
            echo "- 执行状态: 通过" >> "$TEST_LOG"
            ((passed_tests++))
        else
            echo "- 执行状态: 失败" >> "$TEST_LOG"
            echo "- 执行错误:" >> "$TEST_LOG"
            echo "" >> "$TEST_LOG"
            echo "\`\`\`" >> "$TEST_LOG"
            cat /tmp/run_error.log >> "$TEST_LOG"
            echo "\`\`\`" >> "$TEST_LOG"
            ((failed_tests++))
        fi
    else
        echo "- 编译状态: 失败" >> "$TEST_LOG"
        echo "- 编译错误:" >> "$TEST_LOG"
        echo "" >> "$TEST_LOG"
        echo "\`\`\`" >> "$TEST_LOG"
        cat /tmp/compile_error.log >> "$TEST_LOG"
        echo "\`\`\`" >> "$TEST_LOG"
        ((compile_failed++))
    fi
    
    echo "- 结束时间: $(date)" >> "$TEST_LOG"
    echo "" >> "$TEST_LOG"
    
    # 每10个测试显示一次进度
    if (( total_tests % 10 == 0 )); then
        echo "已完成 $total_tests 个测试..."
    fi
done < /tmp/all_tests.txt

# 输出测试总结
echo "## 测试总结" >> "$TEST_LOG"
echo "" >> "$TEST_LOG"
echo "- 总测试数: $total_tests" >> "$TEST_LOG"
echo "- 通过测试: $passed_tests" >> "$TEST_LOG"
echo "- 失败测试: $failed_tests" >> "$TEST_LOG"
echo "- 编译失败: $compile_failed" >> "$TEST_LOG"
echo "- 通过率: $(( passed_tests * 100 / total_tests ))%" >> "$TEST_LOG"

echo "测试完成。总测试数: $total_tests, 通过: $passed_tests, 失败: $failed_tests, 编译失败: $compile_failed"
