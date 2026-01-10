/**
 * @file basic_connection_test.cpp
 * @brief 基本网络连接测试
 *
 * 测试ClientConnection类的基本功能
 */

#include <gtest/gtest.h>
#include <memory>
#include <thread>
#include <chrono>
#include <cstring>
#include <network/client_connection.h>

using namespace sqlcc::network;

// Test fixture for basic connection testing
class BasicConnectionTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a client connection instance
        connection_ = std::make_unique<ClientConnection>("127.0.0.1", 8080);
    }

    void TearDown() override {
        // Cleanup connection
        connection_->Disconnect();
    }

    std::unique_ptr<ClientConnection> connection_;
};

// Test connection creation and initialization
TEST_F(BasicConnectionTest, TestConnectionCreation) {
    EXPECT_NE(connection_, nullptr);
    EXPECT_FALSE(connection_->IsConnected());
}

// Test TLS enable/disable functionality
TEST_F(BasicConnectionTest, TestTLSEnableDisable) {
    // Test enabling TLS
    connection_->EnableTLS(true);
    // Note: We can't easily test the internal state without exposing it,
    // but we can test that the method doesn't throw

    // Test disabling TLS
    connection_->EnableTLS(false);
    // Method should not throw
}

// Test connection state management
TEST_F(BasicConnectionTest, TestConnectionState) {
    // Initially not connected
    EXPECT_FALSE(connection_->IsConnected());

    // After calling Disconnect (should be safe even if not connected)
    connection_->Disconnect();
    EXPECT_FALSE(connection_->IsConnected());
}

// Test data sending without connection (should not crash)
TEST_F(BasicConnectionTest, TestSendWithoutConnection) {
    std::vector<char> test_data = {'H', 'e', 'l', 'l', 'o'};
    // This should fail gracefully without crashing
    bool result = connection_->SendData(test_data);
    EXPECT_FALSE(result);
}

// Test data receiving without connection (should not crash)
TEST_F(BasicConnectionTest, TestReceiveWithoutConnection) {
    // This should return empty data without crashing
    std::vector<char> received = connection_->ReceiveData();
    EXPECT_TRUE(received.empty());
}

// Test multiple connection instances
TEST(BasicConnectionTestSuite, TestMultipleConnections) {
    auto conn1 = std::make_unique<ClientConnection>("127.0.0.1", 8080);
    auto conn2 = std::make_unique<ClientConnection>("127.0.0.1", 8081);

    EXPECT_NE(conn1, nullptr);
    EXPECT_NE(conn2, nullptr);
    EXPECT_FALSE(conn1->IsConnected());
    EXPECT_FALSE(conn2->IsConnected());

    // Cleanup
    conn1->Disconnect();
    conn2->Disconnect();
}

// Test connection with different host/port combinations
TEST(BasicConnectionTestSuite, TestDifferentHostsAndPorts) {
    std::vector<std::pair<std::string, int>> test_cases = {
        {"127.0.0.1", 8080},
        {"localhost", 8081},
        {"192.168.1.1", 3306},
        {"example.com", 80}
    };

    for (const auto& [host, port] : test_cases) {
        auto conn = std::make_unique<ClientConnection>(host, port);
        EXPECT_NE(conn, nullptr);
        EXPECT_FALSE(conn->IsConnected());
        conn->Disconnect();
    }
}

// Test memory management and resource cleanup
TEST(BasicConnectionTestSuite, TestResourceCleanup) {
    {
        auto conn = std::make_unique<ClientConnection>("127.0.0.1", 8080);
        // Connection should be properly destroyed when going out of scope
    }
    // If we get here without issues, the test passes
    EXPECT_TRUE(true);
}

// Test connection behavior with empty data
TEST(BasicConnectionTestSuite, TestEmptyDataOperations) {
    auto conn = std::make_unique<ClientConnection>("127.0.0.1", 8080);

    // Test sending empty data
    std::vector<char> empty_data;
    bool send_result = conn->SendData(empty_data);
    EXPECT_FALSE(send_result); // Should fail since not connected

    // Test receiving when not connected (should return empty)
    std::vector<char> received = conn->ReceiveData();
    EXPECT_TRUE(received.empty());

    conn->Disconnect();
}

// Test TLS configuration handling
TEST(BasicConnectionTestSuite, TestTLSConfiguration) {
    auto conn = std::make_unique<ClientConnection>("127.0.0.1", 8080);

    // Test enabling TLS multiple times
    conn->EnableTLS(true);
    conn->EnableTLS(false);
    conn->EnableTLS(true);

    // Connection should still work normally
    EXPECT_FALSE(conn->IsConnected());

    conn->Disconnect();
}

// Performance test for connection operations
TEST(BasicConnectionTestSuite, TestConnectionPerformance) {
    const int num_iterations = 100;

    auto start_time = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < num_iterations; ++i) {
        auto conn = std::make_unique<ClientConnection>("127.0.0.1", 8080);
        conn->EnableTLS(false);
        conn->Disconnect();
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    // Should complete within reasonable time (less than 1 second for 100 operations)
    EXPECT_LT(duration.count(), 1000);
}