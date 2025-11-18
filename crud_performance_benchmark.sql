-- SQLCC CRUD性能基准测试脚本
-- 此脚本用于验证CRUD操作是否满足"1-10万行数据，单操作耗时<5ms (SSD)"的性能要求

-- 1. 创建和使用测试数据库
CREATE DATABASE crud_benchmark_db;
USE crud_benchmark_db;

-- 2. 创建测试表
CREATE TABLE performance_test (
    id INT PRIMARY KEY AUTO_INCREMENT,
    name VARCHAR(100) NOT NULL,
    email VARCHAR(100) NOT NULL,
    age INT NOT NULL,
    salary DECIMAL(10,2) NOT NULL,
    department VARCHAR(50) NOT NULL,
    hire_date DATETIME NOT NULL,
    last_update TIMESTAMP NOT NULL
);

-- 3. 插入测试数据 - 10,000行
-- 注意：这里实际运行时会生成10,000条记录
-- 为了演示，我们显示前10条插入语句

-- 模拟批量插入测试（10,000条记录）
-- 测试点：插入性能
INSERT INTO performance_test VALUES (1, 'User1', 'user1@example.com', 25, 50000.00, 'Engineering', '2023-01-01 10:00:00', CURRENT_TIMESTAMP);
INSERT INTO performance_test VALUES (2, 'User2', 'user2@example.com', 30, 60000.00, 'Marketing', '2023-01-02 11:00:00', CURRENT_TIMESTAMP);
INSERT INTO performance_test VALUES (3, 'User3', 'user3@example.com', 35, 70000.00, 'Sales', '2023-01-03 12:00:00', CURRENT_TIMESTAMP);
INSERT INTO performance_test VALUES (4, 'User4', 'user4@example.com', 40, 80000.00, 'Engineering', '2023-01-04 13:00:00', CURRENT_TIMESTAMP);
INSERT INTO performance_test VALUES (5, 'User5', 'user5@example.com', 45, 90000.00, 'HR', '2023-01-05 14:00:00', CURRENT_TIMESTAMP);
INSERT INTO performance_test VALUES (6, 'User6', 'user6@example.com', 28, 55000.00, 'Finance', '2023-01-06 15:00:00', CURRENT_TIMESTAMP);
INSERT INTO performance_test VALUES (7, 'User7', 'user7@example.com', 32, 65000.00, 'Engineering', '2023-01-07 16:00:00', CURRENT_TIMESTAMP);
INSERT INTO performance_test VALUES (8, 'User8', 'user8@example.com', 38, 75000.00, 'Marketing', '2023-01-08 17:00:00', CURRENT_TIMESTAMP);
INSERT INTO performance_test VALUES (9, 'User9', 'user9@example.com', 42, 85000.00, 'Sales', '2023-01-09 18:00:00', CURRENT_TIMESTAMP);
INSERT INTO performance_test VALUES (10, 'User10', 'user10@example.com', 48, 95000.00, 'Executive', '2023-01-10 19:00:00', CURRENT_TIMESTAMP);
-- ... 此处省略9,990条插入语句 ...

-- 4. SELECT操作性能测试

-- 4.1 点查询测试（按主键查询单条记录）
-- 测试点：单条记录查询性能，要求<5ms
SELECT * FROM performance_test WHERE id = 5000;

-- 4.2 范围扫描测试（查询多条记录）
-- 测试点：范围查询性能，要求<5ms
SELECT * FROM performance_test WHERE age BETWEEN 30 AND 40;
SELECT * FROM performance_test WHERE department = 'Engineering';

-- 4.3 排序查询测试
-- 测试点：带排序的查询性能
SELECT * FROM performance_test ORDER BY salary DESC LIMIT 10;

-- 5. UPDATE操作性能测试

-- 5.1 单点更新测试
-- 测试点：单条记录更新性能，要求<5ms
UPDATE performance_test SET salary = salary * 1.05 WHERE id = 5000;

-- 5.2 范围更新测试
-- 测试点：多条记录更新性能
UPDATE performance_test SET department = 'R&D' WHERE department = 'Engineering';

-- 6. DELETE操作性能测试

-- 6.1 单点删除测试
-- 测试点：单条记录删除性能，要求<5ms
DELETE FROM performance_test WHERE id = 9999;

-- 6.2 范围删除测试
-- 测试点：多条记录删除性能
DELETE FROM performance_test WHERE age > 60;

-- 7. 复合操作性能测试

-- 7.1 先查询后更新
SELECT * FROM performance_test WHERE salary > 80000;
UPDATE performance_test SET bonus = 5000 WHERE salary > 80000;

-- 7.2 事务模拟（多次操作）
BEGIN TRANSACTION;
INSERT INTO performance_test VALUES (10001, 'NewEmployee', 'new@example.com', 29, 55000.00, 'IT', CURRENT_TIMESTAMP, CURRENT_TIMESTAMP);
UPDATE performance_test SET department = 'IT' WHERE id = 10001;
COMMIT;

-- 8. 大表性能测试（验证10万行数据的性能）
-- 注意：此处仅作概念演示，实际测试需要生成10万行数据

-- 9. 清理测试数据
DROP TABLE performance_test;
DROP DATABASE crud_benchmark_db;

-- 性能测试脚本结束
-- 预期结果：所有CRUD操作（插入、点查、范围扫描、更新、删除）在1-10万行数据规模下，单操作耗时应小于5ms (SSD环境)