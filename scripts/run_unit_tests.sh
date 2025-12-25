#!/bin/bash
# SQLCC 综合测试执行脚本 (改进版)
# 支持单元测试、集成测试、性能测试和覆盖率收集

set -e  # 遇到错误立即退出

# 默认参数
COVERAGE_ENABLED=false
TEST_TYPE="unit"
VERBOSE=false
TIMEOUT=60
REPORT_DIR="test_reports"

# 解析命令行参数
while [[ $# -gt 0 ]]; do
  case $1 in
    --coverage)
      COVERAGE_ENABLED=true
      shift
      ;;
    --type=*)
      TEST_TYPE="${1#*=}"
      shift
      ;;
    --verbose)
      VERBOSE=true
      shift
      ;;
    --timeout=*)
      TIMEOUT="${1#*=}"
      shift
      ;;
    --help)
      echo "Usage: $0 [OPTIONS]"
      echo "Options:"
      echo "  --coverage       启用覆盖率测试和数据收集"
      echo "  --type=TYPE      测试类型: unit, integration, performance, all (默认: unit)"
      echo "  --verbose        详细输出模式"
      echo "  --timeout=SEC    测试超时时间(秒，默认: 60)"
      echo "  --help           显示帮助信息"
      exit 0
      ;;
    *)
      echo "Unknown option: $1"
      echo "Use --help for usage information"
      exit 1
      ;;
  esac
done

echo "========================================="
echo "SQLCC 测试执行脚本 (改进版)"
echo "========================================="
echo "测试类型: $TEST_TYPE"
echo "覆盖率测试: $([ "$COVERAGE_ENABLED" = true ] && echo "启用" || echo "禁用")"
echo "详细模式: $([ "$VERBOSE" = true ] && echo "启用" || echo "禁用")"
echo "超时时间: ${TIMEOUT}秒"
echo "========================================="

# 记录开始时间
START_TIME=$(date +%s)

# 创建报告目录
mkdir -p "$REPORT_DIR"

# 设置Bazel覆盖率参数
BAZEL_COVERAGE_ARGS=""
if [ "$COVERAGE_ENABLED" = true ]; then
  BAZEL_COVERAGE_ARGS="--collect_code_coverage --instrumentation_filter=//src/.* --coverage_report_generator=@bazel_tools//tools/test/CoverageOutputGenerator/java/com/google/devtools/coverageoutputgenerator:Main"
fi

# 设置详细输出参数
VERBOSE_ARGS=""
if [ "$VERBOSE" = true ]; then
  VERBOSE_ARGS="--test_output=all"
else
  VERBOSE_ARGS="--test_output=errors"
fi

# 根据测试类型执行相应的测试
execute_tests() {
  local test_target="$1"
  local test_name="$2"

  echo ""
  echo "----------------------------------------"
  echo "执行 $test_name..."
  echo "目标: $test_target"
  echo "----------------------------------------"

  local cmd="bazel test $test_target $VERBOSE_ARGS --test_timeout=$TIMEOUT $BAZEL_COVERAGE_ARGS"
  echo "执行命令: $cmd"

  if eval "$cmd"; then
    echo "✅ $test_name 执行成功"
    return 0
  else
    echo "❌ $test_name 执行失败"
    return 1
  fi
}

# 1. 执行单元测试 (重构后的目录结构)
if [ "$TEST_TYPE" = "unit" ] || [ "$TEST_TYPE" = "all" ]; then
  echo "开始执行单元测试..."

  # 基础单元测试
  execute_tests "//tests/unit/basic/..." "基础单元测试" || UNIT_FAILED=true

  # 核心组件单元测试
  execute_tests "//tests/unit/core/..." "核心组件单元测试" || UNIT_FAILED=true

  # 执行器单元测试
  execute_tests "//tests/unit/executor/..." "执行器单元测试" || UNIT_FAILED=true

  # 解析器单元测试
  execute_tests "//tests/unit/parser/..." "解析器单元测试" || UNIT_FAILED=true

  # 存储单元测试
  execute_tests "//tests/unit/storage/..." "存储单元测试" || UNIT_FAILED=true

  # 其他单元测试
  execute_tests "//tests/unit/..." "其他单元测试" || UNIT_FAILED=true
fi

# 2. 执行集成测试
if [ "$TEST_TYPE" = "integration" ] || [ "$TEST_TYPE" = "all" ]; then
  echo "开始执行集成测试..."

  # SQL功能集成测试
  execute_tests "//tests/integration/basic_sql/..." "基础SQL集成测试" || INTEGRATION_FAILED=true
  execute_tests "//tests/integration/advanced_sql/..." "高级SQL集成测试" || INTEGRATION_FAILED=true
  execute_tests "//tests/integration/join/..." "JOIN操作集成测试" || INTEGRATION_FAILED=true
  execute_tests "//tests/integration/subquery/..." "子查询集成测试" || INTEGRATION_FAILED=true
  execute_tests "//tests/integration/set_operation/..." "集合操作集成测试" || INTEGRATION_FAILED=true
  execute_tests "//tests/integration/window/..." "窗口函数集成测试" || INTEGRATION_FAILED=true
  execute_tests "//tests/integration/grouping/..." "分组操作集成测试" || INTEGRATION_FAILED=true
  execute_tests "//tests/integration/distinct/..." "DISTINCT操作集成测试" || INTEGRATION_FAILED=true

  # 网络集成测试
  execute_tests "//tests/integration/..." "其他集成测试" || INTEGRATION_FAILED=true
fi

# 3. 执行性能测试
if [ "$TEST_TYPE" = "performance" ] || [ "$TEST_TYPE" = "all" ]; then
  echo "开始执行性能测试..."

  # 基础性能测试
  execute_tests "//tests/performance/basic/..." "基础性能测试" || PERFORMANCE_FAILED=true

  # 高级性能测试
  execute_tests "//tests/performance/advanced/..." "高级性能测试" || PERFORMANCE_FAILED=true

  # 并发性能测试
  execute_tests "//tests/performance/concurrency/..." "并发性能测试" || PERFORMANCE_FAILED=true

  # CPU性能测试
  execute_tests "//tests/performance/cpu_test/..." "CPU性能测试" || PERFORMANCE_FAILED=true

  # 内存压力测试
  execute_tests "//tests/performance/memory_stress_test/..." "内存压力测试" || PERFORMANCE_FAILED=true

  # 稳定性测试
  execute_tests "//tests/performance/stability_test/..." "稳定性测试" || PERFORMANCE_FAILED=true
fi

# 4. 执行存储引擎测试
if [ "$TEST_TYPE" = "storage" ] || [ "$TEST_TYPE" = "all" ]; then
  echo "开始执行存储引擎测试..."
  execute_tests "//tests/storage_engine/..." "存储引擎测试" || STORAGE_FAILED=true
fi

# 5. 执行安全测试
if [ "$TEST_TYPE" = "security" ] || [ "$TEST_TYPE" = "all" ]; then
  echo "开始执行安全测试..."
  execute_tests "//tests/security/..." "安全测试" || SECURITY_FAILED=true
fi

# 6. 执行验证测试
if [ "$TEST_TYPE" = "validate" ] || [ "$TEST_TYPE" = "all" ]; then
  echo "开始执行验证测试..."
  execute_tests "//tests/validate/..." "验证测试" || VALIDATE_FAILED=true
fi

# 7. 执行调试测试
if [ "$TEST_TYPE" = "debug" ] || [ "$TEST_TYPE" = "all" ]; then
  echo "开始执行调试测试..."
  execute_tests "//tests/debug/..." "调试测试" || DEBUG_FAILED=true
fi

# 统计所有测试结果并生成综合报告
echo ""
echo "========================================="
echo "生成测试执行报告..."
echo "========================================="

END_TIME=$(date +%s)
TOTAL_DURATION=$((END_TIME - START_TIME))

# 收集所有测试输出
ALL_TEST_OUTPUT=""

# 运行所有测试获取统计信息
if [ "$TEST_TYPE" = "all" ]; then
    echo "运行完整测试统计..."
    ALL_TEST_OUTPUT=$(bazel test //tests/... --test_output=summary 2>&1)
else
    case "$TEST_TYPE" in
        "unit")
            ALL_TEST_OUTPUT=$(bazel test //tests/unit/... --test_output=summary 2>&1)
            ;;
        "integration")
            ALL_TEST_OUTPUT=$(bazel test //tests/integration/... --test_output=summary 2>&1)
            ;;
        "performance")
            ALL_TEST_OUTPUT=$(bazel test //tests/performance/... --test_output=summary 2>&1)
            ;;
        "storage")
            ALL_TEST_OUTPUT=$(bazel test //tests/storage_engine/... --test_output=summary 2>&1)
            ;;
        "security")
            ALL_TEST_OUTPUT=$(bazel test //tests/security/... --test_output=summary 2>&1)
            ;;
        "validate")
            ALL_TEST_OUTPUT=$(bazel test //tests/validate/... --test_output=summary 2>&1)
            ;;
        "debug")
            ALL_TEST_OUTPUT=$(bazel test //tests/debug/... --test_output=summary 2>&1)
            ;;
    esac
fi

echo "$ALL_TEST_OUTPUT"

# 解析测试结果
TOTAL_TESTS=$(echo "$ALL_TEST_OUTPUT" | grep -o "[0-9]\+ / [0-9]\+" | tail -1 | cut -d' ' -f3)
PASSED_TESTS=$(echo "$ALL_TEST_OUTPUT" | grep -o "[0-9]\+ passed" | cut -d' ' -f1)
FAILED_TESTS=$(echo "$ALL_TEST_OUTPUT" | grep -o "[0-9]\+ failed" | cut -d' ' -f1)

# 计算通过率
if [ -n "$TOTAL_TESTS" ] && [ "$TOTAL_TESTS" -gt 0 ]; then
    PASS_RATE=$((PASSED_TESTS * 100 / TOTAL_TESTS))
else
    PASS_RATE=0
fi

# 覆盖率数据收集和分析
COVERAGE_DATA=""
if [ "$COVERAGE_ENABLED" = true ]; then
    echo ""
    echo "----------------------------------------"
    echo "收集覆盖率数据..."
    echo "----------------------------------------"

    # 查找覆盖率文件
    COVERAGE_FILES=$(find . -name "*.dat" -path "*/_coverage/*" 2>/dev/null | head -5)
    if [ -n "$COVERAGE_FILES" ]; then
        COVERAGE_DATA="覆盖率文件已生成:\n$(echo "$COVERAGE_FILES" | sed 's/^/- /')\n"

        # 尝试生成覆盖率报告
        if command -v genhtml >/dev/null 2>&1; then
            echo "生成HTML覆盖率报告..."
            mkdir -p coverage_html
            genhtml --output-directory coverage_html $(echo "$COVERAGE_FILES" | tr '\n' ' ') 2>/dev/null || true
            COVERAGE_DATA="${COVERAGE_DATA}HTML报告: coverage_html/index.html\n"
        fi
    else
        COVERAGE_DATA="未找到覆盖率数据文件\n"
    fi

    # 分析覆盖率趋势
    ./scripts/analyze_coverage_trends.sh 2>/dev/null || true
fi

# 生成综合测试报告
REPORT_FILE="$REPORT_DIR/comprehensive_test_report_$(date +%Y%m%d_%H%M%S).md"

cat > "$REPORT_FILE" << EOF
# SQLCC 综合测试执行报告

## 📊 执行概况

- **测试类型**: $TEST_TYPE
- **覆盖率测试**: $([ "$COVERAGE_ENABLED" = true ] && echo "✅ 启用" || echo "❌ 禁用")
- **详细模式**: $([ "$VERBOSE" = true ] && echo "✅ 启用" || echo "❌ 禁用")
- **执行时间**: $(date)
- **总耗时**: ${TOTAL_DURATION}秒
- **超时设置**: ${TIMEOUT}秒

## 📈 测试结果统计

| 指标 | 数值 |
|------|------|
| 总测试数 | ${TOTAL_TESTS:-0} |
| 通过测试 | ${PASSED_TESTS:-0} |
| 失败测试 | ${FAILED_TESTS:-0} |
| 通过率 | ${PASS_RATE}% |

## ✅ 质量门禁检查

$(if [ "$PASS_RATE" -ge 80 ]; then
    echo "- ✅ 通过率达标 (≥80%)"
else
    echo "- ❌ 通过率不足 (警告: ${PASS_RATE}% < 80%)"
fi)

$(if [ "$FAILED_TESTS" -eq 0 ]; then
    echo "- ✅ 零失败测试"
else
    echo "- ⚠️ 存在失败测试 (${FAILED_TESTS}个)"
fi)

## 📋 测试执行详情

### 单元测试执行状态
$(if [ -z "$UNIT_FAILED" ]; then echo "- ✅ 全部单元测试通过"; else echo "- ❌ 部分单元测试失败"; fi)

### 集成测试执行状态
$(if [ -z "$INTEGRATION_FAILED" ]; then echo "- ✅ 集成测试就绪"; else echo "- ❌ 集成测试失败"; fi)

### 性能测试执行状态
$(if [ -z "$PERFORMANCE_FAILED" ]; then echo "- ✅ 性能测试就绪"; else echo "- ❌ 性能测试失败"; fi)

### 存储引擎测试执行状态
$(if [ -z "$STORAGE_FAILED" ]; then echo "- ✅ 存储引擎测试就绪"; else echo "- ❌ 存储引擎测试失败"; fi)

### 安全测试执行状态
$(if [ -z "$SECURITY_FAILED" ]; then echo "- ✅ 安全测试就绪"; else echo "- ❌ 安全测试失败"; fi)

### 验证测试执行状态
$(if [ -z "$VALIDATE_FAILED" ]; then echo "- ✅ 验证测试就绪"; else echo "- ❌ 验证测试失败"; fi)

### 调试测试执行状态
$(if [ -z "$DEBUG_FAILED" ]; then echo "- ✅ 调试测试就绪"; else echo "- ❌ 调试测试失败"; fi)

## 📊 覆盖率分析

$(if [ "$COVERAGE_ENABLED" = true ]; then
    echo "### 覆盖率数据"
    echo "$COVERAGE_DATA"
    echo ""
    echo "### 覆盖率趋势"
    if [ -f "test_reports/coverage_trends.txt" ]; then
        echo "\`\`\`"
        tail -10 test_reports/coverage_trends.txt
        echo "\`\`\`"
    else
        echo "暂无历史覆盖率数据"
    fi
else
    echo "覆盖率测试未启用"
fi)

## 🔍 详细测试输出

\`\`\`
$ALL_TEST_OUTPUT
\`\`\`

## 📝 测试建议

$(if [ "$PASS_RATE" -lt 80 ]; then
    echo "- ⚠️ 建议提高测试通过率"
    echo "- 🔧 检查失败的测试用例"
    echo "- 📋 分析测试失败原因"
fi)

$(if [ "$COVERAGE_ENABLED" = true ] && [ -n "$COVERAGE_DATA" ]; then
    echo "- 📊 查看覆盖率报告了解代码覆盖情况"
    echo "- 🎯 针对低覆盖率模块补充测试用例"
fi)

$(if [ "$TOTAL_DURATION" -gt 300 ]; then
    echo "- ⏱️ 测试执行时间较长，建议优化"
fi)

---

报告生成时间: $(date)
测试脚本版本: v2.0 (改进版)
EOF

# 显示报告位置
echo ""
echo "========================================="
echo "✅ 测试执行完成"
echo "========================================="
echo "📄 详细报告已保存至: $REPORT_FILE"
echo "📊 测试结果概览:"
echo "   - 测试类型: $TEST_TYPE"
echo "   - 总测试数: ${TOTAL_TESTS:-0}"
echo "   - 通过测试: ${PASSED_TESTS:-0}"
echo "   - 失败测试: ${FAILED_TESTS:-0}"
echo "   - 通过率: ${PASS_RATE}%"
echo "   - 执行耗时: ${TOTAL_DURATION}秒"

if [ "$COVERAGE_ENABLED" = true ]; then
    echo "📈 覆盖率数据: 已收集"
    [ -d "coverage_html" ] && echo "   - HTML报告: coverage_html/index.html"
fi

echo ""
echo "🎯 质量门禁: $([ "$PASS_RATE" -ge 80 ] && echo "✅ 通过" || echo "⚠️  需要改进")"

# 返回适当的退出码
if [ "$PASS_RATE" -ge 80 ] && [ "${FAILED_TESTS:-0}" -eq 0 ]; then
    exit 0
else
    exit 1
fi
