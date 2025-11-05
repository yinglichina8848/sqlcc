#!/bin/bash

# SQLCC 文档生成脚本
# 用于生成Doxygen API文档

echo "SQLCC 文档生成脚本"
echo "===================="

# 检查Doxygen是否安装
if ! command -v doxygen &> /dev/null; then
    echo "错误: Doxygen 未安装"
    echo "请使用以下命令安装:"
    echo "  sudo apt update && sudo apt install -y doxygen"
    exit 1
fi

# 确保在项目根目录执行
if [ ! -f "Doxyfile" ]; then
    echo "错误: 未找到Doxyfile配置文件"
    echo "请确保在项目根目录执行此脚本"
    exit 1
fi

# 创建文档目录
mkdir -p docs/doxygen

# 生成文档
echo "正在生成Doxygen文档..."
doxygen Doxyfile

# 检查是否成功
if [ $? -eq 0 ]; then
    echo "文档生成成功!"
    echo "API文档位置: docs/doxygen/html/index.html"
else
    echo "文档生成失败!"
    exit 1
fi

echo "完成!"