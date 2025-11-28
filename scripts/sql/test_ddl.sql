-- 测试DDL操作：创建、修改和删除数据库对象

-- 创建数据库
CREATE DATABASE test_db;

-- 使用数据库
USE test_db;

-- 创建表
CREATE TABLE test_users (
    id INT PRIMARY KEY,
    name VARCHAR(50),
    age INT,
    email VARCHAR(100)
);

-- 查看表结构
SHOW TABLES;
DESCRIBE test_users;

-- 修改表（添加列）
ALTER TABLE test_users ADD COLUMN created_at DATETIME;

-- 再次查看表结构
DESCRIBE test_users;

-- 删除表
DROP TABLE test_users;

-- 验证表已删除
SHOW TABLES;

-- 删除数据库
DROP DATABASE test_db;

-- 验证数据库已删除
SHOW DATABASES;