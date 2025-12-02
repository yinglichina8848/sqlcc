#include "../../include/sql_executor.h"
#include "../../include/execution_engine.h"
#include "../../include/database_manager.h"
#include "../../include/user_manager.h"
#include "../../include/sql_parser/parser.h"
#include "../../include/sql_parser/ast_nodes.h"
#include <iostream>
#include <sstream>
#include <memory>
#include <fstream>

namespace sqlcc {

// 构造函数实现
SqlExecutor::SqlExecutor() {
    db_manager_ = std::make_shared<DatabaseManager>("./data", 1024, 16, 64);
    user_manager_ = std::make_shared<UserManager>();
}

// 新增构造函数：接受DatabaseManager实例
SqlExecutor::SqlExecutor(std::shared_ptr<DatabaseManager> db_manager) 
    : db_manager_(db_manager) {
    user_manager_ = std::make_shared<UserManager>();
}

SqlExecutor::~SqlExecutor() = default;

// 执行SQL语句
std::string SqlExecutor::Execute(const std::string& sql) {
    try {
        // 获取当前数据库名称
        std::string db_name = db_manager_->GetCurrentDatabase();
        
        // 创建解析器并解析SQL语句
        sql_parser::Parser parser(sql);
        auto statements = parser.parseStatements();
        
        if (statements.empty()) {
            return "Empty statement";
        }
        
        // 处理第一条语句（简单起见）
        auto& stmt = statements[0];
        
        // 根据语句类型创建相应的执行器
        std::unique_ptr<ExecutionEngine> executor = nullptr;
        
        switch (stmt->getType()) {
        case sql_parser::Statement::CREATE:
        case sql_parser::Statement::DROP:
        case sql_parser::Statement::ALTER:
            executor = std::make_unique<DDLExecutor>(db_manager_);
            break;
            
        case sql_parser::Statement::SELECT:
        case sql_parser::Statement::INSERT:
        case sql_parser::Statement::UPDATE:
        case sql_parser::Statement::DELETE:
            executor = std::make_unique<DMLExecutor>(db_manager_);
            break;
            
        case sql_parser::Statement::CREATE_USER:
        case sql_parser::Statement::DROP_USER:
        case sql_parser::Statement::GRANT:
        case sql_parser::Statement::REVOKE:
            executor = std::make_unique<DCLExecutor>(db_manager_, user_manager_);
            break;
            
        case sql_parser::Statement::USE:
        case sql_parser::Statement::SHOW:
            executor = std::make_unique<UtilityExecutor>(db_manager_);
            break;
            
        default:
            return "Unsupported statement type";
        }
        
        // 执行语句
        ExecutionResult result = executor->execute(std::move(stmt));
        
        // 返回结果
        if (result.success) {
            return result.message.empty() ? "Query executed successfully" : result.message;
        } else {
            return "Error: " + result.message;
        }
    } catch (const std::exception& e) {
        return "Exception occurred: " + std::string(e.what());
    }
}

std::string SqlExecutor::ExecuteFile(const std::string &file_path) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        return "Failed to open file: " + file_path;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    return Execute(buffer.str());
}

// 获取最后一次执行的错误信息
std::string SqlExecutor::GetLastError() const {
    return last_error_;
}

// 设置错误信息
void SqlExecutor::SetError(const std::string& error) {
    last_error_ = error;
}

} // namespace sqlcc