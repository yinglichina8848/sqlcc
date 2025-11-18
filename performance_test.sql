-- SQLCC CRUD性能测试脚本
-- 此脚本用于测试各种SQL操作的执行性能

-- 1. 创建和使用数据库测试
CREATE DATABASE performance_db;
USE performance_db;

-- 2. 创建表测试
CREATE TABLE test_users (
    id INT PRIMARY KEY,
    name VARCHAR(50),
    email VARCHAR(100),
    age INT,
    created_at DATETIME
);

CREATE TABLE test_products (
    id INT PRIMARY KEY,
    name VARCHAR(100),
    price DECIMAL(10,2),
    stock INT,
    category VARCHAR(50)
);

-- 3. INSERT操作性能测试 - 模拟多条插入
INSERT INTO test_users VALUES (1, 'User1', 'user1@example.com', 25, '2023-01-01 10:00:00');
INSERT INTO test_users VALUES (2, 'User2', 'user2@example.com', 30, '2023-01-02 11:00:00');
INSERT INTO test_users VALUES (3, 'User3', 'user3@example.com', 35, '2023-01-03 12:00:00');
INSERT INTO test_users VALUES (4, 'User4', 'user4@example.com', 40, '2023-01-04 13:00:00');
INSERT INTO test_users VALUES (5, 'User5', 'user5@example.com', 45, '2023-01-05 14:00:00');

INSERT INTO test_products VALUES (1, 'Product1', 19.99, 100, 'Category1');
INSERT INTO test_products VALUES (2, 'Product2', 29.99, 200, 'Category2');
INSERT INTO test_products VALUES (3, 'Product3', 39.99, 300, 'Category1');
INSERT INTO test_products VALUES (4, 'Product4', 49.99, 400, 'Category2');
INSERT INTO test_products VALUES (5, 'Product5', 59.99, 500, 'Category3');

-- 4. SELECT操作性能测试 - 测试不同查询复杂度
SELECT * FROM test_users;
SELECT id, name FROM test_users;
SELECT * FROM test_users WHERE id > 2;
SELECT * FROM test_users ORDER BY age DESC;

-- 5. UPDATE操作性能测试
UPDATE test_users SET age = age + 1;
UPDATE test_users SET name = 'UpdatedUser' WHERE id = 1;
UPDATE test_products SET price = price * 1.1;

-- 6. DELETE操作性能测试
DELETE FROM test_users WHERE id = 5;

-- 7. 组合操作测试 - 模拟实际应用场景中的连续操作
SELECT * FROM test_products WHERE stock > 150;
INSERT INTO test_users VALUES (6, 'NewUser', 'newuser@example.com', 28, '2023-01-06 15:00:00');
UPDATE test_users SET email = 'updated@example.com' WHERE id = 6;
SELECT id, name, email FROM test_users ORDER BY name;

-- 8. 数据库管理操作测试
SHOW TABLES;
DESCRIBE test_users;

-- 9. 清理测试数据
DROP TABLE test_users;
DROP TABLE test_products;
DROP DATABASE performance_db;

-- 性能测试脚本结束