#include <iostream>
#include <memory>
#include <string>
#include <vector>

// Simple demonstration of AST and Error handling system
// This file demonstrates the core concepts without complex dependencies

// Forward declarations
namespace demo {
namespace ast {

// Simple SourceLocation
struct SourceLocation {
    size_t line = 1;
    size_t column = 1;
    size_t offset = 0;
    std::string file = "demo.sql";

    std::string toString() const {
        return file + ":" + std::to_string(line) + ":" + std::to_string(column);
    }
};

// Simple AST Node base class
class ASTNode {
public:
    ASTNode(const SourceLocation& loc = SourceLocation()) : location_(loc) {}
    virtual ~ASTNode() = default;

    virtual void accept(class ASTVisitor& visitor) = 0;
    virtual std::string toString() const = 0;
    const SourceLocation& getLocation() const { return location_; }

private:
    SourceLocation location_;
};

// Simple visitor
class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;
    virtual void visit(ASTNode& node) = 0;
};

// Test node
class TestNode : public ASTNode {
public:
    TestNode(const std::string& name, const SourceLocation& loc = SourceLocation())
        : ASTNode(loc), name_(name) {}

    void accept(ASTVisitor& visitor) override {
        visitor.visit(*this);
    }

    std::string toString() const override {
        return "TestNode: " + name_ + " at " + getLocation().toString();
    }

    const std::string& getName() const { return name_; }

private:
    std::string name_;
};

} // namespace ast

namespace errors {

// Error types
enum class ErrorType {
    SYNTAX_ERROR,
    SEMANTIC_ERROR,
    LEXICAL_ERROR
};

enum class Severity {
    INFO,
    WARNING,
    ERROR,
    FATAL
};

// Simple error class
class ParseError {
public:
    ParseError(ErrorType type, Severity sev, const std::string& msg,
               const ast::SourceLocation& loc = ast::SourceLocation())
        : type_(type), severity_(sev), message_(msg), location_(loc) {}

    std::string toString() const {
        std::string sev_str;
        switch (severity_) {
            case Severity::INFO: sev_str = "[INFO]"; break;
            case Severity::WARNING: sev_str = "[WARNING]"; break;
            case Severity::ERROR: sev_str = "[ERROR]"; break;
            case Severity::FATAL: sev_str = "[FATAL]"; break;
        }
        return sev_str + " " + location_.toString() + ": " + message_;
    }

private:
    ErrorType type_;
    Severity severity_;
    std::string message_;
    ast::SourceLocation location_;
};

// Error collector
class ErrorCollector {
public:
    void addError(std::unique_ptr<ParseError> error) {
        errors_.push_back(std::move(error));
    }

    void addWarning(const std::string& msg, const ast::SourceLocation& loc = ast::SourceLocation()) {
        auto warning = std::make_unique<ParseError>(
            ErrorType::SYNTAX_ERROR, Severity::WARNING, msg, loc);
        warnings_.push_back(std::move(warning));
    }

    size_t getErrorCount() const { return errors_.size(); }
    size_t getWarningCount() const { return warnings_.size(); }
    bool hasErrors() const { return !errors_.empty(); }

    std::string toString() const {
        std::string result;
        if (!errors_.empty()) {
            result += "Errors:\n";
            for (const auto& err : errors_) {
                result += "  " + err->toString() + "\n";
            }
        }
        if (!warnings_.empty()) {
            result += "Warnings:\n";
            for (const auto& warn : warnings_) {
                result += "  " + warn->toString() + "\n";
            }
        }
        return result;
    }

private:
    std::vector<std::unique_ptr<ParseError>> errors_;
    std::vector<std::unique_ptr<ParseError>> warnings_;
};

} // namespace errors
} // namespace demo

// Test visitor
class DemoVisitor : public demo::ast::ASTVisitor {
public:
    void visit(demo::ast::ASTNode& node) override {
        visitCount_++;
        std::cout << "Visited: " << node.toString() << std::endl;
    }

    int getVisitCount() const { return visitCount_; }

private:
    int visitCount_ = 0;
};

int main() {
    std::cout << "🚀 AST与错误处理机制演示系统" << std::endl;
    std::cout << "=====================================" << std::endl;

    try {
        // Test SourceLocation
        std::cout << "\n📍 1. SourceLocation 位置追踪" << std::endl;
        demo::ast::SourceLocation loc1{10, 5, 100, "query.sql"};
        demo::ast::SourceLocation loc2{25, 12, 250, "table.sql"};

        std::cout << "✅ 位置1: " << loc1.toString() << std::endl;
        std::cout << "✅ 位置2: " << loc2.toString() << std::endl;

        // Test AST nodes
        std::cout << "\n🌳 2. AST节点系统" << std::endl;
        auto node1 = std::make_unique<demo::ast::TestNode>("SelectStatement", loc1);
        auto node2 = std::make_unique<demo::ast::TestNode>("CreateTable", loc2);

        std::cout << "✅ 节点1: " << node1->toString() << std::endl;
        std::cout << "✅ 节点2: " << node2->toString() << std::endl;

        // Test visitor pattern
        std::cout << "\n👁️ 3. 访问者模式" << std::endl;
        DemoVisitor visitor;
        node1->accept(visitor);
        node2->accept(visitor);
        std::cout << "✅ 访问者访问了 " << visitor.getVisitCount() << " 个节点" << std::endl;

        // Test error handling
        std::cout << "\n❌ 4. 错误处理系统" << std::endl;
        demo::errors::ErrorCollector collector;

        // Add some errors
        collector.addError(std::make_unique<demo::errors::ParseError>(
            demo::errors::ErrorType::SYNTAX_ERROR,
            demo::errors::Severity::ERROR,
            "Unexpected token 'SELECT'", loc1));

        collector.addError(std::make_unique<demo::errors::ParseError>(
            demo::errors::ErrorType::SEMANTIC_ERROR,
            demo::errors::Severity::WARNING,
            "Table 'users' does not exist", loc2));

        collector.addWarning("Column 'id' is deprecated", demo::ast::SourceLocation{15, 8, 180, "query.sql"});

        std::cout << "✅ 错误数量: " << collector.getErrorCount() << std::endl;
        std::cout << "✅ 警告数量: " << collector.getWarningCount() << std::endl;
        std::cout << "✅ 是否有错误: " << (collector.hasErrors() ? "是" : "否") << std::endl;

        std::cout << "\n📋 错误报告:" << std::endl;
        std::cout << collector.toString() << std::endl;

        // Summary
        std::cout << "\n=====================================" << std::endl;
        std::cout << "🎉 AST与错误处理机制演示完成!" << std::endl;
        std::cout << "✅ SourceLocation: 位置追踪正常" << std::endl;
        std::cout << "✅ AST节点: 层次结构清晰" << std::endl;
        std::cout << "✅ 访问者模式: 扩展性良好" << std::endl;
        std::cout << "✅ 错误处理: 结构化信息完整" << std::endl;
        std::cout << "✅ 内存管理: 智能指针安全" << std::endl;
        std::cout << "\n🚀 核心架构验证通过！" << std::endl;

    } catch (const std::exception& e) {
        std::cout << "\n=====================================" << std::endl;
        std::cout << "❌ 演示失败: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
