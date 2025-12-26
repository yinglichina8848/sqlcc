#include "sql_parser/ast_node.h"
#include "../../include/procedure/procedure_parser.h"
#include <cctype>
#include <algorithm>
#include <unordered_set>

namespace sqlcc {
namespace procedure {

// ==================== ProcedureParser Implementation ====================

ProcedureParser::ProcedureParser()
    : error_message_(""), has_error_(false), position_(0), code_("") {}

ProcedureParser::~ProcedureParser() {}

std::unique_ptr<ProcedureAST> ProcedureParser::parse(const std::string& code) {
    code_ = code;
    position_ = 0;
    has_error_ = false;
    error_message_ = "";

    try {
        return parseProcedure();
    } catch (const std::exception& e) {
        setError(std::string("Parse error: ") + e.what());
        return nullptr;
    }
}

const std::string& ProcedureParser::getErrorMessage() const {
    return error_message_;
}

bool ProcedureParser::hasError() const {
    return has_error_;
}

void ProcedureParser::skipWhitespace() {
    while (!isAtEnd() && std::isspace(peek())) {
        advance();
    }
}

bool ProcedureParser::isAtEnd() const {
    return position_ >= code_.size();
}

char ProcedureParser::peek() const {
    if (isAtEnd()) return '\0';
    return code_[position_];
}

char ProcedureParser::advance() {
    if (!isAtEnd()) {
        return code_[position_++];
    }
    return '\0';
}

bool ProcedureParser::match(char expected) {
    if (isAtEnd() || peek() != expected) {
        return false;
    }
    advance();
    return true;
}

bool ProcedureParser::isKeyword(const std::string& word) {
    static const std::unordered_set<std::string> keywords = {
        "DECLARE", "SET", "IF", "THEN", "ELSE", "END", "WHILE", "DO",
        "BEGIN", "SELECT", "INSERT", "UPDATE", "DELETE", "CALL"
    };

    std::string upperWord = word;
    std::transform(upperWord.begin(), upperWord.end(), upperWord.begin(), ::toupper);
    return keywords.find(upperWord) != keywords.end();
}

bool ProcedureParser::matchKeyword(const std::string& keyword) {
    skipWhitespace();
    size_t start = position_;

    // 匹配关键字
    for (char c : keyword) {
        if (isAtEnd() || std::toupper(peek()) != c) {
            position_ = start; // 回退
            return false;
        }
        advance();
    }

    // 检查后面是否是标识符结束符
    if (!isAtEnd() && (std::isalnum(peek()) || peek() == '_')) {
        position_ = start; // 回退
        return false;
    }

    return true;
}

std::unique_ptr<ProcedureAST> ProcedureParser::parseProcedure() {
    skipWhitespace();

    if (!matchKeyword("BEGIN")) {
        setError("Expected 'BEGIN' at procedure start");
        return nullptr;
    }

    auto procedure = std::make_unique<ProcedureDefinition>("anonymous");

    while (!isAtEnd() && !matchKeyword("END")) {
        auto stmt = parseStatement();
        if (!stmt) {
            return nullptr;
        }
        procedure->addStatement(std::move(stmt));

        // 语句分隔符
        skipWhitespace();
        if (!isAtEnd() && peek() == ';') {
            advance();
        }
    }

    if (has_error_) {
        return nullptr;
    }

    return procedure;
}

std::unique_ptr<Statement> ProcedureParser::parseStatement() {
    skipWhitespace();

    if (matchKeyword("DECLARE")) {
        return parseVariableDeclaration();
    } else if (matchKeyword("SET")) {
        return parseAssignment();
    } else if (matchKeyword("IF")) {
        return parseIfStatement();
    } else if (matchKeyword("WHILE")) {
        return parseWhileStatement();
    } else if (matchKeyword("SELECT") || matchKeyword("INSERT") ||
               matchKeyword("UPDATE") || matchKeyword("DELETE") ||
               matchKeyword("CALL")) {
        // 回退关键字位置
        while (position_ > 0 && !std::isspace(code_[position_-1])) {
            position_--;
        }
        return parseCallStatement();
    } else {
        setError("Unknown statement type");
        return nullptr;
    }
}

std::unique_ptr<VariableDeclaration> ProcedureParser::parseVariableDeclaration() {
    skipWhitespace();

    std::string name = parseIdentifier();
    if (name.empty()) {
        setError("Expected variable name in declaration");
        return nullptr;
    }

    if (!matchKeyword("AS")) {
        setError("Expected 'AS' in variable declaration");
        return nullptr;
    }

    std::string type = parseIdentifier();
    if (type.empty()) {
        setError("Expected variable type in declaration");
        return nullptr;
    }

    std::string defaultValue;
    if (match('=')) {
        defaultValue = parseExpression();
        if (defaultValue.empty()) {
            setError("Expected default value in variable declaration");
            return nullptr;
        }
    }

    return std::make_unique<VariableDeclaration>(name, type, defaultValue);
}

std::unique_ptr<AssignmentStatement> ProcedureParser::parseAssignment() {
    skipWhitespace();

    std::string variable = parseIdentifier();
    if (variable.empty()) {
        setError("Expected variable name in assignment");
        return nullptr;
    }

    if (!match('=')) {
        setError("Expected '=' in assignment");
        return nullptr;
    }

    std::string expression = parseExpression();
    if (expression.empty()) {
        setError("Expected expression in assignment");
        return nullptr;
    }

    return std::make_unique<AssignmentStatement>(variable, expression);
}

std::unique_ptr<IfStatement> ProcedureParser::parseIfStatement() {
    skipWhitespace();

    std::string condition = parseExpression();
    if (condition.empty()) {
        setError("Expected condition in IF statement");
        return nullptr;
    }

    if (!matchKeyword("THEN")) {
        setError("Expected 'THEN' in IF statement");
        return nullptr;
    }

    auto ifStmt = std::make_unique<IfStatement>(condition);

    // 解析THEN分支
    while (!isAtEnd() && !matchKeyword("ELSE") && !matchKeyword("END")) {
        auto stmt = parseStatement();
        if (!stmt) {
            return nullptr;
        }
        ifStmt->addThenStatement(std::move(stmt));

        skipWhitespace();
        if (!isAtEnd() && peek() == ';') {
            advance();
        }
    }

    // 解析ELSE分支
    if (matchKeyword("ELSE")) {
        while (!isAtEnd() && !matchKeyword("END")) {
            auto stmt = parseStatement();
            if (!stmt) {
                return nullptr;
            }
            ifStmt->addElseStatement(std::move(stmt));

            skipWhitespace();
            if (!isAtEnd() && peek() == ';') {
                advance();
            }
        }
    }

    if (!matchKeyword("END")) {
        setError("Expected 'END' for IF statement");
        return nullptr;
    }

    if (!matchKeyword("IF")) {
        setError("Expected 'IF' after END");
        return nullptr;
    }

    return ifStmt;
}

std::unique_ptr<WhileStatement> ProcedureParser::parseWhileStatement() {
    skipWhitespace();

    std::string condition = parseExpression();
    if (condition.empty()) {
        setError("Expected condition in WHILE statement");
        return nullptr;
    }

    if (!matchKeyword("DO")) {
        setError("Expected 'DO' in WHILE statement");
        return nullptr;
    }

    auto whileStmt = std::make_unique<WhileStatement>(condition);

    while (!isAtEnd() && !matchKeyword("END")) {
        auto stmt = parseStatement();
        if (!stmt) {
            return nullptr;
        }
        whileStmt->addStatement(std::move(stmt));

        skipWhitespace();
        if (!isAtEnd() && peek() == ';') {
            advance();
        }
    }

    if (!matchKeyword("WHILE")) {
        setError("Expected 'WHILE' after END");
        return nullptr;
    }

    return whileStmt;
}

std::unique_ptr<CallStatement> ProcedureParser::parseCallStatement() {
    skipWhitespace();

    // 简单实现：读取到分号或语句结束
    size_t start = position_;
    std::string statement;

    while (!isAtEnd() && peek() != ';') {
        statement += advance();
    }

    // 去除末尾空白
    while (!statement.empty() && std::isspace(statement.back())) {
        statement.pop_back();
    }

    if (statement.empty()) {
        setError("Empty call statement");
        return nullptr;
    }

    // 判断调用类型
    CallStatement::CallType type = CallStatement::SQL_CALL;
    std::string upperStmt = statement;
    std::transform(upperStmt.begin(), upperStmt.end(), upperStmt.begin(), ::toupper);

    if (upperStmt.find("CALL ") == 0) {
        type = CallStatement::PROCEDURE_CALL;
    }

    return std::make_unique<CallStatement>(type, statement);
}

std::string ProcedureParser::parseExpression() {
    skipWhitespace();

    // 简单表达式解析
    std::string expr;

    while (!isAtEnd() && peek() != ' ' && peek() != '\t' && peek() != '\n' &&
           peek() != '\r' && peek() != ';' && peek() != ')') {
        char c = peek();
        if (c == '(') {
            expr += advance();
            expr += parseExpression();
            if (!match(')')) {
                setError("Expected ')' in expression");
                return "";
            }
            expr += ')';
        } else if (c == '"' || c == '\'') {
            expr += parseStringLiteral();
        } else if (std::isdigit(c)) {
            expr += parseNumberLiteral();
        } else if (std::isalpha(c) || c == '_') {
            expr += parseIdentifier();
        } else {
            expr += advance();
        }
    }

    return expr;
}

std::string ProcedureParser::parseIdentifier() {
    skipWhitespace();

    if (!std::isalpha(peek()) && peek() != '_') {
        return "";
    }

    std::string identifier;
    identifier += advance();

    while (!isAtEnd() && (std::isalnum(peek()) || peek() == '_')) {
        identifier += advance();
    }

    return identifier;
}

std::string ProcedureParser::parseStringLiteral() {
    char quote = advance(); // 消费引号
    std::string literal;

    while (!isAtEnd() && peek() != quote) {
        if (peek() == '\\') {
            advance(); // 消费转义符
            if (!isAtEnd()) {
                literal += advance();
            }
        } else {
            literal += advance();
        }
    }

    if (!match(quote)) {
        setError("Unterminated string literal");
        return "";
    }

    return std::string(1, quote) + literal + quote;
}

std::string ProcedureParser::parseNumberLiteral() {
    std::string number;

    while (!isAtEnd() && std::isdigit(peek())) {
        number += advance();
    }

    if (!isAtEnd() && peek() == '.') {
        number += advance();
        while (!isAtEnd() && std::isdigit(peek())) {
            number += advance();
        }
    }

    return number;
}

void ProcedureParser::setError(const std::string& message) {
    has_error_ = true;
    error_message_ = message;
}

void ProcedureParser::setError(const std::string& message, size_t position) {
    has_error_ = true;
    error_message_ = message + " at position " + std::to_string(position);
}

// ==================== AST Node Implementations ====================

VariableDeclaration::VariableDeclaration(const std::string& name, const std::string& type,
                                       const std::string& defaultValue)
    : name_(name), type_(type), default_value_(defaultValue),
      has_default_(!defaultValue.empty()) {}

VariableDeclaration::~VariableDeclaration() {}

const std::string& VariableDeclaration::getName() const { return name_; }
const std::string& VariableDeclaration::getType() const { return type_; }
const std::string& VariableDeclaration::getDefaultValue() const { return default_value_; }
bool VariableDeclaration::hasDefaultValue() const { return has_default_; }

void VariableDeclaration::accept(ProcedureVisitor& visitor) {
    visitor.visitVariableDeclaration(*this);
}

AssignmentStatement::AssignmentStatement(const std::string& variable, const std::string& expression)
    : variable_(variable), expression_(expression) {}

AssignmentStatement::~AssignmentStatement() {}

const std::string& AssignmentStatement::getVariable() const { return variable_; }
const std::string& AssignmentStatement::getExpression() const { return expression_; }

void AssignmentStatement::accept(ProcedureVisitor& visitor) {
    visitor.visitAssignmentStatement(*this);
}

IfStatement::IfStatement(const std::string& condition)
    : condition_(condition) {}

IfStatement::~IfStatement() {}

const std::string& IfStatement::getCondition() const { return condition_; }

const std::vector<std::unique_ptr<Statement>>& IfStatement::getThenBranch() const {
    return then_branch_;
}

const std::vector<std::unique_ptr<Statement>>& IfStatement::getElseBranch() const {
    return else_branch_;
}

void IfStatement::addThenStatement(std::unique_ptr<Statement> stmt) {
    then_branch_.push_back(std::move(stmt));
}

void IfStatement::addElseStatement(std::unique_ptr<Statement> stmt) {
    else_branch_.push_back(std::move(stmt));
}

void IfStatement::accept(ProcedureVisitor& visitor) {
    visitor.visitIfStatement(*this);
}

WhileStatement::WhileStatement(const std::string& condition)
    : condition_(condition) {}

WhileStatement::~WhileStatement() {}

const std::string& WhileStatement::getCondition() const { return condition_; }

const std::vector<std::unique_ptr<Statement>>& WhileStatement::getBody() const {
    return body_;
}

void WhileStatement::addStatement(std::unique_ptr<Statement> stmt) {
    body_.push_back(std::move(stmt));
}

void WhileStatement::accept(ProcedureVisitor& visitor) {
    visitor.visitWhileStatement(*this);
}

CallStatement::CallStatement(CallType type, const std::string& statement)
    : type_(type), statement_(statement) {}

CallStatement::~CallStatement() {}

CallStatement::CallType CallStatement::getCallType() const { return type_; }
const std::string& CallStatement::getStatement() const { return statement_; }

void CallStatement::accept(ProcedureVisitor& visitor) {
    visitor.visitCallStatement(*this);
}

ProcedureDefinition::ProcedureDefinition(const std::string& name)
    : name_(name) {}

ProcedureDefinition::~ProcedureDefinition() {}

const std::string& ProcedureDefinition::getName() const { return name_; }

const std::vector<std::unique_ptr<Statement>>& ProcedureDefinition::getBody() const {
    return body_;
}

void ProcedureDefinition::addStatement(std::unique_ptr<Statement> stmt) {
    body_.push_back(std::move(stmt));
}

void ProcedureDefinition::accept(ProcedureVisitor& visitor) {
    visitor.visitProcedureDefinition(*this);
}

} // namespace procedure
} // namespace sqlcc
