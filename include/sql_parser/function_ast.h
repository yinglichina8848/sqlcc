#ifndef SQLCC_FUNCTION_AST_H
#define SQLCC_FUNCTION_AST_H

#include <memory>
#include <string>
#include <vector>
#include "ast_node.h"
#include "function/function_definition.h"

namespace sqlcc {
namespace sql_parser {

// 函数调用表达式
class FunctionCallExpression : public Expression {
public:
    explicit FunctionCallExpression(const std::string& function_name);
    ~FunctionCallExpression() override;

    void addArgument(std::unique_ptr<Expression> argument);
    
    const std::string& getFunctionName() const { return function_name_; }
    const std::vector<std::unique_ptr<Expression>>& getArguments() const { return arguments_; }

private:
    std::string function_name_;
    std::vector<std::unique_ptr<Expression>> arguments_;
};

// 函数调用语句
class FunctionCallStatement : public Statement {
public:
    explicit FunctionCallStatement(const std::string& function_name);
    ~FunctionCallStatement() override;

    void addArgument(std::unique_ptr<Expression> argument);
    
    const std::string& getFunctionName() const { return function_name_; }
    const std::vector<std::unique_ptr<Expression>>& getArguments() const { return arguments_; }

private:
    std::string function_name_;
    std::vector<std::unique_ptr<Expression>> arguments_;
};

// 创建函数语句
class CreateFunctionStatement : public Statement {
public:
    explicit CreateFunctionStatement(std::unique_ptr<sqlcc::sql_parser::FunctionDefinition> function_def);
    ~CreateFunctionStatement() override;

    bool isValid() const;
    
    const sqlcc::sql_parser::FunctionDefinition* getFunctionDefinition() const { return function_def_.get(); }

private:
    std::unique_ptr<sqlcc::sql_parser::FunctionDefinition> function_def_;
};

// 删除函数语句
class DropFunctionStatement : public Statement {
public:
    enum DropBehavior {
        RESTRICT,
        CASCADE
    };

    explicit DropFunctionStatement(const std::string& function_name);
    ~DropFunctionStatement() override;

    void setIfExists(bool if_exists) { if_exists_ = if_exists; }
    void setDropBehavior(DropBehavior behavior) { drop_behavior_ = behavior; }
    
    const std::string& getFunctionName() const { return function_name_; }
    bool getIfExists() const { return if_exists_; }
    DropBehavior getDropBehavior() const { return drop_behavior_; }

private:
    std::string function_name_;
    DropBehavior drop_behavior_;
    bool if_exists_;
};

// 修改函数语句
class AlterFunctionStatement : public Statement {
public:
    enum Action {
        RENAME_TO,
        SET_SCHEMA
    };

    explicit AlterFunctionStatement(const std::string& function_name);
    ~AlterFunctionStatement() override;

    void setAction(Action action) { action_ = action; }
    void setNewName(const std::string& new_name) { new_name_ = new_name; }
    void setNewSchema(const std::string& new_schema) { new_schema_ = new_schema; }
    
    const std::string& getFunctionName() const { return function_name_; }
    Action getAction() const { return action_; }
    const std::string& getNewName() const { return new_name_; }
    const std::string& getNewSchema() const { return new_schema_; }

private:
    std::string function_name_;
    Action action_;
    std::string new_name_;
    std::string new_schema_;
};

} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_FUNCTION_AST_H
