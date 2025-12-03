#!/bin/bash

# CRUD性能测试一键运行脚本
# 根据README.md要求，验证1-10万行数据下单操作耗时<5ms (SSD)的性能要求

set -e  # 遇到错误立即退出

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 脚本信息
echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}  SQLCC CRUD性能测试一键运行脚本${NC}"
echo -e "${BLUE}========================================${NC}"
echo -e "${YELLOW}测试要求：1-10万行数据下，单操作耗时<5ms (SSD)${NC}"
echo ""

# 检查是否在项目根目录
if [ ! -f "CMakeLists.txt" ]; then
    echo -e "${RED}错误：请在SQLCC项目根目录运行此脚本${NC}"
    exit 1
fi

# 检查构建目录
BUILD_DIR="build"
if [ ! -d "$BUILD_DIR" ]; then
    echo -e "${YELLOW}构建目录不存在，创建构建目录...${NC}"
    mkdir -p "$BUILD_DIR"
fi

# 进入构建目录
cd "$BUILD_DIR"

# 检查是否已配置CMake
if [ ! -f "CMakeCache.txt" ]; then
    echo -e "${YELLOW}配置CMake...${NC}"
    cmake .. -DCMAKE_BUILD_TYPE=Release
fi

# 构建项目
echo -e "${YELLOW}构建性能测试程序...${NC}"
make performance_test -j$(nproc)
make crud_performance_test_main -j$(nproc)

# 检查构建是否成功
if [ ! -f "bin/performance_test" ] || [ ! -f "bin/crud_performance_test_main" ]; then
    echo -e "${RED}错误：构建失败，性能测试程序未生成${NC}"
    exit 1
fi

echo -e "${GREEN}构建成功！${NC}"
echo ""

# 创建测试结果目录
RESULTS_DIR="performance_results"
mkdir -p "$RESULTS_DIR"

# 检查系统环境
echo -e "${BLUE}=== 系统环境检查 ===${NC}"

# 检查是否为SSD环境
if command -v lsblk >/dev/null 2>&1; then
    echo -e "${YELLOW}检查存储设备类型...${NC}"
    ROOT_DEVICE=$(df / | tail -1 | awk '{print $1}')
    if [[ $ROOT_DEVICE == /dev/* ]]; then
        DEVICE_NAME=$(basename "$ROOT_DEVICE")
        DEVICE_TYPE=$(lsblk -d -o NAME,ROTA | grep "^$DEVICE_NAME" | awk '{print $2}')
        if [ "$DEVICE_TYPE" = "0" ]; then
            echo -e "${GREEN}✓ 检测到SSD存储设备${NC}"
            IS_SSD=true
        else
            echo -e "${YELLOW}⚠ 检测到HDD存储设备，性能可能低于SSD${NC}"
            IS_SSD=false
        fi
    else
        echo -e "${YELLOW}⚠ 无法确定存储设备类型${NC}"
        IS_SSD=false
    fi
else
    echo -e "${YELLOW}⚠ 无法检查存储设备类型（lsblk命令不可用）${NC}"
    IS_SSD=false
fi

# 检查内存
TOTAL_MEM=$(free -g | awk 'NR==2{print $2}')
echo -e "${YELLOW}系统内存：${TOTAL_MEM}GB${NC}"

# 检查CPU核心数
CPU_CORES=$(nproc)
echo -e "${YELLOW}CPU核心数：${CPU_CORES}${NC}"

echo ""

# 运行性能测试
echo -e "${BLUE}=== 开始CRUD性能测试 ===${NC}"

# 定义测试函数
run_crud_tests() {
    local test_type=$1
    local test_name=$2
    local command=$3
    
    echo -e "${YELLOW}运行${test_name}测试...${NC}"
    
    # 执行测试
    TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
    LOG_FILE="${RESULTS_DIR}/crud_${test_type}_${TIMESTAMP}.log"
    
    echo "=== $test_name测试结果 ===" > "$LOG_FILE"
    echo "测试时间: $(date)" >> "$LOG_FILE"
    echo "系统环境: ${CPU_CORES}核心CPU, ${TOTAL_MEM}GB内存" >> "$LOG_FILE"
    echo "存储设备: $([ "$IS_SSD" = true ] && echo "SSD" || echo "HDD/未知")" >> "$LOG_FILE"
    echo "" >> "$LOG_FILE"
    
    # 执行测试命令并捕获输出
    if eval "$command" 2>&1 | tee -a "$LOG_FILE"; then
        echo -e "${GREEN}✓ ${test_name}测试完成${NC}"
        return 0
    else
        echo -e "${RED}✗ ${test_name}测试失败${NC}"
        return 1
    fi
}

# 1. 运行快速CRUD测试（1-1万行数据）
run_crud_tests "quick" "快速CRUD" "./bin/crud_performance_test_main --quick --verbose"

# 2. 运行标准CRUD测试（1-10万行数据）
run_crud_tests "standard" "标准CRUD" "./bin/crud_performance_test_main --all --verbose"

# 3. 运行完整性能测试套件中的CRUD测试
run_crud_tests "full" "完整性能测试" "./bin/performance_test --crud --verbose --output-dir ${RESULTS_DIR}"

echo ""

# 生成测试报告
echo -e "${BLUE}=== 生成性能测试报告 ===${NC}"

REPORT_FILE="${RESULTS_DIR}/crud_performance_report_$(date +%Y%m%d_%H%M%S).txt"

cat > "$REPORT_FILE" << EOF
SQLCC CRUD性能测试报告
生成时间: $(date)
测试环境: ${CPU_CORES}核心CPU, ${TOTAL_MEM}GB内存, $([ "$IS_SSD" = true ] && echo "SSD" || echo "HDD/未知")存储

测试要求:
- 1-10万行数据规模
- 单操作耗时 < 5ms (SSD环境)
- 400万ops/sec基准性能

测试结果摘要:
EOF

# 分析测试日志，提取关键性能指标
analyze_logs() {
    local log_file=$1
    local test_name=$2
    
    if [ -f "$log_file" ]; then
        echo "
=== $test_name ===" >> "$REPORT_FILE"
        
        # 提取关键指标
        grep -E "(QPS|延迟|吞吐量|操作耗时|records|ms)" "$log_file" | head -20 >> "$REPORT_FILE"
        
        # 检查是否满足5ms要求
        if grep -q "单操作耗时.*[0-4]\.[0-9]*ms" "$log_file"; then
            echo "✓ 满足单操作<5ms要求" >> "$REPORT_FILE"
        elif grep -q "单操作耗时.*[5-9]\.[0-9]*ms" "$log_file"; then
            echo "⚠ 单操作耗时接近5ms阈值" >> "$REPORT_FILE"
        else
            echo "✗ 单操作耗时超过5ms要求" >> "$REPORT_FILE"
        fi
    fi
}

# 分析所有测试日志
for log_file in "${RESULTS_DIR}"/crud_*.log; do
    if [ -f "$log_file" ]; then
        test_name=$(basename "$log_file" | sed 's/\.log$//' | sed 's/crud_//')
        analyze_logs "$log_file" "$test_name"
    fi
done

# 添加性能建议
cat >> "$REPORT_FILE" << EOF

性能建议:
1. 确保测试环境为SSD存储以获得最佳性能
2. 关闭不必要的后台进程以减少干扰
3. 对于大规模测试，建议在专用测试服务器上运行
4. 定期监控系统资源使用情况

测试文件位置:
- 详细日志: ${RESULTS_DIR}/crud_*.log
- 可执行程序: bin/crud_performance_test_main
- 性能测试套件: bin/performance_test
EOF

echo -e "${GREEN}测试报告已生成: $REPORT_FILE${NC}"

# 显示关键性能指标
echo ""
echo -e "${BLUE}=== 关键性能指标 ===${NC}"

# 从最新的日志文件中提取关键信息
LATEST_LOG=$(ls -t "${RESULTS_DIR}"/crud_*.log 2>/dev/null | head -1)
if [ -n "$LATEST_LOG" ]; then
    echo -e "${YELLOW}最新测试结果摘要:${NC}"
    grep -E "(测试完成|QPS|平均延迟|单操作耗时|records)" "$LATEST_LOG" | tail -10
    
    # 检查性能是否达标
    if grep -q "单操作耗时.*[0-4]\.[0-9]*ms" "$LATEST_LOG"; then
        echo -e "${GREEN}✓ CRUD操作性能满足<5ms要求${NC}"
    else
        echo -e "${RED}✗ CRUD操作性能未达到<5ms要求${NC}"
    fi
else
    echo -e "${YELLOW}未找到测试日志文件${NC}"
fi

echo ""
echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}  CRUD性能测试执行完成！${NC}"
echo -e "${GREEN}========================================${NC}"

# 提供后续操作建议
echo ""
echo -e "${BLUE}后续操作建议:${NC}"
echo "1. 查看详细测试报告: cat $REPORT_FILE"
echo "2. 查看测试日志: ls -la ${RESULTS_DIR}/crud_*.log"
echo "3. 重新运行快速测试: ./bin/crud_performance_test_main --quick"
echo "4. 运行特定数据规模测试: ./bin/crud_performance_test_main --scale 50000"
echo ""

# 设置脚本可执行权限
chmod +x "../run_crud_performance.sh"

echo -e "${GREEN}一键运行脚本已准备就绪！${NC}"