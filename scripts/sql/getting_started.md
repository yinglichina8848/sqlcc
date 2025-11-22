# SqlCC isql 快速入门指南

## 5分钟上手

本指南将帮助您在5分钟内快速上手使用 SqlCC 的 `isql` 交互式 SQL 工具。

## 步骤1：编译 SqlCC

假设您已经克隆了 SqlCC 项目，首先需要编译项目以生成 `isql` 工具：

```bash
cd /home/liying/sqlcc
mkdir -p build && cd build
cmake ..
make
```

编译成功后，`isql` 可执行文件将位于 `build/bin` 目录下。

## 步骤2：运行 isql

### 方法A：交互式模式

```bash
./bin/isql
```

进入交互式环境后，您将看到如下提示符：

```
Welcome to isql (SqlCC [版本号])
Type 'help' for help, 'exit' or 'quit' to exit.

sqlcc>
```

### 方法B：执行测试脚本

```bash
./bin/isql -f ../scripts/sql/comprehensive_test.sql
```

## 步骤3：基本操作示例

### 查看所有表

```sql
sqlcc> show tables;
```

### 创建表

```sql
sqlcc> CREATE TABLE test_users (id INT, name VARCHAR(50));
```

### 插入数据

```sql
sqlcc> INSERT INTO test_users VALUES (1, '测试用户');
```

### 查询数据

```sql
sqlcc> SELECT * FROM test_users;
```

### 更新数据

```sql
sqlcc> UPDATE test_users SET name = '新用户名' WHERE id = 1;
```

### 删除数据

```sql
sqlcc> DELETE FROM test_users WHERE id = 1;
```

### 查看表结构

```sql
sqlcc> .desc test_users
```

### 退出程序

```sql
sqlcc> exit
```

## 快速参考

| 操作类型 | 示例命令 |
|---------|--------|
| 执行脚本 | `./bin/isql -f 脚本文件路径` |
| 交互式模式 | `./bin/isql` |
| 查看帮助 | `help` |
| 查看表 | `show tables` |
| 查看表结构 | `.desc 表名` |
| 退出 | `exit` 或 `quit` |

## 下一步

- 查看完整的 [使用指南](isql_usage_guide.md) 获取更多详细信息
- 运行 [综合测试脚本](comprehensive_test.sql) 了解所有支持的功能
- 尝试创建自己的 SQL 脚本进行测试

祝您使用愉快！