#!/bin/bash

# 创建临时目录
temp_dir=$(mktemp -d)

# 复制覆盖率报告到临时目录
cp -r /home/liying/sqlcc/build/coverage/* "$temp_dir/"

# 将临时目录中的文件复制到目标目录
cp -r "$temp_dir"/* /home/liying/sqlcc/docs/coverage_reports/

# 清理临时目录
rm -rf "$temp_dir"

echo "覆盖率报告已成功复制到 /home/liying/sqlcc/docs/coverage_reports/"
