# SQLCC 故障排除指南

## 目录

- [快速诊断](#快速诊断)
- [安装和部署问题](#安装和部署问题)
- [启动和运行问题](#启动和运行问题)
- [连接和网络问题](#连接和网络问题)
- [性能问题](#性能问题)
- [数据和查询问题](#数据和查询问题)
- [安全和权限问题](#安全和权限问题)
- [系统资源问题](#系统资源问题)
- [日志分析](#日志分析)
- [高级故障排除](#高级故障排除)

## 快速诊断

### 系统状态检查脚本

创建快速诊断脚本 `diagnose_sqlcc.sh`：

```bash
#!/bin/bash
echo "=== SQLCC 系统诊断 ==="
echo "时间: $(date)"
echo "用户: $(whoami)"
echo "工作目录: $(pwd)"

# 检查服务状态
echo -e "\n1. 服务状态检查:"
if pgrep -f "sqlcc-server" > /dev/null; then
    echo "✓ SQLCC服务器正在运行"
    ps aux | grep sqlcc-server | grep -v grep
else
    echo "✗ SQLCC服务器未运行"
fi

# 检查端口
echo -e "\n2. 端口检查:"
if command -v netstat > /dev/null; then
    netstat -tulpn | grep :3306 || echo "端口3306未监听"
else
    ss -tulpn | grep :3306 || echo "端口3306未监听"
fi

# 检查数据目录
echo -e "\n3. 数据目录检查:"
if [ -d "data" ]; then
    echo "✓ 数据目录存在"
    ls -la data/
else
    echo "✗ 数据目录不存在"
fi

# 检查日志
echo -e "\n4. 日志检查:"
if [ -d "logs" ]; then
    echo "✓ 日志目录存在"
    ls -la logs/
    if [ -f "logs/sqlcc.log" ]; then
        echo "最近10行日志:"
        tail -10 logs/sqlcc.log
    fi
else
    echo "✗ 日志目录不存在"
fi

# 检查配置文件
echo -e "\n5. 配置检查:"
if [ -f "config/sqlcc.conf" ]; then
    echo "✓ 配置文件存在"
    head -5 config/sqlcc.conf
else
    echo "✗ 配置文件不存在"
fi

echo -e "\n=== 诊断完成 ==="
```

运行诊断：
```bash
chmod +x diagnose_sqlcc.sh
./diagnose_sqlcc.sh
```

## 安装和部署问题

### 编译失败

#### 缺少依赖
**现象**: 编译时提示找不到头文件或库

**诊断**:
```bash
# 检查编译器版本
clang++ --version

# 检查依赖是否安装
dpkg -l | grep -E "(openssl|ssl|crypto)"

# 检查库路径
find /usr -name "libssl*" 2>/dev/null
```

**解决方案**:
```bash
# Ubuntu/Debian
sudo apt update
sudo apt install -y build-essential clang-18 libssl-dev zlib1g-dev

# CentOS/RHEL
sudo yum install -y gcc-c++ openssl-devel zlib-devel

# macOS
brew install openssl
export LDFLAGS="-L/usr/local/opt/openssl/lib"
export CPPFLAGS="-I/usr/local/opt/openssl/include"
```

#### 内存不足
**现象**: 编译过程中被杀死 (Killed)

**诊断**:
```bash
# 检查可用内存
free -h
df -h

# 检查交换空间
swapon -s
```

**解决方案**:
```bash
# 增加交换空间
sudo fallocate -l 2G /swapfile
sudo chmod 600 /swapfile
sudo mkswap /swapfile
sudo swapon /swapfile

# 或使用更少的并行作业
make -j2  # 减少并行数
```

### 安装失败

#### 权限问题
**现象**: 安装时提示权限被拒绝

**解决方案**:
```bash
# 使用sudo安装
sudo make install

# 或指定用户目录安装
cmake -DCMAKE_INSTALL_PREFIX=$HOME/sqlcc ..
make install
```

#### 路径问题
**现象**: 安装后找不到可执行文件

**诊断**:
```bash
# 检查PATH
echo $PATH
which sqlcc-server

# 检查安装目录
ls -la /usr/local/bin/sqlcc*
```

**解决方案**:
```bash
# 添加PATH
export PATH=$PATH:/usr/local/bin

# 或创建符号链接
sudo ln -sf /usr/local/bin/sqlcc-server /usr/bin/sqlcc-server
```

## 启动和运行问题

### 服务器无法启动

#### 端口被占用
**现象**: 启动时提示端口已被使用

**诊断**:
```bash
# 检查端口占用
sudo netstat -tulpn | grep :3306
sudo lsof -i :3306

# 杀死占用进程
sudo kill -9 <PID>
```

**解决方案**:
```bash
# 使用不同端口
./sqlcc-server --port 3307

# 或修改配置文件
echo "port = 3307" >> config/sqlcc.conf
```

#### 数据目录权限问题
**现象**: 启动时提示无法访问数据目录

**诊断**:
```bash
# 检查目录权限
ls -la data/

# 检查用户权限
id
groups
```

**解决方案**:
```bash
# 修改目录权限
sudo chown -R sqlcc:sqlcc data/
sudo chmod 750 data/

# 或以正确用户运行
sudo -u sqlcc ./sqlcc-server
```

#### 配置文件错误
**现象**: 启动时提示配置错误

**诊断**:
```bash
# 验证配置文件语法
./sqlcc-server --config-test config/sqlcc.conf

# 检查配置文件格式
cat config/sqlcc.conf | grep -v "^#" | grep -v "^$"
```

**解决方案**:
```ini
# 正确的配置文件格式
[server]
port = 3306
data_dir = ./data
log_dir = ./logs

[storage]
buffer_pool_size = 1GB

[security]
ssl_enabled = false
```

### 服务器意外退出

#### 内存不足
**现象**: 服务器运行一段时间后崩溃

**诊断**:
```bash
# 检查系统内存
free -h
vmstat 1 10

# 查看崩溃前的日志
grep -B 5 -A 5 "SIGSEGV\|SIGABRT" logs/sqlcc.log
```

**解决方案**:
```bash
# 增加系统内存
# 或调整缓冲池大小
[storage]
buffer_pool_size = 512MB

# 启用内存监控
[server]
memory_monitor_enabled = true
memory_limit = 2GB
```

#### 磁盘空间不足
**现象**: 运行时提示磁盘空间不足

**诊断**:
```bash
# 检查磁盘使用情况
df -h
du -sh data/
du -sh logs/

# 检查大文件
find data/ -type f -size +100M
```

**解决方案**:
```bash
# 清理日志
./sqlcc-client -e "PURGE BINARY LOGS BEFORE '2023-01-01';"

# 移动数据目录
./sqlcc-server --data-dir /mnt/large_disk/sqlcc_data

# 压缩数据
# 考虑分区表或归档策略
```

## 连接和网络问题

### 无法连接到数据库

#### 防火墙阻止
**现象**: 本地可以连接，远程无法连接

**诊断**:
```bash
# 检查防火墙规则
sudo ufw status
sudo iptables -L

# 测试端口连通性
telnet localhost 3306
telnet remote_host 3306
```

**解决方案**:
```bash
# 开放端口
sudo ufw allow 3306

# 或修改防火墙规则
sudo iptables -A INPUT -p tcp --dport 3306 -j ACCEPT
```

#### 连接超时
**现象**: 连接尝试超时

**诊断**:
```bash
# 检查网络连通性
ping database_host

# 测试端口连通性
nc -zv database_host 3306

# 检查路由
traceroute database_host
```

**解决方案**:
```ini
# 调整连接超时
[client]
connect_timeout = 30

[server]
connection_timeout = 300
```

#### 连接数限制
**现象**: 提示连接数达到上限

**诊断**:
```bash
# 查看当前连接数
./sqlcc-client -e "SHOW PROCESSLIST;" | wc -l

# 检查配置限制
grep "max_connections" config/sqlcc.conf
```

**解决方案**:
```ini
# 增加最大连接数
[server]
max_connections = 1000

# 优化连接池
[network]
connection_pool_size = 100
connection_pool_timeout = 60
```

### SSL/TLS连接问题

#### 证书问题
**现象**: SSL连接失败

**诊断**:
```bash
# 检查证书文件
ls -la ssl/
openssl x509 -in ssl/server.crt -text -noout

# 测试SSL连接
openssl s_client -connect localhost:3306 -servername localhost
```

**解决方案**:
```bash
# 生成自签名证书
openssl req -x509 -newkey rsa:4096 -keyout ssl/server.key -out ssl/server.crt -days 365 -nodes -subj "/CN=localhost"

# 或配置正确的证书路径
[security]
ssl_cert_file = /etc/sqlcc/ssl/server.crt
ssl_key_file = /etc/sqlcc/ssl/server.key
```

## 性能问题

### 查询执行缓慢

#### 索引问题
**现象**: 查询执行时间过长

**诊断**:
```sql
-- 分析查询执行计划
EXPLAIN SELECT * FROM users WHERE age > 18;

-- 检查索引使用情况
SHOW INDEXES FROM users;

-- 查看慢查询日志
SELECT * FROM slow_query_log ORDER BY query_time DESC LIMIT 10;
```

**解决方案**:
```sql
-- 创建缺失的索引
CREATE INDEX idx_user_age ON users(age);
CREATE INDEX idx_user_name_age ON users(name, age);

-- 优化查询
SELECT * FROM users WHERE age > 18 ORDER BY age;  -- 利用索引排序
```

#### 缓冲池效率低
**现象**: 缓存命中率低

**诊断**:
```sql
-- 查看缓冲池统计
SHOW BUFFER POOL STATUS;

-- 检查命中率
SELECT (pages_hit / (pages_hit + pages_miss)) * 100 as hit_rate FROM buffer_pool_stats;
```

**解决方案**:
```ini
# 增加缓冲池大小
[storage]
buffer_pool_size = 2GB

# 调整分片数量
buffer_pool_shards = 16

# 启用预热
[storage]
buffer_pool_warmup_enabled = true
```

### 系统负载过高

#### CPU使用率高
**现象**: 系统响应缓慢，CPU使用率接近100%

**诊断**:
```bash
# 查看进程CPU使用
top -p $(pgrep sqlcc-server)

# 使用perf分析热点
perf top -p $(pgrep sqlcc-server)

# 查看系统负载
uptime
vmstat 1 5
```

**解决方案**:
```ini
# 启用查询缓存
[performance]
query_cache_enabled = true
query_cache_size = 256MB

# 限制并发查询
[server]
max_concurrent_queries = 50

# 启用线程池
[execution]
thread_pool_size = 8
```

#### 内存使用过高
**现象**: 系统内存不足，出现OOM

**诊断**:
```bash
# 查看内存使用
ps aux | grep sqlcc-server
free -h

# 检查内存泄漏
valgrind --leak-check=full ./sqlcc-server --test-run
```

**解决方案**:
```ini
# 调整缓冲池大小
[storage]
buffer_pool_size = 1GB

# 启用内存监控
[server]
memory_monitor_enabled = true
memory_limit = 2GB

# 配置垃圾回收
[storage]
gc_enabled = true
gc_interval = 300
```

## 数据和查询问题

### 数据一致性问题

#### 事务问题
**现象**: 数据更新丢失或不一致

**诊断**:
```sql
-- 检查事务隔离级别
SHOW VARIABLES LIKE 'transaction_isolation';

-- 查看活跃事务
SELECT * FROM information_schema.innodb_trx;

-- 检查死锁
SHOW ENGINE INNODB STATUS;
```

**解决方案**:
```sql
-- 设置合适的隔离级别
SET GLOBAL transaction_isolation = 'READ-COMMITTED';

-- 优化事务长度
BEGIN;
-- 快速执行操作
COMMIT;
```

#### 数据损坏
**现象**: 查询返回错误数据或崩溃

**诊断**:
```bash
# 检查数据文件完整性
./sqlcc-server --check-data-integrity

# 查看错误日志
grep "corruption\|error" logs/sqlcc.log

# 运行修复工具
./sqlcc-server --repair
```

**解决方案**:
```bash
# 从备份恢复
./sqlcc-client < backup.sql

# 运行完整性检查
CHECK TABLE table_name;

# 考虑数据迁移
# mysqldump > data.sql && mysql < data.sql
```

### 查询结果错误

#### SQL语法错误
**现象**: 查询执行失败

**诊断**:
```sql
-- 检查语法
EXPLAIN SELECT * FROM users WHERE id = 'abc';  -- 类型不匹配

-- 查看错误信息
SHOW ERRORS;
```

**解决方案**:
```sql
-- 修正类型转换
SELECT * FROM users WHERE id = 123;

-- 使用正确的数据类型
SELECT * FROM users WHERE created_at >= '2023-01-01 00:00:00';
```

#### 字符集问题
**现象**: 中文字符显示乱码

**诊断**:
```sql
-- 检查字符集设置
SHOW VARIABLES LIKE 'character_set%';

-- 查看表字符集
SHOW CREATE TABLE users;
```

**解决方案**:
```sql
-- 设置正确的字符集
SET NAMES utf8mb4;
ALTER TABLE users CONVERT TO CHARACTER SET utf8mb4;

-- 配置文件中设置
[server]
character_set = utf8mb4
collation = utf8mb4_unicode_ci
```

## 安全和权限问题

### 访问被拒绝

#### 用户权限不足
**现象**: 执行操作时提示权限不足

**诊断**:
```sql
-- 查看当前用户权限
SHOW GRANTS;

-- 检查用户权限
SELECT * FROM mysql.user WHERE user = 'current_user'\G
```

**解决方案**:
```sql
-- 授予必要权限
GRANT SELECT, INSERT, UPDATE ON database.* TO 'user'@'host';

-- 创建管理员用户
CREATE USER 'admin'@'localhost' IDENTIFIED BY 'secure_password';
GRANT ALL PRIVILEGES ON *.* TO 'admin'@'localhost' WITH GRANT OPTION;
```

#### 密码认证失败
**现象**: 登录时密码错误

**诊断**:
```sql
-- 检查密码过期
SELECT user, password_expired FROM mysql.user WHERE user = 'username';

-- 查看认证插件
SELECT user, plugin FROM mysql.user WHERE user = 'username';
```

**解决方案**:
```sql
-- 重置密码
ALTER USER 'user'@'host' IDENTIFIED BY 'new_password';

-- 或使用旧密码格式（不推荐）
SET old_passwords = 1;
UPDATE mysql.user SET password = PASSWORD('new_password') WHERE user = 'user';
```

### 安全漏洞

#### SQL注入风险
**现象**: 应用程序存在注入漏洞

**诊断**:
```sql
-- 检查查询日志中的可疑模式
SELECT * FROM slow_query_log WHERE sql_text LIKE '%1=1%';

-- 查看参数化查询使用情况
SHOW GLOBAL STATUS LIKE 'Com_prepare_sql';
```

**解决方案**:
```sql
-- 使用预编译语句
PREPARE stmt FROM 'SELECT * FROM users WHERE id = ?';
SET @user_id = 123;
EXECUTE stmt USING @user_id;

-- 应用程序中使用参数化查询
// Java示例
PreparedStatement stmt = conn.prepareStatement("SELECT * FROM users WHERE id = ?");
stmt.setInt(1, userId);
ResultSet rs = stmt.executeQuery();
```

## 系统资源问题

### 磁盘I/O问题

#### I/O负载过高
**现象**: 系统响应缓慢，磁盘利用率100%

**诊断**:
```bash
# 查看I/O统计
iostat -x 1 5

# 检查磁盘性能
hdparm -t /dev/sda

# 查看SQLCC I/O模式
SHOW ENGINE INNODB STATUS;
```

**解决方案**:
```ini
# 启用I/O优化
[storage]
innodb_flush_method = O_DIRECT
innodb_doublewrite = 0  # 对于SSD可以禁用

# 调整I/O线程数
innodb_read_io_threads = 8
innodb_write_io_threads = 8

# 使用SSD存储
# 数据目录放在SSD上
```

### 网络性能问题

#### 网络延迟高
**现象**: 远程连接响应慢

**诊断**:
```bash
# 测试网络延迟
ping -c 5 database_host

# 检查网络配置
ifconfig
netstat -i

# 查看连接统计
netstat -s | grep "connections"
```

**解决方案**:
```ini
# 优化网络设置
[network]
tcp_keepalive_time = 60
tcp_keepalive_intvl = 10
tcp_keepalive_probes = 6

# 使用连接池
connection_pool_enabled = true
connection_pool_size = 50
```

## 日志分析

### 日志级别设置

```ini
# 配置日志级别
[logging]
level = INFO  # DEBUG, INFO, WARN, ERROR
max_file_size = 100MB
max_files = 10

# 特定组件日志
[logging.storage]
level = DEBUG

[logging.network]
level = WARN
```

### 常见日志模式

```bash
# 查找错误
grep "ERROR\|FATAL" logs/sqlcc.log

# 查找警告
grep "WARN" logs/sqlcc.log

# 查找连接问题
grep "connection\|connect" logs/sqlcc.log

# 查找性能问题
grep "slow\|performance" logs/sqlcc.log

# 分析查询性能
grep "Query_time" logs/sqlcc.log | sort -n
```

### 日志轮转和归档

```bash
# 手动轮转日志
mv logs/sqlcc.log logs/sqlcc.log.$(date +%Y%m%d)
kill -HUP $(pgrep sqlcc-server)

# 压缩旧日志
find logs/ -name "*.log.*" -mtime +30 -exec gzip {} \;

# 日志分析工具
# 使用awk分析日志
awk '/ERROR/ {print $1, $2, $0}' logs/sqlcc.log | head -10
```

## 高级故障排除

### 内核调优

```bash
# 增加文件描述符限制
echo "sqlcc soft nofile 65536" >> /etc/security/limits.conf
echo "sqlcc hard nofile 65536" >> /etc/security/limits.conf

# 调整内核参数
echo "net.core.somaxconn = 1024" >> /etc/sysctl.conf
echo "vm.swappiness = 10" >> /etc/sysctl.conf
sysctl -p
```

### 性能监控

#### 安装监控工具
```bash
# Prometheus + Grafana
sudo apt install prometheus grafana

# 或使用内置监控
./sqlcc-server --metrics-enabled --metrics-port 9090
```

#### 关键指标监控
- **查询响应时间**: P95, P99响应时间
- **吞吐量**: QPS, TPS
- **资源使用**: CPU, 内存, 磁盘I/O
- **连接数**: 活跃连接, 等待连接
- **缓冲池命中率**: 缓存效率
- **锁等待时间**: 并发冲突情况

### 紧急恢复

#### 数据恢复流程
```bash
# 1. 停止服务
sudo systemctl stop sqlcc-server

# 2. 备份当前数据
cp -r data/ backup_$(date +%Y%m%d)/

# 3. 尝试修复
./sqlcc-server --repair --force

# 4. 从备份恢复
./sqlcc-server --restore backup.sql

# 5. 验证数据完整性
./sqlcc-server --check-integrity
```

#### 系统重置
```bash
# 完全重置（谨慎使用）
rm -rf data/*
rm -rf logs/*

# 重新初始化
./sqlcc-server --initialize

# 导入基础数据
./sqlcc-client < init_data.sql
```

---

## 获取更多帮助

如果以上方法都无法解决问题：

1. **收集诊断信息**:
   ```bash
   ./diagnose_sqlcc.sh > diagnostic_report.txt
   ```

2. **查看完整文档**:
   - [用户指南](user_guide.md)
   - [FAQ](faq.md)
   - [性能调优](performance_tuning.md)

3. **寻求社区帮助**:
   - GitHub Issues: https://github.com/sqlcc/sqlcc/issues
   - 论坛: https://forum.sqlcc.org/
   - 邮件列表: sqlcc-support@groups.io

4. **商业支持**:
   - 联系: support@sqlcc.org
   - 电话: +86-400-123-4567

---

*最后更新: 2025-12-24*

**重要提醒**: 在进行故障排除时，请务必先备份重要数据，避免数据丢失。生产环境的问题请优先联系专业技术支持。
