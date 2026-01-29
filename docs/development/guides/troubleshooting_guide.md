# SQLCC故障诊断手册 - 问题排查、错误处理与系统恢复指南

## 引言

故障诊断是数据库系统维护的重要组成部分。本手册提供SQLCC数据库系统的全面故障诊断指南，涵盖常见问题排查、错误处理策略和系统恢复流程，帮助管理员快速定位和解决问题。

## 1. 系统启动故障诊断

### 1.1 数据库服务启动失败

**现象：**
- 服务无法启动
- 日志显示启动失败
- 端口绑定错误

**诊断步骤：**

1. **检查系统资源：**
```bash
# 检查内存使用情况
free -h

# 检查磁盘空间
df -h

# 检查CPU负载
top -n 1

# 检查网络连接
netstat -tlnp | grep 3306
```

2. **检查配置文件：**
```bash
# 验证配置文件语法
sqlcc --config-test /etc/sqlcc/sqlcc.conf

# 检查关键配置项
grep -E "(port|data_dir|log_dir)" /etc/sqlcc/sqlcc.conf

# 验证文件权限
ls -la /etc/sqlcc/sqlcc.conf
ls -la /var/lib/sqlcc/
```

3. **检查依赖服务：**
```bash
# 检查系统服务状态
systemctl status sqlcc

# 查看服务日志
journalctl -u sqlcc -n 50

# 检查端口占用
lsof -i :3306
```

4. **常见解决方案：**
```bash
# 释放占用的端口
fuser -k 3306/tcp

# 修复文件权限
chown -R sqlcc:sqlcc /var/lib/sqlcc/
chmod 755 /var/lib/sqlcc/

# 清理临时文件
rm -rf /tmp/sqlcc_*
```

### 1.2 数据目录访问权限问题

**现象：**
- "Permission denied" 错误
- 无法创建数据文件
- 日志显示文件系统错误

**解决方案：**
```bash
# 检查数据目录权限
ls -ld /var/lib/sqlcc/

# 修复所有者权限
chown -R sqlcc:sqlcc /var/lib/sqlcc/

# 设置正确的权限
chmod 700 /var/lib/sqlcc/
chmod 600 /var/lib/sqlcc/*.dat

# 检查SELinux/AppArmor设置
sestatus
aa-status

# 临时禁用SELinux进行测试
setenforce 0
```

## 2. 连接问题诊断

### 2.1 客户端连接超时

**现象：**
- 连接建立超时
- "Connection refused" 错误
- 网络连接被重置

**诊断流程：**

1. **网络层检查：**
```bash
# 测试网络连通性
ping -c 3 database-server

# 测试端口可达性
telnet database-server 3306

# 检查防火墙设置
iptables -L -n | grep 3306
ufw status | grep 3306
```

2. **服务端状态检查：**
```bash
# 检查服务运行状态
ps aux | grep sqlcc

# 查看监听端口
netstat -tlnp | grep 3306

# 检查连接队列
ss -ltn | grep 3306

# 查看错误日志
tail -f /var/log/sqlcc/error.log
```

3. **连接池配置检查：**
```bash
# 检查最大连接数设置
sqlcc-admin show variables like 'max_connections'

# 查看当前连接数
sqlcc-admin show processlist | wc -l

# 检查连接超时设置
sqlcc-admin show variables like 'connect_timeout'
```

4. **性能问题排查：**
```bash
# 检查系统负载
uptime
vmstat 1 5

# 查看慢查询日志
tail -f /var/log/sqlcc/slow.log

# 分析查询性能
sqlcc-admin show engine innodb status
```

### 2.2 连接池耗尽

**现象：**
- "Too many connections" 错误
- 应用响应变慢
- 数据库拒绝新连接

**解决方案：**
```bash
# 临时增加连接数
sqlcc-admin set global max_connections = 200

# 检查连接使用情况
sqlcc-admin show processlist

# 终止空闲连接
sqlcc-admin kill connection <connection_id>

# 优化应用连接池配置
# application.properties
spring.datasource.maximum-pool-size=50
spring.datasource.minimum-idle=5
spring.datasource.max-wait=30000
```

## 3. 查询性能问题诊断

### 3.1 慢查询识别与分析

**识别慢查询：**
```bash
# 启用慢查询日志
sqlcc-admin set global slow_query_log = 'ON'
sqlcc-admin set global long_query_time = 1

# 查看慢查询统计
sqlcc-admin show variables like 'slow%'

# 分析慢查询日志
tail -f /var/log/sqlcc/slow.log | \
  awk '/^# Time:/ {time=$3} /^# Query_time:/ {query_time=$3} /^SELECT/ {print time, query_time, $0}'
```

**慢查询分析工具：**
```bash
# 使用EXPLAIN分析查询计划
sqlcc> EXPLAIN SELECT * FROM users WHERE age > 30;

# 查看索引使用情况
sqlcc> SHOW INDEX FROM users;

# 分析表统计信息
sqlcc> SHOW TABLE STATUS LIKE 'users'\G

# 检查查询优化器选择
sqlcc> SHOW WARNINGS;
```

### 3.2 索引相关问题

**索引失效排查：**
```bash
# 检查索引是否存在
sqlcc> SHOW CREATE TABLE users;

# 验证索引是否被使用
sqlcc> EXPLAIN SELECT * FROM users WHERE name = 'john'\G

# 检查索引统计信息是否过期
sqlcc> ANALYZE TABLE users;

# 重建索引（如果必要）
sqlcc> ALTER TABLE users DROP INDEX idx_name;
sqlcc> ALTER TABLE users ADD INDEX idx_name (name);
```

**复合索引问题：**
```bash
# 检查复合索引使用情况
sqlcc> EXPLAIN SELECT * FROM orders WHERE customer_id = 1 AND order_date > '2023-01-01'\G

# 分析索引选择性
sqlcc> SELECT COUNT(DISTINCT customer_id) / COUNT(*) as selectivity FROM orders;

# 考虑索引顺序调整
sqlcc> ALTER TABLE orders DROP INDEX idx_customer_date;
sqlcc> ALTER TABLE orders ADD INDEX idx_date_customer (order_date, customer_id);
```

### 3.3 锁等待问题

**锁等待诊断：**
```bash
# 查看锁等待情况
sqlcc> SHOW ENGINE INNODB STATUS\G

# 检查当前锁信息
sqlcc> SELECT * FROM information_schema.innodb_locks\G

# 查看锁等待信息
sqlcc> SELECT * FROM information_schema.innodb_lock_waits\G

# 分析事务状态
sqlcc> SHOW PROCESSLIST;
```

**死锁问题排查：**
```bash
# 查看死锁日志
tail -f /var/log/sqlcc/error.log | grep -i deadlock

# 启用死锁检测
sqlcc> SET GLOBAL innodb_deadlock_detect = ON;

# 设置死锁超时
sqlcc> SET GLOBAL innodb_lock_wait_timeout = 10;

# 分析死锁发生的原因
sqlcc> SHOW ENGINE INNODB STATUS;
```

## 4. 存储引擎问题诊断

### 4.1 缓冲池相关问题

**缓冲池命中率低：**
```bash
# 查看缓冲池统计信息
sqlcc> SHOW ENGINE INNODB STATUS;

# 检查缓冲池大小设置
sqlcc> SHOW VARIABLES LIKE 'innodb_buffer_pool_size';

# 分析缓冲池使用情况
sqlcc> SELECT
    pool_size,
    free_buffers,
    database_pages,
    old_database_pages,
    modified_database_pages
  FROM information_schema.innodb_buffer_pool_stats;

# 调整缓冲池大小
sqlcc> SET GLOBAL innodb_buffer_pool_size = 2147483648; -- 2GB
```

**缓冲池污染问题：**
```bash
# 查看页面访问统计
sqlcc> SELECT
    page_type,
    SUM(data_size) as total_size,
    COUNT(*) as page_count
  FROM information_schema.innodb_buffer_page
  GROUP BY page_type
  ORDER BY total_size DESC;

# 检查是否有大量随机访问
sqlcc> SHOW PROCESSLIST; -- 查找全表扫描查询

# 优化查询以减少缓冲池污染
# 添加索引
sqlcc> CREATE INDEX idx_scan_col ON table_name (scan_column);

# 使用覆盖索引
sqlcc> SELECT indexed_col FROM table_name WHERE indexed_col > 100;
```

### 4.2 磁盘I/O性能问题

**I/O性能诊断：**
```bash
# 查看I/O统计信息
sqlcc> SHOW ENGINE INNODB STATUS;

# 检查I/O线程状态
sqlcc> SELECT
    thread_id,
    name,
    type,
    process,
    state
  FROM performance_schema.threads
  WHERE name LIKE '%io%';

# 分析磁盘使用情况
iostat -x 1 5

# 检查存储子系统性能
fio --name=randread --rw=randread --bs=4k --size=1g --numjobs=8 --runtime=30
```

**I/O优化策略：**
```bash
# 调整I/O相关参数
sqlcc> SET GLOBAL innodb_io_capacity = 2000;
sqlcc> SET GLOBAL innodb_io_capacity_max = 4000;

# 启用I/O调度
sqlcc> SET GLOBAL innodb_adaptive_flushing = ON;
sqlcc> SET GLOBAL innodb_adaptive_flushing_lwm = 10;

# 配置RAID和存储
# 使用SSD存储
# 配置RAID 1+0
# 调整文件系统参数
echo "deadline" > /sys/block/sda/queue/scheduler
```

## 5. 内存问题诊断

### 5.1 内存泄漏排查

**内存使用监控：**
```bash
# 查看进程内存使用
ps aux --sort=-%mem | head

# 监控内存增长趋势
vmstat 1 10

# 查看系统内存信息
free -h
cat /proc/meminfo

# 检查swap使用情况
swapon -s
```

**数据库内存诊断：**
```bash
# 查看InnoDB内存使用
sqlcc> SELECT
    @@innodb_buffer_pool_size / 1024 / 1024 as buffer_pool_mb,
    @@innodb_log_buffer_size / 1024 / 1024 as log_buffer_mb,
    @@query_cache_size / 1024 / 1024 as query_cache_mb;

# 检查连接内存使用
sqlcc> SELECT
    user,
    host,
    db,
    command,
    time,
    state,
    info
  FROM information_schema.processlist
  WHERE time > 60; -- 长连接

# 分析临时表使用
sqlcc> SHOW GLOBAL STATUS LIKE 'Created_tmp%tables';
```

### 5.2 OOM问题排查

**Out of Memory诊断：**
```bash
# 检查系统日志
dmesg | grep -i "out of memory"

# 查看OOM killer记录
grep "Out of memory" /var/log/syslog

# 检查应用程序内存使用
valgrind --tool=massif --pages-as-heap=yes ./sqlcc

# 分析堆内存使用
gperftools-heap-profiler ./sqlcc
```

**内存优化策略：**
```bash
# 调整内存相关参数
sqlcc> SET GLOBAL innodb_buffer_pool_size = 1073741824; -- 1GB
sqlcc> SET GLOBAL query_cache_size = 67108864; -- 64MB
sqlcc> SET GLOBAL tmp_table_size = 134217728; -- 128MB
sqlcc> SET GLOBAL max_heap_table_size = 134217728; -- 128MB

# 启用内存监控
sqlcc> SET GLOBAL innodb_monitor_enable = 'all';

# 配置内存限制
# my.cnf
[mysqld]
innodb_buffer_pool_size = 1G
query_cache_size = 64M
tmp_table_size = 128M
max_heap_table_size = 128M
```

## 6. 网络问题诊断

### 6.1 网络连接问题

**连接稳定性检查：**
```bash
# 测试网络延迟
ping -c 10 database-server

# 检查丢包率
ping -f -c 100 database-server

# 分析网络流量
iftop -i eth0

# 检查DNS解析
nslookup database-server
dig database-server
```

**TCP连接调优：**
```bash
# 查看TCP连接状态
netstat -ant | awk '/:3306/ {print $6}' | sort | uniq -c

# 检查TIME_WAIT连接
netstat -ant | grep TIME_WAIT | wc -l

# 调整内核参数
echo "net.ipv4.tcp_tw_reuse = 1" >> /etc/sysctl.conf
echo "net.ipv4.tcp_tw_recycle = 1" >> /etc/sysctl.conf
echo "net.ipv4.ip_local_port_range = 1024 65535" >> /etc/sysctl.conf
sysctl -p
```

### 6.2 SSL/TLS连接问题

**SSL证书验证：**
```bash
# 检查证书有效性
openssl x509 -in /etc/sqlcc/ssl/server.crt -text -noout

# 验证证书链
openssl verify -CAfile /etc/sqlcc/ssl/ca.crt /etc/sqlcc/ssl/server.crt

# 测试SSL连接
openssl s_client -connect localhost:3306 -ssl3
openssl s_client -connect localhost:3306 -tls1
openssl s_client -connect localhost:3306 -tls1_2
```

**SSL性能优化：**
```bash
# 使用更快的加密算法
sqlcc> SET GLOBAL ssl_cipher = 'ECDHE-RSA-AES128-GCM-SHA256';

# 启用会话缓存
sqlcc> SET GLOBAL ssl_session_cache_mode = 'ON';
sqlcc> SET GLOBAL ssl_session_cache_size = 128;

# 配置证书文件
# my.cnf
[client]
ssl-ca = /etc/sqlcc/ssl/ca.crt
ssl-cert = /etc/sqlcc/ssl/client.crt
ssl-key = /etc/sqlcc/ssl/client.key
```

## 7. 复制与高可用问题诊断

### 7.1 主从复制故障

**复制状态检查：**
```bash
# 查看复制状态
sqlcc> SHOW SLAVE STATUS\G

# 检查主库状态
sqlcc> SHOW MASTER STATUS;

# 查看复制错误日志
tail -f /var/log/sqlcc/error.log | grep -i replicate

# 检查网络连接
telnet master-server 3306
```

**复制延迟诊断：**
```bash
# 查看复制延迟
sqlcc> SHOW SLAVE STATUS;

# 检查SQL线程状态
sqlcc> SHOW PROCESSLIST;

# 分析主库负载
sqlcc> SHOW ENGINE INNODB STATUS;
```

**复制修复步骤：**
```bash
# 停止复制
sqlcc> STOP SLAVE;

# 重置复制位置
sqlcc> RESET SLAVE;

# 重新配置复制
sqlcc> CHANGE MASTER TO
  MASTER_HOST='master-server',
  MASTER_USER='repl_user',
  MASTER_PASSWORD='password',
  MASTER_LOG_FILE='mysql-bin.000001',
  MASTER_LOG_POS=1;

# 启动复制
sqlcc> START SLAVE;

# 验证复制状态
sqlcc> SHOW SLAVE STATUS\G
```

### 7.2 集群故障转移问题

**故障转移诊断：**
```bash
# 检查集群状态
sqlcc-cluster status

# 查看节点状态
sqlcc-cluster nodes

# 检查网络分区
ping -c 3 cluster-node-1
ping -c 3 cluster-node-2

# 分析仲裁日志
tail -f /var/log/sqlcc/cluster.log
```

## 8. 备份恢复问题诊断

### 8.1 备份失败排查

**备份过程监控：**
```bash
# 查看备份进度
sqlcc-backup --status

# 检查备份日志
tail -f /var/log/sqlcc/backup.log

# 验证备份完整性
sqlcc-backup --verify backup_file.sql

# 检查磁盘空间
df -h /backup/directory
```

**备份性能优化：**
```bash
# 调整备份参数
sqlcc> SET GLOBAL innodb_max_dirty_pages_pct = 0; -- 刷新脏页

# 使用并行备份
mysqldump --single-transaction --routines --triggers --all-databases \
  --hex-blob --events --max_allowed_packet=1G \
  --quick --lock-tables=false > backup.sql

# 启用压缩备份
mysqldump --all-databases | gzip > backup.sql.gz
```

### 8.2 恢复失败排查

**恢复过程诊断：**
```bash
# 检查恢复脚本语法
sqlcc < backup.sql --check-syntax-only

# 分批恢复以定位问题
split -l 1000 backup.sql backup_part_
for part in backup_part_*; do
  echo "Restoring $part..."
  sqlcc < $part
  if [ $? -ne 0 ]; then
    echo "Failed at $part"
    break
  fi
done

# 检查恢复后的数据一致性
sqlcc> CHECK TABLE table_name;
sqlcc> REPAIR TABLE table_name;
```

## 9. 系统恢复策略

### 9.1 数据损坏恢复

**InnoDB崩溃恢复：**
```bash
# 检查InnoDB状态
sqlcc> SHOW ENGINE INNODB STATUS\G

# 强制恢复模式
# my.cnf
[mysqld]
innodb_force_recovery = 1  # 尝试级别1-6

# 重启服务
systemctl restart sqlcc

# 检查数据完整性
sqlcc> CHECK TABLE table_name;
```

**数据文件修复：**
```bash
# 使用innochecksum验证
innochecksum /var/lib/sqlcc/ibdata1

# 修复表空间
sqlcc> ALTER TABLE table_name ENGINE = InnoDB;

# 重建索引
sqlcc> REPAIR TABLE table_name QUICK;

# 导出导入修复
mysqldump database_name table_name > table.sql
sqlcc> DROP TABLE table_name;
sqlcc database_name < table.sql
```

### 9.2 紧急故障处理

**服务完全不可用时的处理：**
```bash
# 1. 备份所有数据文件（如果可能）
cp -r /var/lib/sqlcc /backup/emergency_backup

# 2. 检查系统日志
tail -n 100 /var/log/syslog
tail -n 100 /var/log/sqlcc/error.log

# 3. 尝试安全重启
systemctl stop sqlcc
systemctl start sqlcc

# 4. 如果失败，尝试最小配置启动
# my.cnf - 最小配置
[mysqld]
skip-grant-tables
skip-networking
innodb_force_recovery = 6

# 5. 数据导出
mysqldump --all-databases --skip-lock-tables > emergency_dump.sql

# 6. 重新安装或恢复
```

## 10. 监控与告警配置

### 10.1 关键指标监控

**性能指标：**
```bash
# CPU使用率 > 80%
# 内存使用率 > 90%
# 磁盘I/O等待 > 20%
# 连接数 > 80%最大值
# 慢查询 > 1秒
# 缓冲池命中率 < 95%
# 锁等待时间 > 1秒
```

**配置Nagios/Zabbix监控：**
```bash
# SQLCC状态检查脚本
#!/bin/bash
MYSQL_CMD="mysql -u monitor -p password -e"

# 检查连接
if ! $MYSQL_CMD "SELECT 1;" > /dev/null 2>&1; then
    echo "CRITICAL: Cannot connect to MySQL"
    exit 2
fi

# 检查复制延迟
SLAVE_LAG=$($MYSQL_CMD "SHOW SLAVE STATUS\G" | grep "Seconds_Behind_Master" | awk '{print $2}')
if [ "$SLAVE_LAG" != "NULL" ] && [ "$SLAVE_LAG" -gt 300 ]; then
    echo "WARNING: Slave lag is ${SLAVE_LAG} seconds"
    exit 1
fi

# 检查活跃连接数
CONNECTIONS=$($MYSQL_CMD "SHOW PROCESSLIST;" | wc -l)
if [ "$CONNECTIONS" -gt 100 ]; then
    echo "WARNING: High connection count: $CONNECTIONS"
    exit 1
fi

echo "OK: MySQL is healthy"
exit 0
```

### 10.2 日志分析与告警

**自动日志分析：**
```bash
# 错误日志监控脚本
#!/bin/bash
LOG_FILE="/var/log/sqlcc/error.log"
ALERT_EMAIL="dba@company.com"

# 检查新的错误
NEW_ERRORS=$(tail -n 100 $LOG_FILE | grep -i error | wc -l)

if [ "$NEW_ERRORS" -gt 0 ]; then
    echo "Found $NEW_ERRORS new errors in MySQL error log" | \
    mail -s "MySQL Error Alert" $ALERT_EMAIL

    # 发送详细日志
    tail -n 50 $LOG_FILE | mail -s "MySQL Error Details" $ALERT_EMAIL
fi

# 检查慢查询
SLOW_QUERIES=$(tail -n 100 /var/log/sqlcc/slow.log | wc -l)
if [ "$SLOW_QUERIES" -gt 10 ]; then
    echo "High number of slow queries: $SLOW_QUERIES" | \
    mail -s "MySQL Slow Query Alert" $ALERT_EMAIL
fi
```

## 11. 总结与最佳实践

### 11.1 故障预防策略

**定期维护：**
- 每日：监控关键指标
- 每周：检查备份完整性
- 每月：分析性能趋势
- 每季度：升级软件版本
- 半年：硬件维护和更换

**容量规划：**
- 监控资源使用趋势
- 预测未来增长需求
- 及时扩展系统容量
- 避免性能突然下降

### 11.2 快速诊断清单

**服务启动问题：**
- [ ] 检查系统资源（CPU、内存、磁盘）
- [ ] 验证配置文件语法
- [ ] 检查文件权限
- [ ] 查看错误日志
- [ ] 测试网络连接

**性能问题：**
- [ ] 识别慢查询
- [ ] 检查索引使用情况
- [ ] 分析锁等待
- [ ] 监控资源使用
- [ ] 优化查询和配置

**数据问题：**
- [ ] 验证备份完整性
- [ ] 检查数据一致性
- [ ] 分析日志文件
- [ ] 执行修复操作
- [ ] 恢复数据

### 11.3 紧急联系信息

**技术支持：**
- 内部DBA团队：dba@company.com
- 厂商技术支持：support@sqlcc.com
- 社区论坛：forum.sqlcc.com

**文档资源：**
- 在线文档：docs.sqlcc.com
- 故障排查指南：troubleshooting.sqlcc.com
- 最佳实践：best-practices.sqlcc.com

---

*文档创建时间: 2025-12-24*
*作者: SQLCC技术委员会*
*版本: v1.2.6*
*最后更新: 2025-12-24*
