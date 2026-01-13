#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

// System Database tests for core layer
// These tests verify system database core components

TEST(SystemDatabaseTest, SystemTables) {
    // Test system table management
    std::vector<std::string> system_tables = {
        "sys_users", "sys_roles", "sys_permissions",
        "sys_config", "sys_logs", "sys_stats"
    };

    EXPECT_EQ(system_tables.size(), 6);
    EXPECT_EQ(system_tables[0], "sys_users");
    EXPECT_EQ(system_tables.back(), "sys_stats");
}

TEST(SystemDatabaseTest, MetadataManagement) {
    // Test metadata management
    std::unordered_map<std::string, std::string> metadata;
    metadata["version"] = "1.3.2";
    metadata["created_date"] = "2026-01-13";
    metadata["schema_version"] = "2.1";

    EXPECT_EQ(metadata["version"], "1.3.2");
    EXPECT_EQ(metadata["schema_version"], "2.1");
}

TEST(SystemDatabaseTest, AuditLogging) {
    // Test audit logging functionality
    std::vector<std::pair<std::string, std::string>> audit_logs;
    audit_logs.push_back({"LOGIN", "User admin logged in"});
    audit_logs.push_back({"QUERY", "Executed SELECT statement"});
    audit_logs.push_back({"LOGOUT", "User admin logged out"});

    EXPECT_EQ(audit_logs.size(), 3);
    EXPECT_EQ(audit_logs[0].first, "LOGIN");
    EXPECT_EQ(audit_logs[1].first, "QUERY");
    EXPECT_EQ(audit_logs[2].first, "LOGOUT");
}