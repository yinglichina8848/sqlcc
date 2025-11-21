-- CRUD功能测试脚本
-- 测试创建数据库
CREATE DATABASE test_db;

-- 使用数据库
USE test_db;

-- 创建表
CREATE TABLE users (
    id INT PRIMARY KEY AUTO_INCREMENT,
    name VARCHAR(100) NOT NULL,
    email VARCHAR(100) NOT NULL UNIQUE,
    age INT,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- 再次创建表，测试IF NOT EXISTS
CREATE TABLE IF NOT EXISTS users (
    id INT PRIMARY KEY AUTO_INCREMENT,
    name VARCHAR(100) NOT NULL,
    email VARCHAR(100) NOT NULL UNIQUE,
    age INT,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- 创建产品表
CREATE TABLE products (
    product_id INT PRIMARY KEY AUTO_INCREMENT,
    product_name VARCHAR(200) NOT NULL,
    price DECIMAL(10, 2) NOT NULL,
    stock INT DEFAULT 0,
    category VARCHAR(50),
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- 插入数据
INSERT INTO users (name, email, age) VALUES 
('张三', 'zhangsan@example.com', 28),
('李四', 'lisi@example.com', 32),
('王五', 'wangwu@example.com', 45);

-- 插入更多产品数据
INSERT INTO products (product_name, price, stock, category) VALUES 
('笔记本电脑', 5999.99, 50, '电子产品'),
('智能手机', 2999.99, 100, '电子产品'),
('无线耳机', 499.99, 200, '电子产品'),
('办公椅', 899.99, 30, '家具'),
('办公桌', 1299.99, 20, '家具');

-- 查询数据 - 查询所有用户
SELECT * FROM users;

-- 查询特定列
SELECT name, email FROM users;

-- 使用WHERE子句
SELECT * FROM users WHERE age > 30;

-- 查询产品表
SELECT * FROM products WHERE category = '电子产品';

-- 更新数据
UPDATE users SET age = 35 WHERE name = '张三';

-- 批量更新
UPDATE products SET stock = stock + 50 WHERE category = '电子产品';

-- 再次查询以验证更新
SELECT * FROM users;
SELECT * FROM products;

-- 删除数据
DELETE FROM users WHERE name = '王五';

-- 再次查询以验证删除
SELECT * FROM users;

-- 修改表结构 - 添加列
ALTER TABLE users ADD COLUMN address VARCHAR(200);

-- 修改表结构 - 修改列
ALTER TABLE users MODIFY COLUMN address VARCHAR(300);

-- 修改表结构 - 重命名列
ALTER TABLE users CHANGE COLUMN address user_address VARCHAR(300);

-- 查看表结构
SHOW CREATE TABLE users;

-- 删除表
DROP TABLE IF EXISTS orders;

-- 创建订单表
CREATE TABLE orders (
    order_id INT PRIMARY KEY AUTO_INCREMENT,
    customer_id INT,
    product_id INT,
    quantity INT NOT NULL,
    order_date DATETIME DEFAULT CURRENT_TIMESTAMP,
    status VARCHAR(20) DEFAULT '待处理',
    FOREIGN KEY (customer_id) REFERENCES users(id),
    FOREIGN KEY (product_id) REFERENCES products(product_id)
);

-- 插入订单数据
INSERT INTO orders (customer_id, product_id, quantity) VALUES
(1, 1, 1),
(2, 3, 2);

-- 复杂查询 - 联表查询
SELECT o.order_id, u.name AS customer_name, p.product_name, o.quantity, o.status
FROM orders o
JOIN users u ON o.customer_id = u.id
JOIN products p ON o.product_id = p.product_id;

-- 删除表
DROP TABLE orders;
DROP TABLE users;
DROP TABLE products;

-- 删除数据库
USE default;
DROP DATABASE IF EXISTS test_db;

-- 测试完成