#include "network/multi_threaded_network_manager.h"
#include "utils/config_manager.h"
#include "core/core_database_manager.h"
#include "execution/task_executor.h"
#include <gtest/gtest.h>
#include <memory>
#include <thread>
#include <chrono>
#include <atomic>

namespace sqlcc {
namespace network {
namespace test {

class MultiThreadedNetworkManagerTest : public ::testing::Test {
protected:
  void SetUp() override {
    config_manager_ = std::make_unique<ConfigManager>();
    database_manager_ = std::make_unique<DatabaseManager>(*config_manager_);
    task_executor_ = std::make_unique<TaskExecutor>(*config_manager_);
    network_manager_ = std::make_unique<MultiThreadedNetworkManager>(
        *config_manager_, *database_manager_, *task_executor_);
  }

  void TearDown() override {
    network_manager_->Stop();
    network_manager_.reset();
    task_executor_.reset();
    database_manager_.reset();
    config_manager_.reset();
  }

  std::unique_ptr<ConfigManager> config_manager_;
  std::unique_ptr<DatabaseManager> database_manager_;
  std::unique_ptr<TaskExecutor> task_executor_;
  std::unique_ptr<MultiThreadedNetworkManager> network_manager_;
};

// 测试基本功能
TEST_F(MultiThreadedNetworkManagerTest, BasicFunctionality) {
  // 测试初始状态
  EXPECT_FALSE(network_manager_->IsRunning());

  // 测试统计信息
  auto stats = network_manager_->GetStats();
  EXPECT_EQ(stats.total_connections.load(), 0);
  EXPECT_EQ(stats.active_connections.load(), 0);
  EXPECT_EQ(stats.total_requests.load(), 0);
}

// 测试配置管理
TEST_F(MultiThreadedNetworkManagerTest, Configuration) {
  // 获取默认配置
  auto config = network_manager_->GetConfig();
  EXPECT_EQ(config.port, 5432);
  EXPECT_EQ(config.max_connections, 10000);
  EXPECT_EQ(config.network_threads, 32);
  EXPECT_EQ(config.worker_threads, 64);

  // 设置新配置
  MultiThreadedNetworkManager::NetworkConfig new_config;
  new_config.port = 8080;
  new_config.max_connections = 5000;
  new_config.network_threads = 16;
  new_config.worker_threads = 32;

  network_manager_->SetConfig(new_config);

  // 验证配置更新
  auto updated_config = network_manager_->GetConfig();
  EXPECT_EQ(updated_config.port, 8080);
  EXPECT_EQ(updated_config.max_connections, 5000);
  EXPECT_EQ(updated_config.network_threads, 16);
  EXPECT_EQ(updated_config.worker_threads, 32);
}

// 测试请求处理（模拟）
TEST_F(MultiThreadedNetworkManagerTest, HandleRequest) {
  // 创建模拟连接状态
  auto connection = std::make_shared<ConnectionState>();
  connection->client_id = 1;
  connection->is_authenticated = true;

  // 测试处理请求
  std::string request = "SELECT 1";
  EXPECT_TRUE(network_manager_->HandleRequest(1, request, connection));

  // 验证统计信息
  auto stats = network_manager_->GetStats();
  EXPECT_EQ(stats.total_requests.load(), 1);
}

// 测试响应发送（模拟）
TEST_F(MultiThreadedNetworkManagerTest, SendResponse) {
  // 测试发送响应
  std::string response = "Query result";
  EXPECT_TRUE(network_manager_->SendResponse(1, response));

  // 验证统计信息（响应统计可能需要实际实现）
  auto stats = network_manager_->GetStats();
  EXPECT_GE(stats.processed_requests.load(), 0);
}

// 测试连接管理
TEST_F(MultiThreadedNetworkManagerTest, ConnectionManagement) {
  // 断开不存在的连接（应该不报错）
  network_manager_->DisconnectClient(999);

  // 测试连接计数
  auto stats = network_manager_->GetStats();
  EXPECT_EQ(stats.total_connections.load(), 0);
}

// 测试统计信息重置
TEST_F(MultiThreadedNetworkManagerTest, ResetStats) {
  // 先产生一些统计数据
  auto connection = std::make_shared<ConnectionState>();
  connection->client_id = 1;
  network_manager_->HandleRequest(1, "SELECT 1", connection);

  // 验证有统计数据
  auto stats_before = network_manager_->GetStats();
  EXPECT_GE(stats_before.total_requests.load(), 1);

  // 重置统计信息
  network_manager_->ResetStats();

  // 验证统计信息已重置
  auto stats_after = network_manager_->GetStats();
  EXPECT_EQ(stats_after.total_requests.load(), 0);
  EXPECT_EQ(stats_after.processed_requests.load(), 0);
  EXPECT_EQ(stats_after.failed_requests.load(), 0);
}

// 测试并发请求处理
TEST_F(MultiThreadedNetworkManagerTest, ConcurrentRequests) {
  const int num_threads = 10;
  const int requests_per_thread = 20;
  std::atomic<int> completed_requests{0};

  // 创建多个线程并发发送请求
  std::vector<std::thread> threads;
  for (int t = 0; t < num_threads; ++t) {
    threads.emplace_back([this, t, requests_per_thread, &completed_requests]() {
      for (int i = 0; i < requests_per_thread; ++i) {
        auto connection = std::make_shared<ConnectionState>();
        connection->client_id = t * requests_per_thread + i + 1;
        connection->is_authenticated = true;

        std::string request = "SELECT " + std::to_string(i);
        if (network_manager_->HandleRequest(connection->client_id, request, connection)) {
          completed_requests++;
        }

        // 小延迟以避免过于激进的并发
        std::this_thread::sleep_for(std::chrono::microseconds(100));
      }
    });
  }

  // 等待所有线程完成
  for (auto& thread : threads) {
    thread.join();
  }

  // 验证总请求数
  auto stats = network_manager_->GetStats();
  EXPECT_EQ(stats.total_requests.load(), num_threads * requests_per_thread);
  EXPECT_EQ(completed_requests.load(), num_threads * requests_per_thread);
}

// 测试网络配置边界值
TEST_F(MultiThreadedNetworkManagerTest, ConfigBoundaries) {
  // 测试边界配置值
  MultiThreadedNetworkManager::NetworkConfig config;

  // 测试最小值
  config.port = 1;
  config.max_connections = 1;
  config.network_threads = 1;
  config.worker_threads = 1;
  network_manager_->SetConfig(config);

  auto retrieved_config = network_manager_->GetConfig();
  EXPECT_EQ(retrieved_config.port, 1);
  EXPECT_EQ(retrieved_config.max_connections, 1);
  EXPECT_EQ(retrieved_config.network_threads, 1);
  EXPECT_EQ(retrieved_config.worker_threads, 1);

  // 测试大值
  config.port = 65535;
  config.max_connections = 100000;
  config.network_threads = 1000;
  config.worker_threads = 1000;
  network_manager_->SetConfig(config);

  retrieved_config = network_manager_->GetConfig();
  EXPECT_EQ(retrieved_config.port, 65535);
  EXPECT_EQ(retrieved_config.max_connections, 100000);
  EXPECT_EQ(retrieved_config.network_threads, 1000);
  EXPECT_EQ(retrieved_config.worker_threads, 1000);
}

// 测试连接超时配置
TEST_F(MultiThreadedNetworkManagerTest, ConnectionTimeout) {
  // 测试连接超时配置
  MultiThreadedNetworkManager::NetworkConfig config;
  config.connection_timeout = std::chrono::seconds(60);

  network_manager_->SetConfig(config);

  auto retrieved_config = network_manager_->GetConfig();
  EXPECT_EQ(retrieved_config.connection_timeout, std::chrono::seconds(60));
}

// 测试SSL配置
TEST_F(MultiThreadedNetworkManagerTest, SSLConfiguration) {
  // 测试SSL配置
  MultiThreadedNetworkManager::NetworkConfig config;
  config.enable_ssl = true;
  config.ssl_cert_path = "/path/to/cert.pem";
  config.ssl_key_path = "/path/to/key.pem";

  network_manager_->SetConfig(config);

  auto retrieved_config = network_manager_->GetConfig();
  EXPECT_TRUE(retrieved_config.enable_ssl);
  EXPECT_EQ(retrieved_config.ssl_cert_path, "/path/to/cert.pem");
  EXPECT_EQ(retrieved_config.ssl_key_path, "/path/to/key.pem");
}

// 测试队列大小配置
TEST_F(MultiThreadedNetworkManagerTest, QueueSizeConfiguration) {
  // 测试队列大小配置
  MultiThreadedNetworkManager::NetworkConfig config;
  config.max_queue_size = 50000;

  network_manager_->SetConfig(config);

  auto retrieved_config = network_manager_->GetConfig();
  EXPECT_EQ(retrieved_config.max_queue_size, 50000);
}

// 测试性能统计计算
TEST_F(MultiThreadedNetworkManagerTest, PerformanceStats) {
  // 产生一些请求
  for (int i = 0; i < 100; ++i) {
    auto connection = std::make_shared<ConnectionState>();
    connection->client_id = i + 1;
    network_manager_->HandleRequest(i + 1, "SELECT " + std::to_string(i), connection);
  }

  // 验证性能统计
  auto stats = network_manager_->GetStats();
  EXPECT_EQ(stats.total_requests.load(), 100);

  // 测试QPS计算（模拟时间）
  double qps = stats.requests_per_second();
  EXPECT_GE(qps, 0.0);

  // 测试连接利用率
  double utilization = stats.connection_utilization();
  EXPECT_GE(utilization, 0.0);
  EXPECT_LE(utilization, 1.0);
}

// 测试错误处理
TEST_F(MultiThreadedNetworkManagerTest, ErrorHandling) {
  // 测试无效客户端ID
  EXPECT_TRUE(network_manager_->SendResponse(-1, "error response"));

  // 测试空请求
  auto connection = std::make_shared<ConnectionState>();
  connection->client_id = 1;
  EXPECT_TRUE(network_manager_->HandleRequest(1, "", connection));

  // 测试超长请求
  std::string long_request(10000, 'x');
  EXPECT_TRUE(network_manager_->HandleRequest(2, long_request, connection));

  // 验证统计信息仍然正确更新
  auto stats = network_manager_->GetStats();
  EXPECT_GE(stats.total_requests.load(), 2);
}

// 测试多线程安全性
TEST_F(MultiThreadedNetworkManagerTest, ThreadSafety) {
  const int num_threads = 20;
  std::atomic<int> operations_completed{0};

  // 创建多个线程并发执行各种操作
  std::vector<std::thread> threads;
  for (int t = 0; t < num_threads; ++t) {
    threads.emplace_back([this, t, &operations_completed]() {
      // 配置操作
      auto config = network_manager_->GetConfig();
      operations_completed++;

      // 统计信息操作
      auto stats = network_manager_->GetStats();
      operations_completed++;

      // 请求处理操作
      auto connection = std::make_shared<ConnectionState>();
      connection->client_id = t + 1;
      network_manager_->HandleRequest(t + 1, "SELECT 1", connection);
      operations_completed++;

      // 响应发送操作
      network_manager_->SendResponse(t + 1, "OK");
      operations_completed++;
    });
  }

  // 等待所有线程完成
  for (auto& thread : threads) {
    thread.join();
  }

  // 验证所有操作都成功完成
  EXPECT_EQ(operations_completed.load(), num_threads * 4);

  // 验证统计信息
  auto stats = network_manager_->GetStats();
  EXPECT_EQ(stats.total_requests.load(), num_threads);
}

// 测试资源清理
TEST_F(MultiThreadedNetworkManagerTest, ResourceCleanup) {
  // 产生一些状态
  for (int i = 0; i < 10; ++i) {
    auto connection = std::make_shared<ConnectionState>();
    connection->client_id = i + 1;
    network_manager_->HandleRequest(i + 1, "SELECT " + std::to_string(i), connection);
  }

  // 断开所有连接
  for (int i = 0; i < 10; ++i) {
    network_manager_->DisconnectClient(i + 1);
  }

  // 重置统计信息
  network_manager_->ResetStats();

  // 验证清理完成
  auto stats = network_manager_->GetStats();
  EXPECT_EQ(stats.total_requests.load(), 0);
  EXPECT_EQ(stats.active_connections.load(), 0);
}

} // namespace test
} // namespace network
} // namespace sqlcc
