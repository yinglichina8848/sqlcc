#include "sql_parser/parser_new.h"
#include "unified_query_plan.h"
#include "database_manager.h"
#include "core/user_manager.h"
#include "core/system_database.h"
#include <iostream>
#include <memory>

using namespace sqlcc;
using namespace sqlcc::sql_parser;

int main() {
    std::cout << "=== 测试DROP和USE命令 ===" << std::endl;
    
    // 创建必要的管理器
    auto db_manager = std::make_shared<DatabaseManager>();
    auto user_manager = std::make_shared<UserManager>();
    auto system_db = std::make_shared<SystemDatabase>(db_manager);
    
    // 首先创建一个测试数据库
    std::cout << "\n--- 创建测试数据库 ---" << std::endl;
    std::string create_db_sql = "CREATE DATABASE test_db;";
    ParserNew create_db_parser(create_db_sql);
    auto create_db_statements = create_db_parser.parse();
    
    if (!create_db_statements.empty()) {
        auto plan = QueryPlanFactory::createPlan(
            std::move(create_db_statements[0]), db_manager, user_manager, system_db);
        if (plan) {
            plan->buildPlan(std::move(create_db_statements[0])); // 传递已解析的语句
            auto result = plan->executePlan();
            std::cout << "创建数据库结果: " << (result.success ? "成功" : "失败") 
                      << " - " << result.message << std::endl;
        }
    }
    
    // 使用数据库
    std::cout << "\n--- 使用数据库 ---" << std::endl;
    std::string use_sql = "USE test_db;";
    ParserNew use_parser(use_sql);
    auto use_statements = use_parser.parse();
    
    if (!use_statements.empty()) {
        auto plan = QueryPlanFactory::createPlan(
            std::move(use_statements[0]), db_manager, user_manager, system_db);
        if (plan) {
            plan->buildPlan(std::move(use_statements[0])); // 传递已解析的语句
            auto result = plan->executePlan();
            std::cout << "使用数据库结果: " << (result.success ? "成功" : "失败") 
                      << " - " << result.message << std::endl;
        }
    }
    
    // 创建测试表
    std::cout << "\n--- 创建测试表 ---" << std::endl;
    std::string create_table_sql = "CREATE TABLE test_table (id INT PRIMARY KEY, name VARCHAR(50));";
    ParserNew create_table_parser(create_table_sql);
    auto create_table_statements = create_table_parser.parse();
    
    if (!create_table_statements.empty()) {
        auto plan = QueryPlanFactory::createPlan(
            std::move(create_table_statements[0]), db_manager, user_manager, system_db);
        if (plan) {
            plan->buildPlan(std::move(create_table_statements[0])); // 传递已解析的语句
            auto result = plan->executePlan();
            std::cout << "创建表结果: " << (result.success ? "成功" : "失败") 
                      << " - " << result.message << std::endl;
        }
    }
    
    // 测试DROP TABLE
    std::cout << "\n--- 测试DROP TABLE ---" << std::endl;
    std::string drop_table_sql = "DROP TABLE test_table;";
    ParserNew drop_table_parser(drop_table_sql);
    auto drop_table_statements = drop_table_parser.parse();
    
    if (!drop_table_statements.empty()) {
        auto plan = QueryPlanFactory::createPlan(
            std::move(drop_table_statements[0]), db_manager, user_manager, system_db);
        if (plan) {
            plan->buildPlan(std::move(drop_table_statements[0])); // 传递已解析的语句
            auto result = plan->executePlan();
            std::cout << "删除表结果: " << (result.success ? "成功" : "失败") 
                      << " - " << result.message << std::endl;
        }
    }
    
    // 测试DROP DATABASE
    std::cout << "\n--- 测试DROP DATABASE ---" << std::endl;
    std::string drop_db_sql = "DROP DATABASE test_db;";
    ParserNew drop_db_parser(drop_db_sql);
    auto drop_db_statements = drop_db_parser.parse();
    
    if (!drop_db_statements.empty()) {
        auto plan = QueryPlanFactory::createPlan(
            std::move(drop_db_statements[0]), db_manager, user_manager, system_db);
        if (plan) {
            plan->buildPlan(std::move(drop_db_statements[0])); // 传递已解析的语句
            auto result = plan->executePlan();
            std::cout << "删除数据库结果: " << (result.success ? "成功" : "失败") 
                      << " - " << result.message << std::endl;
        }
    }
    
    std::cout << "\n=== 测试完成 ===" << std::endl;
    return 0;
}