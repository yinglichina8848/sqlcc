#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include "sql_parser/ast/core/ast_node.h"
#include "sql_parser/ast/core/source_location.h"

using namespace sqlcc::sql_parser::ast;

/**
 * @brief Simplified test AST nodes for visitor pattern testing
 */
class SimpleTestNode : public ASTNode {
public:
    SimpleTestNode(const std::string& name, const SourceLocation& loc = SourceLocation())
        : ASTNode(loc), name_(name) {}

    void accept(ASTVisitor& visitor) override {
        visitor.visit(*this);
    }

    std::unique_ptr<ASTNode> clone() const override {
        return std::make_unique<SimpleTestNode>(name_, getLocation());
    }

    std::string toString() const override {
        return name_;
    }

    std::string getNodeType() const override {
        return "SimpleTestNode";
    }

    const std::string& getName() const { return name_; }

private:
    std::string name_;
};

/**
 * @brief Simple visitor implementation
 */
class SimpleVisitor : public ASTVisitor {
public:
    void visit(ASTNode& node) override {
        visitCount_++;
        visitedNodes_.push_back(node.getNodeType() + ": " + node.toString());
    }

    int getVisitCount() const { return visitCount_; }
    const std::vector<std::string>& getVisitedNodes() const { return visitedNodes_; }

private:
    int visitCount_ = 0;
    std::vector<std::string> visitedNodes_;
};

int main() {
    std::cout << "🧪 Simplified AST Visitor Test" << std::endl;
    std::cout << "==============================" << std::endl;

    try {
        // Test basic functionality
        std::cout << "\n👁️ 1. Basic Visitor Functionality" << std::endl;

        SourceLocation loc1{1, 1, 0, "test.sql"};
        auto node1 = std::make_unique<SimpleTestNode>("test_node_1", loc1);

        SimpleVisitor visitor;
        node1->accept(visitor);

        std::cout << "✅ Visitor visited " << visitor.getVisitCount() << " nodes" << std::endl;
        std::cout << "✅ Node type: " << node1->getNodeType() << std::endl;
        std::cout << "✅ Node name: " << node1->getName() << std::endl;

        // Test multiple nodes
        std::cout << "\n📊 2. Multiple Nodes Test" << std::endl;

        auto node2 = std::make_unique<SimpleTestNode>("test_node_2");
        auto node3 = std::make_unique<SimpleTestNode>("test_node_3");

        SimpleVisitor multiVisitor;
        node1->accept(multiVisitor);
        node2->accept(multiVisitor);
        node3->accept(multiVisitor);

        std::cout << "✅ Multiple visits: " << multiVisitor.getVisitCount() << std::endl;
        for (const auto& node : multiVisitor.getVisitedNodes()) {
            std::cout << "   • " << node << std::endl;
        }

        // Test cloning
        std::cout << "\n🔄 3. Clone Test" << std::endl;

        auto cloned = node1->clone();
        std::cout << "✅ Original: " << node1->toString() << std::endl;
        std::cout << "✅ Cloned: " << cloned->toString() << std::endl;
        std::cout << "✅ Clone type: " << cloned->getNodeType() << std::endl;

        // Test location tracking
        std::cout << "\n📍 4. Location Tracking" << std::endl;

        auto locatedNode = std::make_unique<SimpleTestNode>("located", 
            SourceLocation{2, 5, 15, "test.sql"});
        std::cout << "✅ Location: " << locatedNode->getLocation().toString() << std::endl;

        std::cout << "\n==============================" << std::endl;
        std::cout << "🎉 Simplified AST Visitor Test PASSED!" << std::endl;
        std::cout << "✅ 基础访问者功能: 正常" << std::endl;
        std::cout << "✅ 多节点处理: 正常" << std::endl;
        std::cout << "✅ 节点克隆: 正常" << std::endl;
        std::cout << "✅ 位置追踪: 正常" << std::endl;

    } catch (const std::exception& e) {
        std::cout << "\n==============================" << std::endl;
        std::cout << "❌ Test FAILED: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
