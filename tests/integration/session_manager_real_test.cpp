/**
 * @file session_manager_real_test.cpp
 * @brief SessionManager类的真实单元测试
 * 
 * 测试真实的SessionManager类（来自include/network/network.h），而不是mock版本
 * 测试会话创建、查找、销毁、认证逻辑、权限检查和并发安全性
 */

#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <algorithm>
#include <unordered_map>

// 包含真实的网络模块头文件
#include "network/network.h"
#include "network/encryption.h"

using namespace sqlcc::network;

// 测试夹具
class SessionManagerRealTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建真实的SessionManager实例
        session_manager_ = std::make_shared<SessionManager>();
    }

    void TearDown() override {
        session_manager_.reset();
    }

    std::shared_ptr<SessionManager> session_manager_;
};

// 测试SessionManager基本功能
TEST_F(SessionManagerRealTest, SessionCreation) {
    // 创建第一个会话
    auto session1 = session_manager_->CreateSession();
    ASSERT_NE(session1, nullptr);
    EXPECT_EQ(session1->GetSessionId(), 1);  // 第一个会话ID应该是1

    // 创建第二个会话
    auto session2 = session_manager_->CreateSession();
    ASSERT_NE(session2, nullptr);
    EXPECT_EQ(session2->GetSessionId(), 2);  // 第二个会话ID应该是2

    // 验证会话ID不同
    EXPECT_NE(session1->GetSessionId(), session2->GetSessionId());

    // 验证会话的初始状态
    EXPECT_FALSE(session1->IsAuthenticated());
    EXPECT_FALSE(session2->IsAuthenticated());
}

// 测试会话查找功能
TEST_F(SessionManagerRealTest, SessionLookup) {
    // 创建会话
    auto session = session_manager_->CreateSession();
    int session_id = session->GetSessionId();

    // 查找存在的会话
    auto found_session = session_manager_->GetSession(session_id);
    ASSERT_NE(found_session, nullptr);
    EXPECT_EQ(found_session->GetSessionId(), session_id);

    // 查找不存在的会话
    auto not_found_session = session_manager_->GetSession(99999);
    EXPECT_EQ(not_found_session, nullptr);
}

// 测试会话销毁功能
TEST_F(SessionManagerRealTest, SessionDestruction) {
    // 创建会话
    auto session = session_manager_->CreateSession();
    int session_id = session->GetSessionId();

    // 验证会话存在
    auto found_session = session_manager_->GetSession(session_id);
    ASSERT_NE(found_session, nullptr);

    // 销毁会话
    session_manager_->DestroySession(session_id);

    // 验证会话已被销毁
    auto destroyed_session = session_manager_->GetSession(session_id);
    EXPECT_EQ(destroyed_session, nullptr);
}

// 测试用户认证功能
TEST_F(SessionManagerRealTest, UserAuthentication) {
    // 创建会话
    auto session = session_manager_->CreateSession();
    int session_id = session->GetSessionId();

    // 测试正确的认证信息（admin/password）
    bool auth_result = session_manager_->Authenticate(session_id, "admin", "password");
    EXPECT_TRUE(auth_result);

    // 验证会话状态已更新
    auto authenticated_session = session_manager_->GetSession(session_id);
    ASSERT_NE(authenticated_session, nullptr);
    EXPECT_TRUE(authenticated_session->IsAuthenticated());
    EXPECT_EQ(authenticated_session->GetUser(), "admin");

    // 测试错误的认证信息
    bool wrong_auth = session_manager_->Authenticate(session_id, "admin", "wrong_password");
    EXPECT_FALSE(wrong_auth);

    // 测试不存在的会话
    bool invalid_session_auth = session_manager_->Authenticate(99999, "admin", "password");
    EXPECT_FALSE(invalid_session_auth);

    // 验证认证失败后会话状态没有改变
    auto still_authenticated = session_manager_->GetSession(session_id);
    EXPECT_TRUE(still_authenticated->IsAuthenticated());
    EXPECT_EQ(still_authenticated->GetUser(), "admin");
}

// 测试权限检查功能
TEST_F(SessionManagerRealTest, PermissionCheck) {
    // 创建未认证的会话
    auto unauth_session = session_manager_->CreateSession();
    int unauth_session_id = unauth_session->GetSessionId();

    // 未认证会话应该没有权限
    bool unauth_permission = session_manager_->CheckPermission(unauth_session_id, "test_db", "SELECT");
    EXPECT_FALSE(unauth_permission);

    // 创建已认证的会话
    auto auth_session = session_manager_->CreateSession();
    int auth_session_id = auth_session->GetSessionId();
    session_manager_->Authenticate(auth_session_id, "admin", "password");

    // 已认证会话应该有权限（当前实现总是返回true）
    bool auth_permission = session_manager_->CheckPermission(auth_session_id, "test_db", "SELECT");
    EXPECT_TRUE(auth_permission);

    // 测试不存在的会话
    bool invalid_session_permission = session_manager_->CheckPermission(99999, "test_db", "SELECT");
    EXPECT_FALSE(invalid_session_permission);
}

// 测试边界条件：空用户名和密码
TEST_F(SessionManagerRealTest, EmptyCredentials) {
    auto session = session_manager_->CreateSession();
    int session_id = session->GetSessionId();

    // 测试空用户名
    bool empty_username = session_manager_->Authenticate(session_id, "", "password");
    EXPECT_FALSE(empty_username);

    // 测试空密码
    bool empty_password = session_manager_->Authenticate(session_id, "admin", "");
    EXPECT_FALSE(empty_password);

    // 测试都为空
    bool both_empty = session_manager_->Authenticate(session_id, "", "");
    EXPECT_FALSE(both_empty);

    // 验证会话状态没有改变
    auto still_unauth = session_manager_->GetSession(session_id);
    EXPECT_FALSE(still_unauth->IsAuthenticated());
}

// 测试边界条件：特殊字符凭据
TEST_F(SessionManagerRealTest, SpecialCharacterCredentials) {
    auto session = session_manager_->CreateSession();
    int session_id = session->GetSessionId();

    // 测试包含特殊字符的用户名和密码
    bool special_chars = session_manager_->Authenticate(session_id, "user@domain.com", "pass!@#$%^&*()");
    EXPECT_FALSE(special_chars);  // 当前实现只接受admin/password

    // 测试正确的凭据
    bool correct = session_manager_->Authenticate(session_id, "admin", "password");
    EXPECT_TRUE(correct);
}

// 测试边界条件：长用户名和密码
TEST_F(SessionManagerRealTest, LongCredentials) {
    auto session = session_manager_->CreateSession();
    int session_id = session->GetSessionId();

    // 创建长用户名和密码
    std::string long_username(1000, 'a');
    std::string long_password(1000, 'b');

    // 测试长凭据（应该失败，因为当前实现只接受admin/password）
    bool long_creds = session_manager_->Authenticate(session_id, long_username, long_password);
    EXPECT_FALSE(long_creds);

    // 验证正确凭据仍然工作
    bool correct = session_manager_->Authenticate(session_id, "admin", "password");
    EXPECT_TRUE(correct);
}

// 测试会话ID递增性
TEST_F(SessionManagerRealTest, SessionIdIncrement) {
    // 创建多个会话并验证ID递增
    std::vector<std::shared_ptr<Session>> sessions;
    for (int i = 0; i < 10; ++i) {
        sessions.push_back(session_manager_->CreateSession());
    }

    // 验证会话ID递增
    for (int i = 0; i < 10; ++i) {
        EXPECT_EQ(sessions[i]->GetSessionId(), i + 1);
    }

    // 创建第11个会话
    auto session11 = session_manager_->CreateSession();
    EXPECT_EQ(session11->GetSessionId(), 11);
}

// 测试会话ID重用（不应该重用）
TEST_F(SessionManagerRealTest, SessionIdReuse) {
    // 创建并销毁多个会话
    auto session1 = session_manager_->CreateSession();
    int id1 = session1->GetSessionId();
    session_manager_->DestroySession(id1);

    auto session2 = session_manager_->CreateSession();
    int id2 = session2->GetSessionId();

    // 新创建的会话ID应该是递增的，不应该重用已销毁的ID
    EXPECT_GT(id2, id1);
    EXPECT_EQ(id2, id1 + 1);
}

// 测试多个会话的独立性
TEST_F(SessionManagerRealTest, MultipleSessionIndependence) {
    // 创建多个会话
    auto session1 = session_manager_->CreateSession();
    auto session2 = session_manager_->CreateSession();
    auto session3 = session_manager_->CreateSession();

    int id1 = session1->GetSessionId();
    int id2 = session2->GetSessionId();
    int id3 = session3->GetSessionId();

    // 验证所有ID都不同
    EXPECT_NE(id1, id2);
    EXPECT_NE(id2, id3);
    EXPECT_NE(id1, id3);

    // 认证不同的用户
    session_manager_->Authenticate(id1, "admin", "password");
    // session2保持未认证
    session_manager_->Authenticate(id3, "admin", "password");

    // 验证状态
    auto found1 = session_manager_->GetSession(id1);
    auto found2 = session_manager_->GetSession(id2);
    auto found3 = session_manager_->GetSession(id3);

    EXPECT_TRUE(found1->IsAuthenticated());
    EXPECT_FALSE(found2->IsAuthenticated());
    EXPECT_TRUE(found3->IsAuthenticated());

    EXPECT_EQ(found1->GetUser(), "admin");
    EXPECT_EQ(found3->GetUser(), "admin");
}

// 测试并发访问安全性
TEST(SessionManagerRealConcurrencyTest, ConcurrentAccess) {
    auto session_manager = std::make_shared<SessionManager>();
    const int num_threads = 10;
    const int sessions_per_thread = 100;

    std::vector<std::thread> threads;
    std::mutex results_mutex;
    std::vector<int> created_session_ids;

    // 创建多个线程并发创建会话
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < sessions_per_thread; ++j) {
                auto session = session_manager->CreateSession();
                std::lock_guard<std::mutex> lock(results_mutex);
                created_session_ids.push_back(session->GetSessionId());
            }
        });
    }

    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }

    // 验证所有会话ID都唯一
    std::sort(created_session_ids.begin(), created_session_ids.end());
    auto last = std::unique(created_session_ids.begin(), created_session_ids.end());
    EXPECT_EQ(last - created_session_ids.begin(), num_threads * sessions_per_thread);

    // 验证会话ID连续性（应该从1开始连续）
    std::sort(created_session_ids.begin(), created_session_ids.end());
    for (int i = 0; i < num_threads * sessions_per_thread; ++i) {
        EXPECT_EQ(created_session_ids[i], i + 1);
    }
}

// 测试并发认证访问
TEST(SessionManagerRealConcurrencyTest, ConcurrentAuthentication) {
    auto session_manager = std::make_shared<SessionManager>();
    const int num_sessions = 50;

    // 创建多个会话
    std::vector<std::shared_ptr<Session>> sessions;
    for (int i = 0; i < num_sessions; ++i) {
        sessions.push_back(session_manager->CreateSession());
    }

    // 并发进行认证
    std::vector<std::thread> threads;
    for (int i = 0; i < num_sessions; ++i) {
        threads.emplace_back([session_manager, &sessions, i]() {
            int session_id = sessions[i]->GetSessionId();
            bool auth_result = session_manager->Authenticate(session_id, "admin", "password");
            EXPECT_TRUE(auth_result);
        });
    }

    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }

    // 验证所有会话都已认证
    for (int i = 0; i < num_sessions; ++i) {
        auto session = session_manager->GetSession(sessions[i]->GetSessionId());
        ASSERT_NE(session, nullptr);
        EXPECT_TRUE(session->IsAuthenticated());
        EXPECT_EQ(session->GetUser(), "admin");
    }
}

// 测试销毁不存在的会话
TEST_F(SessionManagerRealTest, DestroyNonExistentSession) {
    // 销毁不存在的会话应该不会崩溃
    EXPECT_NO_THROW(session_manager_->DestroySession(99999));

    // 再次确认不存在
    auto session = session_manager_->GetSession(99999);
    EXPECT_EQ(session, nullptr);

    // 验证现有会话不受影响
    auto existing_session = session_manager_->CreateSession();
    auto found_session = session_manager_->GetSession(existing_session->GetSessionId());
    EXPECT_NE(found_session, nullptr);
}

// 测试认证后再销毁会话
TEST_F(SessionManagerRealTest, AuthenticateThenDestroy) {
    // 创建并认证会话
    auto session = session_manager_->CreateSession();
    int session_id = session->GetSessionId();
    session_manager_->Authenticate(session_id, "admin", "password");

    // 验证认证状态
    auto authenticated_session = session_manager_->GetSession(session_id);
    ASSERT_NE(authenticated_session, nullptr);
    EXPECT_TRUE(authenticated_session->IsAuthenticated());

    // 销毁会话
    session_manager_->DestroySession(session_id);

    // 验证会话已被销毁
    auto destroyed_session = session_manager_->GetSession(session_id);
    EXPECT_EQ(destroyed_session, nullptr);
}

// 测试权限检查的各种场景
TEST_F(SessionManagerRealTest, PermissionCheckScenarios) {
    // 创建会话但不认证
    auto session1 = session_manager_->CreateSession();
    int id1 = session1->GetSessionId();

    // 未认证用户无权限
    EXPECT_FALSE(session_manager_->CheckPermission(id1, "db1", "SELECT"));
    EXPECT_FALSE(session_manager_->CheckPermission(id1, "db1", "INSERT"));
    EXPECT_FALSE(session_manager_->CheckPermission(id1, "", "SELECT"));  // 空数据库名
    EXPECT_FALSE(session_manager_->CheckPermission(id1, "db1", ""));     // 空操作名

    // 认证用户有权限
    session_manager_->Authenticate(id1, "admin", "password");
    EXPECT_TRUE(session_manager_->CheckPermission(id1, "db1", "SELECT"));
    EXPECT_TRUE(session_manager_->CheckPermission(id1, "db1", "INSERT"));
    EXPECT_TRUE(session_manager_->CheckPermission(id1, "", "SELECT"));
    EXPECT_TRUE(session_manager_->CheckPermission(id1, "db1", ""));

    // 不存在的会话无权限
    EXPECT_FALSE(session_manager_->CheckPermission(99999, "db1", "SELECT"));
}

// 测试大量会话的创建和销毁
TEST_F(SessionManagerRealTest, LargeNumberOfSessions) {
    const int num_sessions = 1000;
    std::vector<int> session_ids;

    // 创建大量会话
    for (int i = 0; i < num_sessions; ++i) {
        auto session = session_manager_->CreateSession();
        session_ids.push_back(session->GetSessionId());
    }

    // 验证所有会话都可以被查找
    for (int session_id : session_ids) {
        auto session = session_manager_->GetSession(session_id);
        ASSERT_NE(session, nullptr);
        EXPECT_EQ(session->GetSessionId(), session_id);
    }

    // 销毁一半的会话
    for (int i = 0; i < num_sessions / 2; ++i) {
        session_manager_->DestroySession(session_ids[i]);
    }

    // 验证已销毁的会话无法找到
    for (int i = 0; i < num_sessions / 2; ++i) {
        auto session = session_manager_->GetSession(session_ids[i]);
        EXPECT_EQ(session, nullptr);
    }

    // 验证剩余的会话仍然存在
    for (int i = num_sessions / 2; i < num_sessions; ++i) {
        auto session = session_manager_->GetSession(session_ids[i]);
        ASSERT_NE(session, nullptr);
        EXPECT_EQ(session->GetSessionId(), session_ids[i]);
    }
}

// 测试错误凭据的处理
TEST_F(SessionManagerRealTest, InvalidCredentialsHandling) {
    auto session = session_manager_->CreateSession();
    int session_id = session->GetSessionId();

    // 测试各种无效凭据
    std::vector<std::pair<std::string, std::string>> invalid_credentials = {
        {"", ""},
        {"admin", ""},
        {"", "password"},
        {"wrong_user", "password"},
        {"admin", "wrong_password"},
        {"wrong_user", "wrong_password"},
        {"123", "456"},
        {"special!@#", "chars$%^"}
    };

    for (const auto& creds : invalid_credentials) {
        bool auth_result = session_manager_->Authenticate(session_id, creds.first, creds.second);
        EXPECT_FALSE(auth_result);
    }

    // 验证会话状态没有改变
    auto still_unauth = session_manager_->GetSession(session_id);
    EXPECT_FALSE(still_unauth->IsAuthenticated());
    EXPECT_EQ(still_unauth->GetUser(), "");

    // 验证正确的凭据仍然有效
    bool valid_auth = session_manager_->Authenticate(session_id, "admin", "password");
    EXPECT_TRUE(valid_auth);
}

// 测试会话管理器的生命周期
TEST_F(SessionManagerRealTest, SessionManagerLifecycle) {
    // 创建多个会话并设置状态
    auto session1 = session_manager_->CreateSession();
    auto session2 = session_manager_->CreateSession();
    
    int id1 = session1->GetSessionId();
    int id2 = session2->GetSessionId();

    session_manager_->Authenticate(id1, "admin", "password");
    session1->SetEncryptionDisabled(true);
    
    // 销毁一个会话
    session_manager_->DestroySession(id1);

    // 创建新会话（ID应该继续递增）
    auto session3 = session_manager_->CreateSession();
    int id3 = session3->GetSessionId();

    EXPECT_GT(id3, id2);

    // 验证剩余会话的状态
    auto found2 = session_manager_->GetSession(id2);
    auto found3 = session_manager_->GetSession(id3);

    EXPECT_NE(found2, nullptr);
    EXPECT_NE(found3, nullptr);
    
    EXPECT_FALSE(found2->IsAuthenticated());  // 未认证
    EXPECT_FALSE(found3->IsAuthenticated());  // 未认证

    // 验证已销毁会话的状态（应该为nullptr）
    auto destroyed = session_manager_->GetSession(id1);
    EXPECT_EQ(destroyed, nullptr);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
