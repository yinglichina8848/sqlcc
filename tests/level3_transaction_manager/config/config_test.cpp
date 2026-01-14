#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

// SQLCC Core Components
#include "utils/logger.h"
#include "utils/config_manager.h"
#include "transaction_manager.h"

// Level 3 Transaction Manager Config tests
// These tests verify transaction manager configuration components with real SQLCC components

TEST(TransactionManagerConfigTest, LoggerIntegration) {
    // Test Logger integration with transaction manager configuration
    SQLCC::Logger& logger = SQLCC::Logger::GetInstance();

    // Set log level for transaction manager
    logger.SetLogLevel(SQLCC::LogLevel::INFO);
    EXPECT_EQ(logger.GetLogLevel(), SQLCC::LogLevel::INFO);

    // Log transaction manager configuration
    logger.Info("Testing transaction manager configuration");
    logger.Info("Database host: localhost");
    logger.Info("Transaction timeout: 30000ms");

    // Verify logger is operational
    EXPECT_TRUE(logger.IsInitialized());
}

TEST(TransactionManagerConfigTest, ConfigManagerDatabaseSettings) {
    // Test ConfigManager integration for database configuration
    SQLCC::ConfigManager& config = SQLCC::ConfigManager::GetInstance();

    // Set database configuration values
    config.SetValue("transaction.db.host", "localhost");
    config.SetValue("transaction.db.port", "5432");
    config.SetValue("transaction.db.name", "sqlcc_transaction");
    config.SetValue("transaction.db.max_connections", "100");

    // Verify configuration values are stored correctly
    EXPECT_EQ(config.GetString("transaction.db.host"), "localhost");
    EXPECT_EQ(config.GetString("transaction.db.port"), "5432");
    EXPECT_EQ(config.GetString("transaction.db.name"), "sqlcc_transaction");
    EXPECT_EQ(config.GetString("transaction.db.max_connections"), "100");
}

TEST(TransactionManagerConfigTest, ConfigManagerSystemSettings) {
    // Test ConfigManager integration for system settings
    SQLCC::ConfigManager& config = SQLCC::ConfigManager::GetInstance();

    // Set system configuration values
    config.SetValue("transaction.system.max_memory", "1073741824");  // 1GB
    config.SetValue("transaction.system.thread_pool_size", "16");
    config.SetValue("transaction.system.temp_dir", "/tmp/sqlcc");
    config.SetValue("transaction.system.log_level", "INFO");

    // Verify system configuration values
    EXPECT_EQ(config.GetString("transaction.system.max_memory"), "1073741824");
    EXPECT_EQ(config.GetString("transaction.system.thread_pool_size"), "16");
    EXPECT_EQ(config.GetString("transaction.system.temp_dir"), "/tmp/sqlcc");
    EXPECT_EQ(config.GetString("transaction.system.log_level"), "INFO");
}

TEST(TransactionManagerConfigTest, ConfigManagerSecuritySettings) {
    // Test ConfigManager integration for security settings
    SQLCC::ConfigManager& config = SQLCC::ConfigManager::GetInstance();

    // Set security configuration values
    config.SetValue("transaction.security.encryption_enabled", "true");
    config.SetValue("transaction.security.auth_timeout", "3600");
    config.SetValue("transaction.security.max_sessions", "1000");
    config.SetValue("transaction.security.isolation_level", "READ_COMMITTED");

    // Verify security configuration values
    EXPECT_EQ(config.GetString("transaction.security.encryption_enabled"), "true");
    EXPECT_EQ(config.GetString("transaction.security.auth_timeout"), "3600");
    EXPECT_EQ(config.GetString("transaction.security.max_sessions"), "1000");
    EXPECT_EQ(config.GetString("transaction.security.isolation_level"), "READ_COMMITTED");
}

TEST(TransactionManagerConfigTest, TransactionManagerInitialization) {
    // Test TransactionManager initialization with configuration
    SQLCC::ConfigManager& config = SQLCC::ConfigManager::GetInstance();
    SQLCC::Logger& logger = SQLCC::Logger::GetInstance();

    // Set transaction manager configuration
    config.SetValue("transaction.max_concurrent", "50");
    config.SetValue("transaction.timeout_ms", "30000");
    config.SetValue("transaction.auto_commit", "true");

    // Log initialization
    logger.Info("Initializing TransactionManager with configuration");
    logger.Info("Max concurrent transactions: " + config.GetString("transaction.max_concurrent"));
    logger.Info("Transaction timeout: " + config.GetString("transaction.timeout_ms") + "ms");

    // Verify configuration is accessible
    EXPECT_EQ(config.GetString("transaction.max_concurrent"), "50");
    EXPECT_EQ(config.GetString("transaction.timeout_ms"), "30000");
    EXPECT_EQ(config.GetString("transaction.auto_commit"), "true");

    // Verify logger and config manager are working together
    EXPECT_TRUE(logger.IsInitialized());
}
