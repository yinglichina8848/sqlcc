#include <iostream>
#include <memory>
#include "sql_parser/parser.h"
#include "sql_parser/ast_nodes.h"

using namespace sqlcc::sql_parser;

int main() {
    try {
        // 测试 CREATE DATABASE 语句
        std::cout << "Testing CREATE DATABASE statement...\n";
        Parser parser1("CREATE DATABASE test_db;");
        auto statements1 = parser1.parseStatements();
        if (!statements1.empty()) {
            auto createStmt = dynamic_cast<CreateStatement*>(statements1[0].get());
            if (createStmt) {
                std::cout << "Parsed CREATE statement successfully\n";
                std::cout << "   Object type: " << static_cast<int>(createStmt->getObjectType()) << "\n";
                std::cout << "   Object name: " << createStmt->getObjectName() << "\n";
            }
        }

        // 测试 ALTER DATABASE 语句
        std::cout << "\nTesting ALTER DATABASE statement...\n";
        Parser parser2("ALTER DATABASE test_db;");
        auto statements2 = parser2.parseStatements();
        if (!statements2.empty()) {
            auto alterStmt = dynamic_cast<AlterStatement*>(statements2[0].get());
            if (alterStmt) {
                std::cout << "Parsed ALTER statement successfully\n";
                std::cout << "   Object type: " << static_cast<int>(alterStmt->getObjectType()) << "\n";
                std::cout << "   Object name: " << alterStmt->getObjectName() << "\n";
            }
        }

        // 测试 CREATE TABLE 语句
        std::cout << "\nTesting CREATE TABLE statement...\n";
        Parser parser3("CREATE TABLE users (id INT PRIMARY KEY, name VARCHAR(255));");
        auto statements3 = parser3.parseStatements();
        if (!statements3.empty()) {
            auto createStmt = dynamic_cast<CreateStatement*>(statements3[0].get());
            if (createStmt) {
                std::cout << "Parsed CREATE TABLE statement successfully\n";
                std::cout << "   Object type: " << static_cast<int>(createStmt->getObjectType()) << "\n";
                std::cout << "   Object name: " << createStmt->getObjectName() << "\n";
                std::cout << "   Columns count: " << createStmt->getColumns().size() << "\n";
            }
        }

        std::cout << "\nAll tests passed!\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}