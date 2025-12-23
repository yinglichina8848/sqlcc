#!/bin/bash

# SQLCC 编译性能监控脚本
# 定期监控编译时间变化，跟踪优化效果

set -e

echo "📊 SQLCC 编译性能监控"
echo "======================="

# 配置参数
TARGET_MODULE="src/core:core"
HISTORY_FILE="compile_performance_history.txt"
ALERT_THRESHOLD=10  # 性能下降阈值（百分比）

# 如果历史文件不存在，创建它
if [ ! -f "$HISTORY_FILE" ]; then
    echo "创建新的性能历史文件: $HISTORY_FILE"
    echo "# SQLCC 编译性能历史记录" > "$HISTORY_FILE"
    echo "# 格式: 时间戳 平均编译时间(秒) 备注" >> "$HISTORY_FILE"
    echo "# $(date)" >> "$HISTORY_FILE"
fi

echo "🎯 目标模块: $TARGET_MODULE"
echo "📈 历史文件: $HISTORY_FILE"
echo ""

# 运行基准测试
echo "🏃 运行编译基准测试..."
BENCHMARK_OUTPUT=$(./scripts/benchmark_compile_time.sh 2>/dev/null | grep "平均编译时间" | awk '{print $3}' | sed 's/秒//')

if [ -z "$BENCHMARK_OUTPUT" ]; then
    echo "❌ 基准测试失败，无法获取编译时间"
    exit 1
fi

CURRENT_TIME=$(date +%s)
COMPILE_TIME=$BENCHMARK_OUTPUT

echo "✅ 当前编译时间: ${COMPILE_TIME}秒"

# 记录到历史文件
echo "$CURRENT_TIME $COMPILE_TIME $(date '+%Y-%m-%d %H:%M:%S')" >> "$HISTORY_FILE"

# 分析趋势
echo ""
echo "📈 性能趋势分析:"

# 获取最近5次记录
RECENT_RECORDS=$(tail -n 5 "$HISTORY_FILE" | grep -v "^#" | awk '{print $2}')

if [ $(echo "$RECENT_RECORDS" | wc -l) -ge 2 ]; then
    # 计算平均值和趋势
    AVG_TIME=$(echo "$RECENT_RECORDS" | awk '{sum+=$1} END {print sum/NR}')

    # 获取第一次和最后一次记录
    FIRST_TIME=$(echo "$RECENT_RECORDS" | head -n 1)
    LAST_TIME=$(echo "$RECENT_RECORDS" | tail -n 1)

    # 计算变化百分比
    if [ $(echo "$FIRST_TIME > 0" | bc -l) -eq 1 ]; then
        CHANGE_PERCENT=$(echo "scale=2; (($LAST_TIME - $FIRST_TIME) / $FIRST_TIME) * 100" | bc)

        echo "📊 最近平均编译时间: ${AVG_TIME}秒"
        echo "📈 趋势变化: ${CHANGE_PERCENT}%"

        # 性能警报
        if [ $(echo "$CHANGE_PERCENT > $ALERT_THRESHOLD" | bc -l) -eq 1 ]; then
            echo "🚨 警告: 编译性能下降超过 ${ALERT_THRESHOLD}%!"
            echo "建议检查最近的代码变更和构建配置。"
        elif [ $(echo "$CHANGE_PERCENT < -$ALERT_THRESHOLD" | bc -l) -eq 1 ]; then
            echo "🎉 好消息: 编译性能提升超过 ${ALERT_THRESHOLD}%!"
        fi
    fi
else
    echo "📝 记录数量不足，无法进行趋势分析"
fi

echo ""
echo "📋 最近5次记录:"
tail -n 5 "$HISTORY_FILE" | grep -v "^#" | while read line; do
    timestamp=$(echo $line | awk '{print $1}')
    compile_time=$(echo $line | awk '{print $2}')
    datetime=$(echo $line | cut -d' ' -f3-)
    echo "  $datetime: ${compile_time}秒"
done

echo ""
echo "✅ 性能监控完成"
echo "💡 提示: 定期运行此脚本跟踪编译性能变化"
