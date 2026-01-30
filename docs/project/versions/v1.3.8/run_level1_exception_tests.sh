#!/bin/bash
# Level 1 Exception 模块测试执行脚本
# 用于编译和运行 exception/ 模块的测试

set -e

echo "========================================"
echo "Level 1 Exception 模块测试"
echo "========================================"
echo ""

cd /home/liying/sqlcc

# 1. 编译测试
echo "[1/3] 编译测试..."
bazel build //tests/level1_foundation/exception:all --test_output=errors 2>&1 || {
    echo "编译失败，请检查依赖配置"
    exit 1
}

# 2. 运行测试
echo ""
echo "[2/3] 运行测试..."
bazel test //tests/level1_foundation/exception:all \
    --test_output=summary \
    --test_summary=detailed \
    2>&1 || {
    echo "部分测试失败"
}

# 3. 生成覆盖率报告（如果配置了覆盖率）
echo ""
echo "[3/3] 生成覆盖率报告..."
if bazel test //tests/level1_foundation/exception:all --config=coverage 2>/dev/null; then
    echo "覆盖率报告生成成功"
    echo "报告位置: bazel-testlogs/tests/level1_foundation/exception/"
else
    echo "覆盖率收集需要配置coverage工具链"
fi

echo ""
echo "========================================"
echo "测试执行完成"
echo "========================================"
