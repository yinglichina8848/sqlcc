#include "sql_executor.h"
#include "database_manager.h"
#include "config_manager.h"
#include "unified_query_plan.h"
#include "sql_parser/parser.h"
#include "permission_validator.h"
#include <algorithm>
#include <cctype>
#include <string>
#include <iostream>
#include <fstream>
#include <stdexcept>

namespace sqlcc {

// 构造函数实现 - 重构版本，使用统一查询计划架构
SqlExecutor::SqlExecutor()
    : last_error_(""),
      execution_stats_(""),
      current_user_("admin"),
      current_database_("") {

// 接受DatabaseManager参数的构造函数实现
SqlExecutor::SqlExecutor(std::shared_ptr<DatabaseManager> db_manager)
    : last_error_(""),
      execution_stats_(""),
      current_user_("admin"),
      current_database_(""),
      db_manager_(db_manager) {

     try {
         // 使用传入的数据库管理器，不再重新创建
         if (!db_manager_) {
             throw std::runtime_error("Database manager is null");
         }

         // 初始化用户管理器
         user_manager_ = std::make_shared<UserManager>();

         // 初始化系统数据库
         system_db_ = std::make_shared<SystemDatabase>();

         // 初始化权限验证器
         if (!InitializePermissionValidator()) {
             throw std::runtime_error("Failed to initialize permission validator");
         }

         // 初始化系统数据库
         if (!InitializeSystemDatabase()) {
             throw std::runtime_error("Failed to initialize system database");
         }

     } catch (const std::exception& e) {
         last_error_ = std::string("Failed to initialize SqlExecutor: ") + e.what();
         std::cerr << "SqlExecutor initialization error: " << e.what() << std::endl;
     }
 }

// 析构函数实现
SqlExecutor::~SqlExecutor() {
    // 智能指针会自动释放资源
}

// 执行SQL语句 - 重构版本，使用统一查询计划架构
std::string SqlExecutor::Execute(const std::string &sql) {
  std::string trimmed_sql = sql;
  TrimString(trimmed_sql);

  if (trimmed_sql.empty()) {
    SetError("Empty SQL statement");
    return "错误：空的SQL语句";
  }

  try {
    // 解析SQL语句
    auto stmt = ParseSQL(trimmed_sql);
    if (!stmt) {
      SetError("Failed to parse SQL statement");
      return "错误：SQL语句解析失败";
    }

    // 创建统一查询计划
    auto query_plan = CreateQueryPlan(std::move(stmt));
    if (!query_plan) {
      SetError("Failed to create query plan");
      return "错误：创建查询计划失败";
    }

    // 执行查询计划
    ExecutionResult result = query_plan->executePlan();

    // 更新执行统计信息
    execution_stats_ = query_plan->getExecutionStats();

    // 返回执行结果
    if (result.success) {
      ClearError();
      return result.message;
    } else {
      SetError(result.message);
      return "错误：" + result.message;
    }
  } catch (const std::exception& e) {
    SetError(e.what());
    return "错误：" + std::string(e.what());
  }
}

// 获取最后一次执行的错误信息
std::string SqlExecutor::GetLastError() const {
    return last_error_;
}

// 私有辅助函数实现
void SqlExecutor::SetError(const std::string &error) { last_error_ = error; }

void SqlExecutor::ClearError() { last_error_.clear(); }

void SqlExecutor::TrimString(std::string &str) {
  str.erase(str.begin(),
            std::find_if(str.begin(), str.end(),
                         [](unsigned char ch) { return !std::isspace(ch); }));
  str.erase(std::find_if(str.rbegin(), str.rend(),
                         [](unsigned char ch) { return !std::isspace(ch); })
                .base(),
            str.end());
}

// 初始化系统数据库
bool SqlExecutor::InitializeSystemDatabase() {
    try {
        if (!system_db_) {
            system_db_ = std::make_shared<SystemDatabase>();
        }
        
        // 初始化系统数据库表
        if (!system_db_->Initialize()) {
            SetError("Failed to initialize system database");
            return false;
        }
        
        return true;
    } catch (const std::exception& e) {
        SetError(std::string("System database initialization error: ") + e.what());
        return false;
    }
}

// 初始化权限验证器
bool SqlExecutor::InitializePermissionValidator() {
    try {
        if (!user_manager_) {
            user_manager_ = std::make_shared<UserManager>();
        }
        
        if (!system_db_) {
            system_db_ = std::make_shared<SystemDatabase>();
        }
        
        permission_validator_ = std::make_unique<PermissionValidator>(
            user_manager_, system_db_);
        
        return true;
    } catch (const std::exception& e) {
        SetError(std::string("Permission validator initialization error: ") + e.what());
        return false;
    }
}

// 解析SQL语句
std::unique_ptr<sql_parser::Statement> SqlExecutor::ParseSQL(const std::string& sql) {
    try {
        // 创建SQL解析器
        auto parser = sql_parser::Parser(sql);
        
        // 解析SQL语句
        auto stmt = parser.Parse();
        
        if (!stmt) {
            SetError("SQL parsing failed: null statement returned");
            return nullptr;
        }
        
        return stmt;
    } catch (const std::exception& e) {
        SetError(std::string("SQL parsing error: ") + e.what());
        return nullptr;
    }
}

// 创建统一查询计划
std::unique_ptr<UnifiedQueryPlan> SqlExecutor::CreateQueryPlan(
    std::unique_ptr<sql_parser::Statement> stmt) {
    
    try {
        // 使用查询计划工厂创建对应的查询计划
        auto query_plan = QueryPlanFactory::createPlan(
            std::move(stmt), 
            db_manager_, 
            user_manager_, 
            system_db_);
        
        if (!query_plan) {
            SetError("Failed to create query plan for the given statement type");
            return nullptr;
        }
        
        return query_plan;
    } catch (const std::exception& e) {
        SetError(std::string("Query plan creation error: ") + e.what());
        return nullptr;
    }
}

// 更新当前数据库
void SqlExecutor::UpdateCurrentDatabase(const std::string& sql) {
    // 简单实现：检查是否为USE语句
    std::string upper_sql = sql;
    std::transform(upper_sql.begin(), upper_sql.end(), upper_sql.begin(), ::toupper);
    
    if (upper_sql.find("USE ") == 0) {
        // 提取数据库名
        size_t pos = upper_sql.find(" ");
        if (pos != std::string::npos) {
            std::string db_name = sql.substr(pos + 1);
            TrimString(db_name);
            
            // 移除末尾的分号
            if (!db_name.empty() && db_name.back() == ';') {
                db_name.pop_back();
            }
            
            if (!db_name.empty()) {
                current_database_ = db_name;
            }
        }
    }
}

// 获取执行统计信息
std::string SqlExecutor::GetExecutionStats() const {
    return execution_stats_;
}

// 执行SQL文件
std::string SqlExecutor::ExecuteFile(const std::string& file_path) {
    try {
        // 打开文件
        std::ifstream file(file_path);
        if (!file.is_open()) {
            SetError("无法打开文件: " + file_path);
            return "错误：无法打开文件 " + file_path;
        }
        
        std::string result;
        std::string line;
        std::string current_statement;
        
        // 逐行读取文件
        while (std::getline(file, line)) {
            TrimString(line);
            
            // 跳过空行和注释
            if (line.empty() || line[0] == '#' || line.substr(0, 2) == "--") {
                continue;
            }
            
            // 将行添加到当前语句
            current_statement += line + " ";
            
            // 检查语句是否结束（以分号结尾）
            if (line.back() == ';') {
                // 执行当前语句
                std::string statement_result = Execute(current_statement);
                
                // 添加执行结果
                result += statement_result + "\n";
                
                // 重置当前语句
                current_statement.clear();
            }
        }
        
        // 处理文件末尾没有分号的语句
        if (!current_statement.empty()) {
            TrimString(current_statement);
            if (!current_statement.empty()) {
                std::string statement_result = Execute(current_statement);
                result += statement_result + "\n";
            }
        }
        
        file.close();
        ClearError();
        return result;
        
    } catch (const std::exception& e) {
        SetError(std::string("文件执行错误: ") + e.what());
        return "错误：" + std::string(e.what());
    }
}

} // namespace sqlcc