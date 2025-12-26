#!/bin/bash

# SQLCC 模块覆盖率分析脚本
# 用于分析核心模块的测试覆盖率情况

set -e

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SCRIPT_DIR="${PROJECT_ROOT}/scripts"
REPORTS_DIR="${PROJECT_ROOT}/test_reports"

# 创建报告目录
mkdir -p "${REPORTS_DIR}"

echo "=== SQLCC 模块测试覆盖率分析 ==="
echo "项目根目录: ${PROJECT_ROOT}"
echo "报告目录: ${REPORTS_DIR}"

# 分析函数：统计模块的源码文件和测试文件
analyze_module() {
    local module_name=$1
    local src_dir=$2
    local test_dir=$3

    echo ""
    echo "--- 分析模块: ${module_name} ---"

    # 统计源码文件
    local src_files=$(find "${src_dir}" -name "*.cpp" -o -name "*.cc" 2>/dev/null | wc -l)
    echo "源码文件数量: ${src_files}"

    # 统计测试文件
    local test_files=$(find "${test_dir}" -name "*test*.cpp" -o -name "*test*.cc" 2>/dev/null | wc -l)
    echo "测试文件数量: ${test_files}"

    # 计算覆盖率比率
    if [ "${src_files}" -gt 0 ]; then
        local coverage_ratio=$((test_files * 100 / src_files))
        echo "测试/源码比率: ${coverage_ratio}%"

        # 评估覆盖率水平
        if [ "${coverage_ratio}" -ge 80 ]; then
            echo "覆盖率等级: ⭐⭐⭐⭐⭐ 优秀"
        elif [ "${coverage_ratio}" -ge 60 ]; then
            echo "覆盖率等级: ⭐⭐⭐⭐ 良好"
        elif [ "${coverage_ratio}" -ge 40 ]; then
            echo "覆盖率等级: ⭐⭐⭐ 一般"
        elif [ "${coverage_ratio}" -ge 20 ]; then
            echo "覆盖率等级: ⭐ 可接受"
        else
            echo "覆盖率等级: ⚠️ 需要改进"
        fi
    fi

    # 列出缺少测试的文件
    echo "源码文件列表:"
    find "${src_dir}" -name "*.cpp" -o -name "*.cc" 2>/dev/null | head -10

    echo "测试文件列表:"
    find "${test_dir}" -name "*test*.cpp" -o -name "*test*.cc" 2>/dev/null | head -10
}

# 主分析流程
main() {
    echo "开始分析各模块覆盖率情况..."
    echo ""

    # 核心模块分析
    analyze_module "存储引擎" "${PROJECT_ROOT}/src/storage_engine" "${PROJECT_ROOT}/tests"
    analyze_module "SQL解析器" "${PROJECT_ROOT}/src/sql_parser" "${PROJECT_ROOT}/tests"
    analyze_module "执行引擎" "${PROJECT_ROOT}/src/execution" "${PROJECT_ROOT}/tests"
    analyze_module "网络通信" "${PROJECT_ROOT}/src/network" "${PROJECT_ROOT}/tests"
    analyze_module "核心组件" "${PROJECT_ROOT}/src/core" "${PROJECT_ROOT}/tests"

    echo ""
    echo "=== 详细分析报告 ==="

    # 生成详细报告
    local report_file="${REPORTS_DIR}/module_coverage_analysis_$(date +%Y%m%d_%H%M%S).txt"

    {
        echo "SQLCC 模块测试覆盖率详细分析报告"
        echo "生成时间: $(date)"
        echo "============================================"
        echo ""

        echo "总计统计:"
        echo "- 测试文件总数: $(find "${PROJECT_ROOT}/tests" -name "*.cpp" -o -name "*.cc" | wc -l)"
        echo "- 源码文件总数: $(find "${PROJECT_ROOT}/src" -name "*.cpp" -o -name "*.cc" | wc -l)"
        echo ""

        echo "模块详细分析:"

        # 存储引擎模块详细分析
        echo ""
        echo "1. 存储引擎模块 (src/storage_engine/)"
        echo "源码文件:"
        find "${PROJECT_ROOT}/src/storage_engine" -name "*.cpp" -o -name "*.cc" 2>/dev/null | sed 's|.*/||' | sort

        echo ""
        echo "相关测试文件:"
        find "${PROJECT_ROOT}/tests" -name "*storage*" -name "*.cpp" -o -name "*storage*" -name "*.cc" 2>/dev/null | sed 's|.*/||' | sort

        # SQL解析器模块详细分析
        echo ""
        echo "2. SQL解析器模块 (src/sql_parser/)"
        echo "源码文件:"
        find "${PROJECT_ROOT}/src/sql_parser" -name "*.cpp" -o -name "*.cc" 2>/dev/null | sed 's|.*/||' | sort

        echo ""
        echo "相关测试文件:"
        find "${PROJECT_ROOT}/tests" -name "*parser*" -name "*.cpp" -o -name "*parser*" -name "*.cc" 2>/dev/null | sed 's|.*/||' | sort

        # 执行引擎模块详细分析
        echo ""
        echo "3. 执行引擎模块 (src/execution/)"
        echo "源码文件:"
        find "${PROJECT_ROOT}/src/execution" -name "*.cpp" -o -name "*.cc" 2>/dev/null | sed 's|.*/||' | sort

        echo ""
        echo "相关测试文件:"
        find "${PROJECT_ROOT}/tests" -name "*executor*" -name "*.cpp" -o -name "*executor*" -name "*.cc" 2>/dev/null | sed 's|.*/||' | sort

    } > "${report_file}"

    echo "详细报告已保存到: ${report_file}"
    echo ""
    echo "=== 建议的改进措施 ==="
    echo "1. 优先补充核心模块的单元测试"
    echo "2. 为每个源码文件创建对应的测试文件"
    echo "3. 增加边界条件和错误处理的测试用例"
    echo "4. 建立自动化覆盖率检查机制"
    echo "5. 制定测试覆盖率提升目标和时间表"
}

# 执行主函数
main "$@"
