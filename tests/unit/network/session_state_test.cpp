/**
 * @file session_state_test.cpp
 * @brief 会话管理和状态机测试
 *
 * 测试SessionManager和ConnectionStateMachine的核心功能
 */

#include <gtest/gtest.h>
#include <memory>
#include <network/network.h>

using namespace sqlcc::network;

// Test SessionManager functionality
TEST(SessionManagerTest, CreateAndRetrieveSession) {
    SessionManager session_manager;

    // Create a session
    auto session = session_manager.CreateSession();
    ASSERT_NE(session, nullptr);
    EXPECT_EQ(session->GetSessionId(), 1);

    // Retrieve the session
    auto retrieved = session_manager.GetSession(1);
    ASSERT_NE(retrieved, nullptr);
    EXPECT_EQ(retrieved->GetSessionId(), 1);
    EXPECT_EQ(retrieved, session); // Should be the same object
}

TEST(SessionManagerTest, SessionAuthentication) {
    SessionManager session_manager;

    auto session = session_manager.CreateSession();

    // Test authentication with correct credentials
    bool auth_result = session_manager.Authenticate(session->GetSessionId(), "admin", "password");
    EXPECT_TRUE(auth_result);

    // Verify session state
    auto retrieved = session_manager.GetSession(session->GetSessionId());
    ASSERT_NE(retrieved, nullptr);
    EXPECT_TRUE(retrieved->IsAuthenticated());
    EXPECT_EQ(retrieved->GetUser(), "admin");
}

TEST(SessionManagerTest, SessionDestruction) {
    SessionManager session_manager;

    auto session = session_manager.CreateSession();
    EXPECT_EQ(session->GetSessionId(), 1);

    // Destroy the session
    session_manager.DestroySession(1);

    // Try to retrieve - should return nullptr
    auto retrieved = session_manager.GetSession(1);
    EXPECT_EQ(retrieved, nullptr);
}

TEST(SessionManagerTest, MultipleSessions) {
    SessionManager session_manager;

    // Create multiple sessions
    auto session1 = session_manager.CreateSession();
    auto session2 = session_manager.CreateSession();
    auto session3 = session_manager.CreateSession();

    EXPECT_EQ(session1->GetSessionId(), 1);
    EXPECT_EQ(session2->GetSessionId(), 2);
    EXPECT_EQ(session3->GetSessionId(), 3);

    // All should be retrievable
    EXPECT_NE(session_manager.GetSession(1), nullptr);
    EXPECT_NE(session_manager.GetSession(2), nullptr);
    EXPECT_NE(session_manager.GetSession(3), nullptr);
}

TEST(SessionManagerTest, InvalidSessionId) {
    SessionManager session_manager;

    // Try to get non-existent session
    auto retrieved = session_manager.GetSession(999);
    EXPECT_EQ(retrieved, nullptr);

    // Try to authenticate non-existent session
    bool auth_result = session_manager.Authenticate(999, "admin", "password");
    EXPECT_FALSE(auth_result);
}

TEST(SessionManagerTest, SessionPermissions) {
    SessionManager session_manager;

    auto session = session_manager.CreateSession();

    // Test permission check without authentication
    bool permission = session_manager.CheckPermission(session->GetSessionId(), "test_db", "SELECT");
    EXPECT_FALSE(permission);

    // Authenticate and test permission
    session_manager.Authenticate(session->GetSessionId(), "admin", "password");
    permission = session_manager.CheckPermission(session->GetSessionId(), "test_db", "SELECT");
    EXPECT_TRUE(permission); // Current implementation always returns true for authenticated users
}

// Test ConnectionStateMachine functionality
TEST(ConnectionStateMachineTest, InitialState) {
    ConnectionStateMachine state_machine;

    EXPECT_TRUE(state_machine.IsDisconnected());
    EXPECT_FALSE(state_machine.IsConnected());
    EXPECT_FALSE(state_machine.IsAuthenticated());
    EXPECT_FALSE(state_machine.IsEncrypted());
    EXPECT_EQ(state_machine.GetStateName(ConnectionState::DISCONNECTED), "DISCONNECTED");
}

TEST(ConnectionStateMachineTest, BasicStateTransitions) {
    ConnectionStateMachine state_machine;

    // DISCONNECTED -> CONNECTING
    EXPECT_TRUE(state_machine.TransitionTo(ConnectionState::CONNECTING));
    EXPECT_TRUE(state_machine.IsConnecting());
    EXPECT_FALSE(state_machine.IsDisconnected());

    // CONNECTING -> CONNECTED
    EXPECT_TRUE(state_machine.TransitionTo(ConnectionState::CONNECTED));
    EXPECT_TRUE(state_machine.IsConnected());
    EXPECT_FALSE(state_machine.IsConnecting());
}

TEST(ConnectionStateMachineTest, AuthenticationFlow) {
    ConnectionStateMachine state_machine;

    // Set up connected state
    state_machine.TransitionTo(ConnectionState::CONNECTING);
    state_machine.TransitionTo(ConnectionState::CONNECTED);

    // CONNECTED -> AUTHENTICATING
    EXPECT_TRUE(state_machine.TransitionTo(ConnectionState::AUTHENTICATING));
    EXPECT_TRUE(state_machine.IsInState(ConnectionState::AUTHENTICATING));

    // AUTHENTICATING -> AUTHENTICATED
    EXPECT_TRUE(state_machine.TransitionTo(ConnectionState::AUTHENTICATED));
    EXPECT_TRUE(state_machine.IsAuthenticated());
    EXPECT_TRUE(state_machine.IsConnected());
}

TEST(ConnectionStateMachineTest, EncryptionFlow) {
    ConnectionStateMachine state_machine;

    // Set up authenticated state
    state_machine.TransitionTo(ConnectionState::CONNECTING);
    state_machine.TransitionTo(ConnectionState::CONNECTED);
    state_machine.TransitionTo(ConnectionState::AUTHENTICATING);
    state_machine.TransitionTo(ConnectionState::AUTHENTICATED);

    // AUTHENTICATED -> KEY_EXCHANGING
    EXPECT_TRUE(state_machine.TransitionTo(ConnectionState::KEY_EXCHANGING));
    EXPECT_TRUE(state_machine.IsInState(ConnectionState::KEY_EXCHANGING));

    // KEY_EXCHANGING -> ENCRYPTED
    EXPECT_TRUE(state_machine.TransitionTo(ConnectionState::ENCRYPTED));
    EXPECT_TRUE(state_machine.IsEncrypted());
    EXPECT_TRUE(state_machine.IsAuthenticated());
    EXPECT_TRUE(state_machine.IsConnected());
}

TEST(ConnectionStateMachineTest, InvalidTransitions) {
    ConnectionStateMachine state_machine;

    // Cannot jump directly to AUTHENTICATED from DISCONNECTED
    EXPECT_FALSE(state_machine.TransitionTo(ConnectionState::AUTHENTICATED));
    EXPECT_TRUE(state_machine.IsDisconnected());

    // Cannot jump to ENCRYPTED without authentication
    state_machine.TransitionTo(ConnectionState::CONNECTING);
    state_machine.TransitionTo(ConnectionState::CONNECTED);
    EXPECT_FALSE(state_machine.TransitionTo(ConnectionState::ENCRYPTED));
    EXPECT_FALSE(state_machine.IsEncrypted());
}

TEST(ConnectionStateMachineTest, OperationPermissions) {
    ConnectionStateMachine state_machine;

    // Initially can connect and close
    EXPECT_TRUE(state_machine.CanConnect());
    EXPECT_FALSE(state_machine.CanAuthenticate());
    EXPECT_FALSE(state_machine.CanSendQuery());
    EXPECT_FALSE(state_machine.CanEncrypt());
    EXPECT_TRUE(state_machine.CanClose());

    // After connecting
    state_machine.TransitionTo(ConnectionState::CONNECTING);
    state_machine.TransitionTo(ConnectionState::CONNECTED);

    EXPECT_FALSE(state_machine.CanConnect());
    EXPECT_TRUE(state_machine.CanAuthenticate());
    EXPECT_FALSE(state_machine.CanSendQuery());
    EXPECT_FALSE(state_machine.CanEncrypt());
    EXPECT_TRUE(state_machine.CanClose());

    // After authenticating
    state_machine.TransitionTo(ConnectionState::AUTHENTICATING);
    state_machine.TransitionTo(ConnectionState::AUTHENTICATED);

    EXPECT_FALSE(state_machine.CanConnect());
    EXPECT_FALSE(state_machine.CanAuthenticate());
    EXPECT_TRUE(state_machine.CanSendQuery());
    EXPECT_TRUE(state_machine.CanEncrypt());
    EXPECT_TRUE(state_machine.CanClose());

    // After encrypting
    state_machine.TransitionTo(ConnectionState::KEY_EXCHANGING);
    state_machine.TransitionTo(ConnectionState::ENCRYPTED);

    EXPECT_FALSE(state_machine.CanConnect());
    EXPECT_FALSE(state_machine.CanAuthenticate());
    EXPECT_TRUE(state_machine.CanSendQuery());
    EXPECT_FALSE(state_machine.CanEncrypt()); // Already encrypted
    EXPECT_TRUE(state_machine.CanClose());
}

TEST(ConnectionStateMachineTest, ErrorHandling) {
    ConnectionStateMachine state_machine;

    // Set up some state
    state_machine.TransitionTo(ConnectionState::CONNECTING);
    state_machine.TransitionTo(ConnectionState::CONNECTED);

    // Force error
    state_machine.ForceError();
    EXPECT_TRUE(state_machine.IsError());
    EXPECT_FALSE(state_machine.IsConnected());
}

TEST(ConnectionStateMachineTest, ResetFunctionality) {
    ConnectionStateMachine state_machine;

    // Change state
    state_machine.TransitionTo(ConnectionState::CONNECTING);
    state_machine.TransitionTo(ConnectionState::CONNECTED);

    EXPECT_TRUE(state_machine.IsConnected());

    // Reset
    state_machine.Reset();
    EXPECT_TRUE(state_machine.IsDisconnected());
    EXPECT_FALSE(state_machine.IsConnected());
}

// Test data transmission validator basic functionality
TEST(DataTransmissionValidatorTest, BasicValidation) {
    DataTransmissionValidator validator;

    // Test magic number validation
    EXPECT_TRUE(validator.ValidateMessageMagic(0x53434C53)); // 'SQLC'
    EXPECT_FALSE(validator.ValidateMessageMagic(0x12345678));

    // Test message type validation
    EXPECT_TRUE(validator.ValidateMessageType(MessageType::CONNECT));
    EXPECT_TRUE(validator.ValidateMessageType(MessageType::QUERY));

    // Test buffer size validation
    EXPECT_TRUE(validator.IsBufferSizeValid(1024));
    EXPECT_TRUE(validator.IsMessageSizeWithinLimits(1024));
}

// Test network exception creation
TEST(NetworkExceptionTest, BasicExceptionCreation) {
    NetworkException exception(NetworkExceptionType::CONNECTION_LOST,
                              "Connection lost", "Socket timeout");

    EXPECT_EQ(exception.GetType(), NetworkExceptionType::CONNECTION_LOST);
    EXPECT_EQ(exception.GetDetails(), "Socket timeout");
    EXPECT_TRUE(exception.IsRecoverable());

    std::string full_message = exception.GetFullMessage();
    EXPECT_NE(full_message.find("Connection lost"), std::string::npos);
}

TEST(NetworkExceptionTest, NonRecoverableException) {
    NetworkException exception(NetworkExceptionType::DATA_CORRUPTION,
                              "Data corruption detected", "", false);

    EXPECT_EQ(exception.GetType(), NetworkExceptionType::DATA_CORRUPTION);
    EXPECT_FALSE(exception.IsRecoverable());
}

// Performance test
TEST(PerformanceTest, SessionCreationPerformance) {
    SessionManager session_manager;

    const int num_sessions = 100;

    // Measure session creation performance
    auto start_time = std::chrono::high_resolution_clock::now();

    std::vector<std::shared_ptr<Session>> sessions;
    for (int i = 0; i < num_sessions; ++i) {
        sessions.push_back(session_manager.CreateSession());
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    // Should create sessions quickly (less than 10ms for 100 sessions)
    EXPECT_LT(duration.count(), 10);

    // Verify all sessions were created
    for (int i = 0; i < num_sessions; ++i) {
        EXPECT_EQ(sessions[i]->GetSessionId(), i + 1);
    }
}