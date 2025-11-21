#!/bin/bash

# 自动化发布脚本
# 用法: ./scripts/release.sh <版本号> [提交信息]
# 示例: ./scripts/release.sh v0.2.4 "修复缓冲池溢出问题"

set -e  # 遇到错误立即退出

# 显示帮助信息
show_help() {
    echo "自动化发布脚本"
    echo "用法: $0 <版本号> [提交信息]"
    echo "选项:"
    echo "  --help, -h     显示此帮助信息"
    echo "  --dry-run      模拟运行，不执行实际的Git操作"
    echo ""
    echo "示例:"
    echo "  $0 v0.2.4 \"修复缓冲池溢出问题\""
    echo "  $0 --dry-run v0.2.5 \"添加新功能\""
}

# 检查参数
if [ $# -lt 1 ]; then
    show_help
    exit 1
fi

# 处理选项
DRY_RUN=false
if [ "$1" = "--help" ] || [ "$1" = "-h" ]; then
    show_help
    exit 0
elif [ "$1" = "--dry-run" ]; then
    DRY_RUN=true
    if [ $# -lt 2 ]; then
        echo "错误: --dry-run 选项需要指定版本号"
        show_help
        exit 1
    fi
    shift
fi

VERSION=$1
MESSAGE=${2:-"发布版本 $VERSION"}

echo "开始自动化发布流程..."
echo "版本号: $VERSION"
echo "提交信息: $MESSAGE"
if [ "$DRY_RUN" = true ]; then
    echo "模式: 模拟运行 (不会执行实际的Git操作)"
fi

# 确保在主分支开始
echo "切换到主分支..."
if [ "$DRY_RUN" = false ]; then
    git checkout master
    git pull origin master
else
    echo "[模拟] git checkout master"
    echo "[模拟] git pull origin master"
fi

# 更新版本信息
echo "更新版本信息..."
if [ "$DRY_RUN" = false ]; then
    echo "$VERSION" > VERSION
    git add VERSION
    git commit -m "更新版本到 $VERSION"
else
    echo "[模拟] echo \"$VERSION\" > VERSION"
    echo "[模拟] git add VERSION"
    echo "[模拟] git commit -m \"更新版本到 $VERSION\""
fi

# 创建标签
echo "创建标签 $VERSION..."
if [ "$DRY_RUN" = false ]; then
    git tag -a "$VERSION" -m "$MESSAGE"
else
    echo "[模拟] git tag -a \"$VERSION\" -m \"$MESSAGE\""
fi

# 生成文档
echo "生成Doxygen文档..."
if [ "$DRY_RUN" = false ]; then
    make docs
else
    echo "[模拟] make docs"
fi

echo "生成代码覆盖率测试文档..."
if [ "$DRY_RUN" = false ]; then
    make coverage
else
    echo "[模拟] make coverage"
fi

# 复制生成的文档
echo "复制生成的文档到文档分支..."
if [ "$DRY_RUN" = false ]; then
    rm -rf docs/doxygen docs/coverage
else
    echo "[模拟] rm -rf docs/doxygen docs/coverage"
fi

<<<<<<< Updated upstream
=======
# 从主分支获取文档
>>>>>>> Stashed changes
# 使用临时目录传递文档
TEMP_DIR="/tmp/sqlcc_release_docs_$$"
if [ "$DRY_RUN" = false ]; then
    mkdir -p "$TEMP_DIR"
    cp -r docs/doxygen docs/coverage "$TEMP_DIR/"
else
    echo "[模拟] mkdir -p \"$TEMP_DIR\""
    echo "[模拟] cp -r docs/doxygen docs/coverage \"$TEMP_DIR/\""
fi

# 切换到文档分支
echo "切换到文档分支..."
if [ "$DRY_RUN" = false ]; then
    git stash  # 暂存未跟踪的文档文件
    git checkout docs
    git pull origin docs
else
    echo "[模拟] git stash"
    echo "[模拟] git checkout docs"
    echo "[模拟] git pull origin docs"
fi

# 从临时目录复制文档
if [ "$DRY_RUN" = false ]; then
    cp -r "$TEMP_DIR"/* docs/
    rm -rf "$TEMP_DIR"
else
    echo "[模拟] cp -r \"$TEMP_DIR\"/* docs/"
    echo "[模拟] rm -rf \"$TEMP_DIR\""
fi

# 提交文档分支的更改
echo "提交文档分支的更改..."
if [ "$DRY_RUN" = false ]; then
    git add docs/doxygen/ docs/coverage/
    git commit -m "更新文档 - $MESSAGE"
else
    echo "[模拟] git add docs/doxygen/ docs/coverage/"
    echo "[模拟] git commit -m \"更新文档 - $MESSAGE\""
fi

# 推送更改到远程仓库
echo "推送主分支和标签到远程仓库..."
if [ "$DRY_RUN" = false ]; then
    git checkout master
    git stash pop  # 恢复暂存的文档文件
    git push origin master
    git push origin "$VERSION"
else
    echo "[模拟] git checkout master"
    echo "[模拟] git stash pop"
    echo "[模拟] git push origin master"
    echo "[模拟] git push origin \"$VERSION\""
fi

echo "推送文档分支到远程仓库..."
if [ "$DRY_RUN" = false ]; then
    git checkout docs
    git push origin docs
else
    echo "[模拟] git checkout docs"
    echo "[模拟] git push origin docs"
fi
<<<<<<< Updated upstream
=======

# 确保在主分支开始
echo "切换到主分支..."
if [ "$DRY_RUN" = false ]; then
    git checkout master
    git pull origin master
else
    echo "[模拟] git checkout master"
    echo "[模拟] git pull origin master"
fi

# 更新版本信息
echo "更新版本信息..."
if [ "$DRY_RUN" = false ]; then
    echo "$VERSION" > VERSION
    git add VERSION
    git commit -m "更新版本到 $VERSION"
else
    echo "[模拟] echo \"$VERSION\" > VERSION"
    echo "[模拟] git add VERSION"
    echo "[模拟] git commit -m \"更新版本到 $VERSION\""
fi

# 创建标签
echo "创建标签 $VERSION..."
if [ "$DRY_RUN" = false ]; then
    git tag -a "$VERSION" -m "$MESSAGE"
else
    echo "[模拟] git tag -a \"$VERSION\" -m \"$MESSAGE\""
fi

# 生成文档
echo "生成Doxygen文档..."
if [ "$DRY_RUN" = false ]; then
    make docs
else
    echo "[模拟] make docs"
fi

echo "生成代码覆盖率测试文档..."
if [ "$DRY_RUN" = false ]; then
    make coverage
else
    echo "[模拟] make coverage"
fi

# 复制生成的文档
echo "复制生成的文档到文档分支..."
if [ "$DRY_RUN" = false ]; then
    rm -rf docs/doxygen docs/coverage
else
    echo "[模拟] rm -rf docs/doxygen docs/coverage"
fi

# 使用临时目录传递文档
TEMP_DIR="/tmp/sqlcc_release_docs_$$"
if [ "$DRY_RUN" = false ]; then
    mkdir -p "$TEMP_DIR"
    cp -r docs/doxygen docs/coverage "$TEMP_DIR/"
else
    echo "[模拟] mkdir -p \"$TEMP_DIR\""
    echo "[模拟] cp -r docs/doxygen docs/coverage \"$TEMP_DIR/\""
fi

# 切换到文档分支
echo "切换到文档分支..."
if [ "$DRY_RUN" = false ]; then
    git stash  # 暂存未跟踪的文档文件
    git checkout docs
    git pull origin docs
else
    echo "[模拟] git stash"
    echo "[模拟] git checkout docs"
    echo "[模拟] git pull origin docs"
fi

# 从临时目录复制文档
if [ "$DRY_RUN" = false ]; then
    cp -r "$TEMP_DIR"/* docs/
    rm -rf "$TEMP_DIR"
else
    echo "[模拟] cp -r \"$TEMP_DIR\"/* docs/"
    echo "[模拟] rm -rf \"$TEMP_DIR\""
fi

# 提交文档分支的更改
echo "提交文档分支的更改..."
if [ "$DRY_RUN" = false ]; then
    git add docs/doxygen/ docs/coverage/
    git commit -m "更新文档 - $MESSAGE"
else
    echo "[模拟] git add docs/doxygen/ docs/coverage/"
    echo "[模拟] git commit -m \"更新文档 - $MESSAGE\""
fi

# 推送更改到远程仓库
echo "推送主分支和标签到远程仓库..."
if [ "$DRY_RUN" = false ]; then
    git checkout master
    git stash pop  # 恢复暂存的文档文件
    git push origin master
    git push origin "$VERSION"
else
    echo "[模拟] git checkout master"
    echo "[模拟] git stash pop"
    echo "[模拟] git push origin master"
    echo "[模拟] git push origin \"$VERSION\""
fi

echo "推送文档分支到远程仓库..."
if [ "$DRY_RUN" = false ]; then
    git checkout docs
    git push origin docs
else
    echo "[模拟] git checkout docs"
    echo "[模拟] git push origin docs"
fi
>>>>>>> master
>>>>>>> Stashed changes

echo "发布完成!"
echo "主分支已更新并推送标签 $VERSION"
echo "文档分支已更新并推送"
echo "请访问Gitee查看发布情况"