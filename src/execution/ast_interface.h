#pragma once

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

#include "sql_parser/ast_node.h"
#include "sql_parser/ast_nodes.h"

namespace sqlcc {
namespace execution_ast {

// Forward declarations for basic types
class Value;
class Row;

// Base AST Node Interface
class ASTNodeInterface {
public:
    virtual ~ASTNodeInterface() = default;

    // Basic node information
    virtual std::string getNodeType() const = 0;
    virtual std::string toString() const = 0;

    // Clone operation for node copying
    virtual std::unique_ptr<ASTNodeInterface> clone() const = 0;
};

// Query Node Interface - represents various query types
class QueryNodeInterface : public ASTNodeInterface {
public:
    ~QueryNodeInterface() override = default;

    virtual std::string getQueryType() const = 0;

    // Common query operations
    virtual bool hasWhereClause() const = 0;
    virtual bool hasGroupBy() const = 0;
    virtual bool hasOrderBy() const = 0;
    virtual bool hasLimit() const = 0;
};

// Select Statement Interface
class SelectNodeInterface : public QueryNodeInterface {
public:
    ~SelectNodeInterface() override = default;

    std::string getNodeType() const override { return "SELECT"; }
    std::string getQueryType() const override { return "SELECT"; }

    // Select-specific operations
    virtual const std::vector<std::string>& getSelectColumns() const = 0;
    virtual const std::string& getFromTable() const = 0;
    virtual std::string getWhereCondition() const = 0;

    virtual bool hasWhereClause() const override = 0;
    virtual bool hasGroupBy() const override = 0;
    virtual bool hasOrderBy() const override = 0;
    virtual bool hasLimit() const override = 0;
};

// Function Definition Interface
class FunctionDefinitionInterface : public ASTNodeInterface {
public:
    ~FunctionDefinitionInterface() override = default;

    std::string getNodeType() const override { return "FUNCTION_DEFINITION"; }

    virtual const std::string& getName() const = 0;
    virtual const std::string& getReturnType() const = 0;
    virtual const std::string& getBody() const = 0;
    virtual const std::vector<std::string>& getParameters() const = 0;

    virtual bool isDeterministic() const = 0;
    virtual bool containsSql() const = 0;
    virtual bool readsSqlData() const = 0;
    virtual bool modifiesSqlData() const = 0;
};

// Factory interface for creating AST nodes from concrete implementations
class ASTFactoryInterface {
public:
    virtual ~ASTFactoryInterface() = default;

    // Create SelectNode from concrete SelectStatement
    virtual std::unique_ptr<SelectNodeInterface> createSelectNode(
        const void* concrete_select_statement) = 0;

    // Create FunctionDefinition from concrete FunctionDefinition
    virtual std::unique_ptr<FunctionDefinitionInterface> createFunctionDefinition(
        const void* concrete_function_definition) = 0;
};

// Registry for AST factories (allows dynamic registration)
class ASTFactoryRegistry {
public:
    static ASTFactoryRegistry& getInstance();

    void registerFactory(const std::string& name,
                        std::unique_ptr<ASTFactoryInterface> factory);
    ASTFactoryInterface* getFactory(const std::string& name) const;

private:
    ASTFactoryRegistry() = default;
    std::unordered_map<std::string, std::unique_ptr<ASTFactoryInterface>> factories_;
};

// Implementation of the registry singleton
inline ASTFactoryRegistry& ASTFactoryRegistry::getInstance() {
    static ASTFactoryRegistry instance;
    return instance;
}

inline void ASTFactoryRegistry::registerFactory(const std::string& name,
                                               std::unique_ptr<ASTFactoryInterface> factory) {
    factories_[name] = std::move(factory);
}

inline ASTFactoryInterface* ASTFactoryRegistry::getFactory(const std::string& name) const {
    auto it = factories_.find(name);
    return it != factories_.end() ? it->second.get() : nullptr;
}

} // namespace execution_ast
} // namespace sqlcc
