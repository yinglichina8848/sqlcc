-- 测试DML操作：插入、更新、删除和查询数据

-- 创建测试数据库和表
CREATE DATABASE dml_test;
USE dml_test;

CREATE TABLE users (
    id INT PRIMARY KEY,
    name VARCHAR(50),
    age INT,
    email VARCHAR(100)
);

-- 测试INSERT操作
INSERT INTO users VALUES (1, 'Alice', 25, 'alice@example.com');
INSERT INTO users VALUES (2, 'Bob', 30, 'bob@example.com');
INSERT INTO users VALUES (3, 'Charlie', 35, 'charlie@example.com');

-- 测试SELECT操作
SELECT * FROM users;
SELECT name, age FROM users WHERE age > 25;

-- 测试UPDATE操作
UPDATE users SET age = 26 WHERE id = 1;
UPDATE users SET email = 'bob.new@example.com' WHERE name = 'Bob';

-- 测试SELECT操作（更新后）
SELECT * FROM users;

-- 测试DELETE操作
DELETE FROM users WHERE id = 3;

-- 测试SELECT操作（删除后）
SELECT * FROM users;

-- 清理测试数据
DROP TABLE users;
DROP DATABASE dml_test;
