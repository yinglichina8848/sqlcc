#include "network/network.h"
#include <gtest/gtest.h>
#include <memory>
#include <string>

using namespace sqlcc::network;

// 测试Session类
class SessionTest : public ::testing::Test {
protected:
  Session *session_;

  void SetUp() override { session_ = new Session(1); }

  void TearDown() override { delete session_; }
};

TEST_F(SessionTest, Constructor) {
  EXPECT_EQ(session_->GetSessionId(), 1);
  EXPECT_FALSE(session_->IsAuthenticated());
  EXPECT_TRUE(session_->GetUser().empty());
}

TEST_F(SessionTest, Authentication) {
  EXPECT_FALSE(session_->IsAuthenticated());

  session_->SetAuthenticated("test_user");
  EXPECT_TRUE(session_->IsAuthenticated());
  EXPECT_EQ(session_->GetUser(), "test_user");
}

TEST_F(SessionTest, EncryptionSettings) {
  // 默认情况下加密应该是启用的
  EXPECT_FALSE(session_->IsEncryptionDisabled());

  session_->SetEncryptionDisabled(true);
  EXPECT_TRUE(session_->IsEncryptionDisabled());

  session_->SetEncryptionDisabled(false);
  EXPECT_FALSE(session_->IsEncryptionDisabled());
}

TEST_F(SessionTest, AuthenticationSettings) {
  // 默认情况下认证应该是启用的
  EXPECT_FALSE(session_->IsAuthenticationDisabled());

  session_->SetAuthenticationDisabled(true);
  EXPECT_TRUE(session_->IsAuthenticationDisabled());

  session_->SetAuthenticationDisabled(false);
  EXPECT_FALSE(session_->IsAuthenticationDisabled());
}

// 测试SessionManager类
class SessionManagerTest : public ::testing::Test {
protected:
  std::shared_ptr<SessionManager> session_manager_;

  void SetUp() override {
    session_manager_ = std::make_shared<SessionManager>();
  }

  void TearDown() override {
    // SessionManager会自动管理会话生命周期
  }
};

TEST_F(SessionManagerTest, CreateAndGetSession) {
  // 创建会话
  std::shared_ptr<Session> session1 = session_manager_->CreateSession();
  EXPECT_NE(session1, nullptr);
  EXPECT_EQ(session1->GetSessionId(), 1);

  std::shared_ptr<Session> session2 = session_manager_->CreateSession();
  EXPECT_NE(session2, nullptr);
  EXPECT_EQ(session2->GetSessionId(), 2);

  // 获取会话
  std::shared_ptr<Session> retrieved_session = session_manager_->GetSession(1);
  EXPECT_NE(retrieved_session, nullptr);
  EXPECT_EQ(retrieved_session->GetSessionId(), 1);

  // 获取不存在的会话
  std::shared_ptr<Session> non_existent_session =
      session_manager_->GetSession(999);
  EXPECT_EQ(non_existent_session, nullptr);
}

TEST_F(SessionManagerTest, DestroySession) {
  // 创建会话
  std::shared_ptr<Session> session = session_manager_->CreateSession();
  int session_id = session->GetSessionId();

  // 销毁会话
  session_manager_->DestroySession(session_id);

  // 验证会话已销毁
  std::shared_ptr<Session> retrieved_session =
      session_manager_->GetSession(session_id);
  EXPECT_EQ(retrieved_session, nullptr);
}

TEST_F(SessionManagerTest, Authenticate) {
  // 创建会话
  std::shared_ptr<Session> session = session_manager_->CreateSession();
  int session_id = session->GetSessionId();

  // 测试使用正确的用户名和密码进行认证
  EXPECT_TRUE(session_manager_->Authenticate(session_id, "admin", "password"));

  // 测试使用错误的用户名和密码进行认证
  EXPECT_FALSE(
      session_manager_->Authenticate(session_id, "test_user", "test_password"));

  // 测试认证不存在的会话
  EXPECT_FALSE(session_manager_->Authenticate(999, "admin", "password"));
}

TEST_F(SessionManagerTest, CheckPermission) {
  // 创建并认证会话
  std::shared_ptr<Session> session = session_manager_->CreateSession();
  int session_id = session->GetSessionId();

  // 使用正确的用户名和密码进行认证
  session_manager_->Authenticate(session_id, "admin", "password");

  // 测试权限检查
  EXPECT_TRUE(
      session_manager_->CheckPermission(session_id, "test_db", "SELECT"));

  // 测试不存在的会话
  EXPECT_FALSE(session_manager_->CheckPermission(999, "test_db", "SELECT"));

  // 测试未认证的会话
  std::shared_ptr<Session> session2 = session_manager_->CreateSession();
  int session_id2 = session2->GetSessionId();
  EXPECT_FALSE(
      session_manager_->CheckPermission(session_id2, "test_db", "SELECT"));
}

// 测试MessageHeader结构
TEST(MessageHeaderTest, Structure) {
  MessageHeader header;
  header.magic = 0x53514C43; // 'SQLC'
  header.length = 100;
  header.type = QUERY;
  header.flags = 0;
  header.sequence_id = 1;

  EXPECT_EQ(header.magic, 0x53514C43);
  EXPECT_EQ(header.length, 100);
  EXPECT_EQ(header.type, QUERY);
  EXPECT_EQ(header.flags, 0);
  EXPECT_EQ(header.sequence_id, 1);
}

// 测试消息类型枚举
TEST(MessageTypeTest, Values) {
  EXPECT_EQ(CONNECT, 0);
  EXPECT_EQ(CONN_ACK, 1);
  EXPECT_EQ(AUTH, 2);
  EXPECT_EQ(AUTH_ACK, 3);
  EXPECT_EQ(QUERY, 4);
  EXPECT_EQ(QUERY_RESULT, 5);
  EXPECT_EQ(ERROR, 6);
  EXPECT_EQ(CLOSE, 7);
  EXPECT_EQ(KEY_EXCHANGE, 8);
  EXPECT_EQ(KEY_EXCHANGE_ACK, 9);
}

// 测试SessionManager的会话ID生成
TEST_F(SessionManagerTest, SessionIdGeneration) {
  // 创建多个会话，检查会话ID是否递增
  std::shared_ptr<Session> session1 = session_manager_->CreateSession();
  std::shared_ptr<Session> session2 = session_manager_->CreateSession();
  std::shared_ptr<Session> session3 = session_manager_->CreateSession();

  EXPECT_LT(session1->GetSessionId(), session2->GetSessionId());
  EXPECT_LT(session2->GetSessionId(), session3->GetSessionId());

  // 销毁一个会话，创建新会话，检查会话ID是否继续递增
  session_manager_->DestroySession(session2->GetSessionId());
  std::shared_ptr<Session> session4 = session_manager_->CreateSession();
  EXPECT_LT(session3->GetSessionId(), session4->GetSessionId());
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
