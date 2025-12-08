#!/bin/bash
# 测试文件重新分类脚本

echo "=== 开始重新分类测试文件 ==="

# 1. 移动覆盖率测试文件
echo "1. 移动覆盖率测试文件..."
find . -name "*coverage*test*.cpp" -exec mv {} coverage/unit/ \;

# 2. 移动单元测试文件
echo "2. 移动单元测试文件..."
find unit/ -name "*test*.cpp" -exec mv {} unit/basic/ \;

# 3. 移动性能测试文件
echo "3. 移动性能测试文件..."
find performance/ -name "*performance*.cc" -exec mv {} performance/basic/ \;

echo "=== 文件分类完成 ==="
