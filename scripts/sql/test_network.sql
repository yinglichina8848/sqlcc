-- 网络模式功能测试脚本
USE test_db;
SHOW TABLES;
SELECT * FROM users LIMIT 3;
-- 创建一个新表测试网络模式的DDL操作
CREATE TABLE network_test (id INT PRIMARY KEY, name VARCHAR(50));
-- 插入数据测试DML操作
INSERT INTO network_test VALUES (1, 'network_user1'), (2, 'network_user2');
-- 查询插入的数据
SELECT * FROM network_test;
-- 删除测试表
DROP TABLE network_test;
SHOW TABLES;