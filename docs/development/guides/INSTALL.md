# SQLCC 安装指南

本文档提供SQLCC的详细安装和部署指南，支持多种操作系统和安装方式。

## 📋 系统要求

### 最低系统要求

| 组件 | 要求 | 推荐配置 |
|------|------|----------|
| **操作系统** | Linux Ubuntu 18.04+ / CentOS 7+ / macOS 10.15+ | Ubuntu 20.04+ / CentOS 8+ |
| **CPU** | x86_64架构，2核 | 4核+，支持多线程 |
| **内存** | 4GB RAM | 8GB+ RAM |
| **存储** | 10GB可用空间 | 50GB+ SSD存储 |
| **网络** | 千兆以太网 | 万兆网络（生产环境） |

### 编译工具要求

| 工具 | 版本要求 | 安装命令 |
|------|----------|----------|
| **GCC** | 9.0+ | `sudo apt install gcc-9 g++-9` |
| **Clang** | 10.0+ | `sudo apt install clang-10` |
| **CMake** | 3.15+ | `sudo apt install cmake` |
| **Bazel** | 4.0+ | 见下文安装指南 |
| **OpenSSL** | 1.1.1+ | `sudo apt install libssl-dev` |

## 🚀 快速安装

### 使用预编译包（推荐）

```bash
# 下载最新版本
wget https://github.com/sqlcc/sqlcc/releases/download/v1.2.6/sqlcc-v1.2.6-linux-x64.tar.gz

# 解压安装
tar -xzf sqlcc-v1.2.6-linux-x64.tar.gz
cd sqlcc-v1.2.6

# 运行安装脚本
sudo ./install.sh

# 验证安装
sqlcc --version
```

### 使用包管理器

#### Ubuntu/Debian

```bash
# 添加SQLCC仓库
echo "deb [signed-by=/usr/share/keyrings/sqlcc-archive-keyring.gpg] https://repo.sqlcc.org/apt stable main" | sudo tee /etc/apt/sources.list.d/sqlcc.list

# 安装公钥
curl -fsSL https://repo.sqlcc.org/apt/sqlcc-archive-keyring.gpg | sudo gpg --dearmor -o /usr/share/keyrings/sqlcc-archive-keyring.gpg

# 更新包索引
sudo apt update

# 安装SQLCC
sudo apt install sqlcc sqlcc-server sqlcc-client

# 启动服务
sudo systemctl start sqlcc-server
sudo systemctl enable sqlcc-server
```

#### CentOS/RHEL

```bash
# 添加SQLCC仓库
sudo yum-config-manager --add-repo https://repo.sqlcc.org/rpm/sqlcc.repo

# 安装SQLCC
sudo yum install sqlcc sqlcc-server sqlcc-client

# 启动服务
sudo systemctl start sqlcc-server
sudo systemctl enable sqlcc-server
```

#### macOS (使用Homebrew)

```bash
# 添加SQLCC tap
brew tap sqlcc/sqlcc

# 安装SQLCC
brew install sqlcc

# 启动服务
brew services start sqlcc/sqlcc/sqlcc-server
```

## 🔨 从源码编译安装

### 环境准备

#### Ubuntu/Debian

```bash
# 更新系统
sudo apt update && sudo apt upgrade -y

# 安装基础编译工具
sudo apt install -y build-essential cmake

# 安装Clang编译器
sudo apt install -y clang-18 lld-18

# 安装依赖库
sudo apt install -y libssl-dev zlib1g-dev libbz2-dev

# 设置Clang为默认编译器
sudo update-alternatives --install /usr/bin/cc cc /usr/bin/clang-18 100
sudo update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++-18 100
```

#### CentOS/RHEL

```bash
# 安装开发工具
sudo yum groupinstall -y "Development Tools"

# 安装CMake
sudo yum install -y cmake3

# 安装依赖库
sudo yum install -y openssl-devel zlib-devel bzip2-devel

# 设置CMake命令
sudo ln -sf /usr/bin/cmake3 /usr/bin/cmake
```

#### macOS

```bash
# 安装Xcode命令行工具
xcode-select --install

# 安装Homebrew（如果还没有）
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# 安装依赖
brew install cmake openssl zlib bzip2
```

### 安装Bazel

#### Ubuntu/Debian

```bash
# 添加Bazel仓库
curl -fsSL https://bazel.build/bazel-release.pub.gpg | gpg --dearmor > bazel-archive-keyring.gpg
sudo mv bazel-archive-keyring.gpg /usr/share/keyrings
echo "deb [arch=amd64 signed-by=/usr/share/keyrings/bazel-archive-keyring.gpg] https://storage.googleapis.com/bazel-apt stable jdk1.8" | sudo tee /etc/apt/sources.list.d/bazel.list

# 安装Bazel
sudo apt update && sudo apt install -y bazel
```

#### CentOS/RHEL

```bash
# 下载Bazel安装脚本
wget https://github.com/bazelbuild/bazel/releases/download/5.4.1/bazel-5.4.1-installer-linux-x86_64.sh

# 安装Bazel
chmod +x bazel-5.4.1-installer-linux-x86_64.sh
sudo ./bazel-5.4.1-installer-linux-x86_64.sh

# 添加到PATH
export PATH="$PATH:$HOME/bin"
echo 'export PATH="$PATH:$HOME/bin"' >> ~/.bashrc
```

#### macOS

```bash
# 使用Homebrew安装
brew install bazel

# 或者使用Bazelisk
brew install bazelisk
```

### 下载源码

```bash
# 克隆仓库
git clone https://gitee.com/yinglichina/sqlcc.git
cd sqlcc

# 切换到稳定版本（可选）
git checkout v1.2.6

# 初始化子模块（如果有）
git submodule update --init --recursive
```

### 编译安装

#### 使用CMake构建

```bash
# 创建构建目录
mkdir build && cd build

# 配置构建（调试版本）
cmake -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_C_COMPILER=clang-18 \
      -DCMAKE_CXX_COMPILER=clang++-18 \
      -DCMAKE_INSTALL_PREFIX=/usr/local/sqlcc \
      ..

# 编译（使用所有CPU核心）
make -j$(nproc)

# 运行测试
make test

# 安装
sudo make install

# 添加到PATH
echo 'export PATH="/usr/local/sqlcc/bin:$PATH"' >> ~/.bashrc
source ~/.bashrc
```

#### 使用Bazel构建

```bash
# 构建所有目标
bazel build //...

# 运行测试
bazel test //...

# 构建安装包
bazel build //:sqlcc_install

# 安装
sudo tar -xzf bazel-bin/sqlcc_install.tar.gz -C /usr/local/
```

### 编译选项说明

| 选项 | 说明 | 默认值 |
|------|------|--------|
| `CMAKE_BUILD_TYPE` | 构建类型 (Debug/Release) | Release |
| `CMAKE_INSTALL_PREFIX` | 安装路径 | /usr/local |
| `ENABLE_SSL` | 启用SSL支持 | ON |
| `ENABLE_TESTS` | 构建测试 | ON |
| `ENABLE_BENCHMARKS` | 构建性能测试 | OFF |
| `ENABLE_COVERAGE` | 启用覆盖率分析 | OFF |

## ⚙️ 配置和初始化

### 基本配置

创建配置文件 `/etc/sqlcc/sqlcc.conf`：

```ini
[server]
# 服务器配置
port = 3306
bind_address = 0.0.0.0
max_connections = 1000

[storage]
# 存储配置
data_directory = /var/lib/sqlcc/data
log_directory = /var/lib/sqlcc/log
buffer_pool_size = 1GB

[security]
# 安全配置
ssl_enabled = true
ssl_cert_file = /etc/sqlcc/ssl/server.crt
ssl_key_file = /etc/sqlcc/ssl/server.key

[logging]
# 日志配置
log_level = INFO
log_file = /var/log/sqlcc/sqlcc.log
```

### 初始化数据库

```bash
# 创建数据目录
sudo mkdir -p /var/lib/sqlcc/data
sudo chown sqlcc:sqlcc /var/lib/sqlcc/data

# 初始化系统数据库
sqlcc --initialize --config /etc/sqlcc/sqlcc.conf

# 创建管理员用户
sqlcc --create-admin-user --username admin --password strongpassword
```

### SSL证书配置

```bash
# 创建SSL证书目录
sudo mkdir -p /etc/sqlcc/ssl

# 生成自签名证书（仅用于测试）
openssl req -x509 -newkey rsa:4096 -keyout /etc/sqlcc/ssl/server.key -out /etc/sqlcc/ssl/server.crt -days 365 -nodes -subj "/CN=localhost"

# 设置权限
sudo chown sqlcc:sqlcc /etc/sqlcc/ssl/server.key /etc/sqlcc/ssl/server.crt
sudo chmod 600 /etc/sqlcc/ssl/server.key
sudo chmod 644 /etc/sqlcc/ssl/server.crt
```

## 🚀 启动服务

### 使用systemd服务

```bash
# 重新加载systemd配置
sudo systemctl daemon-reload

# 启动服务
sudo systemctl start sqlcc-server

# 查看状态
sudo systemctl status sqlcc-server

# 设置开机自启
sudo systemctl enable sqlcc-server

# 查看日志
sudo journalctl -u sqlcc-server -f
```

### 手动启动

```bash
# 前台运行（用于调试）
sqlcc-server --config /etc/sqlcc/sqlcc.conf

# 后台运行
sqlcc-server --config /etc/sqlcc/sqlcc.conf --daemon

# 使用screen运行
screen -S sqlcc sqlcc-server --config /etc/sqlcc/sqlcc.conf
```

### 验证安装

```bash
# 检查版本
sqlcc --version

# 连接到数据库
sqlcc-client -h localhost -P 3306 -u admin -p

# 运行基本查询
sqlcc-client -e "SELECT VERSION();"
```

## 🔧 故障排除

### 常见问题

#### 编译错误

**问题**: `fatal error: 'openssl/ssl.h' file not found`

**解决方案**:
```bash
# Ubuntu/Debian
sudo apt install libssl-dev

# CentOS/RHEL
sudo yum install openssl-devel

# macOS
brew install openssl
export LDFLAGS="-L/usr/local/opt/openssl/lib"
export CPPFLAGS="-I/usr/local/opt/openssl/include"
```

#### 服务启动失败

**问题**: `Failed to bind to port 3306`

**解决方案**:
```bash
# 检查端口占用
sudo netstat -tulpn | grep :3306

# 修改配置文件中的端口
vim /etc/sqlcc/sqlcc.conf
# port = 3307
```

#### 权限问题

**问题**: `Permission denied` 错误

**解决方案**:
```bash
# 设置正确的权限
sudo chown -R sqlcc:sqlcc /var/lib/sqlcc
sudo chown -R sqlcc:sqlcc /var/log/sqlcc
sudo chmod 755 /etc/sqlcc/sqlcc.conf
```

### 性能优化

#### 系统调优

```bash
# 增加文件描述符限制
echo "sqlcc soft nofile 65536" | sudo tee -a /etc/security/limits.conf
echo "sqlcc hard nofile 65536" | sudo tee -a /etc/security/limits.conf

# 调整内核参数
sudo sysctl -w net.core.somaxconn=65536
sudo sysctl -w net.ipv4.tcp_max_syn_backlog=65536
```

#### SQLCC调优

```ini
[performance]
# 性能配置
buffer_pool_size = 4GB
max_connections = 1000
query_cache_size = 256MB
thread_pool_size = 16
```

## 📦 升级安装

### 就地升级

```bash
# 停止服务
sudo systemctl stop sqlcc-server

# 备份数据
sudo cp -r /var/lib/sqlcc/data /var/lib/sqlcc/data.backup

# 升级软件
sudo apt update && sudo apt upgrade sqlcc

# 升级配置（如果需要）
sudo sqlcc --upgrade-config

# 启动服务
sudo systemctl start sqlcc-server
```

### 迁移升级

```bash
# 在新服务器上安装新版本
# ... 安装步骤 ...

# 导出数据
sqlcc-dump --all-databases > backup.sql

# 传输到新服务器
scp backup.sql new-server:/tmp/

# 在新服务器导入数据
sqlcc < /tmp/backup.sql
```

## 🐳 Docker部署

### 使用官方Docker镜像

```bash
# 拉取镜像
docker pull sqlcc/sqlcc:latest

# 运行容器
docker run -d \
  --name sqlcc-server \
  -p 3306:3306 \
  -v /var/lib/sqlcc:/var/lib/sqlcc \
  -e SQLCC_ROOT_PASSWORD=mysecretpassword \
  sqlcc/sqlcc:latest

# 连接到容器
docker exec -it sqlcc-server sqlcc-client -u root -p
```

### 自定义Docker构建

```dockerfile
FROM ubuntu:20.04

# 安装依赖
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    clang-18 \
    libssl-dev \
    zlib1g-dev

# 复制源码
COPY . /src
WORKDIR /src

# 构建
RUN mkdir build && cd build && \
    cmake -DCMAKE_BUILD_TYPE=Release .. && \
    make -j$(nproc) && \
    make install

# 暴露端口
EXPOSE 3306

# 启动命令
CMD ["sqlcc-server", "--config", "/etc/sqlcc/sqlcc.conf"]
```

## 📊 监控和维护

### 健康检查

```bash
# 检查服务状态
sudo systemctl status sqlcc-server

# 检查端口监听
sudo netstat -tulpn | grep sqlcc

# 检查日志
sudo tail -f /var/log/sqlcc/sqlcc.log
```

### 备份策略

```bash
# 创建备份脚本
cat > /usr/local/bin/sqlcc-backup.sh << 'EOF'
#!/bin/bash
BACKUP_DIR="/var/backups/sqlcc"
DATE=$(date +%Y%m%d_%H%M%S)

mkdir -p $BACKUP_DIR

# 逻辑备份
sqlcc-dump --all-databases > $BACKUP_DIR/sqlcc_backup_$DATE.sql

# 物理备份
cp -r /var/lib/sqlcc/data $BACKUP_DIR/data_$DATE

# 压缩备份
tar -czf $BACKUP_DIR/backup_$DATE.tar.gz -C $BACKUP_DIR sqlcc_backup_$DATE.sql data_$DATE

# 清理旧备份
find $BACKUP_DIR -name "backup_*.tar.gz" -mtime +30 -delete

echo "Backup completed: $BACKUP_DIR/backup_$DATE.tar.gz"
EOF

# 设置执行权限
chmod +x /usr/local/bin/sqlcc-backup.sh

# 添加到cron任务
echo "0 2 * * * /usr/local/bin/sqlcc-backup.sh" | crontab -
```

## 📞 获取帮助

如果在安装过程中遇到问题：

- 📖 查看[用户指南](docs/user/user_guide.md)
- 🐛 报告问题：[GitHub Issues](https://github.com/sqlcc/sqlcc/issues)
- 💬 社区支持：[社区论坛](https://forum.sqlcc.org/)
- 📧 邮件支持：support@sqlcc.org

---

*最后更新: 2025-12-23*
