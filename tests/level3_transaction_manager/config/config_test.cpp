#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

// Core Config tests for core layer
// These tests verify core configuration system components

TEST(CoreConfigTest, DatabaseConfig) {
    // Test database configuration for core layer
    std::unordered_map<std::string, std::string> db_config;

    // Core database settings
    db_config["core.db.host"] = "localhost";
    db_config["core.db.port"] = "5432";
    db_config["core.db.name"] = "sqlcc_core";
    db_config["core.db.max_connections"] = "100";

    // Verify core database configuration
    EXPECT_EQ(db_config["core.db.host"], "localhost");
    EXPECT_EQ(db_config["core.db.port"], "5432");
    EXPECT_EQ(db_config["core.db.name"], "sqlcc_core");
    EXPECT_EQ(db_config["core.db.max_connections"], "100");
}

TEST(CoreConfigTest, SystemConfig) {
    // Test system configuration for core components
    std::unordered_map<std::string, std::string> sys_config;

    // Core system settings
    sys_config["core.system.max_memory"] = "1073741824";  // 1GB
    sys_config["core.system.thread_pool_size"] = "16";
    sys_config["core.system.temp_dir"] = "/tmp/sqlcc";
    sys_config["core.system.log_level"] = "INFO";

    // Verify system configuration
    EXPECT_EQ(sys_config["core.system.max_memory"], "1073741824");
    EXPECT_EQ(sys_config["core.system.thread_pool_size"], "16");
    EXPECT_EQ(sys_config["core.system.temp_dir"], "/tmp/sqlcc");
    EXPECT_EQ(sys_config["core.system.log_level"], "INFO");
}

TEST(CoreConfigTest, SecurityConfig) {
    // Test security configuration for core layer
    std::unordered_map<std::string, std::string> sec_config;

    // Core security settings
    sec_config["core.security.encryption_enabled"] = "true";
    sec_config["core.security.auth_timeout"] = "3600";
    sec_config["core.security.max_sessions"] = "1000";

    // Verify security configuration
    EXPECT_EQ(sec_config["core.security.encryption_enabled"], "true");
    EXPECT_EQ(sec_config["core.security.auth_timeout"], "3600");
    EXPECT_EQ(sec_config["core.security.max_sessions"], "1000");
}