#include "config_manager.h"
#include "../include/sql_executor.h"
#include "storage_engine.h"
#include <iostream>
#include <memory>

using namespace sqlcc;

int main() {
  try {
    std::cout << "DML功能测试程序" << std::endl;

    // 创建配置管理器实例
    ConfigManager &config_manager = ConfigManager::GetInstance();

    // 加载配置文件
    if (!config_manager.LoadConfig("./config/sqlcc.conf")) {
      std::cout << "配置文件加载失败，使用默认配置" << std::endl;
    }

    // 创建存储引擎实例
    std::unique_ptr<StorageEngine> storage_engine =
        std::make_unique<StorageEngine>(config_manager);

    // 创建SQL执行器实例
    SqlExecutor executor(*storage_engine);

    // 测试创建表（为DML操作准备）
    std::cout << "\n=== 创建表测试 ===" << std::endl;
    std::string create_result = executor.Execute(
        "CREATE TABLE users (id INT PRIMARY KEY, name VARCHAR(50), age INT)");
    std::cout << create_result << std::endl;

    // 测试插入数据
    std::cout << "\n=== 插入数据测试 ===" << std::endl;
    std::string insert_result1 =
        executor.Execute("INSERT INTO users VALUES (1, 'Alice', 25)");
    std::cout << insert_result1 << std::endl;

    std::string insert_result2 =
        executor.Execute("INSERT INTO users VALUES (2, 'Bob', 30)");
    std::cout << insert_result2 << std::endl;

    std::string insert_result3 =
        executor.Execute("INSERT INTO users VALUES (3, 'Charlie', 35)");
    std::cout << insert_result3 << std::endl;

    // 测试查询数据
    std::cout << "\n=== 查询数据测试 ===" << std::endl;
    std::string select_result1 = executor.Execute("SELECT * FROM users");
    std::cout << select_result1 << std::endl;

    std::string select_result2 =
        executor.Execute("SELECT name, age FROM users");
    std::cout << select_result2 << std::endl;

    // 测试更新数据
    std::cout << "\n=== 更新数据测试 ===" << std::endl;
    std::string update_result =
        executor.Execute("UPDATE users SET age = 26 WHERE id = 1");
    std::cout << update_result << std::endl;

    // 验证更新结果
    std::string select_result3 =
        executor.Execute("SELECT * FROM users WHERE id = 1");
    std::cout << select_result3 << std::endl;

    // 测试删除数据
    std::cout << "\n=== 删除数据测试 ===" << std::endl;
    std::string delete_result =
        executor.Execute("DELETE FROM users WHERE id = 2");
    std::cout << delete_result << std::endl;

    // 验证删除结果
    std::string select_result4 = executor.Execute("SELECT * FROM users");
    std::cout << select_result4 << std::endl;

    std::cout << "\nDML功能测试完成!" << std::endl;
  } catch (const std::exception &e) {
    std::cerr << "错误: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}