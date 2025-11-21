#!/usr/bin/env python3
# 生成SQLCC CRUD性能测试数据的脚本
# 此脚本用于生成包含10,000行数据的SQL插入语句

import sys
import random
from datetime import datetime, timedelta

# 生成随机日期
def random_date(start, end):
    return start + timedelta(
        seconds=random.randint(0, int((end - start).total_seconds()))
    )

# 生成随机名字
def random_name():    return f"User{random.randint(1, 100000)}"

# 生成随机邮箱
def random_email(name):
    domains = ['example.com', 'test.com', 'sqlcc.org', 'database.com']    
    return f"{name.lower()}@{random.choice(domains)}"

# 生成随机部门
def random_department():
    departments = ['Engineering', 'Marketing', 'Sales', 'HR', 'Finance', 'IT', 'R&D', 'Executive', 'Support']    
    return random.choice(departments)

def main():
    # 设置日期范围
    start_date = datetime(2020, 1, 1)
    end_date = datetime(2024, 12, 31)
    
    # 创建测试数据库和表
    print("-- SQLCC CRUD性能基准测试脚本 - 包含10,000行实际测试数据")
    print("-- 此脚本用于验证CRUD操作是否满足'1-10万行数据，单操作耗时<5ms (SSD)'的性能要求")
    print()
    print("-- 1. 创建和使用测试数据库")
    print("CREATE DATABASE crud_benchmark_db;")
    print("USE crud_benchmark_db;")
    print()
    print("-- 2. 创建测试表")
    print("CREATE TABLE performance_test (")
    print("    id INT PRIMARY KEY AUTO_INCREMENT,")
    print("    name VARCHAR(100) NOT NULL,")
    print("    email VARCHAR(100) NOT NULL,")
    print("    age INT NOT NULL,")
    print("    salary DECIMAL(10,2) NOT NULL,")
    print("    department VARCHAR(50) NOT NULL,")
    print("    hire_date DATETIME NOT NULL,")
    print("    last_update TIMESTAMP NOT NULL")
    print(");")
    print()
    print("-- 3. 插入测试数据 - 10,000行")
    print("-- 测试点：插入性能")
    
    # 生成10,000条插入语句
    for i in range(1, 10001):
        name = f"User{i}"
        email = f"user{i}@example.com"
        age = random.randint(22, 65)
        salary = random.randint(30000, 150000) + random.random()
        department = random_department()
        hire_date = random_date(start_date, end_date)
        
        print(f"INSERT INTO performance_test VALUES ({i}, '{name}', '{email}', {age}, {salary:.2f}, '{department}', '{hire_date.strftime('%Y-%m-%d %H:%M:%S')}', CURRENT_TIMESTAMP);")
    
    print()
    print("-- 4. SELECT操作性能测试")
    print()
    print("-- 4.1 点查询测试（按主键查询单条记录）")
    print("-- 测试点：单条记录查询性能，要求<5ms")
    print("SELECT * FROM performance_test WHERE id = 5000;")
    print("SELECT * FROM performance_test WHERE id = 100;")
    print("SELECT * FROM performance_test WHERE id = 9999;")
    print()
    print("-- 4.2 范围扫描测试（查询多条记录）")
    print("-- 测试点：范围查询性能，要求<5ms")
    print("SELECT * FROM performance_test WHERE age BETWEEN 30 AND 40;")
    print("SELECT * FROM performance_test WHERE department = 'Engineering';")
    print("SELECT * FROM performance_test WHERE salary > 100000;")
    print()
    print("-- 4.3 排序查询测试")
    print("-- 测试点：带排序的查询性能")
    print("SELECT * FROM performance_test ORDER BY salary DESC LIMIT 10;")
    print("SELECT * FROM performance_test ORDER BY hire_date ASC LIMIT 5;")
    print()
    print("-- 5. UPDATE操作性能测试")
    print()
    print("-- 5.1 单点更新测试")
    print("-- 测试点：单条记录更新性能，要求<5ms")
    print("UPDATE performance_test SET salary = salary * 1.05 WHERE id = 5000;")
    print("UPDATE performance_test SET department = 'Management' WHERE id = 100;")
    print()
    print("-- 5.2 范围更新测试")
    print("-- 测试点：多条记录更新性能")
    print("UPDATE performance_test SET department = 'R&D' WHERE department = 'Engineering';")
    print("UPDATE performance_test SET salary = salary + 1000 WHERE age > 50;")
    print()
    print("-- 6. DELETE操作性能测试")
    print()
    print("-- 6.1 单点删除测试")
    print("-- 测试点：单条记录删除性能，要求<5ms")
    print("DELETE FROM performance_test WHERE id = 9999;")
    print("DELETE FROM performance_test WHERE id = 500;")
    print()
    print("-- 6.2 范围删除测试")
    print("-- 测试点：多条记录删除性能")
    print("DELETE FROM performance_test WHERE age > 60;")
    print("DELETE FROM performance_test WHERE salary < 40000;")
    print()
    print("-- 7. 清理测试数据")
    print("DROP TABLE performance_test;")
    print("DROP DATABASE crud_benchmark_db;")
    print()
    print("-- 性能测试脚本结束")

if __name__ == "__main__":
    main()