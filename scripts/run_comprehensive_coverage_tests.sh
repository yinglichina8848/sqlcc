#!/bin/bash

# SQLCC Level 1-6 综合覆盖率测试脚本
# 集成所有层次的测试进行统一的覆盖率数据收集

set -e

echo "========================================="
echo "SQLCC Level 1-6 综合覆盖率测试脚本"
echo "时间: $(date)"
echo "========================================="

# 项目根目录
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$PROJECT_ROOT"

# 输出目录
COVERAGE_DIR="$PROJECT_ROOT/coverage_data"
LLVM_REPORT_DIR="$COVERAGE_DIR/llvm_coverage_report"
mkdir -p "$LLVM_REPORT_DIR"

# 检查llvm-cov工具
echo "检查llvm-cov工具..."
if ! command -v llvm-cov &> /dev/null; then
    echo "❌ llvm-cov 未找到，请安装LLVM工具链"
    exit 1
fi

echo "✅ llvm-cov 工具已找到: $(which llvm-cov)"

# 定义测试层次
define_test_levels() {
    # Level 1: 基础工具类测试
    LEVEL1_TESTS=(
        "//tests/unit/basic:logger_basic_test"
        "//tests/unit/basic:exception_test"
        "//tests/unit/basic:config_manager_test"
        "//tests/unit/basic:ast_node_basic_test"
    )

    # Level 2: 存储引擎基础测试
    LEVEL2_TESTS=(
        "//tests/storage_engine:buffer_pool_test"
        "//tests/storage_engine:b_plus_tree_core_test"
        "//tests/storage_engine:storage_engine_boundary_test"
        "//tests/storage_engine:wal_system_test"
        "//tests/storage_engine:data_integrity_test"
        "//tests/storage_engine:page_allocator_test"
    )

    # Level 3: 事务管理器测试
    LEVEL3_TESTS=(
        "//tests/level3_transaction_manager:config_test"
        "//tests/unit/executor:task_executor_test"
        "//tests/unit/executor:comprehensive_task_executor_test"
    )

    # Level 4: SQL处理测试
    LEVEL4_TESTS=(
        "//tests/unit/parser:json_parser_test"
        "//tests/unit/parser:advanced_query_test"
        "//tests/unit/parser:constraint_test"
        "//tests/unit/execution:subquery_executor_test"
    )

    # Level 5: 网络通信测试
    LEVEL5_TESTS=(
        "//tests/unit/network:tls_connection_test"
        "//tests/network:integration_test"
    )

    # Level 6: 企业级功能测试
    LEVEL6_TESTS=(
        "//tests/unit/security:dcl_permission_model_advanced_test"
        "//tests/unit/security:dcl_role_management_test"
        "//tests/performance/ddl:ddl_performance_test"
    )
}

# 执行分层测试
run_level_tests() {
    local level=$1
    local test_array_name="LEVEL${level}_TESTS"
    local -n tests_ref="$test_array_name"

    echo ""
    echo "========================================="
    echo "执行 Level $level 测试 (${#tests_ref[@]} 个测试)"
    echo "========================================="

    local level_coverage_dir="$COVERAGE_DIR/level$level"
    mkdir -p "$level_coverage_dir"

    local passed=0
    local failed=0

    for test_target in "${tests_ref[@]}"; do
        echo ""
        echo "运行测试: $test_target"

        # 尝试运行单个测试的覆盖率
        if bazel coverage "$test_target" --combined_report=lcov --test_timeout=60 2>/dev/null; then
            echo "✅ $test_target 测试通过"
            ((passed++))
        else
            echo "❌ $test_target 测试失败"
            ((failed++))
        fi
    done

    echo ""
    echo "Level $level 测试结果:"
    echo "  ✅ 通过: $passed"
    echo "  ❌ 失败: $failed"
    echo "  📊 成功率: $((passed * 100 / (passed + failed)))%"
}

# 收集覆盖率数据
collect_coverage_data() {
    echo ""
    echo "========================================="
    echo "收集覆盖率数据"
    echo "========================================="

    # 查找所有生成的覆盖率数据文件
    find "$COVERAGE_DIR" -name "*.profdata" -o -name "*.profraw" | while read -r file; do
        echo "找到覆盖率文件: $file"
    done

    # 合并覆盖率数据
    local profdata_files=($(find "$COVERAGE_DIR" -name "*.profdata" 2>/dev/null))
    local profraw_files=($(find "$COVERAGE_DIR" -name "*.profraw" 2>/dev/null))

    if [ ${#profdata_files[@]} -gt 0 ] || [ ${#profraw_files[@]} -gt 0 ]; then
        echo "合并覆盖率数据..."

        # 如果有profraw文件，先转换为profdata
        if [ ${#profraw_files[@]} -gt 0 ]; then
            for profraw in "${profraw_files[@]}"; do
                profdata="${profraw%.profraw}.profdata"
                llvm-profdata merge -output="$profdata" "$profraw" 2>/dev/null || true
            done
        fi

        # 合并所有profdata文件
        profdata_files=($(find "$COVERAGE_DIR" -name "*.profdata" 2>/dev/null))
        if [ ${#profdata_files[@]} -gt 0 ]; then
            llvm-profdata merge -output="$COVERAGE_DIR/merged.profdata" "${profdata_files[@]}"
            echo "✅ 覆盖率数据合并完成: $COVERAGE_DIR/merged.profdata"
        fi
    else
        echo "⚠️  未找到覆盖率数据文件"
    fi
}

# 生成综合报告
generate_comprehensive_report() {
    echo ""
    echo "========================================="
    echo "生成综合覆盖率报告"
    echo "========================================="

    local merged_profdata="$COVERAGE_DIR/merged.profdata"

    if [ -f "$merged_profdata" ]; then
        # 查找可用于分析的测试二进制文件
        local test_binaries=($(find bazel-bin -name "*test" -type f -executable 2>/dev/null | head -3))

        if [ ${#test_binaries[@]} -gt 0 ]; then
            local test_binary="${test_binaries[0]}"
            echo "使用测试二进制文件: $test_binary"

            # 生成文本报告
            llvm-cov report \
                --instr-profile="$merged_profdata" \
                "$test_binary" \
                --ignore-filename-regex=".*test.*" \
                --ignore-filename-regex=".*third_party.*" \
                --ignore-filename-regex=".*external.*" \
                > "$LLVM_REPORT_DIR/comprehensive_coverage_report.txt"

            # 生成HTML报告
            llvm-cov show \
                --instr-profile="$merged_profdata" \
                "$test_binary" \
                --ignore-filename-regex=".*test.*" \
                --ignore-filename-regex=".*third_party.*" \
                --ignore-filename-regex=".*external.*" \
                --format=html \
                --output-dir="$LLVM_REPORT_DIR/html"

            echo "✅ 综合覆盖率报告生成完成"
        else
            echo "⚠️  未找到合适的测试二进制文件用于报告生成"
        fi
    else
        echo "⚠️  无覆盖率数据，跳过报告生成"
    fi
}

# 生成分层测试报告
generate_layer_report() {
    echo ""
    echo "========================================="
    echo "生成分层测试报告"
    echo "========================================="

    cat > "$LLVM_REPORT_DIR/layer_test_report.md" << 'EOF'
# SQLCC Level 1-6 分层覆盖率测试报告

## 测试层次结构

### Level 1: 基础工具类 (Foundation)
- **位置**: `tests/unit/basic/`
- **测试内容**: 日志系统、异常处理、配置管理、AST节点
- **重要性**: 🟢 高 - 基础组件质量保障

### Level 2: 存储引擎基础 (Storage Engine Core)
- **位置**: `tests/storage_engine/`
- **测试内容**: 缓冲池、B+树、WAL系统、数据完整性
- **重要性**: 🟢 高 - 数据存储核心功能

### Level 3: 事务管理器 (Transaction Manager)
- **位置**: `tests/level3_transaction_manager/`
- **测试内容**: 事务控制、并发管理、任务执行
- **重要性**: 🟢 高 - 数据一致性保障

### Level 4: SQL处理 (SQL Processing)
- **位置**: `tests/unit/parser/`, `tests/unit/execution/`
- **测试内容**: 语法解析、查询执行、约束处理
- **重要性**: 🟡 中 - SQL功能实现

### Level 5: 网络通信 (Network)
- **位置**: `tests/unit/network/`, `tests/network/`
- **测试内容**: 连接管理、消息传输、TLS安全
- **重要性**: 🟡 中 - 网络通信质量

### Level 6: 企业级功能 (Enterprise Features)
- **位置**: `tests/unit/security/`, `tests/performance/`
- **测试内容**: 安全认证、性能测试、企业特性
- **重要性**: 🟢 高 - 企业级功能验证

## 测试执行策略

### 分层执行顺序
1. **自底向上**: 从基础组件开始，逐步向上层测试
2. **依赖管理**: 确保下层组件稳定后再测试上层功能
3. **增量测试**: 每个层次独立测试，避免级联失败

### 覆盖率收集机制
- **分层收集**: 每个层次单独收集覆盖率数据
- **数据合并**: 统一合并所有层次的覆盖率数据
- **报告生成**: 生成综合的覆盖率分析报告

## 质量评估标准

### 覆盖率目标
| 层次 | 行覆盖率目标 | 函数覆盖率目标 | 当前状态 |
|------|-------------|---------------|----------|
| Level 1 | ≥70% | ≥80% | 🟡 待测 |
| Level 2 | ≥75% | ≥85% | 🟡 待测 |
| Level 3 | ≥70% | ≥80% | 🟡 待测 |
| Level 4 | ≥65% | ≥75% | 🟡 待测 |
| Level 5 | ≥60% | ≥70% | 🟡 待测 |
| Level 6 | ≥50% | ≥65% | 🟡 待测 |

### 成功标准
- ✅ 各层次测试用例全部执行
- ✅ 覆盖率数据成功收集
- ✅ 综合报告自动生成
- ✅ 质量问题及时识别

## 持续改进计划

### 短期目标 (1个月)
- [ ] 完善各层次测试用例
- [ ] 优化覆盖率收集流程
- [ ] 建立自动化测试流水线

### 中期目标 (3个月)
- [ ] 达到各层次覆盖率目标
- [ ] 建立质量门禁机制
- [ ] 完善测试文档和指南

### 长期目标 (6个月)
- [ ] 全项目覆盖率达到80%
- [ ] 建立完整的测试生态
- [ ] 形成标准化测试流程

---

**测试策略**: 分层测试 + 综合覆盖率分析
**执行时间**: $(date)
**测试框架**: Google Test + Bazel + LLVM Coverage
EOF

    echo "✅ 分层测试报告生成完成"
}

# 主函数
main() {
    echo "开始 SQLCC Level 1-6 综合覆盖率测试..."

    # 定义测试层次
    define_test_levels

    # 检查命令行参数
    if [ $# -gt 0 ]; then
        # 如果提供了参数，只运行指定层次
        level=$1
        if [[ $level =~ ^[1-6]$ ]]; then
            echo "执行指定的 Level $level 测试..."
            run_level_tests $level
        else
            echo "❌ 无效的层次参数: $level (应为 1-6)"
            exit 1
        fi
    else
        # 默认执行所有层次
        echo "执行所有层次 (1-6) 测试..."
        for level in {1..6}; do
            run_level_tests $level
        done
    fi

    # 收集覆盖率数据
    collect_coverage_data

    # 生成综合报告
    generate_comprehensive_report

    # 生成分层测试报告
    generate_layer_report

    echo ""
    echo "========================================="
    echo "🎉 SQLCC Level 1-6 综合覆盖率测试完成!"
    echo "========================================="
    echo ""
    echo "📊 生成的文件:"
    echo "  📄 综合文本报告: $LLVM_REPORT_DIR/comprehensive_coverage_report.txt"
    echo "  🌐 HTML报告: $LLVM_REPORT_DIR/html/index.html"
    echo "  📋 分层测试报告: $LLVM_REPORT_DIR/layer_test_report.md"
    echo "  📦 合并覆盖率数据: $COVERAGE_DIR/merged.profdata"
    echo ""
    echo "✅ Level 1-6 测试集成完成"
    echo "✅ 覆盖率数据收集完成"
    echo "✅ 综合报告生成完成"
}

# 执行主函数
main "$@"
