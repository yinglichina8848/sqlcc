-- DDL（数据定义语言）测试脚本
-- 包含CREATE TABLE、ALTER TABLE、DROP TABLE、视图管理、模式管理等语句的测试用例

-- 测试1: 创建基本表
CREATE TABLE test_users (
    id INT PRIMARY KEY,
    name VARCHAR(50),
    email VARCHAR(100),
    age INT
);

-- 测试2: 创建带约束的表
CREATE TABLE test_orders (
    order_id INT PRIMARY KEY,
    user_id INT,
    order_date DATE,
    total_amount DECIMAL(10, 2),
    status VARCHAR(20),
    FOREIGN KEY (user_id) REFERENCES test_users(id)
);

-- 测试3: 创建带默认值的表
CREATE TABLE test_products (
    product_id INT PRIMARY KEY,
    product_name VARCHAR(100) NOT NULL,
    price DECIMAL(10, 2) DEFAULT 0.00,
    stock INT DEFAULT 0,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 测试4: 修改表结构 - 添加列
ALTER TABLE test_users ADD COLUMN address VARCHAR(200);

-- 测试5: 修改表结构 - 修改列
ALTER TABLE test_users MODIFY COLUMN email VARCHAR(150);

-- 测试6: 修改表结构 - 删除列
ALTER TABLE test_users DROP COLUMN age;

-- 测试7: 创建索引
CREATE INDEX idx_user_name ON test_users(name);

-- 测试8: 重命名表（使用ALTER TABLE）
ALTER TABLE test_products RENAME TO test_items;

-- 测试9: 重命名表（使用RENAME TABLE）
RENAME TABLE test_items TO test_products;

-- 测试10: 模式管理测试
-- 创建模式
CREATE SCHEMA sales;
CREATE SCHEMA hr;
CREATE SCHEMA inventory;

-- 修改模式
ALTER SCHEMA hr RENAME TO human_resources;

-- 测试11: 视图管理测试
-- 创建视图
CREATE VIEW test_user_orders AS
SELECT u.id, u.name, o.order_id, o.order_date, o.total_amount
FROM test_users u
JOIN test_orders o ON u.id = o.user_id;

-- 创建带模式的视图
CREATE VIEW sales.sales_report AS
SELECT u.id, u.name, o.order_id, o.order_date, o.total_amount
FROM test_users u
JOIN test_orders o ON u.id = o.user_id;

-- 修改视图
ALTER VIEW test_user_orders AS
SELECT u.id, u.name, u.email, o.order_id, o.order_date, o.total_amount, o.status
FROM test_users u
JOIN test_orders o ON u.id = o.user_id;

-- 测试12: 截断表
TRUNCATE TABLE test_orders;

-- 测试13: 列出所有表
SHOW TABLES;

-- 测试14: 查看表结构
SHOW CREATE TABLE test_users;

-- 测试15: 删除视图
DROP VIEW test_user_orders;
DROP VIEW sales.sales_report;

-- 测试16: 删除模式
DROP SCHEMA sales;
DROP SCHEMA human_resources;
DROP SCHEMA inventory;

-- 测试17: 删除表（按依赖顺序）
DROP TABLE test_orders;
DROP TABLE test_products;
DROP TABLE test_users;

-- 测试18: 创建复合主键表
CREATE TABLE test_order_items (
    order_id INT,
    product_id INT,
    quantity INT,
    price DECIMAL(10, 2),
    PRIMARY KEY (order_id, product_id)
);

-- 测试19: 测试表是否不存在
DROP TABLE IF EXISTS non_existent_table;

-- 测试结束标记
SELECT 'DDL测试完成' AS test_result;
