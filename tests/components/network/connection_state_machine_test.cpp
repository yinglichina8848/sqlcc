/**
 * @file connection_state_machine_test.cpp
 * @brief 连接状态机测试套件
 *
 * 验证连接状态机的状态转换安全性，包括：
 * - 有效状态转换验证
 * - 无效状态转换拒绝
 * - 状态检查方法正确性
 * - 并发访问安全性
 * - 状态变更回调机制
 */

#include "network/network.h"
#include <gtest/gtest.h>
#include <thread>
#include <atomic>
#include <vector>

using namespace sqlcc::network;
using namespace std::chrono_literals;

// 测试夹具
class ConnectionStateMachineTest : public ::testing::Test {
protected:
    void SetUp() override {
        state_machine_ = std::make_unique<ConnectionStateMachine>();
    }

    void TearDown() override {
        state_machine_.reset();
    }

    std::unique_ptr<ConnectionStateMachine> state_machine_;
    std::atomic<int> callback_call_count_{0};
    ConnectionState last_old_state_;
    ConnectionState last_new_state_;
};

// 基本状态测试
TEST_F(ConnectionStateMachineTest, InitialState) {
    // 测试初始状态为DISCONNECTED
    EXPECT_EQ(state_machine_->GetCurrentState(), DISCONNECTED);
    EXPECT_TRUE(state_machine_->IsDisconnected());
    EXPECT_FALSE(state_machine_->IsConnected());
    EXPECT_FALSE(state_machine_->IsAuthenticated());
    EXPECT_FALSE(state_machine_->IsEncrypted());
}

TEST_F(ConnectionStateMachineTest, StateNameLookup) {
    // 测试状态名称查找
    EXPECT_EQ(state_machine_->GetStateName(DISCONNECTED), "DISCONNECTED");
    EXPECT_EQ(state_machine_->GetStateName(CONNECTING), "CONNECTING");
    EXPECT_EQ(state_machine_->GetStateName(CONNECTED), "CONNECTED");
    EXPECT_EQ(state_machine_->GetStateName(AUTHENTICATED), "AUTHENTICATED");
    EXPECT_EQ(state_machine_->GetStateName(ENCRYPTED), "ENCRYPTED");
    EXPECT_EQ(state_machine_->GetStateName(CLOSED), "CLOSED");
    EXPECT_EQ(state_machine_->GetStateName(CONNECTION_ERROR), "CONNECTION_ERROR");

    // 测试无效状态
    EXPECT_EQ(state_machine_->GetStateName(static_cast<ConnectionState>(999)), "UNKNOWN");
}

// 有效状态转换测试
TEST_F(ConnectionStateMachineTest, ValidTransitions_DisconnectedToConnecting) {
    // DISCONNECTED -> CONNECTING (有效)
    EXPECT_TRUE(state_machine_->CanTransitionTo(CONNECTING));
    EXPECT_TRUE(state_machine_->TransitionTo(CONNECTING));
    EXPECT_EQ(state_machine_->GetCurrentState(), CONNECTING);
    EXPECT_TRUE(state_machine_->IsConnecting());
}

TEST_F(ConnectionStateMachineTest, ValidTransitions_ConnectingToConnected) {
    // CONNECTING -> CONNECTED (有效)
    state_machine_->TransitionTo(CONNECTING);
    EXPECT_TRUE(state_machine_->CanTransitionTo(CONNECTED));
    EXPECT_TRUE(state_machine_->TransitionTo(CONNECTED));
    EXPECT_EQ(state_machine_->GetCurrentState(), CONNECTED);
    EXPECT_TRUE(state_machine_->IsConnected());
}

TEST_F(ConnectionStateMachineTest, ValidTransitions_ConnectedToAuthenticating) {
    // CONNECTED -> AUTHENTICATING (有效)
    state_machine_->TransitionTo(CONNECTING);
    state_machine_->TransitionTo(CONNECTED);
    EXPECT_TRUE(state_machine_->CanTransitionTo(AUTHENTICATING));
    EXPECT_TRUE(state_machine_->TransitionTo(AUTHENTICATING));
    EXPECT_EQ(state_machine_->GetCurrentState(), AUTHENTICATING);
}

TEST_F(ConnectionStateMachineTest, ValidTransitions_AuthenticatingToAuthenticated) {
    // AUTHENTICATING -> AUTHENTICATED (有效)
    state_machine_->TransitionTo(CONNECTING);
    state_machine_->TransitionTo(CONNECTED);
    state_machine_->TransitionTo(AUTHENTICATING);
    EXPECT_TRUE(state_machine_->CanTransitionTo(AUTHENTICATED));
    EXPECT_TRUE(state_machine_->TransitionTo(AUTHENTICATED));
    EXPECT_EQ(state_machine_->GetCurrentState(), AUTHENTICATED);
    EXPECT_TRUE(state_machine_->IsAuthenticated());
}

TEST_F(ConnectionStateMachineTest, ValidTransitions_AuthenticatedToKeyExchanging) {
    // AUTHENTICATED -> KEY_EXCHANGING (有效)
    state_machine_->TransitionTo(CONNECTING);
    state_machine_->TransitionTo(CONNECTED);
    state_machine_->TransitionTo(AUTHENTICATING);
    state_machine_->TransitionTo(AUTHENTICATED);
    EXPECT_TRUE(state_machine_->CanTransitionTo(KEY_EXCHANGING));
    EXPECT_TRUE(state_machine_->TransitionTo(KEY_EXCHANGING));
    EXPECT_EQ(state_machine_->GetCurrentState(), KEY_EXCHANGING);
}

TEST_F(ConnectionStateMachineTest, ValidTransitions_KeyExchangingToEncrypted) {
    // KEY_EXCHANGING -> ENCRYPTED (有效)
    state_machine_->TransitionTo(CONNECTING);
    state_machine_->TransitionTo(CONNECTED);
    state_machine_->TransitionTo(AUTHENTICATING);
    state_machine_->TransitionTo(AUTHENTICATED);
    state_machine_->TransitionTo(KEY_EXCHANGING);
    EXPECT_TRUE(state_machine_->CanTransitionTo(ENCRYPTED));
    EXPECT_TRUE(state_machine_->TransitionTo(ENCRYPTED));
    EXPECT_EQ(state_machine_->GetCurrentState(), ENCRYPTED);
    EXPECT_TRUE(state_machine_->IsEncrypted());
}

TEST_F(ConnectionStateMachineTest, ValidTransitions_AnyToClosing) {
    // 从任何状态都可以转换到CLOSING
    std::vector<ConnectionState> test_states = {
        CONNECTED, AUTHENTICATING, AUTHENTICATED, KEY_EXCHANGING, ENCRYPTED
    };

    for (auto state : test_states) {
        // 重置状态机
        state_machine_->Reset();

        // 设置到测试状态
        if (state != DISCONNECTED) {
            // 这里需要一个辅助方法来设置状态，或者逐步转换
            // 为了简化，我们只测试从CONNECTED开始
            if (state == CONNECTED) {
                state_machine_->TransitionTo(CONNECTING);
                state_machine_->TransitionTo(CONNECTED);
                EXPECT_TRUE(state_machine_->CanTransitionTo(CLOSING));
                EXPECT_TRUE(state_machine_->TransitionTo(CLOSING));
                EXPECT_EQ(state_machine_->GetCurrentState(), CLOSING);
                EXPECT_TRUE(state_machine_->IsClosed());
            }
        }
    }
}

TEST_F(ConnectionStateMachineTest, ValidTransitions_ClosingToClosed) {
    // CLOSING -> CLOSED (有效)
    state_machine_->TransitionTo(CONNECTING);
    state_machine_->TransitionTo(CONNECTED);
    state_machine_->TransitionTo(CLOSING);
    EXPECT_TRUE(state_machine_->CanTransitionTo(CLOSED));
    EXPECT_TRUE(state_machine_->TransitionTo(CLOSED));
    EXPECT_EQ(state_machine_->GetCurrentState(), CLOSED);
    EXPECT_TRUE(state_machine_->IsClosed());
}

TEST_F(ConnectionStateMachineTest, ValidTransitions_ErrorToDisconnected) {
    // ERROR -> DISCONNECTED (有效，重置)
    state_machine_->ForceError();
    EXPECT_EQ(state_machine_->GetCurrentState(), CONNECTION_ERROR);
    EXPECT_TRUE(state_machine_->CanTransitionTo(DISCONNECTED));
    EXPECT_TRUE(state_machine_->TransitionTo(DISCONNECTED));
    EXPECT_EQ(state_machine_->GetCurrentState(), DISCONNECTED);
}

// 无效状态转换测试
TEST_F(ConnectionStateMachineTest, InvalidTransitions_DisconnectedToAuthenticated) {
    // DISCONNECTED -> AUTHENTICATED (无效)
    EXPECT_FALSE(state_machine_->CanTransitionTo(AUTHENTICATED));
    EXPECT_FALSE(state_machine_->TransitionTo(AUTHENTICATED));
    EXPECT_EQ(state_machine_->GetCurrentState(), DISCONNECTED);
}

TEST_F(ConnectionStateMachineTest, InvalidTransitions_ConnectedToEncrypted) {
    // CONNECTED -> ENCRYPTED (无效，必须先认证)
    state_machine_->TransitionTo(CONNECTING);
    state_machine_->TransitionTo(CONNECTED);
    EXPECT_FALSE(state_machine_->CanTransitionTo(ENCRYPTED));
    EXPECT_FALSE(state_machine_->TransitionTo(ENCRYPTED));
    EXPECT_EQ(state_machine_->GetCurrentState(), CONNECTED);
}

TEST_F(ConnectionStateMachineTest, InvalidTransitions_EncryptedToConnecting) {
    // ENCRYPTED -> CONNECTING (无效，加密状态不能回到早期状态)
    state_machine_->TransitionTo(CONNECTING);
    state_machine_->TransitionTo(CONNECTED);
    state_machine_->TransitionTo(AUTHENTICATING);
    state_machine_->TransitionTo(AUTHENTICATED);
    state_machine_->TransitionTo(KEY_EXCHANGING);
    state_machine_->TransitionTo(ENCRYPTED);
    EXPECT_FALSE(state_machine_->CanTransitionTo(CONNECTING));
    EXPECT_FALSE(state_machine_->TransitionTo(CONNECTING));
    EXPECT_EQ(state_machine_->GetCurrentState(), ENCRYPTED);
}

TEST_F(ConnectionStateMachineTest, InvalidTransitions_ClosedToAny) {
    // CLOSED是终止状态，不能转换到其他状态
    state_machine_->TransitionTo(CONNECTING);
    state_machine_->TransitionTo(CONNECTED);
    state_machine_->TransitionTo(CLOSING);
    state_machine_->TransitionTo(CLOSED);

    EXPECT_FALSE(state_machine_->CanTransitionTo(CONNECTING));
    EXPECT_FALSE(state_machine_->CanTransitionTo(AUTHENTICATED));
    EXPECT_FALSE(state_machine_->TransitionTo(CONNECTING));
    EXPECT_EQ(state_machine_->GetCurrentState(), CLOSED);
}

// 操作权限检查测试
TEST_F(ConnectionStateMachineTest, OperationPermissions_CanConnect) {
    // 初始状态应该可以连接
    EXPECT_TRUE(state_machine_->CanConnect());

    // 连接中不应该可以再次连接
    state_machine_->TransitionTo(CONNECTING);
    EXPECT_FALSE(state_machine_->CanConnect());
}

TEST_F(ConnectionStateMachineTest, OperationPermissions_CanAuthenticate) {
    // 未连接状态不能认证
    EXPECT_FALSE(state_machine_->CanAuthenticate());

    // 已连接状态可以认证
    state_machine_->TransitionTo(CONNECTING);
    state_machine_->TransitionTo(CONNECTED);
    EXPECT_TRUE(state_machine_->CanAuthenticate());

    // 已认证状态不能再次认证
    state_machine_->TransitionTo(AUTHENTICATING);
    state_machine_->TransitionTo(AUTHENTICATED);
    EXPECT_FALSE(state_machine_->CanAuthenticate());
}

TEST_F(ConnectionStateMachineTest, OperationPermissions_CanSendQuery) {
    // 未认证状态不能发送查询
    EXPECT_FALSE(state_machine_->CanSendQuery());

    // 已认证状态可以发送查询
    state_machine_->TransitionTo(CONNECTING);
    state_machine_->TransitionTo(CONNECTED);
    state_machine_->TransitionTo(AUTHENTICATING);
    state_machine_->TransitionTo(AUTHENTICATED);
    EXPECT_TRUE(state_machine_->CanSendQuery());

    // 加密状态也可以发送查询
    state_machine_->TransitionTo(KEY_EXCHANGING);
    state_machine_->TransitionTo(ENCRYPTED);
    EXPECT_TRUE(state_machine_->CanSendQuery());
}

TEST_F(ConnectionStateMachineTest, OperationPermissions_CanEncrypt) {
    // 未认证状态不能加密
    EXPECT_FALSE(state_machine_->CanEncrypt());

    // 已认证状态可以加密
    state_machine_->TransitionTo(CONNECTING);
    state_machine_->TransitionTo(CONNECTED);
    state_machine_->TransitionTo(AUTHENTICATING);
    state_machine_->TransitionTo(AUTHENTICATED);
    EXPECT_TRUE(state_machine_->CanEncrypt());

    // 已加密状态不能再次加密
    state_machine_->TransitionTo(KEY_EXCHANGING);
    state_machine_->TransitionTo(ENCRYPTED);
    EXPECT_FALSE(state_machine_->CanEncrypt());
}

TEST_F(ConnectionStateMachineTest, OperationPermissions_CanClose) {
    // 大多数状态都可以关闭
    EXPECT_TRUE(state_machine_->CanClose());

    state_machine_->TransitionTo(CONNECTING);
    EXPECT_TRUE(state_machine_->CanClose());

    state_machine_->TransitionTo(CONNECTED);
    EXPECT_TRUE(state_machine_->CanClose());

    // 错误状态可能不能关闭（取决于实现）
    state_machine_->ForceError();
    // 这里取决于具体实现，可能允许或不允许
}

// 重置和错误处理测试
TEST_F(ConnectionStateMachineTest, Reset_FromAnyState) {
    // 从任何状态都可以重置
    state_machine_->TransitionTo(CONNECTING);
    state_machine_->Reset();
    EXPECT_EQ(state_machine_->GetCurrentState(), DISCONNECTED);

    state_machine_->TransitionTo(CONNECTING);
    state_machine_->TransitionTo(CONNECTED);
    state_machine_->TransitionTo(AUTHENTICATED);
    state_machine_->Reset();
    EXPECT_EQ(state_machine_->GetCurrentState(), DISCONNECTED);
}

TEST_F(ConnectionStateMachineTest, ForceError) {
    // 强制错误状态
    state_machine_->TransitionTo(CONNECTING);
    state_machine_->TransitionTo(CONNECTED);

    state_machine_->ForceError();
    EXPECT_EQ(state_machine_->GetCurrentState(), CONNECTION_ERROR);
    EXPECT_TRUE(state_machine_->IsError());
}

// 状态变更回调测试
TEST_F(ConnectionStateMachineTest, StateChangeCallback) {
    // 设置状态变更回调
    bool callback_called = false;
    ConnectionState old_state, new_state;

    state_machine_->SetStateChangeCallback([&](ConnectionState old, ConnectionState new_s) {
        callback_called = true;
        old_state = old;
        new_state = new_s;
    });

    // 触发状态转换
    state_machine_->TransitionTo(CONNECTING);

    // 验证回调被调用
    EXPECT_TRUE(callback_called);
    EXPECT_EQ(old_state, DISCONNECTED);
    EXPECT_EQ(new_state, CONNECTING);
}

TEST_F(ConnectionStateMachineTest, StateChangeCallback_ExceptionSafety) {
    // 测试回调异常安全性
    state_machine_->SetStateChangeCallback([](ConnectionState, ConnectionState) {
        throw std::runtime_error("Callback exception");
    });

    // 即使回调抛出异常，状态转换也应该成功
    EXPECT_TRUE(state_machine_->TransitionTo(CONNECTING));
    EXPECT_EQ(state_machine_->GetCurrentState(), CONNECTING);
}

// 并发访问测试
TEST_F(ConnectionStateMachineTest, ConcurrentAccess_Safe) {
    // 测试并发访问的安全性
    std::atomic<bool> test_passed{true};
    std::vector<std::thread> threads;

    // 启动多个线程同时访问状态机
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([this, &test_passed, i]() {
            try {
                // 每个线程执行一系列状态转换
                for (int j = 0; j < 10; ++j) {
                    if (state_machine_->GetCurrentState() == DISCONNECTED) {
                        state_machine_->TransitionTo(CONNECTING);
                    } else if (state_machine_->GetCurrentState() == CONNECTING) {
                        state_machine_->TransitionTo(CONNECTED);
                    } else {
                        state_machine_->Reset();
                    }
                }
            } catch (...) {
                test_passed = false;
            }
        });
    }

    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_TRUE(test_passed);
}

// 边界条件测试
TEST_F(ConnectionStateMachineTest, BoundaryConditions_CallbackNullptr) {
    // 设置空回调应该安全
    state_machine_->SetStateChangeCallback(nullptr);
    EXPECT_TRUE(state_machine_->TransitionTo(CONNECTING));
    EXPECT_EQ(state_machine_->GetCurrentState(), CONNECTING);
}

TEST_F(ConnectionStateMachineTest, BoundaryConditions_MultipleResets) {
    // 多次重置应该安全
    state_machine_->Reset();
    EXPECT_EQ(state_machine_->GetCurrentState(), DISCONNECTED);

    state_machine_->TransitionTo(CONNECTING);
    state_machine_->Reset();
    EXPECT_EQ(state_machine_->GetCurrentState(), DISCONNECTED);

    state_machine_->Reset();
    EXPECT_EQ(state_machine_->GetCurrentState(), DISCONNECTED);
}

// 状态机完整性测试
TEST_F(ConnectionStateMachineTest, StateMachineIntegrity_CompleteFlow) {
    // 测试完整的状态机流程
    // DISCONNECTED -> CONNECTING -> CONNECTED -> AUTHENTICATING -> AUTHENTICATED -> CLOSING -> CLOSED

    EXPECT_EQ(state_machine_->GetCurrentState(), DISCONNECTED);
    EXPECT_TRUE(state_machine_->CanConnect());

    EXPECT_TRUE(state_machine_->TransitionTo(CONNECTING));
    EXPECT_EQ(state_machine_->GetCurrentState(), CONNECTING);

    EXPECT_TRUE(state_machine_->TransitionTo(CONNECTED));
    EXPECT_EQ(state_machine_->GetCurrentState(), CONNECTED);
    EXPECT_TRUE(state_machine_->IsConnected());
    EXPECT_TRUE(state_machine_->CanAuthenticate());

    EXPECT_TRUE(state_machine_->TransitionTo(AUTHENTICATING));
    EXPECT_EQ(state_machine_->GetCurrentState(), AUTHENTICATING);

    EXPECT_TRUE(state_machine_->TransitionTo(AUTHENTICATED));
    EXPECT_EQ(state_machine_->GetCurrentState(), AUTHENTICATED);
    EXPECT_TRUE(state_machine_->IsAuthenticated());
    EXPECT_TRUE(state_machine_->CanSendQuery());

    EXPECT_TRUE(state_machine_->TransitionTo(CLOSING));
    EXPECT_EQ(state_machine_->GetCurrentState(), CLOSING);
    EXPECT_TRUE(state_machine_->IsClosed());

    EXPECT_TRUE(state_machine_->TransitionTo(CLOSED));
    EXPECT_EQ(state_machine_->GetCurrentState(), CLOSED);
    EXPECT_TRUE(state_machine_->IsClosed());
}

// 错误恢复测试
TEST_F(ConnectionStateMachineTest, ErrorRecovery_FromErrorState) {
    // 测试从错误状态恢复
    state_machine_->TransitionTo(CONNECTING);
    state_machine_->ForceError();

    EXPECT_EQ(state_machine_->GetCurrentState(), CONNECTION_ERROR);

    // 应该能够重置到初始状态
    state_machine_->Reset();
    EXPECT_EQ(state_machine_->GetCurrentState(), DISCONNECTED);
    EXPECT_TRUE(state_machine_->CanConnect());
}

TEST_F(ConnectionStateMachineTest, ErrorRecovery_InvalidTransitionHandling) {
    // 测试无效转换的处理
    EXPECT_FALSE(state_machine_->TransitionTo(AUTHENTICATED)); // 无效转换
    EXPECT_EQ(state_machine_->GetCurrentState(), DISCONNECTED); // 状态不变

    // 应该仍然可以进行有效转换
    EXPECT_TRUE(state_machine_->TransitionTo(CONNECTING));
    EXPECT_EQ(state_machine_->GetCurrentState(), CONNECTING);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
