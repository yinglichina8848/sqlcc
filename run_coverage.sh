#!/bin/bash
# SQLCC v1.1.1 一键覆盖率测试脚本
# 功能：一键构建项目、运行测试并生成覆盖率报告

set -e

# 配置参数
BUILD_DIR="build_coverage"
REPORT_DIR="coverage_report"

# 定义颜色输出
GREEN="\033[0;32m"
YELLOW="\033[1;33m"
RED="\033[0;31m"
BLUE="\033[0;34m"
NC="\033[0m" # No Color

echo -e "${BLUE}=====================================${NC}"
echo -e "${BLUE}🚀 SQLCC v1.1.1 一键覆盖率测试${NC}"
echo -e "${BLUE}=====================================${NC}"
echo -e "${YELLOW}📅 开始时间: $(date)${NC}"

# 步骤1: 清理并创建构建目录
echo -e "${BLUE}📦 步骤1: 准备构建环境...${NC}"
if [ -d "$BUILD_DIR" ]; then
    echo -e "${YELLOW}清理现有构建目录...${NC}"
    rm -rf "$BUILD_DIR"
fi
mkdir -p "$BUILD_DIR"

# 步骤2: 配置CMake启用覆盖率
echo -e "${BLUE}⚙️  步骤2: 配置CMake启用代码覆盖率...${NC}"
cd "$BUILD_DIR"
cmake -DENABLE_COVERAGE=ON ..

# 步骤3: 编译项目
echo -e "${BLUE}🔨 步骤3: 编译项目...${NC}"
make -j$(nproc)

# 步骤4: 运行测试
echo -e "${BLUE}🧪 步骤4: 运行测试生成覆盖率数据...${NC}"
./tests/compare_values_test

# 步骤5: 生成覆盖率报告
echo -e "${BLUE}📊 步骤5: 生成覆盖率报告...${NC}"

# 使用gcov生成文本报告
echo -e "${YELLOW}生成文本覆盖率报告...${NC}"
gcov tests/CMakeFiles/compare_values_test.dir/unit/basic/compare_values_test.cpp.gcda

# 检查覆盖率结果
COVERAGE_FILE="compare_values_test.cpp.gcov"
if [ -f "$COVERAGE_FILE" ]; then
    echo -e "${GREEN}✅ 覆盖率报告生成成功！${NC}"
    
    # 提取覆盖率百分比
    COVERAGE_PERCENT=$(grep "Lines executed:" "$COVERAGE_FILE" | head -1 | awk '{print $3}' | sed 's/%//')
    echo -e "${GREEN}📈 代码覆盖率: ${COVERAGE_PERCENT}%${NC}"
    
    # 显示关键统计信息
    echo -e "${BLUE}📋 覆盖率统计:${NC}"
    grep -E "(Lines executed:|Creating .*\.gcov)" "$COVERAGE_FILE" | head -5
    
    # 创建报告目录
    cd ..
    mkdir -p "$REPORT_DIR"
    
    # 复制覆盖率文件到报告目录
    cp "$BUILD_DIR/compare_values_test.cpp.gcov" "$REPORT_DIR/"
    cp "$BUILD_DIR/compare_values_test.cpp" "$REPORT_DIR/" 2>/dev/null || true
    
    echo -e "${GREEN}📁 报告文件已保存到: $REPORT_DIR/${NC}"
    echo -e "${GREEN}📄 主要报告文件: compare_values_test.cpp.gcov${NC}"
    
    # 显示部分代码覆盖率详情
    echo -e "${BLUE}📝 部分代码覆盖率详情:${NC}"
    head -50 "$REPORT_DIR/compare_values_test.cpp.gcov" | grep -E "(^[0-9]+:|^#####:|^=====:)" | head -10
else
    echo -e "${RED}❌ 覆盖率报告生成失败${NC}"
    exit 1
fi

# 步骤6: 生成HTML报告（如果lcov可用）
echo -e "${BLUE}🌐 步骤6: 尝试生成HTML报告...${NC}"
if command -v lcov >/dev/null 2>&1 && command -v genhtml >/dev/null 2>&1; then
    cd "$BUILD_DIR"
    echo -e "${YELLOW}生成HTML覆盖率报告...${NC}"
    
    # 尝试使用lcov
    lcov --capture --directory . --output-file coverage.info --ignore-errors mismatch 2>/dev/null || \
    echo -e "${YELLOW}lcov报告生成遇到兼容性问题，使用文本报告${NC}"
    
    if [ -f "coverage.info" ]; then
        genhtml coverage.info --output-directory ../"$REPORT_DIR"/html --title "SQLCC v1.1.1 覆盖率报告"
        echo -e "${GREEN}✅ HTML报告生成成功: $REPORT_DIR/html/index.html${NC}"
    fi
else
    echo -e "${YELLOW}⚠️  lcov工具不可用，跳过HTML报告生成${NC}"
fi

# 完成总结
cd ..
echo -e "${BLUE}=====================================${NC}"
echo -e "${GREEN}✅ SQLCC v1.1.1 覆盖率测试完成！${NC}"
echo -e "${GREEN}📅 完成时间: $(date)${NC}"
echo -e "${GREEN}📊 代码覆盖率: ${COVERAGE_PERCENT}%${NC}"
echo -e "${GREEN}📁 报告目录: $REPORT_DIR/${NC}"
echo -e "${BLUE}=====================================${NC}"

# 显示使用说明
echo -e "${YELLOW}💡 使用说明:${NC}"
echo -e "${YELLOW}   查看文本报告: cat $REPORT_DIR/compare_values_test.cpp.gcov${NC}"
echo -e "${YELLOW}   查看HTML报告: 打开 $REPORT_DIR/html/index.html (如果生成成功)${NC}"
echo -e "${YELLOW}   重新运行测试: ./run_coverage.sh${NC}"