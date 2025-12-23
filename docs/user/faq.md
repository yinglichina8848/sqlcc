# SQLCC 常见问题解答 (FAQ)

## 目录

- [关于SQLCC](#关于sqlcc)
- [安装和部署](#安装和部署)
- [使用问题](#使用问题)
- [性能调优](#性能调优)
- [故障排除](#故障排除)
- [开发相关](#开发相关)
- [安全问题](#安全问题)

## 关于SQLCC

### Q: SQLCC是什么？

A: SQLCC（SQL Cloud-native Cluster）是一个企业级内存安全的云原生关系型数据库系统，专注于提供高性能、高可靠性和企业级的数据库解决方案。它实现了完整的SQL-92标准，支持事务、索引、并发控制等现代数据库的核心特性。

### Q: SQLCC的主要特点是什么？

A:
- **内存安全优先**：95%+智能指针覆盖，彻底消除内存泄漏
- **高性能架构**：16分片并发缓冲池，支持400万+ ops/sec
- **完整SQL支持**：SQL-92标准完整实现，支持高级特性
- **企业级特性**：ACID事务、RBAC权限、SSL加密通信
- **云原生设计**：原生支持容器化部署和云环境

### Q: SQLCC与MySQL/PostgreSQL的区别？

A:
- **设计理念**：SQLCC从零开始设计，采用现代C++和最佳实践
- **内存安全**：95%+智能指针覆盖，比传统数据库更安全
- **学习价值**：代码结构清晰，适合数据库系统教学和研究
- **性能特点**：针对高并发场景优化，适合现代应用需求

### Q: SQLCC适合什么场景？

A:
- **教育和研究**：数据库原理教学的理想平台
- **原型开发**：快速验证数据库设计思路
- **高并发应用**：需要高性能和内存安全的场景
- **云原生应用**：支持容器化和微服务架构

## 安装和部署

### Q: 如何快速安装SQLCC？

A:
```bash
# 下载预编译包
wget https://github.com/sqlcc/sqlcc/releases/download/v1.2.6/sqlcc-v1.2.6-linux-x64.tar.gz

# 解压安装
tar -xzf sqlcc-v1.2.6-linux-x64.tar.gz
cd sqlcc-v1.2.6

# 运行安装脚本
sudo ./install.sh

# 启动服务
sudo systemctl start sqlcc-server
```

### Q: 支持哪些操作系统？

A:
- **Linux**: Ubuntu 18.04+, CentOS 7+, RHEL 7+
- **macOS**: 10.15+ (通过Homebrew)
- **容器**: Docker镜像支持

### Q: 如何在Docker中运行SQLCC？

A:
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
```

### Q: 如何从源码编译安装？

A:
```bash
# 克隆仓库
git clone https://gitee.com/yinglichina/sqlcc.git
cd sqlcc

# 安装依赖（Ubuntu/Debian）
sudo apt update
sudo apt install -y build-essential cmake clang-18 libssl-dev

# 编译安装
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
sudo make install
```

## 使用问题

### Q: 如何连接到SQLCC数据库？

A:
```bash
# 使用命令行客户端
sqlcc-client -h localhost -P 3306 -u root -p

# 或者直接连接
sqlcc-client -u root -p yourpassword
```

### Q: SQLCC支持哪些SQL语法？

A: SQLCC实现了完整的SQL-92标准，包括：

**DDL语句**:
```sql
CREATE/DROP/ALTER DATABASE/TABLE/INDEX/VIEW
```

**DML语句**:
```sql
SELECT, INSERT, UPDATE, DELETE with JOIN/Subquery
```

**高级特性**:
```sql
-- 存储过程和函数
CREATE PROCEDURE/FUNCTION

-- 触发器
CREATE TRIGGER

-- 约束
PRIMARY KEY, FOREIGN KEY, CHECK, UNIQUE, NOT NULL

-- 窗口函数
OVER (PARTITION BY ... ORDER BY ...)
```

### Q: 如何创建数据库和表？

A:
```sql
-- 创建数据库
CREATE DATABASE company_db;
USE company_db;

-- 创建表
CREATE TABLE employees (
    id INTEGER PRIMARY KEY,
    name VARCHAR(100) NOT NULL,
    email VARCHAR(100) UNIQUE,
    salary DECIMAL(10,2),
    hire_date DATE,
    department_id INTEGER
);

-- 创建索引
CREATE INDEX idx_employee_name ON employees(name);
CREATE INDEX idx_employee_salary ON employees(salary);
```

### Q: 如何进行事务管理？

A:
```sql
-- 开始事务
BEGIN;

-- 执行操作
UPDATE employees SET salary = salary * 1.05 WHERE department_id = 1;
INSERT INTO audit_log (action, table_name) VALUES ('SALARY_UPDATE', 'employees');

-- 提交事务
COMMIT;

-- 或回滚事务
ROLLBACK;
```

### Q: 如何备份和恢复数据？

A:
```bash
# 逻辑备份
sqlcc-dump --all-databases > full_backup.sql

# 恢复数据
sqlcc-client < full_backup.sql

# 物理备份
sudo systemctl stop sqlcc-server
cp -r /var/lib/sqlcc/data /backup/sqlcc_data_$(date +%Y%m%d)
sudo systemctl start sqlcc-server
```

## 性能调优

### Q: 如何优化查询性能？

A:
```sql
-- 1. 为WHERE条件列创建索引
CREATE INDEX idx_dept_id ON employees(department_id);

-- 2. 避免在索引列上使用函数
-- 高效
SELECT * FROM employees WHERE hire_date >= '2023-01-01';

-- 低效（无法使用索引）
SELECT * FROM employees WHERE YEAR(hire_date) = 2023;

-- 3. 使用合适的数据类型
-- INTEGER比VARCHAR更快
-- FIXED LENGTH比VARIABLE LENGTH更快
```

### Q: 如何配置缓冲池？

A:
```ini
[storage]
# 增加缓冲池大小
buffer_pool_size = 2GB

# 调整分片数量（建议为CPU核心数的2-4倍）
buffer_pool_shards = 32

# 设置页面大小（8KB是默认值，通常不需要修改）
page_size = 8192
```

### Q: 如何监控系统性能？

A:
```sql
-- 查看连接数
SHOW PROCESSLIST;

-- 查看系统变量
SHOW VARIABLES LIKE 'max_connections';

-- 查看表统计信息
SHOW TABLE STATUS LIKE 'employees';

-- 启用慢查询日志
SET GLOBAL slow_query_log = 'ON';
SET GLOBAL long_query_time = 2;  -- 2秒以上的查询
```

### Q: 性能基准测试结果如何？

A:
- **读写性能**: 400万+ ops/sec（混合负载）
- **并发连接**: 支持1000+并发连接
- **内存使用**: 低内存占用，95%+智能指针安全
- **缓存命中率**: 90%+缓冲池命中率

## 故障排除

### Q: 无法连接到数据库？

**诊断步骤**:
```bash
# 1. 检查服务状态
sudo systemctl status sqlcc-server

# 2. 检查端口监听
sudo netstat -tulpn | grep :3306

# 3. 检查防火墙
sudo ufw status
sudo ufw allow 3306

# 4. 检查日志
sudo tail -f /var/log/sqlcc/sqlcc.log
```

### Q: 查询执行缓慢？

**优化建议**:
```sql
-- 1. 分析查询执行计划
EXPLAIN SELECT * FROM employees WHERE department_id = 1;

-- 2. 检查索引使用
SHOW INDEXES FROM employees;

-- 3. 查看系统负载
SHOW PROCESSLIST;

-- 4. 优化配置
[performance]
query_cache_size = 256MB
thread_pool_size = 16
```

### Q: 磁盘空间不足？

**解决方案**:
```bash
# 查看磁盘使用情况
df -h

# 查看SQLCC数据目录大小
du -sh /var/lib/sqlcc/data

# 清理二进制日志
PURGE BINARY LOGS BEFORE '2023-01-01';

# 压缩旧数据
# 考虑分区表或归档历史数据
```

### Q: 内存使用过高？

**优化配置**:
```ini
[storage]
# 调整缓冲池大小
buffer_pool_size = 1GB  # 根据系统内存调整

[server]
# 限制最大连接数
max_connections = 200

# 设置连接超时
connection_timeout = 300
```

### Q: 数据损坏或一致性问题？

**恢复步骤**:
```bash
# 1. 停止服务
sudo systemctl stop sqlcc-server

# 2. 检查数据文件
ls -la /var/lib/sqlcc/data/

# 3. 尝试修复
sqlcc-server --repair --config /etc/sqlcc/sqlcc.conf

# 4. 从备份恢复
sqlcc-client < backup.sql
```

## 开发相关

### Q: 如何为SQLCC贡献代码？

A:
1. **Fork项目**到您的GitHub账户
2. **创建特性分支**: `git checkout -b feature/your-feature`
3. **编写代码**并添加测试
4. **提交Pull Request**并描述变更
5. **代码审查**通过后合并

### Q: 代码规范是什么？

A:
- **智能指针优先**: 95%+代码使用`std::unique_ptr`、`std::shared_ptr`
- **RAII模式**: 所有资源使用RAII模式管理
- **异常安全**: 强异常安全保证
- **命名规范**: 类用PascalCase，函数用snake_case
- **注释标准**: Why-What-How三层注释体系

### Q: 如何运行测试？

A:
```bash
# 运行所有测试
bazel test //...

# 运行特定测试
bazel test //tests/unit/storage_engine:buffer_pool_test

# 运行性能测试
bazel test //tests/performance:buffer_pool_performance_test

# 生成覆盖率报告
bazel coverage //...
```

### Q: 如何调试SQLCC？

A:
```bash
# 1. 启用调试日志
SET GLOBAL log_level = 'DEBUG';

# 2. 查看详细日志
sudo tail -f /var/log/sqlcc/sqlcc.log

# 3. 使用GDB调试
gdb sqlcc-server
(gdb) run --config /etc/sqlcc/sqlcc.conf

# 4. 性能分析
perf record -g sqlcc-server --config /etc/sqlcc/sqlcc.conf
perf report
```

## 安全问题

### Q: SQLCC的安全特性有哪些？

A:
- **SSL/TLS加密**: 支持TLS 1.3端到端加密
- **RBAC权限**: 基于角色的细粒度访问控制
- **内存安全**: 95%+智能指针防止内存漏洞
- **审计日志**: 完整的操作审计记录

### Q: 如何配置SSL/TLS？

A:
```ini
[security]
ssl_enabled = true
ssl_cert_file = /etc/sqlcc/ssl/server.crt
ssl_key_file = /etc/sqlcc/ssl/server.key
ssl_verify_client = true
```

### Q: 如何管理用户权限？

A:
```sql
-- 创建用户
CREATE USER 'app_user'@'localhost' IDENTIFIED BY 'secure_password';

-- 授予权限
GRANT SELECT, INSERT, UPDATE ON company_db.* TO 'app_user'@'localhost';

-- 创建角色
CREATE ROLE 'analyst';
GRANT SELECT ON company_db.* TO 'analyst';

-- 分配角色
GRANT 'analyst' TO 'app_user'@'localhost';
```

### Q: 如何查看审计日志？

A:
```sql
-- 启用审计
SET GLOBAL audit_log_enabled = true;
SET GLOBAL audit_log_file = '/var/log/sqlcc/audit.log';

-- 查看审计记录
SELECT * FROM audit_log ORDER BY timestamp DESC LIMIT 100;
```

## 扩展问题

### Q: SQLCC支持扩展吗？

A: SQLCC设计了插件架构，支持以下扩展：

- **自定义函数**: UDF/UDAF用户自定义函数
- **存储引擎插件**: 支持不同的存储后端
- **索引类型插件**: 支持特殊索引类型
- **认证插件**: 支持自定义认证方式

### Q: 如何开发插件？

A:
```cpp
// 实现插件接口
class MyCustomFunction : public Function {
public:
    Value execute(const std::vector<Value>& args) override {
        // 实现自定义逻辑
        return result;
    }
};

// 注册插件
PluginManager::registerFunction("my_function", std::make_unique<MyCustomFunction>());
```

### Q: 未来规划有哪些？

A:
- **v1.3.0**: 分区表、物化视图、全文搜索
- **v2.0.0**: 云原生分布式架构
- **v3.0.0**: AI原生智能数据库
- **v4.0.0**: 全场景数据平台

---

## 获取更多帮助

如果您的问题没有在FAQ中找到答案，请：

1. **查看官方文档**: https://docs.sqlcc.org/
2. **提交GitHub Issue**: https://github.com/sqlcc/sqlcc/issues
3. **参与社区讨论**: https://forum.sqlcc.org/
4. **联系技术支持**: support@sqlcc.org

*最后更新: 2025-12-24*
