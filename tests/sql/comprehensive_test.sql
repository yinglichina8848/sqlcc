-- SQLCC Comprehensive Test Suite with Verification
-- This file contains comprehensive SQL tests with verification queries

-- ============================================================================
-- PHASE 1: Database and Table Creation with Verification
-- ============================================================================

-- Database creation and verification
CREATE DATABASE test_db;
USE test_db;
-- Verify database selection
SELECT DATABASE();

-- Table creation and verification
CREATE TABLE users (
    id INTEGER PRIMARY KEY,
    name VARCHAR(50) NOT NULL,
    email VARCHAR(100) UNIQUE,
    age INTEGER,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
-- Verify table creation
SHOW TABLES;
DESCRIBE users;

CREATE TABLE orders (
    order_id INTEGER PRIMARY KEY,
    user_id INTEGER,
    product_name VARCHAR(100),
    quantity INTEGER,
    price DECIMAL(10,2),
    order_date DATE
);
-- Verify table creation
SHOW TABLES;

CREATE TABLE products (
    product_id INTEGER PRIMARY KEY,
    name VARCHAR(100) NOT NULL,
    category VARCHAR(50),
    price DECIMAL(10,2),
    stock_quantity INTEGER
);
-- Verify all tables exist
SHOW TABLES;

-- ============================================================================
-- PHASE 2: Index Creation with Verification
-- ============================================================================

CREATE INDEX idx_users_email ON users(email);
CREATE INDEX idx_orders_user_id ON orders(user_id);
CREATE INDEX idx_products_category ON products(category);
-- Note: No direct way to verify indexes in basic SQL, will test via query performance

-- ============================================================================
-- PHASE 3: Data Insertion with Verification
-- ============================================================================

-- Insert users and verify
INSERT INTO users (id, name, email, age) VALUES (1, 'Alice Johnson', 'alice@example.com', 25);
INSERT INTO users (id, name, email, age) VALUES (2, 'Bob Smith', 'bob@example.com', 30);
INSERT INTO users (id, name, email, age) VALUES (3, 'Charlie Brown', 'charlie@example.com', 35);
INSERT INTO users (id, name, email, age) VALUES (4, 'Diana Prince', 'diana@example.com', 28);
INSERT INTO users (id, name, email, age) VALUES (5, 'Eve Wilson', 'eve@example.com', 32);
-- Verify user insertions
SELECT COUNT(*) as user_count FROM users;
SELECT * FROM users ORDER BY id;

-- Insert products and verify
INSERT INTO products (product_id, name, category, price, stock_quantity) VALUES (1, 'Laptop', 'Electronics', 999.99, 50);
INSERT INTO products (product_id, name, category, price, stock_quantity) VALUES (2, 'Mouse', 'Electronics', 29.99, 200);
INSERT INTO products (product_id, name, category, price, stock_quantity) VALUES (3, 'Book', 'Education', 19.99, 150);
INSERT INTO products (product_id, name, category, price, stock_quantity) VALUES (4, 'Chair', 'Furniture', 149.99, 75);
INSERT INTO products (product_id, name, category, price, stock_quantity) VALUES (5, 'Headphones', 'Electronics', 79.99, 100);
-- Verify product insertions
SELECT COUNT(*) as product_count FROM products;
SELECT name, category, price FROM products ORDER BY product_id;

-- Insert orders and verify
INSERT INTO orders (order_id, user_id, product_name, quantity, price, order_date) VALUES (1, 1, 'Laptop', 1, 999.99, '2025-01-15');
INSERT INTO orders (order_id, user_id, product_name, quantity, price, order_date) VALUES (2, 2, 'Mouse', 2, 59.98, '2025-01-16');
INSERT INTO orders (order_id, user_id, product_name, quantity, price, order_date) VALUES (3, 3, 'Book', 3, 59.97, '2025-01-17');
INSERT INTO orders (order_id, user_id, product_name, quantity, price, order_date) VALUES (4, 1, 'Headphones', 1, 79.99, '2025-01-18');
INSERT INTO orders (order_id, user_id, product_name, quantity, price, order_date) VALUES (5, 4, 'Chair', 2, 299.98, '2025-01-19');
-- Verify order insertions
SELECT COUNT(*) as order_count FROM orders;
SELECT user_id, product_name, quantity, price FROM orders ORDER BY order_id;

-- ============================================================================
-- PHASE 4: Basic Query Operations with Verification
-- ============================================================================

-- Simple verification query
SELECT 1 as test_value;

-- Basic SELECT operations
SELECT COUNT(*) as total_users FROM users;
SELECT COUNT(*) as total_orders FROM orders;
SELECT COUNT(*) as total_products FROM products;

-- WHERE clause testing
SELECT id, name, age FROM users WHERE age > 25 ORDER BY age;
SELECT name, email FROM users WHERE name LIKE 'A%' ORDER BY name;

-- Aggregate functions
SELECT COUNT(*) as user_count, AVG(age) as avg_age, MAX(age) as max_age, MIN(age) as min_age FROM users;

-- ============================================================================
-- PHASE 5: JOIN Operations with Verification
-- ============================================================================

-- Simple JOIN verification
SELECT COUNT(*) as join_result_count FROM users u JOIN orders o ON u.id = o.user_id;

-- Detailed JOIN results
SELECT u.name, u.email, o.product_name, o.quantity, o.price
FROM users u
JOIN orders o ON u.id = o.user_id
ORDER BY u.id, o.order_id;

-- Three-way JOIN
SELECT u.name, p.name as product_name, p.category, o.quantity, o.price
FROM users u
JOIN orders o ON u.id = o.user_id
JOIN products p ON o.product_name = p.name
ORDER BY u.name, p.category;

-- ============================================================================
-- PHASE 6: Subquery Operations with Verification
-- ============================================================================

-- Subquery for max age user
SELECT name, age FROM users WHERE age = (SELECT MAX(age) FROM users);

-- Subquery with IN clause
SELECT name FROM users WHERE id IN (SELECT user_id FROM orders WHERE price > 100);

-- Verify subquery results
SELECT COUNT(*) as high_value_orders FROM orders WHERE price > 100;

-- ============================================================================
-- PHASE 7: UPDATE Operations with Verification
-- ============================================================================

-- Update user age and verify
UPDATE users SET age = age + 1 WHERE id = 1;
SELECT id, name, age FROM users WHERE id = 1;

-- Update product stock and verify
UPDATE products SET stock_quantity = stock_quantity - 10 WHERE product_id = 1;
SELECT product_id, name, stock_quantity FROM products WHERE product_id = 1;

-- ============================================================================
-- PHASE 8: DELETE Operations with Verification
-- ============================================================================

-- Delete an order and verify
SELECT COUNT(*) as orders_before_delete FROM orders;
DELETE FROM orders WHERE order_id = 5;
SELECT COUNT(*) as orders_after_delete FROM orders;
SELECT * FROM orders WHERE order_id = 5;  -- Should return empty

-- ============================================================================
-- PHASE 9: ALTER TABLE Operations with Verification
-- ============================================================================

-- Add columns and verify
ALTER TABLE users ADD COLUMN phone VARCHAR(20);
ALTER TABLE users ADD COLUMN city VARCHAR(50) DEFAULT 'Unknown';

-- Verify new columns exist
DESCRIBE users;

-- Insert data with new columns
INSERT INTO users (id, name, email, age, phone, city) VALUES (6, 'Frank Miller', 'frank@example.com', 40, '555-0101', 'New York');
INSERT INTO users (id, name, email, age, phone, city) VALUES (7, 'Grace Lee', 'grace@example.com', 27, '555-0102', 'Los Angeles');

-- Verify new data
SELECT id, name, phone, city FROM users WHERE id >= 6 ORDER BY id;

-- ============================================================================
-- PHASE 10: Complex Queries and Final Verification
-- ============================================================================

-- Complex aggregation query
SELECT
    u.name,
    COUNT(o.order_id) as order_count,
    COALESCE(SUM(o.price), 0) as total_spent,
    AVG(o.price) as avg_order_value
FROM users u
LEFT JOIN orders o ON u.id = o.user_id
GROUP BY u.id, u.name
ORDER BY total_spent DESC;

-- Index usage verification (using indexed columns)
SELECT * FROM users WHERE email = 'alice@example.com';
SELECT COUNT(*) as orders_for_user_1 FROM orders WHERE user_id = 1;

-- Final data verification
SELECT 'FINAL VERIFICATION COMPLETE' as status;
SELECT
    (SELECT COUNT(*) FROM users) as total_users,
    (SELECT COUNT(*) FROM products) as total_products,
    (SELECT COUNT(*) FROM orders) as total_orders,
    (SELECT SUM(price) FROM orders) as total_revenue;

-- ============================================================================
-- CLEANUP (commented out for safety - uncomment to clean up test data)
-- ============================================================================
-- DROP TABLE orders;
-- DROP TABLE users;
-- DROP TABLE products;
-- DROP DATABASE test_db;
