-- Simple ISQL test file with verification
SELECT 1 as connection_test;

-- Create database and verify
CREATE DATABASE simple_test;
USE simple_test;
SELECT DATABASE() as current_database;

-- Create table and verify
CREATE TABLE test (id INTEGER PRIMARY KEY, name VARCHAR(50));
SHOW TABLES;
DESCRIBE test;

-- Insert data and verify
INSERT INTO test VALUES (1, 'Hello SQLCC');
INSERT INTO test VALUES (2, 'World Test');
SELECT COUNT(*) as row_count FROM test;
SELECT * FROM test ORDER BY id;

-- Final verification
SELECT 'ALL TESTS PASSED' as status;
