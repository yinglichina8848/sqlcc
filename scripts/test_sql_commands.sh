#!/bin/bash

# SQL命令测试脚本
# 用于验证各种SQL命令的执行

echo "=== SQLCC SQL命令执行验证脚本 ==="
echo ""

# 启动服务器 (在后台)
echo "1. 启动SQLCC服务器..."
# ./bazel-bin/sqlcc -p 18647 &
# SERVER_PID=$!
# sleep 2

echo "服务器启动命令 (请取消注释以实际运行):"
echo "./bazel-bin/sqlcc -p 18647 &"
echo ""

# 等待服务器启动
# sleep 3

echo "2. 测试各种SQL命令..."

# 测试CREATE DATABASE命令
echo "测试 CREATE DATABASE 命令:"
echo "./bazel-bin/isql -u admin -P password -e <<< \"CREATE DATABASE testdb;\""
echo ""

# 测试USE DATABASE命令
echo "测试 USE DATABASE 命令:"
echo "./bazel-bin/isql -u admin -P password -e <<< \"USE testdb;\""
echo ""

# 测试CREATE TABLE命令
echo "测试 CREATE TABLE 命令:"
echo "./bazel-bin/isql -u admin -P password -e <<< \"CREATE TABLE users (id INT PRIMARY KEY, name VARCHAR(50), email VARCHAR(100));\""
echo ""

# 测试INSERT命令
echo "测试 INSERT 命令:"
echo "./bazel-bin/isql -u admin -P password -e <<< \"INSERT INTO users (id, name, email) VALUES (1, 'Alice', 'alice@example.com');\""
echo "./bazel-bin/isql -u admin -P password -e <<< \"INSERT INTO users (id, name, email) VALUES (2, 'Bob', 'bob@example.com');\""
echo ""

# 测试SELECT命令
echo "测试 SELECT 命令:"
echo "./bazel-bin/isql -u admin -P password -e <<< \"SELECT * FROM users;\""
echo "./bazel-bin/isql -u admin -P password -e <<< \"SELECT name, email FROM users WHERE id = 1;\""
echo ""

# 测试UPDATE命令
echo "测试 UPDATE 命令:"
echo "./bazel-bin/isql -u admin -P password -e <<< \"UPDATE users SET email = 'alice.new@example.com' WHERE id = 1;\""
echo ""

# 测试DELETE命令
echo "测试 DELETE 命令:"
echo "./bazel-bin/isql -u admin -P password -e <<< \"DELETE FROM users WHERE id = 2;\""
echo ""

# 测试SHOW命令
echo "测试 SHOW 命令:"
echo "./bazel-bin/isql -u admin -P password -e <<< \"SHOW DATABASES;\""
echo "./bazel-bin/isql -u admin -P password -e <<< \"SHOW TABLES;\""
echo "./bazel-bin/isql -u admin -P password -e <<< \"SHOW COLUMNS FROM users;\""
echo ""

# 测试DCL命令 (权限管理)
echo "测试 DCL 命令 (权限管理):"
echo "./bazel-bin/isql -u admin -P password -e <<< \"CREATE USER 'testuser' IDENTIFIED BY 'password123';\""
echo "./bazel-bin/isql -u admin -P password -e <<< \"GRANT SELECT, INSERT ON testdb.users TO 'testuser';\""
echo "./bazel-bin/isql -u admin -P password -e <<< \"SHOW GRANTS FOR 'testuser';\""
echo "./bazel-bin/isql -u admin -P password -e <<< \"REVOKE INSERT ON testdb.users FROM 'testuser';\""
echo ""

# 测试DDL命令 (表结构修改)
echo "测试 DDL 命令 (表结构修改):"
echo "./bazel-bin/isql -u admin -P password -e <<< \"ALTER TABLE users ADD COLUMN age INT;\""
echo "./bazel-bin/isql -u admin -P password -e <<< \"CREATE INDEX idx_email ON users(email);\""
echo ""

echo "3. 关闭服务器"
echo "kill $SERVER_PID"

echo ""
echo "=== 测试完成 ==="
echo ""
echo "注意: 以上命令需要在成功编译sqlcc和isql程序后才能运行。"
echo "如果遇到编译问题，请检查系统的GCC和依赖库配置。"