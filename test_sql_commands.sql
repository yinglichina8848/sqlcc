-- DDL测试
CREATE DATABASE testdb;
USE testdb;
CREATE TABLE users (id INT PRIMARY KEY, name VARCHAR(50), email VARCHAR(100));
CREATE INDEX idx_users_name ON users (name);
ALTER TABLE users ADD COLUMN created_at TIMESTAMP;

-- DML测试
INSERT INTO users (id, name, email) VALUES (1, 'Alice', 'alice@example.com');
INSERT INTO users (id, name, email) VALUES (2, 'Bob', 'bob@example.com');
UPDATE users SET email = 'alice.smith@example.com' WHERE id = 1;
DELETE FROM users WHERE id = 2;

-- DQL测试
SELECT * FROM users;
SELECT name, email FROM users WHERE id = 1;

-- DCL测试
CREATE USER testuser IDENTIFIED BY 'password';
GRANT SELECT, INSERT ON users TO testuser;
REVOKE INSERT ON users FROM testuser;
DROP USER testuser;

-- 清理测试
DROP TABLE users;
DROP DATABASE testdb;
