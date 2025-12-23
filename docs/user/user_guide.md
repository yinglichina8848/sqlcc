# SQLCC 用户指南

## 概述

欢迎使用SQLCC！SQLCC是一个企业级内存安全的云原生数据库系统，提供完整的SQL-92标准支持和高性能数据处理能力。本指南将帮助您快速上手并充分利用SQLCC的功能。

## 快速开始

### 系统要求

在开始使用SQLCC之前，请确保您的系统满足以下最低要求：

- **操作系统**: Linux Ubuntu 18.04+ / CentOS 7+ / macOS 10.15+
- **内存**: 最少4GB RAM（推荐8GB+）
- **存储**: 最少10GB可用空间
- **网络**: 支持TCP/IP网络连接

### 下载和安装

#### 方式1：使用预编译包（推荐）

```bash
# 下载最新版本
wget https://github.com/sqlcc/sqlcc/releases/download/v1.2.6/sqlcc-v1.2.6-linux-x64.tar.gz

# 解压安装
tar -xzf sqlcc-v1.2.6-linux-x64.tar.gz
cd sqlcc-v1.2.6

# 运行安装脚本
sudo ./install.sh
```

#### 方式2：使用包管理器

```bash
# Ubuntu/Debian
sudo apt update
sudo apt install sqlcc sqlcc-server sqlcc-client

# CentOS/RHEL
sudo yum install sqlcc sqlcc-server sqlcc-client

# macOS
brew install sqlcc/sqlcc/sqlcc
```

#### 方式3：从源码编译

```bash
git clone https://gitee.com/yinglichina/sqlcc.git
cd sqlcc
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
sudo make install
```

### 启动数据库服务

#### 使用systemd服务

```bash
# 启动服务
sudo systemctl start sqlcc-server

# 设置开机自启
sudo systemctl enable sqlcc-server

# 查看服务状态
sudo systemctl status sqlcc-server
```

#### 手动启动

```bash
# 前台运行（调试用）
sqlcc-server --config /etc/sqlcc/sqlcc.conf

# 后台运行
sqlcc-server --config /etc/sqlcc/sqlcc.conf --daemon
```

### 连接到数据库

```bash
# 使用命令行客户端连接
sqlcc-client -h localhost -P 3306 -u root -p

# 或者使用短参数
sqlcc-client -u root -p yourpassword
```

## 基本操作

### 数据库管理

#### 创建数据库

```sql
CREATE DATABASE company_db;
```

#### 查看数据库

```sql
-- 查看所有数据库
SHOW DATABASES;

-- 切换数据库
USE company_db;
```

#### 删除数据库

```sql
DROP DATABASE test_db;
```

### 表操作

#### 创建表

```sql
-- 创建员工表
CREATE TABLE employees (
    id INTEGER PRIMARY KEY,
    name VARCHAR(100) NOT NULL,
    email VARCHAR(100) UNIQUE,
    salary DECIMAL(10,2),
    hire_date DATE,
    department_id INTEGER
);

-- 创建部门表
CREATE TABLE departments (
    id INTEGER PRIMARY KEY,
    name VARCHAR(50) NOT NULL,
    location VARCHAR(100)
);
```

#### 查看表结构

```sql
-- 查看表结构
DESCRIBE employees;
-- 或者
SHOW COLUMNS FROM employees;

-- 查看所有表
SHOW TABLES;
```

#### 修改表结构

```sql
-- 添加列
ALTER TABLE employees ADD COLUMN phone VARCHAR(20);

-- 修改列类型
ALTER TABLE employees MODIFY COLUMN salary DECIMAL(12,2);

-- 删除列
ALTER TABLE employees DROP COLUMN phone;

-- 添加约束
ALTER TABLE employees ADD CONSTRAINT fk_dept
    FOREIGN KEY (department_id) REFERENCES departments(id);
```

### 数据操作

#### 插入数据

```sql
-- 插入单行数据
INSERT INTO departments (name, location) VALUES ('Engineering', 'Building A');

-- 插入多行数据
INSERT INTO employees (name, email, salary, hire_date, department_id) VALUES
    ('Alice Johnson', 'alice@company.com', 75000.00, '2023-01-15', 1),
    ('Bob Smith', 'bob@company.com', 65000.00, '2023-02-01', 1),
    ('Carol Davis', 'carol@company.com', 70000.00, '2023-01-20', 2);
```

#### 查询数据

```sql
-- 基本查询
SELECT * FROM employees;

-- 条件查询
SELECT name, salary FROM employees WHERE salary > 70000;

-- 排序查询
SELECT name, salary FROM employees ORDER BY salary DESC;

-- 分页查询
SELECT name, salary FROM employees ORDER BY salary DESC LIMIT 10 OFFSET 20;

-- 聚合查询
SELECT department_id, COUNT(*), AVG(salary) FROM employees GROUP BY department_id;

-- 连接查询
SELECT e.name, e.salary, d.name as department
FROM employees e
JOIN departments d ON e.department_id = d.id;
```

#### 更新数据

```sql
-- 更新单行
UPDATE employees SET salary = 80000 WHERE name = 'Alice Johnson';

-- 更新多行
UPDATE employees SET salary = salary * 1.05 WHERE department_id = 1;
```

#### 删除数据

```sql
-- 删除特定记录
DELETE FROM employees WHERE id = 1;

-- 删除满足条件的所有记录
DELETE FROM employees WHERE hire_date < '2023-01-01';
```

## 高级功能

### 索引管理

```sql
-- 创建索引
CREATE INDEX idx_employee_name ON employees(name);
CREATE INDEX idx_employee_salary ON employees(salary);
CREATE INDEX idx_employee_dept_salary ON employees(department_id, salary);

-- 查看索引
SHOW INDEXES FROM employees;

-- 删除索引
DROP INDEX idx_employee_name ON employees;
```

### 事务管理

```sql
-- 开始事务
BEGIN;

-- 执行操作
UPDATE employees SET salary = salary + 1000 WHERE department_id = 1;
INSERT INTO audit_log (action, table_name, user_id) VALUES ('SALARY_UPDATE', 'employees', 1);

-- 提交事务
COMMIT;

-- 或者回滚事务
ROLLBACK;
```

### 存储过程和函数

```sql
-- 创建存储过程
DELIMITER //
CREATE PROCEDURE update_employee_salary(
    IN emp_id INTEGER,
    IN new_salary DECIMAL(10,2)
)
BEGIN
    UPDATE employees SET salary = new_salary WHERE id = emp_id;
    INSERT INTO salary_history (employee_id, old_salary, new_salary, change_date)
    SELECT emp_id, salary, new_salary, NOW() FROM employees WHERE id = emp_id;
END //
DELIMITER ;

-- 调用存储过程
CALL update_employee_salary(1, 85000.00);

-- 创建函数
DELIMITER //
CREATE FUNCTION calculate_bonus(salary DECIMAL(10,2)) RETURNS DECIMAL(10,2)
BEGIN
    RETURN salary * 0.1;
END //
DELIMITER ;

-- 使用函数
SELECT name, salary, calculate_bonus(salary) as bonus FROM employees;
```

### 触发器

```sql
-- 创建触发器
DELIMITER //
CREATE TRIGGER audit_employee_changes
AFTER UPDATE ON employees
FOR EACH ROW
BEGIN
    INSERT INTO audit_log (table_name, action, employee_id, old_value, new_value, change_time)
    VALUES ('employees', 'UPDATE', NEW.id,
            CONCAT('salary:', OLD.salary),
            CONCAT('salary:', NEW.salary),
            NOW());
END //
DELIMITER ;
```

### 视图

```sql
-- 创建视图
CREATE VIEW employee_details AS
SELECT e.id, e.name, e.email, e.salary, d.name as department, d.location
FROM employees e
JOIN departments d ON e.department_id = d.id;

-- 使用视图
SELECT * FROM employee_details WHERE department = 'Engineering';
```

### 用户和权限管理

```sql
-- 创建用户
CREATE USER 'app_user'@'localhost' IDENTIFIED BY 'secure_password';

-- 授予权限
GRANT SELECT, INSERT, UPDATE ON company_db.* TO 'app_user'@'localhost';

-- 创建角色
CREATE ROLE 'analyst';
GRANT SELECT ON company_db.* TO 'analyst';

-- 分配角色给用户
GRANT 'analyst' TO 'app_user'@'localhost';

-- 查看权限
SHOW GRANTS FOR 'app_user'@'localhost';

-- 撤销权限
REVOKE INSERT, UPDATE ON company_db.* FROM 'app_user'@'localhost';
```

## 性能优化

### 查询优化

#### 使用EXPLAIN分析查询

```sql
EXPLAIN SELECT * FROM employees WHERE department_id = 1;

-- 输出示例：
-- TABLE: employees
-- INDEX: idx_dept_id (USED)
-- COST: 15.5
-- ROWS: 25
```

#### 优化建议

1. **为WHERE条件列创建索引**
```sql
CREATE INDEX idx_dept_id ON employees(department_id);
```

2. **避免在索引列上使用函数**
```sql
-- 低效
SELECT * FROM employees WHERE YEAR(hire_date) = 2023;

-- 高效
SELECT * FROM employees WHERE hire_date >= '2023-01-01' AND hire_date < '2024-01-01';
```

3. **使用合适的数据类型**
```sql
-- 为频繁查询的列选择合适类型
-- INTEGER 比 VARCHAR 更快
-- FIXED LENGTH 比 VARIABLE LENGTH 更快
```

### 配置优化

#### 缓冲池配置

```ini
[storage]
# 增加缓冲池大小
buffer_pool_size = 2GB

# 调整分片数量
buffer_pool_shards = 32
```

#### 连接配置

```ini
[server]
# 增加最大连接数
max_connections = 200

# 设置连接超时
connection_timeout = 300
```

### 监控和诊断

#### 查看系统状态

```sql
-- 查看连接数
SHOW PROCESSLIST;

-- 查看系统变量
SHOW VARIABLES LIKE 'max_connections';

-- 查看表统计信息
SHOW TABLE STATUS LIKE 'employees';
```

#### 性能监控

```sql
-- 查看查询执行时间
SET profiling = 1;
SELECT * FROM employees WHERE salary > 50000;
SHOW PROFILES;

-- 查看索引使用情况
SELECT * FROM performance_schema.table_io_waits_summary_by_index_usage
WHERE object_schema = 'company_db' AND object_name = 'employees';
```

## 备份和恢复

### 数据备份

#### 逻辑备份

```bash
# 备份整个数据库
sqlcc-dump --all-databases > full_backup.sql

# 备份特定数据库
sqlcc-dump company_db > company_backup.sql

# 备份特定表
sqlcc-dump company_db employees departments > tables_backup.sql
```

#### 物理备份

```bash
# 停止服务
sudo systemctl stop sqlcc-server

# 复制数据目录
cp -r /var/lib/sqlcc/data /backup/sqlcc_data_$(date +%Y%m%d)

# 重启服务
sudo systemctl start sqlcc-server
```

### 数据恢复

#### 从逻辑备份恢复

```bash
# 创建新数据库
sqlcc-client -e "CREATE DATABASE restored_db;"

# 恢复数据
sqlcc-client restored_db < company_backup.sql
```

#### 增量恢复

```sql
-- 使用二进制日志进行时间点恢复
sqlcc-client < backup.sql
mysqlbinlog binlog.000001 binlog.000002 | sqlcc-client
```

## 安全配置

### SSL/TLS配置

```ini
[security]
# 启用SSL
ssl_enabled = true

# SSL证书路径
ssl_cert_file = /etc/sqlcc/ssl/server.crt
ssl_key_file = /etc/sqlcc/ssl/server.key

# 要求客户端证书
ssl_verify_client = true
```

### 密码策略

```sql
-- 设置密码复杂度要求
SET GLOBAL password_min_length = 12;
SET GLOBAL password_complexity = 'HIGH';

-- 密码过期策略
ALTER USER 'app_user'@'localhost' PASSWORD EXPIRE INTERVAL 90 DAY;
```

### 审计配置

```sql
-- 启用审计日志
SET GLOBAL audit_log_enabled = true;
SET GLOBAL audit_log_file = '/var/log/sqlcc/audit.log';

-- 记录所有DDL操作
SET GLOBAL audit_log_include_commands = 'CREATE,ALTER,DROP';
```

## 故障排除

### 常见问题

#### 连接问题

**问题**: `ERROR 2003 (HY000): Can't connect to MySQL server`

**解决方案**:
```bash
# 检查服务状态
sudo systemctl status sqlcc-server

# 检查端口监听
sudo netstat -tulpn | grep :3306

# 检查防火墙
sudo ufw status
sudo ufw allow 3306
```

#### 性能问题

**问题**: 查询执行缓慢

**诊断步骤**:
```sql
-- 1. 分析查询执行计划
EXPLAIN SELECT * FROM large_table WHERE column = 'value';

-- 2. 检查索引使用
SHOW INDEXES FROM large_table;

-- 3. 查看系统负载
SHOW PROCESSLIST;
SHOW ENGINE INNODB STATUS;
```

#### 存储空间问题

**问题**: 磁盘空间不足

**解决方案**:
```bash
# 查看磁盘使用情况
df -h

# 查看SQLCC数据目录大小
du -sh /var/lib/sqlcc/data

# 清理二进制日志
PURGE BINARY LOGS BEFORE '2023-01-01';
```

### 日志分析

#### 查看错误日志

```bash
# 查看错误日志
sudo tail -f /var/log/sqlcc/sqlcc.log

# 搜索特定错误
grep "ERROR" /var/log/sqlcc/sqlcc.log | tail -20
```

#### 慢查询日志

```sql
-- 启用慢查询日志
SET GLOBAL slow_query_log = 'ON';
SET GLOBAL long_query_time = 2;  -- 2秒以上的查询

-- 查看慢查询
SELECT * FROM mysql.slow_log ORDER BY query_time DESC LIMIT 10;
```

## 最佳实践

### 数据库设计

1. **规范化设计**: 遵循第三范式，避免数据冗余
2. **索引策略**: 为WHERE、JOIN、ORDER BY列创建索引
3. **数据类型选择**: 使用最合适的数据类型节省空间

### 应用开发

1. **连接池**: 使用连接池避免频繁创建连接
2. **事务管理**: 合理使用事务，确保数据一致性
3. **错误处理**: 妥善处理数据库异常和连接问题

### 运维管理

1. **定期备份**: 制定备份策略，定期验证备份有效性
2. **监控告警**: 设置关键指标监控和告警机制
3. **性能调优**: 定期分析慢查询，优化系统配置

## 扩展阅读

- [SQLCC架构设计](design/Architecture.md) - 系统架构详细说明
- [编码规范](code/coding_standards.md) - 开发规范和最佳实践
- [性能基准测试](features/performance_benchmarks.md) - 性能测试结果
- [安全指南](security/security_architecture.md) - 安全配置和最佳实践

---

*最后更新: 2025-12-23*

如果您在使用过程中遇到问题，请查看[故障排除](user/troubleshooting.md)或联系技术支持。
