#!/bin/bash
# 覆盖率趋势分析脚本

CURRENT_COVERAGE="$1"

echo "分析覆盖率趋势..."

# 创建覆盖率历史文件（如果不存在）
HISTORY_FILE="test_reports/coverage_history.txt"
if [ ! -f "$HISTORY_FILE" ]; then
    echo "# SQLCC 覆盖率历史记录" > "$HISTORY_FILE"
    echo "# 格式: 日期 时间 行覆盖率 分支覆盖率 函数覆盖率" >> "$HISTORY_FILE"
    echo "# 自动生成，请勿手动修改" >> "$HISTORY_FILE"
    echo "" >> "$HISTORY_FILE"
fi

# 记录当前覆盖率数据
CURRENT_DATE=$(date +%Y-%m-%d)
CURRENT_TIME=$(date +%H:%M:%S)

# 如果没有提供当前覆盖率，从summary.txt中读取
if [ -z "$CURRENT_COVERAGE" ] || [ "$CURRENT_COVERAGE" = "0" ]; then
    if [ -f "coverage_report/summary.txt" ]; then
        CURRENT_COVERAGE=$(grep "lines......:" coverage_report/summary.txt | sed 's/.*lines......: \([0-9.]*\).*/\1/' | head -1)
    fi
fi

BRANCH_COVERAGE="N/A"
FUNCTION_COVERAGE="N/A"
if [ -f "coverage_report/summary.txt" ]; then
    BRANCH_COVERAGE=$(grep "branches......:" coverage_report/summary.txt | sed 's/.*branches......: \([0-9.]*\).*/\1/' | head -1)
    FUNCTION_COVERAGE=$(grep "functions......:" coverage_report/summary.txt | sed 's/.*functions......: \([0-9.]*\).*/\1/' | head -1)
fi

# 记录到历史文件
echo "$CURRENT_DATE $CURRENT_TIME $CURRENT_COVERAGE $BRANCH_COVERAGE $FUNCTION_COVERAGE" >> "$HISTORY_FILE"

echo "=== 覆盖率趋势分析 ==="
echo ""
echo "当前覆盖率数据:"
echo "- 行覆盖率: ${CURRENT_COVERAGE}%"
echo "- 分支覆盖率: ${BRANCH_COVERAGE}%"
echo "- 函数覆盖率: ${FUNCTION_COVERAGE}%"
echo ""

# 分析历史趋势
echo "历史趋势分析:"
echo ""

# 计算历史平均值
if [ -f "$HISTORY_FILE" ]; then
    # 读取最近10条记录
    RECENT_RECORDS=$(tail -n 10 "$HISTORY_FILE" | grep -v "^#" | grep -v "^$")

    if [ -n "$RECENT_RECORDS" ]; then
        echo "最近10次测试的覆盖率变化:"

        # 解析并显示趋势
        PREV_COVERAGE=""
        TREND_UP=0
        TREND_DOWN=0
        TREND_SAME=0

        echo "$RECENT_RECORDS" | while read -r line; do
            if [[ $line =~ ^([0-9-]+)\ ([0-9:]+)\ ([0-9.]+)\ (.+)\ (.+)$ ]]; then
                date="${BASH_REMATCH[1]}"
                time="${BASH_REMATCH[2]}"
                coverage="${BASH_REMATCH[3]}"
                branch="${BASH_REMATCH[4]}"
                function_cov="${BASH_REMATCH[5]}"

                echo "  $date $time: 行覆盖率=${coverage}%, 分支覆盖率=${branch}%, 函数覆盖率=${function_cov}%"

                # 趋势分析
                if [ -n "$PREV_COVERAGE" ] && [[ $coverage =~ ^[0-9]+(\.[0-9]+)?$ ]] && [[ $PREV_COVERAGE =~ ^[0-9]+(\.[0-9]+)?$ ]]; then
                    if (( $(echo "$coverage > $PREV_COVERAGE" | bc -l 2>/dev/null || echo "0") )); then
                        TREND_UP=$((TREND_UP + 1))
                    elif (( $(echo "$coverage < $PREV_COVERAGE" | bc -l 2>/dev/null || echo "0") )); then
                        TREND_DOWN=$((TREND_DOWN + 1))
                    else
                        TREND_SAME=$((TREND_SAME + 1))
                    fi
                fi
                PREV_COVERAGE="$coverage"
            fi
        done

        echo ""
        echo "趋势统计:"
        echo "- 上升次数: $TREND_UP"
        echo "- 下降次数: $TREND_DOWN"
        echo "- 持平次数: $TREND_SAME"

        if [ "$TREND_UP" -gt "$TREND_DOWN" ]; then
            echo "总体趋势: 📈 上升"
        elif [ "$TREND_DOWN" -gt "$TREND_UP" ]; then
            echo "总体趋势: 📉 下降"
        else
            echo "总体趋势: ➡️ 稳定"
        fi
    else
        echo "暂无历史数据，这是第一次记录"
    fi
fi

echo ""
echo "=== 与基准对比 ==="
echo ""

# 基准覆盖率 (从报告中设定)
BASELINE_COVERAGE="11.9"

if [[ $CURRENT_COVERAGE =~ ^[0-9]+(\.[0-9]+)?$ ]] && [[ $BASELINE_COVERAGE =~ ^[0-9]+(\.[0-9]+)?$ ]]; then
    IMPROVEMENT=$(echo "scale=2; $CURRENT_COVERAGE - $BASELINE_COVERAGE" | bc -l 2>/dev/null)
    IMPROVEMENT_PERCENT=$(echo "scale=2; ($CURRENT_COVERAGE - $BASELINE_COVERAGE) / $BASELINE_COVERAGE * 100" | bc -l 2>/dev/null)

    echo "基准覆盖率: $BASELINE_COVERAGE%"
    echo "当前覆盖率: $CURRENT_COVERAGE%"
    echo "绝对提升: $IMPROVEMENT%"
    echo "相对提升: $IMPROVEMENT_PERCENT%"

    if (( $(echo "$IMPROVEMENT > 0" | bc -l 2>/dev/null || echo "0") )); then
        echo "状态: ✅ 优于基准"
    elif (( $(echo "$IMPROVEMENT == 0" | bc -l 2>/dev/null || echo "0") )); then
        echo "状态: ➡️ 达到基准"
    else
        echo "状态: ❌ 低于基准"
    fi
else
    echo "无法计算与基准的对比（数据格式问题）"
fi

echo ""
echo "=== 改进建议 ==="
echo ""

if [[ $CURRENT_COVERAGE =~ ^[0-9]+(\.[0-9]+)?$ ]]; then
    if (( $(echo "$CURRENT_COVERAGE < 15" | bc -l 2>/dev/null || echo "0") )); then
        echo "🚨 紧急改进需求:"
        echo "- 覆盖率严重不足，需要立即采取措施"
        echo "- 优先改进网络服务和权限管理模块"
        echo "- 考虑增加单元测试用例数量"
        echo ""
    fi

    if (( $(echo "$CURRENT_COVERAGE >= 15 && $CURRENT_COVERAGE < 25" | bc -l 2>/dev/null || echo "0") )); then
        echo "⚠️ 中等改进需求:"
        echo "- 覆盖率处于可接受范围内，但有提升空间"
        echo "- 继续完善现有测试用例"
        echo "- 关注分支覆盖率和函数覆盖率"
        echo ""
    fi

    if (( $(echo "$CURRENT_COVERAGE >= 25" | bc -l 2>/dev/null || echo "0") )); then
        echo "✅ 良好状态:"
        echo "- 覆盖率达到较好水平"
        echo "- 重点关注新增代码的测试覆盖"
        echo "- 维护现有测试质量"
        echo ""
    fi
fi

echo "目标覆盖率路线图:"
echo "- 阶段1 (1个月): 20%  - 网络服务和权限管理模块重点改进"
echo "- 阶段2 (3个月): 35%  - 执行引擎和SQL解析器完善"
echo "- 阶段3 (6个月): 50%  - 全面覆盖率提升"
