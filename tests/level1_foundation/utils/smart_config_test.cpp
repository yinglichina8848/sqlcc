#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <future>
#include <thread>
#include <atomic>

#include "src/utils/smart_config_manager.h"
#include "src/utils/config_lifecycle.h"
#include "src/utils/config_snapshot.h"

namespace sqlcc {
namespace test {

// ==================== SmartConfigManager Tests ====================

class SmartConfigManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 确保单例是干净的
        if (SmartConfigManager::GetInstance()) {
            SmartConfigManager::DestroyInstance();
        }
    }

    void TearDown() override {
        // 清理单例
        SmartConfigManager::DestroyInstance();
    }
};

// 测试单例模式
TEST_F(SmartConfigManagerTest, SingletonPattern) {
    SmartConfigManager* instance1 = SmartConfigManager::GetInstance();
    SmartConfigManager* instance2 = SmartConfigManager::GetInstance();
    
    EXPECT_EQ(instance1, instance2);
    
    // 清理
    SmartConfigManager::DestroyInstance();
}

// 测试初始化和关闭
TEST_F(SmartConfigManagerTest, InitializeAndShutdown) {
    SmartConfigManager* manager = SmartConfigManager::GetInstance();
    
    // 初始化
    bool init_result = manager->Initialize("");
    EXPECT_TRUE(init_result);
    
    // 关闭
    bool shutdown_result = manager->Shutdown();
    EXPECT_TRUE(shutdown_result);
    
    SmartConfigManager::DestroyInstance();
}

// 测试重复初始化
TEST_F(SmartConfigManagerTest, DoubleInitialize) {
    SmartConfigManager* manager = SmartConfigManager::GetInstance();
    
    bool first_init = manager->Initialize("");
    EXPECT_TRUE(first_init);
    
    // 重复初始化应该返回 false
    bool second_init = manager->Initialize("");
    EXPECT_FALSE(second_init);
    
    manager->Shutdown();
    SmartConfigManager::DestroyInstance();
}

// 测试获取版本ID（空配置）
TEST_F(SmartConfigManagerTest, GetCurrentVersionId) {
    SmartConfigManager* manager = SmartConfigManager::GetInstance();
    manager->Initialize("");
    
    std::string version_id = manager->GetCurrentVersionId();
    // 初始化后会创建默认快照
    EXPECT_FALSE(version_id.empty());
    
    manager->Shutdown();
    SmartConfigManager::DestroyInstance();
}

// 测试获取统计信息
TEST_F(SmartConfigManagerTest, GetStatistics) {
    SmartConfigManager* manager = SmartConfigManager::GetInstance();
    manager->Initialize("");
    
    std::string stats = manager->GetStatistics();
    
    // 统计信息应该包含关键指标
    EXPECT_TRUE(stats.find("SmartConfigManager Statistics:") != std::string::npos);
    EXPECT_TRUE(stats.find("Initialized:") != std::string::npos);
    EXPECT_TRUE(stats.find("Config Reads:") != std::string::npos);
    
    manager->Shutdown();
    SmartConfigManager::DestroyInstance();
}

// 测试加密密钥设置和获取
TEST_F(SmartConfigManagerTest, EncryptionKey) {
    SmartConfigManager* manager = SmartConfigManager::GetInstance();
    manager->Initialize("");
    
    std::string test_key = "test_encryption_key_12345";
    
    manager->SetEncryptionKey(test_key);
    std::string retrieved_key = manager->GetEncryptionKey();
    
    EXPECT_EQ(retrieved_key, test_key);
    
    manager->Shutdown();
    SmartConfigManager::DestroyInstance();
}

// 测试热更新启用和停止
TEST_F(SmartConfigManagerTest, HotReloadEnableDisable) {
    SmartConfigManager* manager = SmartConfigManager::GetInstance();
    manager->Initialize("");
    
    // 启用热更新
    bool enable_result = manager->EnableHotReload(std::chrono::milliseconds(1000));
    EXPECT_TRUE(enable_result);
    
    // 停止热更新
    bool disable_result = manager->StopHotReload();
    EXPECT_TRUE(disable_result);
    
    manager->Shutdown();
    SmartConfigManager::DestroyInstance();
}

// 测试重复启用热更新
TEST_F(SmartConfigManagerTest, DoubleEnableHotReload) {
    SmartConfigManager* manager = SmartConfigManager::GetInstance();
    manager->Initialize("");
    
    bool first_enable = manager->EnableHotReload(std::chrono::milliseconds(1000));
    EXPECT_TRUE(first_enable);
    
    // 重复启用应该返回 false
    bool second_enable = manager->EnableHotReload(std::chrono::milliseconds(2000));
    EXPECT_FALSE(second_enable);
    
    manager->StopHotReload();
    manager->Shutdown();
    SmartConfigManager::DestroyInstance();
}

// 测试异步配置更新
TEST_F(SmartConfigManagerTest, AsyncBatchUpdate) {
    SmartConfigManager* manager = SmartConfigManager::GetInstance();
    manager->Initialize("");
    
    std::unordered_map<std::string, ConfigValue> configs;
    configs["test.key1"] = std::string("value1");
    configs["test.key2"] = 42;
    configs["test.key3"] = true;
    
    auto future = manager->BatchUpdateConfigsAsync(configs);
    
    // 等待异步操作完成
    bool result = future.get();
    EXPECT_TRUE(result);
    
    manager->Shutdown();
    SmartConfigManager::DestroyInstance();
}

// 测试并发访问
TEST_F(SmartConfigManagerTest, ConcurrentAccess) {
    SmartConfigManager* manager = SmartConfigManager::GetInstance();
    manager->Initialize("");
    
    std::atomic<bool> done{false};
    std::vector<std::thread> threads;
    
    // 多个线程同时获取统计信息
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back([manager, &done]() {
            while (!done.load()) {
                manager->GetStatistics();
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        });
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    done.store(true);
    
    for (auto& t : threads) {
        t.join();
    }
    
    // 不应该崩溃
    SUCCEED();
    
    manager->Shutdown();
    SmartConfigManager::DestroyInstance();
}

// 测试空配置获取
TEST_F(SmartConfigManagerTest, GetConfigWithDefaults) {
    SmartConfigManager* manager = SmartConfigManager::GetInstance();
    manager->Initialize("");
    
    // 获取不存在的配置应该返回默认值
    std::string str_val = manager->GetStringConfig("nonexistent", "default");
    EXPECT_EQ(str_val, "default");
    
    int int_val = manager->GetIntConfig("nonexistent", 123);
    EXPECT_EQ(int_val, 123);
    
    bool bool_val = manager->GetBoolConfig("nonexistent", true);
    EXPECT_TRUE(bool_val);
    
    double double_val = manager->GetDoubleConfig("nonexistent", 3.14);
    EXPECT_DOUBLE_EQ(double_val, 3.14);
    
    manager->Shutdown();
    SmartConfigManager::DestroyInstance();
}

// ==================== ConfigLifecycleManager Tests ====================

class ConfigLifecycleManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
    }

    void TearDown() override {
    }
};

TEST_F(ConfigLifecycleManagerTest, InitializeAndShutdown) {
    ConfigLifecycleManager manager;
    
    bool init_result = manager.Initialize("");
    EXPECT_TRUE(init_result);
    
    EXPECT_TRUE(manager.IsReady());
    EXPECT_EQ(manager.GetState(), ConfigLifecycleState::READY);
    
    bool shutdown_result = manager.Shutdown();
    EXPECT_TRUE(shutdown_result);
    
    EXPECT_EQ(manager.GetState(), ConfigLifecycleState::SHUTDOWN);
}

TEST_F(ConfigLifecycleManagerTest, GetCurrentSnapshot) {
    ConfigLifecycleManager manager;
    manager.Initialize("");
    
    auto snapshot = manager.GetCurrentSnapshot();
    EXPECT_NE(snapshot, nullptr);
    
    manager.Shutdown();
}

TEST_F(ConfigLifecycleManagerTest, GetStatistics) {
    ConfigLifecycleManager manager;
    manager.Initialize("");
    
    std::string stats = manager.GetStatistics();
    
    EXPECT_TRUE(stats.find("Config Lifecycle Statistics:") != std::string::npos);
    
    manager.Shutdown();
}

TEST_F(ConfigLifecycleManagerTest, Callbacks) {
    ConfigLifecycleManager manager;

    bool init_called = false;
    bool shutdown_called = false;
    bool config_change_called = false;

    manager.SetInitializeCallback([&init_called]() {
        init_called = true;
    });

    manager.SetShutdownCallback([&shutdown_called]() {
        shutdown_called = true;
    });

    manager.SetConfigChangeCallback([&config_change_called](const std::string&) {
        config_change_called = true;
    });

    manager.Initialize("");
    EXPECT_TRUE(init_called);

    manager.Shutdown();
    EXPECT_TRUE(shutdown_called);
}

// 测试重复关闭
TEST_F(ConfigLifecycleManagerTest, DoubleShutdown) {
    ConfigLifecycleManager manager;
    manager.Initialize("");

    bool first_shutdown = manager.Shutdown();
    EXPECT_TRUE(first_shutdown);

    // 重复关闭应该仍然返回 true（幂等操作）
    bool second_shutdown = manager.Shutdown();
    EXPECT_TRUE(second_shutdown);
}

// 测试未初始化时获取快照失败
TEST_F(ConfigLifecycleManagerTest, GetSnapshotNotReady) {
    ConfigLifecycleManager manager;

    // 未初始化时应该抛出异常
    EXPECT_THROW(manager.GetCurrentSnapshot(), ConfigLifecycleException);
}

// 测试未初始化时更新快照失败
TEST_F(ConfigLifecycleManagerTest, UpdateSnapshotNotReady) {
    ConfigLifecycleManager manager;

    auto empty_snapshot = ConfigSnapshotFactory::CreateEmptySnapshot("v1");
    bool result = manager.UpdateSnapshot(empty_snapshot);
    EXPECT_FALSE(result);
}

// 测试更新快照
TEST_F(ConfigLifecycleManagerTest, UpdateSnapshot) {
    ConfigLifecycleManager manager;
    manager.Initialize("");

    // 创建一个新的配置快照
    std::unordered_map<std::string, ConfigValue> config_data;
    config_data["test.key1"] = std::string("value1");
    config_data["test.key2"] = 42;
    config_data["test.key3"] = true;

    auto new_snapshot = ConfigSnapshotFactory::CreateSnapshot(config_data, "v2", "Test config");
    bool result = manager.UpdateSnapshot(new_snapshot);
    EXPECT_TRUE(result);

    manager.Shutdown();
}

// 测试回滚版本
TEST_F(ConfigLifecycleManagerTest, RollbackToVersion) {
    ConfigLifecycleManager manager;
    manager.Initialize("");

    // 创建第一个快照
    std::unordered_map<std::string, ConfigValue> config_v1;
    config_v1["version"] = 1;
    auto snapshot_v1 = ConfigSnapshotFactory::CreateSnapshot(config_v1, "v1", "Version 1");
    manager.UpdateSnapshot(snapshot_v1);

    // 创建第二个快照
    std::unordered_map<std::string, ConfigValue> config_v2;
    config_v2["version"] = 2;
    auto snapshot_v2 = ConfigSnapshotFactory::CreateSnapshot(config_v2, "v2", "Version 2");
    manager.UpdateSnapshot(snapshot_v2);

    // 回滚到 v1
    bool rollback_result = manager.RollbackToVersion("v1");
    EXPECT_TRUE(rollback_result);

    manager.Shutdown();
}

// 测试未初始化时回滚失败
TEST_F(ConfigLifecycleManagerTest, RollbackNotReady) {
    ConfigLifecycleManager manager;

    bool result = manager.RollbackToVersion("v1");
    EXPECT_FALSE(result);
}

// 测试获取快照管理器
TEST_F(ConfigLifecycleManagerTest, GetSnapshotManager) {
    ConfigLifecycleManager manager;
    manager.Initialize("");

    auto& snapshot_manager = manager.GetSnapshotManager();
    EXPECT_NE(snapshot_manager.GetCurrentSnapshot(), nullptr);

    manager.Shutdown();
}

// ==================== ConfigRAIIAccessor Tests ====================

class ConfigRAIIAccessorTest : public ::testing::Test {
protected:
    void SetUp() override {
    }

    void TearDown() override {
    }
};

TEST_F(ConfigRAIIAccessorTest, BasicAccess) {
    ConfigLifecycleManager manager;
    manager.Initialize("");

    // 添加一些配置
    std::unordered_map<std::string, ConfigValue> config_data;
    config_data["database.host"] = std::string("localhost");
    config_data["database.port"] = 5432;
    config_data["database.timeout"] = 30.5;
    config_data["database.ssl"] = true;

    auto snapshot = ConfigSnapshotFactory::CreateSnapshot(config_data, "v1", "Test");
    manager.UpdateSnapshot(snapshot);

    // 创建访问器
    ConfigRAIIAccessor accessor(&manager, "test_accessor");

    EXPECT_TRUE(accessor.IsValid());
    EXPECT_EQ(accessor.GetAccessorId(), "test_accessor");
    EXPECT_EQ(accessor.GetCurrentVersionId(), "v1");

    // 获取值
    ConfigValue value;
    EXPECT_TRUE(accessor.GetValue("database.host", value));
    EXPECT_EQ(std::get<std::string>(value), "localhost");

    EXPECT_TRUE(accessor.GetValue("database.port", value));
    EXPECT_EQ(std::get<int>(value), 5432);

    manager.Shutdown();
}

TEST_F(ConfigRAIIAccessorTest, GetValueWithDefault) {
    ConfigLifecycleManager manager;
    manager.Initialize("");

    ConfigRAIIAccessor accessor(&manager);

    // 获取不存在的键，应该返回默认值
    // 使用空访问器验证
    EXPECT_FALSE(accessor.HasKey("nonexistent"));

    auto keys = accessor.GetAllKeys();
    EXPECT_TRUE(keys.empty());  // 默认快照应该没有配置

    manager.Shutdown();
}

TEST_F(ConfigRAIIAccessorTest, HasKey) {
    ConfigLifecycleManager manager;
    manager.Initialize("");

    std::unordered_map<std::string, ConfigValue> config_data;
    config_data["existing.key"] = std::string("value");
    auto snapshot = ConfigSnapshotFactory::CreateSnapshot(config_data, "v1", "Test");
    manager.UpdateSnapshot(snapshot);

    ConfigRAIIAccessor accessor(&manager);

    EXPECT_TRUE(accessor.HasKey("existing.key"));
    EXPECT_FALSE(accessor.HasKey("nonexistent.key"));

    manager.Shutdown();
}

TEST_F(ConfigRAIIAccessorTest, GetAllKeys) {
    ConfigLifecycleManager manager;
    manager.Initialize("");

    std::unordered_map<std::string, ConfigValue> config_data;
    config_data["key1"] = 1;
    config_data["key2"] = 2;
    config_data["key3"] = 3;
    auto snapshot = ConfigSnapshotFactory::CreateSnapshot(config_data, "v1", "Test");
    manager.UpdateSnapshot(snapshot);

    ConfigRAIIAccessor accessor(&manager);

    auto keys = accessor.GetAllKeys();
    EXPECT_EQ(keys.size(), 3);

    manager.Shutdown();
}

// 测试无效访问器
TEST_F(ConfigRAIIAccessorTest, InvalidAccessor) {
    ConfigLifecycleManager manager;
    // 不初始化，直接创建访问器应该抛出异常
    EXPECT_THROW(ConfigRAIIAccessor accessor(&manager), ConfigLifecycleException);
}

// 测试移动语义
TEST_F(ConfigRAIIAccessorTest, MoveSemantics) {
    ConfigLifecycleManager manager;
    manager.Initialize("");

    ConfigRAIIAccessor accessor1(&manager, "original");
    ConfigRAIIAccessor accessor2(std::move(accessor1));

    EXPECT_EQ(accessor2.GetAccessorId(), "original");
    EXPECT_TRUE(accessor2.IsValid());

    manager.Shutdown();
}

// ==================== ConfigSnapshot Tests ====================

class ConfigSnapshotTest : public ::testing::Test {
protected:
    void SetUp() override {
    }

    void TearDown() override {
    }
};

TEST_F(ConfigSnapshotTest, CreateAndGetValue) {
    std::unordered_map<std::string, ConfigValue> config_data;
    config_data["key1"] = std::string("value1");
    config_data["key2"] = 42;
    config_data["key3"] = 3.14;
    config_data["key4"] = true;

    auto snapshot = ConfigSnapshotFactory::CreateSnapshot(config_data, "v1", "Test");

    ConfigValue value;
    EXPECT_TRUE(snapshot->GetValue("key1", value));
    EXPECT_EQ(std::get<std::string>(value), "value1");

    EXPECT_TRUE(snapshot->GetValue("key2", value));
    EXPECT_EQ(std::get<int>(value), 42);

    EXPECT_TRUE(snapshot->GetValue("key3", value));
    EXPECT_DOUBLE_EQ(std::get<double>(value), 3.14);

    EXPECT_TRUE(snapshot->GetValue("key4", value));
    EXPECT_TRUE(std::get<bool>(value));
}

TEST_F(ConfigSnapshotTest, GetConfigCount) {
    std::unordered_map<std::string, ConfigValue> config_data;
    config_data["key1"] = 1;
    config_data["key2"] = 2;
    config_data["key3"] = 3;

    auto snapshot = ConfigSnapshotFactory::CreateSnapshot(config_data, "v1", "Test");
    EXPECT_EQ(snapshot->GetConfigCount(), 3);
}

TEST_F(ConfigSnapshotTest, GetMetadata) {
    auto snapshot = ConfigSnapshotFactory::CreateEmptySnapshot("v1.0.0", "Test version");

    const auto& metadata = snapshot->GetMetadata();
    EXPECT_EQ(metadata.version_id, "v1.0.0");
    EXPECT_EQ(metadata.description, "Test version");
    EXPECT_EQ(metadata.config_count, 0);
}

TEST_F(ConfigSnapshotTest, ValidateIntegrity) {
    std::unordered_map<std::string, ConfigValue> config_data;
    config_data["key"] = std::string("value");

    auto snapshot = ConfigSnapshotFactory::CreateSnapshot(config_data, "v1", "Test");
    EXPECT_TRUE(snapshot->ValidateIntegrity());
}

TEST_F(ConfigSnapshotTest, Clone) {
    std::unordered_map<std::string, ConfigValue> config_data;
    config_data["key"] = std::string("value");

    auto snapshot = ConfigSnapshotFactory::CreateSnapshot(config_data, "v1", "Test");
    auto cloned = snapshot->Clone();

    EXPECT_NE(snapshot.get(), cloned.get());
    EXPECT_EQ(snapshot->GetMetadata().version_id, cloned->GetMetadata().version_id);
    EXPECT_EQ(snapshot->GetConfigCount(), cloned->GetConfigCount());
}

TEST_F(ConfigSnapshotTest, Equals) {
    std::unordered_map<std::string, ConfigValue> config_data;
    config_data["key"] = std::string("value");

    auto snapshot1 = ConfigSnapshotFactory::CreateSnapshot(config_data, "v1", "Test");
    auto snapshot2 = ConfigSnapshotFactory::CreateSnapshot(config_data, "v1", "Test");

    // 不同对象，即使内容相同也不相等（版本ID相同，但对象不同）
    // 但同一个对象的clone应该等于原对象
    auto cloned = snapshot1->Clone();
    EXPECT_TRUE(snapshot1->Equals(*cloned));
}

TEST_F(ConfigSnapshotTest, GetKeysWithPrefix) {
    std::unordered_map<std::string, ConfigValue> config_data;
    config_data["database.host"] = std::string("localhost");
    config_data["database.port"] = 5432;
    config_data["app.name"] = std::string("myapp");
    config_data["app.version"] = std::string("1.0.0");

    auto snapshot = ConfigSnapshotFactory::CreateSnapshot(config_data, "v1", "Test");

    auto db_keys = snapshot->GetKeysWithPrefix("database.");
    EXPECT_EQ(db_keys.size(), 2);

    auto app_keys = snapshot->GetKeysWithPrefix("app.");
    EXPECT_EQ(app_keys.size(), 2);
}

TEST_F(ConfigSnapshotTest, GetAccessCount) {
    std::unordered_map<std::string, ConfigValue> config_data;
    config_data["key"] = std::string("value");

    auto snapshot = ConfigSnapshotFactory::CreateSnapshot(config_data, "v1", "Test");

    // 访问几次
    ConfigValue value;
    snapshot->GetValue("key", value);
    snapshot->HasKey("key");
    snapshot->GetAllKeys();

    // 访问计数应该增加
    EXPECT_GE(snapshot->GetAccessCount(), 3);
}

// ==================== ConfigSnapshotManager Tests ====================

class ConfigSnapshotManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
    }

    void TearDown() override {
    }
};

TEST_F(ConfigSnapshotManagerTest, AddAndGetSnapshot) {
    ConfigSnapshotManager manager;

    auto snapshot = ConfigSnapshotFactory::CreateEmptySnapshot("v1", "Test");
    bool result = manager.AddSnapshot(snapshot);
    EXPECT_TRUE(result);

    auto retrieved = manager.GetSnapshot("v1");
    EXPECT_NE(retrieved, nullptr);
    EXPECT_EQ(retrieved->GetMetadata().version_id, "v1");
}

TEST_F(ConfigSnapshotManagerTest, GetCurrentSnapshot) {
    ConfigSnapshotManager manager;

    auto snapshot_v1 = ConfigSnapshotFactory::CreateEmptySnapshot("v1", "Version 1");
    auto snapshot_v2 = ConfigSnapshotFactory::CreateEmptySnapshot("v2", "Version 2");

    manager.AddSnapshot(snapshot_v1);
    manager.AddSnapshot(snapshot_v2);

    auto current = manager.GetCurrentSnapshot();
    EXPECT_EQ(current->GetMetadata().version_id, "v2");
}

TEST_F(ConfigSnapshotManagerTest, GetCurrentVersionId) {
    ConfigSnapshotManager manager;

    auto snapshot = ConfigSnapshotFactory::CreateEmptySnapshot("test_version", "Test");
    manager.AddSnapshot(snapshot);

    EXPECT_EQ(manager.GetCurrentVersionId(), "test_version");
}

TEST_F(ConfigSnapshotManagerTest, RemoveSnapshot) {
    ConfigSnapshotManager manager;

    auto snapshot_v1 = ConfigSnapshotFactory::CreateEmptySnapshot("v1", "V1");
    auto snapshot_v2 = ConfigSnapshotFactory::CreateEmptySnapshot("v2", "V2");

    manager.AddSnapshot(snapshot_v1);
    manager.AddSnapshot(snapshot_v2);

    // 不能删除当前版本
    bool remove_current = manager.RemoveSnapshot("v2");
    EXPECT_FALSE(remove_current);

    // 可以删除旧版本
    bool remove_old = manager.RemoveSnapshot("v1");
    EXPECT_TRUE(remove_old);
}

TEST_F(ConfigSnapshotManagerTest, GetAllVersionIds) {
    ConfigSnapshotManager manager;

    manager.AddSnapshot(ConfigSnapshotFactory::CreateEmptySnapshot("v1", "V1"));
    manager.AddSnapshot(ConfigSnapshotFactory::CreateEmptySnapshot("v2", "V2"));
    manager.AddSnapshot(ConfigSnapshotFactory::CreateEmptySnapshot("v3", "V3"));

    auto versions = manager.GetAllVersionIds();
    EXPECT_EQ(versions.size(), 3);
}

TEST_F(ConfigSnapshotManagerTest, GetSnapshotCount) {
    ConfigSnapshotManager manager;

    EXPECT_EQ(manager.GetSnapshotCount(), 0);

    manager.AddSnapshot(ConfigSnapshotFactory::CreateEmptySnapshot("v1", "V1"));
    EXPECT_EQ(manager.GetSnapshotCount(), 1);

    manager.AddSnapshot(ConfigSnapshotFactory::CreateEmptySnapshot("v2", "V2"));
    EXPECT_EQ(manager.GetSnapshotCount(), 2);
}

TEST_F(ConfigSnapshotManagerTest, RollbackToVersion) {
    ConfigSnapshotManager manager;

    auto snapshot_v1 = ConfigSnapshotFactory::CreateEmptySnapshot("v1", "V1");
    auto snapshot_v2 = ConfigSnapshotFactory::CreateEmptySnapshot("v2", "V2");

    manager.AddSnapshot(snapshot_v1);
    manager.AddSnapshot(snapshot_v2);

    // 回滚到 v1
    bool result = manager.RollbackToVersion("v1");
    EXPECT_TRUE(result);
    EXPECT_EQ(manager.GetCurrentVersionId(), "v1");
}

TEST_F(ConfigSnapshotManagerTest, RollbackToNonexistent) {
    ConfigSnapshotManager manager;

    manager.AddSnapshot(ConfigSnapshotFactory::CreateEmptySnapshot("v1", "V1"));

    bool result = manager.RollbackToVersion("nonexistent");
    EXPECT_FALSE(result);
}

TEST_F(ConfigSnapshotManagerTest, GetVersionHistory) {
    ConfigSnapshotManager manager;

    manager.AddSnapshot(ConfigSnapshotFactory::CreateEmptySnapshot("v1", "V1"));
    manager.AddSnapshot(ConfigSnapshotFactory::CreateEmptySnapshot("v2", "V2"));

    auto history = manager.GetVersionHistory();
    EXPECT_GE(history.size(), 2);
}

TEST_F(ConfigSnapshotManagerTest, CleanupSnapshots) {
    ConfigSnapshotManager manager;

    // 添加多个快照
    for (int i = 0; i < 15; ++i) {
        manager.AddSnapshot(ConfigSnapshotFactory::CreateEmptySnapshot(
            "v" + std::to_string(i), "Version " + std::to_string(i)));
    }

    EXPECT_EQ(manager.GetSnapshotCount(), 15);

    // 清理，保留 10 个
    size_t removed = manager.CleanupSnapshots(10);
    EXPECT_EQ(manager.GetSnapshotCount(), 10);
}

// ==================== ConfigSnapshot Merge Tests ====================

class ConfigSnapshotMergeTest : public ::testing::Test {
protected:
    void SetUp() override {
    }

    void TearDown() override {
    }
};

TEST_F(ConfigSnapshotMergeTest, MergeSnapshots) {
    std::unordered_map<std::string, ConfigValue> base_data;
    base_data["key1"] = std::string("base_value1");
    base_data["key2"] = 100;
    auto base = ConfigSnapshotFactory::CreateSnapshot(base_data, "base", "Base config");

    std::unordered_map<std::string, ConfigValue> override_data;
    override_data["key2"] = 200;  // 覆盖
    override_data["key3"] = true; // 新增
    auto override = ConfigSnapshotFactory::CreateSnapshot(override_data, "override", "Override config");

    auto merged = ConfigSnapshotFactory::MergeSnapshots(base, override, "merged", "Merged config");

    ConfigValue value;
    EXPECT_TRUE(merged->GetValue("key1", value));
    EXPECT_EQ(std::get<std::string>(value), "base_value1");

    EXPECT_TRUE(merged->GetValue("key2", value));
    EXPECT_EQ(std::get<int>(value), 200);  // 覆盖的值

    EXPECT_TRUE(merged->GetValue("key3", value));
    EXPECT_TRUE(std::get<bool>(value));    // 新增的值
}

TEST_F(ConfigSnapshotMergeTest, MergeWithNullBase) {
    ConfigSnapshot::SnapshotPtr base = nullptr;
    auto override = ConfigSnapshotFactory::CreateEmptySnapshot("override", "Override only");

    auto result = ConfigSnapshotFactory::MergeSnapshots(base, override, "result", "Result");
    EXPECT_EQ(result->GetMetadata().version_id, "override");
}

TEST_F(ConfigSnapshotMergeTest, MergeWithNullOverride) {
    auto base = ConfigSnapshotFactory::CreateEmptySnapshot("base", "Base only");
    ConfigSnapshot::SnapshotPtr override = nullptr;

    auto result = ConfigSnapshotFactory::MergeSnapshots(base, override, "result", "Result");
    EXPECT_EQ(result->GetMetadata().version_id, "base");
}

// ==================== SafeConfigAccessor Tests ====================

class SafeConfigAccessorTest : public ::testing::Test {
protected:
    void SetUp() override {
    }

    void TearDown() override {
    }
};

TEST_F(SafeConfigAccessorTest, GetStringValue) {
    ConfigLifecycleManager manager;
    manager.Initialize("");

    std::unordered_map<std::string, ConfigValue> config_data;
    config_data["test.string"] = std::string("hello");
    auto snapshot = ConfigSnapshotFactory::CreateSnapshot(config_data, "v1", "Test");
    manager.UpdateSnapshot(snapshot);

    ConfigRAIIAccessor accessor(&manager);

    std::string result = SafeConfigAccessor<std::string>::GetValue(accessor, "test.string", "default");
    EXPECT_EQ(result, "hello");

    // 不存在的键返回默认值
    std::string default_result = SafeConfigAccessor<std::string>::GetValue(accessor, "nonexistent", "default");
    EXPECT_EQ(default_result, "default");

    manager.Shutdown();
}

TEST_F(SafeConfigAccessorTest, GetIntValue) {
    ConfigLifecycleManager manager;
    manager.Initialize("");

    std::unordered_map<std::string, ConfigValue> config_data;
    config_data["test.int"] = 42;
    auto snapshot = ConfigSnapshotFactory::CreateSnapshot(config_data, "v1", "Test");
    manager.UpdateSnapshot(snapshot);

    ConfigRAIIAccessor accessor(&manager);

    int result = SafeConfigAccessor<int>::GetValue(accessor, "test.int", 0);
    EXPECT_EQ(result, 42);

    manager.Shutdown();
}

TEST_F(SafeConfigAccessorTest, GetBoolValue) {
    ConfigLifecycleManager manager;
    manager.Initialize("");

    std::unordered_map<std::string, ConfigValue> config_data;
    config_data["test.bool1"] = true;
    config_data["test.bool2"] = std::string("true");  // 字符串转bool
    auto snapshot = ConfigSnapshotFactory::CreateSnapshot(config_data, "v1", "Test");
    manager.UpdateSnapshot(snapshot);

    ConfigRAIIAccessor accessor(&manager);

    bool result1 = SafeConfigAccessor<bool>::GetValue(accessor, "test.bool1", false);
    EXPECT_TRUE(result1);

    bool result2 = SafeConfigAccessor<bool>::GetValue(accessor, "test.bool2", false);
    EXPECT_TRUE(result2);

    manager.Shutdown();
}

TEST_F(SafeConfigAccessorTest, GetDoubleValue) {
    ConfigLifecycleManager manager;
    manager.Initialize("");

    std::unordered_map<std::string, ConfigValue> config_data;
    config_data["test.double"] = 3.14159;
    config_data["test.int_as_double"] = 10;  // int转double
    auto snapshot = ConfigSnapshotFactory::CreateSnapshot(config_data, "v1", "Test");
    manager.UpdateSnapshot(snapshot);

    ConfigRAIIAccessor accessor(&manager);

    double result1 = SafeConfigAccessor<double>::GetValue(accessor, "test.double", 0.0);
    EXPECT_DOUBLE_EQ(result1, 3.14159);

    double result2 = SafeConfigAccessor<double>::GetValue(accessor, "test.int_as_double", 0.0);
    EXPECT_DOUBLE_EQ(result2, 10.0);

    manager.Shutdown();
}

TEST_F(SafeConfigAccessorTest, SetValue) {
    // 测试 SetValue 转换为 ConfigValue
    ConfigValue str_val = SafeConfigAccessor<std::string>::SetValue(std::string("test"));
    EXPECT_TRUE(std::holds_alternative<std::string>(str_val));

    ConfigValue int_val = SafeConfigAccessor<int>::SetValue(42);
    EXPECT_TRUE(std::holds_alternative<int>(int_val));

    ConfigValue bool_val = SafeConfigAccessor<bool>::SetValue(true);
    EXPECT_TRUE(std::holds_alternative<bool>(bool_val));

    ConfigValue double_val = SafeConfigAccessor<double>::SetValue(3.14);
    EXPECT_TRUE(std::holds_alternative<double>(double_val));
}

// ==================== ConfigLifecycleException Tests ====================

class ConfigLifecycleExceptionTest : public ::testing::Test {
protected:
    void SetUp() override {
    }

    void TearDown() override {
    }
};

TEST_F(ConfigLifecycleExceptionTest, ExceptionMessage) {
    ConfigLifecycleException ex("Test error message", ConfigLifecycleState::READY);

    EXPECT_STREQ(ex.what(), "Test error message");
    EXPECT_EQ(ex.GetState(), ConfigLifecycleState::READY);
}

TEST_F(ConfigLifecycleExceptionTest, DifferentStates) {
    ConfigLifecycleException ex1("Error 1", ConfigLifecycleState::UNINITIALIZED);
    ConfigLifecycleException ex2("Error 2", ConfigLifecycleState::INITIALIZING);
    ConfigLifecycleException ex3("Error 3", ConfigLifecycleState::READY);
    ConfigLifecycleException ex4("Error 4", ConfigLifecycleState::SHUTDOWN);

    EXPECT_EQ(ex1.GetState(), ConfigLifecycleState::UNINITIALIZED);
    EXPECT_EQ(ex2.GetState(), ConfigLifecycleState::INITIALIZING);
    EXPECT_EQ(ex3.GetState(), ConfigLifecycleState::READY);
    EXPECT_EQ(ex4.GetState(), ConfigLifecycleState::SHUTDOWN);
}

} // namespace test
} // namespace sqlcc
