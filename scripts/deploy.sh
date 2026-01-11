#!/bin/bash

# SQLCC 自动化部署脚本
# 支持多种部署环境：Docker、本地、二进制部署

set -e

# 配置变量
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
DEPLOY_ENV="${DEPLOY_ENV:-development}"
DEPLOY_TYPE="${DEPLOY_TYPE:-docker}"
VERSION="${VERSION:-$(date +%Y%m%d-%H%M%S)}"

# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

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

# 显示帮助信息
show_help() {
    echo "用法: $0 [选项]"
    echo ""
    echo "选项:"
    echo "  -e, --env ENV          部署环境 (development|staging|production), 默认: development"
    echo "  -t, --type TYPE        部署类型 (docker|binary|systemd), 默认: docker"
    echo "  -v, --version VERSION  版本号, 默认: 时间戳"
    echo "  --rollback TAG         回滚到指定版本"
    echo "  --cleanup              清理旧版本"
    echo "  -h, --help             显示帮助信息"
    echo ""
    echo "环境变量:"
    echo "  DEPLOY_ENV             部署环境"
    echo "  DEPLOY_TYPE            部署类型"
    echo "  DOCKER_REGISTRY        Docker镜像仓库"
    echo "  DOCKER_USERNAME        Docker用户名"
    echo "  DOCKER_PASSWORD        Docker密码"
    echo ""
    echo "示例:"
    echo "  $0 --env production --type docker           # 生产环境Docker部署"
    echo "  $0 --type binary --cleanup                  # 二进制部署并清理旧版本"
    echo "  $0 --rollback v1.3.0                        # 回滚到v1.3.0版本"
}

# 解析命令行参数
parse_args() {
    while [[ $# -gt 0 ]]; do
        case $1 in
            -e|--env)
                DEPLOY_ENV="$2"
                shift 2
                ;;
            -t|--type)
                DEPLOY_TYPE="$2"
                shift 2
                ;;
            -v|--version)
                VERSION="$2"
                shift 2
                ;;
            --rollback)
                ROLLBACK_TAG="$2"
                DEPLOY_TYPE="rollback"
                shift 2
                ;;
            --cleanup)
                CLEANUP_OLD=true
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
}

# 验证部署环境
validate_environment() {
    log_info "验证部署环境..."

    case $DEPLOY_ENV in
        development|staging|production)
            log_success "部署环境: $DEPLOY_ENV"
            ;;
        *)
            log_error "无效的部署环境: $DEPLOY_ENV"
            exit 1
            ;;
    esac

    case $DEPLOY_TYPE in
        docker|binary|systemd|rollback)
            log_success "部署类型: $DEPLOY_TYPE"
            ;;
        *)
            log_error "无效的部署类型: $DEPLOY_TYPE"
            exit 1
            ;;
    esac

    # 检查必要的工具
    case $DEPLOY_TYPE in
        docker)
            if ! command -v docker &> /dev/null; then
                log_error "Docker未安装"
                exit 1
            fi
            ;;
        binary|systemd)
            if ! command -v systemctl &> /dev/null && [[ "$DEPLOY_TYPE" == "systemd" ]]; then
                log_error "systemd不可用，使用binary部署类型"
                DEPLOY_TYPE="binary"
            fi
            ;;
    esac
}

# 构建Docker镜像
build_docker_image() {
    log_info "构建Docker镜像..."

    local image_name="sqlcc:${VERSION}"
    local full_image_name="${DOCKER_REGISTRY:+${DOCKER_REGISTRY}/}${image_name}"

    # 创建Dockerfile
    cat > Dockerfile << EOF
FROM ubuntu:22.04

# 安装系统依赖
RUN apt-get update && apt-get install -y \\
    libssl-dev \\
    libcrypto++-dev \\
    && rm -rf /var/lib/apt/lists/*

# 创建应用目录
RUN mkdir -p /opt/sqlcc
WORKDIR /opt/sqlcc

# 复制二进制文件和配置
COPY sqlcc_server /opt/sqlcc/
COPY include /opt/sqlcc/include/
COPY scripts/docker-entrypoint.sh /opt/sqlcc/

# 创建数据目录
RUN mkdir -p /var/lib/sqlcc && \\
    mkdir -p /var/log/sqlcc

# 设置权限
RUN chmod +x /opt/sqlcc/sqlcc_server && \\
    chmod +x /opt/sqlcc/docker-entrypoint.sh

# 暴露端口
EXPOSE 3306

# 设置环境变量
ENV SQLCC_DATA_DIR=/var/lib/sqlcc
ENV SQLCC_LOG_DIR=/var/log/sqlcc

# 设置入口点
ENTRYPOINT ["/opt/sqlcc/docker-entrypoint.sh"]
EOF

    # 创建Docker入口点脚本
    cat > scripts/docker-entrypoint.sh << 'EOF'
#!/bin/bash
set -e

# 初始化数据目录
if [ ! -f "${SQLCC_DATA_DIR}/sqlcc.db" ]; then
    echo "初始化SQLCC数据库..."
    mkdir -p "${SQLCC_DATA_DIR}"
    # 这里可以添加数据库初始化逻辑
fi

# 启动SQLCC服务器
echo "启动SQLCC服务器..."
exec /opt/sqlcc/sqlcc_server \
    --data-dir="${SQLCC_DATA_DIR}" \
    --log-dir="${SQLCC_LOG_DIR}" \
    "$@"
EOF

    chmod +x scripts/docker-entrypoint.sh

    # 构建镜像
    docker build -t "$full_image_name" .

    # 推送镜像（如果配置了仓库）
    if [[ -n "${DOCKER_REGISTRY}" ]]; then
        log_info "推送镜像到仓库..."

        if [[ -n "${DOCKER_USERNAME}" && -n "${DOCKER_PASSWORD}" ]]; then
            echo "${DOCKER_PASSWORD}" | docker login -u "${DOCKER_USERNAME}" --password-stdin "${DOCKER_REGISTRY}"
        fi

        docker push "$full_image_name"
        log_success "镜像已推送到: $full_image_name"
    fi

    echo "$full_image_name" > .docker_image_name
}

# Docker部署
deploy_docker() {
    log_info "执行Docker部署..."

    local image_name
    if [[ -f .docker_image_name ]]; then
        image_name=$(cat .docker_image_name)
    else
        image_name="sqlcc:${VERSION}"
    fi

    local container_name="sqlcc-${DEPLOY_ENV}"

    # 停止现有容器
    if docker ps -q -f name="$container_name" | grep -q .; then
        log_info "停止现有容器..."
        docker stop "$container_name"
    fi

    if docker ps -a -q -f name="$container_name" | grep -q .; then
        log_info "删除现有容器..."
        docker rm "$container_name"
    fi

    # 创建必要的目录
    sudo mkdir -p "/var/lib/sqlcc-${DEPLOY_ENV}"
    sudo mkdir -p "/var/log/sqlcc-${DEPLOY_ENV}"

    # 根据环境设置不同的配置
    local env_vars=""
    case $DEPLOY_ENV in
        production)
            env_vars="-e SQLCC_PRODUCTION=true -e SQLCC_MAX_CONNECTIONS=1000"
            ;;
        staging)
            env_vars="-e SQLCC_STAGING=true -e SQLCC_MAX_CONNECTIONS=500"
            ;;
        development)
            env_vars="-e SQLCC_DEVELOPMENT=true -e SQLCC_MAX_CONNECTIONS=100"
            ;;
    esac

    # 启动新容器
    log_info "启动新容器..."
    docker run -d \
        --name "$container_name" \
        --restart unless-stopped \
        -p "${SQLCC_PORT:-3306}:3306" \
        -v "/var/lib/sqlcc-${DEPLOY_ENV}:/var/lib/sqlcc" \
        -v "/var/log/sqlcc-${DEPLOY_ENV}:/var/log/sqlcc" \
        $env_vars \
        "$image_name"

    # 等待服务启动
    log_info "等待服务启动..."
    sleep 10

    # 验证服务状态
    if docker ps -q -f name="$container_name" | grep -q .; then
        log_success "Docker部署成功"
        log_info "容器名称: $container_name"
        log_info "镜像: $image_name"

        # 显示容器日志
        log_info "容器状态:"
        docker ps -f name="$container_name" --format "table {{.Names}}\t{{.Status}}\t{{.Ports}}"
    else
        log_error "Docker部署失败"
        log_info "容器日志:"
        docker logs "$container_name" 2>&1 || true
        exit 1
    fi
}

# 二进制部署
deploy_binary() {
    log_info "执行二进制部署..."

    local install_dir="/opt/sqlcc-${VERSION}"
    local data_dir="/var/lib/sqlcc-${DEPLOY_ENV}"
    local log_dir="/var/log/sqlcc-${DEPLOY_ENV}"
    local config_dir="/etc/sqlcc-${DEPLOY_ENV}"

    # 创建目录
    sudo mkdir -p "$install_dir"
    sudo mkdir -p "$data_dir"
    sudo mkdir -p "$log_dir"
    sudo mkdir -p "$config_dir"

    # 复制文件
    sudo cp "bazel-bin/src/sqlcc_server" "$install_dir/"
    sudo cp -r include "$install_dir/"
    sudo cp LICENSE "$install_dir/"
    sudo cp README.md "$install_dir/"

    # 设置权限
    sudo chmod +x "$install_dir/sqlcc_server"
    sudo chown -R sqlcc:sqlcc "$install_dir" "$data_dir" "$log_dir" 2>/dev/null || true

    # 创建配置文件
    cat > /tmp/sqlcc.conf << EOF
# SQLCC Configuration for ${DEPLOY_ENV}
data_dir=${data_dir}
log_dir=${log_dir}
port=${SQLCC_PORT:-3306}

# Environment specific settings
EOF

    case $DEPLOY_ENV in
        production)
            cat >> /tmp/sqlcc.conf << EOF
max_connections=1000
log_level=info
performance_monitoring=true
EOF
            ;;
        staging)
            cat >> /tmp/sqlcc.conf << EOF
max_connections=500
log_level=debug
performance_monitoring=true
EOF
            ;;
        development)
            cat >> /tmp/sqlcc.conf << EOF
max_connections=100
log_level=debug
performance_monitoring=false
EOF
            ;;
    esac

    sudo cp /tmp/sqlcc.conf "$config_dir/sqlcc.conf"

    # 创建符号链接
    sudo ln -sf "$install_dir" "/opt/sqlcc-current-${DEPLOY_ENV}"

    log_success "二进制部署完成"
    log_info "安装目录: $install_dir"
    log_info "数据目录: $data_dir"
    log_info "日志目录: $log_dir"
    log_info "配置文件: $config_dir/sqlcc.conf"
}

# SystemD部署
deploy_systemd() {
    log_info "执行SystemD部署..."

    local service_name="sqlcc-${DEPLOY_ENV}"
    local service_file="/etc/systemd/system/${service_name}.service"

    # 创建systemd服务文件
    cat > /tmp/sqlcc.service << EOF
[Unit]
Description=SQLCC Database Server (${DEPLOY_ENV})
After=network.target

[Service]
Type=simple
User=sqlcc
Group=sqlcc
ExecStart=/opt/sqlcc-current-${DEPLOY_ENV}/sqlcc_server --config /etc/sqlcc-${DEPLOY_ENV}/sqlcc.conf
Restart=always
RestartSec=5
StandardOutput=journal
StandardError=journal
LimitNOFILE=65536

# Environment variables
Environment=SQLCC_ENV=${DEPLOY_ENV}

[Install]
WantedBy=multi-user.target
EOF

    sudo cp /tmp/sqlcc.service "$service_file"
    sudo systemctl daemon-reload

    # 停止现有服务
    if sudo systemctl is-active --quiet "$service_name"; then
        log_info "停止现有服务..."
        sudo systemctl stop "$service_name"
    fi

    # 启动新服务
    log_info "启动服务..."
    sudo systemctl start "$service_name"
    sudo systemctl enable "$service_name"

    # 等待服务启动
    sleep 5

    # 检查服务状态
    if sudo systemctl is-active --quiet "$service_name"; then
        log_success "SystemD部署成功"
        log_info "服务名称: $service_name"
        log_info "服务状态: $(sudo systemctl is-active $service_name)"

        # 显示服务日志
        log_info "最近的日志:"
        sudo journalctl -u "$service_name" -n 10 --no-pager
    else
        log_error "SystemD部署失败"
        log_info "服务状态: $(sudo systemctl is-active $service_name)"
        log_info "服务日志:"
        sudo journalctl -u "$service_name" -n 20 --no-pager
        exit 1
    fi
}

# 回滚部署
rollback_deployment() {
    log_info "执行回滚部署到: $ROLLBACK_TAG"

    case $DEPLOY_TYPE in
        docker)
            # 查找指定版本的镜像
            local rollback_image="sqlcc:${ROLLBACK_TAG}"
            if [[ -n "${DOCKER_REGISTRY}" ]]; then
                rollback_image="${DOCKER_REGISTRY}/sqlcc:${ROLLBACK_TAG}"
            fi

            if ! docker image inspect "$rollback_image" &> /dev/null; then
                log_error "找不到回滚镜像: $rollback_image"
                exit 1
            fi

            # 停止当前容器并启动旧版本
            DEPLOY_TYPE="docker"
            VERSION="$ROLLBACK_TAG"
            deploy_docker
            ;;
        binary|systemd)
            # 切换符号链接到旧版本
            local rollback_dir="/opt/sqlcc-${ROLLBACK_TAG}"
            if [[ ! -d "$rollback_dir" ]]; then
                log_error "找不到回滚版本目录: $rollback_dir"
                exit 1
            fi

            sudo ln -sf "$rollback_dir" "/opt/sqlcc-current-${DEPLOY_ENV}"

            if [[ "$DEPLOY_TYPE" == "systemd" ]]; then
                sudo systemctl restart "sqlcc-${DEPLOY_ENV}"
            fi

            log_success "回滚到版本: $ROLLBACK_TAG"
            ;;
    esac
}

# 清理旧版本
cleanup_old_versions() {
    log_info "清理旧版本..."

    case $DEPLOY_TYPE in
        docker)
            # 清理悬空镜像
            log_info "清理Docker悬空镜像..."
            docker image prune -f

            # 保留最近5个版本的镜像
            local images_to_keep=5
            docker images "sqlcc" --format "{{.Repository}}:{{.Tag}}" | \
            head -n "$images_to_keep" | \
            xargs -r docker rmi 2>/dev/null || true
            ;;
        binary|systemd)
            # 清理旧的安装目录，保留最近3个版本
            local versions_to_keep=3
            ls -d /opt/sqlcc-* 2>/dev/null | \
            grep -v "sqlcc-current" | \
            sort -V | \
            head -n -"$versions_to_keep" | \
            xargs -r sudo rm -rf
            ;;
    esac

    log_success "旧版本清理完成"
}

# 验证部署
validate_deployment() {
    log_info "验证部署..."

    local health_check_passed=false

    case $DEPLOY_TYPE in
        docker)
            local container_name="sqlcc-${DEPLOY_ENV}"
            if docker ps -q -f name="$container_name" | grep -q .; then
                # 简单的健康检查：检查端口是否监听
                if docker exec "$container_name" netstat -tln | grep -q ":3306 "; then
                    health_check_passed=true
                fi
            fi
            ;;
        binary)
            # 检查进程是否运行
            if pgrep -f "sqlcc_server" > /dev/null; then
                health_check_passed=true
            fi
            ;;
        systemd)
            local service_name="sqlcc-${DEPLOY_ENV}"
            if sudo systemctl is-active --quiet "$service_name"; then
                health_check_passed=true
            fi
            ;;
    esac

    if $health_check_passed; then
        log_success "部署验证通过"
    else
        log_error "部署验证失败"
        exit 1
    fi
}

# 显示部署信息
show_deployment_info() {
    log_info "部署信息:"

    case $DEPLOY_TYPE in
        docker)
            local container_name="sqlcc-${DEPLOY_ENV}"
            echo "容器名称: $container_name"
            echo "镜像版本: $(docker ps -f name="$container_name" --format "{{.Image}}")"
            echo "容器状态: $(docker ps -f name="$container_name" --format "{{.Status}}")"
            echo "端口映射: $(docker ps -f name="$container_name" --format "{{.Ports}}")"
            ;;
        binary)
            echo "安装路径: /opt/sqlcc-current-${DEPLOY_ENV}"
            echo "进程状态: $(pgrep -f sqlcc_server | wc -l) 个进程运行中"
            ;;
        systemd)
            local service_name="sqlcc-${DEPLOY_ENV}"
            echo "服务名称: $service_name"
            echo "服务状态: $(sudo systemctl is-active $service_name)"
            echo "服务日志: sudo journalctl -u $service_name -f"
            ;;
    esac
}

# 主函数
main() {
    parse_args "$@"
    validate_environment

    log_info "开始SQLCC部署"
    log_info "部署环境: $DEPLOY_ENV"
    log_info "部署类型: $DEPLOY_TYPE"
    log_info "版本: $VERSION"

    case $DEPLOY_TYPE in
        docker)
            build_docker_image
            deploy_docker
            ;;
        binary)
            deploy_binary
            ;;
        systemd)
            deploy_binary
            deploy_systemd
            ;;
        rollback)
            rollback_deployment
            ;;
    esac

    if [[ "${CLEANUP_OLD}" == "true" ]]; then
        cleanup_old_versions
    fi

    validate_deployment
    show_deployment_info

    log_success "SQLCC部署完成"

    echo ""
    echo "========================================"
    echo "部署摘要:"
    echo "- 环境: $DEPLOY_ENV"
    echo "- 类型: $DEPLOY_TYPE"
    echo "- 版本: $VERSION"
    echo "- 时间: $(date)"
    echo "========================================"
}

# 执行主函数
main "$@"