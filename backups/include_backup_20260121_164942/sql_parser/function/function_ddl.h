/**
#include "sql_parser/ast_node.h"
 * @file function_ddl.h
 * @brief 函数DDL语句类定义
 *
 * Why: 需要专门的类来处理函数的创建、删除和修改操作
 * What: CreateFunctionStatement、DropFunctionStatement和AlterFunctionStatement类
 * How: 提供函数生命周期管理的DDL语句支持
 */

#pragma once

#include "function_definition.h"
#include <memory>
#include <string>

namespace sqlcc {
namespace sql_parser {

/**
 * @brief 创建函数语句
 *
 * 表示SQL中的CREATE FUNCTION语句
 */
class CreateFunctionStatement : public Statement {
public:
    /**
     * @brief 构造函数
     * @param function_def 函数定义对象
     */
    explicit CreateFunctionStatement(std::unique_ptr<FunctionDefinition> function_def);

    /**
     * @brief 析构函数
     */
    ~CreateFunctionStatement() override;

    /**
     * @brief 验证语句有效性
     * @return 语句是否有效
     */
    bool isValid() const;

    /**
     * @brief 获取函数定义
     * @return 函数定义对象的常量指针
     */
    const FunctionDefinition* getFunctionDefinition() const { return function_def_.get(); }

private:
    std::unique_ptr<FunctionDefinition> function_def_; ///< 函数定义
};

/**
 * @brief 删除函数语句
 *
 * 表示SQL中的DROP FUNCTION语句
 */
class DropFunctionStatement : public Statement {
public:
    /**
     * @brief 删除行为枚举
     */
    enum DropBehavior {
        RESTRICT, ///< 限制删除（默认）
        CASCADE   ///< 级联删除
    };

    /**
     * @brief 构造函数
     * @param function_name 函数名
     */
    explicit DropFunctionStatement(const std::string& function_name);

    /**
     * @brief 析构函数
     */
    ~DropFunctionStatement() override;

    /**
     * @brief 设置IF EXISTS选项
     * @param if_exists 是否使用IF EXISTS
     */
    void setIfExists(bool if_exists) { if_exists_ = if_exists; }

    /**
     * @brief 设置删除行为
     * @param behavior 删除行为
     */
    void setDropBehavior(DropBehavior behavior) { drop_behavior_ = behavior; }

    /**
     * @brief 获取函数名
     * @return 函数名常量引用
     */
    const std::string& getFunctionName() const { return function_name_; }

    /**
     * @brief 获取IF EXISTS选项
     * @return 是否使用IF EXISTS
     */
    bool getIfExists() const { return if_exists_; }

    /**
     * @brief 获取删除行为
     * @return 删除行为
     */
    DropBehavior getDropBehavior() const { return drop_behavior_; }

private:
    std::string function_name_; ///< 函数名称
    DropBehavior drop_behavior_; ///< 删除行为
    bool if_exists_;             ///< 是否使用IF EXISTS
};

/**
 * @brief 修改函数语句
 *
 * 表示SQL中的ALTER FUNCTION语句
 */
class AlterFunctionStatement : public Statement {
public:
    /**
     * @brief 修改动作枚举
     */
    enum Action {
        RENAME_TO,  ///< 重命名函数
        SET_SCHEMA  ///< 设置模式
    };

    /**
     * @brief 构造函数
     * @param function_name 函数名
     */
    explicit AlterFunctionStatement(const std::string& function_name);

    /**
     * @brief 析构函数
     */
    ~AlterFunctionStatement() override;

    /**
     * @brief 设置修改动作
     * @param action 修改动作
     */
    void setAction(Action action) { action_ = action; }

    /**
     * @brief 设置新名称（用于RENAME_TO）
     * @param new_name 新函数名
     */
    void setNewName(const std::string& new_name) { new_name_ = new_name; }

    /**
     * @brief 设置新模式（用于SET_SCHEMA）
     * @param new_schema 新模式名
     */
    void setNewSchema(const std::string& new_schema) { new_schema_ = new_schema; }

    /**
     * @brief 获取函数名
     * @return 函数名常量引用
     */
    const std::string& getFunctionName() const { return function_name_; }

    /**
     * @brief 获取修改动作
     * @return 修改动作
     */
    Action getAction() const { return action_; }

    /**
     * @brief 获取新名称
     * @return 新名称常量引用
     */
    const std::string& getNewName() const { return new_name_; }

    /**
     * @brief 获取新模式
     * @return 新模式常量引用
     */
    const std::string& getNewSchema() const { return new_schema_; }

private:
    std::string function_name_; ///< 函数名称
    Action action_;             ///< 修改动作
    std::string new_name_;      ///< 新名称（RENAME_TO使用）
    std::string new_schema_;    ///< 新模式（SET_SCHEMA使用）
};

} // namespace sql_parser
} // namespace sqlcc
