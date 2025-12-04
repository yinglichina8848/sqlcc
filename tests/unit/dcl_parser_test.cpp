#include "sql_parser/parser_new.h"
#include "sql_parser/ast_nodes.h"
#include <gtest/gtest.h>
#include <memory>
#include <string>

using namespace sqlcc::sql_parser;

class DCLParserTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(DCLParserTest, ParseCreateUserWithIdentifiedBy) {
    std::string sql = "CREATE USER testuser IDENTIFIED BY 'password123';";
    ParserNew parser(sql);
    
    auto statements = parser.parse();
    ASSERT_EQ(statements.size(), 1);
    
    auto stmt = dynamic_cast<CreateUserStatement*>(statements[0].get());
    ASSERT_NE(stmt, nullptr);
    EXPECT_EQ(stmt->getType(), Statement::CREATE_USER);
    EXPECT_EQ(stmt->getUsername(), "testuser");
    EXPECT_EQ(stmt->getPassword(), "password123");
    EXPECT_FALSE(stmt->isWithPassword());
}

TEST_F(DCLParserTest, ParseCreateUserWithPassword) {
    std::string sql = "CREATE USER testuser WITH PASSWORD 'password123';";
    ParserNew parser(sql);
    
    auto statements = parser.parse();
    ASSERT_EQ(statements.size(), 1);
    
    auto stmt = dynamic_cast<CreateUserStatement*>(statements[0].get());
    ASSERT_NE(stmt, nullptr);
    EXPECT_EQ(stmt->getType(), Statement::CREATE_USER);
    EXPECT_EQ(stmt->getUsername(), "testuser");
    EXPECT_EQ(stmt->getPassword(), "password123");
    EXPECT_TRUE(stmt->isWithPassword());
}

TEST_F(DCLParserTest, ParseDropUser) {
    std::string sql = "DROP USER testuser;";
    ParserNew parser(sql);
    
    auto statements = parser.parse();
    ASSERT_EQ(statements.size(), 1);
    
    auto stmt = dynamic_cast<DropUserStatement*>(statements[0].get());
    ASSERT_NE(stmt, nullptr);
    EXPECT_EQ(stmt->getType(), Statement::DROP_USER);
    EXPECT_EQ(stmt->getUsername(), "testuser");
    EXPECT_FALSE(stmt->isIfExists());
}

TEST_F(DCLParserTest, ParseDropUserIfExists) {
    std::string sql = "DROP USER IF EXISTS testuser;";
    ParserNew parser(sql);
    
    auto statements = parser.parse();
    ASSERT_EQ(statements.size(), 1);
    
    auto stmt = dynamic_cast<DropUserStatement*>(statements[0].get());
    ASSERT_NE(stmt, nullptr);
    EXPECT_EQ(stmt->getType(), Statement::DROP_USER);
    EXPECT_EQ(stmt->getUsername(), "testuser");
    EXPECT_TRUE(stmt->isIfExists());
}

TEST_F(DCLParserTest, ParseGrantAllPrivileges) {
    std::string sql = "GRANT ALL PRIVILEGES ON TABLE users TO testuser;";
    ParserNew parser(sql);
    
    auto statements = parser.parse();
    ASSERT_EQ(statements.size(), 1);
    
    auto stmt = dynamic_cast<GrantStatement*>(statements[0].get());
    ASSERT_NE(stmt, nullptr);
    EXPECT_EQ(stmt->getType(), Statement::GRANT);
    
    auto privileges = stmt->getPrivileges();
    ASSERT_EQ(privileges.size(), 1);
    EXPECT_EQ(privileges[0], "ALL PRIVILEGES");
    
    EXPECT_EQ(stmt->getObjectType(), "TABLE");
    EXPECT_EQ(stmt->getObjectName(), "users");
    EXPECT_EQ(stmt->getGrantee(), "testuser");
}

TEST_F(DCLParserTest, ParseGrantMultiplePrivileges) {
    std::string sql = "GRANT SELECT, INSERT, UPDATE ON TABLE users TO testuser;";
    ParserNew parser(sql);
    
    auto statements = parser.parse();
    ASSERT_EQ(statements.size(), 1);
    
    auto stmt = dynamic_cast<GrantStatement*>(statements[0].get());
    ASSERT_NE(stmt, nullptr);
    EXPECT_EQ(stmt->getType(), Statement::GRANT);
    
    auto privileges = stmt->getPrivileges();
    ASSERT_EQ(privileges.size(), 3);
    EXPECT_EQ(privileges[0], "SELECT");
    EXPECT_EQ(privileges[1], "INSERT");
    EXPECT_EQ(privileges[2], "UPDATE");
    
    EXPECT_EQ(stmt->getObjectType(), "TABLE");
    EXPECT_EQ(stmt->getObjectName(), "users");
    EXPECT_EQ(stmt->getGrantee(), "testuser");
}

TEST_F(DCLParserTest, ParseRevokeSinglePrivilege) {
    std::string sql = "REVOKE SELECT ON TABLE users FROM testuser;";
    ParserNew parser(sql);
    
    auto statements = parser.parse();
    ASSERT_EQ(statements.size(), 1);
    
    auto stmt = dynamic_cast<RevokeStatement*>(statements[0].get());
    ASSERT_NE(stmt, nullptr);
    EXPECT_EQ(stmt->getType(), Statement::REVOKE);
    ASSERT_EQ(stmt->getPrivileges().size(), 1);
    EXPECT_EQ(stmt->getPrivileges()[0], "SELECT");
    EXPECT_EQ(stmt->getObjectType(), "TABLE");
    EXPECT_EQ(stmt->getObjectName(), "users");
    EXPECT_EQ(stmt->getGrantee(), "testuser");
}

TEST_F(DCLParserTest, ParseRevokeMultiplePrivileges) {
    std::string sql = "REVOKE SELECT, INSERT, UPDATE ON TABLE users FROM testuser;";
    ParserNew parser(sql);
    
    auto statements = parser.parse();
    ASSERT_EQ(statements.size(), 1);
    
    auto stmt = dynamic_cast<RevokeStatement*>(statements[0].get());
    ASSERT_NE(stmt, nullptr);
    EXPECT_EQ(stmt->getType(), Statement::REVOKE);
    ASSERT_EQ(stmt->getPrivileges().size(), 3);
    EXPECT_EQ(stmt->getPrivileges()[0], "SELECT");
    EXPECT_EQ(stmt->getPrivileges()[1], "INSERT");
    EXPECT_EQ(stmt->getPrivileges()[2], "UPDATE");
    EXPECT_EQ(stmt->getObjectType(), "TABLE");
    EXPECT_EQ(stmt->getObjectName(), "users");
    EXPECT_EQ(stmt->getGrantee(), "testuser");
}

TEST_F(DCLParserTest, ParseRevokeAllPrivileges) {
    std::string sql = "REVOKE ALL PRIVILEGES ON TABLE users FROM testuser;";
    ParserNew parser(sql);
    
    auto statements = parser.parse();
    ASSERT_EQ(statements.size(), 1);
    
    auto stmt = dynamic_cast<RevokeStatement*>(statements[0].get());
    ASSERT_NE(stmt, nullptr);
    EXPECT_EQ(stmt->getType(), Statement::REVOKE);
    ASSERT_EQ(stmt->getPrivileges().size(), 1);
    EXPECT_EQ(stmt->getPrivileges()[0], "ALL");
    EXPECT_EQ(stmt->getObjectType(), "TABLE");
    EXPECT_EQ(stmt->getObjectName(), "users");
    EXPECT_EQ(stmt->getGrantee(), "testuser");
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}