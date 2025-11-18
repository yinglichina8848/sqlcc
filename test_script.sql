-- 这是一个测试脚本
SELECT * FROM users;
INSERT INTO products VALUES (1, 'Test Product', 99.99);
UPDATE orders SET status = 'completed' WHERE id = 1;
DELETE FROM logs WHERE timestamp < '2024-01-01';
-- 测试结束