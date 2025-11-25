-- 测试数据库操作持久性
-- 尝试使用之前创建的数据库
USE test_db;

-- 检查数据库是否存在，如果存在则查询表和数据
SHOW TABLES;

-- 尝试查询之前可能创建的表
SELECT * FROM users;

-- 如果上述操作失败，创建新的数据库和表
CREATE DATABASE IF NOT EXISTS test_db;
USE test_db;

CREATE TABLE IF NOT EXISTS persistent_users (
    id INT PRIMARY KEY,
    name VARCHAR(50),
    email VARCHAR(100)
);

-- 插入一些测试数据
INSERT INTO persistent_users VALUES (1, 'Alice', 'alice@example.com');
INSERT INTO persistent_users VALUES (2, 'Bob', 'bob@example.com');

-- 验证数据插入
SELECT * FROM persistent_users;
