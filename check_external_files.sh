#!/bin/bash

# 检查build目录外的中间文件脚本
echo "=== 检查build目录外的中间文件 ==="
echo "正在检查以下类型的文件："

# 检查.o文件
o_count=$(find /home/liying/sqlcc -name "*.o" -not -path "*/build*" | wc -l)
echo "  - .o文件: 找到 $o_count 个 .o 文件在build目录外"

# 检查.gcov文件
gcov_count=$(find /home/liying/sqlcc -name "*.gcov" -not -path "*/build*" | wc -l)
echo "  - .gcov文件: 找到 $gcov_count 个 .gcov 文件在build目录外"

# 检查.gcno文件
gcno_count=$(find /home/liying/sqlcc -name "*.gcno" -not -path "*/build*" | wc -l)
echo "  - .gcno文件: 找到 $gcno_count 个 .gcno 文件在build目录外"

# 检查.gcda文件
gcda_count=$(find /home/liying/sqlcc -name "*.gcda" -not -path "*/build*" | wc -l)
echo "  - .gcda文件: 找到 $gcda_count 个 .gcda 文件在build目录外"

# 检查.csv文件
csv_count=$(find /home/liying/sqlcc -name "*.csv" -not -path "*/build*" | wc -l)
echo "  - .csv文件: 找到 $csv_count 个 .csv 文件在build目录外"

echo "=== 检查完成 ==="