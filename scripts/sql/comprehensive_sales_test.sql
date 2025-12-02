-- 综合测试脚本：销售系统SQL测试
-- 测试SQLCC系统对SQL-92标准命令的支持情况

-- ===========================================
-- 1. DCL测试：用户和角色管理
-- ===========================================

-- 创建角色
CREATE ROLE admin;
CREATE ROLE manager;
CREATE ROLE employee;
CREATE ROLE readonly;

-- 创建用户
CREATE USER admin_user IDENTIFIED BY 'admin_pass' ROLE admin;
CREATE USER sales_manager IDENTIFIED BY 'manager_pass' ROLE manager;
CREATE USER sales_rep IDENTIFIED BY 'rep_pass' ROLE employee;
CREATE USER accountant IDENTIFIED BY 'acct_pass' ROLE employee;
CREATE USER analyst IDENTIFIED BY 'analyst_pass' ROLE readonly;

-- 授予权限
GRANT ALL PRIVILEGES ON *.* TO admin_user;
GRANT SELECT, INSERT, UPDATE, DELETE ON sales.* TO sales_manager;
GRANT SELECT, INSERT ON sales.orders, sales.order_items TO sales_rep;
GRANT SELECT ON sales.*, inventory.* TO accountant;
GRANT SELECT ON sales.*, inventory.*, hr.* TO analyst;

-- 修改用户角色
ALTER USER sales_rep SET ROLE manager;

-- 修改角色名称
ALTER ROLE readonly RENAME TO report_viewer;

-- 设置当前角色
SET ROLE admin;

-- ===========================================
-- 2. DDL测试：数据库对象创建
-- ===========================================

-- 创建模式
CREATE SCHEMA sales;
CREATE SCHEMA hr;
CREATE SCHEMA inventory;

-- 创建表：部门表
CREATE TABLE hr.departments (
    id INT PRIMARY KEY,
    name VARCHAR(50) NOT NULL UNIQUE,
    manager_id INT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 创建表：员工表
CREATE TABLE hr.employees (
    id INT PRIMARY KEY,
    name VARCHAR(100) NOT NULL,
    email VARCHAR(100) NOT NULL UNIQUE,
    department_id INT NOT NULL,
    role VARCHAR(50) NOT NULL,
    hire_date DATE NOT NULL,
    FOREIGN KEY (department_id) REFERENCES hr.departments(id)
);

-- 创建表：客户表
CREATE TABLE sales.customers (
    id INT PRIMARY KEY,
    name VARCHAR(100) NOT NULL,
    email VARCHAR(100) NOT NULL UNIQUE,
    phone VARCHAR(20),
    address VARCHAR(200),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 创建表：产品表
CREATE TABLE inventory.products (
    id INT PRIMARY KEY,
    name VARCHAR(100) NOT NULL,
    price DECIMAL(10, 2) NOT NULL CHECK (price > 0),
    stock INT NOT NULL DEFAULT 0 CHECK (stock >= 0),
    category VARCHAR(50) NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 创建表：订单表
CREATE TABLE sales.orders (
    id INT PRIMARY KEY,
    customer_id INT NOT NULL,
    order_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    total_amount DECIMAL(10, 2) NOT NULL DEFAULT 0.00,
    status VARCHAR(20) NOT NULL DEFAULT 'pending',
    FOREIGN KEY (customer_id) REFERENCES sales.customers(id)
);

-- 创建表：订单项表
CREATE TABLE sales.order_items (
    order_id INT NOT NULL,
    product_id INT NOT NULL,
    quantity INT NOT NULL CHECK (quantity > 0),
    unit_price DECIMAL(10, 2) NOT NULL CHECK (unit_price > 0),
    subtotal DECIMAL(10, 2) NOT NULL CHECK (subtotal > 0),
    PRIMARY KEY (order_id, product_id),
    FOREIGN KEY (order_id) REFERENCES sales.orders(id),
    FOREIGN KEY (product_id) REFERENCES inventory.products(id)
);

-- 创建索引
CREATE INDEX idx_customers_email ON sales.customers(email);
CREATE INDEX idx_products_category ON inventory.products(category);
CREATE INDEX idx_orders_customer_id ON sales.orders(customer_id);
CREATE INDEX idx_orders_order_date ON sales.orders(order_date);
CREATE INDEX idx_order_items_order_id ON sales.order_items(order_id);
CREATE INDEX idx_order_items_product_id ON sales.order_items(product_id);

-- 创建视图：销售报告
CREATE VIEW sales.sales_report AS
SELECT 
    o.id AS order_id,
    o.order_date,
    c.name AS customer_name,
    c.email AS customer_email,
    o.total_amount,
    o.status
FROM sales.orders o
JOIN sales.customers c ON o.customer_id = c.id;

-- 创建视图：客户汇总
CREATE VIEW sales.customer_summary AS
SELECT 
    c.id,
    c.name,
    COUNT(o.id) AS total_orders,
    SUM(o.total_amount) AS total_spent
FROM sales.customers c
LEFT JOIN sales.orders o ON c.id = o.customer_id
GROUP BY c.id, c.name;

-- 创建视图：产品性能
CREATE VIEW inventory.product_performance AS
SELECT 
    p.id,
    p.name,
    p.category,
    SUM(oi.quantity) AS total_sold,
    SUM(oi.subtotal) AS total_revenue
FROM inventory.products p
LEFT JOIN sales.order_items oi ON p.id = oi.product_id
GROUP BY p.id, p.name, p.category;

-- 创建视图：员工部门
CREATE VIEW hr.employee_department AS
SELECT 
    e.id,
    e.name,
    e.email,
    d.name AS department,
    e.role,
    e.hire_date
FROM hr.employees e
JOIN hr.departments d ON e.department_id = d.id;

-- ===========================================
-- 3. DML测试：数据插入
-- ===========================================

-- 插入部门数据
INSERT INTO hr.departments (id, name) VALUES (1, 'Sales');
INSERT INTO hr.departments (id, name) VALUES (2, 'Marketing');
INSERT INTO hr.departments (id, name) VALUES (3, 'Finance');
INSERT INTO hr.departments (id, name) VALUES (4, 'Human Resources');
INSERT INTO hr.departments (id, name) VALUES (5, 'IT');

-- 插入员工数据
INSERT INTO hr.employees (id, name, email, department_id, role, hire_date) 
VALUES (1, 'John Doe', 'john.doe@company.com', 1, 'Manager', '2020-01-15');
INSERT INTO hr.employees (id, name, email, department_id, role, hire_date) 
VALUES (2, 'Jane Smith', 'jane.smith@company.com', 1, 'Sales Rep', '2021-03-20');
INSERT INTO hr.employees (id, name, email, department_id, role, hire_date) 
VALUES (3, 'Mike Johnson', 'mike.johnson@company.com', 2, 'Marketing Specialist', '2019-07-10');
INSERT INTO hr.employees (id, name, email, department_id, role, hire_date) 
VALUES (4, 'Sarah Williams', 'sarah.williams@company.com', 3, 'Accountant', '2022-01-05');
INSERT INTO hr.employees (id, name, email, department_id, role, hire_date) 
VALUES (5, 'Tom Brown', 'tom.brown@company.com', 5, 'IT Support', '2021-09-30');

-- 插入客户数据
INSERT INTO sales.customers (id, name, email, phone, address) 
VALUES (1, 'Acme Corporation', 'contact@acme.com', '555-1234', '123 Main St, Anytown USA');
INSERT INTO sales.customers (id, name, email, phone, address) 
VALUES (2, 'Globex Corporation', 'info@globex.com', '555-5678', '456 Oak Ave, Somecity USA');
INSERT INTO sales.customers (id, name, email, phone, address) 
VALUES (3, 'Umbrella Corporation', 'sales@umbrella.com', '555-9012', '789 Pine Rd, Othercity USA');
INSERT INTO sales.customers (id, name, email, phone, address) 
VALUES (4, 'Stark Industries', 'orders@stark.com', '555-3456', '321 Iron Way, Techville USA');
INSERT INTO sales.customers (id, name, email, phone, address) 
VALUES (5, 'Wayne Enterprises', 'support@wayne.com', '555-7890', '654 Bat Cave, Gotham USA');

-- 插入产品数据
INSERT INTO inventory.products (id, name, price, stock, category) 
VALUES (1, 'Laptop', 999.99, 50, 'Electronics');
INSERT INTO inventory.products (id, name, price, stock, category) 
VALUES (2, 'Smartphone', 699.99, 100, 'Electronics');
INSERT INTO inventory.products (id, name, price, stock, category) 
VALUES (3, 'Tablet', 499.99, 75, 'Electronics');
INSERT INTO inventory.products (id, name, price, stock, category) 
VALUES (4, 'Desk Chair', 199.99, 30, 'Furniture');
INSERT INTO inventory.products (id, name, price, stock, category) 
VALUES (5, 'Office Desk', 399.99, 20, 'Furniture');
INSERT INTO inventory.products (id, name, price, stock, category) 
VALUES (6, 'Monitor', 299.99, 40, 'Electronics');
INSERT INTO inventory.products (id, name, price, stock, category) 
VALUES (7, 'Keyboard', 89.99, 60, 'Electronics');
INSERT INTO inventory.products (id, name, price, stock, category) 
VALUES (8, 'Mouse', 49.99, 80, 'Electronics');
INSERT INTO inventory.products (id, name, price, stock, category) 
VALUES (9, 'Bookshelf', 149.99, 25, 'Furniture');
INSERT INTO inventory.products (id, name, price, stock, category) 
VALUES (10, 'Printer', 129.99, 35, 'Electronics');

-- 插入订单数据
INSERT INTO sales.orders (id, customer_id, order_date, total_amount, status) 
VALUES (1, 1, '2023-01-10', 1999.98, 'completed');
INSERT INTO sales.orders (id, customer_id, order_date, total_amount, status) 
VALUES (2, 2, '2023-01-15', 1199.97, 'completed');
INSERT INTO sales.orders (id, customer_id, order_date, total_amount, status) 
VALUES (3, 3, '2023-01-20', 799.98, 'pending');
INSERT INTO sales.orders (id, customer_id, order_date, total_amount, status) 
VALUES (4, 1, '2023-02-01', 599.97, 'completed');
INSERT INTO sales.orders (id, customer_id, order_date, total_amount, status) 
VALUES (5, 4, '2023-02-10', 2499.95, 'completed');

-- 插入订单项数据
-- 订单1：2台笔记本电脑
INSERT INTO sales.order_items (order_id, product_id, quantity, unit_price, subtotal) 
VALUES (1, 1, 2, 999.99, 1999.98);

-- 订单2：1台智能手机 + 1台平板电脑 + 1台显示器
INSERT INTO sales.order_items (order_id, product_id, quantity, unit_price, subtotal) 
VALUES (2, 2, 1, 699.99, 699.99);
INSERT INTO sales.order_items (order_id, product_id, quantity, unit_price, subtotal) 
VALUES (2, 3, 1, 499.99, 499.99);
INSERT INTO sales.order_items (order_id, product_id, quantity, unit_price, subtotal) 
VALUES (2, 6, 1, 299.99, 299.99);

-- 订单3：1台打印机 + 2个键盘 + 2个鼠标
INSERT INTO sales.order_items (order_id, product_id, quantity, unit_price, subtotal) 
VALUES (3, 10, 1, 129.99, 129.99);
INSERT INTO sales.order_items (order_id, product_id, quantity, unit_price, subtotal) 
VALUES (3, 7, 2, 89.99, 179.98);
INSERT INTO sales.order_items (order_id, product_id, quantity, unit_price, subtotal) 
VALUES (3, 8, 2, 49.99, 99.98);

-- 订单4：3个键盘
INSERT INTO sales.order_items (order_id, product_id, quantity, unit_price, subtotal) 
VALUES (4, 7, 3, 89.99, 269.97);

-- 订单5：5台智能手机
INSERT INTO sales.order_items (order_id, product_id, quantity, unit_price, subtotal) 
VALUES (5, 2, 5, 699.99, 3499.95);

-- 更新产品库存
UPDATE inventory.products SET stock = stock - 2 WHERE id = 1;
UPDATE inventory.products SET stock = stock - 1 WHERE id = 2;
UPDATE inventory.products SET stock = stock - 1 WHERE id = 3;
UPDATE inventory.products SET stock = stock - 1 WHERE id = 6;
UPDATE inventory.products SET stock = stock - 1 WHERE id = 10;
UPDATE inventory.products SET stock = stock - 2 WHERE id = 7;
UPDATE inventory.products SET stock = stock - 2 WHERE id = 8;
UPDATE inventory.products SET stock = stock - 3 WHERE id = 7;
UPDATE inventory.products SET stock = stock - 5 WHERE id = 2;

-- 更新订单状态
UPDATE sales.orders SET status = 'shipped' WHERE id = 3;

-- 删除过期订单（示例，实际没有过期订单）
DELETE FROM sales.orders WHERE order_date < '2022-01-01';

-- ===========================================
-- 4. DQL测试：数据查询和报告
-- ===========================================

-- 基本查询：所有客户
SELECT * FROM sales.customers;

-- 基本查询：所有产品
SELECT * FROM inventory.products;

-- 条件查询：电子类产品
SELECT * FROM inventory.products WHERE category = 'Electronics';

-- 条件查询：价格大于500的产品
SELECT * FROM inventory.products WHERE price > 500;

-- 排序查询：按价格降序排列的产品
SELECT * FROM inventory.products ORDER BY price DESC;

-- 聚合查询：总销售额
SELECT SUM(total_amount) AS total_sales FROM sales.orders;

-- 聚合查询：平均订单金额
SELECT AVG(total_amount) AS avg_order_amount FROM sales.orders;

-- 聚合查询：订单总数
SELECT COUNT(*) AS total_orders FROM sales.orders;

-- 分组查询：按状态分组的订单数
SELECT status, COUNT(*) AS order_count FROM sales.orders GROUP BY status;

-- 分组查询：按类别分组的产品数
SELECT category, COUNT(*) AS product_count FROM inventory.products GROUP BY category;

-- 连接查询：订单和客户信息
SELECT o.id, o.order_date, c.name, o.total_amount, o.status 
FROM sales.orders o 
JOIN sales.customers c ON o.customer_id = c.id;

-- 连接查询：订单和订单项
SELECT o.id, p.name, oi.quantity, oi.unit_price, oi.subtotal 
FROM sales.orders o 
JOIN sales.order_items oi ON o.id = oi.order_id 
JOIN inventory.products p ON oi.product_id = p.id;

-- 左连接查询：所有客户和他们的订单
SELECT c.name, o.id, o.order_date, o.total_amount 
FROM sales.customers c 
LEFT JOIN sales.orders o ON c.id = o.customer_id;

-- 子查询：销售额大于平均销售额的订单
SELECT * FROM sales.orders 
WHERE total_amount > (SELECT AVG(total_amount) FROM sales.orders);

-- 子查询：购买了特定产品的客户
SELECT DISTINCT c.name 
FROM sales.customers c 
JOIN sales.orders o ON c.id = o.customer_id 
JOIN sales.order_items oi ON o.id = oi.order_id 
WHERE oi.product_id = (SELECT id FROM inventory.products WHERE name = 'Laptop');

-- 视图查询：销售报告
SELECT * FROM sales.sales_report;

-- 视图查询：客户汇总
SELECT * FROM sales.customer_summary ORDER BY total_spent DESC;

-- 视图查询：产品性能
SELECT * FROM inventory.product_performance ORDER BY total_revenue DESC;

-- 视图查询：员工部门
SELECT * FROM hr.employee_department;

-- 复杂查询：按月份统计销售额
SELECT 
    EXTRACT(MONTH FROM order_date) AS month,
    COUNT(*) AS order_count,
    SUM(total_amount) AS monthly_sales
FROM sales.orders
GROUP BY EXTRACT(MONTH FROM order_date)
ORDER BY month;

-- 复杂查询：按客户和产品统计销售额
SELECT 
    c.name AS customer,
    p.name AS product,
    SUM(oi.quantity) AS quantity,
    SUM(oi.subtotal) AS revenue
FROM sales.customers c
JOIN sales.orders o ON c.id = o.customer_id
JOIN sales.order_items oi ON o.id = oi.order_id
JOIN inventory.products p ON oi.product_id = p.id
GROUP BY c.name, p.name
ORDER BY c.name, revenue DESC;

-- ===========================================
-- 5. 高级功能测试
-- ===========================================

-- 事务处理：创建新订单
BEGIN TRANSACTION;

-- 插入订单
INSERT INTO sales.orders (id, customer_id, order_date, total_amount, status) 
VALUES (6, 5, CURRENT_TIMESTAMP, 899.98, 'pending');

-- 插入订单项
INSERT INTO sales.order_items (order_id, product_id, quantity, unit_price, subtotal) 
VALUES (6, 1, 1, 999.99, 999.99);

-- 更新库存
UPDATE inventory.products SET stock = stock - 1 WHERE id = 1;

-- 提交事务
COMMIT;

-- 事务回滚示例
BEGIN TRANSACTION;

-- 插入测试订单
INSERT INTO sales.orders (id, customer_id, order_date, total_amount, status) 
VALUES (7, 1, CURRENT_TIMESTAMP, 100.00, 'pending');

-- 回滚事务
ROLLBACK;

-- ===========================================
-- 6. 清理测试
-- ===========================================

-- 删除视图
DROP VIEW sales.sales_report;
DROP VIEW sales.customer_summary;
DROP VIEW inventory.product_performance;
DROP VIEW hr.employee_department;

-- 删除索引
DROP INDEX idx_customers_email;
DROP INDEX idx_products_category;
DROP INDEX idx_orders_customer_id;
DROP INDEX idx_orders_order_date;
DROP INDEX idx_order_items_order_id;
DROP INDEX idx_order_items_product_id;

-- 删除表（按依赖顺序）
DROP TABLE sales.order_items;
DROP TABLE sales.orders;
DROP TABLE sales.customers;
DROP TABLE inventory.products;
DROP TABLE hr.employees;
DROP TABLE hr.departments;

-- 删除模式
DROP SCHEMA sales;
DROP SCHEMA hr;
DROP SCHEMA inventory;

-- 删除用户和角色
DROP USER admin_user;
DROP USER sales_manager;
DROP USER sales_rep;
DROP USER accountant;
DROP USER analyst;

DROP ROLE admin;
DROP ROLE manager;
DROP ROLE employee;
DROP ROLE report_viewer;

-- 测试结束标记
SELECT 'Comprehensive test completed successfully' AS test_result;