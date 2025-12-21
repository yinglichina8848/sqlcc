#pragma once

#include <memory>
#include <unordered_map>
#include <string>
#include <vector>

namespace sqlcc {
namespace sql_executor {

// Forward declarations
namespace sql_parser {
class CreateFunctionStatement;
class AlterFunctionStatement;
class DropFunctionStatement;
class CreateProcedureStatement;
class CallStatement;
} // namespace sql_parser

/**
 * Procedure and Function Manager - 存储过程和函数管理器
 * 处理CREATE/ALTER/DROP FUNCTION/PROCEDURE、CALL语句
 */
class ProcedureFunctionManager {
public:
    static ProcedureFunctionManager& getInstance();

    // 函数管理
    bool createFunction(const sql_parser::CreateFunctionStatement& stmt);
    bool alterFunction(const sql_parser::AlterFunctionStatement& stmt);
    bool dropFunction(const sql_parser::DropFunctionStatement& stmt);
    bool functionExists(const std::string& functionName) const;

    // 过程管理
    bool createProcedure(const sql_parser::CreateProcedureStatement& stmt);
    bool dropProcedure(const std::string& procedureName);
    bool procedureExists(const std::string& procedureName) const;

    // 调用执行
    bool callProcedure(const sql_parser::CallStatement& stmt);
    std::string callFunction(const std::string& functionName,
                            const std::vector<std::string>& args);

    // 信息查询
    std::string getFunctionInfo(const std::string& functionName) const;
    std::string getProcedureInfo(const std::string& procedureName) const;
    std::vector<std::string> listFunctions() const;
    std::vector<std::string> listProcedures() const;

private:
    ProcedureFunctionManager() = default;

    struct FunctionInfo {
        std::string name;
        std::string returnType;
        std::vector<std::string> parameters;
        std::string body;
        std::string language;
        bool isAggregate = false;
        long created_time;
        std::string created_by;
    };

    struct ProcedureInfo {
        std::string name;
        std::vector<std::string> parameters;
        std::string body;
        std::string language;
        long created_time;
        std::string created_by;
    };

    std::unordered_map<std::string, FunctionInfo> functions_;
    std::unordered_map<std::string, ProcedureInfo> procedures_;
    long next_id_ = 1;
};

} // namespace sql_executor
} // namespace sqlcc
