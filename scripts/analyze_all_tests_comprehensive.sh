#!/bin/bash
# SQLCC测试体系全面分析脚本
# 分析所有测试文件，判断必要性，制定保留/删除决策

echo "=== SQLCC测试体系全面分析 ==="

# 输出文件
COMPREHENSIVE_ANALYSIS="comprehensive_test_analysis.txt"
NECESSITY_DECISIONS="test_necessity_decisions.txt"
CLEANUP_PLAN="test_cleanup_plan.md"

# 清空输出文件
> "$COMPREHENSIVE_ANALYSIS"
> "$NECESSITY_DECISIONS"
> "$CLEANUP_PLAN"

echo "分析时间: $(date)" >> "$COMPREHENSIVE_ANALYSIS"
echo "分析时间: $(date)" >> "$NECESSITY_DECISIONS"
echo "分析时间: $(date)" >> "$CLEANUP_PLAN"

echo "" >> "$COMPREHENSIVE_ANALYSIS"
echo "" >> "$NECESSITY_DECISIONS"
echo "" >> "$CLEANUP_PLAN"

# 统计变量
total_files=0
cpp_files=0
build_files=0
script_files=0
obsolete_files=0
necessary_files=0

# 分析标准定义
declare -A NECESSITY_CRITERIA
NECESSITY_CRITERIA["KEEP_CORE"]="核心功能测试，验证基础组件"
NECESSITY_CRITERIA["KEEP_COVERAGE"]="覆盖率测试，支持质量度量"
NECESSITY_CRITERIA["KEEP_INTEGRATION"]="集成测试，验证组件协作"
NECESSITY_CRITERIA["KEEP_PERFORMANCE"]="性能测试，验证系统性能"
NECESSITY_CRITERIA["REVIEW_DUPLICATE"]="重复测试，需要去重"
NECESSITY_CRITERIA["REVIEW_OUTDATED"]="过时测试，可能已无用"
NECESSITY_CRITERIA["REMOVE_DEBUG"]="调试专用，可删除"
NECESSITY_CRITERIA["REMOVE_OBSOLETE"]="废弃文件，确认删除"

# 分析函数
analyze_file_necessity() {
    local file_path="$1"
    local file_name=$(basename "$file_path")
    local file_dir=$(dirname "$file_path")
    local relative_path="${file_path#tests/}"

    ((total_files++))

    # 基本信息
    echo "=== 分析文件: $relative_path ===" >> "$COMPREHENSIVE_ANALYSIS"
    echo "完整路径: $file_path" >> "$COMPREHENSIVE_ANALYSIS"
    echo "文件大小: $(stat -c%s "$file_path" 2>/dev/null || echo "N/A") bytes" >> "$COMPREHENSIVE_ANALYSIS"
    echo "修改时间: $(stat -c%y "$file_path" 2>/dev/null || echo "N/A")" >> "$COMPREHENSIVE_ANALYSIS"

    # 文件类型判断
    local file_type="UNKNOWN"
    local necessity="REVIEW"
    local reason="需要人工判断"

    case "$file_name" in
        *.cpp|*.cc)
            file_type="C++测试文件"
            ((cpp_files++))
            necessity=$(analyze_cpp_test "$file_path")
            ;;
        BUILD.bazel)
            file_type="Bazel构建配置"
            ((build_files++))
            necessity="KEEP_BUILD"
            reason="构建系统必需"
            ;;
        *.sh)
            file_type="Shell脚本"
            ((script_files++))
            necessity=$(analyze_script "$file_path")
            ;;
        *.py)
            file_type="Python脚本"
            necessity=$(analyze_python "$file_path")
            ;;
        *.txt|*.md)
            file_type="文档/日志"
            necessity=$(analyze_document "$file_path")
            ;;
        *)
            file_type="其他文件"
            necessity="REVIEW"
            reason="未知类型，需要人工判断"
            ;;
    esac

    # 记录分析结果
    echo "文件类型: $file_type" >> "$COMPREHENSIVE_ANALYSIS"
    echo "必要性评估: $necessity" >> "$COMPREHENSIVE_ANALYSIS"
    echo "判断理由: $reason" >> "$COMPREHENSIVE_ANALYSIS"
    echo "" >> "$COMPREHENSIVE_ANALYSIS"

    # 记录决策
    echo "$relative_path|$file_type|$necessity|$reason" >> "$NECESSITY_DECISIONS"

    # 更新统计
    case "$necessity" in
        KEEP*|BUILD*)
            ((necessary_files++))
            ;;
        REMOVE*)
            ((obsolete_files++))
            ;;
    esac
}

# 分析C++测试文件
analyze_cpp_test() {
    local file_path="$1"
    local file_name=$(basename "$file_path")

    # 检查是否包含gtest
    if grep -q "gtest\|GTEST" "$file_path" 2>/dev/null; then
        # 检查功能类型
        if grep -q "PERF\|PERFORMANCE\|BENCHMARK" "$file_path" 2>/dev/null; then
            reason="性能测试，验证系统性能指标"
            echo "KEEP_PERFORMANCE"
            return
        elif grep -q "INTEGRATION\|E2E\|END_TO_END" "$file_path" 2>/dev/null; then
            reason="集成测试，验证组件间协作"
            echo "KEEP_INTEGRATION"
            return
        elif [[ "$file_name" == *coverage* ]] || grep -q "COVERAGE" "$file_path" 2>/dev/null; then
            reason="覆盖率测试，支持质量度量"
            echo "KEEP_COVERAGE"
            return
        elif [[ "$file_name" == *debug* ]] || [[ "$file_name" == *test_* ]] && grep -q "DEBUG\|debug" "$file_path" 2>/dev/null; then
            reason="调试专用测试，可考虑删除"
            echo "REMOVE_DEBUG"
            return
        else
            reason="核心功能测试，验证基础组件"
            echo "KEEP_CORE"
            return
        fi
    else
        # 非gtest文件
        if [[ "$file_name" == *stub* ]] || [[ "$file_name" == *mock* ]]; then
            reason="存根/模拟文件，支持测试"
            echo "KEEP_CORE"
            return
        elif [[ "$file_name" == *demo* ]] || [[ "$file_name" == *example* ]]; then
            reason="演示文件，可能需要保留"
            echo "REVIEW_OUTDATED"
            return
        else
            reason="非测试文件，可能是遗留代码"
            echo "REVIEW_OBSOLETE"
            return
        fi
    fi
}

# 分析脚本文件
analyze_script() {
    local file_path="$1"
    local file_name=$(basename "$file_path")

    if [[ "$file_name" == *test* ]] || [[ "$file_name" == *run* ]] || [[ "$file_name" == *build* ]]; then
        reason="测试运行脚本，构建系统必需"
        echo "KEEP_BUILD"
        return
    elif [[ "$file_name" == *clean* ]] || [[ "$file_name" == *setup* ]]; then
        reason="维护脚本，可能需要保留"
        echo "REVIEW_OUTDATED"
        return
    else
        reason="未知用途脚本，需要人工判断"
        echo "REVIEW"
        return
    fi
}

# 分析Python文件
analyze_python() {
    local file_path="$1"

    if grep -q "unittest\|pytest\|test" "$file_path" 2>/dev/null; then
        reason="Python测试脚本"
        echo "KEEP_CORE"
        return
    else
        reason="Python工具脚本"
        echo "REVIEW"
        return
    fi
}

# 分析文档文件
analyze_document() {
    local file_path="$1"
    local file_name=$(basename "$file_path")

    if [[ "$file_name" == *report* ]] || [[ "$file_name" == *analysis* ]]; then
        reason="分析报告文档，可能需要保留"
        echo "REVIEW_OUTDATED"
        return
    elif [[ "$file_name" == *log* ]]; then
        reason="日志文件，可删除"
        echo "REMOVE_OBSOLETE"
        return
    else
        reason="文档文件，需要人工判断"
        echo "REVIEW"
        return
    fi
}

# 扫描所有文件
echo "开始扫描tests目录下的所有文件..."
find tests -type f | sort | while read -r file; do
    analyze_file_necessity "$file"
done

# 生成统计报告
echo "=== 分析统计 ===" >> "$COMPREHENSIVE_ANALYSIS"
echo "总文件数: $total_files" >> "$COMPREHENSIVE_ANALYSIS"
echo "C++文件数: $cpp_files" >> "$COMPREHENSIVE_ANALYSIS"
echo "构建文件数: $build_files" >> "$COMPREHENSIVE_ANALYSIS"
echo "脚本文件数: $script_files" >> "$COMPREHENSIVE_ANALYSIS"
echo "必需文件数: $necessary_files" >> "$COMPREHENSIVE_ANALYSIS"
echo "可删除文件数: $obsolete_files" >> "$COMPREHENSIVE_ANALYSIS"

# 生成清理计划
echo "# SQLCC测试体系清理计划" >> "$CLEANUP_PLAN"
echo "" >> "$CLEANUP_PLAN"
echo "## 统计概览" >> "$CLEANUP_PLAN"
echo "- 总文件数: $total_files" >> "$CLEANUP_PLAN"
echo "- C++测试文件: $cpp_files" >> "$CLEANUP_PLAN"
echo "- 构建配置文件: $build_files" >> "$CLEANUP_PLAN"
echo "- 脚本文件: $script_files" >> "$CLEANUP_PLAN"
echo "- 必需保留文件: $necessary_files" >> "$CLEANUP_PLAN"
echo "- 可清理文件: $obsolete_files" >> "$CLEANUP_PLAN"
echo "" >> "$CLEANUP_PLAN"

echo "## 清理策略" >> "$CLEANUP_PLAN"
echo "1. **保留文件**: KEEP_* 标记的文件必须保留" >> "$CLEANUP_PLAN"
echo "2. **审查文件**: REVIEW 标记的文件需要人工判断" >> "$CLEANUP_PLAN"
echo "3. **删除文件**: REMOVE_* 标记的文件可以删除" >> "$CLEANUP_PLAN"
echo "4. **备份策略**: 删除前备份到 archive/ 目录" >> "$CLEANUP_PLAN"
echo "" >> "$CLEANUP_PLAN"

echo "## 具体清理清单" >> "$CLEANUP_PLAN"

# 按类型生成清理清单
echo "### 可直接删除的文件 (REMOVE_*)" >> "$CLEANUP_PLAN"
grep "|REMOVE_" "$NECESSITY_DECISIONS" | while IFS='|' read -r path type necessity reason; do
    echo "- [ ] \`$path\` - $reason" >> "$CLEANUP_PLAN"
done
echo "" >> "$CLEANUP_PLAN"

echo "### 需要人工审查的文件 (REVIEW*)" >> "$CLEANUP_PLAN"
grep "|REVIEW" "$NECESSITY_DECISIONS" | while IFS='|' read -r path type necessity reason; do
    echo "- [ ] \`$path\` - $reason" >> "$CLEANUP_PLAN"
done
echo "" >> "$CLEANUP_PLAN"

echo "### 必需保留的文件 (KEEP_*)" >> "$CLEANUP_PLAN"
echo "*(这些文件将被纳入新的测试体系)*" >> "$CLEANUP_PLAN"
grep "|KEEP_" "$NECESSITY_DECISIONS" | while IFS='|' read -r path type necessity reason; do
    echo "- [x] \`$path\` - $reason" >> "$CLEANUP_PLAN"
done
echo "" >> "$CLEANUP_PLAN"

echo "## 执行计划" >> "$CLEANUP_PLAN"
echo "1. **第一阶段**: 直接删除REMOVE_*文件" >> "$CLEANUP_PLAN"
echo "2. **第二阶段**: 审查REVIEW文件，决定去留" >> "$CLEANUP_PLAN"
echo "3. **第三阶段**: 整理KEEP文件到新层次结构" >> "$CLEANUP_PLAN"
echo "4. **验证阶段**: 确保清理后系统仍能正常工作" >> "$CLEANUP_PLAN"
echo "" >> "$CLEANUP_PLAN"

echo "## 备份策略" >> "$CLEANUP_PLAN"
echo "1. 创建 \`tests/archive/\` 目录" >> "$CLEANUP_PLAN"
echo "2. 按删除日期创建子目录，如 \`archive/2026-01-12/\`" >> "$CLEANUP_PLAN"
echo "3. 保留文件元信息（大小、修改时间、依赖关系）" >> "$CLEANUP_PLAN"
echo "4. 生成清理报告，记录所有删除操作" >> "$CLEANUP_PLAN"

echo "分析完成!"
echo "结果已保存到:"
echo "  - $COMPREHENSIVE_ANALYSIS (详细分析)"
echo "  - $NECESSITY_DECISIONS (决策清单)"
echo "  - $CLEANUP_PLAN (清理计划)"