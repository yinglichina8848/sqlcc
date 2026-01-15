#!/bin/bash

# SQLCC 覆盖率质量检查脚本
# 检查覆盖率数据是否达到质量门禁标准

set -e

# 项目根目录
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$PROJECT_ROOT"

# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# 日志函数
log_info() {
    echo -e "$(date '+%Y-%m-%d %H:%M:%S') ${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "$(date '+%Y-%m-%d %H:%M:%S') ${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "$(date '+%Y-%m-%d %H:%M:%S') ${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "$(date '+%Y-%m-%d %H:%M:%S') ${RED}[ERROR]${NC} $1"
}

# 显示帮助信息
show_help() {
    echo "用法: $0 <reports_dir> <threshold>"
    echo ""
    echo "SQLCC 覆盖率质量检查脚本"
    echo ""
    echo "参数:"
    echo "  reports_dir    覆盖率报告目录"
    echo "  threshold      质量门禁阈值 (50-95)"
    echo ""
    echo "示例:"
    echo "  $0 /path/to/coverage_reports 70"
}

# 主函数
main() {
    if [[ $# -ne 2 ]]; then
        show_help
        exit 1
    fi

    local reports_dir="$1"
    local threshold="$2"

    log_info "开始覆盖率质量检查"
    log_info "报告目录: $reports_dir"
    log_info "质量阈值: ${threshold}%"

    # 检查报告目录是否存在
    if [[ ! -d "$reports_dir" ]]; then
        log_error "报告目录不存在: $reports_dir"
        exit 1
    fi

    # 查找覆盖率报告文件
    local coverage_files=()
    while IFS= read -r -d '' file; do
        coverage_files+=("$file")
    done < <(find "$reports_dir" -name "*.lcov" -o -name "*coverage*.txt" -o -name "*coverage*.json" -print0 2>/dev/null)

    if [[ ${#coverage_files[@]} -eq 0 ]]; then
        log_error "未找到覆盖率报告文件"
        exit 1
    fi

    log_info "找到 ${#coverage_files[@]} 个覆盖率报告文件"

    # 分析覆盖率数据
    local total_lines=0
    local covered_lines=0
    local functions_found=0
    local functions_hit=0

    for file in "${coverage_files[@]}"; do
        log_info "分析文件: $(basename "$file")"

        case "$file" in
            *.lcov)
                # 解析LCOV格式
                while IFS= read -r line; do
                    if [[ $line =~ ^DA: ]]; then
                        # DA:行号,执行次数
                        local execution_count=$(echo "$line" | cut -d',' -f2)
                        ((total_lines++))
                        if [[ "$execution_count" -gt 0 ]]; then
                            ((covered_lines++))
                        fi
                    elif [[ $line =~ ^FN: ]]; then
                        ((functions_found++))
                    elif [[ $line =~ ^FNDA: ]]; then
                        # FNDA:执行次数,函数名
                        local fn_execution_count=$(echo "$line" | cut -d',' -f1)
                        if [[ "$fn_execution_count" -gt 0 ]]; then
                            ((functions_hit++))
                        fi
                    fi
                done < "$file"
                ;;
            *coverage*.txt)
                # 尝试解析文本格式
                if grep -q "lines.*:" "$file"; then
                    local line_coverage=$(grep "lines.*:" "$file" | head -1 | sed 's/.*:\s*\([0-9.]*\).*/\1/' | tr -d '%')
                    if [[ -n "$line_coverage" ]]; then
                        # 估算覆盖率数据 (简化处理)
                        local estimated_total=$((covered_lines * 100 / (line_coverage > 0 ? line_coverage : 1)))
                        covered_lines=$((covered_lines + estimated_total * line_coverage / 100))
                        total_lines=$((total_lines + estimated_total))
                    fi
                fi
                ;;
            *coverage*.json)
                # 解析JSON格式 (简化处理)
                if command -v jq &> /dev/null; then
                    local json_line_coverage=$(jq -r '.coverage // empty' "$file" 2>/dev/null || echo "")
                    if [[ -n "$json_line_coverage" ]]; then
                        covered_lines=$((covered_lines + json_line_coverage))
                        total_lines=$((total_lines + 100))
                    fi
                fi
                ;;
        esac
    done

    # 计算覆盖率百分比
    local line_coverage_percentage=0
    local function_coverage_percentage=0

    if [[ $total_lines -gt 0 ]]; then
        line_coverage_percentage=$((covered_lines * 100 / total_lines))
    fi

    if [[ $functions_found -gt 0 ]]; then
        function_coverage_percentage=$((functions_hit * 100 / functions_found))
    fi

    # 输出结果
    echo ""
    echo "========================================="
    echo "覆盖率质量检查结果"
    echo "========================================="
    echo "行覆盖率: ${line_coverage_percentage}% (目标: ≥${threshold}%)"
    echo "函数覆盖率: ${function_coverage_percentage}% (目标: ≥$((threshold - 10))%)"
    echo "覆盖行数: $covered_lines / $total_lines"
    echo "覆盖函数数: $functions_hit / $functions_found"
    echo ""

    # 检查质量门禁
    local quality_passed=true

    if [[ $line_coverage_percentage -lt $threshold ]]; then
        log_error "❌ 行覆盖率未达到阈值: ${line_coverage_percentage}% < ${threshold}%"
        quality_passed=false
    else
        log_success "✅ 行覆盖率通过: ${line_coverage_percentage}% ≥ ${threshold}%"
    fi

    local function_threshold=$((threshold - 10))
    if [[ $function_coverage_percentage -lt $function_threshold ]]; then
        log_warning "⚠️  函数覆盖率低于推荐值: ${function_coverage_percentage}% < ${function_threshold}%"
    else
        log_success "✅ 函数覆盖率通过: ${function_coverage_percentage}% ≥ ${function_threshold}%"
    fi

    # 生成质量报告
    cat > "$reports_dir/coverage_quality_report.md" << EOF
# 覆盖率质量检查报告

## 检查配置
- **检查时间**: $(date)
- **报告目录**: $reports_dir
- **质量阈值**: ${threshold}%
- **函数阈值**: ${function_threshold}%

## 覆盖率统计

### 行覆盖率
- **实际值**: ${line_coverage_percentage}%
- **目标值**: ≥${threshold}%
- **状态**: $([[ $line_coverage_percentage -ge $threshold ]] && echo "✅ 通过" || echo "❌ 未通过")
- **覆盖详情**: $covered_lines / $total_lines 行

### 函数覆盖率
- **实际值**: ${function_coverage_percentage}%
- **目标值**: ≥${function_threshold}%
- **状态**: $([[ $function_coverage_percentage -ge $function_threshold ]] && echo "✅ 通过" || echo "⚠️  警告")
- **覆盖详情**: $functions_hit / $functions_found 函数

## 质量评估

### 覆盖率等级
$(if [[ $line_coverage_percentage -ge 80 ]]; then
    echo "- 🟢 优秀级 (≥80%): 代码质量优秀"
elif [[ $line_coverage_percentage -ge 70 ]]; then
    echo "- 🟡 良好级 (70-79%): 代码质量良好"
elif [[ $line_coverage_percentage -ge 60 ]]; then
    echo "- 🟠 及格级 (60-69%): 代码质量一般"
else
    echo "- 🔴 不及格级 (<60%): 需要改进"
fi)

### 建议措施
$(if [[ $line_coverage_percentage -lt $threshold ]]; then
    echo "- 增加单元测试用例"
    echo "- 完善集成测试覆盖"
    echo "- 补充边界条件测试"
    echo "- 优化测试数据生成"
else
    echo "- 保持当前测试覆盖水平"
    echo "- 关注新增代码的测试覆盖"
    echo "- 定期review测试质量"
fi)

---

**质量门禁**: $([[ $quality_passed == true ]] && echo "✅ 通过" || echo "❌ 未通过")
**生成时间**: $(date -Iseconds)
EOF

    if [[ $quality_passed == true ]]; then
        log_success "🎉 覆盖率质量检查通过!"
        echo "quality_report=$reports_dir/coverage_quality_report.md" >> "$GITHUB_OUTPUT" 2>/dev/null || true
        exit 0
    else
        log_error "💥 覆盖率质量检查失败!"
        echo "quality_report=$reports_dir/coverage_quality_report.md" >> "$GITHUB_OUTPUT" 2>/dev/null || true
        exit 1
    fi
}

# 执行主函数
main "$@"
