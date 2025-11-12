#!/bin/bash

# 清理build目录外的中间文件脚本
echo "=== 清理build目录外的中间文件 ==="

# 删除.o文件
find /home/liying/sqlcc -name "*.o" -not -path "*/build*" -delete
echo "清理.o文件完成"

# 删除.gcov文件
find /home/liying/sqlcc -name "*.gcov" -not -path "*/build*" -delete
echo "清理.gcov文件完成"

# 删除.gcno文件
find /home/liying/sqlcc -name "*.gcno" -not -path "*/build*" -delete
echo "清理.gcno文件完成"

# 删除.gcda文件
find /home/liying/sqlcc -name "*.gcda" -not -path "*/build*" -delete
echo "清理.gcda文件完成"

# 删除.csv文件
find /home/liying/sqlcc -name "*.csv" -not -path "*/build*" -delete
echo "清理.csv文件完成"

echo "=== 清理完成 ==="