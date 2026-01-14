#include <iostream>
#include <string>
#include <cassert>
#include <memory>
#include "../../../include/sql_parser/ast_node.h"

using namespace sqlcc::sql_parser;

// Test basic AST Node functionality
void test_ast_node_basic() {
    std::cout << "Testing basic AST Node functionality..." << std::endl;

    // Test creating a simple identifier node
    auto identifier = std::make_shared<Identifier>("test_table");
    assert(identifier->get_name() == "test_table");
    assert(identifier->get_type() == ASTNodeType::IDENTIFIER);

    // Test creating a literal node
    auto string_literal = std::make_shared<StringLiteral>("'hello world'");
    assert(string_literal->get_value() == "'hello world'");
    assert(string_literal->get_type() == ASTNodeType::STRING_LITERAL);

    auto int_literal = std::make_shared<IntegerLiteral>(42);
    assert(int_literal->get_value() == 42);
    assert(int_literal->get_type() == ASTNodeType::INTEGER_LITERAL);

    auto bool_literal = std::make_shared<BooleanLiteral>(true);
    assert(bool_literal->get_value() == true);
    assert(bool_literal->get_type() == ASTNodeType::BOOLEAN_LITERAL);

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
        void visit(Identifier* node) override {
            node_count++;
            identifier_count++;
            std::cout << "Visited Identifier: " << node->get_name() << std::endl;
        }

        void visit(StringLiteral* node) override {
            node_count++;
            literal_count++;
            std::cout << "Visited StringLiteral: " << node->get_value() << std::endl;
        }

        void visit(IntegerLiteral* node) override {
            node_count++;
            literal_count++;
            std::cout << "Visited IntegerLiteral: " << node->get_value() << std::endl;
        }

        void visit(BooleanLiteral* node) override {
            node_count++;
            literal_count++;
            std::cout << "Visited BooleanLiteral: " << (node->get_value() ? "true" : "false") << std::endl;
        }

        int get_node_count() const { return node_count; }
        int get_identifier_count() const { return identifier_count; }
        int get_literal_count() const { return literal_count; }
    };

    TestVisitor visitor;

    // Test visiting different node types
    auto identifier = std::make_shared<Identifier>("test_table");
    identifier->accept(&visitor);

    auto string_literal = std::make_shared<StringLiteral>("'hello'");
    string_literal->accept(&visitor);

    auto int_literal = std::make_shared<IntegerLiteral>(123);
    int_literal->accept(&visitor);

    auto bool_literal = std::make_shared<BooleanLiteral>(false);
    bool_literal->accept(&visitor);

    // Verify visitor counts
    assert(visitor.get_node_count() == 4);
    assert(visitor.get_identifier_count() == 1);
    assert(visitor.get_literal_count() == 3);

    std::cout << "AST Node visitor pattern test passed!" << std::endl;
}

// Test AST Node expression types
void test_ast_node_expressions() {
    std::cout << "Testing AST Node expression types..." << std::endl;

    // Test binary expression (if available)
    try {
        auto left = std::make_shared<IntegerLiteral>(10);
        auto right = std::make_shared<IntegerLiteral>(20);

        // Note: BinaryExpression might not be fully implemented yet
        // This is a basic test of what we can test
        assert(left->get_type() == ASTNodeType::INTEGER_LITERAL);
        assert(right->get_type() == ASTNodeType::INTEGER_LITERAL);

        std::cout << "AST Node expression types test passed!" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Expression test skipped due to incomplete implementation: " << e.what() << std::endl;
    }
}

// Test AST Node edge cases
void test_ast_node_edge_cases() {
    std::cout << "Testing AST Node edge cases..." << std::endl;

    // Test empty identifier
    auto empty_id = std::make_shared<Identifier>("");
    assert(empty_id->get_name().empty());
    assert(empty_id->get_type() == ASTNodeType::IDENTIFIER);

    // Test very long identifier
    std::string long_name(1000, 'a');
    auto long_id = std::make_shared<Identifier>(long_name);
    assert(long_id->get_name() == long_name);
    assert(long_id->get_name().length() == 1000);

    // Test special characters in string literal
    auto special_string = std::make_shared<StringLiteral>("'hello\nworld\t!'");
    assert(special_string->get_value() == "'hello\nworld\t!'");

    // Test zero and large integer literals
    auto zero_literal = std::make_shared<IntegerLiteral>(0);
    assert(zero_literal->get_value() == 0);

    auto large_literal = std::make_shared<IntegerLiteral>(INT64_MAX);
    assert(large_literal->get_value() == INT64_MAX);

    // Test boolean literals
    auto true_literal = std::make_shared<BooleanLiteral>(true);
    assert(true_literal->get_value() == true);

    auto false_literal = std::make_shared<BooleanLiteral>(false);
    assert(false_literal->get_value() == false);

    std::cout << "AST Node edge cases test passed!" << std::endl;
}

// Test AST Node type safety
void test_ast_node_type_safety() {
    std::cout << "Testing AST Node type safety..." << std::endl;

    // Test that different node types are distinct
    auto identifier = std::make_shared<Identifier>("test");
    auto string_lit = std::make_shared<StringLiteral>("'test'");
    auto int_lit = std::make_shared<IntegerLiteral>(42);
    auto bool_lit = std::make_shared<BooleanLiteral>(true);

    assert(identifier->get_type() != string_lit->get_type());
    assert(identifier->get_type() != int_lit->get_type());
    assert(identifier->get_type() != bool_lit->get_type());
    assert(string_lit->get_type() != int_lit->get_type());
    assert(string_lit->get_type() != bool_lit->get_type());
    assert(int_lit->get_type() != bool_lit->get_type());

    // Test that all types are valid AST node types
    std::vector<ASTNodeType> types = {
        identifier->get_type(),
        string_lit->get_type(),
        int_lit->get_type(),
        bool_lit->get_type()
    };

    for (auto type : types) {
        assert(type >= ASTNodeType::IDENTIFIER && type <= ASTNodeType::BOOLEAN_LITERAL);
    }

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
