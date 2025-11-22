-- SqlCC 全面测试脚本
-- 本脚本演示了基本的 DDL 和 DML 操作

-- DDL 操作：创建表

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

-- 创建订单表
CREATE TABLE orders (
    order_id INT PRIMARY KEY,
    user_id INT,
    order_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    status VARCHAR(20) DEFAULT 'pending',
    total_amount DECIMAL(10, 2) NOT NULL
);

-- DML 操作：数据操作语言

-- 1. INSERT 操作 - 插入数据
INSERT INTO users (id, username, email, age) VALUES (1, 'zhangsan', 'zhangsan@example.com', 28);
INSERT INTO users (id, username, email, age) VALUES (2, 'lisi', 'lisi@example.com', 32);
INSERT INTO users (id, username, email, age) VALUES (3, 'wangwu', 'wangwu@example.com', 45);

-- 插入产品数据
INSERT INTO products (product_id, name, price, stock, category) VALUES (1, '笔记本电脑', 5999.99, 50, '电子产品');
INSERT INTO products (product_id, name, price, stock, category) VALUES (2, '智能手机', 2999.99, 100, '电子产品');
INSERT INTO products (product_id, name, price, stock, category) VALUES (3, '无线耳机', 899.99, 200, '配件');

-- 插入订单数据
INSERT INTO orders (order_id, user_id, status, total_amount) VALUES (1, 1, 'completed', 5999.99);
INSERT INTO orders (order_id, user_id, status, total_amount) VALUES (2, 2, 'pending', 2999.99);
INSERT INTO orders (order_id, user_id, status, total_amount) VALUES (3, 1, 'processing', 899.99);

-- 2. SELECT 操作 - 查询数据

-- 查询所有用户
SELECT * FROM users;

-- 查询特定列
SELECT id, username, email FROM users;

-- 使用 WHERE 条件筛选
SELECT * FROM users WHERE age > 30;
SELECT * FROM products WHERE price < 3000;
SELECT * FROM orders WHERE status = 'pending';

-- 3. UPDATE 操作 - 更新数据
UPDATE users SET age = 29 WHERE id = 1;
UPDATE products SET stock = 150 WHERE product_id = 2;
UPDATE orders SET status = 'shipped' WHERE order_id = 2;

-- 更新后再次查询，验证更新效果
SELECT * FROM users WHERE id = 1;
SELECT * FROM products WHERE product_id = 2;
SELECT * FROM orders WHERE order_id = 2;

-- 4. DELETE 操作 - 删除数据
DELETE FROM users WHERE id = 3;

-- 删除后查询，验证删除效果
SELECT * FROM users;

-- 5. 表结构查看（使用 isql 内置命令）
-- .schema
-- .desc users
-- .desc products
-- .desc orders

-- 6. 高级查询示例

-- 连接查询（如果支持）
-- SELECT o.order_id, u.username, o.status, o.total_amount 
-- FROM orders o 
-- JOIN users u ON o.user_id = u.id;

-- DDL 操作：修改表

-- 添加列
ALTER TABLE users ADD COLUMN address VARCHAR(200);

-- 更新新列数据
UPDATE users SET address = '北京市海淀区' WHERE id = 1;
UPDATE users SET address = '上海市浦东新区' WHERE id = 2;

-- 查询更新后的表
SELECT * FROM users;

-- DDL 操作：删除表（谨慎使用）
-- DROP TABLE orders;
-- DROP TABLE products;
-- DROP TABLE users;

-- 测试完成
SELECT '测试脚本执行完成！' AS message;