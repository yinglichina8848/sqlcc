# SqlCC isql 工具使用指南

## 概述

`isql` 是 SqlCC 项目的交互式 SQL 命令行工具，允许用户执行 SQL 语句、运行 SQL 脚本以及查看数据库结构信息。本指南将详细介绍如何使用该工具进行基本的 DDL（数据定义语言）和 DML（数据操作语言）操作。

## 工具功能

- **交互式 SQL 命令执行**：实时输入和执行 SQL 语句
- **SQL 脚本执行**：通过文件批量执行 SQL 命令
- **数据库结构查看**：列出表、查看表结构等管理功能
- **支持的 SQL 命令**：CREATE TABLE、INSERT、SELECT、UPDATE、DELETE 等

## 基本使用方法

### 1. 启动交互式模式

```bash
./isql
```

启动后将显示欢迎信息，并等待用户输入命令。提示符为 `sqlcc>`。

### 2. 执行 SQL 脚本

使用 `-f` 参数指定 SQL 脚本文件路径：

```bash
./isql -f scripts/sql/comprehensive_test.sql
```

### 3. 查看帮助信息

```bash
./isql -h
# 或在交互式模式下输入
help
```

### 4. 查看版本信息

```bash
./isql -v
```

## 交互式命令

在交互式模式下，支持以下特殊命令：

| 命令 | 描述 |
|------|------|
| `exit`, `quit`, `.exit`, `.quit` | 退出程序 |
| `help` | 显示帮助信息 |
| `show tables`, `.schema` | 显示所有表 |
| `describe <table>`, `.desc <table>` | 显示表结构 |
| `show create table <table>` | 显示创建表的语句 |

## DDL 操作示例

### 创建表

```sql
CREATE TABLE users (
    id INT PRIMARY KEY,
    username VARCHAR(50) NOT NULL,
    email VARCHAR(100),
    age INT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
```

### 修改表结构

```sql
ALTER TABLE users ADD COLUMN address VARCHAR(200);
```

### 删除表

```sql
DROP TABLE users;
```

### 查看表结构

在交互式模式下：

```sql
.desc users
```

## DML 操作示例

### 插入数据（INSERT）

```sql
INSERT INTO users (id, username, email, age) VALUES (1, 'zhangsan', 'zhangsan@example.com', 28);
```

### 查询数据（SELECT）

- 查询所有列：
  ```sql
  SELECT * FROM users;
  ```

- 查询特定列：
  ```sql
  SELECT id, username, email FROM users;
  ```

- 使用 WHERE 条件：
  ```sql
  SELECT * FROM users WHERE age > 30;
  ```

### 更新数据（UPDATE）

```sql
UPDATE users SET age = 29 WHERE id = 1;
```

### 删除数据（DELETE）

```sql
DELETE FROM users WHERE id = 3;
```

## 执行测试脚本

我们提供了一个全面的测试脚本 `comprehensive_test.sql`，演示了基本的 DDL 和 DML 操作。要执行该脚本：

1. 确保 isql 工具已编译
2. 运行以下命令：

```bash
./isql -f scripts/sql/comprehensive_test.sql
```

## 注意事项

1. **SQL 语句结束**：交互式模式下，SQL 语句必须以分号（;）结尾才会执行
2. **多行输入**：如果 SQL 语句较长，可以多行输入，系统会在检测到分号后执行
3. **错误处理**：如果 SQL 语法错误，系统会显示错误信息，请检查语句后重新输入
4. **退出程序**：使用 `exit` 或 `quit` 命令退出交互式模式

## 进阶使用

### 执行自定义脚本

您可以创建自己的 SQL 脚本文件，然后使用 `-f` 参数执行：

1. 创建脚本文件，例如 `my_script.sql`
2. 编辑脚本，添加所需的 SQL 命令
3. 执行：`./isql -f my_script.sql`

### 查看表列表

在交互式模式下，使用以下命令查看所有表：

```sql
show tables;
-- 或
.schema
```

## 故障排除

如果遇到问题：

1. 检查 SQL 语句语法是否正确
2. 确保表名和列名正确无误
3. 验证数据类型是否匹配
4. 查看错误信息以获取更多详细信息

如果问题仍然存在，请检查 SqlCC 的编译和安装是否正确。

---

祝您使用愉快！如有任何问题，请参考 SqlCC 项目的文档或提交问题报告。