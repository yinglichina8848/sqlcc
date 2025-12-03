#include "../include/execution_engine.h"
#include "../include/database_manager.h"
#include "../include/table_storage.h"
#include "../include/storage_engine.h"
#include "../include/sql_parser/parser.h"
#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <chrono>

/**
 * 索引优化功能演示程序
 * 展示索引查询优化的工作原理
 */
int main() {
    std::cout << "=== SQLCC 索引优化功能演示 ===\n" << std::endl;

    try {
        // 1. 创建数据库管理器
        auto db_manager = std::make_shared<sqlcc::DatabaseManager>();
        auto dml_executor = std::make_shared<sqlcc::DMLExecutor>(db_manager);

        std::cout << "✓ 创建数据库管理器和DML执行器" << std::endl;

        // 2. 创建测试数据库和表
        if (!db_manager->CreateDatabase("demo_db")) {
            std::cerr << "✗ 创建数据库失败" << std::endl;
            return 1;
        }

        if (!db_manager->UseDatabase("demo_db")) {
            std::cerr << "✗ 切换数据库失败" << std::endl;
            return 1;
        }

        // 创建测试表：employees(id, name, department, salary)
        std::vector<std::pair<std::string, std::string>> columns = {
            {"id", "INT"},
            {"name", "VARCHAR(50)"},
            {"department", "VARCHAR(30)"},
            {"salary", "INT"}
        };

        if (!db_manager->CreateTable("employees", columns)) {
            std::cerr << "✗ 创建表失败" << std::endl;
            return 1;
        }

        std::cout << "✓ 创建测试表 employees(id, name, department, salary)" << std::endl;

        // 3. 插入测试数据
        std::vector<std::vector<std::string>> test_data = {
            {"1", "Alice Johnson", "Engineering", "75000"},
            {"2", "Bob Smith", "Sales", "65000"},
            {"3", "Charlie Brown", "Engineering", "80000"},
            {"4", "Diana Prince", "HR", "70000"},
            {"5", "Eve Wilson", "Sales", "68000"},
            {"6", "Frank Miller", "Engineering", "85000"},
            {"7", "Grace Lee", "HR", "72000"},
            {"8", "Henry Ford", "Sales", "69000"}
        };

        auto storage_engine = db_manager->GetStorageEngine();
        if (!storage_engine) {
            std::cerr << "✗ 获取存储引擎失败" << std::endl;
            return 1;
        }

        sqlcc::TableStorageManager table_storage(storage_engine);
        for (const auto& record : test_data) {
            int32_t page_id;
            size_t offset;
            if (!table_storage.InsertRecord("employees", record, page_id, offset)) {
                std::cerr << "✗ 插入测试数据失败" << std::endl;
                return 1;
            }
        }

        std::cout << "✓ 插入 " << test_data.size() << " 条测试记录" << std::endl;

        // 4. 演示索引优化查询
        std::cout << "\n=== 索引优化查询演示 ===\n" << std::endl;

        // 测试场景1：等式查询
        std::cout << "场景1：等式查询 - WHERE id = 3" << std::endl;
        sqlcc::sql_parser::WhereClause equal_condition("id", "=", "3");

        bool used_index = false;
        std::string index_info;

        auto start_time = std::chrono::high_resolution_clock::now();
        auto locations = dml_executor->optimizeQueryWithIndex(
            "employees", equal_condition, table_storage, used_index, index_info);
        auto end_time = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);

        std::cout << "查询结果：" << std::endl;
        std::cout << "  - 是否使用索引: " << (used_index ? "是" : "否") << std::endl;
        std::cout << "  - 索引信息: " << index_info << std::endl;
        std::cout << "  - 扫描记录数: " << locations.size() << std::endl;
        std::cout << "  - 查询耗时: " << duration.count() << " 微秒" << std::endl;

        // 显示找到的记录
        if (!locations.empty()) {
            auto record = table_storage.GetRecord("employees", locations[0].first, locations[0].second);
            if (!record.empty()) {
                std::cout << "  - 找到记录: ID=" << record[0] << ", Name=" << record[1]
                         << ", Dept=" << record[2] << ", Salary=" << record[3] << std::endl;
            }
        }

        // 测试场景2：范围查询
        std::cout << "\n场景2：范围查询 - WHERE salary > 70000" << std::endl;
        sqlcc::sql_parser::WhereClause range_condition("salary", ">", "70000");

        start_time = std::chrono::high_resolution_clock::now();
        locations = dml_executor->optimizeQueryWithIndex(
            "employees", range_condition, table_storage, used_index, index_info);
        end_time = std::chrono::high_resolution_clock::now();

        duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);

        std::cout << "查询结果：" << std::endl;
        std::cout << "  - 是否使用索引: " << (used_index ? "是" : "否") << std::endl;
        std::cout << "  - 索引信息: " << index_info << std::endl;
        std::cout << "  - 扫描记录数: " << locations.size() << std::endl;
        std::cout << "  - 查询耗时: " << duration.count() << " 微秒" << std::endl;

        // 显示找到的记录
        std::cout << "  - 符合条件的记录:" << std::endl;
        for (const auto& location : locations) {
            auto record = table_storage.GetRecord("employees", location.first, location.second);
            if (!record.empty() && std::stoi(record[3]) > 70000) {
                std::cout << "    * ID=" << record[0] << ", Name=" << record[1]
                         << ", Dept=" << record[2] << ", Salary=" << record[3] << std::endl;
            }
        }

        // 测试场景3：不支持的操作符（全表扫描）
        std::cout << "\n场景3：不支持的操作符 - WHERE name LIKE 'A%'" << std::endl;
        sqlcc::sql_parser::WhereClause like_condition("name", "LIKE", "A%");

        start_time = std::chrono::high_resolution_clock::now();
        locations = dml_executor->optimizeQueryWithIndex(
            "employees", like_condition, table_storage, used_index, index_info);
        end_time = std::chrono::high_resolution_clock::now();

        duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);

        std::cout << "查询结果：" << std::endl;
        std::cout << "  - 是否使用索引: " << (used_index ? "是" : "否") << std::endl;
        std::cout << "  - 索引信息: " << index_info << std::endl;
        std::cout << "  - 扫描记录数: " << locations.size() << std::endl;
        std::cout << "  - 查询耗时: " << duration.count() << " 微秒" << std::endl;

        // 5. 演示UPDATE语句的索引优化
        std::cout << "\n=== UPDATE语句索引优化演示 ===\n" << std::endl;

        std::cout << "执行: UPDATE employees SET salary = 90000 WHERE id = 1" << std::endl;

        sqlcc::sql_parser::UpdateStatement update_stmt("employees");
        update_stmt.addUpdateValue("salary", "90000");
        sqlcc::sql_parser::WhereClause update_condition("id", "=", "1");
        update_stmt.setWhereClause(update_condition);

        auto update_result = dml_executor->execute(std::make_unique<sqlcc::sql_parser::UpdateStatement>(update_stmt));

        std::cout << "UPDATE结果: " << (update_result.success ? "成功" : "失败") << std::endl;
        std::cout << "消息: " << update_result.message << std::endl;

        // 验证更新结果
        sqlcc::sql_parser::WhereClause verify_condition("id", "=", "1");
        locations = dml_executor->optimizeQueryWithIndex(
            "employees", verify_condition, table_storage, used_index, index_info);

        if (!locations.empty()) {
            auto record = table_storage.GetRecord("employees", locations[0].first, locations[0].second);
            if (!record.empty()) {
                std::cout << "验证更新后的记录: ID=" << record[0] << ", Name=" << record[1]
                         << ", Dept=" << record[2] << ", Salary=" << record[3] << std::endl;
            }
        }

        // 6. 演示DELETE语句的索引优化
        std::cout << "\n=== DELETE语句索引优化演示 ===\n" << std::endl;

        std::cout << "执行: DELETE FROM employees WHERE id = 8" << std::endl;

        sqlcc::sql_parser::DeleteStatement delete_stmt("employees");
        sqlcc::sql_parser::WhereClause delete_condition("id", "=", "8");
        delete_stmt.setWhereClause(delete_condition);

        auto delete_result = dml_executor->execute(std::make_unique<sqlcc::sql_parser::DeleteStatement>(delete_stmt));

        std::cout << "DELETE结果: " << (delete_result.success ? "成功" : "失败") << std::endl;
        std::cout << "消息: " << delete_result.message << std::endl;

        // 7. 性能对比
        std::cout << "\n=== 性能对比演示 ===\n" << std::endl;

        // 索引查询
        sqlcc::sql_parser::WhereClause perf_condition("id", "=", "3");
        start_time = std::chrono::high_resolution_clock::now();

        auto index_locations = dml_executor->optimizeQueryWithIndex(
            "employees", perf_condition, table_storage, used_index, index_info);

        end_time = std::chrono::high_resolution_clock::now();
        auto index_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);

        // 全表扫描
        auto full_scan_start = std::chrono::high_resolution_clock::now();
        auto all_locations = table_storage.ScanTable("employees");
        auto full_scan_end = std::chrono::high_resolution_clock::now();
        auto full_scan_duration = std::chrono::duration_cast<std::chrono::microseconds>(full_scan_end - full_scan_start);

        std::cout << "性能对比测试 (查找ID=3的记录):" << std::endl;
        std::cout << "索引查询: " << index_duration.count() << " 微秒, 扫描 " << index_locations.size() << " 条记录" << std::endl;
        std::cout << "全表扫描: " << full_scan_duration.count() << " 微秒, 扫描 " << all_locations.size() << " 条记录" << std::endl;

        double speedup = static_cast<double>(full_scan_duration.count()) / index_duration.count();
        std::cout << "性能提升: " << std::fixed << std::setprecision(2) << speedup << "x" << std::endl;

        // 8. 清理
        db_manager->DropDatabase("demo_db");

        std::cout << "\n=== 演示完成 ===" << std::endl;
        std::cout << "索引优化功能已成功集成到SQLCC查询执行引擎中！" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "演示过程中发生错误: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
