#include "sql_parser/ast/ast_node.h"
#ifndef SQLCC_PROCEDURE_PROCEDURE_PARSER_H
#define SQLCC_PROCEDURE_PROCEDURE_PARSER_H

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

namespace sqlcc {
namespace procedure {

class ProcedureAST;
class Statement;
class VariableDeclaration;
class AssignmentStatement;
class IfStatement;
class WhileStatement;
class CallStatement;

/**
 * @brief 存储过程解析器
 *
 * 解析存储过程语言，支持：
 * - 变量声明和赋值
 * - 控制流 (IF-ELSE, WHILE)
 * - SQL语句调用
 * - 过程调用
 */
class ProcedureParser {
public:
    ProcedureParser();
    ~ProcedureParser();

    /**
     * 解析存储过程代码
     * @param code 存储过程代码
     * @return 解析后的AST
     */
    std::unique_ptr<ProcedureAST> parse(const std::string& code);

    /**
     * 获取解析错误信息
     */
    const std::string& getErrorMessage() const;

    /**
     * 检查是否有解析错误
     */
    bool hasError() const;

private:
    std::string error_message_;
    bool has_error_;
    size_t position_;
    std::string code_;

    // 解析辅助方法
    void skipWhitespace();
    bool isAtEnd() const;
    char peek() const;
    char advance();
    bool match(char expected);

    // 关键字检查
    bool isKeyword(const std::string& word);
    bool matchKeyword(const std::string& keyword);

    // 解析方法
    std::unique_ptr<ProcedureAST> parseProcedure();
    std::unique_ptr<Statement> parseStatement();
    std::unique_ptr<VariableDeclaration> parseVariableDeclaration();
    std::unique_ptr<AssignmentStatement> parseAssignment();
    std::unique_ptr<IfStatement> parseIfStatement();
    std::unique_ptr<WhileStatement> parseWhileStatement();
    std::unique_ptr<CallStatement> parseCallStatement();

    // 表达式解析
    std::string parseExpression();
    std::string parseIdentifier();
    std::string parseStringLiteral();
    std::string parseNumberLiteral();

    // 错误处理
    void setError(const std::string& message);
    void setError(const std::string& message, size_t position);
};

/**
 * @brief 存储过程AST节点基类
 */
class ProcedureAST {
public:
    virtual ~ProcedureAST() = default;
    virtual void accept(class ProcedureVisitor& visitor) = 0;
};

/**
 * @brief 语句基类
 */
class Statement : public ProcedureAST {
public:
    virtual ~Statement() = default;
};

/**
 * @brief 变量声明语句
 */
class VariableDeclaration : public Statement {
public:
    VariableDeclaration(const std::string& name, const std::string& type,
                       const std::string& defaultValue = "");
    ~VariableDeclaration() override;

    const std::string& getName() const;
    const std::string& getType() const;
    const std::string& getDefaultValue() const;
    bool hasDefaultValue() const;

    void accept(ProcedureVisitor& visitor) override;

private:
    std::string name_;
    std::string type_;
    std::string default_value_;
    bool has_default_;
};

/**
 * @brief 赋值语句
 */
class AssignmentStatement : public Statement {
public:
    AssignmentStatement(const std::string& variable, const std::string& expression);
    ~AssignmentStatement() override;

    const std::string& getVariable() const;
    const std::string& getExpression() const;

    void accept(ProcedureVisitor& visitor) override;

private:
    std::string variable_;
    std::string expression_;
};

/**
 * @brief IF语句
 */
class IfStatement : public Statement {
public:
    IfStatement(const std::string& condition);
    ~IfStatement() override;

    const std::string& getCondition() const;
    const std::vector<std::unique_ptr<Statement>>& getThenBranch() const;
    const std::vector<std::unique_ptr<Statement>>& getElseBranch() const;

    void addThenStatement(std::unique_ptr<Statement> stmt);
    void addElseStatement(std::unique_ptr<Statement> stmt);

    void accept(ProcedureVisitor& visitor) override;

private:
    std::string condition_;
    std::vector<std::unique_ptr<Statement>> then_branch_;
    std::vector<std::unique_ptr<Statement>> else_branch_;
};

/**
 * @brief WHILE语句
 */
class WhileStatement : public Statement {
public:
    WhileStatement(const std::string& condition);
    ~WhileStatement() override;

    const std::string& getCondition() const;
    const std::vector<std::unique_ptr<Statement>>& getBody() const;

    void addStatement(std::unique_ptr<Statement> stmt);

    void accept(ProcedureVisitor& visitor) override;

private:
    std::string condition_;
    std::vector<std::unique_ptr<Statement>> body_;
};

/**
 * @brief 调用语句 (SQL或过程调用)
 */
class CallStatement : public Statement {
public:
    enum CallType { SQL_CALL, PROCEDURE_CALL };

    CallStatement(CallType type, const std::string& statement);
    ~CallStatement() override;

    CallType getCallType() const;
    const std::string& getStatement() const;

    void accept(ProcedureVisitor& visitor) override;

private:
    CallType type_;
    std::string statement_;
};

/**
 * @brief 过程定义
 */
class ProcedureDefinition : public ProcedureAST {
public:
    ProcedureDefinition(const std::string& name);
    ~ProcedureDefinition() override;

    const std::string& getName() const;
    void setName(const std::string& name);
    const std::vector<std::unique_ptr<Statement>>& getBody() const;

    void addStatement(std::unique_ptr<Statement> stmt);

    void accept(ProcedureVisitor& visitor) override;

private:
    std::string name_;
    std::vector<std::unique_ptr<Statement>> body_;
};

/**
 * @brief 访问者模式接口
 */
class ProcedureVisitor {
public:
    virtual ~ProcedureVisitor() = default;

    virtual void visitVariableDeclaration(VariableDeclaration& node) = 0;
    virtual void visitAssignmentStatement(AssignmentStatement& node) = 0;
    virtual void visitIfStatement(IfStatement& node) = 0;
    virtual void visitWhileStatement(WhileStatement& node) = 0;
    virtual void visitCallStatement(CallStatement& node) = 0;
    virtual void visitProcedureDefinition(ProcedureDefinition& node) = 0;
};

} // namespace procedure
} // namespace sqlcc

#endif // SQLCC_PROCEDURE_PROCEDURE_PARSER_H
