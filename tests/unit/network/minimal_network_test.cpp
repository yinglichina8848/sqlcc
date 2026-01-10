/**
 * @file minimal_network_test.cpp
 * @brief 最小化网络测试 - 只测试不依赖复杂组件的类
 *
 * 测试SessionManager和ConnectionStateMachine等核心类
 */

#include <gtest/gtest.h>
#include <memory>
#include <network/network.h>

using namespace sqlcc::network;

// Test session management functionality
TEST(SessionManagerTest, TestSessionCreation) {
    SessionManager session_manager;

    // Test session creation
    auto session1 = session_manager.CreateSession();
    EXPECT_NE(session1, nullptr);
    EXPECT_EQ(session1->GetSessionId(), 1);

    // Test session retrieval
    auto retrieved_session = session_manager.GetSession(1);
    EXPECT_NE(retrieved_session, nullptr);
    EXPECT_EQ(retrieved_session->GetSessionId(), 1);

    // Test session destruction
    session_manager.DestroySession(1);
    auto null_session = session_manager.GetSession(1);
    EXPECT_EQ(null_session, nullptr);
}

TEST(SessionManagerTest, TestMultipleSessions) {
    SessionManager session_manager;

    // Create multiple sessions
    auto session1 = session_manager.CreateSession();
    auto session2 = session_manager.CreateSession();
    auto session3 = session_manager.CreateSession();

    EXPECT_EQ(session1->GetSessionId(), 1);
    EXPECT_EQ(session2->GetSessionId(), 2);
    EXPECT_EQ(session3->GetSessionId(), 3);

    // Test retrieval of all sessions
    auto retrieved1 = session_manager.GetSession(1);
    auto retrieved2 = session_manager.GetSession(2);
    auto retrieved3 = session_manager.GetSession(3);

    EXPECT_NE(retrieved1, nullptr);
    EXPECT_NE(retrieved2, nullptr);
    EXPECT_NE(retrieved3, nullptr);
}

TEST(SessionManagerTest, TestAuthentication) {
    SessionManager session_manager;

    auto session = session_manager.CreateSession();

    // Test authentication
    bool auth_result = session_manager.Authenticate(session->GetSessionId(), "admin", "password");
    EXPECT_TRUE(auth_result);

    // Verify session is authenticated
    auto retrieved_session = session_manager.GetSession(session->GetSessionId());
    EXPECT_TRUE(retrieved_session->IsAuthenticated());
    EXPECT_EQ(retrieved_session->GetUser(), "admin");

    // Test failed authentication
    bool bad_auth = session_manager.Authenticate(session->GetSessionId(), "wrong", "wrong");
    EXPECT_FALSE(bad_auth);
}

TEST(SessionManagerTest, TestPermissionCheck) {
    SessionManager session_manager;

    auto session = session_manager.CreateSession();

    // Test permission check without authentication (should fail)
    bool permission_result = session_manager.CheckPermission(session->GetSessionId(), "test_db", "SELECT");
    EXPECT_FALSE(permission_result);

    // Authenticate and test permission check
    session_manager.Authenticate(session->GetSessionId(), "admin", "password");
    permission_result = session_manager.CheckPermission(session->GetSessionId(), "test_db", "SELECT");
    EXPECT_TRUE(permission_result);
}

// Test connection state machine
TEST(ConnectionStateMachineTest, TestInitialState) {
    ConnectionStateMachine state_machine;

    EXPECT_TRUE(state_machine.IsDisconnected());
    EXPECT_FALSE(state_machine.IsConnected());
    EXPECT_FALSE(state_machine.IsAuthenticated());
    EXPECT_FALSE(state_machine.IsEncrypted());
    EXPECT_FALSE(state_machine.IsClosed());
    EXPECT_FALSE(state_machine.IsError());
}

TEST(ConnectionStateMachineTest, TestStateTransitions) {
    ConnectionStateMachine state_machine;

    // Test DISCONNECTED -> CONNECTING
    EXPECT_TRUE(state_machine.TransitionTo(ConnectionState::CONNECTING));
    EXPECT_TRUE(state_machine.IsConnecting());
    EXPECT_FALSE(state_machine.IsDisconnected());

    // Test CONNECTING -> CONNECTED
    EXPECT_TRUE(state_machine.TransitionTo(ConnectionState::CONNECTED));
    EXPECT_TRUE(state_machine.IsConnected());
    EXPECT_FALSE(state_machine.IsConnecting());

    // Test CONNECTED -> AUTHENTICATING
    EXPECT_TRUE(state_machine.TransitionTo(ConnectionState::AUTHENTICATING));
    EXPECT_TRUE(state_machine.IsInState(ConnectionState::AUTHENTICATING));

    // Test AUTHENTICATING -> AUTHENTICATED
    EXPECT_TRUE(state_machine.TransitionTo(ConnectionState::AUTHENTICATED));
    EXPECT_TRUE(state_machine.IsAuthenticated());
    EXPECT_TRUE(state_machine.IsConnected());

    // Test AUTHENTICATED -> KEY_EXCHANGING
    EXPECT_TRUE(state_machine.TransitionTo(ConnectionState::KEY_EXCHANGING));
    EXPECT_TRUE(state_machine.IsInState(ConnectionState::KEY_EXCHANGING));

    // Test KEY_EXCHANGING -> ENCRYPTED
    EXPECT_TRUE(state_machine.TransitionTo(ConnectionState::ENCRYPTED));
    EXPECT_TRUE(state_machine.IsEncrypted());
    EXPECT_TRUE(state_machine.IsAuthenticated());
    EXPECT_TRUE(state_machine.IsConnected());
}

TEST(ConnectionStateMachineTest, TestInvalidTransitions) {
    ConnectionStateMachine state_machine;

    // Test invalid transition from DISCONNECTED to AUTHENTICATED
    EXPECT_FALSE(state_machine.TransitionTo(ConnectionState::AUTHENTICATED));
    EXPECT_TRUE(state_machine.IsDisconnected()); // Should still be in initial state

    // Go to CONNECTED state first
    EXPECT_TRUE(state_machine.TransitionTo(ConnectionState::CONNECTING));
    EXPECT_TRUE(state_machine.TransitionTo(ConnectionState::CONNECTED));

    // Test invalid transition from CONNECTED to ENCRYPTED (skipping AUTHENTICATED)
    EXPECT_FALSE(state_machine.TransitionTo(ConnectionState::ENCRYPTED));
    EXPECT_TRUE(state_machine.IsConnected());
    EXPECT_FALSE(state_machine.IsEncrypted());
}

TEST(ConnectionStateMachineTest, TestStateOperations) {
    ConnectionStateMachine state_machine;

    // Test CanConnect from initial state
    EXPECT_TRUE(state_machine.CanConnect());
    EXPECT_FALSE(state_machine.CanAuthenticate());
    EXPECT_FALSE(state_machine.CanSendQuery());
    EXPECT_FALSE(state_machine.CanEncrypt());
    EXPECT_TRUE(state_machine.CanClose());

    // Transition to CONNECTED
    state_machine.TransitionTo(ConnectionState::CONNECTING);
    state_machine.TransitionTo(ConnectionState::CONNECTED);

    EXPECT_FALSE(state_machine.CanConnect());
    EXPECT_TRUE(state_machine.CanAuthenticate());
    EXPECT_FALSE(state_machine.CanSendQuery());
    EXPECT_FALSE(state_machine.CanEncrypt());
    EXPECT_TRUE(state_machine.CanClose());

    // Transition to AUTHENTICATED
    state_machine.TransitionTo(ConnectionState::AUTHENTICATING);
    state_machine.TransitionTo(ConnectionState::AUTHENTICATED);

    EXPECT_FALSE(state_machine.CanConnect());
    EXPECT_FALSE(state_machine.CanAuthenticate());
    EXPECT_TRUE(state_machine.CanSendQuery());
    EXPECT_TRUE(state_machine.CanEncrypt());
    EXPECT_TRUE(state_machine.CanClose());

    // Transition to ENCRYPTED
    state_machine.TransitionTo(ConnectionState::KEY_EXCHANGING);
    state_machine.TransitionTo(ConnectionState::ENCRYPTED);

    EXPECT_FALSE(state_machine.CanConnect());
    EXPECT_FALSE(state_machine.CanAuthenticate());
    EXPECT_TRUE(state_machine.CanSendQuery());
    EXPECT_FALSE(state_machine.CanEncrypt()); // Already encrypted
    EXPECT_TRUE(state_machine.CanClose());
}

TEST(ConnectionStateMachineTest, TestResetAndError) {
    ConnectionStateMachine state_machine;

    // Transition to some state
    state_machine.TransitionTo(ConnectionState::CONNECTING);
    state_machine.TransitionTo(ConnectionState::CONNECTED);

    EXPECT_TRUE(state_machine.IsConnected());

    // Test reset
    state_machine.Reset();
    EXPECT_TRUE(state_machine.IsDisconnected());

    // Transition again and test error
    state_machine.TransitionTo(ConnectionState::CONNECTING);
    state_machine.ForceError();

    EXPECT_TRUE(state_machine.IsError());
    EXPECT_FALSE(state_machine.IsConnected());
}

TEST(ConnectionStateMachineTest, TestStateNames) {
    ConnectionStateMachine state_machine;

    EXPECT_EQ(state_machine.GetStateName(ConnectionState::DISCONNECTED), "DISCONNECTED");
    EXPECT_EQ(state_machine.GetStateName(ConnectionState::CONNECTING), "CONNECTING");
    EXPECT_EQ(state_machine.GetStateName(ConnectionState::CONNECTED), "CONNECTED");
    EXPECT_EQ(state_machine.GetStateName(ConnectionState::AUTHENTICATING), "AUTHENTICATING");
    EXPECT_EQ(state_machine.GetStateName(ConnectionState::AUTHENTICATED), "AUTHENTICATED");
    EXPECT_EQ(state_machine.GetStateName(ConnectionState::KEY_EXCHANGING), "KEY_EXCHANGING");
    EXPECT_EQ(state_machine.GetStateName(ConnectionState::ENCRYPTED), "ENCRYPTED");
    EXPECT_EQ(state_machine.GetStateName(ConnectionState::CLOSING), "CLOSING");
    EXPECT_EQ(state_machine.GetStateName(ConnectionState::CLOSED), "CLOSED");
    EXPECT_EQ(state_machine.GetStateName(ConnectionState::CONNECTION_ERROR), "CONNECTION_ERROR");
}

// Test data transmission validator
TEST(DataTransmissionValidatorTest, TestMessageHeaderValidation) {
    DataTransmissionValidator validator;

    MessageHeader valid_header = {
        .magic = 0x53434C53, // 'SQLC'
        .length = 100,
        .type = MessageType::QUERY,
        .flags = 0,
        .sequence_id = 1
    };

    EXPECT_TRUE(validator.ValidateMessageHeader(valid_header));
    EXPECT_TRUE(validator.ValidateMessageMagic(valid_header.magic));
    EXPECT_TRUE(validator.ValidateMessageType(valid_header.type));

    // Test invalid magic
    MessageHeader invalid_magic = valid_header;
    invalid_magic.magic = 0x12345678;
    EXPECT_FALSE(validator.ValidateMessageMagic(invalid_magic.magic));

    // Test invalid type
    MessageHeader invalid_type = valid_header;
    invalid_type.type = static_cast<uint16_t>(-1); // Invalid type
    EXPECT_FALSE(validator.ValidateMessageType(invalid_type.type));
}

TEST(DataTransmissionValidatorTest, TestBufferSizeValidation) {
    DataTransmissionValidator validator;

    // Test valid buffer sizes
    EXPECT_TRUE(validator.IsBufferSizeValid(1024));
    EXPECT_TRUE(validator.IsBufferSizeValid(1024 * 1024)); // 1MB

    // Test invalid buffer sizes (assuming max is 128MB)
    EXPECT_FALSE(validator.IsBufferSizeValid(200 * 1024 * 1024)); // 200MB

    // Test message size limits
    EXPECT_TRUE(validator.IsMessageSizeWithinLimits(1024));
    EXPECT_TRUE(validator.IsMessageSizeWithinLimits(64 * 1024 * 1024)); // 64MB
    EXPECT_FALSE(validator.IsMessageSizeWithinLimits(200 * 1024 * 1024)); // 200MB
}

// Test network exception
TEST(NetworkExceptionTest, TestExceptionCreation) {
    NetworkException exception(NetworkExceptionType::CONNECTION_LOST,
                              "Connection lost", "Connection timeout");

    EXPECT_EQ(exception.GetType(), NetworkExceptionType::CONNECTION_LOST);
    EXPECT_EQ(exception.GetDetails(), "Connection timeout");
    EXPECT_TRUE(exception.IsRecoverable());

    std::string full_message = exception.GetFullMessage();
    EXPECT_NE(full_message.find("Connection lost"), std::string::npos);
    EXPECT_NE(full_message.find("Connection timeout"), std::string::npos);
}

// Test network monitor basic functionality
TEST(NetworkMonitorTest, TestMonitorInitialization) {
    NetworkMonitor monitor;

    // Test initial state
    EXPECT_EQ(monitor.GetActiveConnections(), 0);
    EXPECT_EQ(monitor.GetTotalMessagesSent(), static_cast<size_t>(0));
    EXPECT_EQ(monitor.GetTotalMessagesReceived(), static_cast<size_t>(0));

    // Test logging
    monitor.LogEvent(NetworkMonitor::MonitorLevel::INFO, "TestComponent", "Test event");
    monitor.LogPerformance("TestMetric", 100.0, "ms");

    // Test connection tracking
    monitor.RecordConnectionEstablished();
    EXPECT_EQ(monitor.GetActiveConnections(), 1);

    monitor.RecordConnectionLost();
    EXPECT_EQ(monitor.GetActiveConnections(), 0);
}

// Test network stability guard basic functionality
TEST(NetworkStabilityGuardTest, TestStabilityGuardInitialization) {
    NetworkStabilityGuard guard;

    // Test initial state
    EXPECT_FALSE(guard.ShouldAcceptNewConnection());
    EXPECT_FALSE(guard.ShouldThrottleRequests());

    int load_level = guard.GetCurrentLoadLevel();
    EXPECT_GE(load_level, 0);
    EXPECT_LE(load_level, 100);
}

// Performance test
TEST(PerformanceTest, TestSessionManagerPerformance) {
    SessionManager session_manager;

    const int num_sessions = 1000;

    // Test session creation performance
    auto start_time = std::chrono::high_resolution_clock::now();

    std::vector<std::shared_ptr<Session>> sessions;
    for (int i = 0; i < num_sessions; ++i) {
        sessions.push_back(session_manager.CreateSession());
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    // Should create 1000 sessions in reasonable time (less than 100ms)
    EXPECT_LT(duration.count(), 100);

    // Verify all sessions were created
    for (int i = 0; i < num_sessions; ++i) {
        EXPECT_EQ(sessions[i]->GetSessionId(), i + 1);
    }
}