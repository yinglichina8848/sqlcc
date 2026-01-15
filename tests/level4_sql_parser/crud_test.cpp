#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include "../include/sql_executor.h"

using namespace sqlcc;

int main() {
    std::cout << "开始真实 CRUD 性能测试..." << std::endl;

    try {
        // 创建SQL执行器
        auto sql_executor = std::make_unique<SqlExecutor>();

        // 测试数据库操作
        std::cout << "1. 创建数据库..." << std::endl;
        std::string result = sql_executor->Execute("CREATE DATABASE testdb;");
        std::cout << "   创建数据库结果: " << result << std::endl;

        // 测试使用数据库
        std::cout << "2. 使用数据库..." << std::endl;
        result = sql_executor->Execute("USE testdb;");
        std::cout << "   使用数据库结果: " << result << std::endl;

        // 测试表操作
        std::cout << "3. 创建表..." << std::endl;
        result = sql_executor->Execute("CREATE TABLE users (id INT PRIMARY KEY, name VARCHAR(100), age INT);");
        std::cout << "   创建表结果: " << result << std::endl;

        // 测试插入数据 (Create)
        std::cout << "4. 插入数据 (Create)..." << std::endl;
        result = sql_executor->Execute("INSERT INTO users VALUES (1, 'Alice', 25);");
        std::cout << "   插入数据结果: " << result << std::endl;

        result = sql_executor->Execute("INSERT INTO users VALUES (2, 'Bob', 30);");
        std::cout << "   插入数据结果: " << result << std::endl;

        result = sql_executor->Execute("INSERT INTO users VALUES (3, 'Charlie', 35);");
        std::cout << "   插入数据结果: " << result << std::endl;

        // 测试查询数据 (Read)
        std::cout << "5. 查询数据 (Read)..." << std::endl;
        result = sql_executor->Execute("SELECT * FROM users;");
        std::cout << "   查询数据结果: " << result << std::endl;

        // 测试查询特定记录
        std::cout << "6. 查询特定记录..." << std::endl;
        result = sql_executor->Execute("SELECT name, age FROM users WHERE id = 2;");
        std::cout << "   查询特定记录结果: " << result << std::endl;

        // 测试更新数据 (Update)
        std::cout << "7. 更新数据 (Update)..." << std::endl;
        result = sql_executor->Execute("UPDATE users SET age = 26 WHERE id = 1;");
        std::cout << "   更新数据结果: " << result << std::endl;

        // 验证更新结果
        result = sql_executor->Execute("SELECT * FROM users WHERE id = 1;");
        std::cout << "   验证更新结果: " << result << std::endl;

        // 测试删除数据 (Delete)
        std::cout << "8. 删除数据 (Delete)..." << std::endl;
        result = sql_executor->Execute("DELETE FROM users WHERE id = 3;");
        std::cout << "   删除数据结果: " << result << std::endl;

        // 验证删除结果
        result = sql_executor->Execute("SELECT * FROM users;");
        std::cout << "   验证删除结果: " << result << std::endl;

        // 测试事务操作
        std::cout << "9. 测试事务..." << std::endl;
        result = sql_executor->Execute("BEGIN;");
        std::cout << "   开始事务结果: " << result << std::endl;

        result = sql_executor->Execute("INSERT INTO users VALUES (4, 'David', 40);");
        std::cout << "   事务中插入数据结果: " << result << std::endl;

        result = sql_executor->Execute("COMMIT;");
        std::cout << "   提交事务结果: " << result << std::endl;

        // 验证事务结果
        result = sql_executor->Execute("SELECT * FROM users WHERE id = 4;");
        std::cout << "   验证事务结果: " << result << std::endl;

        // 测试聚合查询
        std::cout << "10. 测试聚合查询..." << std::endl;
        result = sql_executor->Execute("SELECT COUNT(*) as total_users, AVG(age) as avg_age FROM users;");
        std::cout << "    聚合查询结果: " << result << std::endl;

        // 测试删除表
        std::cout << "11. 删除表..." << std::endl;
        result = sql_executor->Execute("DROP TABLE users;");
        std::cout << "    删除表结果: " << result << std::endl;

        // 测试删除数据库
        std::cout << "12. 删除数据库..." << std::endl;
        result = sql_executor->Execute("DROP DATABASE testdb;");
        std::cout << "    删除数据库结果: " << result << std::endl;

        std::cout << "\n真实 CRUD 性能测试完成!" << std::endl;
        std::cout << "测试覆盖了完整的 CRUD 操作：创建、读取、更新、删除" << std::endl;
        std::cout << "包括事务操作和聚合查询" << std::endl;

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "测试失败: " << e.what() << std::endl;
        return 1;
    }
}
