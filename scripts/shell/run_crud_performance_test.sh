#!/bin/bash

# SQLCC CRUD性能测试一键运行脚本
# 测试要求：插入、点查、范围扫描、更新、删除
# 数据规模：1-10万行数据
# 性能要求：单操作耗时<5ms (SSD)

set -e  # 遇到错误时退出

echo "================================================"
echo "SQLCC CRUD性能测试一键运行脚本"
echo "测试要求：1-10万行数据，单操作耗时<5ms (SSD)"
echo "================================================"
echo ""

# 检查是否在项目根目录
if [ ! -f "CMakeLists.txt" ] || [ ! -d "src" ]; then
    echo "错误: 请在SQLCC项目根目录下运行此脚本"
    exit 1
fi

# 创建构建目录
echo "1. 创建构建目录..."
mkdir -p build
cd build

# 配置CMake，启用性能测试
echo "2. 配置CMake，启用性能测试..."
cmake .. -DENABLE_PERFORMANCE_TESTS=ON -DCMAKE_BUILD_TYPE=Release

# 构建项目
echo "3. 构建项目..."
make -j$(nproc)

# 创建结果目录
TIMESTAMP=$(date +%Y%m%d_%H%M%S)
RESULTS_DIR="crud_performance_results_${TIMESTAMP}"
mkdir -p "$RESULTS_DIR"

echo "4. 开始CRUD性能测试..."
echo "   测试环境信息：" > "$RESULTS_DIR/test_environment.txt"
echo "   时间: $(date)" >> "$RESULTS_DIR/test_environment.txt"
echo "   主机名: $(hostname)" >> "$RESULTS_DIR/test_environment.txt"
echo "   CPU: $(lscpu | grep "Model name" | cut -d: -f2 | xargs)" >> "$RESULTS_DIR/test_environment.txt"
echo "   内存: $(free -h | grep Mem | awk '{print $2}')" >> "$RESULTS_DIR/test_environment.txt"
echo "   存储: $(lsblk -o NAME,SIZE,TYPE,MOUNTPOINT | grep -E "sd|nvme" | head -1)" >> "$RESULTS_DIR/test_environment.txt"

# 运行CRUD性能测试
echo "5. 运行CRUD性能测试..."

# 测试1: 插入性能测试 (1万行数据)
echo "   测试1: 插入性能测试 (10,000行数据)..."
START_TIME=$(date +%s.%N)
./tests/performance/performance_test --type=insert_performance --rows=10000 --output="$RESULTS_DIR" --verbose 2>&1 | tee "$RESULTS_DIR/insert_test.log"
END_TIME=$(date +%s.%N)
INSERT_TIME=$(echo "$END_TIME - $START_TIME" | bc)
echo "   插入测试完成，耗时: ${INSERT_TIME}秒"

# 测试2: 点查性能测试
echo "   测试2: 点查性能测试..."
START_TIME=$(date +%s.%N)
./tests/performance/performance_test --type=point_query --rows=10000 --queries=1000 --output="$RESULTS_DIR" --verbose 2>&1 | tee "$RESULTS_DIR/point_query_test.log"
END_TIME=$(date +%s.%N)
POINT_QUERY_TIME=$(echo "$END_TIME - $START_TIME" | bc)
echo "   点查测试完成，耗时: ${POINT_QUERY_TIME}秒"

# 测试3: 范围扫描性能测试
echo "   测试3: 范围扫描性能测试..."
START_TIME=$(date +%s.%N)
./tests/performance/performance_test --type=range_scan --rows=10000 --ranges=100 --output="$RESULTS_DIR" --verbose 2>&1 | tee "$RESULTS_DIR/range_scan_test.log"
END_TIME=$(date +%s.%N)
RANGE_SCAN_TIME=$(echo "$END_TIME - $START_TIME" | bc)
echo "   范围扫描测试完成，耗时: ${RANGE_SCAN_TIME}秒"

# 测试4: 更新性能测试
echo "   测试4: 更新性能测试..."
START_TIME=$(date +%s.%N)
./tests/performance/performance_test --type=update_performance --rows=10000 --updates=1000 --output="$RESULTS_DIR" --verbose 2>&1 | tee "$RESULTS_DIR/update_test.log"
END_TIME=$(date +%s.%N)
UPDATE_TIME=$(echo "$END_TIME - $START_TIME" | bc)
echo "   更新测试完成，耗时: ${UPDATE_TIME}秒"

# 测试5: 删除性能测试
echo "   测试5: 删除性能测试..."
START_TIME=$(date +%s.%N)
./tests/performance/performance_test --type=delete_performance --rows=10000 --deletes=1000 --output="$RESULTS_DIR" --verbose 2>&1 | tee "$RESULTS_DIR/delete_test.log"
END_TIME=$(date +%s.%N)
DELETE_TIME=$(echo "$END_TIME - $START_TIME" | bc)
echo "   删除测试完成，耗时: ${DELETE_TIME}秒"

# 测试6: 混合CRUD操作测试
echo "   测试6: 混合CRUD操作测试..."
START_TIME=$(date +%s.%N)
./tests/performance/performance_test --type=mixed_crud --rows=10000 --operations=5000 --output="$RESULTS_DIR" --verbose 2>&1 | tee "$RESULTS_DIR/mixed_crud_test.log"
END_TIME=$(date +%s.%N)
MIXED_CRUD_TIME=$(echo "$END_TIME - $START_TIME" | bc)
echo "   混合CRUD测试完成，耗时: ${MIXED_CRUD_TIME}秒"

# 测试7: 10万行数据压力测试
echo "   测试7: 10万行数据压力测试..."
START_TIME=$(date +%s.%N)
./tests/performance/performance_test --type=stress_test --rows=100000 --operations=10000 --output="$RESULTS_DIR" --verbose 2>&1 | tee "$RESULTS_DIR/stress_test.log"
END_TIME=$(date +%s.%N)
STRESS_TEST_TIME=$(echo "$END_TIME - $START_TIME" | bc)
echo "   压力测试完成，耗时: ${STRESS_TEST_TIME}秒"

# 生成测试报告
echo "6. 生成测试报告..."
cd ..
cat > "build/$RESULTS_DIR/crud_performance_report.md" << EOF
# SQLCC CRUD性能测试报告

## 测试概述
- **测试时间**: $(date)
- **测试环境**: $(hostname)
- **数据规模**: 1-10万行数据
- **性能要求**: 单操作耗时<5ms (SSD)

## 测试结果汇总

| 测试类型 | 数据规模 | 操作次数 | 总耗时(秒) | 平均延迟(ms) | 是否达标 |
|----------|----------|----------|------------|--------------|----------|
| 插入测试 | 10,000行 | 10,000次 | ${INSERT_TIME} | $(echo "scale=3; $INSERT_TIME * 1000 / 10000" | bc) | $(if [ $(echo "$INSERT_TIME * 1000 / 10000 < 5" | bc) -eq 1 ]; then echo "✅"; else echo "❌"; fi) |
| 点查测试 | 10,000行 | 1,000次 | ${POINT_QUERY_TIME} | $(echo "scale=3; $POINT_QUERY_TIME * 1000 / 1000" | bc) | $(if [ $(echo "$POINT_QUERY_TIME * 1000 / 1000 < 5" | bc) -eq 1 ]; then echo "✅"; else echo "❌"; fi) |
| 范围扫描 | 10,000行 | 100次 | ${RANGE_SCAN_TIME} | $(echo "scale=3; $RANGE_SCAN_TIME * 1000 / 100" | bc) | $(if [ $(echo "$RANGE_SCAN_TIME * 1000 / 100 < 5" | bc) -eq 1 ]; then echo "✅"; else echo "❌"; fi) |
| 更新测试 | 10,000行 | 1,000次 | ${UPDATE_TIME} | $(echo "scale=3; $UPDATE_TIME * 1000 / 1000" | bc) | $(if [ $(echo "$UPDATE_TIME * 1000 / 1000 < 5" | bc) -eq 1 ]; then echo "✅"; else echo "❌"; fi) |
| 删除测试 | 10,000行 | 1,000次 | ${DELETE_TIME} | $(echo "scale=3; $DELETE_TIME * 1000 / 1000" | bc) | $(if [ $(echo "$DELETE_TIME * 1000 / 1000 < 5" | bc) -eq 1 ]; then echo "✅"; else echo "❌"; fi) |
| 混合CRUD | 10,000行 | 5,000次 | ${MIXED_CRUD_TIME} | $(echo "scale=3; $MIXED_CRUD_TIME * 1000 / 5000" | bc) | $(if [ $(echo "$MIXED_CRUD_TIME * 1000 / 5000 < 5" | bc) -eq 1 ]; then echo "✅"; else echo "❌"; fi) |
| 压力测试 | 100,000行 | 10,000次 | ${STRESS_TEST_TIME} | $(echo "scale=3; $STRESS_TEST_TIME * 1000 / 10000" | bc) | $(if [ $(echo "$STRESS_TEST_TIME * 1000 / 10000 < 5" | bc) -eq 1 ]; then echo "✅"; else echo "❌"; fi) |

## 性能分析

### 1. 插入性能
- **测试数据**: 10,000行数据插入
- **平均延迟**: $(echo "scale=3; $INSERT_TIME * 1000 / 10000" | bc) ms
- **吞吐量**: $(echo "scale=0; 10000 / $INSERT_TIME" | bc) ops/sec
- **达标情况**: $(if [ $(echo "$INSERT_TIME * 1000 / 10000 < 5" | bc) -eq 1 ]; then echo "✅ 达标 (<5ms)"; else echo "❌ 未达标 (>5ms)"; fi)

### 2. 点查性能
- **测试数据**: 10,000行数据，1,000次点查
- **平均延迟**: $(echo "scale=3; $POINT_QUERY_TIME * 1000 / 1000" | bc) ms
- **吞吐量**: $(echo "scale=0; 1000 / $POINT_QUERY_TIME" | bc) ops/sec
- **达标情况**: $(if [ $(echo "$POINT_QUERY_TIME * 1000 / 1000 < 5" | bc) -eq 1 ]; then echo "✅ 达标 (<5ms)"; else echo "❌ 未达标 (>5ms)"; fi)

### 3. 范围扫描性能
- **测试数据**: 10,000行数据，100次范围扫描
- **平均延迟**: $(echo "scale=3; $RANGE_SCAN_TIME * 1000 / 100" | bc) ms
- **吞吐量**: $(echo "scale=0; 100 / $RANGE_SCAN_TIME" | bc) ops/sec
- **达标情况**: $(if [ $(echo "$RANGE_SCAN_TIME * 1000 / 100 < 5" | bc) -eq 1 ]; then echo "✅ 达标 (<5ms)"; else echo "❌ 未达标 (>5ms)"; fi)

### 4. 更新性能
- **测试数据**: 10,000行数据，1,000次更新
- **平均延迟**: $(echo "scale=3; $UPDATE_TIME * 1000 / 1000" | bc) ms
- **吞吐量**: $(echo "scale=0; 1000 / $UPDATE_TIME" | bc) ops/sec
- **达标情况**: $(if [ $(echo "$UPDATE_TIME * 1000 / 1000 < 5" | bc) -eq 1 ]; then echo "✅ 达标 (<5ms)"; else echo "❌ 未达标 (>5ms)"; fi)

### 5. 删除性能
- **测试数据**: 10,000行数据，1,000次删除
- **平均延迟**: $(echo "scale=3; $DELETE_TIME * 1000 / 1000" | bc) ms
- **吞吐量**: $(echo "scale=0; 1000 / $DELETE_TIME" | bc) ops/sec
- **达标情况**: $(if [ $(echo "$DELETE_TIME * 1000 / 1000 < 5" | bc) -eq 1 ]; then echo "✅ 达标 (<5ms)"; else echo "❌ 未达标 (>5ms)"; fi)

## 结论

$(if [ $(echo "$INSERT_TIME * 1000 / 10000 < 5" | bc) -eq 1 ] && [ $(echo "$POINT_QUERY_TIME * 1000 / 1000 < 5" | bc) -eq 1 ] && [ $(echo "$RANGE_SCAN_TIME * 1000 / 100 < 5" | bc) -eq 1 ] && [ $(echo "$UPDATE_TIME * 1000 / 1000 < 5" | bc) -eq 1 ] && [ $(echo "$DELETE_TIME * 1000 / 1000 < 5" | bc) -eq 1 ]; then
  echo "✅ **所有CRUD操作均达到性能要求 (<5ms)**"
  echo ""
  echo "SQLCC数据库系统在1-10万行数据规模下，所有CRUD操作（插入、点查、范围扫描、更新、删除）的平均延迟均低于5ms，满足SSD环境下的性能要求。"
else
  echo "⚠️ **部分CRUD操作未达到性能要求**"
  echo ""
  echo "需要优化的操作："
  [ $(echo "$INSERT_TIME * 1000 / 10000 >= 5" | bc) -eq 1 ] && echo "- 插入操作: $(echo "scale=3; $INSERT_TIME * 1000 / 10000" | bc) ms (要求: <5ms)"
  [ $(echo "$POINT_QUERY_TIME * 1000 / 1000 >= 5" | bc) -eq 1 ] && echo "- 点查操作: $(echo "scale=3; $POINT_QUERY_TIME * 1000 / 1000" | bc) ms (要求: <5ms)"
  [ $(echo "$RANGE_SCAN_TIME * 1000 / 100 >= 5" | bc) -eq 1 ] && echo "- 范围扫描: $(echo "scale=3; $RANGE_SCAN_TIME * 1000 / 100" | bc) ms (要求: <5ms)"
  [ $(echo "$UPDATE_TIME * 1000 / 1000 >= 5" | bc) -eq 1 ] && echo "- 更新操作: $(echo "scale=3; $UPDATE_TIME * 1000 / 1000" | bc) ms (要求: <5ms)"
  [ $(echo "$DELETE_TIME * 1000 / 1000 >= 5" | bc) -eq 1 ] && echo "- 删除操作: $(echo "scale=3; $DELETE_TIME * 1000 / 1000" | bc) ms (要求: <5ms)"
fi)

## 详细日志
- 插入测试日志: insert_test.log
- 点查测试日志: point_query_test.log
- 范围扫描日志: range_scan_test.log
- 更新测试日志: update_test.log
- 删除测试日志: delete_test.log
- 混合CRUD日志: mixed_crud_test.log
- 压力测试日志: stress_test.log

## 测试环境
$(cat "build/$RESULTS_DIR/test_environment.txt")
EOF

echo "7. 安装Python依赖（用于数据分析）..."
if ! python3 -c "import pandas, matplotlib, seaborn" 2>/dev/null; then
    echo "   安装Python依赖..."
    pip3 install pandas matplotlib seaborn
else
    echo "   Python依赖已安装"
fi

# 生成性能图表
echo "8. 生成性能图表..."
mkdir -p "build/$RESULTS_DIR/charts"
python3 << EOF
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import os

# 设置中文字体
plt.rcParams['font.sans-serif'] = ['SimHei', 'DejaVu Sans']
plt.rcParams['axes.unicode_minus'] = False

# 测试数据
test_types = ['插入', '点查', '范围扫描', '更新', '删除', '混合CRUD', '压力测试']
avg_latencies = [$(echo "scale=3; $INSERT_TIME * 1000 / 10000" | bc), 
                 $(echo "scale=3; $POINT_QUERY_TIME * 1000 / 1000" | bc),
                 $(echo "scale=3; $RANGE_SCAN_TIME * 1000 / 100" | bc),
                 $(echo "scale=3; $UPDATE_TIME * 1000 / 1000" | bc),
                 $(echo "scale=3; $DELETE_TIME * 1000 / 1000" | bc),
                 $(echo "scale=3; $MIXED_CRUD_TIME * 1000 / 5000" | bc),
                 $(echo "scale=3; $STRESS_TEST_TIME * 1000 / 10000" | bc)]

throughputs = [$(echo "scale=0; 10000 / $INSERT_TIME" | bc),
               $(echo "scale=0; 1000 / $POINT_QUERY_TIME" | bc),
               $(echo "scale=0; 100 / $RANGE_SCAN_TIME" | bc),
               $(echo "scale=0; 1000 / $UPDATE_TIME" | bc),
               $(echo "scale=0; 1000 / $DELETE_TIME" | bc),
               $(echo "scale=0; 5000 / $MIXED_CRUD_TIME" | bc),
               $(echo "scale=0; 10000 / $STRESS_TEST_TIME" | bc)]

# 创建DataFrame
df = pd.DataFrame({
    '测试类型': test_types,
    '平均延迟(ms)': avg_latencies,
    '吞吐量(ops/sec)': throughputs
})

# 1. 延迟对比图
plt.figure(figsize=(12, 6))
bars = plt.bar(df['测试类型'], df['平均延迟(ms)'], color=['#4CAF50', '#2196F3', '#FF9800', '#9C27B0', '#F44336', '#00BCD4', '#795548'])
plt.axhline(y=5, color='r', linestyle='--', label='性能要求 (5ms)')
plt.xlabel('测试类型')
plt.ylabel('平均延迟 (ms)')
plt.title('SQLCC CRUD操作延迟对比')
plt.legend()

# 在柱子上添加数值
for bar, latency in zip(bars, avg_latencies):
    height = bar.get_height()
    plt.text(bar.get_x() + bar.get_width()/2., height + 0.1,
             f'{latency:.2f}ms', ha='center', va='bottom')

plt.tight_layout()
plt.savefig('build/$RESULTS_DIR/charts/latency_comparison.png', dpi=300)
plt.close()

# 2. 吞吐量对比图
plt.figure(figsize=(12, 6))
bars = plt.bar(df['测试类型'], df['吞吐量(ops/sec)'], color=['#4CAF50', '#2196F3', '#FF9800', '#9C27B0', '#F44336', '#00BCD4', '#795548'])
plt.xlabel('测试类型')
plt.ylabel('吞吐量 (ops/sec)')
plt.title('SQLCC CRUD操作吞吐量对比')

# 在柱子上添加数值
for bar, throughput in zip(bars, throughputs):
    height = bar.get_height()
    plt.text(bar.get_x() + bar.get_width()/2., height + 1000,
             f'{throughput:,}', ha='center', va='bottom')

plt.tight_layout()
plt.savefig('build/$RESULTS_DIR/charts/throughput_comparison.png', dpi=300)
plt.close()

# 3. 延迟分布图
plt.figure(figsize=(10, 6))
colors = ['#4CAF50' if lat < 5 else '#F44336' for lat in avg_latencies]
plt.scatter(df['测试类型'], df['平均延迟(ms)'], s=200, c=colors, alpha=0.7)
plt.axhline(y=5, color='r', linestyle='--', label='性能要求 (5ms)')
plt.xlabel('测试类型')
plt.ylabel('平均延迟 (ms)')
plt.title('CRUD操作延迟分布')
plt.legend()
plt.grid(True, alpha=0.3)

for i, (test_type, latency) in enumerate(zip(test_types, avg_latencies)):
    plt.annotate(f'{latency:.2f}ms', (test_type, latency), 
                 textcoords="offset points", xytext=(0,10), ha='center')

plt.tight_layout()
plt.savefig('build/$RESULTS_DIR/charts/latency_distribution.png', dpi=300)
plt.close()

# 4. 性能达标情况图
plt.figure(figsize=(10, 6))
passed = sum(1 for lat in avg_latencies if lat < 5)
failed = len(avg_latencies) - passed
labels = ['达标', '未达标']
sizes = [passed, failed]
colors = ['#4CAF50', '#F44336']
explode = (0.1, 0) if passed > failed else (0, 0.1)

plt.pie(sizes, explode=explode, labels=labels, colors=colors, autopct='%1.1f%%',
        shadow=True, startangle=90)
plt.axis('equal')
plt.title(f'CRUD操作性能达标情况 ({passed}/{len(avg_latencies)} 项达标)')

plt.tight_layout()
plt.savefig('build/$RESULTS_DIR/charts/pass_rate.png', dpi=300)
plt.close()

print("图表生成完成！")
EOF

echo ""
echo "================================================"
echo "CRUD性能测试完成！"
echo "================================================"
echo ""
echo "📊 测试结果汇总："
echo "   插入测试: $(echo "scale=3; $INSERT_TIME * 1000 / 10000" | bc) ms $(if [ $(echo "$INSERT_TIME * 1000 / 10000 < 5" | bc) -eq 1 ]; then echo "✅"; else echo "❌"; fi)"
echo "   点查测试: $(echo "scale=3; $POINT_QUERY_TIME * 1000 / 1000" | bc) ms $(if [ $(echo "$POINT_QUERY_TIME * 1000 / 1000 < 5" | bc) -eq 1 ]; then echo "✅"; else echo "❌"; fi)"
echo "   范围扫描: $(echo "scale=3; $RANGE_SCAN_TIME * 1000 / 100" | bc) ms $(if [ $(echo "$RANGE_SCAN_TIME * 1000 / 100 < 5" | bc) -eq 1 ]; then echo "✅"; else echo "❌"; fi)"
echo "   更新测试: $(echo "scale=3; $UPDATE_TIME * 1000 / 1000" | bc) ms $(if [ $(echo "$UPDATE_TIME * 1000 / 1000 < 5" | bc) -eq 1 ]; then echo "✅"; else echo "❌"; fi)"
echo "   删除测试: $(echo "scale=3; $DELETE_TIME * 1000 / 1000" | bc) ms $(if [ $(echo "$DELETE_TIME * 1000 / 1000 < 5" | bc) -eq 1 ]; then echo "✅"; else echo "❌"; fi)"
echo ""
echo "📁 结果目录: build/$RESULTS_DIR/"
echo "📄 测试报告: build/$RESULTS_DIR/crud_performance_report.md"
echo "📈 性能图表: build/$RESULTS_DIR/charts/"
echo ""
echo "要查看详细报告，请运行:"
echo "  cat build/$RESULTS_DIR/crud_performance_report.md"
echo ""
echo "或打开图表文件查看可视化结果。"
