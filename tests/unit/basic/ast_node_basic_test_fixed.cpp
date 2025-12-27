#include <iostream>
#include <string>
#include <cassert>
#include <memory>
#include "../../../include/sql_parser/ast_node.h"
#include "../../../include/sql_parser/ast_nodes.h"
#include "../../../include/sql_parser/node_visitor.h"

using namespace sqlcc::sql_parser;

// Test basic AST Node functionality
void test_ast_node_basic() {
    std::cout << "Testing basic AST Node functionality..." << std::endl;

    // Test creating a simple identifier expression node
    auto identifier = std::make_shared<IdentifierExpression>("test_table");
    assert(identifier->getName() == "test_table");

    // Test creating a literal node
    auto string_literal = std::make_shared<StringLiteralExpression>("'hello world'");
    assert(string_literal->getValue() == "'hello world'");

    auto numeric_literal = std::make_shared<NumericLiteralExpression>(42.0);
    assert(numeric_literal->getValue() == 42.0);

    auto bool_literal = std::make_shared<BooleanLiteralExpression>(true);
    assert(bool_literal->getValue() == true);

    std::cout << "Basic AST Node functionality test passed!" << std::endl;
}

// Test AST Node visitor pattern
void test_ast_node_visitor() {
    std::cout << "Testing AST Node visitor pattern..." << std::endl;

    // Create a simple visitor that counts nodes
    class TestVisitor : public NodeVisitor {
    private:
        int node_count = 0;
        int identifier_count = 0;
        int literal_count = 0;

    public:
        void visit(IdentifierExpression &node) override {
            node_count++;
            identifier_count++;
            std::cout << "Visited IdentifierExpression: " << node.getName() << std::endl;
        }

        void visit(StringLiteralExpression &node) override {
            node_count++;
            literal_count++;
            std::cout << "Visited StringLiteralExpression: " << node.getValue() << std::endl;
        }

        void visit(NumericLiteralExpression &node) override {
            node_count++;
            literal_count++;
            std::cout << "Visited NumericLiteralExpression: " << node.getValue() << std::endl;
        }

        void visit(BooleanLiteralExpression &node) override {
            node_count++;
            literal_count++;
            std::cout << "Visited BooleanLiteralExpression: " << (node.getValue() ? "true" : "false") << std::endl;
        }

        void visit(NullLiteralExpression &node) override {
            node_count++;
            literal_count++;
            std::cout << "Visited NullLiteralExpression" << std::endl;
        }

        // Implement all required visitor methods with empty implementations for this test
        void visit(CreateStatement &node) override { node_count++; }
        void visit(CreateViewStatement &node) override { node_count++; }
        void visit(SelectStatement &node) override { node_count++; }
        void visit(InsertStatement &node) override { node_count++; }
        void visit(UpdateStatement &node) override { node_count++; }
        void visit(DeleteStatement &node) override { node_count++; }
        void visit(DropStatement &node) override { node_count++; }
        void visit(AlterStatement &node) override { node_count++; }
        void visit(UseStatement &node) override { node_count++; }
        void visit(CreateIndexStatement &node) override { node_count++; }
        void visit(DropIndexStatement &node) override { node_count++; }
        void visit(CreateUserStatement &node) override { node_count++; }
        void visit(DropUserStatement &node) override { node_count++; }
        void visit(GrantStatement &node) override { node_count++; }
        void visit(RevokeStatement &node) override { node_count++; }
        void visit(ShowStatement &node) override { node_count++; }
        void visit(CommitStatement &node) override { node_count++; }
        void visit(RollbackStatement &node) override { node_count++; }
        void visit(CreateProcedureStatement &node) override { node_count++; }
        void visit(CallProcedureStatement &node) override { node_count++; }
        void visit(DropProcedureStatement &node) override { node_count++; }
        void visit(CreateTriggerStatement &node) override { node_count++; }
        void visit(DropTriggerStatement &node) override { node_count++; }
        void visit(AlterTriggerStatement &node) override { node_count++; }
        void visit(AlterViewStatement &node) override { node_count++; }
        void visit(DropViewStatement &node) override { node_count++; }
        void visit(BinaryExpression &node) override { node_count++; }
        void visit(SetOperation &node) override { node_count++; }
        void visit(CompositeSelectStatement &node) override { node_count++; }
        void visit(WindowFunction &node) override { node_count++; }
        void visit(WindowSpecification &node) override { node_count++; }
        void visit(WithRecursiveClause &node) override { node_count++; }

        int get_node_count() const { return node_count; }
        int get_identifier_count() const { return identifier_count; }
        int get_literal_count() const { return literal_count; }
    };

    TestVisitor visitor;

    // Test visiting different node types
    auto identifier = std::make_shared<IdentifierExpression>("test_table");
    identifier->accept(visitor);

    auto string_literal = std::make_shared<StringLiteralExpression>("'hello'");
    string_literal->accept(visitor);

    auto numeric_literal = std::make_shared<NumericLiteralExpression>(123.0);
    numeric_literal->accept(visitor);

    auto bool_literal = std::make_shared<BooleanLiteralExpression>(false);
    bool_literal->accept(visitor);

    // Verify visitor counts
    assert(visitor.get_node_count() == 4);
    assert(visitor.get_identifier_count() == 1);
    assert(visitor.get_literal_count() == 3);

    std::cout << "AST Node visitor pattern test passed!" << std::endl;
}

// Test AST Node expression types
void test_ast_node_expressions() {
    std::cout << "Testing AST Node expression types..." << std::endl;

    // Test creating different expression types
    auto identifier = std::make_shared<IdentifierExpression>("column_name");
    assert(identifier->getTypeName() == "IdentifierExpression");

    auto string_literal = std::make_shared<StringLiteralExpression>("'value'");
    assert(string_literal->getTypeName() == "StringLiteralExpression");

    auto numeric_literal = std::make_shared<NumericLiteralExpression>(42.5);
    assert(numeric_literal->getTypeName() == "NumericLiteralExpression");

    auto bool_literal = std::make_shared<BooleanLiteralExpression>(true);
    assert(bool_literal->getTypeName() == "BooleanLiteralExpression");

    auto null_literal = std::make_shared<NullLiteralExpression>();
    assert(null_literal->getTypeName() == "NullLiteralExpression");

    std::cout << "AST Node expression types test passed!" << std::endl;
}

// Test AST Node edge cases
void test_ast_node_edge_cases() {
    std::cout << "Testing AST Node edge cases..." << std::endl;

    // Test empty identifier
    auto empty_id = std::make_shared<IdentifierExpression>("");
    assert(empty_id->getName().empty());

    // Test very long identifier
    std::string long_name(1000, 'a');
    auto long_id = std::make_shared<IdentifierExpression>(long_name);
    assert(long_id->getName() == long_name);
    assert(long_id->getName().length() == 1000);

    // Test special characters in string literal
    auto special_string = std::make_shared<StringLiteralExpression>("'hello\nworld\t!'");
    assert(special_string->getValue() == "'hello\nworld\t!'");

    // Test zero and large numeric literals
    auto zero_literal = std::make_shared<NumericLiteralExpression>(0.0);
    assert(zero_literal->getValue() == 0.0);

    auto large_literal = std::make_shared<NumericLiteralExpression>(999999.99);
    assert(large_literal->getValue() == 999999.99);

    // Test boolean literals
    auto true_literal = std::make_shared<BooleanLiteralExpression>(true);
    assert(true_literal->getValue() == true);

    auto false_literal = std::make_shared<BooleanLiteralExpression>(false);
    assert(false_literal->getValue() == false);

    std::cout << "AST Node edge cases test passed!" << std::endl;
}

// Test AST Node type safety
void test_ast_node_type_safety() {
    std::cout << "Testing AST Node type safety..." << std::endl;

    // Test that different node types are distinct
    auto identifier = std::make_shared<IdentifierExpression>("test");
    auto string_lit = std::make_shared<StringLiteralExpression>("'test'");
    auto numeric_lit = std::make_shared<NumericLiteralExpression>(42.0);
    auto bool_lit = std::make_shared<BooleanLiteralExpression>(true);

    // Test type names are different
    assert(identifier->getTypeName() != string_lit->getTypeName());
    assert(identifier->getTypeName() != numeric_lit->getTypeName());
    assert(identifier->getTypeName() != bool_lit->getTypeName());
    assert(string_lit->getTypeName() != numeric_lit->getTypeName());
    assert(string_lit->getTypeName() != bool_lit->getTypeName());
    assert(numeric_lit->getTypeName() != bool_lit->getTypeName());

    std::cout << "AST Node type safety test passed!" << std::endl;
}

int main() {
    std::cout << "Running comprehensive AST Node tests..." << std::endl;

    try {
        test_ast_node_basic();
        test_ast_node_visitor();
        test_ast_node_expressions();
        test_ast_node_edge_cases();
        test_ast_node_type_safety();

        std::cout << "All comprehensive AST Node tests passed successfully!" << std::endl;
        std::cout << "AST Node coverage: Medium (basic nodes, visitor pattern, edge cases)" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Test failed with unknown exception!" << std::endl;
        return 1;
    }
}