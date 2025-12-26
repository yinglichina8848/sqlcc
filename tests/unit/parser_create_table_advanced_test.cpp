#include "sql_parser/ast_node.h"
#include "sql_parser/parser.h"
#include "sql_parser/ast_nodes.h"
#include <iostream>
#include <memory>

using namespace sqlcc::sql_parser;

void testCreateTableBasic() {
    std::cout << "\n=== 测试基本CREATE TABLE语句 ===" << std::endl;
    std::string sql = "CREATE TABLE users (id INT PRIMARY KEY, name VARCHAR(255) NOT NULL, age INT DEFAULT 18);";
    
    std::cout << "解析SQL: " << sql << std::endl;
    
    Parser parser(sql);
    auto statements = parser.parse();
    
    if (!statements.empty()) {
        auto& stmt = statements[0];
        if (stmt->getType() == Statement::CREATE) {
            CreateStatement* createStmt = dynamic_cast<CreateStatement*>(stmt.get());
            if (createStmt) {
                std::cout << "表名: " << createStmt->getObjectName() << std::endl;
                const auto& columns = createStmt->getColumns();
                std::cout << "列数量: " << columns.size() << std::endl;
                for (size_t i = 0; i < columns.size(); ++i) {
                    const auto& col = columns[i];
                    std::cout << "  列 " << i << ": " << col.getName() << " " << col.getType() << std::endl;
                    if (col.isPrimaryKey()) {
                        std::cout << "    主键约束" << std::endl;
                    }
                    if (!col.isNullable()) {
                        std::cout << "    NOT NULL约束" << std::endl;
                    }
                    if (!col.getDefaultValue().empty()) {
                        std::cout << "    默认值: " << col.getDefaultValue() << std::endl;
                    }
                }
            }
        }
    }
}

void testCreateTableWithTableConstraints() {
    std::cout << "\n=== 测试带表级约束的CREATE TABLE语句 ===" << std::endl;
    std::string sql = "CREATE TABLE orders (id INT, user_id INT, product_id INT, PRIMARY KEY (id), UNIQUE (user_id, product_id), FOREIGN KEY (user_id) REFERENCES users (id));";
    
    std::cout << "解析SQL: " << sql << std::endl;
    
    Parser parser(sql);
    auto statements = parser.parse();
    
    if (!statements.empty()) {
        auto& stmt = statements[0];
        if (stmt->getType() == Statement::CREATE) {
            CreateStatement* createStmt = dynamic_cast<CreateStatement*>(stmt.get());
            if (createStmt) {
                std::cout << "表名: " << createStmt->getObjectName() << std::endl;
                
                // 输出列信息
                const auto& columns = createStmt->getColumns();
                std::cout << "列数量: " << columns.size() << std::endl;
                for (size_t i = 0; i < columns.size(); ++i) {
                    const auto& col = columns[i];
                    std::cout << "  列 " << i << ": " << col.getName() << " " << col.getType() << std::endl;
                }
                
                // 输出约束信息
                const auto& constraints = createStmt->getConstraints();
                std::cout << "约束数量: " << constraints.size() << std::endl;
                for (size_t i = 0; i < constraints.size(); ++i) {
                    const auto& constraint = constraints[i];
                    switch (constraint.getType()) {
                        case TableConstraint::PRIMARY_KEY:
                            std::cout << "  主键约束";
                            break;
                        case TableConstraint::UNIQUE:
                            std::cout << "  唯一约束";
                            break;
                        case TableConstraint::FOREIGN_KEY:
                            std::cout << "  外键约束";
                            break;
                        case TableConstraint::CHECK:
                            std::cout << "  检查约束";
                            break;
                    }
                    
                    if (!constraint.getConstraintName().empty()) {
                        std::cout << " (名称: " << constraint.getConstraintName() << ")";
                    }
                    std::cout << std::endl;
                    
                    const auto& cols = constraint.getColumns();
                    std::cout << "    列: ";
                    for (size_t j = 0; j < cols.size(); ++j) {
                        if (j > 0) std::cout << ", ";
                        std::cout << cols[j];
                    }
                    std::cout << std::endl;
                    
                    if (constraint.getType() == TableConstraint::FOREIGN_KEY) {
                        std::cout << "    引用表: " << constraint.getReferencedTable() << std::endl;
                        const auto& refCols = constraint.getReferencedColumns();
                        if (!refCols.empty()) {
                            std::cout << "    引用列: ";
                            for (size_t j = 0; j < refCols.size(); ++j) {
                                if (j > 0) std::cout << ", ";
                                std::cout << refCols[j];
                            }
                            std::cout << std::endl;
                        }
                    }
                }
            }
        }
    }
}

void testCreateTableWithNamedConstraints() {
    std::cout << "\n=== 测试带命名约束的CREATE TABLE语句 ===" << std::endl;
    std::string sql = "CREATE TABLE products (id INT, name VARCHAR(255), price DECIMAL(10,2), CONSTRAINT pk_products PRIMARY KEY (id), CONSTRAINT uk_products_name UNIQUE (name));";
    
    std::cout << "解析SQL: " << sql << std::endl;
    
    Parser parser(sql);
    auto statements = parser.parse();
    
    if (!statements.empty()) {
        auto& stmt = statements[0];
        if (stmt->getType() == Statement::CREATE) {
            CreateStatement* createStmt = dynamic_cast<CreateStatement*>(stmt.get());
            if (createStmt) {
                std::cout << "表名: " << createStmt->getObjectName() << std::endl;
                
                // 输出列信息
                const auto& columns = createStmt->getColumns();
                std::cout << "列数量: " << columns.size() << std::endl;
                for (size_t i = 0; i < columns.size(); ++i) {
                    const auto& col = columns[i];
                    std::cout << "  列 " << i << ": " << col.getName() << " " << col.getType() << std::endl;
                }
                
                // 输出约束信息
                const auto& constraints = createStmt->getConstraints();
                std::cout << "约束数量: " << constraints.size() << std::endl;
                for (size_t i = 0; i < constraints.size(); ++i) {
                    const auto& constraint = constraints[i];
                    switch (constraint.getType()) {
                        case TableConstraint::PRIMARY_KEY:
                            std::cout << "  主键约束";
                            break;
                        case TableConstraint::UNIQUE:
                            std::cout << "  唯一约束";
                            break;
                        case TableConstraint::FOREIGN_KEY:
                            std::cout << "  外键约束";
                            break;
                        case TableConstraint::CHECK:
                            std::cout << "  检查约束";
                            break;
                    }
                    
                    if (!constraint.getConstraintName().empty()) {
                        std::cout << " (名称: " << constraint.getConstraintName() << ")";
                    }
                    std::cout << std::endl;
                    
                    const auto& cols = constraint.getColumns();
                    std::cout << "    列: ";
                    for (size_t j = 0; j < cols.size(); ++j) {
                        if (j > 0) std::cout << ", ";
                        std::cout << cols[j];
                    }
                    std::cout << std::endl;
                }
            }
        }
    }
}

int main() {
    try {
        testCreateTableBasic();
        testCreateTableWithTableConstraints();
        testCreateTableWithNamedConstraints();
        std::cout << "\n所有测试完成！" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "测试过程中出现错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}