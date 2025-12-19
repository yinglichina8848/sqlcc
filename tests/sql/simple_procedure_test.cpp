#include <iostream>
#include <string>
#include <memory>
#include "include/procedure/procedure_parser.h"
#include "include/procedure/procedure_vm.h"
#include "include/core/sql_executor.h"

// 简单的SQL执行器实现，仅用于测试
class SimpleSqlExecutor : public sqlcc::SqlExecutor {
public:
    SimpleSqlExecutor() = default;
    ~SimpleSqlExecutor() override = default;

    std::string Execute(const std::string& sql) override {
        std::cout << "[SQL EXECUTOR] Executing: " << sql << std::endl;
        // 简单模拟SQL执行
        if (sql.find("SELECT") == 0) {
            return "EXECUTED: " + sql;
        } else if (sql.find("INSERT") == 0) {
            return "EXECUTED: " + sql;
        } else if (sql.find("UPDATE") == 0) {
            return "EXECUTED: " + sql;
        } else if (sql.find("DELETE") == 0) {
            return "EXECUTED: " + sql;
        }
        return "EXECUTED: " + sql;
    }

    std::string ExecuteFile(const std::string& file_path) override {
        return "NOT IMPLEMENTED";
    }

    std::string GetLastError() const override {
        return "";
    }

    std::string GetExecutionStats() const override {
        return "STATS";
    }

    void SetError(const std::string& error) override {
        std::cerr << "[SQL EXECUTOR ERROR] " << error << std::endl;
    }

    void ClearError() override {}

    bool InitializeSystemDatabase() override { return true; }

    std::unique_ptr<sql_parser::Statement> ParseSQL(const std::string& sql) override {
        return nullptr;
    }

    std::unique_ptr<UnifiedQueryPlan> CreateQueryPlan(std::unique_ptr<sql_parser::Statement> stmt) override {
        return nullptr;
    }

    bool InitializePermissionValidator() override { return true; }

    void UpdateCurrentDatabase(const std::string& sql) override {}
};

int main() {
    std::cout << "=== 存储过程和触发器功能验证测试 ===" << std::endl;

    // 创建简单的SQL执行器
    auto sql_executor = std::make_shared<SimpleSqlExecutor>();

    // 测试存储过程解析器
    std::cout << "\n--- 测试存储过程解析器 ---" << std::endl;

    sqlcc::procedure::ProcedureParser parser;

    // 测试简单的存储过程代码
    std::string procedure_code = R"(
        DECLARE counter AS INT = 0;
        SET counter = counter + 1;
        SELECT counter;
    )";

    std::cout << "解析存储过程代码:" << std::endl;
    std::cout << procedure_code << std::endl;

    auto ast = parser.parse(procedure_code);
    if (ast) {
        std::cout << "✓ 存储过程解析成功!" << std::endl;

        // 测试虚拟机执行
        std::cout << "\n--- 测试存储过程虚拟机执行 ---" << std::endl;

        sqlcc::procedure::ProcedureVM vm;
        vm.initialize(sql_executor.get());

        sqlcc::procedure::ProcedureContext context(sql_executor.get());

        bool execute_result = vm.execute(ast.get(), context);
        if (execute_result) {
            std::cout << "✓ 存储过程执行成功!" << std::endl;

            // 检查返回值
            const sqlcc::procedure::Value& return_value = context.getReturnValue();
            std::cout << "返回值: " << return_value.toString() << std::endl;
        } else {
            std::cout << "✗ 存储过程执行失败: " << vm.getLastError() << std::endl;
        }
    } else {
        std::cout << "✗ 存储过程解析失败: " << parser.getErrorMessage() << std::endl;
    }

    // 测试更复杂的存储过程
    std::cout << "\n--- 测试复杂存储过程 ---" << std::endl;

    std::string complex_procedure = R"(
        DECLARE x AS INT = 10;
        DECLARE result AS INT;

        IF x > 5 THEN
            SET result = x * 2;
            SELECT result;
        ELSE
            SET result = x + 5;
            SELECT result;
        END IF;
    )";

    std::cout << "解析复杂存储过程代码:" << std::endl;
    std::cout << complex_procedure << std::endl;

    auto complex_ast = parser.parse(complex_procedure);
    if (complex_ast) {
        std::cout << "✓ 复杂存储过程解析成功!" << std::endl;

        sqlcc::procedure::ProcedureContext complex_context(sql_executor.get());
        bool complex_execute_result = vm.execute(complex_ast.get(), complex_context);

        if (complex_execute_result) {
            std::cout << "✓ 复杂存储过程执行成功!" << std::endl;
            const sqlcc::procedure::Value& return_value = complex_context.getReturnValue();
            std::cout << "返回值: " << return_value.toString() << std::endl;
        } else {
            std::cout << "✗ 复杂存储过程执行失败: " << vm.getLastError() << std::endl;
        }
    } else {
        std::cout << "✗ 复杂存储过程解析失败: " << parser.getErrorMessage() << std::endl;
    }

    std::cout << "\n=== 测试完成 ===" << std::endl;
    return 0;
}
