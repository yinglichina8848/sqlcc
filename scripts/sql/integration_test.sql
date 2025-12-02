-- SQLCC 集成测试脚本
-- 包含基本的SQL操作测试

-- 设置测试环境
SET ECHO ON;

-- 1. 测试基本查询
SELECT '集成测试开始' AS message;
SELECT 1 AS test_result;

-- 2. 测试表创建
CREATE TABLE test_users (
    id INT PRIMARY KEY,
    username VARCHAR(50) NOT NULL,
    email VARCHAR(100) NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 3. 测试数据插入
INSERT INTO test_users (id, username, email) VALUES (1, 'user1', 'user1@example.com');
INSERT INTO test_users (id, username, email) VALUES (2, 'user2', 'user2@example.com');
INSERT INTO test_users (id, username, email) VALUES (3, 'user3', 'user3@example.com');

-- 4. 测试数据查询
SELECT * FROM test_users;
SELECT COUNT(*) AS user_count FROM test_users;
SELECT username, email FROM test_users WHERE id = 1;

-- 5. 测试数据更新
UPDATE test_users SET email = 'updated_user1@example.com' WHERE id = 1;
SELECT username, email FROM test_users WHERE id = 1;

-- 6. 测试数据删除
DELETE FROM test_users WHERE id = 3;
SELECT * FROM test_users;
SELECT COUNT(*) AS user_count FROM test_users;

-- 7. 测试条件查询
SELECT * FROM test_users WHERE username LIKE 'user%';

-- 8. 测试排序
SELECT * FROM test_users ORDER BY username DESC;

-- 9. 测试表删除
DROP TABLE test_users;

-- 10. 测试事务（如果支持）
BEGIN TRANSACTION;
CREATE TABLE test_transaction (
    id INT PRIMARY KEY,
    value VARCHAR(50)
);
INSERT INTO test_transaction (id, value) VALUES (1, 'test');
ROLLBACK;

-- 检查事务回滚是否成功
SELECT * FROM test_transaction;

-- 11. 测试聚合函数
CREATE TABLE test_products (
    id INT PRIMARY KEY,
    name VARCHAR(100),
    price DECIMAL(10, 2),
    category VARCHAR(50)
);

INSERT INTO test_products (id, name, price, category) VALUES (1, 'Product 1', 10.99, 'Category A');
INSERT INTO test_products (id, name, price, category) VALUES (2, 'Product 2', 19.99, 'Category A');
INSERT INTO test_products (id, name, price, category) VALUES (3, 'Product 3', 5.99, 'Category B');
INSERT INTO test_products (id, name, price, category) VALUES (4, 'Product 4', 29.99, 'Category B');

SELECT category, COUNT(*) AS product_count, AVG(price) AS avg_price FROM test_products GROUP BY category;

-- 12. 测试连接查询（如果支持）
CREATE TABLE test_orders (
    order_id INT PRIMARY KEY,
    user_id INT,
    product_id INT,
    quantity INT
);

INSERT INTO test_orders (order_id, user_id, product_id, quantity) VALUES (1, 1, 1, 2);
INSERT INTO test_orders (order_id, user_id, product_id, quantity) VALUES (2, 1, 3, 1);
INSERT INTO test_orders (order_id, user_id, product_id, quantity) VALUES (3, 2, 2, 3);

-- 清理测试表
DROP TABLE test_products;
DROP TABLE test_orders;

-- 13. 测试最终结果
SELECT '集成测试完成' AS message;
SELECT 1 AS final_result;

-- 关闭回显
SET ECHO OFF;