#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <functional>
#include <thread>
#include <chrono>
#include <atomic>
#include <vector>

#include "src/utils/connection_pool.h"
#include "src/utils/ssl_wrapper.h"

namespace sqlcc {
namespace test {

// ==================== ConnectionPool Tests ====================

// 模拟连接类用于测试
class MockConnection {
public:
    MockConnection(int id) : id_(id) {}
    ~MockConnection() = default;
    int get_id() const { return id_; }
    
private:
    int id_;
};

// 连接验证器
bool validate_connection(const std::shared_ptr<MockConnection>& conn) {
    return conn != nullptr && conn->get_id() >= 0;
}

class ConnectionPoolTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建连接工厂
        factory_ = []() {
            static int counter = 0;
            return std::make_shared<MockConnection>(++counter);
        };
        
        validator_ = validate_connection;
        
        pool_ = std::make_unique<utils::ConnectionPool<MockConnection>>(
            factory_, validator_, config_);
    }

    void TearDown() override {
        if (pool_) {
            pool_->shutdown();
            pool_.reset();
        }
    }

    using Pool = utils::ConnectionPool<MockConnection>;
    std::unique_ptr<Pool> pool_;
    typename Pool::ConnectionFactory factory_;
    typename Pool::ConnectionValidator validator_;
    typename Pool::PoolConfig config_;
};

// 测试默认构造
TEST_F(ConnectionPoolTest, DefaultConstruction) {
    Pool::PoolConfig default_config;
    EXPECT_EQ(default_config.initial_size, 10);
    EXPECT_EQ(default_config.max_size, 100);
    EXPECT_EQ(default_config.min_size, 5);
}

// 测试连接池启动
TEST_F(ConnectionPoolTest, StartAndShutdown) {
    pool_->start();
    auto stats = pool_->getStats();
    EXPECT_EQ(stats.created_connections, config_.initial_size);
    
    pool_->shutdown();
    stats = pool_->getStats();
    EXPECT_EQ(stats.active_connections, 0);
    EXPECT_EQ(stats.idle_connections, 0);
}

// 测试获取连接
TEST_F(ConnectionPoolTest, AcquireConnection) {
    pool_->start();
    
    auto conn = pool_->acquire();
    ASSERT_NE(conn, nullptr);
    EXPECT_GE(conn->get_id(), 1);
    
    pool_->release(conn);
    
    auto stats = pool_->getStats();
    EXPECT_EQ(stats.active_connections, 0);
    EXPECT_EQ(stats.idle_connections, config_.initial_size);
}

// 测试连接释放
TEST_F(ConnectionPoolTest, ReleaseConnection) {
    pool_->start();
    
    auto conn1 = pool_->acquire();
    auto conn2 = pool_->acquire();
    
    pool_->release(conn1);
    pool_->release(conn2);
    
    auto stats = pool_->getStats();
    EXPECT_EQ(stats.active_connections, 0);
    EXPECT_EQ(stats.idle_connections, config_.initial_size);
}

// 测试连接耗尽时创建新连接
TEST_F(ConnectionPoolTest, CreateNewConnectionWhenExhausted) {
    config_.initial_size = 2;
    config_.max_size = 5;
    pool_ = std::make_unique<Pool>(factory_, validator_, config_);
    pool_->start();
    
    // 获取所有初始连接
    auto conn1 = pool_->acquire();
    auto conn2 = pool_->acquire();
    
    // 请求第三个连接（应该创建新连接）
    auto conn3 = pool_->acquire();
    ASSERT_NE(conn3, nullptr);
    
    pool_->release(conn1);
    pool_->release(conn2);
    pool_->release(conn3);
}

// 测试获取统计信息
TEST_F(ConnectionPoolTest, GetStatistics) {
    pool_->start();
    
    auto stats = pool_->getStats();
    EXPECT_EQ(stats.created_connections, config_.initial_size);
    EXPECT_EQ(stats.idle_connections, config_.initial_size);
    EXPECT_EQ(stats.active_connections, 0);
}

// 测试配置设置和获取
TEST_F(ConnectionPoolTest, ConfigSetAndGet) {
    Pool::PoolConfig new_config;
    new_config.initial_size = 5;
    new_config.max_size = 50;
    new_config.min_size = 2;
    
    pool_->setConfig(new_config);
    
    auto retrieved_config = pool_->getConfig();
    EXPECT_EQ(retrieved_config.initial_size, 5);
    EXPECT_EQ(retrieved_config.max_size, 50);
    EXPECT_EQ(retrieved_config.min_size, 2);
}

// 测试重复获取和释放
TEST_F(ConnectionPoolTest, RepeatedAcquireRelease) {
    pool_->start();
    
    for (int i = 0; i < 10; ++i) {
        auto conn = pool_->acquire();
        ASSERT_NE(conn, nullptr);
        pool_->release(conn);
    }
    
    auto stats = pool_->getStats();
    EXPECT_EQ(stats.idle_connections, config_.initial_size);
}

// 测试无效连接的处理
TEST_F(ConnectionPoolTest, InvalidConnectionHandling) {
    pool_->start();
    
    auto conn = pool_->acquire();
    ASSERT_NE(conn, nullptr);
    
    // 模拟连接失效（通过释放后创建新的）
    pool_->release(conn);
    
    auto new_conn = pool_->acquire();
    ASSERT_NE(new_conn, nullptr);
    
    pool_->release(new_conn);
}

// ==================== SSLWrapper Tests (Linux only) ====================

#ifdef __linux__

class SSLWrapperTest : public ::testing::Test {
protected:
    void SetUp() override {
    }

    void TearDown() override {
    }
};

// 测试 SSLContext 创建
TEST_F(SSLWrapperTest, CreateSSLContext) {
    auto ctx = utils::SSLContext::create(TLS_method());
    EXPECT_TRUE(ctx.is_valid());
    EXPECT_NE(ctx.get(), nullptr);
}

// 测试 SSLContext 默认构造
TEST_F(SSLWrapperTest, DefaultConstructor) {
    utils::SSLContext ctx;
    EXPECT_FALSE(ctx.is_valid());
    EXPECT_EQ(ctx.get(), nullptr);
}

// 测试 SSLContext move 构造
TEST_F(SSLWrapperTest, SSLContextMoveConstructor) {
    auto ctx1 = utils::SSLContext::create(TLS_method());
    utils::SSLContext ctx2(std::move(ctx1));
    
    EXPECT_FALSE(ctx1.is_valid());
    EXPECT_TRUE(ctx2.is_valid());
}

// 测试 SSLContext move 赋值
TEST_F(SSLWrapperTest, SSLContextMoveAssignment) {
    auto ctx1 = utils::SSLContext::create(TLS_method());
    auto ctx2 = utils::SSLContext::create(TLS_method());
    
    ctx1 = std::move(ctx2);
    
    EXPECT_TRUE(ctx1.is_valid());
}

// 测试 SSLContext release
TEST_F(SSLWrapperTest, SSLContextRelease) {
    auto ctx = utils::SSLContext::create(TLS_method());
    SSL_CTX* raw_ctx = ctx.release();
    
    EXPECT_FALSE(ctx.is_valid());
    EXPECT_NE(raw_ctx, nullptr);
    
    SSL_CTX_free(raw_ctx);
}

// 测试 SSLContext reset
TEST_F(SSLWrapperTest, SSLContextReset) {
    auto ctx = utils::SSLContext::create(TLS_method());
    EXPECT_TRUE(ctx.is_valid());
    
    ctx.reset();
    EXPECT_FALSE(ctx.is_valid());
}

// 测试 SSLContext get()
TEST_F(SSLWrapperTest, SSLContextGet) {
    auto ctx = utils::SSLContext::create(TLS_method());
    EXPECT_NE(ctx.get(), nullptr);
}

// 测试 SSLContext get_ssl_error_string
TEST_F(SSLWrapperTest, GetSSLErrorString) {
    std::string error = utils::SSLContext::get_ssl_error_string();
    // 可能有错误也可能没有，取决于OpenSSL状态
    EXPECT_FALSE(error.empty());
}

// 测试 SSLSocket 创建
TEST_F(SSLWrapperTest, CreateSSLSocket) {
    auto ctx = utils::SSLContext::create(TLS_method());
    auto ssl = utils::SSLSocket::create(ctx.get());
    
    EXPECT_TRUE(ssl.is_valid());
    EXPECT_NE(ssl.get(), nullptr);
}

// 测试 SSLSocket 默认构造
TEST_F(SSLWrapperTest, SSLSocketDefaultConstructor) {
    utils::SSLSocket ssl;
    EXPECT_FALSE(ssl.is_valid());
    EXPECT_EQ(ssl.get(), nullptr);
}

// 测试 SSLSocket move 构造
TEST_F(SSLWrapperTest, SSLSocketMoveConstructor) {
    auto ctx = utils::SSLContext::create(TLS_method());
    auto ssl1 = utils::SSLSocket::create(ctx.get());
    utils::SSLSocket ssl2(std::move(ssl1));
    
    EXPECT_FALSE(ssl1.is_valid());
    EXPECT_TRUE(ssl2.is_valid());
}

// 测试 SSLSocket move 赋值
TEST_F(SSLWrapperTest, SSLSocketMoveAssignment) {
    auto ctx = utils::SSLContext::create(TLS_method());
    auto ssl1 = utils::SSLSocket::create(ctx.get());
    auto ssl2 = utils::SSLSocket::create(ctx.get());
    
    ssl1 = std::move(ssl2);
    
    EXPECT_TRUE(ssl1.is_valid());
}

// 测试 SSLSocket release
TEST_F(SSLWrapperTest, SSLSocketRelease) {
    auto ctx = utils::SSLContext::create(TLS_method());
    auto ssl = utils::SSLSocket::create(ctx.get());
    SSL* raw_ssl = ssl.release();
    
    EXPECT_FALSE(ssl.is_valid());
    EXPECT_NE(raw_ssl, nullptr);
    
    SSL_free(raw_ssl);
}

// 测试 SSLSocket reset
TEST_F(SSLWrapperTest, SSLSocketReset) {
    auto ctx = utils::SSLContext::create(TLS_method());
    auto ssl = utils::SSLSocket::create(ctx.get());
    EXPECT_TRUE(ssl.is_valid());
    
    ssl.reset();
    EXPECT_FALSE(ssl.is_valid());
}

// 测试 SSLSocket shutdown
TEST_F(SSLWrapperTest, SSLSocketShutdown) {
    auto ctx = utils::SSLContext::create(TLS_method());
    auto ssl = utils::SSLSocket::create(ctx.get());
    
    // 不会崩溃
    ssl.shutdown();
}

// 测试 SSLSocket get_error_string
TEST_F(SSLWrapperTest, SSLSocketGetErrorString) {
    auto ctx = utils::SSLContext::create(TLS_method());
    auto ssl = utils::SSLSocket::create(ctx.get());
    
    std::string error = ssl.get_error_string();
    EXPECT_FALSE(error.empty());
}

// 测试异常：空指针构造
TEST_F(SSLWrapperTest, SSLContextNullPointer) {
    EXPECT_THROW(utils::SSLContext ctx(nullptr), std::invalid_argument);
}

TEST_F(SSLWrapperTest, SSLSocketNullPointer) {
    EXPECT_THROW(utils::SSLSocket ssl(nullptr), std::invalid_argument);
}

// 测试异常：release 空指针
TEST_F(SSLWrapperTest, SSLContextReleaseNull) {
    utils::SSLContext ctx;
    EXPECT_THROW(ctx.release(), std::logic_error);
}

TEST_F(SSLWrapperTest, SSLSocketReleaseNull) {
    utils::SSLSocket ssl;
    EXPECT_THROW(ssl.release(), std::logic_error);
}

#else // 非 Linux 平台测试

class SSLWrapperNonLinuxTest : public ::testing::Test {
protected:
    void SetUp() override {
    }

    void TearDown() override {
    }
};

TEST_F(SSLWrapperNonLinuxTest, SSLContextNotSupported) {
    EXPECT_THROW(utils::SSLContext::create(nullptr), std::runtime_error);
}

TEST_F(SSLWrapperNonLinuxTest, SSLSocketNotSupported) {
    EXPECT_THROW(utils::SSLSocket::create(nullptr), std::runtime_error);
}

#endif // __linux__

} // namespace test
} // namespace sqlcc
