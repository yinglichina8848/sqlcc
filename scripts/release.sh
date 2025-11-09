#!/bin/bash

# 自动化发布脚本
# 用法: ./scripts/release.sh <版本号> [提交信息]
# 示例: ./scripts/release.sh v0.2.4 "修复缓冲池溢出问题"

set -e  # 遇到错误立即退出

# 检查参数
if [ $# -lt 1 ]; then
    echo "用法: $0 <版本号> [提交信息]"
    echo "示例: $0 v0.2.4 \"修复缓冲池溢出问题\""
    exit 1
fi

VERSION=$1
MESSAGE=${2:-"发布版本 $VERSION"}

echo "开始自动化发布流程..."
echo "版本号: $VERSION"
echo "提交信息: $MESSAGE"

# 确保在主分支开始
echo "切换到主分支..."
git checkout master
git pull origin master

# 更新版本信息
echo "更新版本信息..."
echo "$VERSION" > VERSION
git add VERSION
git commit -m "更新版本到 $VERSION"

# 创建标签
echo "创建标签 $VERSION..."
git tag -a "$VERSION" -m "$MESSAGE"

# 生成文档
echo "生成Doxygen文档..."
make docs

echo "生成代码覆盖率测试文档..."
make coverage

# 复制生成的文档
echo "复制生成的文档到文档分支..."
rm -rf docs/doxygen docs/coverage

# 使用临时目录传递文档
TEMP_DIR="/tmp/sqlcc_release_docs_$$"
mkdir -p "$TEMP_DIR"
cp -r docs/doxygen docs/coverage "$TEMP_DIR/"

# 切换到文档分支
git stash  # 暂存未跟踪的文档文件
git checkout docs
git pull origin docs

# 从临时目录复制文档
cp -r "$TEMP_DIR"/* docs/
rm -rf "$TEMP_DIR"

# 提交文档分支的更改
echo "提交文档分支的更改..."
git add docs/doxygen/ docs/coverage/
git commit -m "更新文档 - $MESSAGE"

# 推送更改到远程仓库
echo "推送主分支和标签到远程仓库..."
git checkout master
git stash pop  # 恢复暂存的文档文件
git push origin master
git push origin "$VERSION"

echo "推送文档分支到远程仓库..."
git checkout docs
git push origin docs

echo "发布完成!"
echo "主分支已更新并推送标签 $VERSION"
echo "文档分支已更新并推送"
echo "请访问Gitee查看发布情况"