#!/bin/bash

# SQLCC 注释质量检查脚本
# 检查核心组件的注释覆盖率和质量

echo "🔍 SQLCC 核心代码注释质量检查"
echo "================================="

# 检查文件是否存在
check_files() {
    local files=(
        "include/storage_engine.h"
        "include/storage/buffer_pool_sharded.h"
        "include/storage/b_plus_tree.h"
        "include/sql_parser/parser.h"
        "include/sql_executor.h"
        "include/core/core_database_manager.h"
    )

    local missing_files=()
    for file in "${files[@]}"; do
        if [[ ! -f "$file" ]]; then
            missing_files+=("$file")
        fi
    done

    if [[ ${#missing_files[@]} -gt 0 ]]; then
        echo "❌ 缺少以下文件:"
        printf '  - %s\n' "${missing_files[@]}"
        return 1
    fi

    echo "✅ 所有核心文件都存在"
    return 0
}

# 检查注释覆盖率
check_comment_coverage() {
    echo ""
    echo "📊 注释覆盖率分析:"

    local total_lines=0
    local comment_lines=0

    # 检查主要头文件
    local files=(
        "include/storage_engine.h"
        "include/storage/buffer_pool_sharded.h"
        "include/storage/b_plus_tree.h"
        "include/sql_parser/parser.h"
        "include/sql_executor.h"
        "include/core/core_database_manager.h"
    )

    for file in "${files[@]}"; do
        if [[ -f "$file" ]]; then
            local file_lines=$(wc -l < "$file")
            local file_comments=$(grep -c "^[[:space:]]*//\|^[[:space:]]*/\*" "$file")
            local coverage=$((file_comments * 100 / file_lines))

            echo "  $file: $file_comments/$file_lines 行注释 (${coverage}%)"

            total_lines=$((total_lines + file_lines))
            comment_lines=$((comment_lines + file_comments))
        fi
    done

    if [[ $total_lines -gt 0 ]]; then
        local overall_coverage=$((comment_lines * 100 / total_lines))
        echo ""
        echo "📈 总体注释覆盖率: ${overall_coverage}% ($comment_lines/$total_lines)"

        if [[ $overall_coverage -ge 30 ]]; then
            echo "✅ 注释覆盖率达标 (≥30%)"
        else
            echo "⚠️  注释覆盖率不足 (目标: ≥30%)"
        fi
    fi
}

# 检查Why/What/How注释结构
check_comment_structure() {
    echo ""
    echo "🏗️  Why/What/How注释结构检查:"

    local files=(
        "include/storage_engine.h"
        "include/storage/buffer_pool_sharded.h"
        "include/storage/b_plus_tree.h"
        "include/sql_parser/parser.h"
        "include/sql_executor.h"
        "include/core/core_database_manager.h"
    )

    local has_why=0
    local has_what=0
    local has_how=0

    for file in "${files[@]}"; do
        if [[ -f "$file" ]]; then
            if grep -q "WHY:" "$file"; then
                has_why=$((has_why + 1))
            fi
            if grep -q "WHAT:" "$file"; then
                has_what=$((has_what + 1))
            fi
            if grep -q "HOW:" "$file"; then
                has_how=$((has_how + 1))
            fi
        fi
    done

    echo "  WHY层注释: $has_why/${#files[@]} 个文件"
    echo "  WHAT层注释: $has_what/${#files[@]} 个文件"
    echo "  HOW层注释: $has_how/${#files[@]} 个文件"

    if [[ $has_why -eq ${#files[@]} && $has_what -eq ${#files[@]} && $has_how -ge $((${#files[@]} / 2)) ]]; then
        echo "✅ Why/What/How结构完整"
    else
        echo "⚠️  Why/What/How结构需要完善"
    fi
}

# 检查性能优化注释
check_performance_comments() {
    echo ""
    echo "⚡ 性能优化注释检查:"

    local perf_keywords=("性能\|优化\|并发\|复杂度\|O(")
    local perf_files=0
    local total_files=6

    local files=(
        "include/storage_engine.h"
        "include/storage/buffer_pool_sharded.h"
        "include/storage/b_plus_tree.h"
        "include/sql_parser/parser.h"
        "include/sql_executor.h"
        "include/core/core_database_manager.h"
    )

    for file in "${files[@]}"; do
        if [[ -f "$file" ]]; then
            local has_perf=0
            for keyword in "${perf_keywords[@]}"; do
                if grep -q "$keyword" "$file"; then
                    has_perf=1
                    break
                fi
            done
            if [[ $has_perf -eq 1 ]]; then
                perf_files=$((perf_files + 1))
            fi
        fi
    done

    echo "  包含性能分析的文件: $perf_files/$total_files"

    if [[ $perf_files -ge 3 ]]; then
        echo "✅ 性能优化注释充足"
    else
        echo "⚠️  需要增加性能优化注释"
    fi
}

# 主函数
main() {
    echo "开始检查 SQLCC 核心代码注释质量..."
    echo ""

    if ! check_files; then
        exit 1
    fi

    check_comment_coverage
    check_comment_structure
    check_performance_comments

    echo ""
    echo "🎯 检查完成"
    echo ""
    echo "📋 改进建议:"
    echo "  1. 确保所有核心方法都有Why/What/How注释"
    echo "  2. 添加更多性能优化和复杂度分析"
    echo "  3. 完善设计模式和架构决策说明"
    echo "  4. 建立注释维护机制"
}

# 执行主函数
main "$@"</content>
