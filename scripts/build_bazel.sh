#!/bin/bash

# SQLCC Bazel构建脚本
# 功能：使用Bazel构建系统构建项目

echo "====================================="
echo "SQLCC Bazel构建脚本"
echo "====================================="

# 定义颜色输出
GREEN="\033[0;32m"
YELLOW="\033[1;33m"
RED="\033[0;31m"
NC="\033[0m" # No Color
BLUE="\033[0;34m"

# 解析命令行参数
BUILD_TYPE="fastbuild"  # 默认构建类型
CLEAN_BUILD=false
VERBOSE=false
SHOW_PROGRESS=true

for arg in "$@"; do
    case "$arg" in
        --debug)
            BUILD_TYPE="dbg"
            echo -e "${YELLOW}调试模式构建已启用${NC}"
            ;;
        --release)
            BUILD_TYPE="opt"
            echo -e "${YELLOW}发布模式构建已启用${NC}"
            ;;
        --clean)
            CLEAN_BUILD=true
            echo -e "${YELLOW}清理构建已启用${NC}"
            ;;
        --verbose)
            VERBOSE=true
            echo -e "${YELLOW}详细输出已启用${NC}"
            ;;
        --no-progress)
            SHOW_PROGRESS=false
            echo -e "${YELLOW}进度显示已禁用${NC}"
            ;;
        --help)
            echo "用法: $0 [选项]"
            echo "选项:"
            echo "  --debug      使用调试模式构建"
            echo "  --release    使用发布模式构建"
            echo "  --clean      清理之前的构建结果"
            echo "  --verbose    显示详细构建信息"
            echo "  --no-progress 禁用进度显示"
            echo "  --help       显示帮助信息"
            exit 0
            ;;
    esac
done

# 保存原始目录
ORIGINAL_DIR=$(pwd)

# 确保在项目根目录
if [ ! -f "WORKSPACE" ] || [ ! -f "BUILD.bazel" ]; then
    echo -e "${RED}错误：请在SQLCC项目根目录运行此脚本${NC}"
    exit 1
fi

echo -e "${BLUE}项目根目录: $(pwd)${NC}"

# 设置Bazel选项
BAZEL_OPTS="--compilation_mode=$BUILD_TYPE"

if [ "$VERBOSE" = true ]; then
    BAZEL_OPTS="$BAZEL_OPTS --verbose_failures"
fi

if [ "$SHOW_PROGRESS" = true ]; then
    BAZEL_OPTS="$BAZEL_OPTS --show_progress"
fi

# 清理构建（如果需要）
if [ "$CLEAN_BUILD" = true ]; then
    echo -e "${BLUE}清理之前的构建结果...${NC}"
    bazel clean
    if [ $? -ne 0 ]; then
        echo -e "${RED}清理失败${NC}"
        exit 1
    fi
    echo -e "${GREEN}清理完成${NC}"
fi

# 构建项目
echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}开始构建项目${NC}"
echo -e "${BLUE}构建类型: $BUILD_TYPE${NC}"
echo -e "${BLUE}选项: $BAZEL_OPTS${NC}"
echo -e "${BLUE}========================================${NC}"

# 构建所有目标
bazel build $BAZEL_OPTS //...

BUILD_RESULT=$?

if [ $BUILD_RESULT -eq 0 ]; then
    echo -e "${GREEN}✓ 项目构建成功${NC}"
    
    # 显示构建的可执行文件
    echo -e "${BLUE}构建的可执行文件:${NC}"
    bazel query //... --output=label | grep -E "_binary|_test" | head -10
    
    # 显示构建输出目录
    echo -e "${BLUE}构建输出目录:${NC}"
    echo "  bazel-bin/"
    echo "  bazel-out/"
    
else
    echo -e "${RED}✗ 项目构建失败${NC}"
    echo -e "${RED}请检查错误信息并修复问题${NC}"
fi

# 显示构建统计
echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}构建统计${NC}"
echo -e "${BLUE}========================================${NC}"
echo -e "构建类型: $BUILD_TYPE"
echo -e "清理构建: $CLEAN_BUILD"
echo -e "详细输出: $VERBOSE"
echo -e "进度显示: $SHOW_PROGRESS"

# 返回结果
exit $BUILD_RESULT