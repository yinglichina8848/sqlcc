# SqlCC isql 演示指南

## 概述

本指南提供了一个完整的演示，展示如何使用 SqlCC 的 SQL 脚本测试功能。由于原始的 `isql` 工具需要完整编译项目，我们提供了一个简化版的 `simple_isql.py` 脚本，用于演示 SQL 命令的执行过程。

## 文件结构

```
/home/liying/sqlcc/scripts/sql/
├── comprehensive_test.sql    # 综合测试脚本，包含完整的DDL和DML操作
├── isql_usage_guide.md       # 详细的isql使用指南
├── getting_started.md        # 5分钟快速入门指南
├── simple_isql.py            # 简化版isql工具（Python实现）
├── demo_guide.md             # 本演示指南
└── test_script.sql           # 原始测试脚本
```

## 使用简化版isql工具

### 1. 基本使用方法

```bash
# 执行SQL脚本
python3 simple_isql.py -f comprehensive_test.sql

# 查看帮助信息
python3 simple_isql.py -h

# 查看版本信息
python3 simple_isql.py -v
```

### 2. 演示DDL操作

以下是在 `comprehensive_test.sql` 中的 DDL 操作示例：

```sql
-- 创建用户表
CREATE TABLE users (
    id INT PRIMARY KEY,
    username VARCHAR(50) NOT NULL,
    email VARCHAR(100),
    age INT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 创建产品表
CREATE TABLE products (
    product_id INT PRIMARY KEY,
    name VARCHAR(100) NOT NULL,
    price DECIMAL(10, 2) NOT NULL,
    stock INT DEFAULT 0,
    category VARCHAR(50)
);

-- 添加列到用户表
ALTER TABLE users ADD COLUMN address VARCHAR(200);
```

### 3. 演示DML操作

以下是 DML 操作示例：

```sql
-- 插入数据
INSERT INTO users (id, username, email, age) VALUES (1, 'zhangsan', 'zhangsan@example.com', 28);

-- 查询数据
SELECT * FROM users WHERE age > 30;

-- 更新数据
UPDATE users SET age = 29 WHERE id = 1;

-- 删除数据
DELETE FROM users WHERE id = 3;
```

## 执行演示

按照以下步骤执行完整演示：

1. **进入脚本目录**
   ```bash
   cd /home/liying/sqlcc/scripts/sql
   ```

2. **确保脚本可执行**
   ```bash
   chmod +x simple_isql.py
   ```

3. **执行综合测试脚本**
   ```bash
   python3 simple_isql.py -f comprehensive_test.sql
   ```

4. **观察执行结果**
   - 脚本将解析并执行所有SQL语句
   - 对于每种类型的SQL语句，会显示相应的执行信息
   - SELECT查询会模拟显示结果

## 示例执行输出

当执行 `comprehensive_test.sql` 时，您将看到类似以下的输出：

```
正在执行 SQL 脚本: comprehensive_test.sql
============================================================
找到 30 条 SQL 语句

语句 1: CREATE TABLE users (id INT PRIMARY KEY, username VARCHAR(50) NOT NULL, email VARCHAR(100), age...
[执行成功] 创建表: users

语句 2: CREATE TABLE products (product_id INT PRIMARY KEY, name VARCHAR(100) NOT NULL, price DE...
[执行成功] 创建表: products

...

语句 12: SELECT * FROM users;
[执行成功] SELECT 查询: SELECT * FROM users;...
[查询结果模拟显示]
+------------------+
| 模拟数据行       |
+------------------+
| 数据已成功查询   |
+------------------+

...

============================================================
SQL 脚本执行完成！共执行 30 条语句
```

## 原始isql工具使用说明

当您能够成功编译 SqlCC 项目后，可以使用原始的 `isql` 工具。基本用法如下：

### 编译 isql（在项目根目录）

```bash
mkdir -p build && cd build
cmake ..
make
```

### 使用原始 isql 工具

```bash
# 交互式模式
./bin/isql

# 执行SQL脚本
./bin/isql -f ../scripts/sql/comprehensive_test.sql

# 查看帮助
./bin/isql -h
```

### 交互式模式命令

在原始 `isql` 工具的交互式模式下，可以使用以下命令：

- `show tables` 或 `.schema` - 显示所有表
- `describe <table>` 或 `.desc <table>` - 显示表结构
- `show create table <table>` - 显示创建表的语句
- `exit` 或 `quit` - 退出程序
- `help` - 显示帮助信息

## 学习资源

- [完整使用指南](isql_usage_guide.md) - 详细的isql工具使用说明
- [快速入门指南](getting_started.md) - 5分钟内上手使用isql
- [综合测试脚本](comprehensive_test.sql) - 包含所有支持的SQL操作示例

## 注意事项

1. **简化版工具限制**：提供的 `simple_isql.py` 仅用于演示目的，不会实际执行SQL操作
2. **真实执行**：要实际执行SQL操作，需要使用完整编译的 `isql` 工具
3. **SQL语法**：所有SQL脚本使用标准SQL语法，确保兼容SqlCC引擎
4. **表名和列名**：使用时请注意大小写敏感性（取决于具体实现）

## 扩展建议

1. 创建自定义测试脚本，包含您特定的表结构和查询
2. 尝试编写更复杂的SQL查询和数据操作
3. 当项目完全编译后，使用真实的 `isql` 工具进行测试

祝您使用愉快！