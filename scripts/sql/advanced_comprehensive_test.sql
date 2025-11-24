-- SqlCC 高级综合测试脚本
-- 包含：用户创建、数据库操作、复合表结构、复杂查询

-- 1. 用户管理操作
-- 创建新用户
CREATE USER test_user WITH PASSWORD 'test_password';

-- 授予权限
GRANT ALL PRIVILEGES TO test_user;

-- 2. 数据库管理（如果支持）
-- 注意：某些数据库系统中，数据库创建可能需要特定权限
CREATE DATABASE test_company_db;

-- 切换到新数据库
USE test_company_db;

-- 3. 创建复合表结构

-- 创建客户表
CREATE TABLE customers (
    customer_id INT PRIMARY KEY,
    first_name VARCHAR(50) NOT NULL,
    last_name VARCHAR(50) NOT NULL,
    email VARCHAR(100) UNIQUE NOT NULL,
    phone VARCHAR(20),
    address VARCHAR(200),
    city VARCHAR(50),
    country VARCHAR(50),
    registration_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 创建产品表
CREATE TABLE products (
    product_id INT PRIMARY KEY,
    product_name VARCHAR(100) NOT NULL,
    category VARCHAR(50) NOT NULL,
    price DECIMAL(10, 2) NOT NULL,
    stock_quantity INT DEFAULT 0,
    description TEXT,
    manufacturer VARCHAR(50)
);

-- 创建订单表
CREATE TABLE orders (
    order_id INT PRIMARY KEY,
    customer_id INT NOT NULL,
    order_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    total_amount DECIMAL(10, 2) NOT NULL,
    status VARCHAR(20) DEFAULT 'pending',
    shipping_address VARCHAR(200),
    shipping_city VARCHAR(50),
    shipping_country VARCHAR(50),
    FOREIGN KEY (customer_id) REFERENCES customers(customer_id)
);

-- 创建订单项表
CREATE TABLE order_items (
    item_id INT PRIMARY KEY,
    order_id INT NOT NULL,
    product_id INT NOT NULL,
    quantity INT NOT NULL DEFAULT 1,
    unit_price DECIMAL(10, 2) NOT NULL,
    item_total DECIMAL(10, 2) NOT NULL,
    FOREIGN KEY (order_id) REFERENCES orders(order_id),
    FOREIGN KEY (product_id) REFERENCES products(product_id)
);

-- 4. 插入测试数据

-- 插入客户数据
INSERT INTO customers (customer_id, first_name, last_name, email, phone, address, city, country)
VALUES 
(1, '张三', '李', 'zhangsan@example.com', '13800138001', '中关村大街1号', '北京', '中国'),
(2, '李四', '王', 'lisi@example.com', '13800138002', '南京东路123号', '上海', '中国'),
(3, 'John', 'Smith', 'john.smith@example.com', '+1234567890', '123 Main St', 'New York', 'USA'),
(4, 'Alice', 'Johnson', 'alice.j@example.com', '+1987654321', '456 Market Ave', 'San Francisco', 'USA'),
(5, '王五', '赵', 'wangwu@example.com', '13800138003', '天河路385号', '广州', '中国');

-- 插入产品数据
INSERT INTO products (product_id, product_name, category, price, stock_quantity, description, manufacturer)
VALUES 
(1, '高性能笔记本电脑', '电子产品', 8999.99, 50, '16GB内存，512GB固态硬盘', '品牌A'),
(2, '智能手机Pro', '电子产品', 4999.99, 100, '256GB存储，专业级相机', '品牌B'),
(3, '无线蓝牙耳机', '配件', 899.99, 200, '主动降噪，30小时续航', '品牌C'),
(4, '机械键盘', '配件', 399.99, 150, '青轴，RGB背光', '品牌D'),
(5, '游戏鼠标', '配件', 299.99, 180, '16000DPI，可编程按键', '品牌E'),
(6, '平板电脑', '电子产品', 3299.99, 80, '10.9英寸，256GB存储', '品牌A'),
(7, '智能手表', '电子产品', 1599.99, 120, '健康监测，防水设计', '品牌F');

-- 插入订单数据
INSERT INTO orders (order_id, customer_id, order_date, total_amount, status, shipping_address, shipping_city, shipping_country)
VALUES 
(101, 1, '2024-05-01 10:30:00', 10299.97, 'completed', '中关村大街1号', '北京', '中国'),
(102, 2, '2024-05-02 14:20:00', 5899.98, 'shipped', '南京东路123号', '上海', '中国'),
(103, 3, '2024-05-03 09:15:00', 1399.98, 'processing', '123 Main St', 'New York', 'USA'),
(104, 1, '2024-05-04 16:45:00', 3299.99, 'pending', '中关村大街1号', '北京', '中国'),
(105, 4, '2024-05-05 11:05:00', 1899.98, 'completed', '456 Market Ave', 'San Francisco', 'USA');

-- 插入订单项数据
INSERT INTO order_items (item_id, order_id, product_id, quantity, unit_price, item_total)
VALUES 
(1, 101, 1, 1, 8999.99, 8999.99),
(2, 101, 3, 1, 899.99, 899.99),
(3, 101, 4, 1, 399.99, 399.99),
(4, 102, 2, 1, 4999.99, 4999.99),
(5, 102, 5, 3, 299.99, 899.99),
(6, 103, 3, 1, 899.99, 899.99),
(7, 103, 5, 2, 299.99, 599.99),
(8, 104, 6, 1, 3299.99, 3299.99),
(9, 105, 7, 1, 1599.99, 1599.99),
(10, 105, 5, 1, 299.99, 299.99);

-- 5. 复杂查询示例

-- 5.1 基本JOIN查询
-- 查询所有订单及其对应的客户信息
SELECT o.order_id, o.order_date, o.total_amount, o.status,
       c.customer_id, c.first_name, c.last_name, c.email
FROM orders o
JOIN customers c ON o.customer_id = c.customer_id;

-- 5.2 多表JOIN查询
-- 查询订单详情，包括客户、产品和订单项信息
SELECT o.order_id, o.order_date, o.status,
       c.first_name || ' ' || c.last_name AS customer_name,
       p.product_name, p.category,
       oi.quantity, oi.unit_price, oi.item_total
FROM orders o
JOIN customers c ON o.customer_id = c.customer_id
JOIN order_items oi ON o.order_id = oi.order_id
JOIN products p ON oi.product_id = p.product_id
ORDER BY o.order_id, oi.item_id;

-- 5.3 分组聚合查询
-- 按客户统计订单总数和总消费金额
SELECT c.customer_id, c.first_name, c.last_name, 
       COUNT(o.order_id) AS order_count,
       SUM(o.total_amount) AS total_spent
FROM customers c
LEFT JOIN orders o ON c.customer_id = o.customer_id
GROUP BY c.customer_id, c.first_name, c.last_name
ORDER BY total_spent DESC;

-- 5.4 按产品类别统计销售情况
SELECT p.category,
       COUNT(oi.item_id) AS items_sold,
       SUM(oi.quantity) AS total_quantity,
       SUM(oi.item_total) AS revenue
FROM products p
JOIN order_items oi ON p.product_id = oi.product_id
GROUP BY p.category
ORDER BY revenue DESC;

-- 5.5 子查询示例
-- 查找购买了特定产品的所有客户
SELECT DISTINCT c.customer_id, c.first_name, c.last_name, c.email
FROM customers c
WHERE c.customer_id IN (
    SELECT o.customer_id 
    FROM orders o
    JOIN order_items oi ON o.order_id = oi.order_id
    WHERE oi.product_id = 3
);

-- 5.6 聚合过滤（HAVING）
-- 查找订单金额大于5000的客户
SELECT c.customer_id, c.first_name, c.last_name,
       COUNT(o.order_id) AS order_count,
       SUM(o.total_amount) AS total_spent
FROM customers c
JOIN orders o ON c.customer_id = o.customer_id
GROUP BY c.customer_id, c.first_name, c.last_name
HAVING SUM(o.total_amount) > 5000
ORDER BY total_spent DESC;

-- 5.7 按日期范围查询
-- 查询特定日期范围内的订单
SELECT o.order_id, o.order_date, o.total_amount, o.status,
       c.first_name, c.last_name
FROM orders o
JOIN customers c ON o.customer_id = c.customer_id
WHERE o.order_date BETWEEN '2024-05-01' AND '2024-05-03'
ORDER BY o.order_date;

-- 5.8 联合查询（UNION）
-- 联合查询高价值客户和高价值订单
SELECT '高价值客户' AS type, c.customer_id AS id, 
       c.first_name || ' ' || c.last_name AS name,
       SUM(o.total_amount) AS value
FROM customers c
JOIN orders o ON c.customer_id = o.customer_id
GROUP BY c.customer_id, c.first_name, c.last_name
HAVING SUM(o.total_amount) > 8000

UNION

SELECT '高价值订单' AS type, o.order_id AS id,
       '订单 #' || o.order_id AS name,
       o.total_amount AS value
FROM orders o
WHERE o.total_amount > 5000
ORDER BY value DESC;

-- 5.9 窗口函数（如果支持）
-- 计算每个客户的订单排名
SELECT c.customer_id, c.first_name, c.last_name,
       o.order_id, o.order_date, o.total_amount,
       ROW_NUMBER() OVER (PARTITION BY c.customer_id ORDER BY o.order_date) AS order_rank,
       SUM(o.total_amount) OVER (PARTITION BY c.customer_id) AS customer_total
FROM customers c
JOIN orders o ON c.customer_id = o.customer_id
ORDER BY c.customer_id, order_rank;

-- 5.10 复杂条件查询
-- 查询特定条件组合的订单
SELECT o.order_id, o.order_date, o.total_amount, o.status,
       c.first_name, c.last_name, c.country
FROM orders o
JOIN customers c ON o.customer_id = c.customer_id
JOIN order_items oi ON o.order_id = oi.order_id
JOIN products p ON oi.product_id = p.product_id
WHERE (
    (c.country = '中国' AND p.category = '电子产品') OR
    (c.country = 'USA' AND p.category = '配件')
) AND o.status = 'completed'
GROUP BY o.order_id, o.order_date, o.total_amount, o.status, c.first_name, c.last_name, c.country
ORDER BY o.total_amount DESC;

-- 6. 数据验证查询

-- 6.1 验证订单总金额是否等于订单项之和
SELECT o.order_id, o.total_amount,
       SUM(oi.item_total) AS calculated_total,
       CASE 
           WHEN o.total_amount = SUM(oi.item_total) THEN '匹配'
           ELSE '不匹配'
       END AS validation
FROM orders o
JOIN order_items oi ON o.order_id = oi.order_id
GROUP BY o.order_id, o.total_amount
ORDER BY validation, o.order_id;

-- 6.2 验证库存数量是否合理（假设库存应该大于已销售数量）
SELECT p.product_id, p.product_name, p.stock_quantity,
       SUM(oi.quantity) AS sold_quantity,
       CASE
           WHEN p.stock_quantity >= SUM(oi.quantity) THEN '库存充足'
           ELSE '库存不足'
       END AS inventory_status
FROM products p
LEFT JOIN order_items oi ON p.product_id = oi.product_id
GROUP BY p.product_id, p.product_name, p.stock_quantity
ORDER BY inventory_status, p.product_id;

-- 7. 事务操作示例（如果支持）
-- 注意：根据数据库系统不同，事务语法可能有所差异

-- 开始事务
-- BEGIN TRANSACTION;

-- 创建一个新订单及相关订单项
-- INSERT INTO orders (order_id, customer_id, total_amount, status, shipping_address, shipping_city, shipping_country)
-- VALUES (106, 3, 5299.98, 'pending', '123 Main St', 'New York', 'USA');

-- INSERT INTO order_items (item_id, order_id, product_id, quantity, unit_price, item_total)
-- VALUES (11, 106, 2, 1, 4999.99, 4999.99),
--        (12, 106, 5, 1, 299.99, 299.99);

-- 更新产品库存
-- UPDATE products SET stock_quantity = stock_quantity - 1 WHERE product_id = 2;
-- UPDATE products SET stock_quantity = stock_quantity - 1 WHERE product_id = 5;

-- 提交事务
-- COMMIT TRANSACTION;

-- 8. 清理测试数据（可选）
-- 注意：在实际测试环境中，根据需要决定是否执行清理
-- DROP TABLE order_items;
-- DROP TABLE orders;
-- DROP TABLE products;
-- DROP TABLE customers;
-- DROP DATABASE test_company_db;
-- DROP USER test_user;

-- 测试完成标记
SELECT '高级综合测试脚本执行完成！' AS message;
