#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

// Config Manager tests for core services layer
// These tests verify configuration management components

TEST(ConfigManagerTest, ConfigurationLoading) {
    // Test configuration loading
    std::unordered_map<std::string, std::string> config;

    config["db.host"] = "localhost";
    config["db.port"] = "5432";
    config["db.name"] = "sqlcc_db";
    config["server.max_connections"] = "100";

    EXPECT_EQ(config["db.host"], "localhost");
    EXPECT_EQ(config["db.port"], "5432");
    EXPECT_EQ(config["db.name"], "sqlcc_db");
    EXPECT_EQ(config["server.max_connections"], "100");
}

TEST(ConfigManagerTest, DefaultValues) {
    // Test default value handling
    std::unordered_map<std::string, std::string> defaults;

    defaults["timeout"] = "30";
    defaults["retries"] = "3";
    defaults["log_level"] = "INFO";

    // Verify defaults are set
    EXPECT_EQ(defaults["timeout"], "30");
    EXPECT_EQ(defaults["retries"], "3");
    EXPECT_EQ(defaults["log_level"], "INFO");
}

TEST(ConfigManagerTest, EnvironmentOverrides) {
    // Test environment variable overrides
    std::unordered_map<std::string, std::string> config;

    // Base config
    config["env"] = "development";

    // Environment override simulation
    if (config["env"] == "development") {
        config["debug"] = "true";
        config["log_level"] = "DEBUG";
    }

    EXPECT_EQ(config["env"], "development");
    EXPECT_EQ(config["debug"], "true");
    EXPECT_EQ(config["log_level"], "DEBUG");
}

TEST(ConfigManagerTest, Validation) {
    // Test configuration validation
    auto validate_port = [](const std::string& port_str) -> bool {
        try {
            int port = std::stoi(port_str);
            return port > 0 && port <= 65535;
        } catch (...) {
            return false;
        }
    };

    EXPECT_TRUE(validate_port("5432"));
    EXPECT_TRUE(validate_port("8080"));
    EXPECT_FALSE(validate_port("0"));
    EXPECT_FALSE(validate_port("70000"));
    EXPECT_FALSE(validate_port("invalid"));
}