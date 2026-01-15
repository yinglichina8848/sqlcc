#!/bin/bash

# SQLCC 统一覆盖率测试脚本
# 整合所有覆盖率相关脚本，提供统一的测试和分析接口

set -e

# 项目根目录
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$PROJECT_ROOT"

# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
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

log_step() {
    echo -e "$(date '+%Y-%m-%d %H:%M:%S') ${CYAN}[STEP]${NC} $1"
}

log_section() {
    echo -e "$(date '+%Y-%m-%d %H:%M:%S') ${PURPLE}[SECTION]${NC} $1"
}

# 显示帮助信息
show_help() {
    echo "用法: $0 [选项]"
    echo ""
    echo "SQLCC 统一覆盖率测试脚本 - 整合所有覆盖率相关功能"
    echo ""
    echo "选项:"
    echo "  -m, --mode MODE       测试模式 (basic|comprehensive|incremental|performance|security)"
    echo "  -f, --format FORMAT   输出格式 (html|lcov|json|text|all)"
    echo "  -d, --depth DEPTH     分析深度 (file|function|module|component|system)"
    echo "  -t, --threshold PCT   质量门禁阈值 (50-95)"
    echo "  -o, --output DIR      指定输出目录"
    echo "  -s, --step STEP       执行特定步骤"
    echo "  -l, --list-steps      列出所有可用步骤"
    echo "  -v, --verbose         详细输出"
    echo "  -h, --help            显示帮助信息"
    echo ""
    echo "可用步骤:"
    echo "  setup                 环境设置和依赖检查"
    echo "  data_collection       覆盖率数据收集"
    echo "  analysis              覆盖率分析"
    echo "  reporting             报告生成"
    echo "  quality_gate          质量门禁检查"
    echo "  cleanup               清理临时文件"
    echo ""
    echo "示例:"
    echo "  ./scripts/run_unified_coverage.sh --mode comprehensive --format html"
    echo "  ./scripts/run_unified_coverage.sh --step data_collection --verbose"
    echo "  ./scripts/run_unified_coverage.sh --list-steps"
}

# 解析命令行参数
parse_args() {
    MODE="basic"
    FORMAT="html"
    DEPTH="module"
    THRESHOLD="70"
    OUTPUT_DIR=""
    SPECIFIC_STEP=""
    VERBOSE=false
    LIST_STEPS=false

    while [[ $# -gt 0 ]]; do
        case $1 in
            -m|--mode)
                MODE="$2"
                shift 2
                ;;
            -f|--format)
                FORMAT="$2"
                shift 2
                ;;
            -d|--depth)
                DEPTH="$2"
                shift 2
                ;;
            -t|--threshold)
                THRESHOLD="$2"
                shift 2
                ;;
            -o|--output)
                OUTPUT_DIR="$2"
                shift 2
                ;;
            -s|--step)
                SPECIFIC_STEP="$2"
                shift 2
                ;;
            -l|--list-steps)
                LIST_STEPS=true
                shift
                ;;
            -v|--verbose)
                VERBOSE=true
                shift
                ;;
            -h|--help)
                show_help
                exit 0
                ;;
            *)
                log_error "未知选项: $1"
                show_help
                exit 1
                ;;
        esac
    done

    # 设置输出目录
    if [[ -z "$OUTPUT_DIR" ]]; then
        OUTPUT_DIR="$PROJECT_ROOT/coverage_results_$(date +%Y%m%d_%H%M%S)"
    fi

    # 验证参数
    validate_args
}

# 验证参数
validate_args() {
    # 验证模式
    case "$MODE" in
        basic|comprehensive|incremental|performance|security)
            ;;
        *)
            log_error "无效的模式: $MODE (应为 basic|comprehensive|incremental|performance|security)"
            exit 1
            ;;
    esac

    # 验证格式
    case "$FORMAT" in
        html|lcov|json|text|all)
            ;;
        *)
            log_error "无效的格式: $FORMAT (应为 html|lcov|json|text|all)"
            exit 1
            ;;
    esac

    # 验证深度
    case "$DEPTH" in
        file|function|module|component|system)
            ;;
        *)
            log_error "无效的深度: $DEPTH (应为 file|function|module|component|system)"
            exit 1
            ;;
    esac

    # 验证阈值
    if [[ ! "$THRESHOLD" =~ ^[0-9]+$ ]] || [[ "$THRESHOLD" -lt 50 ]] || [[ "$THRESHOLD" -gt 95 ]]; then
        log_error "无效的阈值: $THRESHOLD (应为 50-95)"
        exit 1
    fi
}

# 环境设置和依赖检查
setup_environment() {
    log_section "环境设置和依赖检查"

    # 检查必要工具
    check_dependencies

    # 创建输出目录
    create_directories

    # 初始化测试状态跟踪
    init_test_tracking

    log_success "环境设置完成"
}

# 检查依赖工具
check_dependencies() {
    log_step "检查依赖工具"

    local missing_tools=()

    # 检查编译工具
    if ! command -v bazel &> /dev/null; then
        missing_tools+=("bazel")
    fi

    # 检查覆盖率工具
    if ! command -v llvm-cov &> /dev/null; then
        missing_tools+=("llvm-cov")
    fi

    if ! command -v llvm-profdata &> /dev/null; then
        missing_tools+=("llvm-profdata")
    fi

    # 检查Python
    if ! command -v python3 &> /dev/null; then
        missing_tools+=("python3")
    fi

    # 检查genhtml (lcov)
    if ! command -v genhtml &> /dev/null && ! command -v lcov &> /dev/null; then
        missing_tools+=("lcov/genhtml")
    fi

    if [[ ${#missing_tools[@]} -gt 0 ]]; then
        log_error "缺少必要的工具: ${missing_tools[*]}"
        log_error "请安装必要的开发工具后重试"
        exit 1
    fi

    log_success "所有依赖工具都已安装"
}

# 创建目录结构
create_directories() {
    log_step "创建目录结构"

    mkdir -p "$OUTPUT_DIR"
    mkdir -p "$OUTPUT_DIR/coverage_data"
    mkdir -p "$OUTPUT_DIR/coverage_reports"
    mkdir -p "$OUTPUT_DIR/logs"
    mkdir -p "$OUTPUT_DIR/temp"

    log_success "目录结构创建完成: $OUTPUT_DIR"
}

# 初始化测试状态跟踪
init_test_tracking() {
    log_step "初始化测试状态跟踪"

    # 创建测试状态文件
    cat > "$OUTPUT_DIR/test_status.json" << EOF
{
  "start_time": "$(date -Iseconds)",
  "mode": "$MODE",
  "format": "$FORMAT",
  "depth": "$DEPTH",
  "threshold": "$THRESHOLD",
  "steps": {
    "setup": {"status": "completed", "start_time": "$(date -Iseconds)", "end_time": "$(date -Iseconds)"},
    "data_collection": {"status": "pending"},
    "analysis": {"status": "pending"},
    "reporting": {"status": "pending"},
    "quality_gate": {"status": "pending"},
    "cleanup": {"status": "pending"}
  },
  "test_results": [],
  "coverage_stats": {}
}
EOF

    log_success "测试状态跟踪初始化完成"
}

# 执行数据收集步骤
run_data_collection() {
    log_section "覆盖率数据收集"

    update_step_status "data_collection" "running"

    case "$MODE" in
        "basic")
            collect_basic_coverage
            ;;
        "comprehensive")
            collect_comprehensive_coverage
            ;;
        "incremental")
            collect_incremental_coverage
            ;;
        "performance")
            collect_performance_coverage
            ;;
        "security")
            collect_security_coverage
            ;;
    esac

    update_step_status "data_collection" "completed"
    log_success "覆盖率数据收集完成"
}

# 基础覆盖率收集
collect_basic_coverage() {
    log_step "执行基础覆盖率测试"

    # 运行基础单元测试
    run_test_target "//tests/unit/basic:basic_tests" "基础测试"

    # 收集覆盖率数据
    collect_coverage_files
}

# 综合覆盖率收集
collect_comprehensive_coverage() {
    log_step "执行Level 1-6综合覆盖率测试"

    # Level 1: 基础工具类测试
    log_step "Level 1: 基础工具类测试"
    run_test_target "//tests/unit/basic:logger_basic_test" "日志系统测试"
    run_test_target "//tests/unit/basic:exception_test" "异常处理测试"
    run_test_target "//tests/unit/basic:config_manager_test" "配置管理测试"
    run_test_target "//tests/unit/basic:ast_node_basic_test" "AST节点测试"

    # Level 2: 存储引擎基础测试
    log_step "Level 2: 存储引擎基础测试"
    run_test_target "//tests/storage_engine:buffer_pool_test" "缓冲池测试"
    run_test_target "//tests/storage_engine:b_plus_tree_core_test" "B+树核心测试"
    run_test_target "//tests/storage_engine:storage_engine_boundary_test" "存储引擎边界测试"
    run_test_target "//tests/storage_engine:wal_system_test" "WAL系统测试"
    run_test_target "//tests/storage_engine:data_integrity_test" "数据完整性测试"

    # Level 3: 事务管理器测试
    log_step "Level 3: 事务管理器测试"
    run_test_target "//tests/level3_transaction_manager:config_test" "事务配置测试"
    run_test_target "//tests/unit/executor:task_executor_test" "任务执行器测试"
    run_test_target "//tests/unit/executor:comprehensive_task_executor_test" "综合任务执行器测试"

    # Level 4: SQL处理测试
    log_step "Level 4: SQL处理测试"
    run_test_target "//tests/unit/parser:json_parser_test" "JSON解析器测试"
    run_test_target "//tests/unit/parser:advanced_query_test" "高级查询测试"
    run_test_target "//tests/unit/parser:constraint_test" "约束测试"
    run_test_target "//tests/unit/execution:subquery_executor_test" "子查询执行器测试"

    # Level 5: 网络通信测试
    log_step "Level 5: 网络通信测试"
    run_test_target "//tests/unit/network:tls_connection_test" "TLS连接测试"
    run_test_target "//tests/network:integration_test" "网络集成测试"

    # Level 6: 企业级功能测试
    log_step "Level 6: 企业级功能测试"
    run_test_target "//tests/unit/security:dcl_permission_model_advanced_test" "权限模型测试"
    run_test_target "//tests/unit/security:dcl_role_management_test" "角色管理测试"
    run_test_target "//tests/performance/ddl:ddl_performance_test" "DDL性能测试"

    # 收集所有覆盖率数据
    collect_coverage_files
}

# 增量覆盖率收集
collect_incremental_coverage() {
    log_step "执行增量覆盖率测试"

    # 检查上次运行的结果，只测试修改的文件
    if [[ -f "$PROJECT_ROOT/coverage_results/last_run.json" ]]; then
        log_info "发现上次运行记录，执行增量测试"
        run_incremental_tests
    else
        log_warning "未找到上次运行记录，执行完整测试"
        collect_comprehensive_coverage
    fi
}

# 性能覆盖率收集
collect_performance_coverage() {
    log_step "执行性能覆盖率测试"

    # 运行性能相关的测试
    run_test_target "//tests/performance:buffer_pool_performance_test" "缓冲池性能测试"
    run_test_target "//tests/performance:memory_stress_test" "内存压力测试"
    run_test_target "//tests/performance:concurrency_performance_test" "并发性能测试"

    collect_coverage_files
}

# 安全覆盖率收集
collect_security_coverage() {
    log_step "执行安全覆盖率测试"

    # 运行安全相关的测试
    run_test_target "//tests/unit/security:dcl_permission_model_advanced_test" "权限模型测试"
    run_test_target "//tests/unit/security:dcl_role_management_test" "角色管理测试"
    run_test_target "//tests/unit/security:dcl_advanced_test" "高级安全测试"
    run_test_target "//tests/unit/network:tls_connection_test" "TLS安全测试"

    collect_coverage_files
}

# 运行测试目标
run_test_target() {
    local target="$1"
    local description="$2"

    log_info "运行测试: $description ($target)"

    if bazel coverage "$target" --combined_report=lcov --test_timeout=300 2>/dev/null; then
        log_success "✅ $description 测试通过"
        record_test_result "$target" "passed" "$description"
    else
        log_warning "⚠️  $description 测试失败或跳过"
        record_test_result "$target" "failed" "$description"
    fi
}

# 记录测试结果
record_test_result() {
    local target="$1"
    local status="$2"
    local description="$3"

    # 更新测试状态文件
    python3 -c "
import json
import sys
from datetime import datetime

with open('$OUTPUT_DIR/test_status.json', 'r') as f:
    data = json.load(f)

data['test_results'].append({
    'target': '$target',
    'status': '$status',
    'description': '$description',
    'timestamp': datetime.now().isoformat()
})

with open('$OUTPUT_DIR/test_status.json', 'w') as f:
    json.dump(data, f, indent=2)
"
}

# 收集覆盖率文件
collect_coverage_files() {
    log_step "收集覆盖率数据文件"

    # 查找并复制覆盖率文件
    find . -name "*.profdata" -o -name "*.profraw" -o -name "*.lcov" 2>/dev/null | \
    while read -r file; do
        if [[ -f "$file" ]]; then
            cp "$file" "$OUTPUT_DIR/coverage_data/"
            log_info "收集覆盖率文件: $(basename "$file")"
        fi
    done

    log_success "覆盖率数据文件收集完成"
}

# 执行分析步骤
run_analysis() {
    log_section "覆盖率数据分析"

    update_step_status "analysis" "running"

    case "$DEPTH" in
        "file")
            analyze_file_level
            ;;
        "function")
            analyze_function_level
            ;;
        "module")
            analyze_module_level
            ;;
        "component")
            analyze_component_level
            ;;
        "system")
            analyze_system_level
            ;;
    esac

    update_step_status "analysis" "completed"
    log_success "覆盖率数据分析完成"
}

# 文件级分析
analyze_file_level() {
    log_step "执行文件级覆盖率分析"
    python3 scripts/enhanced_coverage_analyzer.py analyze \
        --input-dir "$OUTPUT_DIR/coverage_data" \
        --output-dir "$OUTPUT_DIR/analysis" \
        --level file
}

# 函数级分析
analyze_function_level() {
    log_step "执行函数级覆盖率分析"
    python3 scripts/enhanced_coverage_analyzer.py analyze \
        --input-dir "$OUTPUT_DIR/coverage_data" \
        --output-dir "$OUTPUT_DIR/analysis" \
        --level function
}

# 模块级分析
analyze_module_level() {
    log_step "执行模块级覆盖率分析"
    bash scripts/analyze_module_coverage.sh \
        "$OUTPUT_DIR/coverage_data" \
        "$OUTPUT_DIR/analysis"
}

# 组件级分析
analyze_component_level() {
    log_step "执行组件级覆盖率分析"
    python3 scripts/enhanced_coverage_analyzer.py analyze \
        --input-dir "$OUTPUT_DIR/coverage_data" \
        --output-dir "$OUTPUT_DIR/analysis" \
        --level component
}

# 系统级分析
analyze_system_level() {
    log_step "执行系统级覆盖率分析"
    bash scripts/analyze_coverage_trends.sh \
        "$OUTPUT_DIR/coverage_data" \
        "$OUTPUT_DIR/analysis"
}

# 执行报告生成步骤
run_reporting() {
    log_section "覆盖率报告生成"

    update_step_status "reporting" "running"

    case "$FORMAT" in
        "html")
            generate_html_report
            ;;
        "lcov")
            generate_lcov_report
            ;;
        "json")
            generate_json_report
            ;;
        "text")
            generate_text_report
            ;;
        "all")
            generate_all_reports
            ;;
    esac

    update_step_status "reporting" "completed"
    log_success "覆盖率报告生成完成"
}

# 生成HTML报告
generate_html_report() {
    log_step "生成HTML覆盖率报告"
    bash scripts/generate_coverage_html_report.sh \
        "$OUTPUT_DIR/coverage_data" \
        "$OUTPUT_DIR/coverage_reports"
}

# 生成LCOV报告
generate_lcov_report() {
    log_step "生成LCOV覆盖率报告"
    bash scripts/generate_llvm_coverage_report.sh \
        "$OUTPUT_DIR/coverage_data" \
        "$OUTPUT_DIR/coverage_reports"
}

# 生成JSON报告
generate_json_report() {
    log_step "生成JSON覆盖率报告"
    python3 scripts/enhanced_coverage_analyzer.py report \
        --input-dir "$OUTPUT_DIR/coverage_data" \
        --output-dir "$OUTPUT_DIR/coverage_reports" \
        --format json
}

# 生成文本报告
generate_text_report() {
    log_step "生成文本覆盖率报告"
    bash scripts/generate_coverage_report.sh \
        "$OUTPUT_DIR/coverage_data" \
        "$OUTPUT_DIR/coverage_reports"
}

# 生成所有格式报告
generate_all_reports() {
    log_step "生成所有格式覆盖率报告"
    generate_html_report
    generate_lcov_report
    generate_json_report
    generate_text_report
}

# 执行质量门禁检查
run_quality_gate() {
    log_section "质量门禁检查"

    update_step_status "quality_gate" "running"

    bash scripts/check_coverage_quality.sh \
        "$OUTPUT_DIR/coverage_reports" \
        "$THRESHOLD"

    local quality_check_result=$?

    if [[ $quality_check_result -eq 0 ]]; then
        update_step_status "quality_gate" "passed"
        log_success "✅ 质量门禁检查通过"
    else
        update_step_status "quality_gate" "failed"
        log_error "❌ 质量门禁检查失败"
        exit 1
    fi
}

# 执行清理步骤
run_cleanup() {
    log_section "清理临时文件"

    update_step_status "cleanup" "running"

    # 清理临时文件
    rm -rf "$OUTPUT_DIR/temp" 2>/dev/null || true
    rm -f "$OUTPUT_DIR"/*.tmp 2>/dev/null || true

    # 压缩旧的覆盖率数据
    if [[ -d "$OUTPUT_DIR/coverage_data" ]]; then
        cd "$OUTPUT_DIR/coverage_data"
        tar -czf coverage_data_backup.tar.gz *.profdata *.profraw 2>/dev/null || true
        cd - > /dev/null
    fi

    update_step_status "cleanup" "completed"
    log_success "临时文件清理完成"
}

# 更新步骤状态
update_step_status() {
    local step="$1"
    local status="$2"

    python3 -c "
import json
import sys
from datetime import datetime

with open('$OUTPUT_DIR/test_status.json', 'r') as f:
    data = json.load(f)

if '$status' == 'running':
    data['steps']['$step']['start_time'] = datetime.now().isoformat()
else:
    data['steps']['$step']['end_time'] = datetime.now().isoformat()

data['steps']['$step']['status'] = '$status'

with open('$OUTPUT_DIR/test_status.json', 'w') as f:
    json.dump(data, f, indent=2)
"
}

# 显示执行摘要
show_summary() {
    log_section "执行摘要"

    if [[ -f "$OUTPUT_DIR/test_status.json" ]]; then
        python3 -c "
import json
from datetime import datetime

with open('$OUTPUT_DIR/test_status.json', 'r') as f:
    data = json.load(f)

print('SQLCC 统一覆盖率测试执行摘要')
print('=' * 50)
print(f'执行时间: {data[\"start_time\"]}')
print(f'测试模式: {data[\"mode\"]}')
print(f'输出格式: {data[\"format\"]}')
print(f'分析深度: {data[\"depth\"]}')
print(f'质量阈值: {data[\"threshold\"]}%')
print()
print('步骤执行状态:')
for step, info in data['steps'].items():
    status = info['status']
    if status == 'completed':
        print(f'  ✅ {step}: {status}')
    elif status == 'passed':
        print(f'  ✅ {step}: {status}')
    elif status == 'failed':
        print(f'  ❌ {step}: {status}')
    else:
        print(f'  ⏳ {step}: {status}')
print()
print(f'测试结果数量: {len(data[\"test_results\"])}')
print(f'输出目录: {\"$OUTPUT_DIR\"}')
print()
print('主要输出文件:')
if '$FORMAT' == 'html' or '$FORMAT' == 'all':
    print(f'  🌐 HTML报告: {\"$OUTPUT_DIR\"}/coverage_reports/index.html')
if '$FORMAT' == 'lcov' or '$FORMAT' == 'all':
    print(f'  📊 LCOV数据: {\"$OUTPUT_DIR\"}/coverage_reports/coverage.lcov')
if '$FORMAT' == 'json' or '$FORMAT' == 'all':
    print(f'  📋 JSON报告: {\"$OUTPUT_DIR\"}/coverage_reports/coverage.json')
print(f'  📄 执行日志: {\"$OUTPUT_DIR\"}/test_status.json')
"
    fi
}

# 列出所有步骤
list_steps() {
    echo "SQLCC 统一覆盖率测试脚本 - 可用步骤:"
    echo ""
    echo "1. setup          - 环境设置和依赖检查"
    echo "2. data_collection - 覆盖率数据收集"
    echo "3. analysis       - 覆盖率数据分析"
    echo "4. reporting      - 覆盖率报告生成"
    echo "5. quality_gate   - 质量门禁检查"
    echo "6. cleanup        - 清理临时文件"
    echo ""
    echo "执行特定步骤的示例:"
    echo "  $0 --step setup"
    echo "  $0 --step data_collection --mode comprehensive"
}

# 主函数
main() {
    parse_args "$@"

    if [[ "$LIST_STEPS" == true ]]; then
        list_steps
        exit 0
    fi

    log_section "SQLCC 统一覆盖率测试开始"
    log_info "配置: 模式=$MODE, 格式=$FORMAT, 深度=$DEPTH, 阈值=${THRESHOLD}%"
    log_info "输出目录: $OUTPUT_DIR"

    # 执行特定步骤或完整流程
    if [[ -n "$SPECIFIC_STEP" ]]; then
        case "$SPECIFIC_STEP" in
            "setup")
                setup_environment
                ;;
            "data_collection")
                run_data_collection
                ;;
            "analysis")
                run_analysis
                ;;
            "reporting")
                run_reporting
                ;;
            "quality_gate")
                run_quality_gate
                ;;
            "cleanup")
                run_cleanup
                ;;
            *)
                log_error "未知步骤: $SPECIFIC_STEP"
                list_steps
                exit 1
                ;;
        esac
    else
        # 执行完整流程
        setup_environment
        run_data_collection
        run_analysis
        run_reporting
        run_quality_gate
        run_cleanup
    fi

    show_summary

    log_section "SQLCC 统一覆盖率测试完成"
    log_success "🎉 所有任务执行完成!"
}

# 执行主函数
main "$@"
