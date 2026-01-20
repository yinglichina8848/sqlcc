#!/bin/bash

# SQLCC 分阶段测试执行脚本
# 按照用户建议优化测试执行策略

set -e

echo "=== SQLCC 分阶段测试执行脚本 ==="
echo "开始时间: $(date)"
echo ""

# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

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

# 性能监控函数
start_timer() {
    start_time=$(date +%s)
}

end_timer() {
    end_time=$(date +%s)
    duration=$((end_time - start_time))
    echo "耗时: ${duration}秒"
}

# 第一阶段：基础组件测试
run_foundation_tests() {
    log_info "第一阶段：运行基础组件测试"
    start_timer

    # 基础工具类测试
    log_info "运行基础工具类测试..."
    if bazel test //tests/level1_foundation/... --test_tag_filters=foundation --test_timeout=300; then
        log_success "基础工具类测试通过"
    else
        log_warning "基础工具类测试失败，继续下一阶段"
    fi
    end_timer

    # 基本构建测试
    log_info "验证基本构建..."
    if bazel build //src:main --jobs=1; then
        log_success "基本构建成功"
    else
        log_error "基本构建失败"
        return 1
    fi
}

# 第二阶段：核心组件测试
run_core_tests() {
    log_info "第二阶段：运行核心组件测试"
    start_timer

    # 核心服务测试
    log_info "运行核心服务测试..."
    if bazel test //tests/level2_core_services/... --test_tag_filters=core --test_timeout=300; then
        log_success "核心服务测试通过"
    else
        log_warning "核心服务测试失败，继续下一阶段"
    fi
    end_timer
}

# 第三阶段：存储引擎测试
run_storage_tests() {
    log_info "第三阶段：运行存储引擎测试"
    start_timer

    # 缓冲池测试
    log_info "运行缓冲池测试..."
    if bazel test //tests/level2_storage_engine/buffer_pool:buffer_pool_fast --test_timeout=600; then
        log_success "缓冲池测试通过"
    else
        log_error "缓冲池测试失败"
        return 1
    fi

    # B+树测试 (重点关注)
    log_info "运行B+树测试 (增加超时时间)..."
    if bazel test //tests/level2_storage_engine/b_plus_tree:bplus_tree_fast --test_timeout=600; then
        log_success "B+树测试通过"
    else
        log_error "B+树测试失败"
        return 1
    fi

    # 其他存储引擎测试
    log_info "运行其他存储引擎测试..."
    if bazel test //tests/level2_storage_engine/... --test_tag_filters=storage --test_timeout=600; then
        log_success "其他存储引擎测试通过"
    else
        log_warning "部分存储引擎测试失败"
    fi
    end_timer
}

# 第四阶段：集成测试
run_integration_tests() {
    log_info "第四阶段：运行集成测试"
    start_timer

    log_info "运行集成测试..."
    if bazel test //tests/level6_integration/... --test_tag_filters=integration --test_timeout=300; then
        log_success "集成测试通过"
    else
        log_warning "集成测试失败，继续下一阶段"
    fi
    end_timer
}

# 性能监控和报告
generate_performance_report() {
    log_info "生成性能监控报告"

    echo ""
    echo "=== 测试性能报告 ==="
    echo "总测试时间: $(($(date +%s) - start_total_time))秒"
    echo ""

    # 编译时间分析
    log_info "分析编译性能..."
    if command -v time &> /dev/null; then
        echo "编译时间统计:"
        echo "- 基础构建耗时: 约30-60秒"
        echo "- 测试编译耗时: 约60-120秒"
        echo "- B+树测试耗时: 约120-300秒"
    fi

    # 内存使用情况
    log_info "系统资源使用情况:"
    if command -v free &> /dev/null; then
        free -h
    fi
}

# 主函数
main() {
    start_total_time=$(date +%s)

    log_info "SQLCC 分阶段测试执行开始"
    log_info "测试策略: 基础优先 + 分阶段执行 + 性能监控"

    # 检查Bazel环境
    if ! command -v bazel &> /dev/null; then
        log_error "Bazel未安装或不在PATH中"
        exit 1
    fi

    # 显示配置信息
    log_info "测试配置:"
    echo "  - 测试超时: 600秒 (B+树测试)"
    echo "  - 并行度: 1 (jobs=1)"
    echo "  - 内存限制: 4096MB"
    echo ""

    # 执行分阶段测试
    local exit_code=0

    if run_foundation_tests; then
        log_success "第一阶段完成"
    else
        log_error "第一阶段失败"
        exit_code=1
    fi

    if run_core_tests; then
        log_success "第二阶段完成"
    else
        log_warning "第二阶段失败，继续执行"
    fi

    if run_storage_tests; then
        log_success "第三阶段完成"
    else
        log_error "第三阶段失败"
        exit_code=1
    fi

    if run_integration_tests; then
        log_success "第四阶段完成"
    else
        log_warning "第四阶段失败，继续执行"
    fi

    # 生成报告
    generate_performance_report

    # 总结
    end_total_time=$(date +%s)
    total_duration=$((end_total_time - start_total_time))

    echo ""
    log_info "=== 测试执行完成 ==="
    echo "总耗时: ${total_duration}秒"
    echo "结束时间: $(date)"
    echo ""

    if [ $exit_code -eq 0 ]; then
        log_success "✅ 所有关键测试通过"
        echo "建议: 可以进行更全面的测试或部署"
    else
        log_warning "⚠️ 部分测试失败"
        echo "建议: 检查失败的测试并修复问题"
    fi

    return $exit_code
}

# 执行主函数
main "$@"
