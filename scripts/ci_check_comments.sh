#!/bin/bash

# SQLCC注释质量CI/CD检查脚本
# 用于在持续集成中自动检查注释质量

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 配置变量
CONFIG_FILE="${PROJECT_ROOT}/tools/comment_quality_config.yaml"
OUTPUT_DIR="${PROJECT_ROOT}/ci_reports"
COMMENT_QUALITY_THRESHOLD=80
WARNING_THRESHOLD=90

# 创建输出目录
mkdir -p "$OUTPUT_DIR"

# 日志函数
log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# 检查依赖
check_dependencies() {
    if ! command -v python3 &> /dev/null; then
        log_error "Python3 未找到，请安装 Python3"
        exit 1
    fi

    if [[ ! -f "${PROJECT_ROOT}/tools/comment_quality_analyzer.py" ]]; then
        log_error "注释质量分析工具未找到"
        exit 1
    fi

    if [[ ! -f "$CONFIG_FILE" ]]; then
        log_warning "配置文件未找到，使用默认配置: $CONFIG_FILE"
    fi
}

# 运行注释质量检查
run_comment_quality_check() {
    local output_file="${OUTPUT_DIR}/comment_quality_report_$(date +%Y%m%d_%H%M%S).md"
    local exit_code=0

    log_info "开始注释质量检查..."
    log_info "输出文件: $output_file"

    # 构建命令
    local cmd="python3 ${PROJECT_ROOT}/tools/comment_quality_analyzer.py"
    cmd="$cmd ${PROJECT_ROOT}/include"
    cmd="$cmd --pattern \"*.h\""
    cmd="$cmd --output \"$output_file\""

    if [[ -f "$CONFIG_FILE" ]]; then
        cmd="$cmd --config \"$CONFIG_FILE\""
    fi

    # 执行命令
    log_info "执行命令: $cmd"
    if eval "$cmd"; then
        log_success "注释质量检查完成"
    else
        exit_code=$?
        log_error "注释质量检查失败 (退出码: $exit_code)"
    fi

    # 分析结果
    if [[ -f "$output_file" ]]; then
        analyze_results "$output_file"
        local analysis_exit=$?

        if [[ $analysis_exit -ne 0 ]]; then
            exit_code=$analysis_exit
        fi
    else
        log_error "输出文件未生成: $output_file"
        exit_code=1
    fi

    return $exit_code
}

# 分析检查结果
analyze_results() {
    local report_file="$1"
    local exit_code=0

    log_info "分析检查结果..."

    # 读取报告内容
    local content
    content=$(cat "$report_file")

    # 提取统计信息
    local total_files=$(echo "$content" | grep -o "总文件数: [0-9]*" | grep -o "[0-9]*" | head -1)
    local passed_files=$(echo "$content" | grep -o "通过文件: [0-9]*" | grep -o "[0-9]*" | head -1)
    local failed_files=$(echo "$content" | grep -o "失败文件: [0-9]*" | grep -o "[0-9]*" | head -1)
    local avg_score=$(echo "$content" | grep -o "平均评分: [0-9.]*" | grep -o "[0-9.]*" | head -1)

    # 输出统计信息
    log_info "=== 检查结果统计 ==="
    log_info "总文件数: $total_files"
    log_info "通过文件: $passed_files"
    log_info "失败文件: $failed_files"
    log_info "平均评分: $avg_score"

    # 检查阈值
    if [[ -n "$avg_score" ]] && [[ $(echo "$avg_score < $COMMENT_QUALITY_THRESHOLD" | bc -l 2>/dev/null || echo "false") == "1" ]]; then
        log_error "注释质量评分过低: $avg_score < $COMMENT_QUALITY_THRESHOLD"
        log_error "请修复注释质量问题"
        exit_code=1
    elif [[ -n "$avg_score" ]] && [[ $(echo "$avg_score < $WARNING_THRESHOLD" | bc -l 2>/dev/null || echo "false") == "1" ]]; then
        log_warning "注释质量评分较低: $avg_score < $WARNING_THRESHOLD"
        log_warning "建议改进注释质量"
    else
        log_success "注释质量检查通过"
    fi

    # 检查失败文件
    if [[ "$failed_files" -gt 0 ]]; then
        log_warning "发现 $failed_files 个文件注释质量不合格"

        # 提取失败的文件列表
        echo "$content" | awk '
        /^### ❌/ {
            getline next_line
            if (next_line ~ /总体评分/) {
                split($0, parts, " ")
                file = parts[3]
                getline score_line
                split(score_line, score_parts, " ")
                score = score_parts[3]
                print file " (评分: " score ")"
            }
        }
        ' | while read -r failed_file; do
            log_warning "失败文件: $failed_file"
        done
    fi

    return $exit_code
}

# 生成摘要报告
generate_summary() {
    local timestamp=$(date +%Y%m%d_%H%M%S)
    local summary_file="${OUTPUT_DIR}/ci_summary_${timestamp}.txt"

    log_info "生成CI摘要报告: $summary_file"

    cat > "$summary_file" << EOF
SQLCC 注释质量 CI 检查摘要
生成时间: $(date)
检查结果: $([[ $? -eq 0 ]] && echo "通过" || echo "失败")

详细报告位置: ${OUTPUT_DIR}/comment_quality_report_*.md
配置文件: $CONFIG_FILE

检查阈值:
- 失败阈值: $COMMENT_QUALITY_THRESHOLD
- 警告阈值: $WARNING_THRESHOLD

如有问题，请查看详细报告并修复相关文件的注释。
EOF

    log_info "摘要报告已生成: $summary_file"
}

# 设置GitHub Actions输出
set_github_output() {
    if [[ -n "$GITHUB_OUTPUT" ]]; then
        echo "comment_quality_report=${OUTPUT_DIR}/comment_quality_report_*.md" >> "$GITHUB_OUTPUT"
        echo "comment_quality_summary=${OUTPUT_DIR}/ci_summary_*.txt" >> "$GITHUB_OUTPUT"
    fi
}

# 主函数
main() {
    log_info "开始SQLCC注释质量CI检查"
    log_info "项目根目录: $PROJECT_ROOT"
    log_info "输出目录: $OUTPUT_DIR"

    # 检查依赖
    check_dependencies

    # 运行检查
    if run_comment_quality_check; then
        log_success "✅ 注释质量检查通过"
        generate_summary
        set_github_output
        exit 0
    else
        log_error "❌ 注释质量检查失败"
        generate_summary
        set_github_output
        exit 1
    fi
}

# 参数处理
case "${1:-}" in
    --help|-h)
        echo "SQLCC注释质量CI检查脚本"
        echo
        echo "用法: $0 [选项]"
        echo
        echo "选项:"
        echo "  --help, -h        显示帮助信息"
        echo "  --threshold <num> 设置失败阈值 (默认: $COMMENT_QUALITY_THRESHOLD)"
        echo "  --warning <num>   设置警告阈值 (默认: $WARNING_THRESHOLD)"
        echo
        echo "环境变量:"
        echo "  COMMENT_QUALITY_THRESHOLD  失败阈值"
        echo "  COMMENT_QUALITY_WARNING    警告阈值"
        exit 0
        ;;
    --threshold)
        COMMENT_QUALITY_THRESHOLD="$2"
        shift 2
        main "$@"
        ;;
    --warning)
        WARNING_THRESHOLD="$2"
        shift 2
        main "$@"
        ;;
    *)
        # 从环境变量读取配置
        if [[ -n "$COMMENT_QUALITY_THRESHOLD" ]]; then
            COMMENT_QUALITY_THRESHOLD="$COMMENT_QUALITY_THRESHOLD"
        fi
        if [[ -n "$COMMENT_QUALITY_WARNING" ]]; then
            WARNING_THRESHOLD="$COMMENT_QUALITY_WARNING"
        fi

        main "$@"
        ;;
esac
