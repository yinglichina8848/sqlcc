#ifndef SQLCC_FUNCTION_AST_H
#define SQLCC_FUNCTION_AST_H

#include <memory>
#include <string>
#include <vector>
#include "ast_node.h"

namespace sqlcc {
namespace sql_parser {

// 函数特征枚举
enum class FunctionCharacteristic {
    DETERMINISTIC,
    NOT_DETERMINISTIC,
    CONTAINS_SQL,
    READS_SQL_DATA,
    MODIFIES_SQL_DATA
};

// 函数参数结构
struct FunctionParameter {
    std::string name;
    std::string type;
    bool is_in = true;
    bool is_out = false;
    bool is_inout = false;
};

// 函数定义类
class FunctionDefinition {
public:
    FunctionDefinition(const std::string& name, const std::string& return_type);
    ~FunctionDefinition();

    // 参数管理
    void addParameter(const FunctionParameter& param);
    void addCharacteristic(const std::string& characteristic);
    
    // 属性设置
    void setBody(const std::string& body);
    void setLanguage(const std::string& language);

    // 属性获取
    const std::string& getName() const { return name_; }
    const std::string& getReturnType() const { return return_type_; }
    const std::vector<FunctionParameter>& getParameters() const { return parameters_; }
    const std::vector<std::string>& getCharacteristics() const { return characteristics_; }
    const std::string& getBody() const { return body_; }
    const std::string& getLanguage() const { return language_; }

    // 特征检查
    bool isDeterministic() const;
    bool containsSql() const;
    bool readsSqlData() const;
    bool modifiesSqlData() const;

    // 工具函数
    static std::string characteristicToString(FunctionCharacteristic characteristic);
    static FunctionCharacteristic stringToCharacteristic(const std::string& str);

private:
    std::string name_;
    std::string return_type_;
    std::vector<FunctionParameter> parameters_;
    std::vector<std::string> characteristics_;
    std::string body_;
    std::string language_;
};

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
    explicit CreateFunctionStatement(std::unique_ptr<FunctionDefinition> function_def);
    ~CreateFunctionStatement() override;

    bool isValid() const;
    
    const FunctionDefinition* getFunctionDefinition() const { return function_def_.get(); }

private:
    std::unique_ptr<FunctionDefinition> function_def_;
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
