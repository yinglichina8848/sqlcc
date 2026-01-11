#!/bin/bash

# SQLCC 监控和告警脚本
# 监控服务状态、性能指标，并发送告警通知

set -e

# 配置变量
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
MONITOR_INTERVAL="${MONITOR_INTERVAL:-60}"  # 监控间隔（秒）
ALERT_COOLDOWN="${ALERT_COOLDOWN:-300}"     # 告警冷却时间（秒）
LOG_FILE="${PROJECT_ROOT}/logs/monitoring_$(date +%Y%m%d).log"

# 监控配置
MONITOR_CONFIG="${PROJECT_ROOT}/config/monitoring.json"

# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# 日志函数
log_info() {
    echo -e "$(date '+%Y-%m-%d %H:%M:%S') ${BLUE}[INFO]${NC} $1" | tee -a "$LOG_FILE"
}

log_success() {
    echo -e "$(date '+%Y-%m-%d %H:%M:%S') ${GREEN}[SUCCESS]${NC} $1" | tee -a "$LOG_FILE"
}

log_warning() {
    echo -e "$(date '+%Y-%m-%d %H:%M:%S') ${YELLOW}[WARNING]${NC} $1" | tee -a "$LOG_FILE"
}

log_error() {
    echo -e "$(date '+%Y-%m-%d %H:%M:%S') ${RED}[ERROR]${NC} $1" | tee -a "$LOG_FILE"
}

# 创建必要的目录
setup_directories() {
    mkdir -p "${PROJECT_ROOT}/logs"
    mkdir -p "${PROJECT_ROOT}/config"
    mkdir -p "${PROJECT_ROOT}/metrics"
}

# 创建默认监控配置
create_default_config() {
    if [[ ! -f "$MONITOR_CONFIG" ]]; then
        log_info "创建默认监控配置..."

        cat > "$MONITOR_CONFIG" << 'EOF'
{
  "service": {
    "name": "sqlcc",
    "environments": ["development", "staging", "production"],
    "health_check": {
      "enabled": true,
      "endpoint": "http://localhost:8080/health",
      "timeout": 30,
      "interval": 30
    }
  },
  "alerts": {
    "enabled": true,
    "channels": {
      "email": {
        "enabled": true,
        "recipients": ["admin@company.com"],
        "smtp_server": "smtp.company.com",
        "smtp_port": 587,
        "smtp_user": "alerts@company.com",
        "smtp_password": "password"
      },
      "slack": {
        "enabled": false,
        "webhook_url": "https://hooks.slack.com/services/...",
        "channel": "#sqlcc-alerts"
      },
      "webhook": {
        "enabled": false,
        "url": "https://api.company.com/alerts",
        "headers": {
          "Authorization": "Bearer token",
          "Content-Type": "application/json"
        }
      }
    }
  },
  "thresholds": {
    "cpu_usage_percent": 80,
    "memory_usage_percent": 85,
    "disk_usage_percent": 90,
    "response_time_ms": 1000,
    "error_rate_percent": 5,
    "connection_count": 1000
  },
  "metrics": {
    "collection_interval": 60,
    "retention_days": 30,
    "export": {
      "prometheus": {
        "enabled": false,
        "port": 9090
      },
      "json": {
        "enabled": true,
        "file": "metrics/metrics.json"
      }
    }
  }
}
EOF

        log_success "默认监控配置已创建: $MONITOR_CONFIG"
    fi
}

# 加载监控配置
load_config() {
    if [[ ! -f "$MONITOR_CONFIG" ]]; then
        create_default_config
    fi

    # 使用简单的配置解析（生产环境中建议使用jq或其他JSON解析器）
    CONFIG_CONTENT=$(cat "$MONITOR_CONFIG")
}

# 检查服务状态
check_service_status() {
    local service_type="$1"
    local service_name="${2:-sqlcc}"

    case $service_type in
        docker)
            if docker ps -q -f name="$service_name" | grep -q .; then
                echo "running"
            else
                echo "stopped"
            fi
            ;;
        systemd)
            if sudo systemctl is-active --quiet "$service_name" 2>/dev/null; then
                echo "running"
            else
                echo "stopped"
            fi
            ;;
        binary)
            if pgrep -f "$service_name" > /dev/null; then
                echo "running"
            else
                echo "stopped"
            fi
            ;;
        *)
            echo "unknown"
            ;;
    esac
}

# 收集系统指标
collect_system_metrics() {
    local service_type="$1"
    local service_name="${2:-sqlcc}"

    local metrics="{}"

    # CPU使用率
    local cpu_usage
    if command -v top &> /dev/null; then
        cpu_usage=$(top -bn1 | grep "Cpu(s)" | sed "s/.*, *\([0-9.]*\)%* id.*/\1/" | awk '{print 100 - $1}')
    else
        cpu_usage="0"
    fi

    # 内存使用率
    local mem_usage
    if command -v free &> /dev/null; then
        mem_usage=$(free | grep Mem | awk '{printf "%.2f", $3/$2 * 100.0}')
    else
        mem_usage="0"
    fi

    # 磁盘使用率
    local disk_usage
    if command -v df &> /dev/null; then
        disk_usage=$(df / | tail -1 | awk '{print $5}' | sed 's/%//')
    else
        disk_usage="0"
    fi

    # 网络连接数
    local connection_count
    case $service_type in
        docker)
            connection_count=$(docker exec "$service_name" netstat -t 2>/dev/null | wc -l 2>/dev/null || echo "0")
            ;;
        *)
            connection_count=$(netstat -t 2>/dev/null | grep -c ":3306 " 2>/dev/null || echo "0")
            ;;
    esac

    # 服务特定指标
    local service_cpu="0"
    local service_mem="0"

    case $service_type in
        docker)
            # 获取容器资源使用情况
            local container_stats
            container_stats=$(docker stats --no-stream --format "table {{.CPUPerc}}\t{{.MemPerc}}" "$service_name" 2>/dev/null | tail -1)
            if [[ -n "$container_stats" ]]; then
                service_cpu=$(echo "$container_stats" | awk '{print $1}' | sed 's/%//')
                service_mem=$(echo "$container_stats" | awk '{print $2}' | sed 's/%//')
            fi
            ;;
        systemd)
            # 获取systemd服务资源使用情况
            local pid
            pid=$(sudo systemctl show "$service_name" -p MainPID --value 2>/dev/null)
            if [[ -n "$pid" && "$pid" != "0" ]]; then
                service_cpu=$(ps -p "$pid" -o %cpu --no-headers 2>/dev/null || echo "0")
                service_mem=$(ps -p "$pid" -o %mem --no-headers 2>/dev/null || echo "0")
            fi
            ;;
        binary)
            # 获取进程资源使用情况
            service_cpu=$(ps -C "$service_name" -o %cpu --no-headers 2>/dev/null | awk '{s+=$1} END {print s}' || echo "0")
            service_mem=$(ps -C "$service_name" -o %mem --no-headers 2>/dev/null | awk '{s+=$1} END {print s}' || echo "0")
            ;;
    esac

    # 构建指标JSON
    metrics=$(cat << EOF
{
  "timestamp": "$(date -Iseconds)",
  "system": {
    "cpu_usage_percent": $cpu_usage,
    "memory_usage_percent": $mem_usage,
    "disk_usage_percent": $disk_usage
  },
  "service": {
    "cpu_usage_percent": $service_cpu,
    "memory_usage_percent": $service_mem,
    "connection_count": $connection_count,
    "status": "$(check_service_status "$service_type" "$service_name")"
  }
}
EOF
)

    echo "$metrics"
}

# 检查健康状态
check_health() {
    local service_type="$1"
    local service_name="${2:-sqlcc}"
    local health_endpoint="${3:-http://localhost:8080/health}"

    local health_status="unknown"
    local response_time="0"

    # 检查服务是否运行
    local service_status
    service_status=$(check_service_status "$service_type" "$service_name")

    if [[ "$service_status" == "running" ]]; then
        # 尝试健康检查端点
        if command -v curl &> /dev/null; then
            local start_time
            start_time=$(date +%s%N)

            if curl -f -s --max-time 30 "$health_endpoint" > /dev/null 2>&1; then
                health_status="healthy"
            else
                health_status="unhealthy"
            fi

            local end_time
            end_time=$(date +%s%N)
            response_time=$(( (end_time - start_time) / 1000000 ))  # 转换为毫秒
        else
            # 简单的端口检查
            case $service_type in
                docker)
                    if docker exec "$service_name" nc -z localhost 3306 2>/dev/null; then
                        health_status="healthy"
                    else
                        health_status="unhealthy"
                    fi
                    ;;
                *)
                    if nc -z localhost 3306 2>/dev/null; then
                        health_status="healthy"
                    else
                        health_status="unhealthy"
                    fi
                    ;;
            esac
        fi
    else
        health_status="stopped"
    fi

    # 返回健康状态JSON
    cat << EOF
{
  "status": "$health_status",
  "response_time_ms": $response_time,
  "service_status": "$service_status",
  "timestamp": "$(date -Iseconds)"
}
EOF
}

# 发送告警
send_alert() {
    local alert_type="$1"
    local message="$2"
    local severity="${3:-warning}"

    log_warning "发送告警: [$severity] $alert_type - $message"

    # 检查告警冷却时间
    local alert_key="${alert_type}_last_sent"
    local last_sent="${!alert_key:-0}"
    local current_time
    current_time=$(date +%s)

    if (( current_time - last_sent < ALERT_COOLDOWN )); then
        log_info "告警冷却中，跳过告警发送"
        return
    fi

    # 读取配置中的告警渠道
    local email_enabled
    local slack_enabled
    local webhook_enabled

    # 简化配置解析（生产环境建议使用jq）
    if grep -q '"email": {"enabled": true' "$MONITOR_CONFIG" 2>/dev/null; then
        email_enabled=true
    fi

    if grep -q '"slack": {"enabled": true' "$MONITOR_CONFIG" 2>/dev/null; then
        slack_enabled=true
    fi

    if grep -q '"webhook": {"enabled": true' "$MONITOR_CONFIG" 2>/dev/null; then
        webhook_enabled=true
    fi

    # 发送邮件告警
    if [[ "$email_enabled" == "true" ]]; then
        send_email_alert "$alert_type" "$message" "$severity"
    fi

    # 发送Slack告警
    if [[ "$slack_enabled" == "true" ]]; then
        send_slack_alert "$alert_type" "$message" "$severity"
    fi

    # 发送Webhook告警
    if [[ "$webhook_enabled" == "true" ]]; then
        send_webhook_alert "$alert_type" "$message" "$severity"
    fi

    # 更新最后发送时间
    eval "${alert_key}=$(date +%s)"
}

# 发送邮件告警
send_email_alert() {
    local alert_type="$1"
    local message="$2"
    local severity="$3"

    log_info "发送邮件告警..."

    # 这里实现邮件发送逻辑
    # 生产环境可以使用sendmail、postfix或第三方邮件服务

    local subject="SQLCC Alert: $alert_type ($severity)"
    local body="Alert Time: $(date)\nType: $alert_type\nSeverity: $severity\nMessage: $message\n"

    # 示例：使用mail命令发送邮件（需要配置邮件系统）
    # echo -e "$body" | mail -s "$subject" admin@company.com

    log_info "邮件告警已发送（模拟）"
}

# 发送Slack告警
send_slack_alert() {
    local alert_type="$1"
    local message="$2"
    local severity="$3"

    log_info "发送Slack告警..."

    # 从配置中提取Slack配置
    local webhook_url
    webhook_url=$(grep -o '"webhook_url": "[^"]*"' "$MONITOR_CONFIG" 2>/dev/null | cut -d'"' -f4)

    if [[ -n "$webhook_url" ]]; then
        local payload
        payload=$(cat << EOF
{
  "text": "SQLCC Alert: $alert_type",
  "blocks": [
    {
      "type": "header",
      "text": {
        "type": "plain_text",
        "text": "🚨 SQLCC Alert: $alert_type"
      }
    },
    {
      "type": "section",
      "fields": [
        {
          "type": "mrkdwn",
          "text": "*Severity:* $severity"
        },
        {
          "type": "mrkdwn",
          "text": "*Time:* $(date)"
        }
      ]
    },
    {
      "type": "section",
      "text": {
        "type": "mrkdwn",
        "text": "*Message:* $message"
      }
    }
  ]
}
EOF
)

        # 发送到Slack
        if command -v curl &> /dev/null; then
            curl -X POST -H 'Content-type: application/json' \
                 --data "$payload" \
                 "$webhook_url" 2>/dev/null || true
        fi

        log_success "Slack告警已发送"
    else
        log_warning "Slack配置不完整，跳过告警发送"
    fi
}

# 发送Webhook告警
send_webhook_alert() {
    local alert_type="$1"
    local message="$2"
    local severity="$3"

    log_info "发送Webhook告警..."

    # 从配置中提取Webhook配置
    local webhook_url
    webhook_url=$(grep -o '"url": "[^"]*"' "$MONITOR_CONFIG" 2>/dev/null | cut -d'"' -f4)

    if [[ -n "$webhook_url" ]]; then
        local payload
        payload=$(cat << EOF
{
  "alert_type": "$alert_type",
  "severity": "$severity",
  "message": "$message",
  "timestamp": "$(date -Iseconds)",
  "service": "sqlcc"
}
EOF
)

        # 发送Webhook
        if command -v curl &> /dev/null; then
            curl -X POST \
                 -H 'Content-Type: application/json' \
                 --data "$payload" \
                 "$webhook_url" 2>/dev/null || true
        fi

        log_success "Webhook告警已发送"
    else
        log_warning "Webhook配置不完整，跳过告警发送"
    fi
}

# 监控循环
monitor_loop() {
    local service_type="$1"
    local service_name="${2:-sqlcc}"

    log_info "开始监控循环 (间隔: ${MONITOR_INTERVAL}秒)"
    log_info "服务类型: $service_type"
    log_info "服务名称: $service_name"

    # 初始化告警状态
    local last_service_status=""
    local consecutive_failures=0

    while true; do
        log_info "执行监控检查..."

        # 检查服务状态
        local service_status
        service_status=$(check_service_status "$service_type" "$service_name")

        if [[ "$service_status" != "running" ]]; then
            ((consecutive_failures++))
            if [[ "$last_service_status" != "down" ]]; then
                send_alert "SERVICE_DOWN" "SQLCC服务已停止运行 (类型: $service_type, 名称: $service_name)" "critical"
                last_service_status="down"
            fi
        else
            if [[ "$last_service_status" == "down" ]]; then
                send_alert "SERVICE_UP" "SQLCC服务已恢复运行" "info"
                last_service_status="running"
                consecutive_failures=0
            fi
        fi

        # 收集和检查系统指标
        local metrics
        metrics=$(collect_system_metrics "$service_type" "$service_name")

        # 解析指标并检查阈值
        local cpu_usage
        local mem_usage
        local disk_usage

        cpu_usage=$(echo "$metrics" | grep -o '"cpu_usage_percent": \[[^]]*\]' | cut -d'[' -f2 | cut -d']' -f1 | cut -d',' -f1)
        mem_usage=$(echo "$metrics" | grep -o '"memory_usage_percent": \[[^]]*\]' | cut -d'[' -f2 | cut -d']' -f1 | cut -d',' -f1)
        disk_usage=$(echo "$metrics" | grep -o '"disk_usage_percent": \[[^]]*\]' | cut -d'[' -f2 | cut -d']' -f1 | cut -d',' -f1)

        # 检查CPU使用率
        if [[ -n "$cpu_usage" && $(echo "$cpu_usage > 80" | bc -l 2>/dev/null) ]]; then
            send_alert "HIGH_CPU_USAGE" "CPU使用率过高: ${cpu_usage}%" "warning"
        fi

        # 检查内存使用率
        if [[ -n "$mem_usage" && $(echo "$mem_usage > 85" | bc -l 2>/dev/null) ]]; then
            send_alert "HIGH_MEMORY_USAGE" "内存使用率过高: ${mem_usage}%" "warning"
        fi

        # 检查磁盘使用率
        if [[ -n "$disk_usage" && "$disk_usage" -gt 90 ]]; then
            send_alert "HIGH_DISK_USAGE" "磁盘使用率过高: ${disk_usage}%" "error"
        fi

        # 检查健康状态
        local health_status
        health_status=$(check_health "$service_type" "$service_name")

        local health_status_value
        health_status_value=$(echo "$health_status" | grep -o '"status": "[^"]*"' | cut -d'"' -f4)

        if [[ "$health_status_value" == "unhealthy" ]]; then
            send_alert "HEALTH_CHECK_FAILED" "健康检查失败" "error"
        fi

        # 保存指标数据
        local metrics_file="${PROJECT_ROOT}/metrics/$(date +%Y%m%d_%H%M%S).json"
        echo "$metrics" > "$metrics_file"

        # 清理旧的指标文件（保留最近7天的）
        find "${PROJECT_ROOT}/metrics" -name "*.json" -mtime +7 -delete 2>/dev/null || true

        log_info "监控检查完成，下次检查在 ${MONITOR_INTERVAL} 秒后"

        sleep "$MONITOR_INTERVAL"
    done
}

# 显示使用帮助
show_help() {
    echo "用法: $0 [选项] <命令>"
    echo ""
    echo "命令:"
    echo "  start <service_type> [service_name]  - 开始监控指定服务"
    echo "  check <service_type> [service_name]  - 执行一次性检查"
    echo "  status                             - 显示监控状态"
    echo "  config                             - 显示当前配置"
    echo ""
    echo "服务类型:"
    echo "  docker    - Docker容器"
    echo "  systemd   - SystemD服务"
    echo "  binary    - 二进制进程"
    echo ""
    echo "选项:"
    echo "  -i, --interval SEC   - 监控间隔 (默认: 60秒)"
    echo "  -c, --config FILE    - 配置文件路径"
    echo "  -h, --help          - 显示帮助信息"
    echo ""
    echo "示例:"
    echo "  $0 start docker sqlcc-production    # 监控Docker容器"
    echo "  $0 check systemd sqlcc               # 检查SystemD服务状态"
    echo "  $0 start binary --interval 30       # 每30秒监控二进制进程"
}

# 主函数
main() {
    local command=""
    local service_type=""
    local service_name="sqlcc"

    # 解析参数
    while [[ $# -gt 0 ]]; do
        case $1 in
            -i|--interval)
                MONITOR_INTERVAL="$2"
                shift 2
                ;;
            -c|--config)
                MONITOR_CONFIG="$2"
                shift 2
                ;;
            -h|--help)
                show_help
                exit 0
                ;;
            start|check|status|config)
                command="$1"
                shift
                if [[ $# -gt 0 && ! "$1" =~ ^- ]]; then
                    service_type="$1"
                    shift
                    if [[ $# -gt 0 && ! "$1" =~ ^- ]]; then
                        service_name="$1"
                        shift
                    fi
                fi
                ;;
            *)
                log_error "未知选项: $1"
                show_help
                exit 1
                ;;
        esac
    done

    setup_directories
    load_config

    case $command in
        start)
            if [[ -z "$service_type" ]]; then
                log_error "必须指定服务类型"
                show_help
                exit 1
            fi
            monitor_loop "$service_type" "$service_name"
            ;;
        check)
            if [[ -z "$service_type" ]]; then
                log_error "必须指定服务类型"
                show_help
                exit 1
            fi

            log_info "执行一次性检查..."
            local status
            status=$(check_service_status "$service_type" "$service_name")
            log_info "服务状态: $status"

            local health
            health=$(check_health "$service_type" "$service_name")
            log_info "健康状态: $(echo "$health" | grep -o '"status": "[^"]*"' | cut -d'"' -f4)"

            local metrics
            metrics=$(collect_system_metrics "$service_type" "$service_name")
            log_info "系统指标已收集"
            ;;
        status)
            log_info "监控系统状态"
            log_info "配置文件: $MONITOR_CONFIG"
            log_info "日志文件: $LOG_FILE"
            log_info "监控间隔: ${MONITOR_INTERVAL}秒"
            ;;
        config)
            log_info "当前配置:"
            cat "$MONITOR_CONFIG"
            ;;
        *)
            log_error "未知命令: $command"
            show_help
            exit 1
            ;;
    esac
}

# 执行主函数
main "$@"